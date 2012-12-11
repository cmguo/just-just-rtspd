// RtpTcpSink.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpTcpSink.h"

namespace ppbox
{
    namespace rtspd
    {

        RtpTcpSink::RtpTcpSink(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::uint8_t interleaveds[2], 
            boost::system::error_code & ec)
            : RtpSink<RtpTcpSocket>(rtp_socket_, rtcp_socket_)
            , rtp_socket_(rtsp_socket, interleaveds[0])
            , rtcp_socket_(rtsp_socket, interleaveds[1])
        {
            ec.clear();
        }

        RtpTcpSink::~RtpTcpSink()
        {
        }

    } // namespace rtspd
} // namespace ppbox
