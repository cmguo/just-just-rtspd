// RtpRawMuxer.h

#ifndef _PPBOX_RTSPD_RTP_RAW_MUXER_H_
#define _PPBOX_RTSPD_RTP_RAW_MUXER_H_

#include "ppbox/rtspd/RtpMuxer.h"

namespace ppbox
{
    namespace rtspd
    {

        class RtpRawMuxer
            : public RtpMuxer
        {
        public:
            RtpRawMuxer();

            ~RtpRawMuxer();

        private:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);
        };

        PPBOX_REGISTER_MUXER("rtp-raw", RtpRawMuxer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_RAW_MUXER_H_
