// UdpTransport.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/Transport.h"
#include "ppbox/rtspd/UdpTransport.h"
#include "ppbox/rtspd/TcpTransport.h"

#include <framework/system/BytesOrder.h>
#include <framework/string/Base16.h>

#include <framework/string/Slice.h>
#include <framework/string/Join.h>
using namespace framework::string;

using namespace boost::system;

namespace ppbox
{
    namespace rtspd
    {

        std::pair<Transport *, Transport *> Transport::create(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            std::string const & in_transport, 
            std::string & out_transport, 
            error_code & ec)
        {
            std::vector<std::string> vec;
            slice<std::string>(in_transport, std::back_inserter(vec), ";", "{", "}");
            std::pair<Transport *, Transport *> transports;
            if (vec[0] == "RTP/AVP" || vec[0] == "RTP/AVP/UDP") {
                for (size_t i = 1; i < vec.size(); ++i) {
                    if (vec[i].substr(0, sizeof("client_port")) == "client_port=") {
                        std::vector<boost::uint16_t> ports(2, 0);
                        slice<boost::uint16_t>(vec[i], ports.begin(), "-", "client_port=");
                        transports.first = new UdpTransport(rtsp_socket, ports[0], ports[0], ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            break;
                        }
                        transports.second = new UdpTransport(rtsp_socket, ports[1], ports[1], ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            delete transports.second;
                            transports.second = NULL;
                            break;
                        }
                        vec[i] += ";" + join(ports.begin(), ports.end(), "-", "server_port=");
                    }
                }
            } else {
                for (size_t i = 1; i < vec.size(); ++i) {
                    if (vec[i].substr(0, sizeof("interleaved")) == "interleaved=") {
                        std::vector<boost::uint16_t> ports(2, 0);
                        slice<boost::uint16_t>(vec[i], ports.begin(), "-", "interleaved=");
                        transports.first = new TcpTransport(rtsp_socket, ports[0], ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            break;
                        }
                        transports.second = new TcpTransport(rtsp_socket, ports[1], ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            delete transports.second;
                            transports.second = NULL;
                            break;
                        }
                    }
                }
            }
            if (!ec) {
                /*boost::uint32_t iSsrc = 0;
                framework::string::parse2(ssrc.c_str(),iSsrc);
                boost::uint32_t ssrc_eb = framework::system::BytesOrder::big_endian_to_host(iSsrc);
                iSsrc = framework::system::BytesOrder::host_to_big_endian(ssrc_eb);*/
                out_transport = join(vec.begin(), vec.end(), ";", "", "");
            }
            return transports;
        }

    } // namespace rtspd
} // namespace ppbox
