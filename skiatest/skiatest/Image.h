#pragma once
#include <windows.h>
#include <iostream>
#include<math.h>
#include "atlimage.h"
using namespace std;
class Image
{
public:
	Image();
	virtual ~Image();

	static bool Screen(const HWND hWnd, const POINT pos, const SIZE rect, const LPCWSTR filename);
	static bool Bmp24ToBmp32(const LPCTSTR filename);

private:
	static bool Bmp32ToBmp24(const LPCTSTR filename);

};

