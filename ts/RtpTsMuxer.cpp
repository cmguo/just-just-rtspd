// RtpTsMuxer.cpp

#include "just/rtspd/Common.h"
#include "just/rtspd/ts/RtpTsMuxer.h"
#include "just/rtspd/ts/RtpTsTransfer.h"

#include <just/mux/filter/MergeFilter.h>
using namespace just::mux;

namespace just
{
    namespace rtspd
    {

        RtpTsMuxer::RtpTsMuxer(
            boost::asio::io_service & io_svc)
            : RtpMuxer(io_svc, &ts_mux_)
            , ts_mux_(io_svc)
            , rtp_ts_transfer_(NULL)
        {
            format("ts");
        }

        RtpTsMuxer::~RtpTsMuxer()
        {
            // auto release by MergeFilter
            rtp_ts_transfer_ = NULL;
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
            pipe.insert(new MergeFilter(rtp_ts_transfer_));
        }

        void RtpTsMuxer::file_header(
            Sample & tag)
        {
            RtpMuxer::file_header(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

    } // namespace rtspd
} // namespace just
