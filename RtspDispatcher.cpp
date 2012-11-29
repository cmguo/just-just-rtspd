// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/Transport.h"

#include <util/protocol/rtsp/RtspFieldRange.h>
#include <util/protocol/rtsp/RtspError.h>
using namespace util::protocol;

#include <framework/system/LogicError.h>
#include <framework/string/Base16.h>
#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::string;

#include <boost/bind.hpp>
using namespace boost::system;

#include <fstream>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.rtspd.RtspDispatcher", framework::logger::Debug)

namespace ppbox
{
    namespace rtspd
    {
        RtspDispatcher::RtspDispatcher(
            ppbox::dispatch::DispatcherBase & dispatcher)
            : ppbox::dispatch::CustomDispatcher(dispatcher)
        {
        }

        RtspDispatcher::~RtspDispatcher()
        {
        }

        void RtspDispatcher::async_open(
            framework::string::Url & url, 
            std::string const & client, 
            boost::asio::streambuf & os, 
            ppbox::dispatch::response_t  const & resp)
        {
            CustomDispatcher::async_open(url, 
                boost::bind(&RtspDispatcher::handle_open, this, boost::ref(os), resp, _1));
        }

        bool RtspDispatcher::setup(
            boost::asio::ip::tcp::socket & rtsp_sock, 
            std::string const & control, 
            std::string const & in_transport, 
            std::string & out_transport, 
            boost::system::error_code & ec)
        {
            std::string stream_index_str = control.substr(sizeof("track") - 1,2);
            size_t stream_index = -1;
            parse2<size_t>(stream_index_str, stream_index);

            transport_pair_t transports = 
                create_transport_pair(rtsp_sock, in_transport, out_transport, ec);
            if (!CustomDispatcher::setup(stream_index, *transports.first, ec)) {
                return false;
            }
            //if (!CustomDispatcher::setup(stream_index + 0x100, *transports.second, ec)) {
            //    return false;
            //}

            //组 ssrc
            ppbox::data::StreamStatus info;
            info.desc = "ssrc=" + stream_index_str; // this is tricking
            if (!CustomDispatcher::get_stream_status(info, ec)) {
                return false;
            }
            out_transport += ";";
            out_transport += info.desc;

            return true;
        }

        void RtspDispatcher::async_play(
            util::protocol::rtsp_field::Range & range, 
            std::string & rtp_info, 
            ppbox::dispatch::response_t const & seek_resp, 
            ppbox::dispatch::response_t const & resp)
        {
            ppbox::dispatch::SeekRange seek_range_;
            util::protocol::rtsp_field::Range::Unit unit = range[0];
            seek_range_.type = ppbox::dispatch::SeekRange::time;
            seek_range_.beg = boost::uint64_t(unit.begin() * 1000);
            if (unit.has_end()) {
                seek_range_.end = boost::uint64_t(unit.end() * 1000);
            }

            return CustomDispatcher::async_play(seek_range_, 
                boost::bind(&RtspDispatcher::handle_seek, this, boost::ref(rtp_info), boost::ref(range), seek_resp, _1), resp);
        }
        
#ifdef BOOST_WINDOWS_API
#  define gmtime_r(x, y) (gmtime_s(y, x), y)
#endif

        void RtspDispatcher::handle_open(
            boost::asio::streambuf & os_sdp, 
            ppbox::dispatch::response_t const & resp, 
            boost::system::error_code ec)
        {

            ppbox::data::MediaInfo info;
            if (!ec) {
                CustomDispatcher::get_media_info(info, ec);
            }

            if(ec) {
                resp(ec);
                return;
            }

            std::ostream os(&os_sdp);

            //组sdp
            os <<  "v=0\r\n";
            os << "o=- 1322720027229880 1 IN IP4 192.168.1.100\r\n";
            os << "s=Session streamed by PPBOX\r\n";
            os << "i=" << "info.name" << "\r\n";
            //os << "c=IN IP4 " << "0.0.0.0" << "\r\n";
            os << "t=0 0\r\n";
            if (info.type == ppbox::data::MediaInfo::live) {
                os << "a=type:broadcast\r\n";
                os << "a=range:npt=now-\r\n";
            } else {
                os << "a=range:npt=0.000-" << (float)info.duration / 1000.0 << "\r\n";
            }
            os << "a=control:*" << "\r\n";
            os << "c=IN IP4 " << "0.0.0.0" << "\r\n";

            os << info.format_data;

            resp(ec);
        }

        void RtspDispatcher::handle_seek(
            std::string & rtp_info, 
            util::protocol::rtsp_field::Range & range, 
            ppbox::dispatch::response_t const & resp, 
            boost::system::error_code ec)
        {
            ppbox::data::StreamStatus info;
            info.desc = rtp_info; // this is tricking
            if (!ec) {
                CustomDispatcher::get_stream_status(info, ec);
            }

            if (ec) {
                resp(ec);
                return;
            }

            //组 rtp_info
            rtp_info = info.desc;
            
            //填充Range值
            float be = (float)info.time_range.beg / 1000.0;
            float en = (float)info.time_range.end / 1000.0;

            if (info.time_range.end != ppbox::data::invalid_size) {
                en = (float)info.time_range.end / 1000.0;
                range[0] = rtsp_field::Range::Unit(be, en); 
            } else {
                range[0] = rtsp_field::Range::Unit(be, be - 1.0);
            }

            resp(ec);
        }

    } // namespace rtspd
} // namespace ppbox
