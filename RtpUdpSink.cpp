// RtpUdpSink.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpUdpSink.h"

namespace ppbox
{
    namespace rtspd
    {

        RtpUdpSink::RtpUdpSink(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::uint16_t client_port, 
            boost::uint16_t & server_port, 
            boost::system::error_code & ec)
            : RtpSink<boost::asio::ip::udp::socket>(socket_)
            , socket_(rtsp_socket.get_io_service())
        {
            boost::asio::ip::udp::endpoint endpoint(
                rtsp_socket.remote_endpoint().address(), client_port);
            if (server_port == 0) {
                do {
                    socket_.close(ec);
                    socket_.connect(endpoint, ec);
                    server_port = socket_.local_endpoint().port();
                } while (!ec && (server_port & 1));
            } else {
                boost::asio::ip::udp::endpoint local_endpoint(
                    rtsp_socket.local_endpoint().address(), server_port);
                socket_.open(local_endpoint.protocol(), ec) 
                    || socket_.bind(local_endpoint, ec)
                    || socket_.connect(endpoint, ec);
            }
        }

        RtpUdpSink::~RtpUdpSink()
        {
        }

    } // namespace rtspd
} // namespace ppbox
