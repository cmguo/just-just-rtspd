// RtpAc3Transfer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpAc3Transfer.h"

#include <ppbox/avbase/stream/SampleBuffers.h>
using namespace ppbox::avbase;

namespace ppbox
{
    namespace rtspd
    {

        /*
         * RFC 4184: RTP Payload Format for AC-3 Audio
         */

        RtpAc3Transfer::RtpAc3Transfer()
            : RtpTransfer("RtpAc3", "ac3", 97)
            , mtu_size_(1436)
        {
            memset(header_, sizeof(header_), 0);
            header_[1] = 1; // one frame
            header_[2] = 1;
            header_[4] = 2;
            header_[6] = 3;
        }

        RtpAc3Transfer::~RtpAc3Transfer()
        {
        }

        void RtpAc3Transfer::transfer(
            StreamInfo & info)
        {
            RtpTransfer::transfer(info);
        }

        void RtpAc3Transfer::transfer(
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
                boost::uint8_t * header = header_;
                boost::uint8_t n = (l + mtu_size - 1) / mtu_size;
                if (l * 5 / 8 < mtu_size) {
                    header = header_ + 2;
                } else {
                    header = header_ + 4;
                }
                SampleBuffers::BuffersPosition position(sample.data.begin(), sample.data.end());
                SampleBuffers::BuffersPosition begin = position;
                SampleBuffers::BuffersPosition end(sample.data.end());
                // first seg
                header[1] = n;
                begin_packet(false, sample.dts, 2 + mtu_size);
                push_buffers(boost::asio::buffer(header, 2));
                position.increment_bytes(end, mtu_size);
                push_buffers(SampleBuffers::range_buffers(begin, position));
                finish_packet();
                l -= mtu_size;
                header_[7] = n;
                while (l > mtu_size) {
                    SampleBuffers::BuffersPosition begin = position;
                    begin_packet(false, sample.dts, 2 + mtu_size);
                    push_buffers(boost::asio::buffer(header_ + 6, 2));
                    position.increment_bytes(end, mtu_size);
                    push_buffers(SampleBuffers::range_buffers(begin, position));
                    finish_packet();
                    l -= mtu_size;
                }
                // last seg
                begin_packet(true, sample.dts, 2 + l);
                push_buffers(boost::asio::buffer(header_ + 6, 2));
                push_buffers(SampleBuffers::range_buffers(position, end));
                finish_packet();
            }

            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace ppbox
