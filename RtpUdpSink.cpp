// RtpUdpSink.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpUdpSink.h"

namespace ppbox
{
    namespace rtspd
    {

        RtpUdpSink::RtpUdpSink(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::uint16_t client_port[2], 
            boost::uint16_t server_port[2], 
            boost::system::error_code & ec)
            : RtpSink<boost::asio::ip::udp::socket>(rtsp_socket.get_io_service(), rtp_socket_, rtcp_socket_)
            , rtp_socket_(rtsp_socket.get_io_service())
            , rtcp_socket_(rtsp_socket.get_io_service())
        {
            boost::asio::ip::udp::endpoint endpoint(
                rtsp_socket.remote_endpoint().address(), client_port[0]);
            while (true) {
                endpoint.port(client_port[0]);
                do {
                    rtp_socket_.close(ec);
                    rtp_socket_.connect(endpoint, ec);
                    server_port[0] = rtp_socket_.local_endpoint().port();
                } while (!ec && (server_port[0] & 1));
                if (ec) {
                    break;
                }
                endpoint.port(client_port[1]);
                server_port[1] = server_port[0] + 1;
                boost::asio::ip::udp::endpoint local_endpoint(
                    rtsp_socket.local_endpoint().address(), server_port[1]);
                rtcp_socket_.open(local_endpoint.protocol(), ec)
                    || rtcp_socket_.bind(local_endpoint, ec)
                    || rtcp_socket_.connect(endpoint, ec);
                if (ec != boost::asio::error::address_in_use) {
                    break;
                }
                rtcp_socket_.close(ec);
            }
        }

        RtpUdpSink::~RtpUdpSink()
        {
        }

    } // namespace rtspd
} // namespace ppbox
