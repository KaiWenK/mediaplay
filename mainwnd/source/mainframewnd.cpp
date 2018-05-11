#include "stdafx.h"
#include "mainframewnd.h"
#include <mmsystem.h>  
#pragma comment(lib, "winmm.lib") 

DUI_BEGIN_MESSAGE_MAP( CMainFramWnd, WindowImplBase )
	DUI_ON_CLICK_CTRNAME( _T(""), OnClose )

DUI_END_MESSAGE_MAP()

HANDLE CMainFramWnd::m_audiolock = CreateSemaphore(NULL, 4, 4, NULL);

CMainFramWnd::CMainFramWnd()
: m_pPlayWnd(NULL)
{
	memset( &m_AudioCtx, 0, sizeof(m_AudioCtx) );
	memset(&m_VideoCtx, 0, sizeof(m_VideoCtx));
	m_VideoCtx.start_pts = AV_NOPTS_VALUE;
}

void CMainFramWnd::Demutex()
{
	av_register_all();
	av_log_set_callback(avlog);
	av_log_set_level(AV_LOG_DEBUG);
	
	AVPacket * packet = NULL;
	m_MediaParam.ppq = new CPktQueue;
	if (!m_MediaParam.ppq->Init())
	{
		return;
	}
	m_MediaParam.pfmtCtx = avformat_alloc_context();

	if ( avformat_open_input(&m_MediaParam.pfmtCtx, "./说散就散.mp4", NULL, NULL) < 0 )
	{
		av_log( NULL, AV_LOG_ERROR, "file open faild" );
		return;
	}
	if ( avformat_find_stream_info(m_MediaParam.pfmtCtx, NULL) <0 )
	{
		av_log(NULL, AV_LOG_ERROR, "find stream faild");
		return;
	}

	for ( int i=0; i<(int)m_MediaParam.pfmtCtx->nb_streams; i++ )
	{
		if ( m_MediaParam.pfmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
		{
			if ( InitStream( AVMEDIA_TYPE_VIDEO, i ) )
			{
				m_MediaParam.nMediaType |= AV_VIDEO;
			}
		}
		else if( m_MediaParam.pfmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO )
		{
			if ( InitStream( AVMEDIA_TYPE_AUDIO, i ) )
			{
				m_MediaParam.nMediaType |= AV_AUDIO;
			}
		}
	}

	if ((m_MediaParam.nMediaType & AV_VIDEO) == AV_VIDEO)
	{
		m_MediaParam.hVideoThread = (HANDLE)_beginthreadex(NULL, 0, VideoDecodeThread, this, 0, NULL);
	}

	if ((m_MediaParam.nMediaType & AV_VIDEO) == AV_VIDEO)
	{
		m_MediaParam.hAudioThread = (HANDLE)_beginthreadex(NULL, 0, AudioDecodeThread, this, 0, NULL);
	}

	while ( true )
	{
		packet = m_MediaParam.ppq->FreePktDequeue();
		if (packet == NULL)
		{
			av_usleep(20 * 1000);
			continue;
		}
		if (av_read_frame(m_MediaParam.pfmtCtx, packet) >= 0)
		{
			if (packet->stream_index == m_MediaParam.videoIndex )
			{
				m_MediaParam.ppq->VideoPktEnqueue(packet);
			}
			else if (packet->stream_index == m_MediaParam.audioIndex)
			{
				m_MediaParam.ppq->AudioPktEnqueue(packet);
			}
			else
			{
				av_packet_unref(packet); // free packet
				m_MediaParam.ppq->FreePktCancel(packet);
			}
		}
		else
		{
			av_packet_unref(packet); // free packet
			m_MediaParam.ppq->FreePktCancel(packet);
			//avformat_close_input(&m_MediaParam.pfmtCtx);
			//avlog( NULL, AV_LOG_INFO, "read frame is end or error", NULL );
			continue;
		}
	}
}

void CMainFramWnd::InitMediaParam()
{
	m_MediaParam.pfmtCtx = NULL;
	m_MediaParam.ppq = NULL;

	//video param
	m_MediaParam.pvCodecCtx = NULL;
	m_MediaParam.pvCodec = NULL;
	m_MediaParam.videoIndex = -1;

	//audio param
	m_MediaParam.paCodecCtx = NULL;
	m_MediaParam.paCodec = NULL;
	m_MediaParam.audioIndex = -1;
}

void CMainFramWnd::DestoryMediaParam()
{

	WaitForSingleObject(m_MediaParam.hVideoThread, INFINITE);
	WaitForSingleObject(m_MediaParam.hAudioThread, INFINITE);

	//释放媒体相关参数内存空间
	avformat_free_context( m_MediaParam.pfmtCtx );
	avcodec_free_context(  &m_MediaParam.pvCodecCtx );
	avcodec_free_context(  &m_MediaParam.paCodecCtx );
	delete m_MediaParam.ppq;
}

BOOL CMainFramWnd::InitStream(AVMediaType mType, int StreamIndex)
{
	BOOL bInitSuccess = FALSE;
	switch ( mType )
	{
	case AVMEDIA_TYPE_VIDEO:
	{
		//视频
		m_MediaParam.pvCodecCtx = avcodec_alloc_context3(NULL);

		if (avcodec_parameters_to_context(m_MediaParam.pvCodecCtx
			, m_MediaParam.pfmtCtx->streams[StreamIndex]->codecpar) < 0)
		{
			av_log( NULL, AV_LOG_ERROR, "video avcodec_parameters_to_context faild" );
		}

		m_MediaParam.pvCodec = avcodec_find_decoder(m_MediaParam.pvCodecCtx->codec_id);
		if ( m_MediaParam.pvCodec == NULL )
		{
			av_log(NULL, AV_LOG_ERROR, "find video decoder faild");
		}

		if (!avcodec_open2(m_MediaParam.pvCodecCtx, m_MediaParam.pvCodec, NULL))
		{
			m_MediaParam.videoIndex = StreamIndex;
			bInitSuccess = TRUE;
		}
		else
		{
			av_log(NULL, AV_LOG_ERROR, "video decodec open faild");
		}
	}
	break;
	case AVMEDIA_TYPE_AUDIO:
	{
		//音频
		m_MediaParam.paCodecCtx = avcodec_alloc_context3(NULL);
		if (avcodec_parameters_to_context(m_MediaParam.paCodecCtx
			, m_MediaParam.pfmtCtx->streams[StreamIndex]->codecpar) < 0)
		{
			av_log( NULL, AV_LOG_ERROR, "audio avcodec_parameters_to_context faild" );
		}

		m_MediaParam.paCodec = avcodec_find_decoder(m_MediaParam.paCodecCtx->codec_id);
		if (m_MediaParam.paCodec == NULL)
		{
			av_log( NULL, AV_LOG_ERROR, "find audio decodec faild" );
		}

		if (!avcodec_open2(m_MediaParam.paCodecCtx, m_MediaParam.paCodec, NULL))
		{
			m_MediaParam.audioIndex = StreamIndex;
			bInitSuccess = TRUE;
		}
		else
		{
			av_log( NULL, AV_LOG_ERROR, "open audio decodec faild" );
		}
	}
	break;
	default:
		break;
	}

	return bInitSuccess;
}

void CMainFramWnd::avlog( void* p, int n, const char* str, va_list vlist )
{
	int n1 = n;
	char strl[1000] = { 0 };
	int len = strlen(str) > 1000 ? 1000 : strlen(str);
	memcpy(&strl, str, len);
	if (len = 0)
	{
		strl[0] = '\r';
		strl[1] = '\n';
	}
	else if(len == 1)
	{
		strl[1] = '\r';
		strl[2] = '\n';
	}
	else if(len>1&&len<998)
	{
		strl[len] = '\r';
		strl[len + 1] = '\n';
	}
	else
	{
		return;
	}
	va_list list = vlist;

	if ( n <= AV_LOG_ERROR )
	{
		FILE *pf = NULL;
		pf = _tfopen(_T("avlog.txt"), _T("a"));

		fwrite((void*)strl, 1, len, pf);

		fclose(pf);
	}
}

void CMainFramWnd::InitWindow()
{
	m_pPlayWnd = static_cast<CControlWndUI*>(m_PaintManager.FindControl(_T("PlayWnd")));

}

CMainFramWnd::~CMainFramWnd()
{

}

unsigned int CMainFramWnd::DemutexThread(LPVOID p)
{
	CMainFramWnd* pMain = (CMainFramWnd*)p;
	pMain->Demutex();
	return 0;
}

unsigned int CMainFramWnd::VideoDecodeThread(LPVOID p)
{
	CMainFramWnd* pMain = (CMainFramWnd*)p;
	VideoCtx* videoCtx = &pMain->m_VideoCtx;
	const MediaParam *pmediaParam = pMain->getMediaParam();
	unsigned char *out_bufRgb;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameYUV = NULL;

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	const CControlWndUI *pWnd = pMain->GetPlayWnd();
	RECT rc;
	GetWindowRect(pWnd->GetHWND(), &rc);

	videoCtx->surface = pWnd->GetHWND();
	videoCtx->bufnum = 3;
	videoCtx->pixfmt = AV_PIX_FMT_RGB32;
	videoCtx->w = rc.right-rc.left;
	videoCtx->h = rc.bottom-rc.top;
	videoCtx->sw = rc.right - rc.left;
	videoCtx->sh = rc.bottom - rc.top;
	videoCtx->tickframe = 1000 / 25;
	videoCtx->ticksleep = videoCtx->tickframe;
	videoCtx->apts = -1;
	videoCtx->vpts = -1;

	// alloc buffer & semaphore
	videoCtx->ppts = (int64_t*)calloc(videoCtx->bufnum, sizeof(int64_t));
	videoCtx->hbitmaps = (HBITMAP*)calloc(videoCtx->bufnum, sizeof(HBITMAP));
	videoCtx->pbmpbufs = (BYTE**)calloc(videoCtx->bufnum, sizeof(BYTE*));

	// create semaphore
	videoCtx->semr = CreateSemaphore(NULL, 0, videoCtx->bufnum, NULL);
	videoCtx->semw = CreateSemaphore(NULL, videoCtx->bufnum, videoCtx->bufnum, NULL);

	videoCtx->hdcdst = GetDC((HWND)pWnd->GetHWND());
	videoCtx->hdcsrc = CreateCompatibleDC(videoCtx->hdcdst);

	//同步
	(pMain->m_AudioCtx).apts = &videoCtx->apts;

	_beginthreadex(NULL, 0, VideoRenderThread, p, 0, NULL);

	//申请用于存储RGB数据的buffer
	out_bufRgb = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32
		, rc.right - rc.left
		, rc.bottom - rc.top
		, 1));

	//pFrameYUV->data = out_bufRGB 
	//av_image_fill_arrays(pFrameYUV->data
	//	, pFrameYUV->linesize, out_bufRgb
	//	, AV_PIX_FMT_BGR24, rc.right - rc.left
	//	, rc.bottom - rc.top, 1);

	SwsContext* img_convert_ctx = sws_getContext(pmediaParam->pvCodecCtx->width, pmediaParam->pvCodecCtx->height
		, pmediaParam->pvCodecCtx->pix_fmt, rc.right - rc.left, rc.bottom - rc.top
		, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

	//PVOID pbit = NULL;
	//BITMAP bmp = { 0 };
	//HBITMAP hBitmap = NULL;
	//BITMAPINFO bmpinfo = { 0 };
	//bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//bmpinfo.bmiHeader.biWidth = rc.right - rc.left;
	//bmpinfo.bmiHeader.biHeight = -rc.bottom + rc.top;
	//bmpinfo.bmiHeader.biPlanes = 1;
	//bmpinfo.bmiHeader.biBitCount = 24;
	//bmpinfo.bmiHeader.biCompression = BI_RGB;

	int  errcode1 = 0;
	int  errcode2 = 0;
	while (true)
	{
		AVPacket* packet = pMain->m_MediaParam.ppq->VideoPktDequeue();
		if (packet == NULL)
		{
			av_usleep(20*1000);
			continue;
		}
		errcode1 = avcodec_send_packet(pMain->m_MediaParam.pvCodecCtx, packet);
		if( errcode1 == 0 )
		{
			errcode2 = avcodec_receive_frame(pMain->m_MediaParam.pvCodecCtx, pFrame);
			if ( errcode2 == 0 )
			{
				WaitForSingleObject( videoCtx->semw, INFINITE );

				pFrame->pts = av_rescale_q( av_frame_get_best_effort_timestamp(pFrame)
					, pmediaParam->pfmtCtx->streams[pmediaParam->videoIndex]->time_base, {1,1000} );

				BITMAP bitmap;
				int bmpw = 0;
				int bmph = 0;
				if (videoCtx->hbitmaps[videoCtx->tail]) {
					GetObject(videoCtx->hbitmaps[videoCtx->tail], sizeof(BITMAP), &bitmap);
					bmpw = bitmap.bmWidth;
					bmph = bitmap.bmHeight;
				}

				if (bmpw != videoCtx->w || bmph != videoCtx->h) {
					videoCtx->sw = videoCtx->w; videoCtx->sh = videoCtx->h;
					if (videoCtx->hbitmaps[videoCtx->tail]) {
						DeleteObject(videoCtx->hbitmaps[videoCtx->tail]);
					}

					BITMAPINFO bmpinfo = { 0 };
					bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmpinfo.bmiHeader.biWidth = videoCtx->w;
					bmpinfo.bmiHeader.biHeight = -videoCtx->h;
					bmpinfo.bmiHeader.biPlanes = 1;
					bmpinfo.bmiHeader.biBitCount = 32;
					bmpinfo.bmiHeader.biCompression = BI_RGB;
					videoCtx->hbitmaps[videoCtx->tail] = CreateDIBSection(videoCtx->hdcsrc, &bmpinfo, DIB_RGB_COLORS,
						(void**)&videoCtx->pbmpbufs[videoCtx->tail], NULL, 0);
					GetObject(videoCtx->hbitmaps[videoCtx->tail], sizeof(BITMAP), &bitmap);
				}

				av_image_fill_arrays(pFrameYUV->data
					, pFrameYUV->linesize, videoCtx->pbmpbufs[videoCtx->tail]
					, AV_PIX_FMT_RGB32, videoCtx->w
					, videoCtx->h, 1);

				if (pFrameYUV->data[0] && pFrame->pts != -1) {
					sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize
						, 0, pmediaParam->pvCodecCtx->height,
						pFrameYUV->data, pFrameYUV->linesize);
				}

				videoCtx->ppts[videoCtx->tail] = pFrame->pts;
				if (++videoCtx->tail == videoCtx->bufnum)
					videoCtx->tail = 0;
				ReleaseSemaphore(videoCtx->semr, 1, NULL);

				//hBitmap = CreateDIBSection( videoCtx.hdcsrc, &bmpinfo, DIB_RGB_COLORS, &pbit, NULL, 0);
				//GetObject( hBitmap, sizeof(BITMAP), &bmp );
				//memcpy( pbit, out_bufRgb, (rc.right - rc.left)*(rc.bottom-rc.top)*3 );
				//SelectObject(videoCtx.hdcsrc, hBitmap);
				//BitBlt( videoCtx.hdcsrc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, videoCtx.hdcsrc, 0, 0, SRCCOPY);
				//DeleteObject(hBitmap);

			}
			else if( AVERROR_EOF == errcode2 )
			{
				break;
			}
		}
		else if (AVERROR_EOF == errcode1)
		{
			break;
		}

		av_packet_unref(packet);
		pmediaParam->ppq->FreePktEnqueue(packet);
	}

	return 0;
}

