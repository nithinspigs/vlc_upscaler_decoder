/* 
 * Direct references and boilerplate code:
 * vlc/modules/codec/ddummy.c
*/

// Items to include for VLC plugin functionality
#include "vlc_upscaler_decoder.h"

// Items to include for OpenCV functionality
// #include "./cvDecode/cvDecodeWrapper.h"

static int Open(vlc_object_t *);
static void Close(vlc_object_t *);

#define MODULE_STRING "vlc_upscaler_decoder"

vlc_module_begin()
	set_shortname("VLC Upscaler Decoder")
	set_description("VLC plugin that decodes video and upscales it in real time")
	set_capability("video decoder", 900)
	set_callbacks(Open, Close)
	add_bool("test boolean setting", false, "test boolean short", "test boolean long text", true)
vlc_module_end()

static int Open(vlc_object_t *p_this) {

	fprintf(stderr, "%s", "Open function\n");

	// Do I need p_dec -> p_sys?
	decoder_t *p_dec = (decoder_t *) p_this;
	decoder_sys_t *p_sys = malloc(sizeof(decoder_sys_t));
	if (!p_sys) {
		return VLC_ENOMEM;
	}

	fprintf(stderr, "%s%u\n", "p_dec -> fmt_in.i_codec: ", p_dec -> fmt_in.i_codec);
	fprintf(stderr, "%s%s\n", "fourcc description: ", vlc_fourcc_GetDescription(VIDEO_ES, p_dec -> fmt_in.i_codec));

	// Get ffmpeg codec and initialize decoding context
	const AVCodec *p_codec;
	enum AVCodecID i_ffmpeg_codec;
	if (!getFfmpegCodec(p_dec -> fmt_in.i_codec, &i_ffmpeg_codec))
		return VLC_EGENERIC;
	fprintf(stderr, "avcodec_get_name(i_ffmpeg_codec): %s\n", avcodec_get_name(i_ffmpeg_codec));
	p_codec = avcodec_find_decoder(i_ffmpeg_codec);
	fprintf(stderr, "p_codec -> long_name: %s\n", p_codec -> long_name);
	AVCodecContext *p_context = avcodec_alloc_context3(p_codec);
	if (p_context == NULL) {
		return VLC_EGENERIC;
	}

	p_dec -> p_sys = p_sys;
	p_sys -> v = newCvDecode();
	p_sys -> p_context = p_context;
	p_sys -> p_codec = p_codec;
	p_dec -> pf_decode = Decode;
	p_dec -> pf_flush = NULL; // not needed?
	
	// p_sys -> nvdec = nullptr; // do i need to specify this is nullptr?
	// p_sys->nvdec = cv::cudacodec::createVideoDecoder(cv::cudacodec::Format::H264); // for now no nvdec

	return VLC_SUCCESS;

}

