#pragma once
#include "Colors.h"
#include "Fonts.h"
#include "resource.h"
#include "MathExt.h"
#include "Vec2.h"
#include <math.h>
#include <assert.h>
#include <emmintrin.h>
#include <d3d9.h>
#include <GdiPlus.h>
#pragma comment(lib, "gdiplus.lib")

class D3DGraphics
{
public:
	D3DGraphics( HWND hWnd );
	~D3DGraphics();
	void PutPixel(int x, int y, int r, int g, int b);
	void PutPixel(int x, int y, const D3DCOLOR& c);
	void PutPixel(unsigned int index, const D3DCOLOR& c);
	D3DCOLOR GetPixel(int x, int y);
	D3DCOLOR GetPixel(unsigned int index);
	void DrawLine(int x1,int y1,int x2,int y2, const D3DCOLOR& c);
	void DrawCircle(int cx,int cy,int radius,int r,int g,int b);
	void DrawCircle(int cx, int cy, int rad, D3DCOLOR c);
	void DrawDisc(int cx,int cy,int r,int rd,int g,int b);
	void DrawDisc(int cx, int cy, int rad, D3DCOLOR c);
	void DrawChar(char ch,int xo,int yo,Font* font,const RGB32* c);
	void DrawString(const char* str,int x,int y,int horiPad,Font* font,const RGB32& c);
	void DrawFlatTriangle(float y0,float y1,float m0,float b0,float m1,float b1,const RGB32& c);
	void DrawTriangle(Vec2 v0,Vec2 v1,Vec2 v2,const RGB32& c);
	void DrawRectangle(const Vec2& topLeft, const Vec2& botRight, const RGB32& c);
	void BeginFrame();
	void EndFrame();
private:
	IDirect3D9*			pDirect3D;
	IDirect3DDevice9*	pDevice;
	IDirect3DSurface9*	pBackBuffer;
	D3DCOLOR*			pSysBuffer;
	D3DLOCKED_RECT		backRect;
	ULONG_PTR			gdiplusToken;
};