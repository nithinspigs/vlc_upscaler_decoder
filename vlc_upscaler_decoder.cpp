/* 
 * Direct references and boilerplate code:
 * vlc/modules/codec/ddummy.c
*/

// Items to include for VLC plugin functionality
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>

// Items to include for OpenCV and CUDA functionality
#include <opencv2/opencv.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/cudawarping.hpp>

// Prototypes
static int Open(vlc_object_t *);
static int Close(vlc_object_t *);
static int Decode(decoder_t *, block_t *);

vlc_module_begin()
	set_shortname(N_("Upscaler Decoder"));
	set_description(N_("VLC plugin that decodes video and upscales it in real time"));
	set_capability("decoder", 0);
	set_callbacks(Open, Close);
vlc_module_end()

typedef struct {

	cv::Ptr<cv::cudacodec::VideoDecoder> nvdec; // NVDEC decoder

} decoder_sys_t;

static int Open(vlc_object_t *p_this) {

	decoder_t *p_dec = static_cast<decoder_t *>(p_this);
	decoder_sys_t *p_sys = new decoder_sys_t();
	if(!p_sys) {
		return VLC_ENOMEM;
	}

	p_dec -> p_sys = static_cast<void *>(p_sys);
	p_dec -> pf_decode = Decode;
	p_dec -> pf_flush = nullptr; // not needed?
	
	// p_sys -> nvdec = nullptr; // do i need to specify this is nullptr?
	// p_sys->nvdec = cv::cudacodec::createVideoDecoder(cv::cudacodec::Format::H264); // for now no nvdec

	return VLC_SUCCESS;

}

static int Decode(decode_t *p_dec, block_t *p_block) {

	// No data to decode
	if(!p_block) {
		return VLCDEC_SUCCESS;
	}

	// Just decode, no processing
	
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

	return VLCDEC_SUCCESS;

}

static int Close(vlc_object_t *p_this) {

	decoder_t *p_dec = static_cast<decoder_t *>(p_this);
	delete p_dec -> p_sys;

}
