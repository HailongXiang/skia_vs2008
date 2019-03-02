// skiatest.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "windowsx.h"
#include "skiatest.h"

#include "core\SkBitmap.h"
#include "core\SkDevice.h"
#include "core\SkPaint.h"
#include "core\SkRect.h"
#include "core\SkTypeface.h"
#include "core\SkScalar.h"
#include "core\SkImageEncoder.h"
#include "core\SkImageDecoder.h"
#include "effects\SkBlurMaskFilter.h"
#include "effects\SkBlurMask.h"
#include "effects\SkDiscretePathEffect.h"

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "usp10.lib")
#pragma comment(lib, "DelayImp.lib")
#pragma comment(lib, "windowscodecs.lib")

#define MAX_LOADSTRING 256

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������
HWND m_hWnd;
LPVOID		g_pBmpBits;
HDC			g_hMemDc;
HBITMAP		g_hBmp;
SkBitmap	g_skBitmap;
int			g_nWidth;
int			g_nHeight;
// �˴���ģ���а����ĺ�����ǰ������:
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

LPCSTR GetUTF8String(LPCWSTR str)
{
	int iLen = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

	if (iLen > 1)
	{ 
		char *pBuf=new char[iLen];
		::WideCharToMultiByte(CP_UTF8, 0, str, -1, pBuf, iLen, NULL, NULL);
		return pBuf;
	}

	return NULL;
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

		/*HBITMAP */g_hBmp= CreateGDIBitmap(g_nWidth, g_nHeight, &g_pBmpBits);
		g_hMemDc = CreateCompatibleDC(hdc);
		SelectObject(g_hMemDc, g_hBmp);

		g_skBitmap.setConfig(SkImageInfo::Make(g_nWidth, g_nHeight, kN32_SkColorType, kPremul_SkAlphaType));
		g_skBitmap.setPixels(g_pBmpBits);
	}

	POINT ptDest = { rcClient.left, rcClient.top };
	SIZE szLayered = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
	
	Image::Screen(m_hWnd,ptDest, szLayered);		//��ͼ

	g_hBmp = (HBITMAP)LoadImage(NULL, _T("screen.bmp"), IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (!Gauss::GaussBlur(g_hBmp))
	{
		if (g_hBmp) DeleteObject(g_hBmp);
		return 0;
	}

	// �ڴ�DC
	HDC mem_dc = CreateCompatibleDC(hdc);
	HBITMAP mem_bmp = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
	HGDIOBJ mem_old = SelectObject(mem_dc, mem_bmp);

	// λͼDC
	HDC bmp_dc = CreateCompatibleDC(mem_dc);
	HGDIOBJ bmp_old = SelectObject(bmp_dc, g_hBmp);

	CImage image;
	image.Attach(g_hBmp);
	image.Save(L"screen.bmp", Gdiplus::ImageFormatBMP);
	Image::Bmp24ToBmp32(L"screen.bmp", L"screen.bmp");

	// �����ڴ�
	SelectObject(bmp_dc, bmp_old);
	DeleteDC(bmp_dc);
	SelectObject(mem_dc, mem_old);
	DeleteObject(mem_bmp);
	DeleteDC(mem_dc);
	DeleteObject(g_hBmp);


	SkPaint paint;
	SkCanvas canvas(g_skBitmap);
	SkBitmap bmpPng;
	if(!SkImageDecoder::DecodeFile("screen.bmp", &bmpPng))		//ͼƬ���룬���ݴ洢��bmpPng�У��ɹ�����true
		MessageBox(m_hWnd, _T("�޷���"), _T(""), MB_OK);
	
	paint.setAlpha(255);	//͸����	0ȫ͸�� ~ 255��͸��
	canvas.drawBitmap(bmpPng, 0, 0, &paint);	//������ͼƬ��С������x�ᣬy�ᣬ���ʶ���
	

	//::SetBkMode(g_hMemDc, TRANSPARENT);		//TRANSPARENT ���óɱ���͸��
	//::SetTextColor(g_hMemDc, RGB(88,88,88));

	/*RECT rcDraw = {180,40, 320, 380};
	DrawText(g_hMemDc, _T("GDI�������ֻᴩ͸"), -1, &rcDraw, DT_WORDBREAK|DT_NOPREFIX);*/

	//paint.setPathEffect(SkDiscretePathEffect::Create(1, 1));	//��ɢ·��ЧӦ
	
	//paint.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10))));
	////paint.setAlpha(0);
	SkRect r;
	paint.setARGB(150, 183, 183, 183);
	r.set(0, 0, 500, 500);
	canvas.drawRect(r, paint); 

	//paint.setMaskFilter(SkBlurMaskFilter::Create( SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)), SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMaskFilter::kIgnoreTransform_BlurFlag));
	//paint.setARGB(240, 183, 183, 183);
	//r.offset(145, 0);
	//canvas.drawRect(r, paint);

	//paint.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)), SkBlurMaskFilter::kAll_BlurFlag));
	//paint.setARGB(240, 183, 183, 183);
	//r.offset(145, 0);
	//canvas.drawRect(r, paint);

	/*LPCSTR strFont = GetUTF8String(L"΢���ź�");
	SkTypeface *font = SkTypeface::CreateFromName(strFont, SkTypeface::kBold);
	delete strFont;

	LPCSTR strFont2 = GetUTF8String(L"����");
	SkTypeface *font2 = SkTypeface::CreateFromName(strFont2, SkTypeface::kNormal);
	delete strFont2;


	if ( font && font2)
	{
		paint.setStyle(SkPaint::kStrokeAndFill_Style);
		paint.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kSolid_BlurStyle,SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3))));
		paint.setARGB(255, 255, 0, 0);
		paint.setTextSize(24);
		paint.setTextEncoding(SkPaint::kUTF16_TextEncoding); 
		paint.setAntiAlias(true);
		paint.setTypeface( font );
		canvas.drawText(L"redrain΢���ź�", wcslen(L"redrain΢���ź�")*2, 180, 80, paint);
		paint.setTypeface( font2 );
		canvas.drawText(L"redrain����", wcslen(L"redrain����")*2, 180, 120, paint);
	}
*/
	
	//POINT ptDest = {rcClient.left, rcClient.top};
	POINT ptSrc = {0, 0};
	//SIZE szLayered = {rcClient.right - rcClient.left, rcClient.bottom - rcClient.top};
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	::UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &szLayered, g_hMemDc, &ptSrc, (COLORREF)0, &bf, ULW_ALPHA);
	//SkImageEncoder::EncodeFile("out.png", g_skBitmap, SkImageEncoder::kPNG_Type, 80);
	//DeleteBitmap(g_hBmp);
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

 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKIATEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SKIATEST));

	// ����Ϣѭ��:
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
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
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
		// �����˵�ѡ��:
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

			hdc = BeginPaint(hWnd, &ps);
			//test(hdc);
			doPrint(hdc);
			EndPaint(hWnd, &ps);
			DeleteDC(hdc);

			return HTCAPTION;
			//return HTCLIENT;
		}
		break;/*
	case WM_NCMOUSEMOVE:
		{
			hdc = BeginPaint(hWnd, &ps);
			doPrint(hdc);
			EndPaint(hWnd, &ps);
		}
		break;*/
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

// �����ڡ������Ϣ�������
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
