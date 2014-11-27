// RtpTsTransfer.h

#ifndef _JUST_RTSPD_TS_TRANSFER_H_
#define _JUST_RTSPD_TS_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpTsTransfer
            : public RtpTransfer
        {
        public:
            RtpTsTransfer();

            ~RtpTsTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            void header_rtp_packet(
                Sample & tag);
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_TS_TRANSFER_H_
