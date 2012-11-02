// RtpSink.h

#ifndef _PPBOX_RTSPD_RTP_SINK_H_
#define _PPBOX_RTSPD_RTP_SINK_H_

#include <ppbox/mux/rtp/RtpPacket.h>

#include <util/stream/Socket.h>

namespace ppbox
{
    namespace rtspd
    {

        template <typename SocketType>
        class RtpSink
            : public util::stream::Socket<SocketType>
        {
        protected:
            RtpSink(
                SocketType & socket)
                : util::stream::Socket<SocketType>(socket)
            {
            }

        private:
            virtual size_t private_write_some(
                util::stream::StreamConstBuffers const & buffers, 
                boost::system::error_code & ec)
            {
                ppbox::mux::RtpPacket const * packets = 
                    boost::asio::buffer_cast<ppbox::mux::RtpPacket const *>(*buffers.begin());
                size_t num = 
                    boost::asio::buffer_size(*buffers.begin());
                for (size_t i = 0; i < num; ++i) {
                    util::stream::Socket<SocketType>::private_write_some(packets[i].buffers, ec);
                    if (ec) {
                        return i;
                    }
                }
                return num;
            }
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
