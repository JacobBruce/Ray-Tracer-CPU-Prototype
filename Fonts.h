#pragma once
#include <d3d9.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GdiPlus.h>
#include "ReadWrite.h"

class Font
{
public:
	Font() :
		key(0),
		charWidth(0),
		charHeight(0),
		nCharsPerRow(0)
	{}
	Font(string filename, char* fontName, const UINT cWidth, const UINT cHeight, const UINT nCPR, const D3DCOLOR k)
	:	key(k),
		name(fontName),
		charWidth(cWidth),
		charHeight(cHeight),
		nCharsPerRow(nCPR)
	{
		Gdiplus::Bitmap bmp(StrToWstr(filename).c_str());
		try {
			if (bmp.GetLastStatus() == Gdiplus::Ok) {

				Gdiplus::Color pixel;

				height = bmp.GetHeight();
				width = bmp.GetWidth();

				D3DCOLOR* pixels = new D3DCOLOR[height * width];

				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x <  width; x++)
					{
						bmp.GetPixel(x, y, &pixel);
						pixels[x + (y *  width)] =
							D3DCOLOR_ARGB(pixel.GetA(), pixel.GetR(), pixel.GetG(), pixel.GetB());
					}
				}

				this->surface = pixels;

			} else {
				throw 060;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
	~Font()
	{
		delete[] surface;
	}
	void SetSurface(D3DCOLOR* surf, D3DCOLOR k)
	{
		surface = surf;
		key = k;
	}
	D3DCOLOR* GetSurface() const
	{
		return surface;
	}
	D3DCOLOR GetPixel(unsigned int x, unsigned int y) const
	{
		return surface[x + y * width];
	}
public:
	UINT width, height;
	UINT charWidth;
	UINT charHeight;
	UINT nCharsPerRow;
	D3DCOLOR key;
	char* name;
private:
	D3DCOLOR* surface;
};

class FontSet
{
public:
	FontSet() :
		fonts(NULL),
		fontCount(0)
	{}
	FontSet(Font* fontArray, UINT nCount) :
		fonts(fontArray),
		fontCount(nCount)
	{}
	~FontSet()
	{
		delete[] fonts;
	}
	Font* GetFontByID(UINT index) const
	{
		return &fonts[index];
	}
	Font* GetFontByName(const char* fontName) const
	{
		for (UINT index = 0; index < fontCount; index++)
		{
			if (fonts[index].name == fontName)
			{
				return &fonts[index];
			}
		}
		return NULL;
	}
	void SetFonts(Font* fontArray, UINT nCount)
	{
		fontCount = nCount;
		fonts = fontArray;
	}
	UINT Count() const
	{
		return fontCount;
	}
private:
	Font* fonts;
	UINT fontCount;
};