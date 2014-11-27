// RtpEac3Transfer.h

#ifndef _JUST_RTSPD_RTP_EAC3_TRANSFER_H_
#define _JUST_RTSPD_RTP_EAC3_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpEac3Transfer
            : public RtpTransfer
        {
        public:
            RtpEac3Transfer();

            ~RtpEac3Transfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint32_t mtu_size_;
            boost::uint8_t header_[4];
        };

        JUST_REGISTER_RTP_TRANSFER("eac3", RtpEac3Transfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_EAC3_TRANSFER_H_
