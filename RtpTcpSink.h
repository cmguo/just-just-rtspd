// RtpTcpSink.h

#ifndef _PPBOX_RTSPD_RTP_TCP_SINK_H_
#define _PPBOX_RTSPD_RTP_TCP_SINK_H_

#include <util/stream/TcpSocket.h>

namespace ppbox
{
    namespace rtspd
    {

        class RtpTcpSink
            : public util::stream::TcpSocket
        {
        public:
            RtpTcpSink(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::system::error_code & ec);

            virtual ~RtpTcpSink();
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_TCP_SINK_H_