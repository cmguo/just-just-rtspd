// RtpMpeg4GenericTransfer.h

#ifndef _JUST_RTSPD_RTP_MPEG4_GENERIC_TRANSFER_H_
#define _JUST_RTSPD_RTP_MPEG4_GENERIC_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpMpeg4GenericTransfer
            : public RtpTransfer
        {
        public:
            RtpMpeg4GenericTransfer();

            ~RtpMpeg4GenericTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint8_t index_;
            boost::uint8_t au_header_section_[4];
            std::vector<boost::uint8_t> packat_header_;
        };

        JUST_REGISTER_RTP_TRANSFER("mpeg4-generic", RtpMpeg4GenericTransfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_MPEG4_GENERIC_TRANSFER_H_
