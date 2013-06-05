// RtpFormat.h

#ifndef _PPBOX_RTSPD_RAW_RTP_FORMAT_H_
#define _PPBOX_RTSPD_RAW_RTP_FORMAT_H_

#include <ppbox/avformat/Format.h>

namespace ppbox
{
    namespace rtspd
    {

        class RtpFormat
            : public ppbox::avformat::Format
        {
        public:
            RtpFormat();

        private:
            static ppbox::avformat::CodecInfo const codecs_[];
        };

        PPBOX_REGISTER_FORMAT("rtp-raw", RtpFormat);

    } // namespace rtspd
} // namespace ppbox


#endif // _PPBOX_RTSPD_RAW_RTP_FORMAT_H_
