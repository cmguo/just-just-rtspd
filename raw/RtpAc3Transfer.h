// RtpAc3Transfer.h

#ifndef _JUST_RTSPD_RTP_AC3_TRANSFER_H_
#define _JUST_RTSPD_RTP_AC3_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpAc3Transfer
            : public RtpTransfer
        {
        public:
            RtpAc3Transfer();

            ~RtpAc3Transfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint32_t mtu_size_;
            boost::uint8_t header_[8];
        };

        JUST_REGISTER_RTP_TRANSFER("ac3", RtpAc3Transfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_AC3_TRANSFER_H_
