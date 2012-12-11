// RtspdModule.h

#ifndef _PPBOX_RTSPD_RTSPD_MODULE_H_
#define _PPBOX_RTSPD_RTSPD_MODULE_H_

#include <util/protocol/rtsp/RtspServerManager.h>

#include <framework/string/Url.h>

namespace ppbox
{
    namespace dispatch
    {
        class DispatchModule;
    }

    namespace rtspd
    {

        class RtspSession;
        class RtspDispatcher;

        class RtspdModule 
            : public ppbox::common::CommonModuleBase<RtspdModule>
            , public util::protocol::RtspServerManager<RtspSession, RtspdModule>
        {
        public:
            RtspdModule(
                util::daemon::Daemon & daemon);

            virtual ~RtspdModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            using ppbox::common::CommonModuleBase<RtspdModule>::io_svc;

            RtspDispatcher * alloc_dispatcher(
                framework::string::Url & url, 
                boost::system::error_code & ec);

            void free_dispatcher(
                RtspDispatcher * dispatcher);

        private:
            framework::network::NetName addr_;
            ppbox::dispatch::DispatchModule & dispatch_module_;
        };

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTSPD_MODULE_H_