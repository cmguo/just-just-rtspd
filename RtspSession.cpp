// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspSession.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspManager.h"

#include <util/protocol/rtsp/RtspRequest.h>
#include <util/protocol/rtsp/RtspResponse.h>
#include <util/protocol/rtsp/RtspError.h>
using namespace util::protocol;

#include <framework/string/Url.h>
#include <framework/logger/LoggerStreamRecord.h>
#include <framework/string/Base64.h>
using namespace framework::logger;
using namespace framework::string;

using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("RtspSession", 0)

namespace ppbox
{
    namespace rtspd
    {
        RtspSession::RtspSession(
            RtspManager & mgr)
            : util::protocol::RtspServer(mgr.io_svc()),mgr_(mgr)
        {   
            dispatcher_ = mgr.dispatcher();
            session_id_ = 0;
			rtp_info_send_ = false;
        }

        RtspSession::~RtspSession()
        {
            dispatcher_ = NULL;
        }

        static void nop_deletor(void *) {}

        void RtspSession::local_process(
            local_process_response_type const & resp)
        {
            //如果不是 Init状态 就是异常情况
            
            boost::system::error_code ec;

            LOG_S(Logger::kLevelDebug,"[local_process] session_id:"<<session_id_<<" request:"<<request().head().path);
            request().head().get_content(std::cout);

            switch (request().head().method) {
                case RtspRequestHead::options:
                    response().head().public_.reset("OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN");
                    break;
                case RtspRequestHead::describe:
                    {
                        std::string tmphost = "http://host";
                        std::string url_profix = "base64";
                        framework::string::Url url(request().head().path);
                        std::string url_path(url.path_all());
                        std::string useAgent = request().head()["User-Agent"];

                        //防止一些播放器不支持 playlink带参数的方式
                        if (url_path.compare(1, url_profix.size(), url_profix) == 0) {
                            url_path = url_path.substr(url_profix.size()+1, url_path.size() - url_profix.size()+1);
                            url_path = Base64::decode(url_path);
                            url_path = std::string("/") + url_path;
                        }

                        tmphost += url_path;
                        framework::string::Url request_url(tmphost);
                        //request_url.decode();

                        path_ = request_url.param_or("playlink");

                        response().head()["Content-Type"] = "{application/sdp}";
                        response().head()["Content-Base"] = "{" + request().head().path + "/}";

                        std::string type_ = request_url.param_or("type");
                        if (!type_.empty())
                        {
                            type_ = type_ +"://";
                            path_ = type_ + path_;
                        }
                        path_ = framework::string::Url::decode(path_);

                        std::string format = request_url.param_or("format");

                        if(format.empty())
                        {//取后缀作为格式
                            if (request_url.path().size() > 1) 
                            {
                                std::string option = request_url.path().substr(1);
                                std::vector<std::string> parm;
                                slice<std::string>(option, std::inserter(parm, parm.end()), ".");
                                if (parm.size() == 2) 
                                {
                                    format = parm[1]; 
                                }
                            }
                        }

                        if(!format.empty())
                        {//如果为 es 或 ts 人工加上rtp-es
                            if(format.substr(0, sizeof("rtp"))!= "rtp-")
                            {   
                                format = "rtp-" + format;
                            }
                        }

                        ec = dispatcher_->open(session_id_,path_,format,true,response().data(),
                            useAgent.find("Samsung") == std::string::npos?0:1,
                            get_io_service().wrap(resp));

                        if(ec)
                        {
                            resp(ec);
                        }
                        return;
                    }
                    break;
                case RtspRequestHead::setup:
                    {
                        Url url(request().head().path);
                        
                        std::string myurl = url.to_string();
                        int t = myurl.find("/index=");
                        if(t < 0 )
                        {
                            ec = rtsp_error::bad_request;
                            break;
                        }
                        std::string control = myurl.substr(t+ 1);

                        std::string transport = request().head().transport.get();
                        response().head().transport = "";

                        //监听端口之类的
                        ec = dispatcher_->setup(session_id_
                            ,(boost::asio::ip::tcp::socket*)this
                            , control, transport
                            ,response().head().transport.get()
                            ,get_io_service().wrap(resp)
                            );

                        response().head()["Session"] = "{" + format(session_id_) + "}";
                        return;

                    }
                    break;
                case RtspRequestHead::play:
                    {
                        if (!request().head().range.is_initialized() && rtp_info_send_)
                        {
                            break;
                        }
                        else
                        {
							rtp_info_send_ = true;
                            
							response().head().rtp_info = request().head().path;
                            
							if (request().head().range.is_initialized())
								response().head().range =  request().head().range;
							else
								response().head().range = util::protocol::rtsp_field::Range(0);

                            dispatcher_->seek(session_id_,
                                response().head().range.get(),
                                response().head().rtp_info.get(),
                                get_io_service().wrap(resp));
                            
                            response().head()["Session"] = "{" + format(session_id_) + "}";
                        }
                        return;
                        
                    }
                    break;
                case RtspRequestHead::pause:
                    {
                        ec = dispatcher_->pause(session_id_,
                            get_io_service().wrap(resp));
                        return;
                    }
                    break;
                case RtspRequestHead::teardown:
                    {
                        close_token_.reset((void *)0, nop_deletor);
                        ec = dispatcher_->close(session_id_);
                        response().head()["Session"] = "{" + format(session_id_) + "}";
                        session_id_ = 0;
                    }   
                    break;
                case RtspRequestHead::set_parameter:
                case RtspRequestHead::get_parameter:    
                    {
                        ec.clear();
                    }   
                    break;
                default:
                    ec = rtsp_error::option_not_supported;
            }
            resp(ec);
        }

        void RtspSession::on_error(
            error_code const & ec)
        {
            LOG_S(Logger::kLevelEvent, "[on_error] session_id:"<<session_id_<<" ec:"<<ec.message());
            dispatcher_->close(session_id_);
        }

        void RtspSession::on_play(
            boost::weak_ptr<void> const & token, 
            boost::system::error_code const & ec)
        {
            if (token.expired())
                return;

            LOG_S(Logger::kLevelEvent, "[on_play] session_id:"<<session_id_<<" ec:"<<ec.message());

            if(boost::asio::error::operation_aborted == ec)
            {
                boost::system::error_code ec1;
                cancel(ec1);
            }
        }

        void RtspSession::on_finish()
        {
            response().head().get_content(std::cout);

            if (request().head().method == RtspRequestHead::play) 
            {
                close_token_.reset((void *)0, nop_deletor);
                dispatcher_->play(session_id_, get_io_service().wrap(
                    boost::bind(&RtspSession::on_play, this, boost::weak_ptr<void>(close_token_), _1)));
            }
        }

    } // namespace rtspd
} // namespace ppbox
