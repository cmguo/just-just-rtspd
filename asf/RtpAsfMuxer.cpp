// RtpAsfMuxer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/asf/RtpAsfMuxer.h"
#include "ppbox/rtspd/asf/RtpAsfTransfer.h"

#include <ppbox/mux/filter/MergeFilter.h>
using namespace ppbox::mux;

namespace ppbox
{
    namespace rtspd
    {

        RtpAsfMuxer::RtpAsfMuxer()
            : RtpMuxer(&asf_mux_)
            , rtp_asf_transfer_(NULL)
        {
            format("asf");
        }

        RtpAsfMuxer::~RtpAsfMuxer()
        {
            if (rtp_asf_transfer_) {
                delete rtp_asf_transfer_;
                rtp_asf_transfer_ = NULL;
            }
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
            pipe.push_back(new MergeFilter(rtp_asf_transfer_));
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
} // namespace ppbox
