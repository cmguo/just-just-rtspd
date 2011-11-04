#ifndef _PPBOX_RTSPD_RTPSINK_H_
#define _PPBOX_RTSPD_RTPSINK_H_

#include <ppbox/mux/tool/Sink.h>
#include <map>
#include <boost/asio/ip/tcp.hpp>


namespace ppbox
{
    namespace rtspd
    {
        class Transport;

        class RtpSink : public ppbox::mux::Sink
        {
        public:
            RtpSink();
            virtual ~RtpSink();

        public:
            boost::system::error_code setup(
                boost::asio::ip::tcp::socket * rtsp_sock, 
                std::string const & transport,
                std::string & output);

        private:
            virtual boost::system::error_code write(ppbox::demux::Sample&);

        private:
            std::pair<Transport *, Transport *> transports_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif 