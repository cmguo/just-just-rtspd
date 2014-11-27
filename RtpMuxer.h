// RtpMuxer.h

#ifndef _JUST_RTSPD_RTP_MUXER_H_
#define _JUST_RTSPD_RTP_MUXER_H_

#include "just/rtspd/RtpPacket.h"

#include <just/mux/Muxer.h>

namespace just
{
    namespace rtspd
    {

        using just::mux::StreamInfo;
        using just::mux::MediaInfo;
        using just::mux::Sample;
        using just::mux::FilterPipe;

        class RtpTransfer;

        class RtpMuxer
            : public just::mux::Muxer
        {
        public:
            RtpMuxer(
                boost::asio::io_service & io_svc);

            RtpMuxer(
                boost::asio::io_service & io_svc, 
                MuxerBase * base);

            ~RtpMuxer();

        public:
            virtual bool setup(
                boost::uint32_t index, 
                boost::system::error_code & ec);

        public:
            virtual void media_info(
                MediaInfo & info) const;

            virtual void stream_info(
                std::vector<StreamInfo> & streams) const;

        protected:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            void file_header(
                Sample & sample);

            void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        protected:
           void add_rtp_transfer(
               RtpTransfer * rtp_transfer);

        private:
            MuxerBase * base_;
            std::vector<RtpTransfer *> rtp_transfers_;
        };

    } // namespace rtspd
} // namespace just

#endif // _JUST_RTSPD_RTP_MUXER_H_
