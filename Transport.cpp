// UdpTransport.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/Transport.h"
#include "ppbox/rtspd/RtpUdpSink.h"
#include "ppbox/rtspd/RtpTcpSink.h"

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

        struct find_parameter
        {
            find_parameter(
                std::string const & find)
                : find_(find)
            {
            }

            bool operator()(
                std::string const & r)
            {
                return r.compare(0, find_.size(), find_) == 0;
            }

        private:
            std::string const find_;
        };

        transport_pair_t create_transport_pair(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            std::string const & in_transport, 
            std::string & out_transport, 
            error_code & ec)
        {
            std::vector<std::string> vec_t;
            slice<std::string>(in_transport, std::back_inserter(vec_t), ",", "", "");
            std::vector<std::string> vec;
            slice<std::string>(vec_t[0], std::back_inserter(vec), ";");
            transport_pair_t transports;
            if (vec[0] == "RTP/AVP" || vec[0] == "RTP/AVP/UDP") {
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("client_port="));
                if (iter != vec.end()) {
                    std::vector<boost::uint16_t> client_ports(2, 0);
                    std::vector<boost::uint16_t> server_ports(2, 0);
                    slice<boost::uint16_t>(*iter, client_ports.begin(), "-", "client_port=");
                    while (true) {
                        transports.first = new RtpUdpSink(rtsp_socket, client_ports[0], server_ports[0], ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            break;
                        }
                        server_ports[1] = server_ports[0] + 1;
                        transports.second = new RtpUdpSink(rtsp_socket, client_ports[1], server_ports[1], ec);
                        if (!ec) {
                            vec.insert(++iter, join(server_ports.begin(), server_ports.end(), "-", "server_port="));
                            break;
                        } else if (ec != boost::asio::error::address_in_use) {
                            delete transports.first;
                            transports.first = NULL;
                            delete transports.second;
                            transports.second = NULL;
                            break;
                        }
                    }
                }
            } else {
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("interleaved="));
                if (iter != vec.end()) {
                    std::vector<boost::uint16_t> interleaveds(2, 0);
                    slice<boost::uint16_t>(*iter, interleaveds.begin(), "-", "interleaved=");
                    transports.first = new RtpTcpSink(rtsp_socket, ec);
                    if (ec) {
                        delete transports.first;
                        transports.first = NULL;
                    } else {
                        transports.second = new RtpTcpSink(rtsp_socket, ec);
                        if (ec) {
                            delete transports.first;
                            transports.first = NULL;
                            delete transports.second;
                            transports.second = NULL;
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
