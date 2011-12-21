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
            RtpSink();
            virtual ~RtpSink();

        public:
            boost::system::error_code setup(
                boost::asio::ip::tcp::socket * rtsp_sock, 
                std::string const & transport,
                std::string & output);

        private:
            virtual boost::system::error_code write(
                boost::posix_time::ptime const & time_send, 
                ppbox::demux::Sample&);

            void send_rtcp(
                boost::posix_time::ptime const & time_send, 
                ppbox::mux::RtpPacket const & rtp);

        private:
            std::pair<Transport *, Transport *> transports_;
            boost::asio::streambuf rtcp_buf_;
            boost::uint32_t num_pkt_;
            boost::uint64_t num_byte_;
            framework::timer::Time next_rtcp_time_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
