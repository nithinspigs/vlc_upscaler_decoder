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

#define MODULE_STRING "VLC Upscaler Decoder"

vlc_module_begin()
	set_shortname("VLC Upscaler Decoder")
	set_description("VLC plugin that decodes video and upscales it in real time")
	set_capability("decoder", 0)
	set_callbacks(Open, Close)
vlc_module_end()

static int Open(vlc_object_t *p_this) {

	// Do I need p_dec -> p_sys?
	decoder_t *p_dec = (decoder_t *) p_this;
	decoder_sys_t *p_sys = malloc(sizeof(decoder_sys_t));
	if(!p_sys) {
		return VLC_ENOMEM;
	}

	p_dec -> p_sys = (void *) p_sys;
	p_sys -> v = newCvDecode();
	p_dec -> pf_decode = Decode;
	p_dec -> pf_flush = NULL; // not needed?
	
	// p_sys -> nvdec = nullptr; // do i need to specify this is nullptr?
	// p_sys->nvdec = cv::cudacodec::createVideoDecoder(cv::cudacodec::Format::H264); // for now no nvdec

	return VLC_SUCCESS;

}

static int Decode(decoder_t *p_dec, block_t *p_block) {

	printf("%s", "Verify that decoder plugin is running");

	// No data to decode
	if(!p_block) {
		return VLCDEC_SUCCESS;
	}

	// Just decode, no processing
	// picture_t p_pic;
	picture_t *p_pic; // = decoder_NewPicture(p_dec);
	// Find a way to not pass VLC structs to opencv, cuz g++ cannot compile VLC header files properly
	decoder_sys_t *p_sys = (decoder_sys_t *) p_dec -> p_sys;
	CvDecode_decode(p_sys -> v, p_block -> p_buffer, p_block -> i_buffer);

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
	// block_Release can't be identified by linked idk why
	// block_Release(p_block);

	return VLCDEC_SUCCESS;

}

static void Close(vlc_object_t *p_this) {

	decoder_t *p_dec = (decoder_t *) p_this;
	decoder_sys_t *p_sys = p_dec -> p_sys;
	deleteCvDecode(p_sys -> v);
	free(p_dec -> p_sys);

}
