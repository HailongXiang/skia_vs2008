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
	~Image();

	static void Screen(HWND hWnd, POINT pos, SIZE rect);
	static void *bmp32to24(char *fileName);
	static void Bmp24ToBmp32(LPCTSTR lpSrcFileName, LPCTSTR lpDesFileName);
};

