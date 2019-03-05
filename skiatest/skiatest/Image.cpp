#include "stdafx.h"
#include "Image.h"


Image::Image()
{
}

bool Image::Screen(const HWND hWnd, const POINT pos, const SIZE rect, const LPCWSTR filename)
{
	HDC hdcSrc = GetDC(NULL);
	/*int nScreenWidth = GetDeviceCaps(hdcSrc, HORZRES);
	int scale = 1920 / nScreenWidth;*/
	int nBitPerPixel = GetDeviceCaps(hdcSrc, BITSPIXEL);
	int nWidth = rect.cx;
	int nHeight = rect.cy;
	CImage image;
	image.Create(nWidth, nHeight, nBitPerPixel);
	ShowWindow(hWnd, SW_HIDE);
	BitBlt(image.GetDC(), 0, 0, nWidth, nHeight, hdcSrc, pos.x, pos.y, SRCCOPY);
	ShowWindow(hWnd, SW_SHOW);
	ReleaseDC(NULL, hdcSrc);
	image.ReleaseDC();
	image.Save(filename, Gdiplus::ImageFormatBMP);
	if(!Bmp32ToBmp24(filename))
		return false;

	return true;
}

bool Image::Bmp32ToBmp24(const LPCTSTR filename)
{
	HBITMAP hbmp32 = (HBITMAP)LoadImage(NULL, filename,
		IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE |
		LR_CREATEDIBSECTION);

	BITMAP bmp;//获取位图信息
	GetObject(hbmp32, sizeof(BITMAP), &bmp);

	printf("Image Bit Depth : %dnWidth : %d , Height : %d n",
		bmp.bmBitsPixel, bmp.bmWidth, bmp.bmHeight);//显示位图颜色模式和图像宽高

	   //计算24位图像每行的字节数
	int BytesPerLine = 3 * bmp.bmWidth;
	while (BytesPerLine % 4 != 0)
		BytesPerLine++;

	BITMAPINFOHEADER bih = { 0 };//位图信息头
	bih.biBitCount = 24;//每个像素字节大小
	bih.biCompression = BI_RGB;
	bih.biHeight = bmp.bmHeight;
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = BytesPerLine * bmp.bmHeight;//图像数据大小
	bih.biWidth = bmp.bmWidth;

	BITMAPFILEHEADER bfh = { 0 };//位图文件头
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
	bfh.bfSize = bfh.bfOffBits + bih.biSizeImage;//文件总的大小
	bfh.bfType = (WORD)0x4d42;

	FILE *fp = _tfopen(filename, L"w+b");

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头

	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头

	byte * p = new byte[bih.biSizeImage];

	//获取当前32位图像数据
	GetDIBits(GetDC(NULL), hbmp32, 0, bmp.bmHeight, p, (LPBITMAPINFO)&bih, DIB_RGB_COLORS);

	//只取rgb值，存入文件
	byte b = 0;//用于填充
	for (int i = 0; i < bmp.bmWidth * bmp.bmHeight; i++)
	{
		//32位位图图像的格式为：Blue, Green, Red, Alpha
		fwrite(&(p[i * 3]), 1, 3, fp);
		if (i % bmp.bmWidth == bmp.bmWidth - 1)//填充字节
		{
			for (int k = 0; k < (BytesPerLine - bmp.bmWidth * 3); k++)
				fwrite(&b, sizeof(byte), 1, fp);
		}
	}

	delete[] p;
	fclose(fp);
	DeleteObject(hbmp32);
	return true;
}

bool Image::Bmp24ToBmp32(const LPCTSTR filename)
{
	HBITMAP hbmp24 = (HBITMAP)LoadImage(NULL, filename,
		IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE |
		LR_CREATEDIBSECTION);

	BITMAP bmp;
	GetObject(hbmp24, sizeof(BITMAP), &bmp);

	BITMAPINFOHEADER bih = { 0 };
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bmp.bmWidth;
	bih.biHeight = bmp.bmHeight;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = (bmp.bmWidth*bih.biBitCount + 31) / 32 * 4 * bmp.bmHeight;

	byte * pData = new byte[bih.biSizeImage];

	GetDIBits(GetDC(NULL), hbmp24, 0, bmp.bmHeight, pData, (LPBITMAPINFO)&bih, DIB_RGB_COLORS);

	bih.biBitCount = 32;
	bih.biSizeImage = (bmp.bmWidth*bih.biBitCount + 31) / 32 * 4 * bmp.bmHeight;

	BITMAPFILEHEADER bfh = { 0 };
	bfh.bfType = (WORD)0x4d42;
	bfh.bfSize = bfh.bfOffBits + bih.biSizeImage;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	FILE *fp = _tfopen(filename, _T("wb+"));

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);
	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);

	DWORD dwSrcHorizontalOffset;
	DWORD dwSrcVerticalOffset = 0;
	DWORD dwSrcTotalOffset = 0;

	int m_iBpp = 3;
	long _pitch = bmp.bmWidth * m_iBpp;
	int Alpha = 0;
	for (long i = 0; i < bmp.bmHeight; i++)
	{
		dwSrcHorizontalOffset = 0;
		for (long j = 0; j < bmp.bmWidth; j++)
		{
			dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

			fwrite(&pData[dwSrcTotalOffset], 1, 1, fp);
			fwrite(&pData[dwSrcTotalOffset + 1], 1, 1, fp);
			fwrite(&pData[dwSrcTotalOffset + 2], 1, 1, fp);
			fwrite(&Alpha, 1, 1, fp);

			dwSrcHorizontalOffset += m_iBpp;
		}
		dwSrcVerticalOffset += _pitch;
	}
	delete[] pData;
	fclose(fp);
	DeleteObject(hbmp24);
	return true;
}



Image::~Image()
{
}
