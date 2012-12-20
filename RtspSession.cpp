// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspSession.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspdModule.h"

#include <util/protocol/rtsp/RtspRequest.h>
#include <util/protocol/rtsp/RtspResponse.h>
#include <util/protocol/rtsp/RtspError.h>
using namespace util::protocol;

#include <framework/string/Url.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Base64.h>
using namespace framework::string;

using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.rtspd.RtspSession", framework::logger::Debug)

namespace ppbox
{
    namespace rtspd
    {

        RtspSession::RtspSession(
            RtspdModule & mgr)
            : util::protocol::RtspServer(mgr.io_svc())
            , mgr_(mgr)
            , dispatcher_(NULL)
            , playing_(false)
        {
            static boost::uint32_t g_id = 0;
            session_id_ = ++g_id;
            rtp_info_send_ = false;

            boost::system::error_code ec;
            set_non_block(true, ec);
        }

        RtspSession::~RtspSession()
        {
            if (dispatcher_) {
                mgr_.free_dispatcher(dispatcher_);
                dispatcher_ = NULL;
            }
        }

        static void nop_deletor(void *) {}

        void RtspSession::local_process(
            response_type const & resp)
        {
            //如果不是 Init状态 就是异常情况
            
            boost::system::error_code ec;

            LOG_DEBUG("[local_process] session_id:"<<session_id_<<" request:"<<request().head().path);
            request().head().get_content(std::cout);

            switch (request().head().method) {
                case RtspRequestHead::options:
                    response().head().public_.reset("OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN");
                    break;
                case RtspRequestHead::describe:
                    {
                        framework::string::Url url(request().head().path);

                        dispatcher_ = mgr_.alloc_dispatcher(url, ec);

                        response().head()["Content-Type"] = "{application/sdp}";
                        response().head()["Content-Base"] = "{" + request().head().path + "/}";

                        std::string user_agent = request().head()["User-Agent"];

                        dispatcher_->async_open(
                            url, 
                            user_agent, 
                            response().data(),
                            resp);

                        return;
                    }
                    break;
                case RtspRequestHead::setup:
                    {
                        Url url(request().head().path);
                        
                        std::string myurl = url.to_string();
                        int t = myurl.find("/track");
                        if(t < 0 ) {
                            ec = rtsp_error::bad_request;
                            break;
                        }
                        std::string control = myurl.substr(t + 1);

                        std::string transport = request().head().transport.get();
                        response().head().transport = "";

                        //监听端口之类的
                        dispatcher_->setup(
                            (boost::asio::ip::tcp::socket &)(*this), 
                            control, 
                            transport, 
                            response().head().transport.get(), 
                            ec);

                        response().head()["Session"] = "{" + format(session_id_) + "}";
                    }
                    break;
                case RtspRequestHead::play:
                    {
                        response().head().rtp_info = request().head().path;

                        if (request().head().range.is_initialized())
                            response().head().range =  request().head().range;
                        else
                            response().head().range = util::protocol::rtsp_field::Range(0);

                        playing_ = true;

                        dispatcher_->async_play(
                            response().head().range.get(), 
                            response().head().rtp_info.get(), 
                            resp, 
                            boost::bind(&RtspSession::on_play, this, _1));

                        response().head()["Session"] = "{" + format(session_id_) + "}";
                        
                    }
                    return;

                case RtspRequestHead::pause:
                    dispatcher_->cancel(ec);
                    break;

                case RtspRequestHead::teardown:
                    dispatcher_->close(ec);
                    dispatcher_ = NULL;
                    response().head()["Session"] = "{" + format(session_id_) + "}";
                    session_id_ = 0;
                    if (playing_) {
                        post_resp_ = resp;
                        return;
                    }
                    break;

                case RtspRequestHead::set_parameter:
                case RtspRequestHead::get_parameter:
                    ec.clear();
                    break;

                default:
                    ec = rtsp_error::option_not_supported;
            }
            resp(ec);
        }

        void RtspSession::on_error(
            error_code const & ec)
        {
            LOG_INFO("[on_error] session_id:" << session_id_ << " ec:" << ec.message());
            if (dispatcher_) {
                boost::system::error_code ec1;
                dispatcher_->close(ec1);
                dispatcher_ = NULL;
            }
        }

        void RtspSession::post_process(
            response_type const & resp)
        {
            LOG_INFO("[post_process] session_id:" << session_id_);

            if (playing_) {
                post_resp_ = resp;
            } else {
                boost::system::error_code ec;
                resp(ec);
            }
        }

        void RtspSession::on_play(
            boost::system::error_code const & ec)
        {
            LOG_INFO("[on_play] session_id:" << session_id_ << " ec:" << ec.message());

            playing_ = false;

            if (!post_resp_.empty()) {
                boost::system::error_code ec1;
                response_type resp;
                resp.swap(post_resp_);
                resp(ec1);
            } else if (ec && ec != boost::asio::error::operation_aborted) {
                boost::system::error_code ec1;
                cancel(ec1);
            }
        }

        void RtspSession::on_finish()
        {
            response().head().get_content(std::cout);

            if (request().head().method == RtspRequestHead::play) {
                boost::system::error_code ec1;
                dispatcher_->resume(ec1);
            }
        }

    } // namespace rtspd
} // namespace ppbox
