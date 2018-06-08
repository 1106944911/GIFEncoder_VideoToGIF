#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>

#include "gif_encoder.h"

#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "avformat.lib" )	
#pragma comment( lib, "swscale.lib" )
#pragma comment( lib, "avutil.lib" )	

int main() {
	
	AVFormatContext			*pFormatCtx = NULL;
	int						i, videoStream;
	AVCodecContext			*pCodecCtx = NULL;
	AVCodec					*pCodec = NULL;
	AVFrame					*pFrame = NULL;
	AVFrame					*pFrameRGBA = NULL;
	AVPacket				packet;
	int						frameFinished;
	void* 					buffer;
	int						bufferSize;
	void*					bitmapBuffer;

	AVDictionary			*optionsDict = NULL;
	struct SwsContext		*sws_ctx = NULL;
	char					*videoFileName;

	GifInfo					*gifInfo = NULL;
	char					*gifFileName = NULL;

	// Register all formats and codecs
	av_register_all();

	// Video File Name
	videoFileName = "Test.mp4";

	// Open video file
	if (avformat_open_input(&pFormatCtx, videoFileName, NULL, NULL) != 0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, videoFileName, 0);

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1)
		return -1; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
		return -1; // Could not open codec

	// Allocate video frame
	pFrame = av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGBA = av_frame_alloc();
	if (pFrameRGBA == NULL)
		return -1;

	// GIF File Name
	gifFileName = "Test.gif";

	// Initialize GIF Encoder
	gifInfo = init(gifInfo, pCodecCtx->width, pCodecCtx->height, gifFileName);
	
	if(gifInfo == NULL)
		return -1; // Fail to initialize gif encoder

	// Create a bitmap as the buffer for pFrameRGBA
	bufferSize = avpicture_get_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);
	buffer = (void*)malloc(sizeof(uint8_t) * bufferSize);

	//get the scaling context
	sws_ctx = sws_getContext
	(
		pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_RGBA,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);

	// Assign appropriate parts of bitmap to image planes in pFrameRGBA
	// Note that pFrameRGBA is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGBA, buffer, AV_PIX_FMT_RGBA,
		pCodecCtx->width, pCodecCtx->height);

	// Read frames and save first five frames to disk
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				&packet);
			// Did we get a video frame?
			if (frameFinished) {
				// Convert the image from its native format to RGBA
				sws_scale
				(
					sws_ctx,
					(uint8_t const * const *)pFrame->data,
					pFrame->linesize,
					0,
					pCodecCtx->height,
					pFrameRGBA->data,
					pFrameRGBA->linesize
				);

				uint32_t* tempPixels = (uint32_t*)malloc((pCodecCtx->width * pCodecCtx->height) * sizeof(uint32_t));
				int stride = pCodecCtx->width * 4;
				int pixelsCount = stride * pCodecCtx->height;
				memcpy(tempPixels, buffer, pixelsCount);

				basicReduceColor(gifInfo, tempPixels);
				writeNetscapeExt(gifInfo);
				graphicsControlExtension(gifInfo, 0);
				imageDescriptor(gifInfo);
				imageData(gifInfo, tempPixels);

			}
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}

	// Free buffer
	free(buffer);
	// Free the RGB image
	av_free(pFrameRGBA);
	// Free the YUV frame
	av_free(pFrame);
	// Close the codec
	avcodec_close(pCodecCtx);
	// Close the video file
	avformat_close_input(&pFormatCtx);
	
	finish(gifInfo);

	return 0;
}
