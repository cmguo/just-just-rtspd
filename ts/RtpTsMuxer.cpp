// RtpTsMuxer.cpp

#include "ppbox/rtspd/Common.h"
#include "ppbox/rtspd/ts/RtpTsMuxer.h"
#include "ppbox/rtspd/ts/RtpTsTransfer.h"

#include <ppbox/mux/filter/MergeFilter.h>
using namespace ppbox::mux;

namespace ppbox
{
    namespace rtspd
    {

        RtpTsMuxer::RtpTsMuxer()
            : RtpMuxer(&ts_mux_)
            , rtp_ts_transfer_(NULL)
        {
            format("ts");
        }

        RtpTsMuxer::~RtpTsMuxer()
        {
            if (rtp_ts_transfer_) {
                delete rtp_ts_transfer_;
                rtp_ts_transfer_ = NULL;
            }
        }

        void RtpTsMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            RtpMuxer::add_stream(info, pipe);
            if (rtp_ts_transfer_ == NULL) {
                rtp_ts_transfer_ = new RtpTsTransfer;
                add_rtp_transfer(rtp_ts_transfer_);
            }
            pipe.push_back(new MergeFilter(rtp_ts_transfer_));
        }

        void RtpTsMuxer::file_header(
            Sample & tag)
        {
            RtpMuxer::file_header(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

    } // namespace rtspd
} // namespace ppbox
