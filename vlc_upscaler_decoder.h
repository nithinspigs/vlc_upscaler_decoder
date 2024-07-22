/* 
 * Direct references and boilerplate code:
 * vlc/modules/codec/ddummy.c
*/

// Items to include for VLC plugin functionality
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>

// Items to include for OpenCV functionality
#include "./cvDecode/cvDecodeWrapper.h"

// Prototypes
static int Open(vlc_object_t *);
static int Close(vlc_object_t *);
static int Decode(decoder_t *, block_t *);

typedef struct {

	struct CvDecode *v;

} decoder_sys_t;
