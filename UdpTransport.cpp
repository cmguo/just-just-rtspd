// UdpTransport.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/UdpTransport.h"

using namespace boost::system;

namespace ppbox
{
    namespace rtspd
    {

        UdpTransport::UdpTransport(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::uint16_t client_port, 
            boost::uint16_t & server_port, 
            error_code & ec)
            : socket_(rtsp_socket.get_io_service())
        {
            boost::asio::ip::udp::endpoint endpoint(
                rtsp_socket.remote_endpoint().address(), client_port);
            if (!socket_.connect(endpoint, ec))
                server_port = socket_.local_endpoint().port();
        }

        UdpTransport::~UdpTransport()
        {
        }

        error_code UdpTransport::send_packet(
            std::vector<boost::asio::const_buffer> const & buffers)
        {
            error_code ec;
            socket_.send(buffers, 0, ec);
            //assert(!ec);
            return ec;
        }

    } // namespace rtspd
} // namespace ppbox
