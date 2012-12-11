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

        ppbox::dispatch::Sink * create_transport(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            std::string const & in_transport, 
            std::string & out_transport, 
            error_code & ec)
        {
            std::vector<std::string> vec_t;
            slice<std::string>(in_transport, std::back_inserter(vec_t), ",", "", "");
            std::vector<std::string> vec;
            slice<std::string>(vec_t[0], std::back_inserter(vec), ";");
            ppbox::dispatch::Sink * sink = NULL;
            if (vec[0] == "RTP/AVP" || vec[0] == "RTP/AVP/UDP") {
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("client_port="));
                if (iter != vec.end()) {
                    boost::uint16_t client_ports[2] = {0, 0};
                    boost::uint16_t server_ports[2] = {0, 0};
                    slice<boost::uint16_t>(*iter, client_ports, "-", "client_port=");
                    sink = new RtpUdpSink(rtsp_socket, client_ports, server_ports, ec);
                    if (ec) {
                        delete sink;
                        sink = NULL;
                    } else {
                        vec.insert(++iter, join(server_ports, server_ports + 2, "-", "server_port="));
                    }
                }
            } else {
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("interleaved="));
                if (iter != vec.end()) {
                    boost::uint8_t interleaveds[2] = {0, 0};
                    slice<boost::uint8_t>(*iter, interleaveds, "-", "interleaved=");
                    sink = new RtpTcpSink(rtsp_socket, interleaveds, ec);
                    if (ec) {
                        delete sink;
                        sink = NULL;
                    }
                }
            }
            if (!ec) {
                out_transport = join(vec.begin(), vec.end(), ";", "", "");
            }
            return sink;
        }

    } // namespace rtspd
} // namespace ppbox
