/* 
 * Direct references and boilerplate code:
 * vlc/modules/codec/ddummy.c
*/

// Items to include for VLC plugin functionality
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
//#include <vlc_avcodec.h>
#include <vlc_cpu.h>
// #include <vlc_ancillary.h>
#include <assert.h>

// Items to include for FFMPEG libavcodec functionality
#include "libavcodec/avcodec.h"
#include <libavutil/mem.h>
#include <libavutil/pixdesc.h>
#include <libavutil/mastering_display_metadata.h>

// Items to include for VLC avcodec plugin functionality
//#include "modules/codec/avcodec/avcodec.h"

// Items to include for OpenCV functionality
#include "./cvDecode/cvDecodeWrapper.h"

// Prototypes
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);
static int Decode(decoder_t *, block_t *);


typedef struct decoder_sys_t {

	struct CvDecode *v;
	AVCodecContext *p_context;
	const AVCodec *p_codec;

} decoder_sys_t;

