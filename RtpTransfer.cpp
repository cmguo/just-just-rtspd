// RtpTransfer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/RtpTransfer.h"
#include "just/rtspd/RtcpPacket.h"

#include <just/avformat/Error.h>

using namespace just::mux;

namespace just
{
    namespace rtspd
    {

        RtpTransfer::RtpTransfer(
            char const * const name, 
            char const * const format, 
            boost::uint8_t type)
            : time_scale_(0)
            , name_(name)
            , format_(format)
            , total_size_(0)
            , rtcp_interval_(3000)
            , num_pkt_(0)
            , num_byte_(0)
            , next_time_(0)
        {
            static size_t g_ssrc = rand();

            rtp_head_.vpxcc = 0x80;
            rtp_head_.mpt = type;
            rtp_head_.sequence = rand();
            rtp_head_.timestamp = rand();
            rtp_head_.ssrc = framework::system::BytesOrder::host_to_big_endian(g_ssrc++);

            rtp_info_.stream_index = (boost::uint32_t)-1;
            rtp_info_.timestamp = rtp_head_.timestamp;
            rtp_info_.seek_time = 0;
            rtp_info_.ssrc = rtp_head_.ssrc;
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.setup = false;
        }

        RtpTransfer::~RtpTransfer()
        {
        }

        void RtpTransfer::config(
            framework::configure::Config & conf)
        {
            conf.register_module(name_)
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc)
                << CONFIG_PARAM_NAME_RDWR("rtcp_interval", rtcp_interval_);
        }

        void RtpTransfer::setup()
        {
            rtp_info_.setup = true;
        }

        void RtpTransfer::transfer(
            StreamInfo & info)
        {
            time_scale_ = info.time_scale;

            using namespace framework::string;

            std::ostringstream oss;
            int map_id = rtp_head_.mpt;
            if (info.type == StreamType::VIDE) {
                oss << "m=video 0 RTP/AVP " << map_id << "\r\n";
                oss << "a=rtpmap:" << map_id << " " << format_ << "/" << time_scale_ << "\r\n";
            } else {
                oss << "m=audio 0 RTP/AVP " << map_id << "\r\n";
                oss << "a=rtpmap:" << map_id << " " << format_ << "/" << time_scale_ 
                    << "/" << info.audio_format.channel_count
                    << "\r\n";
            }
            oss << rtp_info_.sdp; // from child class
            oss << "a=control:track" << info.index << "\r\n";
            rtp_info_.sdp = oss.str();

            rtp_info_.stream_index = info.index;
        }

        void RtpTransfer::on_event(
            MuxEvent const & event)
        {
            if (event.type != event.after_seek)
                return;
            boost::uint64_t time = event.time;
            if (!packets_.empty()) {
                boost::uint32_t last_timestamp = 
                    framework::system::BytesOrder::host_to_big_endian(packets_[0].timestamp);
                // add 8 seconds to keep distance from timestamp before
                rtp_info_.timestamp = last_timestamp + time_scale_ * 8;
            }
            rtp_head_.timestamp = rtp_info_.timestamp 
                - (boost::uint32_t)framework::system::ScaleTransform::static_transfer(1000, time_scale_, time);
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.seek_time = (boost::uint32_t)time;

            boost::posix_time::ptime t1900(boost::gregorian::date(1900, 1, 1));
            time_start_from_1900_ = boost::posix_time::microsec_clock::universal_time() - boost::posix_time::milliseconds(time) - t1900;
            if (rtcp_interval_) {
                next_time_ = time;
            }
        }

        void RtpTransfer::begin(
            Sample & sample)
        {
            packets_.clear();
            total_size_ = 0;
            buffers_.clear();

            time_ms_ = sample.time;
            timestamp_ = sample.dts + sample.cts_delta;

            buffers_.push_back(boost::asio::buffer(&packets_, sizeof(packets_)));
            total_size_ += sizeof(packets_);
        }

