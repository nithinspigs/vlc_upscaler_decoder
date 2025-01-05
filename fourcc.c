#include "fourcc.h"

bool getFfmpegCodec(vlc_fourcc_t i_codec, enum AVCodecID *i_ffmpeg_codec) {

	for (int i = 0; i < sizeof(video_codecs) / sizeof(video_codecs[0]); i++) {

		if (i_codec == video_codecs[i].i_fourcc) {
			*i_ffmpeg_codec = video_codecs[i].i_codec;
			return true;
		}

	}

	return false;

}
