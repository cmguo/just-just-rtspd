// RtpDispatcher.h

#ifndef _PPBOX_RTSPD_RTSP_DISPATCHER_H_
#define _PPBOX_RTSPD_RTSP_DISPATCHER_H_

#include <ppbox/mux/tool/Dispatcher.h>

#include <boost/asio/ip/tcp.hpp>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {
            class Range;
        }
    }
}

namespace ppbox
{
    namespace mux
    {
        struct MuxTag;
    }

    namespace rtspd
    {  
        class RtpSink;

        class RtspDispatcher 
            : public ppbox::mux::Dispatcher
        {
        public:

            RtspDispatcher(
                util::daemon::Daemon & daemon);

            ~RtspDispatcher();

        public:

            boost::system::error_code seek(
                const boost::uint32_t session_id
                ,util::protocol::rtsp_field::Range& range
                ,std::string& rtp_info
                ,ppbox::mux::session_callback_respone const &resp
                );

            boost::system::error_code open(
                boost::uint32_t& session_id,
                std::string const & play_link,
                std::string const & format,
                bool need_session,
                boost::asio::streambuf& os,
                boost::uint32_t clientType,
                ppbox::mux::session_callback_respone const & resp
                );

            boost::system::error_code setup(
                boost::uint32_t session_id, 
                boost::asio::ip::tcp::socket * rtsp_sock, 
                std::string const &  control, 
                std::string const &  transport,
                std::string & rtp_setup,
                ppbox::mux::session_callback_respone const & resp);

        private:
            void on_seek(
                std::string& rtp_info
                ,util::protocol::rtsp_field::Range& range
                ,ppbox::mux::session_callback_respone const &resp
                ,boost::system::error_code ec);

            void on_open(
                boost::asio::streambuf& os_sdp
                ,boost::uint32_t clientType
                ,ppbox::mux::session_callback_respone const &resp
                ,boost::system::error_code ec);

            void on_setup(
                size_t& index,
                std::string& rtp_info,
                ppbox::mux::session_callback_respone const &resp,
                boost::system::error_code ec);
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_DISPATCHER_H_