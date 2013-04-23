// RtpPacket.h

#ifndef _PPBOX_RTSPD_RTP_STREAM_DESC_H_
#define _PPBOX_RTSPD_RTP_STREAM_DESC_H_

namespace ppbox
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
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_STREAM_DESC_H_
