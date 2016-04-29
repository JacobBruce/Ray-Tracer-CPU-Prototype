#pragma once
#include "Colors.h"
#include "MathExt.h"
#include "Vectors.h"
#include <string>
#include <fstream>
#include <assert.h>
#include <gdiplus.h>

using namespace std;

struct Surface
{
	RGB32* colors;
	unsigned int height;
	unsigned int width;
	unsigned int count;
};

struct Surf24
{
	RGB24* colors;
	unsigned int height;
	unsigned int width;
	unsigned int count;
};

struct VectSurf
{
	Vect* vectors;
	unsigned int height;
	unsigned int width;
	unsigned int count;
};

class Texture
{
public:
	Surface* surface;
	VectSurf* normalMap;
	string id;
	unsigned int count;
public:
	Texture()
	{
		InitTexture();
	}
	Texture(unsigned int layers)
	{
		InitTexture();
		count = layers;
		surface = new Surface[count];
		normalMap = new VectSurf[count];
	}
	void InitTexture() 
	{
		normalMap = nullptr;
		surface = nullptr;
		count = 0;
		id = "";
	}
	void FreeTexture() 
	{
		if (surface != nullptr) {
			delete[] surface;
			surface = nullptr;
		}
		if (normalMap != nullptr) {
			delete[] normalMap;
			normalMap = nullptr;
		}
		count = 0;
	}
	void LoadTexture(const string filename, unsigned int index)
	{
		surface[index] = LoadSurface(filename);
	}
	void LoadNormalMap(const string filename, unsigned int index)
	{
		normalMap[index] = LoadNormMap(filename);
	}
	Surface LoadSurface(const string filename)
	{
		try {
			ifstream file(filename);
			if (file.is_open()) {
				file.close();
				Gdiplus::Bitmap bitmap( StrToWstr(filename).c_str() );
				Gdiplus::Color pixel;

				Surface surf;
				surf.width = bitmap.GetWidth();
				surf.height = bitmap.GetHeight();
				surf.count = surf.width * surf.height;
				surf.colors = new RGB32[surf.count];

				int yy=0;

				for( int y = surf.height-1; y >= 0; y-- ) {
					for( int x = 0; x < surf.width; x++ )
					{
						bitmap.GetPixel( x,y,&pixel );
						surf.colors[ x + yy * surf.width ] = CREATE_ARGB32(pixel.GetR(), pixel.GetG(), pixel.GetB(), pixel.GetA());
					}
					yy++;
				}

				return surf;
			} else {
				throw 200;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
	VectSurf LoadNormMap(const string filename)
	{
		try {
			ifstream file(filename);
			if (file.is_open()) {
				file.close();
				Gdiplus::Bitmap bitmap( StrToWstr(filename).c_str() );
				Gdiplus::Color pixel;

				VectSurf surf;
				surf.width = bitmap.GetWidth();
				surf.height = bitmap.GetHeight();
				surf.count = surf.width * surf.height;
				surf.vectors = new Vect[surf.count];

				int yy=0;

				for( int y = surf.height-1; y >= 0; y-- ) {
					for( int x = 0; x < surf.width; x++ )
					{
						bitmap.GetPixel( x,y,&pixel );
						surf.vectors[ x + yy * surf.width ] = RGBNtoVect(CREATE_RGB24(pixel.GetR(), pixel.GetG(), pixel.GetB()));
					}
					yy++;
				}

				return surf;
			} else {
				throw 200;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
};


class TextureSet {
private:
	vector<Texture**> textures;
	vector<unsigned int> lodMap;
public:
	unsigned int count;
public:
	TextureSet()
	{
		count = 0;
	}
	~TextureSet()
	{
		Clear();
	}
	void Clear()
	{
		textures.clear();
		lodMap.clear();
		count = 0;
	}
	Texture** GetTexture(unsigned int index)
	{
		return textures[index];
	}
	unsigned int CountLoDs(unsigned int index)
	{
		return lodMap[index];
	}
	void InsertTexture(Texture** ot, unsigned int LoD)
	{
		textures.push_back(ot);
		lodMap.push_back(LoD);
		count++;
	}
	void LoadTexture(const string filename)
	{
		ifstream textfile(filename);
		string tline, tkey, tdata;
		unsigned int i, l, tvers;
		unsigned int lc, tc;
		size_t tbpos;
		string tdir, tid;

		if (textfile.is_open()) {

			for (i = 0; i < 5; i++) {
				getline(textfile, tline);
				tbpos = tline.find(" ");
				tkey = tline.substr(0, tbpos);
				tdata = tline.substr(tbpos+1);
				switch (i) {
					case 0: tvers = stoi(tdata); break;
					case 1: lc = stoi(tdata); break;
					case 2: tc = stoi(tdata); break;
					case 3: tid = tdata; break;
					case 4: tdir = tdata; break;
				}
			}

			Texture** newText = new Texture*[lc];

			for (l = 0; l < lc; l++) {
				newText[l] = new Texture(tc);
				newText[l]->id = tid+"_LoD"+IntToStr(l);
				for (i=0; i < tc; i++) {
					getline(textfile, tline);
					newText[l]->LoadTexture(tdir+tline, i);
					getline(textfile, tline);
					if (tline != "null") {
						newText[l]->LoadNormalMap(tdir+tline, i);
					} else {
						newText[l]->normalMap = nullptr;
					}
				}
			}

			InsertTexture(newText, lc);
			textfile.close();
		}
	}
};