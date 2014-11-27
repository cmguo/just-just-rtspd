// RtpAsfTransfer.h

#ifndef _JUST_RTSPD_RTP_ASF_TRANSFER_H_
#define _JUST_RTSPD_RTP_ASF_TRANSFER_H_

#include "just/rtspd/RtpTransfer.h"

namespace just
{
    namespace rtspd
    {

        class RtpAsfTransfer
            : public RtpTransfer
        {
        public:
            RtpAsfTransfer();

            ~RtpAsfTransfer();

        public:
            virtual void transfer(
                Sample & sample);

            virtual void transfer(
                StreamInfo & info);

        public:
            void get_sdp(
                Sample const & tag, 
                std::string & sdp);

        private:
            boost::uint8_t header_[2][4];
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_ASF_TRANSFER_H_
