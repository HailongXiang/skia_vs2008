// skiatest.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"

#include "skiatest.h"



// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HWND m_hWnd;
LPVOID		g_pBmpBits;
HDC			g_hMemDc;
HBITMAP		g_hBmp;
SkBitmap	g_skBitmap;
int			g_nWidth;
int			g_nHeight;
int			plans = 1;							/*1~3 : 方案一~三		方案一要求屏幕无缩放*/
// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HBITMAP CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
{
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = nWid;
	bmi.bmiHeader.biHeight      = -nHei; // top-down image 
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = 0;

	HDC hdc=GetDC(NULL);
	LPVOID pBits=NULL;
	HBITMAP hBmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,ppBits,0,0);
	ReleaseDC(NULL,hdc);
	return hBmp;
}



int	doPrint(HDC hdc)
{
	DWORD dwExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	if((dwExStyle&WS_EX_LAYERED) != WS_EX_LAYERED)
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle|WS_EX_LAYERED);
	}

	RECT rcClient = {0};
	::GetWindowRect(m_hWnd, &rcClient);
	if ((g_nWidth != rcClient.right - rcClient.left) ||
		(g_nHeight != rcClient.bottom - rcClient.top) ||
		g_hBmp == NULL || g_hMemDc == NULL)
	{				
		g_nWidth = rcClient.right - rcClient.left;
		g_nHeight = rcClient.bottom - rcClient.top;
		
		if (g_hBmp != NULL) 
		{
			DeleteObject(g_hBmp);
			g_hBmp = NULL;
		}
		if (g_hMemDc != NULL) 
		{
			DeleteObject(g_hMemDc);
			g_hMemDc = NULL;
		}

		g_hBmp= CreateGDIBitmap(g_nWidth, g_nHeight, &g_pBmpBits);
		g_hMemDc = CreateCompatibleDC(hdc);
		SelectObject(g_hMemDc, g_hBmp);

		g_skBitmap.setConfig(SkImageInfo::Make(g_nWidth, g_nHeight, kN32_SkColorType, kPremul_SkAlphaType));
		g_skBitmap.setPixels(g_pBmpBits);
	}

	POINT ptDest = { rcClient.left, rcClient.top };
	SIZE szLayered = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
	
	SkPaint paint;
	SkCanvas canvas(g_skBitmap);
	SkBitmap bmpPng;
	
	switch (plans)
	{
		case 1:
			/*方案一：实时截图，高斯处理后设置为背景*/
			if (Plans::Gauss(m_hWnd, hdc, ptDest, szLayered) && \
				SkImageDecoder::DecodeFile("screen.bmp", &bmpPng))		//图片解码，数据存储在bmpPng中，成功返回true
				canvas.drawBitmap(bmpPng, 0, 0, &paint);
			else
				MessageBox(m_hWnd, _T("无法打开"), _T(""), MB_OK);
		break;
		case 2:
			/*方案二：在画布上等间距绘透明度随机或者统一的点*/
			Plans::DrawPoints(g_skBitmap);
		break;
		case 3:
			/*方案三：使用透明模糊色块填充*/
			Plans::ColorBk(g_skBitmap);
		break;
		default:
			MessageBox(NULL, L"模式选择错误", L"提示", MB_OK);
			exit(0);
		break;
	}

	/*::SetBkMode(g_hMemDc, TRANSPARENT);		//TRANSPARENT 设置成背景透明
	::SetTextColor(g_hMemDc, RGB(88,88,88));

	RECT rcDraw = {180,40, 320, 380};
	DrawText(g_hMemDc, _T("GDI绘制文字会穿透"), -1, &rcDraw, DT_WORDBREAK|DT_NOPREFIX);*/

	POINT ptSrc = {0, 0};
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	::UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &szLayered, g_hMemDc, &ptSrc, (COLORREF)0, &bf, ULW_ALPHA);
	remove("screen.bmp");
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKIATEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SKIATEST));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SKIATEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SKIATEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中
   g_pBmpBits = NULL;
   g_hBmp = NULL;
   g_hMemDc = NULL;
   g_nWidth = 0;
   g_nHeight = 0;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      500, 300, 500, 500, NULL, NULL, hInstance, NULL);
   
   if (!hWnd)
   {
      return FALSE;
   }
   m_hWnd = hWnd;
   SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

   PAINTSTRUCT ps;
   HDC hdc;

   hdc = BeginPaint(hWnd, &ps);
   doPrint(hdc);
   EndPaint(hWnd, &ps);
   DeleteDC(hdc);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		{
			DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
			dwStyle &= ~WS_CAPTION;
			SetWindowLong(hWnd, GWL_STYLE, dwStyle);

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		{
			//hdc = BeginPaint(hWnd, &ps);
			//doPrint(hdc);
			////test(hdc);
			//EndPaint(hWnd, &ps);
			//DeleteDC(hdc);
		}
		break;
	case WM_ERASEBKGND:
		{
			return 1;
		}
		break;
	case WM_NCCALCSIZE:
		{
			LPRECT pRect=NULL;

			if ( wParam == TRUE)
			{
				LPNCCALCSIZE_PARAMS pParam = (LPNCCALCSIZE_PARAMS)lParam;
				pRect=&pParam->rgrc[0];
			}
			else
			{
				pRect=(LPRECT)lParam;
			}

			if ( ::IsZoomed(m_hWnd))
			{
				MONITORINFO oMonitor = {};
				oMonitor.cbSize = sizeof(oMonitor);
				::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
				RECT rcWork = oMonitor.rcWork;
				RECT rcMonitor = oMonitor.rcMonitor;
				rcWork.left -= oMonitor.rcMonitor.left;
				rcWork.right -= oMonitor.rcMonitor.left;
				rcWork.top -= oMonitor.rcMonitor.top;
				rcWork.bottom -= oMonitor.rcMonitor.top;

				pRect->right = pRect->left + rcWork.right - rcWork.left;
				pRect->bottom = pRect->top + rcWork.bottom - rcWork.top;
				return WVR_REDRAW;
			}
		}
		break;
	case WM_NCPAINT:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_NCHITTEST:
		{
			POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
			::ScreenToClient(m_hWnd, &pt);
			
			RECT rcClient;
			::GetClientRect(m_hWnd, &rcClient);

			if (plans == 1)
			{
				hdc = BeginPaint(hWnd, &ps);
				doPrint(hdc);
				EndPaint(hWnd, &ps);
				DeleteDC(hdc);
			}

			return HTCAPTION;
		}
		break;
	case WM_DESTROY:
		{
			if (g_hBmp != NULL) 
			{
				DeleteObject(g_hBmp);
				g_hBmp = NULL;
				g_pBmpBits = NULL;
			}
			if (g_hMemDc != NULL) 
			{
				DeleteObject(g_hMemDc);
				g_hMemDc = NULL;
			}
			PostQuitMessage(0);
		}
		
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
