// RtspSession.h

#ifndef _PPBOX_RTSPD_RTSP_SESSION_H_
#define _PPBOX_RTSPD_RTSP_SESSION_H_

#include <util/protocol/rtsp/RtspServer.h>
#include <util/protocol/rtsp/RtspFieldRange.h>

#include <boost/shared_ptr.hpp>

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
            virtual void local_process_request(
                response_type const & resp);

            virtual void on_error(
                boost::system::error_code const & ec);

            virtual void on_finish();

            virtual void post_process(
                response_type const & resp);

            void on_play(
                boost::system::error_code const & ec);

        private:
            RtspdModule & mgr_;
            boost::uint32_t session_id_;
            std::string content_base_;
            RtspDispatcher * dispatcher_;
            boost::uint32_t play_count_;
            response_type post_resp_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSP_SESSION_H_