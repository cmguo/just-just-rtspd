// RtspManager.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtspDispatcher.h"
#include "ppbox/rtspd/RtspSession.h"

#include "ppbox/rtspd/RtspManager.h"

namespace ppbox
{
    namespace rtspd
    {

        RtspManager::RtspManager(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<RtspManager>(daemon, "RtspManager")
            , util::protocol::RtspServerManager<RtspSession, RtspManager>(daemon.io_svc())
            ,addr_("0.0.0.0:554")
        {
            daemon.config().register_module("RtspManager")
                << CONFIG_PARAM_NAME_NOACC("addr",addr_ );
        }

        RtspManager::~RtspManager()
        {
        }

        boost::system::error_code RtspManager::startup()
        {
            boost::system::error_code ec;
            dispatcher_ = new RtspDispatcher(get_daemon());
            start(addr_,ec);
            return ec;
        }

        void RtspManager::shutdown()
        {
            stop();
            dispatcher_->stop();
            delete dispatcher_;
        }


    } // namespace rtspd
} // namespace ppbox
