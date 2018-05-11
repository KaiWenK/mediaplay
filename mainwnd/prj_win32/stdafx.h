// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <process.h>
#include <mmsystem.h>


// TODO:  在此处引用程序需要的其他头文件

#include "../../DuiLib/UIlib.h"
using namespace DuiLib;
#pragma comment( lib, "./Debug/duilib.lib" )

extern "C" {
#include "libavcodec\avcodec.h"  
#include "libavformat\avformat.h"  
#include "libavutil\channel_layout.h"  
#include "libavutil\common.h"  
#include "libavutil\imgutils.h"  
#include "libswscale\swscale.h" 
#include "libavutil\imgutils.h"      
#include "libavutil\opt.h"         
#include "libavutil\mathematics.h"      
#include "libavutil\samplefmt.h"
#include "libavutil\timestamp.h"
#include "libavutil\log.h"
#include "libavutil\time.h"
#include "libswresample\swresample.h" 
}

#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "avdevice.lib" )
#pragma comment( lib, "avfilter.lib" )
#pragma comment( lib, "avformat.lib" )
#pragma comment( lib, "avutil.lib" )
#pragma comment( lib, "postproc.lib" )
#pragma comment( lib, "swresample.lib" )
#pragma comment( lib, "swscale.lib" )
#pragma comment(lib,"winmm.lib")

