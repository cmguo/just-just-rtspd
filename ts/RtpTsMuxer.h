// RtpTsMuxer.h

#ifndef _PPBOX_RTSPD_RTP_TS_MUXER_H_
#define _PPBOX_RTSPD_RTP_TS_MUXER_H_

#include "ppbox/rtspd/RtpMuxer.h"

#include <ppbox/mux/ts/TsMuxer.h>

namespace ppbox
{
    namespace rtspd
    {

        class RtpTsTransfer;

        class RtpTsMuxer
            : public RtpMuxer
        {
        public:
            RtpTsMuxer();

            ~RtpTsMuxer();

        private:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            void file_header(
                Sample & sample);

        private:
            ppbox::mux::TsMuxer ts_mux_;
            RtpTsTransfer * rtp_ts_transfer_;
        };

        PPBOX_REGISTER_MUXER("rtp-ts", RtpTsMuxer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_TS_MUXER_H_
