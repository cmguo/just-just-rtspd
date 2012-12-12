// RtspSession.h

#ifndef _PPBOX_RTSPD_RTSP_SESSION_H_
#define _PPBOX_RTSPD_RTSP_SESSION_H_

#include <util/protocol/rtsp/RtspServer.h>

#include <boost/shared_ptr.hpp>

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
    namespace rtspd
    {
        class ResourcePool;

        class RtspDispatcher;
        class RtspdModule;

        class RtspSession
            : public util::protocol::RtspServer
             //,public AsyncCallback
        {
        public:
            RtspSession(
                RtspdModule & mgr);

            ~RtspSession();

        public:
            virtual void local_process(
                response_type const & resp);

            virtual void on_error(
                boost::system::error_code const & ec);

            virtual void on_finish();

            virtual void post_process(
                response_type const & resp);

            void on_play(
                boost::system::error_code const & ec);

        private:
            std::string path_;
            RtspdModule& mgr_;
            boost::uint32_t session_id_;
            RtspDispatcher * dispatcher_;
            util::protocol::rtsp_field::Range range_;
            bool rtp_info_send_;
            bool playing_;
            response_type post_resp_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSP_SESSION_H_