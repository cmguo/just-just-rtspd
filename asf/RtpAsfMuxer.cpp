// RtpAsfMuxer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/asf/RtpAsfMuxer.h"
#include "just/rtspd/asf/RtpAsfTransfer.h"

#include <just/mux/filter/MergeFilter.h>
using namespace just::mux;

namespace just
{
    namespace rtspd
    {

        RtpAsfMuxer::RtpAsfMuxer(
            boost::asio::io_service & io_svc)
            : RtpMuxer(io_svc, &asf_mux_)
            , asf_mux_(io_svc)
            , rtp_asf_transfer_(NULL)
        {
            format("asf");
        }

        RtpAsfMuxer::~RtpAsfMuxer()
        {
            // auto release by MergeFilter
            rtp_asf_transfer_ = NULL;
        }

        void RtpAsfMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            RtpMuxer::add_stream(info, pipe);
            if (rtp_asf_transfer_ == NULL) {
                rtp_asf_transfer_ = new RtpAsfTransfer;
                add_rtp_transfer(rtp_asf_transfer_);
            }
            pipe.insert(new MergeFilter(rtp_asf_transfer_));
        }

        void RtpAsfMuxer::media_info(
            MediaInfo & info) const
        {
            RtpMuxer::media_info(info);
            std::string sdp;
            Sample tag;
            const_cast<RtpAsfMuxer *>(this)->RtpMuxer::file_header(tag);
            rtp_asf_transfer_->get_sdp(tag, sdp);
            sdp += info.format_data;
            info.format_data.swap(sdp);
        }

    } // namespace rtspd
} // namespace just
