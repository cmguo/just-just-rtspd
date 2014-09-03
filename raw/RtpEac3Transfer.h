// RtpEac3Transfer.h

#ifndef _PPBOX_RTSPD_RTP_EAC3_TRANSFER_H_
#define _PPBOX_RTSPD_RTP_EAC3_TRANSFER_H_

#include "ppbox/rtspd/RtpTransfer.h"

namespace ppbox
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

        PPBOX_REGISTER_RTP_TRANSFER("eac3", RtpEac3Transfer);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_EAC3_TRANSFER_H_
