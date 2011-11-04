// TcpTransport.h

#ifndef _PPBOX_RTSPD_TCP_TRANSPORT_H_
#define _PPBOX_RTSPD_TCP_TRANSPORT_H_

#include "ppbox/rtspd/Transport.h"

namespace ppbox
{
    namespace rtspd
    {

        class TcpTransport
            : public Transport
        {
        public:
            TcpTransport(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::uint8_t interleave, 
                boost::system::error_code & ec);

            virtual ~TcpTransport();

        public:
            virtual boost::system::error_code send_packet(
                std::vector<boost::asio::const_buffer> const & buffers);

        private:
            boost::asio::ip::tcp::socket & socket_;
            boost::uint8_t interleave_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_TCP_TRANSPORT_H_