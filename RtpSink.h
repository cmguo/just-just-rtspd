// RtpSink.h

#ifndef _PPBOX_RTSPD_RTP_SINK_H_
#define _PPBOX_RTSPD_RTP_SINK_H_

#include <ppbox/rtspd/RtpPacket.h>

#include <util/stream/Sink.h>

namespace ppbox
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
                size_t total_size = 0;
                if (packets_.empty()) {
                    assert(sizeof(packets_) == boost::asio::buffer_size(*--buffers.end()));
                    std::vector<RtpPacket> const & packets = 
                        *boost::asio::buffer_cast<std::vector<RtpPacket> const *>(*--buffers.end());
                    packets_.swap(const_cast<std::vector<RtpPacket> &>(packets));
                }
                bool finish = true;
                for (size_t i = next_packet_; i < packets_.size(); ++i) {
                    size_t bytes_send = 0;
                    if (packets_[i].mpt == 0) { // RTCP
                        bytes_send = rtcp_socket_.send(
                            sub_collection(buffers, packets_[i].buf_beg, packets_[i].buf_end), 
                            0, ec);
                    } else {
                        bytes_send = rtp_socket_.send(
                            sub_collection(buffers, packets_[i].buf_beg, packets_[i].buf_end), 
                            0, ec);
                    }
                    total_size += bytes_send;
                    if (ec || bytes_send < packets_[i].size) {
                        finish = false;
                        break;
                    }
                }
                if (finish) {
                    total_size += sizeof(packets_);
                    packets_.clear();
                    next_packet_ = 0;
                } else {
                    size_t total_size2 = total_size;
                    for (size_t i = next_packet_; i < packets_.size(); ++i) {
                        if (packets_[i].size > total_size2) {
                            next_packet_ = i;
                            packets_[i].size -= total_size2;
                            buffers_t::const_iterator iter = buffers.begin() + packets_[i].buf_beg;
                            for (size_t j = packets_[i].buf_beg; j < packets_[i].buf_end; ++j, ++iter) {
                                if (boost::asio::buffer_size(*iter) > total_size2) {
                                    packets_[i].buf_beg = j;
                                    for (size_t k = i; k < packets_.size(); ++k) {
                                        packets_[k].buf_beg -= j;
                                        packets_[k].buf_end -= j;
                                    }
                                    break;
                                }
                                total_size2 -= boost::asio::buffer_size(*iter);
                            }
                            break;
                        }
                        total_size2 -= packets_[i].size;
                    }
                }
                return total_size;
            }

        private:
            SocketType & rtp_socket_;
            SocketType & rtcp_socket_;
            std::vector<RtpPacket> packets_;
            size_t next_packet_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
