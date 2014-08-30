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

        ppbox::avformat::CodecInfo const RtpFormat::codecs_[] = {
            {StreamType::VIDE,  (intptr_t)"H264",           VideoSubType::AVC,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"mpeg4-generic",  AudioSubType::AAC,  AacFormatType::raw,     1}, 
            {StreamType::AUDI,  (intptr_t)"mpa",            AudioSubType::MP1A, StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"mpa",            AudioSubType::MP2A, StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"mpa",            AudioSubType::MP2,  StreamFormatType::none, 90000}, 
            {StreamType::AUDI,  (intptr_t)"mpa",            AudioSubType::MP3,  StreamFormatType::none, 90000}, 
        };

        RtpFormat::RtpFormat()
            : ppbox::avformat::Format(codecs_, sizeof(codecs_) / sizeof(codecs_[0]))
        {
        }

    } // namespace rtspd
} // namespace ppbox