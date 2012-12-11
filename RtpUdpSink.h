// RtpUdpSink.h

#ifndef _PPBOX_RTSPD_RTP_UDP_SINK_H_
#define _PPBOX_RTSPD_RTP_UDP_SINK_H_

#include "ppbox/rtspd/RtpSink.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace ppbox
{

    namespace rtspd
    {

        class RtpUdpSink
            : public RtpSink<boost::asio::ip::udp::socket>
        {
        public:
            RtpUdpSink(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::uint16_t client_port[2], 
                boost::uint16_t server_port[2], 
                boost::system::error_code & ec);

            virtual ~RtpUdpSink();

        private:
            boost::asio::ip::udp::socket rtp_socket_;
            boost::asio::ip::udp::socket rtcp_socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_UDP_SINK_H_
