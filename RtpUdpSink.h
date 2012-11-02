// RtpUdpSink.h

#ifndef _PPBOX_RTSPD_RTP_UDP_SINK_H_
#define _PPBOX_RTSPD_RTP_UDP_SINK_H_

#include "ppbox/rtspd/RtpSink.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/utility/base_from_member.hpp>

namespace ppbox
{

    namespace rtspd
    {

        class RtpUdpSink
            : boost::base_from_member<boost::asio::ip::udp::socket>
            , public RtpSink<boost::asio::ip::udp::socket>
        {
        public:
            RtpUdpSink(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::uint16_t client_port, 
                boost::uint16_t & server_port, 
                boost::system::error_code & ec);

            virtual ~RtpUdpSink();

        private:
            socket_t & socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_UDP_SINK_H_
