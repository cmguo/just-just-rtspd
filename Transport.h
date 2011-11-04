// Transport.h

#ifndef _PPBOX_RTSPD_TRANSPORT_H_
#define _PPBOX_RTSPD_TRANSPORT_H_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>

namespace ppbox
{
    namespace rtspd
    {

        class Transport
        {
        public:
            Transport() {}

            virtual ~Transport() {};

        public:
            virtual boost::system::error_code send_packet(
                std::vector<boost::asio::const_buffer> const & buffers) = 0;

            virtual boost::system::error_code send(const char*,const size_t){return boost::system::error_code(); }

        public:
            static std::pair<Transport *, Transport *> create(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                std::string const & in_transport, 
                std::string & out_transport, 
                boost::system::error_code & ec);
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_TRANSPORT_H_