// RtpDispatcher.h

#ifndef _PPBOX_RTSPD_RTSP_DISPATCHER_H_
#define _PPBOX_RTSPD_RTSP_DISPATCHER_H_

#include <ppbox/dispatch/CustomDispatcher.h>

#include <util/protocol/rtsp/RtspFieldRange.h>

#include <boost/asio/ip/tcp.hpp>

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace rtspd
    {  

        class RtspDispatcher 
            : public ppbox::dispatch::CustomDispatcher
        {
        public:

            RtspDispatcher(
                ppbox::dispatch::DispatcherBase & dispatcher);

            ~RtspDispatcher();

        public:
            void async_open(
                framework::string::Url & url, 
                std::string const & client, 
                boost::asio::streambuf & os, 
                ppbox::dispatch::response_t  const & resp);

            bool setup(
                boost::asio::ip::tcp::socket & rtsp_sock, 
                std::string const & control, 
                std::string const & transport, 
                std::string & rtp_setup, 
                boost::system::error_code & ec);

            void async_play(
                util::protocol::rtsp_field::Range & range, 
                std::string & rtp_info, 
                ppbox::dispatch::response_t const & seek_resp, 
                ppbox::dispatch::response_t const & resp);

        private:
            void handle_open(
                boost::asio::streambuf& os_sdp, 
                ppbox::dispatch::response_t  const & resp, 
                boost::system::error_code ec);

            void handle_seek(
                std::string & rtp_info, 
                util::protocol::rtsp_field::Range & range, 
                ppbox::dispatch::response_t const & resp, 
                boost::system::error_code ec);
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_DISPATCHER_H_