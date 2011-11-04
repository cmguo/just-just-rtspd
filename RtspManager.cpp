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

            dispatcher_ = new RtspDispatcher(daemon);
        }

        RtspManager::~RtspManager()
        {
            delete dispatcher_;
        }

        boost::system::error_code RtspManager::startup()
        {
#ifndef _LIB
            PPBOX_StartP2PEngine("12","161","08ae1acd062ea3ab65924e07717d5994");
#endif
            boost::system::error_code ec;
            start(addr_,ec);
            return ec;
        }

        void RtspManager::shutdown()
        {
            stop();
        }


    } // namespace rtspd
} // namespace ppbox
