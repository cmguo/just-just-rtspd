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
            {StreamType::VIDE,  (intptr_t)"H264",           VideoSubType::AVC1, StreamFormatType::none}, 
            {StreamType::AUDI,  (intptr_t)"mpeg4-generic",  AudioSubType::MP4A, AacFormatType::raw}, 
            {StreamType::AUDI,  (intptr_t)"mpa",            AudioSubType::MP1A, StreamFormatType::none}, 
        };

        RtpFormat::RtpFormat()
            : ppbox::avformat::Format(codecs_, sizeof(codecs_) / sizeof(codecs_[0]))
        {
        }

    } // namespace rtspd
} // namespace ppbox
