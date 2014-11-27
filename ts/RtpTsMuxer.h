// RtpTsMuxer.h

#ifndef _JUST_RTSPD_RTP_TS_MUXER_H_
#define _JUST_RTSPD_RTP_TS_MUXER_H_

#include "just/rtspd/RtpMuxer.h"

#include <just/mux/mp2/TsMuxer.h>

namespace just
{
    namespace rtspd
    {

        class RtpTsTransfer;

        class RtpTsMuxer
            : public RtpMuxer
        {
        public:
            RtpTsMuxer(
                boost::asio::io_service & io_svc);

            ~RtpTsMuxer();

        private:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            void file_header(
                Sample & sample);

        private:
            just::mux::TsMuxer ts_mux_;
            RtpTsTransfer * rtp_ts_transfer_;
        };

        JUST_REGISTER_MUXER("rtp-ts", RtpTsMuxer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_TS_MUXER_H_
