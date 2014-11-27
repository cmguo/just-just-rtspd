// RtpRawMuxer.h

#ifndef _JUST_RTSPD_RTP_RAW_MUXER_H_
#define _JUST_RTSPD_RTP_RAW_MUXER_H_

#include "just/rtspd/RtpMuxer.h"

namespace just
{
    namespace rtspd
    {

        class RtpRawMuxer
            : public RtpMuxer
        {
        public:
            RtpRawMuxer(
                boost::asio::io_service & io_svc);

            ~RtpRawMuxer();

        private:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);
        };

        JUST_REGISTER_MUXER("rtp-raw", RtpRawMuxer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_RAW_MUXER_H_
