// RtpAc3Transfer.h

#ifndef _PPBOX_RTSPD_RTP_AC3_TRANSFER_H_
#define _PPBOX_RTSPD_RTP_AC3_TRANSFER_H_

#include "ppbox/rtspd/RtpTransfer.h"

namespace ppbox
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

        PPBOX_REGISTER_RTP_TRANSFER("ac3", RtpAc3Transfer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_AC3_TRANSFER_H_
