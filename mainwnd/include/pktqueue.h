#pragma once
#ifndef PKTQUEUE_H
#define PKTQUEUE_H

#define DEFAULT_QUEUE_LEN      256          //默认队列长度

class CPktQueue
{
public:
	CPktQueue();
	~CPktQueue();

	BOOL Init();

	void FreePktEnqueue(AVPacket *pkt);
	AVPacket* FreePktDequeue();
	void FreePktCancel(AVPacket *pkt);

	void AudioPktEnqueue(AVPacket *pkt);
	AVPacket* AudioPktDequeue();

	void VideoPktEnqueue(AVPacket *pkt);
	AVPacket* VideoPktDequeue();

	void PktqueueReset();
	void PktQueDestroy();

private:
	UINT        m_fsize;	// 缓冲队列中包的总数
	UINT        m_asize;	// 音频包缓冲总数
	UINT        m_vsize;	// 视频包缓冲总数
	UINT        m_fhead;	// 包缓冲队列头的位置
	UINT        m_ftail;	// 包缓冲队列尾部的位置
	UINT        m_ahead;	// 音频包队列头的位置
	UINT        m_atail;	// 音频包队列尾部的位置
	UINT        m_vhead;	// 视频包队列头的位置
	UINT        m_vtail;	// 视频包队列尾部的位置
	HANDLE      m_fsem;	    // 包缓冲队列信号量
	HANDLE      m_asem;	    // 音频队列信号量
	HANDLE      m_vsem;	    // 视频队列信号量
	AVPacket   *m_bpkts;    // 包缓冲
	AVPacket  **m_fpkts;    // 包缓冲队列的指针
	AVPacket  **m_apkts;    // 音频缓冲队列的指针
	AVPacket  **m_vpkts;    // 视频缓冲队列的指针

	HANDLE      m_lock;     //实现互斥访问
};

#endif


