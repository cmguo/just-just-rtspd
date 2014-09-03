// RtpMpegAudioTransfer.h

#ifndef _PPBOX_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_
#define _PPBOX_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_

#include "ppbox/rtspd/RtpTransfer.h"

namespace ppbox
{
    namespace rtspd
    {

        class RtpMpegAudioTransfer
            : public RtpTransfer
        {
        public:
            RtpMpegAudioTransfer();

            ~RtpMpegAudioTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint16_t header_[2];
        };

        PPBOX_REGISTER_RTP_TRANSFER("MPA", RtpMpegAudioTransfer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_
