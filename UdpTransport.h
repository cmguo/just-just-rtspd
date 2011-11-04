// UdpTransport.h

#ifndef _PPBOX_RTSPD_UDP_TRANSPORT_H_
#define _PPBOX_RTSPD_UDP_TRANSPORT_H_

#include "ppbox/rtspd/Transport.h"

#include <boost/asio/ip/udp.hpp>

namespace ppbox
{
    namespace rtspd
    {

        class UdpTransport
            : public Transport
        {
        public:
            UdpTransport(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::uint16_t client_port, 
                boost::uint16_t & server_port, 
                boost::system::error_code & ec);

            virtual ~UdpTransport();

        public:
            virtual boost::system::error_code send_packet(
                std::vector<boost::asio::const_buffer> const & buffers);

        private:
            boost::asio::ip::udp::socket socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_UDP_TRANSPORT_H_