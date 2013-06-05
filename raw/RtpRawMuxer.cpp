// RtpRawMuxer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpRawMuxer.h"
#include "ppbox/rtspd/raw/RtpMpeg4GenericTransfer.h"
#include "ppbox/rtspd/raw/RtpMpegAudioTransfer.h"
#include "ppbox/rtspd/raw/RtpH264Transfer.h"

#include <ppbox/avcodec/CodecType.h>
using namespace ppbox::avcodec;

namespace ppbox
{
    namespace rtspd
    {

        RtpRawMuxer::RtpRawMuxer()
        {
        }

        RtpRawMuxer::~RtpRawMuxer()
        {
        }

        void RtpRawMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            if (info.type == StreamType::VIDE) {
                if (info.sub_type == VideoSubType::AVC1) {
                    RtpTransfer * rtp_transfer = new RtpH264Transfer;
                    pipe.push_back(rtp_transfer);
                    add_rtp_transfer(rtp_transfer);
                }
            } else if (StreamType::AUDI == info.type){
                RtpTransfer * rtp_transfer = NULL;
                if (info.sub_type == AudioSubType::MP1A) {
                    rtp_transfer = new RtpMpegAudioTransfer;
                } else if (info.sub_type == AudioSubType::MP4A) {
                    rtp_transfer = new RtpMpeg4GenericTransfer;
                }
                pipe.push_back(rtp_transfer);
                add_rtp_transfer(rtp_transfer);
            } else {
                add_rtp_transfer(NULL);
            }
        }

    } // namespace rtspd
} // namespace ppbox