static int Decode(decoder_t *p_dec, block_t *p_block) {

	fprintf(stderr, "%s", "Decode function\n");
	fprintf(stderr, "%s%u\n", "p_dec -> fmt_in.i_codec: ", p_dec -> fmt_in.i_codec);
	fprintf(stderr, "%s%s\n", "fourcc description: ", vlc_fourcc_GetDescription(VIDEO_ES, p_dec -> fmt_in.i_codec));

	/*
	// No data to decode
	if (!p_block) {
		return VLCDEC_SUCCESS;
	}
	*/

	/* Set output properties */
	p_dec -> fmt_out.video.i_chroma =
		p_dec -> fmt_out.i_codec = VLC_CODEC_RGB24;
	p_dec -> fmt_out.video.i_sar_num = 1;
	p_dec -> fmt_out.video.i_sar_den = 1;
	p_dec -> fmt_out.video.i_visible_width = 
		p_dec -> fmt_out.video.i_width = p_dec -> fmt_in.video.i_width;
	p_dec -> fmt_out.video.i_visible_height = 
		p_dec -> fmt_out.video.i_height = p_dec -> fmt_in.video.i_height;

	// Just decode, no processing
	if (decoder_UpdateVideoFormat(p_dec) < 0) {
		fprintf(stderr, "%s\n", "decoder video format not updated");
		block_Release(p_block);
		return VLC_EGENERIC;
	}
	// Actually p_pic = frame -> opaque
	// Only use decoder_New Picture if opaque is null
	// picture_Release upon error, else use same pic for next iteration of while loop
	picture_t *p_pic = decoder_NewPicture(p_dec);
	fprintf(stderr, "%s%u\n", "p_pic -> format.i_chroma: ", p_pic -> format.i_chroma);
	fprintf(stderr, "%s%s\n", "fourcc description: ", vlc_fourcc_GetDescription(VIDEO_ES, p_pic -> format.i_chroma));
	/*
	p_pic -> format.i_chroma: 875714130
	fourcc description: 24 bits RGB
	*/

	decoder_sys_t *p_sys = p_dec -> p_sys;

	// Decode block using ffmpeg until there is no data left
	while(true)
	{
	
		int i_used = 0;

		if (!p_block || p_block -> i_buffer <= 0)
			continue;

		AVPacket *packet = av_packet_alloc();
		if (unlikely(!packet))
			return VLCDEC_ECRITICAL;
		fprintf(stderr, "%s\n", "allocated packet");

		packet -> data = p_block -> p_buffer;
		packet -> size = p_block -> i_buffer;
		packet -> pts = p_block -> i_pts != VLC_TICK_INVALID ? 
			av_rescale_q(p_block -> i_pts, VLC_TIME_BASE_Q, AV_TIME_BASE_Q) - VLC_TICK_0 : AV_NOPTS_VALUE;
		packet -> dts = p_block -> i_dts != VLC_TICK_INVALID ? 
			av_rescale_q(p_block -> i_pts, VLC_TIME_BASE_Q, AV_TIME_BASE_Q) - VLC_TICK_0 : AV_NOPTS_VALUE;
		p_block->i_pts = p_block->i_dts = VLC_TICK_INVALID;
		fprintf(stderr, "%s\n", "edited packet data and size");

		fprintf(stderr, "%s\n", "sending packet to decoder");
		int ret = avcodec_send_packet(p_sys -> p_context, packet);
		if (ret != 0 && ret != AVERROR(EAGAIN)) {
			fprintf(stderr, "%s\n", "packet ret < 0");
			av_packet_free(&packet);
			if (ret == AVERROR(ENOMEM)) {
				fprintf(stderr, "%s\n", "critical decoder error: no memory");
				return VLCDEC_ECRITICAL;
			} else if (ret == AVERROR(EINVAL)) {
				fprintf(stderr, "%s\n", "critical decoder error: invalid argument");
				return VLCDEC_ECRITICAL;
			}
			break;
		}

		i_used = ret != AVERROR(EAGAIN) ? packet -> size : 0;
		av_packet_free(&packet);

		AVFrame *frame = av_frame_alloc();
		if (unlikely(!frame))
			return VLCDEC_ECRITICAL;
		fprintf(stderr, "%s\n", "allocated frame");	

		// Getting stuck here so causing decoder to deadlock?
		// Receive frame from decoder
		fprintf(stderr, "%s\n", "receiving frame from decoder");
		ret = avcodec_receive_frame(p_sys -> p_context, frame);
		if (ret != 0 && ret != AVERROR(EAGAIN)) {
			fprintf(stderr, "%s\n", "frame ret < 0");
			av_frame_free(&frame);
			if (ret == AVERROR(ENOMEM) || ret == AVERROR(EINVAL))
				return VLCDEC_ECRITICAL;
			if (ret == AVERROR_EOF)
				break;
		}
		fprintf(stderr, "av_get_pix_fmt_name(frame -> format): %s\n", av_get_pix_fmt_name(frame -> format));

		if (p_block) {
			p_block->p_buffer += i_used;
			p_block->i_buffer -= i_used;
		}

		if (ret) {
			av_frame_free(&frame);
			if (i_used == 0)
				break;
			continue;
		}

		vlc_mutex_lock(&p_sys->lock);

		int64_t av_pts = frame -> best_effort_timestamp;
		if (av_pts == AV_NOPTS_VALUE)
			av_pts = frame -> pkt_dts;

		vlc_tick_t i_pts;
		if (av_pts == AV_NOPTS_VALUE)
			i_pts = p_sys -> pts.date;
		else
			i_pts = av_rescale_q(av_pts, AV_TIME_BASE_Q, VLC_TIME_BASE_Q) + VLC_TICK_0;
		if (i_pts != VLC_TICK_INVALID)
			p_sys -> pts.date = i_pts;
		fprintf(stderr, "%s\n", "calculated i_pts");

		picture_t *p_pic = frame -> opaque;
		fprintf(stderr, "%s\n", "initialized p_pic");

		if (!p_pic) {

			if (decoder_UpdateVideoFormat(p_dec) == 0) {
				fprintf(stderr, "%s\n", "unchanged output format");
				p_pic = decoder_NewPicture(p_dec);
			}

			if (!p_pic) {
				fprintf(stderr, "%s\n", "p_pic is null, error");
				vlc_mutex_unlock(&p_sys -> lock);
				av_frame_free(&frame);
				break;
			}

			if (lavc_CopyPicture(p_dec, p_pic, frame) != VLC_SUCCESS) {
				fprintf(stderr, "%s\n", "p_pic unsuccessfully copied");
				vlc_mutex_unlock(&p_sys -> lock);
				av_frame_free(&frame);
				picture_Release(p_pic);
				break;
			}

		} else {

			fprintf(stderr, "%s\n", "proactive cloning");
			p_pic = picture_Clone(p_pic);
			if (unlikely(!p_pic)) {
				vlc_mutex_unlock(&p_sys->lock);
				av_frame_free(&frame);
				break;
			}

		}

		if (!p_dec -> fmt_in.video.i_sar_num || !p_dec -> fmt_in.video.i_sar_den) {
			fprintf(stderr, "%s\n", "fetching aspect ratio");
			p_dec -> fmt_out.video.i_sar_num
				= p_sys -> p_context -> sample_aspect_ratio.num;
			p_dec -> fmt_out.video.i_sar_den
				= p_sys -> p_context -> sample_aspect_ratio.den;

			if (!p_dec -> fmt_out.video.i_sar_num || !p_dec -> fmt_out.video.i_sar_den) {
				p_dec -> fmt_out.video.i_sar_num = 1;
				p_dec -> fmt_out.video.i_sar_den = 1;
			}
		}

		p_pic -> date = i_pts;
		
		av_frame_free(&frame);
		
		if (i_pts != VLC_TICK_INVALID) {
			// p_sys->b_first_frame = false;
			fprintf(stderr, "%s\n", "successing adding pic to video queue");
			vlc_mutex_unlock(&p_sys->lock);
			decoder_QueueVideo(p_dec, p_pic);
		}
		else {
			vlc_mutex_unlock(&p_sys->lock);
			picture_Release(p_pic);
		}
		
	}

	// I only need opencv for upscaling, not decoding
	// CvDecode_decode(p_sys -> v, p_block -> p_buffer, p_block -> i_buffer);

	/*
	
	std::vector<uchar> encoded_data(p_block -> p_buffer, p_block -> p_buffer + p_block -> i_buffer);
	cv::Mat decoded_frame = cv::imdecode(encoded_data, cv::IMREAD_UNCHANGED);

	if (decoded_frame.empty()) {
		msg_Err(p_dec, "Decoded frame is empty, OpenCV could not decode frame");
		return VLC_EGENERIC;
	}

	// Convert BGR to YUV420
	cv::Mat yuv_frame;
	cv::cvtColor(decoded_frame, yuv_frame, cv::COLOR_BGR2YUV_I420);

	// Byte size of frame is the number of array elements times the size of each element
	int required_size = yuv_frame.total() * yuv_frame.elemSize();
	if (required_size > i_buffer) {
		msg_Err(p_dec, "i_buffer size is insufficient for the required size of decoded_frame");
		return VLC_EGENERIC;
	}

	memcpy(p_block->p_buffer, yuv_frame.data, required_size);
	p_block -> i_buffer = required_size;

	*/

}

static void Close(vlc_object_t *p_this) {

	fprintf(stderr, "%s", "Close function\n");

	decoder_t *p_dec = (decoder_t *) p_this;
	decoder_sys_t *p_sys = p_dec -> p_sys;
	AVCodecContext *p_context = p_sys -> p_context;
	avcodec_free_context(&p_context);
	deleteCvDecode(p_sys -> v);
	free(p_dec -> p_sys);

}
