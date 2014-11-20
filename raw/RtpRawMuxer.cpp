// RtpRawMuxer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpRawMuxer.h"

#include <util/tools/ClassRegister.h>

#include "ppbox/rtspd/raw/RtpMpeg4GenericTransfer.h"
#include "ppbox/rtspd/raw/RtpMpegAudioTransfer.h"
#include "ppbox/rtspd/raw/RtpH264Transfer.h"
#include "ppbox/rtspd/raw/RtpH265Transfer.h"
#include "ppbox/rtspd/raw/RtpAc3Transfer.h"
#include "ppbox/rtspd/raw/RtpEac3Transfer.h"

#include <ppbox/avformat/rtp/RtpFormat.h>
using namespace ppbox::avformat;

#include <ppbox/avcodec/CodecType.h>
using namespace ppbox::avcodec;

namespace ppbox
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
            ppbox::avformat::CodecInfo const * codec = rtp.codec_from_codec(info.type, info.sub_type, ec);
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
} // namespace ppbox
