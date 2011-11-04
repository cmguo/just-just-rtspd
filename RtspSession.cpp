// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspSession.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspManager.h"

#include <util/protocol/rtsp/RtspRequest.h>
#include <util/protocol/rtsp/RtspResponse.h>
#include <util/protocol/rtsp/RtspError.h>
using namespace util::protocol;

#include <framework/logger/LoggerFormatRecord.h>
#include <framework/string/Url.h>
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
        }

        RtspSession::~RtspSession()
        {

            if( dispatcher_ != NULL)
            {
                dispatcher_->close(session_id_);
                dispatcher_ = NULL;
            }
        }

        void RtspSession::local_process(
            local_process_response_type const & resp)
        {
            //如果不是 Init状态 就是异常情况
            

            boost::system::error_code ec;
            request().head().get_content(std::cout);
            switch (request().head().method) {
                case RtspRequestHead::options:
                    response().head().public_.reset("OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN");
                    break;
                case RtspRequestHead::describe:
                    {
                        
                        Url url(request().head().path);

                        path_ = url.param_or("playlink");

                        response().head()["Content-Type"] = "{application/sdp}";
                        response().head()["Content-Base"] = "{" + request().head().path + "/}";

                        std::string type_ = url.param_or("type");
                        if (!type_.empty())
                        {
                            type_ = type_ +"://";
                            path_ = type_ + path_;
                        }
                        std::string format_ = url.param_or("format");
                        if (format_.empty())
                        {
                            format_ = "rtp-es";
                        }
                        else if ("rtp-ts" == format_ || "rtp-es" == format_)
                        {

                        }
                        else
                        {
                            ec = rtsp_error::bad_request;
                            break;
                        }

                        ec = dispatcher_->open(session_id_,path_,format_,true,rtp_sdp_,
                            get_io_service().wrap(boost::bind(&RtspSession::AsyncOpenCallback,this,resp,_1)));

                        if(ec)
                        {
                            dispatcher_ = NULL;
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

                        std::string transport = request().head()["Transport"];

                        //监听端口之类的
                        ec = dispatcher_->setup(session_id_,(boost::asio::ip::tcp::socket*)this, control, transport,rtp_setup_,
                            get_io_service().wrap(boost::bind(&RtspSession::AsyncSetupCallback,this,resp,_1)));
                        return;

                    }
                    break;
                case RtspRequestHead::play:
                    {
                        //三种可能
                        //1 从头播放 2 seek播放  3取消暂停
                        
                        if (!request().head().range.is_initialized())
                        {
                            dispatcher_->resume(session_id_,
                                get_io_service().wrap(boost::bind(&RtspSession::AsyncTempCallback,this,_1)));
                            break;
                        }
                        else
                        {
                            //如果Range 0-  表示是Play
                            //如果Range 0-XXX 表示是Seek
                            rtsp_field::Range range = request().head().range.get();

                            rtsp_field::Range::Unit & unit = range[0];

                            boost::uint32_t seek_beg = boost::uint32_t(unit.b_ * 1000.0f);
                            boost::uint32_t seek_end = boost::uint32_t(unit.e_ * 1000.0f);

                            seek_end_ = seek_end;
                            seek_beg_ = seek_beg;

                            dispatcher_->seek(session_id_,seek_beg, seek_end,rtp_info_,seek_beg_,seek_end_,
                                get_io_service().wrap(boost::bind(&RtspSession::AsyncSeekCallback,this,resp,_1)));
                            response().head()["Session"] = "{" + format(session_id_) + "}";
                            //response().head().range.reset(range);
                            response().head()["Transport"] = "";
                        }
                        return;
                        
                    }
                    break;
                case RtspRequestHead::pause:
                    {
                        ec = dispatcher_->pause(session_id_,
                        get_io_service().wrap(boost::bind(&RtspSession::AsyncPauseCallback,this,resp,_1)));
                        return;
                    }
                    break;
                case RtspRequestHead::teardown:
                    {
                        ec = dispatcher_->close(session_id_);
                        response().head()["Session"] = "{" + format(session_id_) + "}";
                        response().head()["RTP-Info"] = "";
                        dispatcher_ = NULL;
                    }   
                    break;
                default:
                    ec = rtsp_error::option_not_supported;
            }
            response().head().err_msg = "OK";
            response().head().get_content(std::cout);
            resp(ec);
        }

        void RtspSession::on_error(
            error_code const & ec)
        {
            LOG_S(Logger::kLevelEvent, "[on_error] ["<<(boost::uint32_t)this<<"]"
                <<"["<<session_id_<<"] Client Clost Link");
            if (NULL == dispatcher_)
            {
                return;
            }
            LOG_S(Logger::kLevelEvent, "[on_error] "<< ec.message());
            dispatcher_->close(session_id_);
            dispatcher_ = NULL;
        }

        void RtspSession::on_finish()
        {
            response().head().get_content(std::cout);
            
            if (request().head().method == RtspRequestHead::play) 
            {
                dispatcher_->play(session_id_,
                    get_io_service().wrap(boost::bind(&RtspSession::AsyncTempCallback,this,_1)));
            }
        }


        void RtspSession::AsyncSeekCallback(
            local_process_response_type const & resp,
            boost::system::error_code const & ec
            )
        {
            LOG_S(Logger::kLevelEvent, "[AsyncSeekCallback] ["<<(boost::uint32_t)this<<"]"
                <<ec<<"["<<session_id_<<"] ");
            if (!ec)
            {
                rtsp_field::Range range;
                
                range[0].b_ = (float)(seek_beg_/1000.0);
                range[0].e_ = (float)(seek_end_/1000.0);

                response().head().range.reset(range);

                response().head().err_msg = "OK";
                LOG_S(Logger::kLevelEvent, "[AsyncSeekCallback] ["<<rtp_info_);
                response().head().rtp_info=  rtp_info_;

                dispatcher_->play(session_id_,
                    get_io_service().wrap(boost::bind(&RtspSession::AsyncTempCallback,this,_1)));
            }
            resp(ec);
        }

         void RtspSession::AsyncOpenCallback(
             local_process_response_type const & resp,
             boost::system::error_code const & ec
             )
         {
             LOG_S(Logger::kLevelEvent, "[AsyncOpenCallback] ["<<(boost::uint32_t)this<<"]"
                 <<ec<<"["<<session_id_<<"] ");
             if (ec)
             {
                 //频步打开失败
             }
             else
             {
                 std::ostream os(&response().data());
                 os<<rtp_sdp_;
                 LOG_S(Logger::kLevelEvent, "[AsyncSeekCallback] ["<<rtp_sdp_);
                 response().head().err_msg = "OK";
                 response().head().get_content(std::cout);
             }

             resp(ec);
         }

         void RtspSession::AsyncPlayCallback(
             local_process_response_type const & resp,
             boost::system::error_code const & ec
             )
         {
             LOG_S(Logger::kLevelEvent, "[AsyncPlayCallback] ["<<(boost::uint32_t)this<<"]"
                 <<ec<<"["<<session_id_<<"] ");
             if (ec)
             {
                 //频步打开失败
             }
             else
             {
                 response().head().err_msg = "OK";
             }

             resp(ec);
         }
         void RtspSession::AsyncPauseCallback(
             local_process_response_type const & resp,
             boost::system::error_code const & ec
             )
         {
             LOG_S(Logger::kLevelEvent, "[AsyncPauseCallback] ["<<(boost::uint32_t)this<<"]"
                 <<ec<<"["<<session_id_<<"] ");
             if (ec)
             {
                 //频步打开失败
             }
             else
             {
                 response().head().err_msg = "OK";
             }
             resp(ec);

         }
        void RtspSession::AsyncSetupCallback(
             local_process_response_type const & resp,
             boost::system::error_code const & ec
             )
         {
             LOG_S(Logger::kLevelEvent, "[AsyncSetupCallback] ["<<(boost::uint32_t)this<<"]"
                 <<ec<<"["<<session_id_<<"] ");
             if (ec)
             {
                 //频步打开失败
             }
             else
             {
                 LOG_S(Logger::kLevelEvent, "[AsyncSeekCallback] ["<<rtp_setup_);
                 response().head()["Transport"] = rtp_setup_;
                 response().head()["Session"] = "{" + format(session_id_) + "}";
                 response().head().err_msg = "OK";
             }
             resp(ec);

         }

        void RtspSession::AsyncTempCallback(
            boost::system::error_code const & ec
            )
        {
            LOG_S(Logger::kLevelEvent, "[AsyncTempCallback] Play End");
        }
    } // namespace rtspd
} // namespace ppbox
