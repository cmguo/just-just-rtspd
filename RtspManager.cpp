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
            ,addr_("0.0.0.0:5054")
            ,dispatcher_(new RtspDispatcher(daemon))
        {
            daemon.config().register_module("RtspManager")
                << CONFIG_PARAM_NAME_NOACC("addr",addr_ );
        }

        RtspManager::~RtspManager()
        {
            delete dispatcher_;
        }

        boost::system::error_code RtspManager::startup()
        {
            boost::system::error_code ec;
            dispatcher_->start();
            start(addr_,ec);
            return ec;
        }

        void RtspManager::shutdown()
        {
            stop();
            dispatcher_->stop();
        }


    } // namespace rtspd
} // namespace ppbox
