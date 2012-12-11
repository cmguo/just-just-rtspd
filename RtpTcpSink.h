// RtpTcpSink.h

#ifndef _PPBOX_RTSPD_RTP_TCP_SINK_H_
#define _PPBOX_RTSPD_RTP_TCP_SINK_H_

#include "ppbox/rtspd/RtpSink.h"

#include <util/buffers/BuffersSize.h>

#include <boost/asio/ip/tcp.hpp>

namespace ppbox
{
    namespace rtspd
    {

        class RtpTcpSocket
        {
        public:
            RtpTcpSocket(
                boost::asio::ip::tcp::socket & tcp_socket, 
                boost::uint8_t interleaved)
                : tcp_socket_(tcp_socket)
                , head_left_(0)
            {
                head_[0] = '$';
                head_[1] = interleaved;
                head_[2] = 0;
                head_[3] = 0;
            }

        public:
            template <typename ConstBufferSequence>
            std::size_t send(
                const ConstBufferSequence& buffers,
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code& ec)
            {
                std::vector<boost::asio::const_buffer> buffers2;
                if (head_left_) {
                    buffers2.push_back(boost::asio::buffer(head_ + 4 - head_left_, head_left_));
                } else if (body_left_) {
                    assert(body_left_ == util::buffers::buffers_size(buffers));
                } else {
                    size_t len = util::buffers::buffers_size(buffers);
                    head_[2] = (boost::uint8_t)(len >> 8);
                    head_[3] = (boost::uint8_t)(len & 0xff);
                    head_left_ = 4;
                    buffers2.push_back(boost::asio::buffer(head_, 4));
                }
                buffers2.insert(buffers2.end(), buffers.begin(), buffers.end());
                size_t byte_sent = tcp_socket_.send(buffers2, flags, ec);
                if (byte_sent < head_left_) {
                    head_left_ -= byte_sent;
                    byte_sent = 0;
                } else {
                    head_left_ = 0;
                    byte_sent -= head_left_;
                    body_left_ -= byte_sent;
                }
                return byte_sent;
            }

        private:
            boost::asio::ip::tcp::socket & tcp_socket_;
            boost::uint8_t head_[4];
            size_t head_left_;
            size_t body_left_;
        };

        class RtpTcpSink
            : public RtpSink<RtpTcpSocket>
        {
        public:
            RtpTcpSink(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                boost::uint8_t interleaveds[2], 
                boost::system::error_code & ec);

            virtual ~RtpTcpSink();

        private:
            RtpTcpSocket rtp_socket_;
            RtpTcpSocket rtcp_socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_TCP_SINK_H_