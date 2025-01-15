#include "avcodec_helpers.h"

bool getFfmpegCodec(vlc_fourcc_t i_codec, enum AVCodecID *i_ffmpeg_codec) {

	for (int i = 0; i < sizeof(video_codecs) / sizeof(video_codecs[0]); i++) {

		if (i_codec == video_codecs[i].i_fourcc) {
			*i_ffmpeg_codec = video_codecs[i].i_codec;
			return true;
		}

	}

	return false;

}

int getVlcChroma(video_format_t *fmt, enum AVPixelFormat i_ffmpeg_chroma) {

	return 0;

}

bool chroma_compatible(const video_format_t *a, const video_format_t *b) {

	if (a->i_chroma != b->i_chroma)
		return false;
	/*
	if (a->color_range != b->color_range && a->color_range != COLOR_RANGE_UNDEF)
		return false;
	*/

	return true;
}

int lavc_CopyPicture(decoder_t *dec, picture_t *pic, AVFrame *frame) {

	video_format_t test_chroma;
	video_format_Init(&test_chroma, 0);
	if (getVlcChroma(&test_chroma, frame->format) != VLC_SUCCESS)
	{
		return VLC_EGENERIC;
	} else if (!chroma_compatible(&test_chroma, &pic->format)
	 /* ensure we never read more than dst lines/pixels from src */
	 || frame->width != (int) pic->format.i_visible_width
	 || frame->height < (int) pic->format.i_visible_height)
	{
		msg_Warn(dec, "dropping frame because the vout changed");
		return VLC_EGENERIC;
	}

	for (int plane = 0; plane < pic->i_planes; plane++)
	{
		const uint8_t *src = frame->data[plane];
		uint8_t *dst = pic->p[plane].p_pixels;
		size_t src_stride = frame->linesize[plane];
		size_t dst_stride = pic->p[plane].i_pitch;
		size_t size = __MIN(src_stride, dst_stride);

		for (int line = 0; line < pic->p[plane].i_visible_lines; line++)
		{
			memcpy(dst, src, size);
			src += src_stride;
			dst += dst_stride;
		}
	}
	return VLC_SUCCESS;

}
