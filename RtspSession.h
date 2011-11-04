// RtspSession.h

#ifndef _PPBOX_RTSPD_RTSP_SESSION_H_
#define _PPBOX_RTSPD_RTSP_SESSION_H_

#include <util/protocol/rtsp/RtspServer.h>


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

            void AsyncOpenCallback(
                local_process_response_type const & resp,
                boost::system::error_code const & ec
                );

            void AsyncSeekCallback(
                local_process_response_type const & resp,
                boost::system::error_code const & ec
                );
            void AsyncPlayCallback(
                local_process_response_type const & resp,
                boost::system::error_code const & ec
                );
            void AsyncPauseCallback(
                local_process_response_type const & resp,
                boost::system::error_code const & ec
                );
            void AsyncSetupCallback(
                local_process_response_type const & resp,
                boost::system::error_code const & ec
                );

            void AsyncTempCallback(
                boost::system::error_code const & ec
                );


        private:
            std::string path_;
            RtspDispatcher * dispatcher_;
            RtspManager& mgr_;
            boost::uint32_t session_id_;


            boost::uint32_t seek_beg_;
            boost::uint32_t seek_end_;
            std::string rtp_info_;
            std::string rtp_sdp_;
            std::string rtp_setup_;

        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSP_SESSION_H_