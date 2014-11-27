// RtpEac3Transfer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/raw/RtpEac3Transfer.h"

#include <just/avbase/stream/SampleBuffers.h>
using namespace just::avbase;

namespace just
{
    namespace rtspd
    {

        /*
         * RFC 4598: Real-time Transport Protocol (RTP) Payload Format for Enhanced AC-3 (E-AC-3) Audio
         */

        RtpEac3Transfer::RtpEac3Transfer()
            : RtpTransfer("RtpEac3", "eac3", 100)
            , mtu_size_(1436)
        {
            memset(header_, sizeof(header_), 0);
            header_[1] = 1; // one frame
            header_[2] = 1;
        }

        RtpEac3Transfer::~RtpEac3Transfer()
        {
        }

        void RtpEac3Transfer::transfer(
            StreamInfo & info)
        {
            std::ostringstream oss;
            oss << "a=fmtp:" << (int)rtp_head_.mpt
                << " bitStreamConfig i2" 
                << "\r\n";
            rtp_info_.sdp += oss.str();
            RtpTransfer::transfer(info);
        }

        void RtpEac3Transfer::transfer(
            Sample & sample)
        {
            RtpTransfer::begin(sample);

            boost::uint32_t l = sample.size;
            boost::uint32_t mtu_size = mtu_size_ - 2;
            if (l <= mtu_size) {
                begin_packet(true, sample.dts, 2 + sample.size);
                push_buffers(boost::asio::buffer(header_, 2));
                push_buffers(sample.data);
                finish_packet();
            } else {
                header_[3] = (l + mtu_size - 1) / mtu_size;
                SampleBuffers::BuffersPosition position(sample.data.begin(), sample.data.end());
                SampleBuffers::BuffersPosition begin = position;
                SampleBuffers::BuffersPosition end(sample.data.end());
                while (l > mtu_size) {
                    SampleBuffers::BuffersPosition begin = position;
                    begin_packet(false, sample.dts, 2 + mtu_size);
                    push_buffers(boost::asio::buffer(header_ + 2, 2));
                    position.increment_bytes(end, mtu_size);
                    push_buffers(SampleBuffers::range_buffers(begin, position));
                    finish_packet();
                    l -= mtu_size;
                }
                // last seg
                begin_packet(true, sample.dts, 2 + l);
                push_buffers(boost::asio::buffer(header_ + 2, 2));
                push_buffers(SampleBuffers::range_buffers(position, end));
                finish_packet();
            }

            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace just
