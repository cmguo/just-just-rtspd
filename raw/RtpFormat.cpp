// RtpFormat.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpFormat.h"

#include <ppbox/avcodec/avc/AvcFormatType.h>
#include <ppbox/avcodec/aac/AacFormatType.h>

using namespace ppbox::avcodec;

namespace ppbox
{
    namespace rtspd
    {

        // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml

        ppbox::avformat::CodecInfo const RtpFormat::codecs_[] = {
            {StreamType::VIDE,  (intptr_t)"H264",           VideoSubType::AVC,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"mpeg4-generic",  AudioSubType::AAC,  AacFormatType::raw,     1}, 
            {StreamType::AUDI,  (intptr_t)"MPA",            AudioSubType::MP1A, StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"MPA",            AudioSubType::MP2A, StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"MPA",            AudioSubType::MP1,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"MPA",            AudioSubType::MP2,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"MPA",            AudioSubType::MP3,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"ac3",            AudioSubType::AC3,  StreamFormatType::none, 1}, 
        };

        RtpFormat::RtpFormat()
            : ppbox::avformat::Format(codecs_, sizeof(codecs_) / sizeof(codecs_[0]))
        {
        }

    } // namespace rtspd
} // namespace ppbox
