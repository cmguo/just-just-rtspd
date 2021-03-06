// RtpH264Transfer.h

#ifndef _JUST_RTSPD_RTP_H264_TRANSFER_H_
#define _JUST_RTSPD_RTP_H264_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpH264Transfer
            : public RtpTransfer
        {
        public:
            RtpH264Transfer();

            ~RtpH264Transfer();

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
            boost::uint8_t prefix_[3][2];
            bool sps_pps_sent_;
            bool use_dts_;
        };

        JUST_REGISTER_RTP_TRANSFER("H264", RtpH264Transfer);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_H264_TRANSFER_H_
