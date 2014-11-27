// RtpEsAudioTransfer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/raw/RtpMpeg4GenericTransfer.h"

#include <framework/string/Base16.h>

#include <sstream>

namespace just
{
    namespace rtspd
    {

        /* 
         * RFC 3640: RTP Payload Format for Transport of MPEG-4 Elementary Streams
         */

        RtpMpeg4GenericTransfer::RtpMpeg4GenericTransfer()
            : RtpTransfer("RtpMpeg4Generic", "mpeg4-generic", 97)
            , index_(0)
        {
            au_header_section_[0] = 0;
            au_header_section_[1] = 16;
        }

        RtpMpeg4GenericTransfer::~RtpMpeg4GenericTransfer()
        {
        }

        void RtpMpeg4GenericTransfer::transfer(
            StreamInfo & info)
        {
            using namespace framework::string;
            std::ostringstream oss;
            oss << "a=fmtp:" << (int)rtp_head_.mpt
                << " streamType=5"
                << ";profile-level-id=41"
                //<< ";objecttype=64"
                << ";mode=AAC-hbr"
                << ";sizeLength=13"
                << ";indexLength=3"
                << ";indexDeltaLength=3"
                << ";config=" << Base16::encode(std::string((char const *)&info.format_data.at(0), info.format_data.size()))
                << "\r\n";
            rtp_info_.sdp += oss.str();

            RtpTransfer::transfer(info);
        }

        void RtpMpeg4GenericTransfer::transfer(
            Sample & sample)
        {
            au_header_section_[2] = (boost::uint8_t)(sample.size >> 5);
            au_header_section_[3] = (boost::uint8_t)((sample.size << 3) /*| (index_++ & 0x07)*/);

            RtpTransfer::begin(sample);

            begin_packet(true, sample.dts, 4 + sample.size);
            push_buffers(boost::asio::buffer(au_header_section_, 4));
            push_buffers(sample.data);
            finish_packet();

            RtpTransfer::finish(sample);
        }

    } // namespace rtspd
} // namespace just
