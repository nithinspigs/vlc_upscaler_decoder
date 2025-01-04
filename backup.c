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
	add_bool("test boolean setting", false, "test boolean short", "test boolean long text")
vlc_module_end()

static int Open(vlc_object_t *p_this) {

	fprintf(stderr, "%s", "Open function\n");

	// Do I need p_dec -> p_sys?
	decoder_t *p_dec = (decoder_t *) p_this;
	decoder_sys_t *p_sys = malloc(sizeof(decoder_sys_t));
	if(!p_sys) {
		return VLC_ENOMEM;
	}

	const AVCodec *p_codec;
	AVCodecContext *p_context = ffmpeg_AllocContext(p_dec, &p_codec);
	if(p_context == NULL) {
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
	fprintf(stderr, "%s%u\n", "p_dec -> fmt_in -> i_codec: ", p_dec -> fmt_in -> i_codec);
	fprintf(stderr, "%s%s\n", "fourcc description: ", vlc_fourcc_GetDescription(VIDEO_ES, p_dec -> fmt_in -> i_codec));

	/*
	// No data to decode
	if(!p_block) {
		return VLCDEC_SUCCESS;
	}
	*/

	/* Set output properties */
	p_dec -> fmt_out.video.i_chroma =
	p_dec -> fmt_out.i_codec = VLC_CODEC_RGB24;
	p_dec -> fmt_out.video.i_sar_num = 1;
	p_dec -> fmt_out.video.i_sar_den = 1;
	p_dec -> fmt_out.video.i_visible_width = p_dec -> fmt_out.video.i_width = p_dec -> fmt_in -> video.i_width;
	p_dec -> fmt_out.video.i_visible_height = p_dec -> fmt_out.video.i_height = p_dec -> fmt_in -> video.i_height;

	// Just decode, no processing
	decoder_UpdateVideoFormat(p_dec);
	picture_t *p_pic = decoder_NewPicture(p_dec);
	fprintf(stderr, "%s%u\n", "p_pic -> format.i_chroma: ", p_pic -> format.i_chroma);
	fprintf(stderr, "%s%s\n", "fourcc description: ", vlc_fourcc_GetDescription(VIDEO_ES, p_pic -> format.i_chroma));
	/*
	p_pic -> format.i_chroma: 875714130
	fourcc description: 24 bits RGB
	*/

	decoder_sys_t *p_sys = p_dec -> p_sys;
	
	// Decode block using ffmpeg
	fprintf(stderr, "%s%s\n", "p_dec -> p_sys -> p_codec.long_name: ", p_sys -> p_codec -> long_name);	
	AVFrame *frame = av_frame_alloc();
	fprintf(stderr, "%s\n", "allocated frame");
	AVPacket *packet = av_packet_alloc();
	fprintf(stderr, "%s\n", "declared packet");

	// Prepare the packet with the data from p_buffer
	av_init_packet(packet);
	fprintf(stderr, "%s\n", "initialized packet");
	packet -> data = p_block -> p_buffer;
	packet -> size = p_block -> i_buffer;
	fprintf(stderr, "%s\n", "edited packet data and size");

	// Send packet to decoder
	int ret = avcodec_send_packet(p_sys -> p_context, packet);
	
	fprintf(stderr, "%s\n", "sent packet to decoder");
	if (ret < 0) {
		// goto error;
		fprintf(stderr, "%s\n", "packet ret < 0");
	}

	ret = avcodec_receive_frame(p_sys -> p_context, frame);
	fprintf(stderr, "%s\n", "received frame from decoder");
	if (ret < 0) {
		// goto error;
		fprintf(stderr, "%s\n", "frame ret < 0");
	}

	fprintf(stderr, "%s\n", av_get_pix_fmt_name(frame -> format));

	// I only need opencv for upscaling, not decoding
	// CvDecode_decode(p_sys -> v, p_block -> p_buffer, p_block -> i_buffer);

	/*
	
	std::vector<uchar> encoded_data(p_block -> p_buffer, p_block -> p_buffer + p_block -> i_buffer);
	cv::Mat decoded_frame = cv::imdecode(encoded_data, cv::IMREAD_UNCHANGED);

	if(decoded_frame.empty()) {
		msg_Err(p_dec, "Decoded frame is empty, OpenCV could not decode frame");
		return VLC_EGENERIC;
	}

	// Convert BGR to YUV420
	cv::Mat yuv_frame;
	cv::cvtColor(decoded_frame, yuv_frame, cv::COLOR_BGR2YUV_I420);

	// Byte size of frame is the number of array elements times the size of each element
	int required_size = yuv_frame.total() * yuv_frame.elemSize();
	if(required_size > i_buffer) {
		msg_Err(p_dec, "i_buffer size is insufficient for the required size of decoded_frame");
		return VLC_EGENERIC;
	}

	memcpy(p_block->p_buffer, yuv_frame.data, required_size);
	p_block -> i_buffer = required_size;

	*/

	decoder_QueueVideo(p_dec, p_pic);

error:
	printf("%s", "Reached error");
	av_frame_free(&frame);
	av_packet_free(&packet);
	block_Release(p_block);
	return VLCDEC_SUCCESS;

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
