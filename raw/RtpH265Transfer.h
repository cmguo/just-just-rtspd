// RtpH265Transfer.h

#ifndef _JUST_RTSPD_RTP_H265_TRANSFER_H_
#define _JUST_RTSPD_RTP_H265_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpH265Transfer
            : public RtpTransfer
        {
        public:
            RtpH265Transfer();

            ~RtpH265Transfer();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_event(
                MuxEvent const & event);

        private:
            boost::uint32_t mtu_size_;
            boost::uint8_t prefix_[3][3];
            bool sps_pps_sent_;
            bool use_dts_;
        };

        JUST_REGISTER_RTP_TRANSFER("H265", RtpH265Transfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_H265_TRANSFER_H_
