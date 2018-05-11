#ifndef COMMENDEF_H
#define COMMENDEF_H
#include "stdafx.h"
#include "pktqueue.h"

#define AV_NO_PTS          0			        //没有显示时间撮PTS

#define AV_NOT			   ( 0 )				//媒体文件初始类型，代表没有视频、音频、字幕等
#define AV_AUDIO           ( 1 )		    //音频标志
#define AV_VIDEO           ( AV_AUDIO << 1 )	    //视频标志

typedef struct tagMediaParam
{
	AVFormatContext* pfmtCtx;
	CPktQueue* ppq;

	//video param
	AVCodecContext* pvCodecCtx;
	AVCodec* pvCodec;
	int videoIndex;

	//audio param
	AVCodecContext* paCodecCtx;
	AVCodec* paCodec;
	int audioIndex;

	//media file type
	int nMediaType;

	//thread handle
	HANDLE hVideoThread;
	HANDLE hAudioThread;

	tagMediaParam() {
		memset(this, 0, sizeof(tagMediaParam));
	}
}MediaParam;

struct tagbuff
{
	uint8_t *buffer;
	tagbuff() {
		buffer = NULL;
	}
};



#endif
