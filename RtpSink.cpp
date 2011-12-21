// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpSink.h"
#include "ppbox/rtspd/Transport.h"

#include <ppbox/mux/rtp/RtpPacket.h>

#include <framework/system/BytesOrder.h>
using namespace framework::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("RtpSink", 0)

namespace ppbox
{
    namespace rtspd
    {
        RtpSink::RtpSink()
            : num_pkt_(0)
            , num_byte_(0)
        {
            next_rtcp_time_ -= framework::timer::Duration::seconds(4);
        }

        RtpSink::~RtpSink()
        {
            if (transports_.first)
                delete transports_.first;
            if (transports_.second)
                delete transports_.second;
        }

        //工作线程调用
        boost::system::error_code RtpSink::write(
            boost::posix_time::ptime const & time_send, 
            ppbox::demux::Sample& tag)
        {
            assert(tag.context);
            boost::system::error_code ec;
            ppbox::mux::RtpSplitContent & packets = *(ppbox::mux::RtpSplitContent *)tag.context;
            for (size_t ii = 0; ii < packets.size(); ++ii)
            {
               ec =  transports_.first->send_packet(packets[ii].buffers);
               if(ec)
               {
                   std::cout<<" write failed "<<std::endl;
                   break;
               }
               ++num_pkt_;
               num_byte_ += packets[ii].size;
               framework::timer::Time now;
               if (next_rtcp_time_ < now) {
                   boost::posix_time::ptime time_play = 
                       time_send + boost::posix_time::microseconds(packets.ustime - tag.ustime);
                   send_rtcp(time_play, packets[ii]);
                   next_rtcp_time_ = now + framework::timer::Duration::seconds(3);
               }
            }
            return ec;
        }

        struct RtcpHead
        {
            //struct
            //{
            //    UInt8   sc : 5;
            //    UInt8   pad : 1;
            //    UInt8   ver : 2;
            //};
            boost::uint8_t pre;
            boost::uint8_t type;
            boost::uint16_t length;
        };

        struct RtcpSR
        {
            boost::uint32_t ssrc;
            boost::uint32_t ntph;
            boost::uint32_t ntpl;
            boost::uint32_t timestamp;
            boost::uint32_t packet;
            boost::uint32_t octet;
        };

        struct RtcpSDESItem
        {
            boost::uint8_t type;
            boost::uint8_t len;
            boost::uint8_t data[14];
        };

        void RtpSink::send_rtcp(
            boost::posix_time::ptime const & time_send, 
            ppbox::mux::RtpPacket const & rtp)
        {
            size_t length = sizeof(RtcpHead) + sizeof(RtcpSR) 
                    + sizeof(RtcpHead) + sizeof(boost::uint32_t) + sizeof(RtcpSDESItem);
            boost::uint8_t * buf = boost::asio::buffer_cast<boost::uint8_t *>(rtcp_buf_.prepare(length));

            RtcpHead * head = (RtcpHead *)buf;
            head->pre = 0x80; // ver = 2, pad = 0, sc = 0
            head->type = 200;
            head->length = BytesOrder::host_to_big_endian(boost::uint16_t(6));

            RtcpSR * sr = (RtcpSR *)(head + 1);
            sr->ssrc = rtp.ssrc;
            boost::posix_time::ptime t1900(boost::gregorian::date(1900, 1, 1));
            boost::posix_time::time_duration time_since_1900 = time_send - t1900;
            boost::uint32_t ntp_sec = time_since_1900.total_seconds();
            boost::uint32_t ntp_frac = (time_since_1900 - boost::posix_time::seconds(ntp_sec)).total_microseconds();
            ntp_frac = (boost::uint32_t)((boost::uint64_t)ntp_frac << 32 / 1000000);
            sr->ntph = BytesOrder::host_to_big_endian(ntp_sec);
            sr->ntpl = BytesOrder::host_to_big_endian(ntp_frac);
            sr->timestamp = rtp.timestamp;
            sr->packet = BytesOrder::host_to_big_endian(num_pkt_);
            sr->octet = BytesOrder::host_to_big_endian((boost::uint32_t)num_byte_);

            head = (RtcpHead *)(sr + 1);
            head->pre = 0x81; // ver = 2, pad = 0, sc = 1
            head->type = 202;
            head->length = BytesOrder::host_to_big_endian(boost::uint16_t(5));

            boost::uint32_t * identifier = (boost::uint32_t *)(head + 1);
            *identifier = rtp.ssrc;

            RtcpSDESItem * sdes = (RtcpSDESItem *)(identifier + 1);
            sdes->type = 1;
            sdes->len = 13;
            strcpy((char *)sdes->data, "PPBOX12345678");

            rtcp_buf_.commit(length);
            std::vector<boost::asio::const_buffer> vec(1, rtcp_buf_.data());
            transports_.second->send_packet(vec);
        }

        //下面为主线程调用
        boost::system::error_code RtpSink::setup(
            boost::asio::ip::tcp::socket * rtsp_sock, 
            std::string const & transport,
            std::string & output)
        {
            boost::system::error_code ec;
            transports_ = Transport::create(*rtsp_sock, transport, output, ec);
            return ec;
        }

    } // namespace rtspd
} // namespace ppbox
