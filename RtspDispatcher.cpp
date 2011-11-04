// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtpSink.h"

#include <ppbox/mux/rtp/RtpPacket.h>
#include <ppbox/mux/Muxer.h>

#include <framework/system/LogicError.h>
#include <framework/string/Base16.h>
#include <boost/bind.hpp>
#include <util/protocol/rtsp/RtspError.h>

using namespace framework::system::logic_error;
using namespace framework::logger;
using namespace boost::system;
using namespace util::protocol;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("RtspDispatcher", 0)

namespace ppbox
{
    namespace rtspd
    {
        RtspDispatcher::RtspDispatcher(
            util::daemon::Daemon & daemon)
            : ppbox::mux::Dispatcher(daemon)
        {
        }

        RtspDispatcher::~RtspDispatcher()
        {
        }

        boost::system::error_code RtspDispatcher::seek(
            const boost::uint32_t session_id
            ,boost::uint32_t begin
            ,boost::uint32_t end
            ,std::string & rtp_info
            ,boost::uint32_t& seek_beg
            ,boost::uint32_t& seek_end
            ,ppbox::mux::session_callback_respone const &resp)
        {
            return Dispatcher::seek(session_id,begin,end,
                boost::bind(&RtspDispatcher::on_seek,this,boost::ref(rtp_info),boost::ref(seek_beg),boost::ref(seek_end),resp,_1));
        }

        
        error_code RtspDispatcher::open(
            boost::uint32_t& session_id,
            std::string const & play_link,
            std::string const & format,
            bool need_session,
            std::string & rtp_sdp,
            ppbox::mux::session_callback_respone const & resp
            )
        {
            return Dispatcher::open(session_id,play_link,format,need_session,
                boost::bind(&RtspDispatcher::on_open,this,boost::ref(rtp_sdp),resp,_1));
        }

        boost::system::error_code RtspDispatcher::setup(
            boost::uint32_t session_id, 
            boost::asio::ip::tcp::socket * rtsp_sock, 
            std::string const &  control, 
            std::string const &  transport,
            std::string & rtp_setup,
            ppbox::mux::session_callback_respone const & resp)
        {
            std::string stream_index_str = control.substr(sizeof("index=") - 1,2);
            size_t stream_index = -1;
            parse2<size_t>(stream_index_str, stream_index);
            
                
            RtpSink* sink = new RtpSink();
            sink->setup(rtsp_sock, transport, rtp_setup);
            return Dispatcher::setup(session_id,
                stream_index,
                sink, 
                boost::bind(&RtspDispatcher::on_setup,this,stream_index,boost::ref(rtp_setup),resp,_1));
        }

        void RtspDispatcher::on_seek(
            std::string& rtp_info
            ,boost::uint32_t& seek_beg
            ,boost::uint32_t& seek_end
            ,ppbox::mux::session_callback_respone const &resp
            ,boost::system::error_code ec)
        {
            if(ec)
            {
                resp(ec);
                return;
            }
            assert(NULL != cur_mov_);
            assert(NULL != cur_mov_->muxer);
            const ppbox::mux::MediaFileInfo & infoTemp = cur_mov_->muxer->mediainfo();

            seek_end = infoTemp.duration;
            //×é rtp_info
            
            std::ostringstream os;
            ppbox::mux::RtpInfo* pinfo = NULL;
 
            if ("rtp-ts" == cur_mov_->format)
            {
                pinfo = (ppbox::mux::RtpInfo*)infoTemp.attachment;
                seek_beg = pinfo->seek_time;

                os<<"url="<<cur_mov_->play_link ;
                os<<"/index=-1";
                os<<";seq="<<pinfo->sequence;
                //timestamp = pinfo->timestamp + cur_mov_->seek*90;
                os<<";rtptime="<<pinfo->timestamp;
            }
            else if("rtp-es" == cur_mov_->format)
            {

                pinfo = 
                    (ppbox::mux::RtpInfo*)infoTemp.stream_infos[infoTemp.video_index].attachment;

                seek_beg = pinfo->seek_time;

                os<<"url="<<cur_mov_->play_link ;
                os<<"/index="<<infoTemp.video_index;
                os<<";seq="<<pinfo->sequence;
                //timestamp = pinfo->timestamp + cur_mov_->seek*90;
                os<<";rtptime="<<pinfo->timestamp;

                const ppbox::demux::MediaInfo& info = infoTemp.stream_infos[infoTemp.audio_index];
                pinfo = 
                    (ppbox::mux::RtpInfo*)info.attachment;

                os<<",url="<<cur_mov_->play_link ;
                os<<"/index="<<infoTemp.audio_index;
                os<<";seq="<<pinfo->sequence;
                
                //timestamp = pinfo->timestamp + 
                 //   cur_mov_->seek * info.audio_format.sample_rate / 1000;
                os<<";rtptime="<<pinfo->timestamp;
            }
            else
            {
                LOG_S(Logger::kLevelError, "[on_play] Wrong Fromat");
                assert(0);
            }

            rtp_info = os.str();
            resp(ec);
            
        }

