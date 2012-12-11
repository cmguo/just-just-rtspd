// Transport.h

#ifndef _PPBOX_RTSPD_TRANSPORT_H_
#define _PPBOX_RTSPD_TRANSPORT_H_

#include "ppbox/rtspd/RtpSink.h"

#include <boost/asio/ip/tcp.hpp>

namespace ppbox
{
    namespace rtspd
    {

        extern ppbox::dispatch::Sink * create_transport(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                std::string const & in_transport, 
                std::string & out_transport, 
                boost::system::error_code & ec);

    } // namespace rtspd
} // namespace ppbox

#endif // _PPBOX_RTSPD_TRANSPORT_H_