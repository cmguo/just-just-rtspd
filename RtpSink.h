// RtpSink.h

#ifndef _PPBOX_RTSPD_RTP_SINK_H_
#define _PPBOX_RTSPD_RTP_SINK_H_

#include <ppbox/mux/rtp/RtpPacket.h>

#include <ppbox/dispatch/DispatchBase.h>
#include <ppbox/dispatch/Sink.h>

namespace ppbox
{
    namespace rtspd
    {

        template <typename SocketType>
        class RtpSink
            : public ppbox::dispatch::Sink
        {
        protected:
            RtpSink(
                SocketType & socket)
                : socket_(socket)
            {
            }

        private:
            virtual size_t write(
                ppbox::avformat::Sample const & sample, 
                boost::system::error_code & ec)
            {
                ppbox::mux::RtpSplitContent const & packets = 
                    *(ppbox::mux::RtpSplitContent const *)(sample.context);
                size_t total_size = 0;
                for (size_t i = 0; i < packets.size(); ++i) {
                    total_size += socket_.send(packets[i].buffers, 0, ec);
                    if (ec) {
                        break;;
                    }
                }
                return total_size;
            }

        private:
            SocketType & socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
