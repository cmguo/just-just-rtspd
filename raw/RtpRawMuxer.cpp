// RtpRawMuxer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/raw/RtpRawMuxer.h"

#include <util/tools/ClassRegister.h>

#include "just/rtspd/raw/RtpMpeg4GenericTransfer.h"
#include "just/rtspd/raw/RtpMpegAudioTransfer.h"
#include "just/rtspd/raw/RtpH264Transfer.h"
#include "just/rtspd/raw/RtpH265Transfer.h"
#include "just/rtspd/raw/RtpAc3Transfer.h"
#include "just/rtspd/raw/RtpEac3Transfer.h"

#include <just/avformat/rtp/RtpFormat.h>
using namespace just::avformat;

#include <just/avcodec/CodecType.h>
using namespace just::avcodec;

namespace just
{
    namespace rtspd
    {

        RtpRawMuxer::RtpRawMuxer(
            boost::asio::io_service & io_svc)
            : RtpMuxer(io_svc)
        {
        }

        RtpRawMuxer::~RtpRawMuxer()
        {
        }

        void RtpRawMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            RtpFormat rtp;
            boost::system::error_code ec;
            just::avformat::CodecInfo const * codec = rtp.codec_from_codec(info.type, info.sub_type, ec);
            if (codec) {
                RtpTransfer * rtp_transfer = RtpTransferFactory::create((char const *)codec->context, ec);
                if (rtp_transfer) {
                    pipe.insert(rtp_transfer);
                    add_rtp_transfer(rtp_transfer);
                } else {
                    add_rtp_transfer(NULL);
                }
            } else {
                add_rtp_transfer(NULL);
            }
        }

    } // namespace rtspd
} // namespace just