unsigned int CMainFramWnd::AudioDecodeThread(LPVOID p)
{
	CMainFramWnd* pMain = (CMainFramWnd*)p;
	AudioCtx* pAuCtx = &pMain->m_AudioCtx;
	MediaParam *pmediaParam = pMain->getMediaParam();
	BYTE         *pwavbuf;

	int sendpkterr = 0;
	int recvfrmerr = 0;
	int channel_layout = 0;
	AVFrame *paframe;
	
	paframe = av_frame_alloc();

	int bufnum = 5;
	int buflen = (int)((double)44100 * 1 / 25 + 0.5) * 4;

	pAuCtx->bufnum = 5;
	pAuCtx->buflen = buflen;
	pAuCtx->head = 0;
	pAuCtx->tail = 0;
	pAuCtx->ppts = (int64_t*)calloc(pAuCtx->bufnum, sizeof(int64_t));
	pAuCtx->pWaveHdr = (WAVEHDR*)calloc(pAuCtx->bufnum, (sizeof(WAVEHDR) + pAuCtx->buflen));
	pAuCtx->bufsem = CreateSemaphore(NULL, pAuCtx->bufnum, pAuCtx->bufnum, NULL);
	pAuCtx->curdata = (int16_t*)calloc(1, pAuCtx->buflen);
	if (!pAuCtx->ppts || !pAuCtx->pWaveHdr || !pAuCtx->bufsem || !pAuCtx->curdata) {
		av_log(NULL, AV_LOG_ERROR, "failed to allocate waveout buffer and waveout semaphore !\n");
		exit(0);
	}

	SwrContext *swrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
		av_get_default_channel_layout(pmediaParam->paCodecCtx->channels),
		pmediaParam->paCodecCtx->sample_fmt, pmediaParam->paCodecCtx->sample_rate, 0, NULL);
	swr_init(swrCtx);

	//int outsize = av_samples_get_buffer_size(NULL, 2, 44100 / 2, AV_SAMPLE_FMT_S16, 0);
	//unsigned int outcount = swr_get_out_samples(swrCtx, 44100);
	//unsigned char * pout = NULL;
	////av_fast_malloc( pout, &outcount, outsize);

	////转码输出后的参数
	//uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO; 	//采样的布局方式
	//int out_nb_samples = 1024;							//采样个数
	//enum AVSampleFormat  sample_fmt = AV_SAMPLE_FMT_S16; 	//采样格式
	//int out_sample_rate = 44100;							//采样率
	//int out_channels = av_get_channel_layout_nb_channels(out_channel_layout); 	//通道数
	//int buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, sample_fmt, 1); 	//创建buffer

	//int64_t in_channel_layout = av_get_default_channel_layout(pmediaParam->paCodecCtx->channels);

	int nChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

	WAVEFORMATEX waveFormatex;
	waveFormatex.cbSize = sizeof(waveFormatex);
	waveFormatex.wFormatTag = WAVE_FORMAT_PCM;     //波形声音的格式,单声道双声道使用WAVE_FORMAT_PCM.当包含在WAVEFORMATEXTENSIBLE结构中时,使用WAVE_FORMAT_EXTENSIBLE
	waveFormatex.wBitsPerSample = 16;              //采样位数.wFormatTag为WAVE_FORMAT_PCM时,为8或者16
	waveFormatex.nSamplesPerSec = 44100;		   //采样率.wFormatTag为WAVE_FORMAT_PCM时, 有8.0kHz, 11.025kHz, 22.05kHz, 和44.1kHz
	waveFormatex.nChannels = nChannels;					   //声道数量
	waveFormatex.nBlockAlign = waveFormatex.nChannels * waveFormatex.wBitsPerSample / 8; //每次采样的字节数.通过nChannels * wBitsPerSample / 8计算
	waveFormatex.nAvgBytesPerSec = waveFormatex.nBlockAlign * waveFormatex.nSamplesPerSec; //每秒的采样字节数.通过nSamplesPerSec * nChannels * wBitsPerSample / 8计算  

	if (waveOutOpen((LPHWAVEOUT)&pAuCtx->hWaveOut, WAVE_MAPPER, &waveFormatex
		, (DWORD_PTR)AudioCallBack, (DWORD_PTR)pAuCtx, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		avlog(NULL, AV_LOG_ERROR, "audio out open faild", NULL);
		return 0;
	}

	pwavbuf = (BYTE*)(pAuCtx->pWaveHdr + pAuCtx->bufnum);
	for (int i = 0; i < pAuCtx->bufnum; i++) {
		pAuCtx->pWaveHdr[i].lpData = (LPSTR)(pwavbuf + i * buflen);
		pAuCtx->pWaveHdr[i].dwBufferLength = buflen;
		waveOutPrepareHeader(pAuCtx->hWaveOut, &pAuCtx->pWaveHdr[i], sizeof(WAVEHDR));
	}

	//返回每个通道输出的样本数
	int nLen = 0;
	int adev_buf_avail = 0;
	uint8_t* adev_buf_cur = 0;
	int64_t apts = AV_NOPTS_VALUE;
	while (true)
	{
		AVPacket *pkt = pMain->m_MediaParam.ppq->AudioPktDequeue();
		if (pkt == NULL)
		{
			av_usleep(20*1000);
			continue;
		}
		sendpkterr = avcodec_send_packet(pMain->m_MediaParam.paCodecCtx, pkt);
		if (sendpkterr == 0)
		{
			while (true)
			{
				recvfrmerr = avcodec_receive_frame(pMain->m_MediaParam.paCodecCtx, paframe);
				if (recvfrmerr == 0)
				{
					apts = paframe->pts;
					AVRational tb_sample_rate = { 1, pMain->m_MediaParam.paCodecCtx->sample_rate };
					if (apts == AV_NOPTS_VALUE) {
						apts = av_rescale_q(paframe->pts, av_codec_get_pkt_timebase(pMain->m_MediaParam.paCodecCtx), tb_sample_rate);
					}
					else {
						apts += paframe->nb_samples;
					}
					paframe->pts = av_rescale_q(apts, tb_sample_rate, {1,1000});
					
					int sampnum = 0;
					apts = paframe->pts;
					do {
						if (adev_buf_avail == 0) {
							WaitForSingleObject(pAuCtx->bufsem, -1);
							apts += 40;
							adev_buf_avail = (int)(pAuCtx->pWaveHdr[pAuCtx->tail].dwBufferLength);
							adev_buf_cur = (uint8_t*)pAuCtx->pWaveHdr[pAuCtx->tail].lpData;
						}
						sampnum = swr_convert(swrCtx,
							(uint8_t**)&adev_buf_cur, adev_buf_avail / 4,
							(const uint8_t**)paframe->extended_data, paframe->nb_samples);
						paframe->extended_data = NULL;
						paframe->nb_samples = 0;
						adev_buf_avail -= sampnum * 4;
						adev_buf_cur += sampnum * 4;

						if (adev_buf_avail == 0) {
							pAuCtx->ppts[pAuCtx->tail] = apts;
							int16_t *buf = (int16_t*)pAuCtx->pWaveHdr[pAuCtx->tail].lpData;
							int      n = pAuCtx->pWaveHdr[pAuCtx->tail].dwBufferLength / sizeof(int16_t);
							waveOutWrite(pAuCtx->hWaveOut, &pAuCtx->pWaveHdr[pAuCtx->tail], sizeof(WAVEHDR));
							if (++pAuCtx->tail == pAuCtx->bufnum)
								pAuCtx->tail = 0;
						}
					} while (sampnum > 0);
		
				}
				else if (AVERROR_EOF == recvfrmerr)
				{
					//解码到结尾了
					return 0;
				}
				else if (AVERROR(EAGAIN) == recvfrmerr)
				{
					//输出状态不可用，用户必须尝试发送新的输入
					av_packet_unref(pkt);
					pmediaParam->ppq->FreePktEnqueue(pkt);
					break;
				}
				else if (AVERROR(EINVAL) == recvfrmerr)
				{
					//编解码器错误
					return 0;
				}
			}			
		}
		else if (AVERROR_EOF == sendpkterr)
		{
			//文件结尾
			return 0;
		}
		else if (AVERROR(EAGAIN) == sendpkterr)
		{
			//当前输入状态是不可接收的，用户必须读取输出
			continue;
		}
		else if (AVERROR(EINVAL) == sendpkterr)
		{
			//编解码器错误
			break;
		}
		//av_packet_unref(pkt);
		//pmediaParam->ppq->FreePktEnqueue(pkt);
	}

	return 0;
}

