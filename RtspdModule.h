// RtspdModule.h

#ifndef _JUST_RTSPD_RTSPD_MODULE_H_
#define _JUST_RTSPD_RTSPD_MODULE_H_

#include <framework/network/ServerManager.h>
#include <framework/string/Url.h>

namespace just
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
            : public just::common::CommonModuleBase<RtspdModule>
            , public framework::network::ServerManager<RtspSession, RtspdModule>
        {
        public:
            RtspdModule(
                util::daemon::Daemon & daemon);

            virtual ~RtspdModule();

        public:
            virtual bool startup(
                boost::system::error_code & ec);

            virtual bool shutdown(
                boost::system::error_code & ec);

        public:
            using just::common::CommonModuleBase<RtspdModule>::io_svc;

            RtspDispatcher * alloc_dispatcher(
                framework::string::Url & url, 
                boost::system::error_code & ec);

            void free_dispatcher(
                RtspDispatcher * dispatcher);

        private:
            framework::network::NetName addr_;
            just::dispatch::DispatchModule & dispatch_module_;
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTSPD_MODULE_H_
