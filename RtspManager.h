// PlayManager.h

#ifndef _PPBOX_RTSPD_RTSPMANAGER_H_
#define _PPBOX_RTSPD_RTSPMANAGER_H_



#include <util/protocol/rtsp/RtspServerManager.h>

namespace ppbox
{

    namespace rtspd
    {
        class RtspSession;
        class RtspDispatcher;

        class RtspManager 
            : public ppbox::common::CommonModuleBase<RtspManager>
            , public util::protocol::RtspServerManager<RtspSession, RtspManager>  
        {
        public:
            RtspManager(
                util::daemon::Daemon & daemon);

            virtual ~RtspManager();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            using ppbox::common::CommonModuleBase<RtspManager>::io_svc;

            RtspDispatcher * dispatcher()
            {
                return dispatcher_;
            }

        private:
            framework::network::NetName addr_;
            RtspDispatcher * dispatcher_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_PLAY_MANAGER_H_