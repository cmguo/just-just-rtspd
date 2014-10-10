// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspSession.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspdModule.h"

#include <util/protocol/rtsp/RtspRequest.h>
#include <util/protocol/rtsp/RtspResponse.h>
#include <util/protocol/rtsp/RtspSocket.hpp>
#include <util/protocol/rtsp/RtspError.h>
#include <util/archive/ArchiveBuffer.h>
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

        using util::protocol::rtsp_field::f_public;
        using util::protocol::rtsp_field::f_transport;
        using util::protocol::rtsp_field::f_range;
        using util::protocol::rtsp_field::f_rtp_info;

        RtspSession::RtspSession(
            RtspdModule & mgr)
            : util::protocol::RtspServer(mgr.io_svc())
            , mgr_(mgr)
            , dispatcher_(NULL)
            , play_count_(0)
        {
            static boost::uint32_t g_id = rand();
            session_id_ = ++g_id;

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

        void RtspSession::on_start()
        {
            /*
            RtspRequest req;
            RtspRequestHead & head = req.head();
            head.method = RtspRequestHead::options;
            head.path = "*";
            head["Require"] = "org.wfa.wfd1.0";
            post(req);
            */
        }

        void RtspSession::on_recv(
            RtspRequest const & req)
        {
            //如果不是 Init状态 就是异常情况
            
            boost::system::error_code ec;

            LOG_DEBUG("[on_recv] session_id: " << session_id_ << " path: " << req.head().path);
            req.head().get_content(std::cout);

            if (req.head().method >= RtspRequestHead::setup && dispatcher_ == NULL) {
                ec = rtsp_error::not_open;
                response(ec);
                return;
            }

            switch (req.head().method) {
                case RtspRequestHead::options:
                    response().head()[f_public].reset("OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN");
                    break;
                case RtspRequestHead::describe:
                    {
                        framework::string::Url url(req.head().path);
                        content_base_ = url.to_string() + "/";

                        response().head()["Content-Type"] = "application/sdp";
                        response().head()["Content-Base"] = content_base_;

                        dispatcher_ = mgr_.alloc_dispatcher(url, ec);

                        std::string user_agent = req.head()["User-Agent"];

                        dispatcher_->async_open(
                            url, 
                            user_agent, 
                            response().data(),
                            boost::bind(&RtspServer::response, this, _1));

                        return;
                    }
                    break;
                case RtspRequestHead::setup:
                    {
                        Url url(req.head().path);
                        
                        std::string myurl = url.to_string();
                        int t = myurl.find("/track");
                        if(t < 0 ) {
                            ec = rtsp_error::bad_request;
                            break;
                        }
                        std::string control = myurl.substr(t + 1);

                        std::string transport = req.head()[f_transport].get();
                        response().head()[f_transport] = "";

                        //监听端口之类的
                        dispatcher_->setup(
                            (boost::asio::ip::tcp::socket &)(*this), 
                            control, 
                            transport, 
                            transport, 
                            ec);

                        response().head()[f_transport].set(transport);
                        response().head()["Session"] = format(session_id_);
                    }
                    break;
                case RtspRequestHead::play:
                    {
                        response().head()[f_rtp_info] = content_base_;

                        if (req.head()[f_range].is_set())
                            response().head()[f_range] = req.head()[f_range].get();
                        else
                            response().head()[f_range] = rtsp_field::Range(0);

                        ++play_count_;

                        rtsp_field::Range * range = 
                            new rtsp_field::Range(response().head()[f_range].get());
                        std::string * rtp_info = new std::string(response().head()[f_rtp_info].get());

                        dispatcher_->async_play(
                            *range, 
                            *rtp_info, 
                            boost::bind(&RtspSession::on_seek, this, _1, range, rtp_info), 
                            boost::bind(&RtspSession::on_play, this, _1));

                        response().head()["Session"] = format(session_id_);
                        
                    }
                    return;

                case RtspRequestHead::pause:
                    dispatcher_->cancel(ec);
                    break;

                case RtspRequestHead::teardown:
                    dispatcher_->close(ec);
                    dispatcher_ = NULL;
                    response().head()["Session"] = format(session_id_);
                    session_id_ = 0;
                    break;

                case RtspRequestHead::set_parameter:
                case RtspRequestHead::get_parameter:
                    ec.clear();
                    break;

                default:
                    ec = rtsp_error::option_not_supported;
            }

            response(ec);
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

        void RtspSession::on_seek(
            boost::system::error_code const & ec, 
            rtsp_field::Range * range, 
            std::string * rtp_info)
        {
            response().head()[f_range].set(*range);
            response().head()[f_rtp_info].set(*rtp_info);
            response(ec);
        }

        void RtspSession::on_play(
            boost::system::error_code const & ec)
        {
            LOG_INFO("[on_play] session_id:" << session_id_ << " ec:" << ec.message());

            --play_count_;

            if (!dispatcher_ && play_count_ == 0) {
                stop();
            } else if (ec && ec != boost::asio::error::operation_aborted) {
                boost::system::error_code ec1;
                cancel(ec1);
            }
        }

        void RtspSession::on_sent(
            RtspResponse const & resp)
        {
            resp.head().get_content(std::cout);
            util::archive::ArchiveBuffer<> abuf(resp.data().data());
            std::cout << (&abuf);
            std::cout.clear();

            if (!resp.head()["RTP-Info"].empty()) {
                boost::system::error_code ec1;
                dispatcher_->resume(ec1);
            }
        }

    } // namespace rtspd
} // namespace ppbox
