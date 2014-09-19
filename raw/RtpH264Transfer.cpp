// RtpH264Transfer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/raw/RtpH264Transfer.h"

#include <ppbox/avcodec/avc/AvcConfigHelper.h>
#include <ppbox/avcodec/avc/AvcConfig.h>
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
         * RFC 3984: RTP Payload Format for H.264 Video
         * RFC 6184: RTP Payload Format for H.264 Video
         */

        RtpH264Transfer::RtpH264Transfer()
            : RtpTransfer("RtpH264", "H264", 96)
            , mtu_size_(1436)
            , sps_pps_sent_(false)
            , use_dts_(false)
        {
        }

        RtpH264Transfer::~RtpH264Transfer()
        {
        }

        void RtpH264Transfer::config(
            framework::configure::Config & conf)
        {
            RtpTransfer::config(conf);
            conf.register_module("RtpH264")
                << CONFIG_PARAM_NAME_RDWR("usedts", use_dts_);
        }

        void RtpH264Transfer::transfer(
            StreamInfo & info)
        {
            AvcConfigHelper const & config = *(AvcConfigHelper *)info.context;
            std::vector<boost::uint8_t> sps_data = config.data().sequenceParameterSetNALUnit[0];
            std::vector<boost::uint8_t> pps_data = config.data().pictureParameterSetNALUnit[0];

            boost::uint8_t const * profile_level_id = &sps_data.front() + 1;

            using namespace framework::string;
            std::string profile_level_id_str = 
                Base16::encode(std::string((char const *)profile_level_id, 3));
            std::string sps = Base64::encode(&sps_data.front(), sps_data.size());
            std::string pps = Base64::encode(&pps_data.front(), pps_data.size());

            std::ostringstream oss;
            int map_id = rtp_head_.mpt;
            oss << "a=framesize:" << map_id
                << " " << info.video_format.width << "-" << info.video_format.height
                << "\r\n";
            oss << "a=cliprect:0,0," 
                << info.video_format.height << "," << info.video_format.width
                << "\r\n";
            oss << "a=fmtp:" << map_id
                << " packetization-mode=1" 
                << ";profile-level-id=" << profile_level_id_str
                << ";sprop-parameter-sets=" << sps << "," << pps
                << "\r\n";
            rtp_info_.sdp += oss.str();

            RtpTransfer::transfer(info);
        }

        void RtpH264Transfer::transfer(
            Sample & sample)
        {
            boost::uint64_t rtp_time = use_dts_ ? sample.dts : sample.dts + sample.cts_delta;

            RtpTransfer::begin(sample);

            // add two sps pps rtp packet
            if (!sps_pps_sent_) {
                StreamInfo const & info = *(StreamInfo const *)sample.stream_info;

                sps_pps_sent_ = true;
                // info.context is AvcConfigHelper, set by AvcPacketSplitter or AvcByteStreamSplitter
                AvcConfigHelper const & config = *(AvcConfigHelper *)info.context;

                for (size_t i = 0; i < config.data().sequenceParameterSetNALUnit.size(); ++i) {
                    begin_packet(false, rtp_time, config.data().sequenceParameterSetNALUnit[i].size());
                    push_buffers(boost::asio::buffer(config.data().sequenceParameterSetNALUnit[i]));
                    finish_packet();
                }

                for (size_t i = 0; i < config.data().pictureParameterSetNALUnit.size(); ++i) {
                    begin_packet(false, rtp_time, config.data().pictureParameterSetNALUnit[i].size());
                    push_buffers(boost::asio::buffer(config.data().pictureParameterSetNALUnit[i]));
                    finish_packet();
                }
            }

            // sample.context is NaluHelper, set by AvcPacketSplitter or AvcByteStreamSplitter
            NaluHelper & helper = *(NaluHelper *)sample.context;
            std::vector<NaluBuffer> & nalus = helper.nalus();
            for (size_t i = 0; i < nalus.size(); ++i) {
                NaluBuffer & nalu = nalus[i];
                size_t l = nalu.size;
                if (l > (mtu_size_)) {
                    boost::uint8_t b = nalu.begin.dereference_byte();
                    prefix_[0][0] = (b & 0xe0) | 28;
                    prefix_[0][1] = (b & 0x1f) | 0x80;
                    nalu.begin.increment_byte(nalu.end);
                    --l;
                    NaluBuffer::BuffersPosition pos = nalu.begin;
                    nalu.begin.increment_bytes(nalu.end, mtu_size_ - 2);
                    begin_packet(false, rtp_time, mtu_size_);
                    push_buffers(boost::asio::buffer(prefix_[0], 2));
                    push_buffers(NaluBuffer::range_buffers(pos, nalu.begin));
                    finish_packet();
                    l -= mtu_size_ - 2;
                    prefix_[1][0] = prefix_[0][0];
                    prefix_[1][1] = prefix_[0][1] & 0x7f;
                    while (l > mtu_size_ - 2) {
                        NaluBuffer::BuffersPosition pos1 = nalu.begin;
                        nalu.begin.increment_bytes(nalu.end, mtu_size_ - 2);
                        begin_packet(false, rtp_time, mtu_size_);
                        push_buffers(boost::asio::buffer(prefix_[1], 2));
                        push_buffers(NaluBuffer::range_buffers(pos1, nalu.begin));
                        finish_packet();
                        l -= mtu_size_ - 2;
                    };
                    prefix_[2][0] = prefix_[1][0];
                    prefix_[2][1] = prefix_[1][1] | 0x40;
                    begin_packet(i == nalus.size() - 1, rtp_time, l + 2);
                    push_buffers(boost::asio::buffer(prefix_[2], 2));
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

        void RtpH264Transfer::on_event(
            MuxEvent const & event)
        {
            RtpTransfer::on_event(event);
            if (event.type == event.before_reset) {
                sps_pps_sent_ = false;
            }
        }

    } // namespace rtspd
} // namespace ppbox
