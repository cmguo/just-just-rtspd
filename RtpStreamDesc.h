// RtpPacket.h

#ifndef _JUST_RTSPD_RTP_STREAM_DESC_H_
#define _JUST_RTSPD_RTP_STREAM_DESC_H_

namespace just
{
    namespace rtspd
    {

        struct RtpStreamDesc
        {
            std::string stream;
            boost::uint32_t ssrc;
            bool setup;
            std::string sdp_info;
            std::string rtp_info;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream & ssrc & setup;
                ar & sdp_info & rtp_info;
            }

            void from_data(
                std::vector<boost::uint8_t> const & desc);

            void to_data(
                std::vector<boost::uint8_t> & desc);
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_STREAM_DESC_H_
