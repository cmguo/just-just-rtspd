// RtspdModule.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspdModule.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspSession.h"

#include <ppbox/dispatch/DispatchModule.h>

namespace ppbox
{
    namespace rtspd
    {

        RtspdModule::RtspdModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<RtspdModule>(daemon, "RtspdModule")
            , util::protocol::RtspServerManager<RtspSession, RtspdModule>(daemon.io_svc())
            , addr_("0.0.0.0:5054")
            , dispatch_module_(util::daemon::use_module<ppbox::dispatch::DispatchModule>(get_daemon()))
        {
            daemon.config().register_module("RtspdModule")
                << CONFIG_PARAM_NAME_RDWR("addr",addr_ );
        }

        RtspdModule::~RtspdModule()
        {
        }

        boost::system::error_code RtspdModule::startup()
        {
            boost::system::error_code ec;
            start(addr_,ec);
            return ec;
        }

        void RtspdModule::shutdown()
        {
            stop();
        }

        RtspDispatcher * RtspdModule::alloc_dispatcher(
            framework::string::Url & url, 
            boost::system::error_code & ec)
        {
            dispatch_module_.normalize_url(url, ec);
            return new RtspDispatcher(*dispatch_module_.alloc_dispatcher(true));
        }

        void RtspdModule::free_dispatcher(
            RtspDispatcher * dispatcher)
        {
            dispatch_module_.free_dispatcher(&dispatcher->get_dispatcher());
            delete dispatcher;
        }

    } // namespace rtspd
} // namespace ppbox
