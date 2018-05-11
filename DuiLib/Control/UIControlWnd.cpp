#include "StdAfx.h"
#include "UIControlWnd.h"

namespace DuiLib
{
	CControlWndUI::CControlWndUI() :
		m_gdiplusToken(0)
	{

#ifdef _USE_GDIPLUS
        GdiplusStartup( &m_gdiplusToken,&m_gdiplusStartupInput, NULL);
#endif
	}

	CControlWndUI::~CControlWndUI()
	{
#ifdef _USE_GDIPLUS
		GdiplusShutdown( m_gdiplusToken );
#endif
	}

	LPCTSTR CControlWndUI::GetClass() const
	{
		return DUI_CTR_CONTROLWND;
	}

	LPVOID CControlWndUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_CONTROLWND) == 0) return static_cast<CControlWndUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

    void CControlWndUI::SetFixedWidth(int cx)
    {
        CControlUI::SetFixedWidth(cx);
    }

    void CControlWndUI::SetFixedHeight(int cy)
    {
        CControlUI::SetFixedHeight(cy);
    }

	void CControlWndUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CControlWndUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CControlWndUI::SetPos(RECT rc, bool bNeedInvalidate /*= true*/)
	{
		if (!::IsWindow(m_hWnd))
		{
			return;
		}

		CControlUI::SetPos(rc);
		MoveWindow( m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );
		GetClientRect(m_hWnd, &rc);
		InvalidateRect(m_hWnd, &rc, TRUE);

		if (m_pManager != NULL)
		{
			m_pManager->SendNotify( this, DUI_MSGTYPE_WINDOWINIT );
		}
	
	}

	void CControlWndUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit /*= true*/)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if (pManager != NULL)
		{
			::SetParent(m_hWnd, pManager->GetPaintWindow());
		}
	}

	void CControlWndUI::SetEnabled(bool bEnable /*= true*/)
	{
		CControlUI::SetEnabled(bEnable);
		if (!::IsWindow(m_hWnd))
		{
			return;
		}

		EnableWindow(m_hWnd, (BOOL)bEnable);
	}

	LPCTSTR CControlWndUI::GetWindowClassName() const
	{
		return _T("CControlWndUI");
	}

	LRESULT CControlWndUI::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg)
		{
		case WM_CREATE:
			lRes = OnCreate(uMsg, wParam, lParam, bHandled);
			break;
		case WM_PAINT:
			lRes = OnPaint(uMsg, wParam, lParam, bHandled);
			break;
		case WM_CLOSE:
		{
			DestroyWindow(m_hWnd);
			break;
		}
		default: bHandled = FALSE;
		}

		if (bHandled)
		{
			return lRes;
		}

		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT CControlWndUI::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue );
		return 0l;
	}

	LRESULT CControlWndUI::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;
		PAINTSTRUCT paintstruct;
		::BeginPaint(m_hWnd, &paintstruct);
		HBRUSH hBrush = ::CreateSolidBrush(RGB(GetRValue(m_dwBackColor), GetGValue(m_dwBackColor), GetBValue(m_dwBackColor)));
		::FillRect(paintstruct.hdc, &paintstruct.rcPaint, hBrush);
		::EndPaint(m_hWnd, &paintstruct);
		return 1L;
	}

	void CControlWndUI::DoInit()
	{
		if (m_hWnd == NULL)
		{
			Create( m_pManager->GetPaintWindow(), _T("ControlWndUI"), UI_WNDSTYLE_CHILD, WS_EX_TOOLWINDOW, m_rcItem );
			ShowWindow(true);
		}
	}

}
