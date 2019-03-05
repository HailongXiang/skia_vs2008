#pragma once
#include "skiatest.h"
class SkBitmap;
class SKPaint;

class Plans
{
public:
	Plans();
	virtual ~Plans();

	/*实时截图，高斯处理后设置为背景*/
	static bool Gauss(const HWND m_hWnd, const HDC hdc, const POINT ptDest, const SIZE szLayered);

	/*在画布上等间距绘透明度随机或者统一的点*/
	static bool DrawPoints(const SkBitmap bitmap);

	/*使用透明模糊色块填充*/
	static void ColorBk(const SkBitmap bitmap);
};

