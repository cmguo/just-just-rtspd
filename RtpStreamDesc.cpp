// RtpStreamDesc.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/RtpStreamDesc.h"

#include <util/archive/TextIArchive.h>
#include <util/archive/TextOArchive.h>
#include <util/archive/ArchiveBuffer.h>

namespace ppbox
{
    namespace rtspd
    {

        void RtpStreamDesc::from_data(
            std::vector<boost::uint8_t> const & desc)
        {
            util::archive::ArchiveBuffer<> buf((char *)&desc.front(), desc.size(), desc.size());
            util::archive::TextIArchive<> ia(buf);
            ia >> (*this);
        }

        void RtpStreamDesc::to_data(
            std::vector<boost::uint8_t> & desc)
        {
            desc.resize(sdp_info.size() + rtp_info.size() + 1024);
            util::archive::ArchiveBuffer<> buf((char *)&desc.front(), desc.size());
            util::archive::TextOArchive<> oa(buf);
            oa << (*this);
            desc.resize(buf.size());
        }

    } // namespace rtspd
} // namespace ppbox
