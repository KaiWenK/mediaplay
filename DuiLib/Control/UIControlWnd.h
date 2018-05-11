#ifndef __UICONTROLWND_H__
#define __UICONTROLWND_H__

#pragma once

#define _USE_GDIPLUS 1

#ifdef _USE_GDIPLUS
#include <GdiPlus.h>
#pragma comment( lib, "GdiPlus.lib" )
using namespace Gdiplus;
class DUILIB_API Gdiplus::RectF;
struct DUILIB_API Gdiplus::GdiplusStartupInput;
#endif


namespace DuiLib
{
	class DUILIB_API CControlWndUI 
	: public CControlUI
	, public CWindowWnd
	{
	public:
		CControlWndUI();
		~CControlWndUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

        void SetFixedWidth(int cx);
        void SetFixedHeight(int cy);

		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	public:
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		virtual void SetEnabled(bool bEnable = true);

	protected:
		virtual LPCTSTR GetWindowClassName() const;
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		//Control的init会调用这个函数，我们在这个函数中创建窗口
		virtual void DoInit();
	
	protected:
		ULONG_PTR				m_gdiplusToken;
#ifdef _USE_GDIPLUS
		GdiplusStartupInput		m_gdiplusStartupInput;
#endif
	};
}

#endif // __UILABEL_H__