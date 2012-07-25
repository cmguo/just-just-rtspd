#ifndef _PPBOX_RTSPD_RTPSINK_H_
#define _PPBOX_RTSPD_RTPSINK_H_

#include <ppbox/mux/tool/Sink.h>

#include <framework/timer/ClockTime.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

namespace ppbox
{

    namespace mux
    {
        class RtpPacket;
    }

    namespace rtspd
    {
        class Transport;

        class RtpSink
            : public ppbox::mux::Sink
        {
        public:
            RtpSink(
                bool with_rtcp = true);

            virtual ~RtpSink();

        public:
            boost::system::error_code setup(
                boost::asio::ip::tcp::socket * rtsp_sock, 
                std::string const & transport,
                std::string & output);

        private:
            virtual size_t write(
                boost::posix_time::ptime const & time_send, 
                ppbox::demux::Sample&,
                boost::system::error_code&);

            void send_rtcp(
                boost::posix_time::ptime const & time_send, 
                ppbox::mux::RtpPacket const & rtp);

        private:
            std::pair<Transport *, Transport *> transports_;
            boost::uint32_t num_pkt_;
            boost::uint64_t num_byte_;
            bool with_rtcp_;
            boost::asio::streambuf rtcp_buf_;
            framework::timer::Time next_rtcp_time_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
