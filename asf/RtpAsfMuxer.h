// RtpAsfMuxer.h

#ifndef _PPBOX_RTSPD_RTP_ASF_MUXER_H_
#define _PPBOX_RTSPD_RTP_ASF_MUXER_H_

#include "ppbox/rtspd/RtpMuxer.h"

#include <ppbox/mux/asf/AsfMuxer.h>

namespace ppbox
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
            ppbox::mux::AsfMuxer asf_mux_;
            RtpAsfTransfer * rtp_asf_transfer_;
        };

        PPBOX_REGISTER_MUXER("rtp-asf", RtpAsfMuxer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_ASF_MUXER_H_