        void RtspDispatcher::on_open(
            std::string & rtp_sdp
            ,ppbox::mux::session_callback_respone const &resp
            ,boost::system::error_code ec)
        {
            if(ec)
            {
                resp(ec);
                return;
            }
            assert(NULL != cur_mov_);
            assert(NULL != cur_mov_->muxer);
            const ppbox::mux::MediaFileInfo & infoTemp = cur_mov_->muxer->mediainfo();

            std::ostringstream os;
            //×ésdp
            os <<  "v=0\r\n";
            os << "o=PPBOX " << " 3523770323 1314781361000 IN IP4 0.0.0.0\r\n";
            os << "s=" << cur_mov_->play_link << "\r\n";
            os << "c=IN IP4 " << "0.0.0.0" << "\r\n";
            os << "t=0 0\r\n";
            os << "a=range:npt=0.000-" << (float)infoTemp.duration/1000.0<< "\r\n";
            os << "a=control:*" << "\r\n";
            
            ppbox::mux::RtpInfo* pinfo = NULL;

            if ("rtp-ts" == cur_mov_->format)
            {
                pinfo = (ppbox::mux::RtpInfo*)infoTemp.attachment;
                os << pinfo->sdp;
            }
            else if("rtp-es" == cur_mov_->format)
            {
                //infoTemp.stream_infos[infoTemp.video_index].attachment;
                for(size_t ii = 0; ii < infoTemp.stream_infos.size(); ++ii )
                {
                    pinfo = (ppbox::mux::RtpInfo*)infoTemp.stream_infos[ii].attachment;
                    os << pinfo->sdp;
                }
                
            }
            else
            {
                LOG_S(Logger::kLevelError, "[on_open] Wrong Fromat");
                assert(0);
            }
            rtp_sdp = os.str();
            resp(ec);
        }

        void RtspDispatcher::on_setup(
            size_t& index,
            std::string& rtp_info,
            ppbox::mux::session_callback_respone const &resp,
            boost::system::error_code ec)
        {

            //boost::system::error_code ec;

            assert(NULL != cur_mov_);
            assert(NULL != cur_mov_->muxer);
            const ppbox::mux::MediaFileInfo & infoTemp = cur_mov_->muxer->mediainfo();
            
            boost::uint32_t iSsrc = 0;
            //×é rtp_info

            std::ostringstream os;

            if ("rtp-ts" == cur_mov_->format)
            {
                ppbox::mux::RtpInfo* pinfo = (ppbox::mux::RtpInfo*)infoTemp.attachment;
                iSsrc = pinfo->ssrc;

            }
            else if("rtp-es" == cur_mov_->format)
            {
                if(index < infoTemp.stream_infos.size())
                {
                    ppbox::mux::RtpInfo* pinfo = 
                        (ppbox::mux::RtpInfo*)infoTemp.stream_infos[index].attachment;
                    iSsrc = pinfo->ssrc;
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
            
            std::string str_ssrc = "{";
            str_ssrc += rtp_info;
            str_ssrc +=  ";ssrc=";
            str_ssrc += framework::string::Base16::encode(std::string((char const *)&iSsrc, 4));
            str_ssrc +=  "}";
            rtp_info = str_ssrc;

            resp(ec);
        }
    } // namespace rtspd
} // namespace ppbox
