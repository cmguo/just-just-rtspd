// RtpDispatcher.h

#ifndef _JUST_RTSPD_RTSP_DISPATCHER_H_
#define _JUST_RTSPD_RTSP_DISPATCHER_H_

#include <just/dispatch/CustomDispatcher.h>

#include <util/protocol/rtsp/RtspFieldRange.h>

#include <boost/asio/ip/tcp.hpp>

#include <boost/asio/streambuf.hpp>

namespace just
{
    namespace rtspd
    {  

        class RtspDispatcher 
            : public just::dispatch::CustomDispatcher
        {
        public:

            RtspDispatcher(
                just::dispatch::DispatcherBase & dispatcher);

            ~RtspDispatcher();

        public:
            void async_open(
                framework::string::Url & url, 
                std::string const & client, 
                boost::asio::streambuf & os, 
                just::dispatch::response_t  const & resp);

            bool setup(
                boost::asio::ip::tcp::socket & rtsp_sock, 
                std::string const & control, 
                std::string const & transport, 
                std::string & rtp_setup, 
                boost::system::error_code & ec);

            void async_play(
                util::protocol::rtsp_field::Range & range, 
                std::string & rtp_info, 
                just::dispatch::response_t const & seek_resp, 
                just::dispatch::response_t const & resp);

        private:
            void handle_open(
                boost::asio::streambuf& os_sdp, 
                just::dispatch::response_t  const & resp, 
                boost::system::error_code ec);

            void handle_seek(
                std::string & rtp_info, 
                util::protocol::rtsp_field::Range & range, 
                just::dispatch::response_t const & resp, 
                boost::system::error_code ec);

        private:
            std::vector<util::stream::Sink *> sinks_;
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_DISPATCHER_H_
