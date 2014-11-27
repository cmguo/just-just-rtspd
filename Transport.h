// Transport.h

#ifndef _JUST_RTSPD_TRANSPORT_H_
#define _JUST_RTSPD_TRANSPORT_H_

#include "just/rtspd/RtpSink.h"

#include <boost/asio/ip/tcp.hpp>

namespace just
{
    namespace rtspd
    {

        extern util::stream::Sink * create_transport(
                boost::asio::ip::tcp::socket & rtsp_socket, 
                std::string const & in_transport, 
                std::string & out_transport, 
                boost::system::error_code & ec);

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_TRANSPORT_H_