#pragma once
#include "skiatest.h"
class SkBitmap;
class SKPaint;

class Plans
{
public:
	Plans();
	virtual ~Plans();

	/*ʵʱ��ͼ����˹���������Ϊ����*/
	static bool Gauss(const HWND m_hWnd, const HDC hdc, const POINT ptDest, const SIZE szLayered);

	/*�ڻ����ϵȼ���͸�����������ͳһ�ĵ�*/
	static bool DrawPoints(const SkBitmap bitmap);

	/*ʹ��͸��ģ��ɫ�����*/
	static void ColorBk(const SkBitmap bitmap);
};

