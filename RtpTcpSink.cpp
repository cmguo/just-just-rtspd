// RtpTcpSink.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpTcpSink.h"

namespace ppbox
{
    namespace rtspd
    {

        RtpTcpSink::RtpTcpSink(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::system::error_code & ec)
            : RtpSink<boost::asio::ip::tcp::socket>(rtsp_socket)
        {
            ec.clear();
        }

        RtpTcpSink::~RtpTcpSink()
        {
        }

    } // namespace rtspd
} // namespace ppbox
