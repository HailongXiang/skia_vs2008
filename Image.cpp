#include "stdafx.h"
#include "Image.h"


Image::Image()
{
}

void Image::Screen(HWND hWnd,POINT pos, SIZE rect)
{
	
	HDC hdcSrc = GetDC(NULL);
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
	image.Save(L"screen.bmp", Gdiplus::ImageFormatBMP);
	bmp32to24("screen.bmp");
}


void* Image::bmp32to24(char *fileName)
{
	static int g_iExtraSize = 0;
	static void *g_pExtra = 0;
	static const int g_iHeaderSize = 54;
	static void *g_pHeader = 0;
	unsigned char *p = 0;
	unsigned char *q = 0;
	FILE *fr = 0;
	int t = 0;
	unsigned char bfType[2] = { 0 };
	int bfSize = 0;
	unsigned short bfReserved1 = 0;
	unsigned short bfReserved2 = 0;
	int bfOffbits = 0;
	int biSize = 0;
	int biWidth = 0;
	int biHeight = 0;
	unsigned short biPlanes = 0;
	unsigned short biBitCount = 0;
	int biCompression = 0;
	int biSizeImage = 0;
	int biXpelsPermeter = 0;
	int biYpelsPermeter = 0;
	int biClrUsed = 0;
	int biClrImportant = 0;
	int bMask = 0;
	int gMask = 0;
	int rMask = 0;


	fr = fopen(fileName, "rb");
	if (fr)
	{
		g_pHeader = malloc(g_iHeaderSize);
		if (g_pHeader)
		{
			t = fread(g_pHeader, 1, g_iHeaderSize, fr);
			if (t == g_iHeaderSize)
			{
				q = (unsigned char *)g_pHeader;
				memcpy(bfType, q, 2);
				q += 2;
				memcpy(&bfSize, q, 4);
				q += 4;
				memcpy(&bfReserved1, q, 2);
				q += 2;
				memcpy(&bfReserved2, q, 2);
				q += 2;
				memcpy(&bfOffbits, q, 4);
				q += 4;
				memcpy(&biSize, q, 4);
				q += 4;
				memcpy(&biWidth, q, 4);
				q += 4;
				memcpy(&biHeight, q, 4);
				q += 4;
				memcpy(&biPlanes, q, 2);
				q += 2;
				memcpy(&biBitCount, q, 2);
				q += 2;
				memcpy(&biCompression, q, 4);
				q += 4;
				memcpy(&biSizeImage, q, 4);
				q += 4;
				memcpy(&biXpelsPermeter, q, 4);
				q += 4;
				memcpy(&biYpelsPermeter, q, 4);
				q += 4;
				memcpy(&biClrUsed, q, 4);
				q += 4;
				memcpy(&biClrImportant, q, 4);
				q += 4;
				g_iExtraSize = bfOffbits - g_iHeaderSize;
				g_pExtra = malloc(g_iHeaderSize);
				t = fread(g_pExtra, 1, g_iExtraSize, fr);
				if (t > 11 && t == g_iExtraSize)
				{
					q = (unsigned char *)g_pExtra;
					memcpy(&bMask, q, 4);
					q += 4;
					memcpy(&gMask, q, 4);
					q += 4;
					memcpy(&rMask, q, 4);
					q += 4;
				}
				else
				{
					printf("error");
				}
				if (biBitCount == 32)
				{
					void *buf32 = 0, *buf24 = 0;
					buf32 = malloc(biWidth*biHeight * 4);
					buf24 = malloc(biWidth*biHeight * 3);
					if (buf32 && buf24)
					{
						t = fread(buf32, 1, biWidth*biHeight * 4, fr);
						if (t == biWidth * biHeight * 4)
						{
							int i = 0, j = 0;
							unsigned char headerBuffer[54] = { 0 };


							p = (unsigned char *)buf32;
							q = (unsigned char *)buf24;
							for (j = 0; j < biHeight; j++)
							{
								for (i = 0; i < biWidth; i++)
								{
									*q = *p;
									q++;
									p++;
									*q = *p;
									q++;
									p++;
									*q = *p;
									q++;
									p++;
									p++;
								}
							}
							memcpy(headerBuffer, g_pHeader, g_iHeaderSize);
							p = headerBuffer;
							p += 2;
							t = biWidth * biHeight * 3 + 54;
							memcpy(p, &t, 4);
							p = headerBuffer;
							p += 10;
							t = 54;
							memcpy(p, &t, 4);
							p = headerBuffer;
							p += 28;
							t = 24;
							memcpy(p, &t, 2);
							p = headerBuffer;
							p += 30;
							t = 0;
							memcpy(p, &t, 4);
							p = headerBuffer;
							p += 34;
							t = biWidth * biHeight * 3;
							memcpy(p, &t, 4);
							{
								FILE *fw = 0;
								fw = fopen("screen.bmp.", "wb");
								if (fw)
								{
									t = fwrite(headerBuffer, 1, g_iHeaderSize, fw);
									t = fwrite(buf24, 1, biWidth*biHeight * 3, fw);
									fclose(fw);
								}
							}
						}
						else
						{
							printf("error");
						}
						free(buf32);
						free(buf24);
					}
					else
					{
						printf("error");
					}
				}
				else
				{
					printf("error");
				}
			}
			else
			{
				printf("error");
			}
		}
		fclose(fr);
	}
	return p;
}

void SaveDC2BMP(HWND hwnd, TCHAR *path)
{
	HBITMAP hBitmap;
	HDC hdc, hTempDC;
	RECT rect;
	int w, h;
	hdc = ::GetDC(hwnd);
	::GetClientRect(hwnd, &rect);

	w = rect.right - rect.left;
	h = rect.bottom - rect.top;

	hTempDC = ::CreateCompatibleDC(hdc);
	hBitmap = ::CreateCompatibleBitmap(hdc, w, h);
	::SelectObject(hTempDC, hBitmap);

	::BitBlt(hTempDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);

	CImage image;
	image.Attach(hBitmap);
	image.Save(path);

	::DeleteDC(hTempDC);
	::DeleteObject(hBitmap);
	::ReleaseDC(hwnd, hdc);
}

void Image::Bmp24ToBmp32(LPCTSTR lpSrcFileName, LPCTSTR lpDesFileName)
{
	HBITMAP hbmp24 = (HBITMAP)LoadImage(NULL, lpSrcFileName,
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

	FILE *fp = _tfopen(lpDesFileName, _T("wb+"));

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
			// Update source total offset 
			dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

			// Update bitmap 
			fwrite(&pData[dwSrcTotalOffset], 1, 1, fp);
			fwrite(&pData[dwSrcTotalOffset + 1], 1, 1, fp);
			fwrite(&pData[dwSrcTotalOffset + 2], 1, 1, fp);
			fwrite(&Alpha, 1, 1, fp);

			// Update source horizontal offset 
			dwSrcHorizontalOffset += m_iBpp;
		}

		// Update source vertical offset 
		dwSrcVerticalOffset += _pitch;
	}

	delete[] pData;

	fclose(fp);

	DeleteObject(hbmp24);
}



Image::~Image()
{
}
