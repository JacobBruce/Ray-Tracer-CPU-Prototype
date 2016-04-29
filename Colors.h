#pragma once
#include "Vectors.h"
#include <d3d9.h>

struct RGB24
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};

union RGB32
{
	D3DCOLOR value;
	struct
	{
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char alpha;
	};
};

struct RGB32Surf
{
	RGB32* colors;
	unsigned int count;
};

struct ColorBuffer 
{
	UInt4 avgRGBA;
	unsigned int count;
};

inline RGB32 CREATE_RGB32(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	RGB32 result;
	result.blue = b;
	result.green = g;
	result.red = r;
	result.alpha = a;
	return result;
}

inline RGB32 CREATE_XRGB32(unsigned char r, unsigned char g, unsigned char b)
{
	RGB32 result;
	result.blue = b;
	result.green = g;
	result.red = r;
	result.alpha = 255;
	return result;
}

inline RGB32 CREATE_ARGB32(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	RGB32 result;
	result.blue = b;
	result.green = g;
	result.red = r;
	result.alpha = a;
	return result;
}

inline RGB24 CREATE_RGB24(unsigned char r, unsigned char g, unsigned char b)
{
	RGB24 result;
	result.blue = b;
	result.green = g;
	result.red = r;
	return result;
}

inline RGB24 D3DCOLORtoRGB24(const D3DCOLOR src)
{
	RGB24 color;

	color.red = (src & 0x00FF0000) >> 16;
	color.green = (src & 0x0000FF00) >> 8;
	color.blue = (src & 0x000000FF);

	return color;
}

inline D3DCOLOR RGB24toD3DCOLOR(const RGB24 src)
{
	return D3DCOLOR_XRGB(src.red, src.green, src.blue);
}

inline Vect RGB24toVect(const RGB24 src)
{
	return Vect(src.red/(double)255, src.green/(double)255, src.blue/(double)255);
}

inline Vect RGBNtoVect(const RGB24 src)
{
	return RGB24toVect(src).scalarMult(2).scalarSub(1);
}

inline RGB32 VectToRGB32(const Vect v)
{
	return CREATE_XRGB32(v.x, v.y, v.z);
}

inline RGB32 VectToColor(const Vect v)
{
	return CREATE_XRGB32(v.x*255, v.y*255, v.z*255);
}

inline double Brightness(const RGB32& c) 
{
	return (c.red + c.green + c.blue) / 3.0;
}
	
inline RGB32 ColorScalar(const RGB32& c, double scalar) 
{
	RGB32 result;
	result.red = c.red * scalar;
	result.green = c.green * scalar;
	result.blue = c.blue * scalar;
	result.alpha = c.alpha * scalar;
	return result;
}
	
inline RGB32 ColorAdd(const RGB32& c1, const RGB32& c2) 
{
	RGB32 result;
	result.red = c1.red + c2.red;
	result.green = c1.green + c2.green;
	result.blue = c1.blue + c2.blue;
	result.alpha = c1.alpha;
	return result;
}
	
inline RGB32 ColorMultiply(const RGB32& c1, const RGB32& c2) 
{
	RGB32 result;
	result.red = c1.red * c2.red;
	result.green = c1.green * c2.green;
	result.blue = c1.blue * c2.blue;
	result.alpha = c1.alpha;
	return result;
}
	
inline RGB32 ColorAverage(const RGB32& c1, const RGB32& c2) 
{
	RGB32 result;
	result.red = (c1.red + c2.red) / 2;
	result.green = (c1.green + c2.green) / 2;
	result.blue = (c1.blue + c2.blue) / 2;
	result.alpha = c1.alpha;
	return result;
}

inline RGB32 AlphaBlend(const RGB32& c1, const RGB32& c2)
{
	RGB32 result;
	unsigned char invAlpha = (unsigned char)255 - c2.alpha;
	result.red = (c2.red * c2.alpha + c1.red * invAlpha) >> 8;
	result.green = (c2.green * c2.alpha + c1.green * invAlpha) >> 8;
	result.blue = (c2.blue * c2.alpha + c1.blue * invAlpha) >> 8;
	return result;
}

inline Vect ColorScalarV(const RGB32& c, double scalar) 
{
	Vect result;
	result.x = double(c.red) * scalar;
	result.y = double(c.green) * scalar;
	result.z = double(c.blue) * scalar;
	return result;
}
	
inline Vect ColorAddV(const RGB32& c, const Vect& v) 
{
	Vect result;
	result.x = double(c.red) + v.x;
	result.y = double(c.green) + v.y;
	result.z = double(c.blue) + v.z;
	return result;
}

inline void ColorAddV(RGB32* c, const Vect& v) 
{
	c->red = double(c->red) + v.x;
	c->green = double(c->green) + v.y;
	c->blue = double(c->blue) + v.z;
}
	
inline Vect ColorMultiplyV(const RGB32& c, const Vect& v) 
{
	Vect result;
	result.x = double(c.red) * v.x;
	result.y = double(c.green) * v.y;
	result.z = double(c.blue) * v.z;
	return result;
}

inline void ColorMultiplyV(RGB32* c, const Vect& v) 
{
	c->red = min(double(c->red) * v.x, 255);
	c->green = min(double(c->green) * v.y, 255);
	c->blue = min(double(c->blue) * v.z, 255);
}

inline RGB32 ColorClipV(const Vect& v) {

	RGB32 result;
	result.red = max(min(round(v.x), 255), 0);
	result.green = max(min(round(v.y), 255), 0);
	result.blue = max(min(round(v.z), 255), 0);

	return result;
}

const RGB32 BLACK  = CREATE_XRGB32( 0,0,0 );
const RGB32 WHITE  = CREATE_XRGB32( 255,255,255 );
const RGB32 GRAY   = CREATE_XRGB32( 128,128,128 );
const RGB32 RED    = CREATE_XRGB32( 255,0,0 );
const RGB32 GREEN  = CREATE_XRGB32( 0,255,0 );
const RGB32 BLUE   = CREATE_XRGB32( 0,0,255 );
const RGB32 YELLOW = CREATE_XRGB32( 255,255,0 );
const RGB32 ORANGE = CREATE_XRGB32( 255,111,0 );
const RGB32 BROWN  = CREATE_XRGB32( 139,69,19 );
const RGB32 PURPLE = CREATE_XRGB32( 127,0,255 );
const RGB32 AQUA   = CREATE_XRGB32( 0,255,255 );
const RGB32 VIOLET = CREATE_XRGB32( 204,0,204 );