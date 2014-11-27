// RtpTransfer.h

#ifndef _JUST_RTSPD_RTP_TRANSFER_H_
#define _JUST_RTSPD_RTP_TRANSFER_H_

#include "just/rtspd/RtpPacket.h"

#include <just/mux/Transfer.h>

#include <util/tools/ClassFactory.h>

#include <framework/system/BytesOrder.h>
#include <framework/system/ScaleTransform.h>

namespace just
{
    namespace rtspd
    {

        using just::mux::StreamInfo;
        using just::mux::Sample;
        using just::mux::MuxEvent;

        class RtpTransfer
            : public just::mux::Transfer
        {
        public:
            RtpTransfer(
                char const * const name, 
                char const * const format, 
                boost::uint8_t type);

            virtual ~RtpTransfer();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void on_event(
                MuxEvent const & event);

            virtual void setup();

        public:
            RtpInfo const & rtp_info() const
            {
                return rtp_info_;
            }

        protected:
            void begin(
                Sample & sample);

            void finish(
                Sample & sample);

        protected:
            void begin_packet(
                bool mark, 
                boost::uint64_t time,
                boost::uint32_t size);

            template <typename ConstBuffers>
            void push_buffers(
                ConstBuffers const & buffers1)
            {
                buffers_.insert(buffers_.end(), buffers1.begin(), buffers1.end());
            }

            template <typename ConstBuffersIterator>
            void push_buffers(
                ConstBuffersIterator const & beg, 
                ConstBuffersIterator const & end)
            {
                buffers_.insert(buffers_.end(), beg, end);
            }

            void finish_packet();

        protected:
            void push_rtcp_packet();

        protected:
            boost::uint32_t time_scale_;

        protected:
            char const * const name_;
            char const * const format_;
            RtpHead rtp_head_;
            RtpInfo rtp_info_;
            std::vector<RtpPacket> packets_;
            size_t total_size_;
            std::deque<boost::asio::const_buffer> buffers_;
            // for rtcp
            boost::uint32_t rtcp_interval_;
            boost::uint32_t num_pkt_;
            boost::uint32_t num_byte_;
            boost::uint32_t next_time_;
            boost::uint64_t time_ms_;
            boost::uint64_t timestamp_;
            boost::posix_time::time_duration time_start_from_1900_;
            boost::uint8_t rtcp_buffer_[64];
        };

        struct RtpTransferTraits
            : util::tools::ClassFactoryTraits
        {
            typedef std::string key_type;
            typedef RtpTransfer * (create_proto)();

            static boost::system::error_code error_not_found();
        };

        typedef util::tools::ClassFactory<RtpTransferTraits> RtpTransferFactory;

    } // namespace rtspd
} // namespace just

#define JUST_REGISTER_RTP_TRANSFER(key, cls) UTIL_REGISTER_CLASS(just::rtspd::RtpTransferFactory, key, cls)

#endif // _JUST_RTSPD_RTP_TRANSFER_H_
