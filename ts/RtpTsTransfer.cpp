// RtpTsTransfer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/ts/RtpTsTransfer.h"
#include "just/rtspd/RtpPacket.h"

using namespace just::mux;

#include <just/avformat/mp2/TsPacket.h>
using namespace just::avformat;

namespace just
{
    namespace rtspd
    {

        static boost::uint32_t const TS_PACKETS_PER_RTP_PACKET = 7;

        RtpTsTransfer::RtpTsTransfer()
            : RtpTransfer("RtpTs", "MP2T", 33)
        {
        }

        RtpTsTransfer::~RtpTsTransfer()
        {
        }

        void RtpTsTransfer::transfer(
            StreamInfo & info)
        {
            boost::uint32_t stream_index = info.index;
            info.index = boost::uint32_t(-1);
            if (info.type == StreamType::VIDE) {
                rtp_info_.sdp.clear();
                RtpTransfer::transfer(info);
            } else if (rtp_info_.sdp.empty()) {
                RtpTransfer::transfer(info);
            }
            info.index = stream_index;
        }

        void RtpTsTransfer::transfer(
            Sample & sample)
        {
            // Don't need adjust time scale, ts transfer already done it
            //RtpTransfer::transfer(sample);

            RtpTransfer::begin(sample);
            std::vector<size_t> const & off_segs = 
                *(std::vector<size_t> const *)sample.context;
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            boost::uint32_t i = TS_PACKETS_PER_RTP_PACKET;
            for (; i + 1 < off_segs.size(); i += TS_PACKETS_PER_RTP_PACKET) {
                // i + 1，这里+1是为了保证至少有一个RTP在后面生成，因为需要mark置为true
                buf_end = sample.data.begin() + off_segs[i];
                begin_packet(false, sample.dts + sample.cts_delta, TS_PACKETS_PER_RTP_PACKET * TsPacket::PACKET_SIZE);
                push_buffers(buf_beg, buf_end);
                finish_packet();
                buf_beg = buf_end;
            }
            i -= TS_PACKETS_PER_RTP_PACKET;
            buf_end = sample.data.end();
            begin_packet(true, sample.dts + sample.cts_delta, (off_segs.size() - i) * TsPacket::PACKET_SIZE);
            push_buffers(buf_beg, buf_end);
            finish_packet();
            RtpTransfer::finish(sample);
        }

        void RtpTsTransfer::header_rtp_packet(
            Sample & sample)
        {
            RtpTransfer::begin(sample);
            begin_packet(true, rtp_info_.timestamp, sample.size);
            push_buffers(sample.data);
            finish_packet();
            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace just
