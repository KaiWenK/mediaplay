// mainwnd.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "mainwnd.h"
#include "mainframewnd.h"
#include <ctime>

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	CoInitialize(NULL);

	CPaintManagerUI::SetInstance(hInstance);   //将程序实例与皮肤绘制管理器挂钩。
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

	CMainFramWnd *pMainWnd = new CMainFramWnd();
	pMainWnd->Create(NULL, _T("manwend"), UI_WNDSTYLE_FRAME, UI_WNDSTYLE_EX_FRAME, RECT{ 0, 0, 0, 0 }, NULL);
	pMainWnd->CenterWindow();
	pMainWnd->ShowWindow(true);

	CPaintManagerUI::MessageLoop();

	CoUninitialize();
	return 0;
}