// RtpH265Transfer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpH265Transfer.h"

#include <ppbox/avcodec/hevc/HevcConfigHelper.h>
#include <ppbox/avcodec/hevc/HevcConfig.h>
#include <ppbox/avcodec/hevc/HevcEnum.h>
#include <ppbox/avcodec/nalu/NaluHelper.h>
#include <ppbox/avcodec/nalu/NaluBuffer.h>
using namespace ppbox::avcodec;

#include <framework/string/Base16.h>
#include <framework/string/Base64.h>

namespace ppbox
{
    namespace rtspd
    {

        /*
         * RTP Payload Format for High Efficiency Video Coding
         * draft-ietf-payload-rtp-h265-06.txt
         */

        RtpH265Transfer::RtpH265Transfer()
            : RtpTransfer("RtpH265", "H265", 98)
            , mtu_size_(1436)
            , sps_pps_sent_(false)
            , use_dts_(false)
        {
        }

        RtpH265Transfer::~RtpH265Transfer()
        {
        }

        void RtpH265Transfer::config(
            framework::configure::Config & conf)
        {
            RtpTransfer::config(conf);
            conf.register_module("RtpH265")
                << CONFIG_PARAM_NAME_RDWR("usedts", use_dts_);
        }

        void RtpH265Transfer::transfer(
            StreamInfo & info)
        {
            HevcConfigHelper const & config = *(HevcConfigHelper *)info.context;

            //boost::uint8_t const * profile_level_id = &sps_data.front() + 1;

            using namespace framework::string;
            //std::string profile_level_id_str = 
            //    Base16::encode(std::string((char const *)profile_level_id, 3));

            std::ostringstream oss;
            int map_id = rtp_head_.mpt;
            oss << "a=fmtp:" << map_id
                << " tx-mode=SSM" 
                //<< ";profile-level-id=" << profile_level_id_str
                ;
            char T[] = "vsp";
            for (boost::uint8_t t = HevcNaluType::VPS_NUT; t <= HevcNaluType::PPS_NUT; ++t) {
                HevcConfigHelper::param_set_t const & ps = config.param_set(t);
                for (size_t i = 0; i < ps.size(); ++i) {
                    if (i == 0) {
                        oss << ";sprop-" << T[t - HevcNaluType::VPS_NUT] << "ps=";
                    } else {
                        oss << ',';
                    }
                    oss << Base64::encode(&ps[i].front(), ps[i].size());
                }
            }
            oss << "\r\n";
            rtp_info_.sdp += oss.str();

            RtpTransfer::transfer(info);
        }

        void RtpH265Transfer::transfer(
            Sample & sample)
        {
            boost::uint64_t rtp_time = use_dts_ ? sample.dts : sample.dts + sample.cts_delta;

            RtpTransfer::begin(sample);

            // add two sps pps rtp packet
            if (!sps_pps_sent_) {
                StreamInfo const & info = *(StreamInfo const *)sample.stream_info;

                sps_pps_sent_ = true;
                // info.context is HevcConfigHelper, set by HevcPacketSplitter or HevcByteStreamSplitter
                HevcConfigHelper const & config = *(HevcConfigHelper *)info.context;

                for (boost::uint8_t t = HevcNaluType::VPS_NUT; t <= HevcNaluType::PPS_NUT; ++t) {
                    HevcConfigHelper::param_set_t const & ps = config.param_set(t);
                    for (size_t i = 0; i < ps.size(); ++i) {
                        begin_packet(false, rtp_time, ps[i].size());
                        push_buffers(boost::asio::buffer(ps[i]));
                        finish_packet();
                    }
                }
            }

            // sample.context is NaluHelper, set by HevcPacketSplitter or HevcByteStreamSplitter
            NaluHelper & helper = *(NaluHelper *)sample.context;
            std::vector<NaluBuffer> & nalus = helper.nalus();
            for (size_t i = 0; i < nalus.size(); ++i) {
                NaluBuffer & nalu = nalus[i];
                size_t l = nalu.size;
                if (l > (mtu_size_)) {
                    size_t seg_size = mtu_size_ - 3;
                    boost::uint8_t b1 = nalu.begin.dereference_byte();
                    nalu.begin.increment_byte(nalu.end);
                    boost::uint8_t b2 = nalu.begin.dereference_byte();
                    nalu.begin.increment_byte(nalu.end);
                    boost::uint8_t type = (b1 >> 1) & 0x3f;
                    prefix_[0][0] = (b1 & 0x81) | (49 << 1);
                    prefix_[0][1] = b2;
                    prefix_[0][2] = 0x80 | type;
                    --l;--l;
                    NaluBuffer::BuffersPosition pos = nalu.begin;
                    nalu.begin.increment_bytes(nalu.end, seg_size);
                    begin_packet(false, rtp_time, mtu_size_);
                    push_buffers(boost::asio::buffer(prefix_[0], 3));
                    push_buffers(NaluBuffer::range_buffers(pos, nalu.begin));
                    finish_packet();
                    l -= seg_size;
                    prefix_[1][0] = prefix_[0][0];
                    prefix_[1][1] = prefix_[0][1];
                    prefix_[1][2] = type;
                    while (l > seg_size) {
                        NaluBuffer::BuffersPosition pos1 = nalu.begin;
                        nalu.begin.increment_bytes(nalu.end, seg_size);
                        begin_packet(false, rtp_time, mtu_size_);
                        push_buffers(boost::asio::buffer(prefix_[1], 3));
                        push_buffers(NaluBuffer::range_buffers(pos1, nalu.begin));
                        finish_packet();
                        l -= seg_size;
                    };
                    prefix_[2][0] = prefix_[1][0];
                    prefix_[2][1] = prefix_[1][1];
                    prefix_[2][2] = 0x40 | type;
                    begin_packet(i == nalus.size() - 1, rtp_time, l + 3);
                    push_buffers(boost::asio::buffer(prefix_[2], 3));
                    push_buffers(nalu.buffers());
                    finish_packet();
                } else {
                    begin_packet(i == nalus.size() - 1, rtp_time, l);
                    push_buffers(nalu.buffers());
                    finish_packet();
                }
            }
            RtpTransfer::finish(sample);
        }

        void RtpH265Transfer::on_event(
            MuxEvent const & event)
        {
            RtpTransfer::on_event(event);
            if (event.type == event.before_reset) {
                sps_pps_sent_ = false;
            }
        }

    } // namespace rtspd
} // namespace ppbox
