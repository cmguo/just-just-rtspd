// TcpTransport.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/TcpTransport.h"

#include <util/buffers/BufferSize.h>

#include <boost/asio/write.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace rtspd
    {

        TcpTransport::TcpTransport(
            boost::asio::ip::tcp::socket & rtsp_socket, 
            boost::uint8_t interleave, 
            error_code & ec)
            : socket_(rtsp_socket)
            , interleave_(interleave)
        {
            ec = error_code();
        }

        TcpTransport::~TcpTransport()
        {
        }

        error_code TcpTransport::send_packet(
            std::vector<boost::asio::const_buffer> const & buffers)
        {
            size_t len = util::buffers::buffer_size(buffers);
            boost::uint8_t head[3] = {
                (boost::uint8_t)interleave_, 
                (boost::uint8_t)(len >> 8), 
                (boost::uint8_t)(len & 0xff), 
            };
            error_code ec;
            std::vector<boost::asio::const_buffer> buffers2(
                1, boost::asio::const_buffer(head, 3));
            buffers2.insert(buffers2.end(), buffers.begin(), buffers.end());
            boost::asio::write(socket_, buffers2, boost::asio::transfer_all(), ec);
            return ec;
        }

    } // namespace rtspd
} // namespace ppbox
