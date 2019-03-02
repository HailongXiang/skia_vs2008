#pragma once

#include "resource.h"

#include <math.h>

struct bitmap_t
{
	struct pix_t
	{
		BYTE g;
		BYTE b;
		BYTE r;
	} *m_bits;
	long m_w, m_h;
};

class Gauss
{
public:
	Gauss();
	~Gauss();

	static bool GaussBlur(HBITMAP hBmp);

private:
	static bool GetBitmap(HBITMAP hBmp, bitmap_t* pBmp);
	static bool GaussBmp(bitmap_t* pBmp, long nRadius);

};

