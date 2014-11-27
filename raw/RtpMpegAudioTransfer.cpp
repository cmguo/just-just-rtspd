// RtpMpegAudioTransfer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/raw/RtpMpegAudioTransfer.h"

namespace just
{
    namespace rtspd
    {
        /*
         * RFC 2038: RTP Payload Format for MPEG1/MPEG2 Video
         * RFC 2250: RTP Payload Format for MPEG1/MPEG2 Video
         * RFC 3551: RTP Profile for Audio and Video Conferences with Minimal Control
         * RFC 3555: MIME Type Registration of RTP Payload Formats
         */

        RtpMpegAudioTransfer::RtpMpegAudioTransfer()
            : RtpTransfer("RtpAudioMpeg", "MPA", 97)
        {
            header_[0] = 0;
            header_[1] = 0;
        }

        RtpMpegAudioTransfer::~RtpMpegAudioTransfer()
        {
        }

        void RtpMpegAudioTransfer::transfer(
            StreamInfo & info)
        {
            RtpTransfer::transfer(info);
        }

        void RtpMpegAudioTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::begin(sample);

            // No fragmentation.
            begin_packet(true, sample.dts, 4 + sample.size);
            push_buffers(boost::asio::buffer(header_, 4));
            push_buffers(sample.data);
            finish_packet();

            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace just
