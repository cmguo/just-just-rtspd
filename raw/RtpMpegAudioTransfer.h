// RtpMpegAudioTransfer.h

#ifndef _JUST_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_
#define _JUST_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
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

        JUST_REGISTER_RTP_TRANSFER("MPA", RtpMpegAudioTransfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_MPEG_AUDIO_TRANSFER_H_