unsigned int CALLBACK CMainFramWnd::VideoRenderThread(LPVOID p)
{
	CMainFramWnd* pMain = (CMainFramWnd*)p;
	VideoCtx* pvideoCtx = &pMain->m_VideoCtx;
	const MediaParam *pmediaParam = pMain->getMediaParam();

	while (true) {
		WaitForSingleObject(pvideoCtx->semr, INFINITE);

		int64_t vpts = pvideoCtx->vpts = pvideoCtx->ppts[pvideoCtx->head];
		if (vpts != -1) {
			SelectObject(pvideoCtx->hdcsrc, pvideoCtx->hbitmaps[pvideoCtx->head]);
			BitBlt(pvideoCtx->hdcdst, pvideoCtx->x, pvideoCtx->y, pvideoCtx->w, pvideoCtx->h, pvideoCtx->hdcsrc, 0, 0, SRCCOPY);
		}

		av_log(NULL, AV_LOG_DEBUG, "vpts: %lld\n", vpts);
		if (++pvideoCtx->head == pvideoCtx->bufnum) pvideoCtx->head = 0;

		ReleaseSemaphore(pvideoCtx->semw, 1, NULL);

		// handle av-sync & frame rate & complete
		VideoSyncComplete(*pvideoCtx);
	}

	return NULL;
}

