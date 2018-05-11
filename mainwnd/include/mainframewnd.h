#ifndef _MAINFRAMEWND_
#define _MAINFRAMEWND_
#include "commendef.h"

typedef struct tagAudioCtx
{
	int64_t *ppts;
	int      bufnum;
	int      buflen;
	int      head;
	int      tail;
	int64_t *apts;
	/* store current audio data */
	int16_t *curdata;
	/* software volume */ 
	int      vol_scaler[256];
	int      vol_zerodb;
	int      vol_curvol;

	HWAVEOUT hWaveOut;
	WAVEHDR *pWaveHdr;
	HANDLE   bufsem;
} AudioCtx;

typedef struct tagVideoCtx {
	int       bufnum;      //缓冲数量
	int       pixfmt;	   //像素格式
	int       x;           //视频显示的X坐标
	int       y;           //视频显示的y坐标
	int       w;           //视频显示的宽度
	int       h;           //视频显示的高度
	int       sw;          //表面的宽度
	int       sh;          //表面的高度
	HWND      surface;     //表面（窗口）
	int64_t  *ppts;        //显示时间撮队列
	int64_t   apts;		   //音频时间撮
	int64_t   vpts;		   //视频时间撮
	int       head;        //显示时间撮队列头位置
	int       tail;        //显示时间撮队列尾位置
	HANDLE    semr; 
	HANDLE    semw;                           
	int       tickavdiff;
	int       tickframe;   //一帧显示的时钟
	int       ticksleep;	
	int64_t   ticklast;
	int       status;
	HANDLE    thread; 
	int       completed_counter;
	int64_t   completed_apts; 
	int64_t   completed_vpts;
	int       refresh_flag;

	/* used to sync video to system clock */
	int64_t   start_pts;
	int64_t   start_tick;
	void(*lock)(void *ctxt, uint8_t *buffer[8], int linesize[8]);
	void(*unlock)(void *ctxt, int64_t pts); 
	void(*setrect)(void *ctxt, int x, int y, int w, int h);
	void(*setparam)(void *ctxt, int id, void *param);
	void(*getparam)(void *ctxt, int id, void *param); 
	void(*destroy)(void *ctxt);

	HDC      hdcsrc;
	HDC      hdcdst;
	HBITMAP *hbitmaps;
	BYTE   **pbmpbufs;
}VideoCtx;

class CMainFramWnd : public WindowImplBase
{

public:
	CMainFramWnd();

	void Demutex();

	//初始化/清理媒体参数
	void InitMediaParam();
	void DestoryMediaParam();

	//初始化流参数
	BOOL InitStream( AVMediaType mType, int StreamIndex );

	//日志
	static void avlog( void* p, int n, const char* str, va_list vlist );


	void InitWindow();

	MediaParam* getMediaParam() { return &m_MediaParam; }
	const CControlWndUI* GetPlayWnd() { return m_pPlayWnd; }

	static HANDLE m_audiolock;		//锁，防止队列一边入一边取会有问题


private:
	~CMainFramWnd();

	static unsigned int CALLBACK DemutexThread(LPVOID p);
	static unsigned int CALLBACK VideoDecodeThread(LPVOID p);
	static unsigned int CALLBACK AudioDecodeThread(LPVOID p);
	static unsigned int CALLBACK VideoRenderThread(LPVOID p);
	static void VideoSyncComplete(VideoCtx& videoCtx);
	static void CALLBACK  AudioCallBack(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance,
		DWORD dwParam1, DWORD dwParam2);

protected:
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;

protected:

	void OnClose(TNotifyUI &msg);

	DUI_DECLARE_MESSAGE_MAP()

private:
	tagMediaParam m_MediaParam;


	CControlWndUI *m_pPlayWnd;

	AudioCtx m_AudioCtx;
	VideoCtx m_VideoCtx;
};

#endif