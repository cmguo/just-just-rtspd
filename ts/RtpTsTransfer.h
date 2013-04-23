// RtpTsTransfer.h

#ifndef _PPBOX_RTSPD_TS_TRANSFER_H_
#define _PPBOX_RTSPD_TS_TRANSFER_H_

#include "ppbox/rtspd/RtpTransfer.h"

namespace ppbox
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
} // namespace ppbox

#endif // _PPBOX_RTSPD_TS_TRANSFER_H_