void CMainFramWnd::VideoSyncComplete(VideoCtx& videoCtx)
{
	int     tickdiff, scdiff, avdiff = -1;
	int64_t tickcur;

	if (videoCtx.completed_apts != videoCtx.apts || videoCtx.completed_vpts != videoCtx.vpts) {
		videoCtx.completed_apts = videoCtx.apts;
		videoCtx.completed_vpts = videoCtx.vpts;
		videoCtx.completed_counter = 0;
	}

	//++ frame rate & av sync control ++//

	tickcur = av_gettime_relative() / 1000;
	tickdiff = (int)(tickcur - videoCtx.ticklast);
	videoCtx.ticklast = tickcur;

	// re-calculate start_pts & start_tick if needed
	if (videoCtx.start_pts == AV_NOPTS_VALUE) {
		videoCtx.start_pts = videoCtx.vpts;
		videoCtx.start_tick = tickcur;
	}

	avdiff = (int)(videoCtx.apts - videoCtx.vpts - videoCtx.tickavdiff); // diff between audio and video pts
	scdiff = (int)(videoCtx.start_pts + tickcur - videoCtx.start_tick - videoCtx.vpts - videoCtx.tickavdiff); // diff between system clock and video pts
	if (videoCtx.apts <= 0) avdiff = scdiff; // if apts is invalid, sync vpts to system clock

	if (tickdiff - videoCtx.tickframe > 5) videoCtx.ticksleep--;
	if (tickdiff - videoCtx.tickframe < -5) videoCtx.ticksleep++;
	if (videoCtx.vpts >= 0) {
		if (avdiff > 500) videoCtx.ticksleep -= 3;
		else if (avdiff > 50) videoCtx.ticksleep -= 1;
		else if (avdiff < -500) videoCtx.ticksleep += 3;
		else if (avdiff < -50) videoCtx.ticksleep += 1;
	}
	if (videoCtx.ticksleep < 0) videoCtx.ticksleep = 0;
	//-- frame rate & av sync control --//

	if (videoCtx.ticksleep > 0)
		av_usleep(videoCtx.ticksleep * 1000);
}

void CALLBACK  CMainFramWnd::AudioCallBack(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	AudioCtx *pAudioCtx = (AudioCtx*)dwInstance;
	switch (uMsg)
	{
	case WOM_DONE:
	{
		memcpy(pAudioCtx->curdata, pAudioCtx->pWaveHdr[pAudioCtx->head].lpData, pAudioCtx->buflen);
		if (pAudioCtx->apts) *pAudioCtx->apts = pAudioCtx->ppts[pAudioCtx->head];
		if (++pAudioCtx->head == pAudioCtx->bufnum) pAudioCtx->head = 0;
		ReleaseSemaphore(pAudioCtx->bufsem, 1, NULL);
	}
	break;
	}
}

DuiLib::CDuiString CMainFramWnd::GetSkinFolder()
{
	return _T("");
}

DuiLib::CDuiString CMainFramWnd::GetSkinFile()
{
	return _T("res\\xml\\mainframewnd.xml");
}

LPCTSTR CMainFramWnd::GetWindowClassName(void) const
{
	return _T("mainframewnd");
}

void CMainFramWnd::OnClose(TNotifyUI &msg)
{
	_beginthreadex(NULL, 0, DemutexThread, this, 0, NULL);

}
