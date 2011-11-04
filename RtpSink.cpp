// RtspSession.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpSink.h"
#include "ppbox/rtspd/Transport.h"

#include <ppbox/mux/rtp/RtpPacket.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("RtpSink", 0)

namespace ppbox
{
    namespace rtspd
    {
        RtpSink::RtpSink()
        {
        }

        RtpSink::~RtpSink()
        {
            if (transports_.first)
                delete transports_.first;
            if (transports_.second)
                delete transports_.second;
        }

        //工作线程调用
        boost::system::error_code RtpSink::write( ppbox::demux::Sample& tag)
        {
            boost::system::error_code ec;
            ppbox::mux::RtpSplitContent * packet = (ppbox::mux::RtpSplitContent*)tag.context;
            if(NULL == packet)
            {
                return ec;
            }
            for (size_t ii = 0; ii < packet->packets.size(); ++ii)
            {
               ec =  transports_.first->send_packet(packet->packets[ii].buffers);
               if(ec)
               {
                   std::cout<<" write failed "<<std::endl;
                   break;
               }
            }
            return ec;
        }

        //下面为主线程调用
        boost::system::error_code RtpSink::setup(
            boost::asio::ip::tcp::socket * rtsp_sock, 
            std::string const & transport,
            std::string & output)
        {
            boost::system::error_code ec;
            transports_ = Transport::create(*rtsp_sock, transport, output, ec);
            return ec;
        }

    } // namespace rtspd
} // namespace ppbox
