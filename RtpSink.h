// RtpSink.h

#ifndef _PPBOX_RTSPD_RTP_SINK_H_
#define _PPBOX_RTSPD_RTP_SINK_H_

#include <ppbox/mux/rtp/RtpPacket.h>

#include <ppbox/dispatch/DispatchBase.h>
#include <ppbox/dispatch/Sink.h>

namespace ppbox
{
    namespace rtspd
    {

        template <typename SocketType>
        class RtpSink
            : public ppbox::dispatch::Sink
        {
        protected:
            RtpSink(
                SocketType & rtp_socket, 
                SocketType & rtcp_socket)
                : rtp_socket_(rtp_socket)
                , rtcp_socket_(rtcp_socket)
            {
            }

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
            virtual size_t write(
                ppbox::avformat::Sample const & sample, 
                boost::system::error_code & ec)
            {
                std::vector<ppbox::mux::RtpPacket> const & packets = 
                    *(std::vector<ppbox::mux::RtpPacket> const *)(sample.context);
                size_t total_size = 0;
                for (size_t i = 0; i < packets.size(); ++i) {
                    if (packets[i].size == 0) { // rtcp
                        rtcp_socket_.send(
                            sub_collection(sample.data, packets[i].buf_beg, packets[i].buf_end), 
                            0, ec);
                        continue;
                    }
                    total_size += rtp_socket_.send(
                        sub_collection(sample.data, packets[i].buf_beg, packets[i].buf_end), 
                        0, ec);
                    if (ec) {
                        break;;
                    }
                }
                return total_size;
            }

        private:
            SocketType & rtp_socket_;
            SocketType & rtcp_socket_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 
