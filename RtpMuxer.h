// RtpMuxer.h

#ifndef _PPBOX_RTSPD_RTP_MUXER_H_
#define _PPBOX_RTSPD_RTP_MUXER_H_

#include "ppbox/rtspd/RtpPacket.h"

#include <ppbox/mux/MuxerBase.h>

namespace ppbox
{
    namespace rtspd
    {

        using ppbox::mux::StreamInfo;
        using ppbox::mux::MediaInfo;
        using ppbox::mux::Sample;
        using ppbox::mux::FilterPipe;

        class RtpTransfer;

        class RtpMuxer
            : public ppbox::mux::MuxerBase
        {
        public:
            RtpMuxer();

            RtpMuxer(
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
} // namespace ppbox

#endif // _PPBOX_RTSPD_RTP_MUXER_H_
