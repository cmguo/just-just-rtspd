// RtpSink.h

#ifndef _JUST_RTSPD_RTP_SINK_H_
#define _JUST_RTSPD_RTP_SINK_H_

#include <just/rtspd/RtpPacket.h>

#include <util/stream/Sink.h>
#include <util/buffers/BuffersSize.h>

namespace just
{
    namespace rtspd
    {

        template <typename SocketType>
        class RtpSink
            : public util::stream::Sink
        {
        protected:
            RtpSink(
                boost::asio::io_service & io_svc, 
                SocketType & rtp_socket, 
                SocketType & rtcp_socket)
                : util::stream::Sink(io_svc)
                , rtp_socket_(rtp_socket)
                , rtcp_socket_(rtcp_socket)
                , next_packet_(0)
            {
            }

            virtual ~RtpSink() {}

        private:
            template <typename Collection>
            struct sub_collection_t
            {
                typedef typename Collection::value_type value_type;
                typedef typename Collection::const_iterator const_iterator;

                sub_collection_t(
                    const_iterator begin, 
                    const_iterator end)
                    : begin_(begin)
                    , end_(end)
                {
                }

                const_iterator begin() const {return begin_;}
                const_iterator end() const {return end_;}

            private:
                const_iterator begin_;
                const_iterator end_;
            };

            template <typename Collection>
            sub_collection_t<Collection> sub_collection(
                Collection & collection, 
                size_t begin, 
                size_t end)
            {
                return sub_collection_t<Collection>(collection.begin() + begin, collection.begin() + end);
            }

        private:
            virtual size_t private_write_some(
                buffers_t const & buffers,
                boost::system::error_code & ec)
            {
                size_t size_left = util::buffers::buffers_size(buffers);
                size_t size_send = 0;
                if (packets_.empty()) {
                    assert(sizeof(packets_) == boost::asio::buffer_size(*buffers.begin()));
                    std::vector<RtpPacket> const & packets = 
                        *boost::asio::buffer_cast<std::vector<RtpPacket> const *>(*buffers.begin());
                    packets_.swap(const_cast<std::vector<RtpPacket> &>(packets));
                    size_left -= sizeof(packets_);
                    size_send += sizeof(packets_);
                }
                size_t ipacket = next_packet_;
                size_t buf_beg = 0;
                for (; ipacket < packets_.size(); ++ipacket) {
                    RtpPacket & packet = packets_[ipacket];
                    if (size_left < packet.size) {
                        buf_beg = packet.buf_beg;
                        break;
                    }
                    size_t bytes_send = 0;
                    if (packet.mpt == 0) { // RTCP
                        bytes_send = rtcp_socket_.send(
                            sub_collection(buffers, packet.buf_beg, packet.buf_end), 
                            0, ec);
                    } else {
                        bytes_send = rtp_socket_.send(
                            sub_collection(buffers, packet.buf_beg, packet.buf_end), 
                            0, ec);
                    }
                    size_left -= bytes_send;
                    size_send += bytes_send;
                    if (ec || bytes_send < packet.size) {
                        packet.size -= bytes_send;
                        buffers_t::const_iterator iter = buffers.begin() + packet.buf_beg;
                        for (size_t i = packet.buf_beg; i < packet.buf_end; ++i, ++iter) {
                            if (boost::asio::buffer_size(*iter) > bytes_send) {
                                buf_beg = i;
                                break;
                            }
                            bytes_send -= boost::asio::buffer_size(*iter);
                        }
                        break;
                    }
                }
                if (ipacket == packets_.size()) {
                    packets_.clear();
                    next_packet_ = 0;
                } else {
                    next_packet_ = ipacket;
                    for (size_t i = ipacket; i < packets_.size(); ++i) {
                        packets_[i].buf_beg -= buf_beg;
                        packets_[i].buf_end -= buf_beg;
                    }
                }
                return size_send;
            }

        private:
            SocketType & rtp_socket_;
            SocketType & rtcp_socket_;
            std::vector<RtpPacket> packets_;
            size_t next_packet_;
        };

    } // namespace rtspd
} // namespace just

#endif 
