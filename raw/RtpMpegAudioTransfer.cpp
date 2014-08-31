// RtpMpegAudioTransfer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpMpegAudioTransfer.h"

namespace ppbox
{
    namespace rtspd
    {

        RtpMpegAudioTransfer::RtpMpegAudioTransfer()
            : RtpTransfer("RtpAudioMpegTransfer", "mpa", 97)
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

            begin_packet(true, sample.dts, 4 + sample.size);
            push_buffers(boost::asio::buffer(header_, 4));
            push_buffers(sample.data);
            finish_packet();

            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace ppbox
