// RtcpPacket.h

#ifndef _JUST_RTSPD_RTCP_PACKET_H_
#define _JUST_RTSPD_RTCP_PACKET_H_

namespace just
{
    namespace rtspd
    {

        struct RtcpHead
        {
            //struct
            //{
            //    UInt8   sc : 5;
            //    UInt8   pad : 1;
            //    UInt8   ver : 2;
            //};
            boost::uint8_t pre;
            boost::uint8_t type;
            boost::uint16_t length;
        };

        struct RtcpSR
        {
            boost::uint32_t ssrc;
            boost::uint32_t ntph;
            boost::uint32_t ntpl;
            boost::uint32_t timestamp;
            boost::uint32_t packet;
            boost::uint32_t octet;
        };

        struct RtcpSDESItem
        {
            boost::uint8_t type;
            boost::uint8_t len;
            boost::uint8_t data[14];
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTCP_PACKET_H_