        void RtpTransfer::begin_packet(
            bool mark, 
            boost::uint64_t time,
            boost::uint32_t size)
        {
            RtpPacket packet;
            packet.vpxcc = rtp_head_.vpxcc;
            packet.mpt = (mark ? 0x80 : 0) | rtp_head_.mpt;
            packet.sequence = framework::system::BytesOrder::host_to_big_endian(rtp_head_.sequence++);
            packet.timestamp = framework::system::BytesOrder::host_to_big_endian(rtp_head_.timestamp + (boost::uint32_t)time);
            packet.ssrc = rtp_head_.ssrc;

            packet.size = (sizeof(RtpHead) + size);
            packet.buf_beg = buffers_.size();
            total_size_ += packet.size;

            ++num_pkt_;
            num_byte_ += size;

            packets_.push_back(packet);
            buffers_.push_back(boost::asio::buffer(&packets_.back(), sizeof(RtpHead))); // ������ܻ�Ҫ���µ���
        }

        void RtpTransfer::finish_packet()
        {
            RtpPacket & packet = packets_.back();
            packet.buf_end = buffers_.size();

            if (rtcp_interval_ && time_ms_ >= next_time_) {
                push_rtcp_packet();
                next_time_ += rtcp_interval_;
            }
        }

        void RtpTransfer::finish(
            Sample & sample)
        {
            for (size_t i = 0; i < packets_.size(); ++i) {
                if (packets_[i].mpt == 0) { // RTCP
                    continue;
                }
                if (boost::asio::buffer_cast<void const *>(buffers_[packets_[i].buf_beg]) == &packets_[i]) {
                    break;
                }
                buffers_[packets_[i].buf_beg] = boost::asio::buffer(&packets_[i], sizeof(RtpHead));
            }
            sample.size = total_size_;
            sample.data.swap(buffers_);
            sample.context = &packets_;
        }

        void RtpTransfer::push_rtcp_packet()
        {
            size_t length = sizeof(RtcpHead) + sizeof(RtcpSR) 
                + sizeof(RtcpHead) + sizeof(boost::uint32_t) + sizeof(RtcpSDESItem);

            RtcpHead * head = (RtcpHead *)rtcp_buffer_;
            head->pre = 0x80; // ver = 2, pad = 0, sc = 0
            head->type = 200;
            head->length = framework::system::BytesOrder::host_to_big_endian(boost::uint16_t(6));

            RtcpSR * sr = (RtcpSR *)(head + 1);
            sr->ssrc = rtp_head_.ssrc;
            boost::posix_time::time_duration time_since_1900 = time_start_from_1900_ + boost::posix_time::milliseconds(time_ms_);
            boost::uint32_t ntp_sec = time_since_1900.total_seconds();
            boost::uint32_t ntp_frac = (time_since_1900 - boost::posix_time::seconds(ntp_sec)).total_microseconds();
            ntp_frac = (boost::uint32_t)(((boost::uint64_t)ntp_frac << 32) / 1000000);
            sr->ntph = framework::system::BytesOrder::host_to_big_endian(ntp_sec);
            sr->ntpl = framework::system::BytesOrder::host_to_big_endian(ntp_frac);
            sr->timestamp = framework::system::BytesOrder::host_to_big_endian(rtp_head_.timestamp + (boost::uint32_t)timestamp_);
            sr->packet = framework::system::BytesOrder::host_to_big_endian(num_pkt_);
            sr->octet = framework::system::BytesOrder::host_to_big_endian((boost::uint32_t)num_byte_);

            head = (RtcpHead *)(sr + 1);
            head->pre = 0x81; // ver = 2, pad = 0, sc = 1
            head->type = 202;
            head->length = framework::system::BytesOrder::host_to_big_endian(boost::uint16_t(5));

            boost::uint32_t * identifier = (boost::uint32_t *)(head + 1);
            *identifier = rtp_head_.ssrc;

            RtcpSDESItem * sdes = (RtcpSDESItem *)(identifier + 1);
            sdes->type = 1;
            sdes->len = 13;
            strcpy((char *)sdes->data, "JUST12345678");

            RtpPacket packet;
            packet.vpxcc = 0;
            packet.mpt = 0;
            packet.sequence = 0;
            packet.timestamp = sr->timestamp;
            packet.ssrc = 0;
            packet.size = length;
            total_size_ += packet.size;
            packet.buf_beg = buffers_.size();
            buffers_.push_back(boost::asio::buffer(rtcp_buffer_, length));
            packet.buf_end = buffers_.size();
            packets_.push_back(packet);
        }

        boost::system::error_code RtpTransferTraits::error_not_found()
        {
            return just::avformat::error::format_not_support;
        }

    } // namespace rtspd
} // namespace just
