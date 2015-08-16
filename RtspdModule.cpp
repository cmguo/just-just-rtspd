// RtspdModule.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/RtspdModule.h"
#include "just/rtspd/RtspDispatcher.h"
#include "just/rtspd/RtspSession.h"
#include "just/rtspd/RtpMuxerTypes.h"

#include <just/dispatch/DispatchModule.h>

#include <framework/network/TcpSocket.hpp>

namespace just
{
    namespace rtspd
    {

        RtspdModule::RtspdModule(
            util::daemon::Daemon & daemon)
            : just::common::CommonModuleBase<RtspdModule>(daemon, "RtspdModule")
            , framework::network::ServerManager<RtspSession, RtspdModule>(daemon.io_svc())
            , addr_("0.0.0.0:5054+")
            , dispatch_module_(util::daemon::use_module<just::dispatch::DispatchModule>(get_daemon()))
        {
            daemon.config().register_module("RtspdModule")
                << CONFIG_PARAM_NAME_RDWR("addr",addr_ );
        }

        RtspdModule::~RtspdModule()
        {
        }

        bool RtspdModule::startup(
            boost::system::error_code & ec)
        {
            srand(time(NULL));
            start(addr_,ec);
            return !ec;
        }

        bool RtspdModule::shutdown(
            boost::system::error_code & ec)
        {
            stop(ec);
            return !ec;
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
            dispatch_module_.free_dispatcher(dispatcher->detach());
            delete dispatcher;
        }

    } // namespace rtspd
} // namespace just
