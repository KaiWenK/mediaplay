#include "stdafx.h"
#include "pktqueue.h"

CPktQueue::CPktQueue()
: m_fsize(0)
, m_asize(0)
, m_vsize(0)
, m_fhead(0)
, m_ftail(0)
, m_ahead(0)
, m_atail(0)
, m_vhead(0)
, m_vtail(0)
, m_fsem(NULL)
, m_asem(NULL)
, m_vsem(NULL)
, m_bpkts(NULL)
, m_fpkts(NULL)
, m_apkts(NULL)
, m_vpkts(NULL)
, m_lock(NULL)
{

}

CPktQueue::~CPktQueue()
{

}

BOOL CPktQueue::Init()
{
	//初始赋值buffer size
	m_fsize = m_vsize = m_asize = DEFAULT_QUEUE_LEN;

	//分配buffer内存空间
	m_bpkts = (AVPacket*)calloc(m_fsize, sizeof(AVPacket));
	m_fpkts = (AVPacket**)calloc(m_fsize, sizeof(AVPacket*));
	m_apkts = (AVPacket**)calloc(m_asize, sizeof(AVPacket*));
	m_vpkts = (AVPacket**)calloc(m_vsize, sizeof(AVPacket*));

	//创建并初始化信号量
	m_fsem = CreateSemaphore(NULL, m_fsize, m_fsize, NULL);
	m_vsem = CreateSemaphore(NULL, 0, m_fsize, NULL);
	m_asem = CreateSemaphore(NULL, 0, m_fsize, NULL);

	//创建并初始化锁
	m_lock = CreateMutex(NULL, FALSE, NULL);

	//检查buffer空间是否分配成功
	if (!m_bpkts && !m_fpkts && !m_apkts && !m_vpkts)
	{
		av_log(NULL, AV_LOG_ERROR, "[CPktQueue::Init()] alloc buffer faild");
		return FALSE;
	}

	//检查锁和信号量是否都有效
	if (!m_fsem && !m_vsem && !m_asem && !m_lock)
	{
		av_log(NULL, AV_LOG_ERROR, "[CPktQueue::Init()] init sem faild");
		return FALSE;
	}

	//初始化m_fpkts
	for (int i = 0; i < m_fsize; i++)
	{
		m_fpkts[i] = &m_bpkts[i];
	}

	return TRUE;
}

void CPktQueue::FreePktEnqueue(AVPacket *pkt)
{
	WaitForSingleObject(m_lock, INFINITE);
	m_fpkts[m_ftail++ & (m_fsize - 1)] = pkt;
	ReleaseMutex(m_lock);

	ReleaseSemaphore(m_fsem, 1, NULL);
}

AVPacket* CPktQueue::FreePktDequeue()
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_fsem, 0))
	{
		return NULL;
	}
	return m_fpkts[m_fhead++ & (m_fsize - 1)];
}

void CPktQueue::FreePktCancel(AVPacket *pkt)
{
	WaitForSingleObject(m_lock, INFINITE);
	m_fpkts[m_ftail++ & (m_fsize - 1)] = pkt;
	ReleaseMutex(m_lock);

	ReleaseSemaphore(m_fsem, 1, NULL);
}

void CPktQueue::AudioPktEnqueue(AVPacket *pkt)
{
	m_apkts[m_atail++ & (m_asize - 1)] = pkt;
	ReleaseSemaphore(m_asem, 1, NULL);
}

AVPacket* CPktQueue::AudioPktDequeue()
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_asem, 0))
	{
		return NULL;
	}
	
	return m_apkts[m_ahead++ & (m_asize - 1)];
}

void CPktQueue::VideoPktEnqueue(AVPacket *pkt)
{
	m_vpkts[m_vtail++ & (m_vsize - 1)] = pkt;
	ReleaseSemaphore(m_vsem, 1, NULL);
}

AVPacket* CPktQueue::VideoPktDequeue()
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_vsem, 0))
	{
		return NULL;
	}
	return m_vpkts[m_vhead++ & (m_vsize - 1)];
}

void CPktQueue::PktqueueReset()
{
	AVPacket *packet = NULL;

	while (NULL != (packet = AudioPktDequeue())) {
		av_packet_unref(packet);
		FreePktEnqueue(packet);
	}

	while (NULL != (packet = VideoPktDequeue())) {
		av_packet_unref(packet);
		FreePktEnqueue(packet);
	}

	m_fhead = m_ftail = 0;
	m_ahead = m_atail = 0;
	m_vhead = m_vtail = 0;
}

void CPktQueue::PktQueDestroy()
{
	// 重新设置包为unref状态
	PktqueueReset();

	//关闭信号量和互斥变量
	CloseHandle(m_fsem);
	CloseHandle(m_asem);
	CloseHandle(m_vsem);
	CloseHandle(m_lock);

	// free
	free(m_bpkts);
	free(m_fpkts);
	free(m_apkts);
	free(m_vpkts);
}
