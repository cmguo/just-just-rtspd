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
        class RtspManager;

        class RtspSession
            : public util::protocol::RtspServer
             //,public AsyncCallback
        {
        public:
            RtspSession(
                RtspManager & mgr);

            ~RtspSession();

        public:
            virtual void local_process(
                local_process_response_type const & resp);

            virtual void on_error(
                boost::system::error_code const & ec);

            virtual void on_finish();


            void on_play(
                boost::weak_ptr<void> const & token, 
                boost::system::error_code const & ec);


        private:
            std::string path_;
            RtspDispatcher * dispatcher_;
            RtspManager& mgr_;
            boost::uint32_t session_id_;
            util::protocol::rtsp_field::Range range_;
            boost::shared_ptr<void> close_token_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSP_SESSION_H_