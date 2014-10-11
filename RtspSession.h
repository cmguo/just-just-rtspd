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

        using util::protocol::RtspRequest;
        using util::protocol::RtspResponse;

        class RtspSession
            : public util::protocol::RtspServer
             //,public AsyncCallback
        {
        public:
            RtspSession(
                RtspdModule & mgr);

            ~RtspSession();

        protected:
            virtual void on_start();

            virtual void on_recv(
                RtspRequest const & req);

            virtual void on_sent(
                RtspResponse const & resp);

        public:
            // public: called by ServerManager
            virtual void on_error(
                boost::system::error_code const & ec);

        private:
            void on_seek(
                boost::system::error_code const & ec, 
                util::protocol::rtsp_field::Range * range, 
                std::string * rtp_info);

            void on_play(
                boost::system::error_code const & ec);

        private:
            RtspdModule & mgr_;
            boost::uint32_t session_id_;
            std::string content_base_;
            RtspDispatcher * dispatcher_;
            bool closed_;
            boost::uint32_t play_count_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSP_SESSION_H_
