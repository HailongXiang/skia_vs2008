#include "stdafx.h"
#include "Plans.h"


Plans::Plans()
{
}

bool Plans::Gauss(const HWND m_hWnd,const HDC hdc,const POINT ptDest,const SIZE szLayered)
{
	RECT rcClient = { 0 };
	::GetWindowRect(m_hWnd, &rcClient);

	Image::Screen(m_hWnd, ptDest, szLayered);		//½ØÍ¼

	HBITMAP g_hBmp = (HBITMAP)LoadImage(NULL, _T("screen.bmp"), IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (!Gauss::GaussBlur(g_hBmp))
	{
		if (g_hBmp) DeleteObject(g_hBmp);
		return false;
	}
	// ÄÚ´æDC
	HDC mem_dc = CreateCompatibleDC(hdc);
	HBITMAP mem_bmp = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
	HGDIOBJ mem_old = SelectObject(mem_dc, mem_bmp);

	// Î»Í¼DC
	HDC bmp_dc = CreateCompatibleDC(mem_dc);
	HGDIOBJ bmp_old = SelectObject(bmp_dc, g_hBmp);

	CImage image;
	image.Attach(g_hBmp);
	image.Save(L"screen.bmp", Gdiplus::ImageFormatBMP);

	//Image::Bmp24ToBmp32(L"screen.bmp", L"screen.bmp");

	// ÇåÀíÄÚ´æ
	SelectObject(bmp_dc, bmp_old);
	DeleteDC(bmp_dc);
	SelectObject(mem_dc, mem_old);
	DeleteObject(mem_bmp);
	DeleteDC(mem_dc);
	DeleteObject(g_hBmp);
	return true;
}

LPCSTR GetUTF8String(LPCWSTR str)
{
	int iLen = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

	if (iLen > 1)
	{
		char *pBuf = new char[iLen];
		::WideCharToMultiByte(CP_UTF8, 0, str, -1, pBuf, iLen, NULL, NULL);
		return pBuf;
	}

	return NULL;
}

bool Plans::DrawPoints(const SkBitmap bitmap)
{
	SkPaint paint;
	SkCanvas canvas(bitmap);
	LPCSTR strFont = GetUTF8String(L"Î¢ÈíÑÅºÚ");
	SkTypeface *font = SkTypeface::CreateFromName(strFont, SkTypeface::kBold);
	delete strFont;
	if (font)
	{
		paint.setStyle(SkPaint::kStrokeAndFill_Style);
		//paint.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kInner_BlurStyle,SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10))));
		paint.setARGB(120, 255, 255, 255);
		paint.setTextSize(50);
		paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
		paint.setAntiAlias(true);
		paint.setTypeface(font);
		for (int i = 0; i < 500; i += 4)
			for (int j = 0; j < 500; j += 4)
			{
				//paint.setARGB(100 + rand()%31, 255, 255, 255);
				canvas.drawText(L".", wcslen(L"."), j, i, paint);
			}
		return true;
	}
	return false;
}

void Plans::ColorBk(const SkBitmap bitmap)
{
	SkPaint paint;
	SkCanvas canvas(bitmap);
	paint.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kInner_BlurStyle, SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10))));
	SkRect r;
	paint.setARGB(220, 255, 255, 255);
	r.set(0, 0, 500, 500);
	canvas.drawRect(r, paint);
}



Plans::~Plans()
{
}
