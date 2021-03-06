// RtspSession.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/RtspDispatcher.h"
#include "just/rtspd/Transport.h"
#include "just/rtspd/RtpStreamDesc.h"

using namespace just::dispatch;

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

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.rtspd.RtspDispatcher", framework::logger::Debug)

namespace just
{
    namespace rtspd
    {
        RtspDispatcher::RtspDispatcher(
            just::dispatch::DispatcherBase & dispatcher)
            : just::dispatch::CustomDispatcher(dispatcher)
        {
        }

        RtspDispatcher::~RtspDispatcher()
        {
            for (size_t i = 0; i < sinks_.size(); ++i) {
                delete sinks_[i];
            }
        }

        void RtspDispatcher::async_open(
            framework::string::Url & url, 
            std::string const & client, 
            boost::asio::streambuf & os, 
            just::dispatch::response_t  const & resp)
        {
            if (client.find("Samsung") != std::string::npos
                && client.find("NexPlayer") != std::string::npos) {
                    url.param("mux.RtpH264.usedts", "true");
            }
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
            std::string stream_index_str = control.substr(sizeof("track") - 1);
            size_t stream_index = -1;
            parse2<size_t>(stream_index_str, stream_index);

            util::stream::Sink * sink = 
                create_transport(rtsp_sock, in_transport, out_transport, ec);
            if (!CustomDispatcher::setup(stream_index, *sink, ec)) {
                delete sink;
                return false;
            }
            //if (!CustomDispatcher::setup(stream_index + 0x100, *transports.second, ec)) {
            //    return false;
            //}
 
            sinks_.push_back(sink);

            //�� ssrc
            std::vector<StreamInfo> info;
            if (!CustomDispatcher::get_stream_info(info, ec)) {
                return false;
            }

            // RtpStreamDesc for track(-1) is at index 0
            if (stream_index == size_t(-1))
                stream_index = 0;
            RtpStreamDesc rtp_desc;
            rtp_desc.from_data(info[stream_index].format_data);
            
            out_transport += ";ssrc=";
            out_transport += framework::string::Base16::encode(std::string((char const *)&rtp_desc.ssrc, 4));

            return true;
        }

        void RtspDispatcher::async_play(
            util::protocol::rtsp_field::Range & range, 
            std::string & rtp_info, 
            just::dispatch::response_t const & seek_resp, 
            just::dispatch::response_t const & resp)
        {
            just::dispatch::SeekRange seek_range_;
            util::protocol::rtsp_field::Range::Unit unit = range[0];
            seek_range_.type = just::dispatch::SeekRange::time;
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
            just::dispatch::response_t const & resp, 
            boost::system::error_code ec)
        {

            just::avbase::MediaInfo info;
            if (!ec) {
                CustomDispatcher::get_media_info(info, ec);
            }

            if(ec) {
                resp(ec);
                return;
            }

            std::ostream os(&os_sdp);

            //��sdp
            os << "v=0\r\n";
            os << "o=- 1322720027229880 1 IN IP4 192.168.1.100\r\n";
            os << "s=Session streamed by JUST\r\n";
            os << "i=" << info.name << "\r\n";
            os << "c=IN IP4 " << "0.0.0.0" << "\r\n";
            os << "t=0 0\r\n";
            if (info.type == just::avbase::MediaInfo::live) {
                os << "a=type:broadcast\r\n";
                os << "a=range:npt=now-\r\n";
            } else if (info.duration == just::avbase::invalid_size) {
                os << "a=range:npt=0.000-\r\n";
            } else {
                os << "a=range:npt=0.000-" << (float)info.duration / 1000.0 << "\r\n";
            }
            os << "a=control:*" << "\r\n";

            os << info.format_data;

            resp(ec);
        }

        void RtspDispatcher::handle_seek(
            std::string & rtp_info, 
            util::protocol::rtsp_field::Range & range, 
            just::dispatch::response_t const & resp, 
            boost::system::error_code ec)
        {
            just::avbase::StreamStatus status;
            std::vector<just::avbase::StreamInfo> streams;

            if (!ec) {
                CustomDispatcher::get_stream_status(status, ec);
            }
            if (!ec) {
                CustomDispatcher::get_stream_info(streams, ec);
            }

            if (ec) {
                resp(ec);
                return;
            }

            //�� rtp_info
            std::string url;
            url.swap(rtp_info);
            for (size_t i = 0; i < streams.size(); ++i) {
                if (streams[i].format_data.empty())
                    continue;
                RtpStreamDesc rtp_desc;
                rtp_desc.from_data(streams[i].format_data);
                if (rtp_desc.setup) {
                    rtp_info.append("url=");
                    rtp_info.append(url);
                    rtp_info.append(rtp_desc.stream);
                    rtp_info.append(1, ';');
                    rtp_info.append(rtp_desc.rtp_info);
                    rtp_info.append(1, ',');
                }
            }
            if (!rtp_info.empty()) {
                rtp_info.erase(--rtp_info.end());
            }
            
            //���Rangeֵ
            float be = (float)status.time_range.beg / 1000.0f;
            float en = (float)status.time_range.end / 1000.0f;

            if (status.time_range.end != just::avbase::invalid_size) {
                en = (float)status.time_range.end / 1000.0f;
                range[0] = rtsp_field::Range::Unit(be, en); 
            } else {
                range[0] = rtsp_field::Range::Unit(be, be - 1.0f);
            }

            resp(ec);
        }

    } // namespace rtspd
} // namespace just
