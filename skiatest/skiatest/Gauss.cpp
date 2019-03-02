#include "stdafx.h"
#include "Gauss.h"


Gauss::Gauss()
{
}

bool Gauss::GetBitmap(HBITMAP hBmp, bitmap_t* pBmp)
{
	if (!hBmp || !pBmp) return false;
	BITMAP bm = { 0 };
	GetObject(hBmp, sizeof(bm), &bm);
	if (!bm.bmBits) return false;
	if (bm.bmBitsPixel != 24) return false;	// 仅处理24位位图
	pBmp->m_bits = (bitmap_t::pix_t*)bm.bmBits;
	pBmp->m_w = bm.bmWidth;
	pBmp->m_h = bm.bmHeight;
	return true;
}

bool Gauss::GaussBmp(bitmap_t* pBmp, long nRadius)
{
	if (!pBmp) return false;
	if (nRadius < 1) return true;
	// 定义变量
	long diamet = (nRadius << 1) + 1;				// 采样区域直径,或者方阵的边长
	double s = (double)nRadius / 3.0;				// 正态分布的标准偏差σ
	double sigma2 = 2.0 * s * s;					// 2倍的σ平方,参考N维空间正态分布方程
	double nuclear = 0.0;							// 高斯卷积核
	double* matrix = new double[diamet * diamet];	// 高斯矩阵定义
	bitmap_t::pix_t* bits = pBmp->m_bits;			// 像素内存块
	long w = pBmp->m_w, h = pBmp->m_h;				// 像素矩阵的宽与高
	// 计算高斯矩阵
	int i = 0;
	for (long y = -nRadius; y <= nRadius; ++y)
		for (long x = -nRadius; x <= nRadius; ++x)
		{
			matrix[i] =
				exp(-(double)(x * x + y * y) / sigma2);
			nuclear += matrix[i];
			++i;
		}
	// 遍历并处理像素
	for (long y_s = 0; y_s < h; ++y_s)
	{
		for (long x_s = 0; x_s < w; ++x_s)
		{
			// 分析取样区域
			double r = 0.0, g = 0.0, b = 0.0;
			int i_m = 0;
			for (long m = -nRadius; m <= nRadius; ++m)
			{
				long y = y_s + m;
				if (y >= 0 && y < h)
					for (long n = -nRadius; n <= nRadius; ++n)
					{
						long x = x_s + n;
						if (x >= 0 && x < w)
						{
							double weight = matrix[i_m] / nuclear;
							long i = (h - y - 1) * w + x;
							r += weight * bits[i].r;
							g += weight * bits[i].g;
							b += weight * bits[i].b;
						}
						++i_m;
					}
				else
					i_m += diamet;
			}
			// 保存处理结果
			long i_s = (h - y_s - 1) * w + x_s;
			bits[i_s].r = (BYTE)(r > (BYTE)~0 ? (BYTE)~0 : r);
			bits[i_s].g = (BYTE)(g > (BYTE)~0 ? (BYTE)~0 : g);
			bits[i_s].b = (BYTE)(b > (BYTE)~0 ? (BYTE)~0 : b);
		}
	}
	// 清理内存
	delete[] matrix;
	return true;
}

bool Gauss::GaussBlur(HBITMAP hBmp)
{
	long nRadius = 5;
	bitmap_t bmp = { 0 };
	if (!GetBitmap(hBmp, &bmp)) return false;
	return GaussBmp(&bmp, nRadius);
}

Gauss::~Gauss()
{
}
