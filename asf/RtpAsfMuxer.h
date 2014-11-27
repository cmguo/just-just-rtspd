// RtpAsfMuxer.h

#ifndef _JUST_RTSPD_RTP_ASF_MUXER_H_
#define _JUST_RTSPD_RTP_ASF_MUXER_H_

#include "just/rtspd/RtpMuxer.h"

#include <just/mux/asf/AsfMuxer.h>

namespace just
{
    namespace rtspd
    {

        class RtpAsfTransfer;

        class RtpAsfMuxer
            : public RtpMuxer
        {
        public:
            RtpAsfMuxer(
                boost::asio::io_service & io_svc);

            ~RtpAsfMuxer();

        public:
            virtual void media_info(
                MediaInfo & info) const;

        private:
            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

        private:
            just::mux::AsfMuxer asf_mux_;
            RtpAsfTransfer * rtp_asf_transfer_;
        };

        JUST_REGISTER_MUXER("rtp-asf", RtpAsfMuxer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_ASF_MUXER_H_
