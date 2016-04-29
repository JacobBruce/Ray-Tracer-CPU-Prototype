#pragma once
#include <cstdlib>
#include <string>
#include <math.h>
#include <sstream>

using namespace std;

#define HALF_PI 1.5707963267948966192
#define PI		3.1415926535897932384
#define TWO_PI	6.2831853071795864769

template <typename T>
inline T sq( T val )
{
	return val * val;
}

template <typename T> 
T sgn( T val )
{
	return (T)( (T)0 <= val ) - ( val < (T)0 );
}

/*struct Point {
	int x;
	int y;
};*/

struct UInt2 {
	unsigned int x;
	unsigned int y;
};

struct UInt3 {
	unsigned int x;
	unsigned int y;
	unsigned int z;
};

struct UInt4 {
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int w;
};

struct Float3 {
	float x;
	float y;
	float z;
};

inline int round(double num)
{
	return (num > 0.0l) ? floor(num + 0.5l) : ceil(num - 0.5l);
}

inline void normAngle(double& angle)
{
	while (angle < 0.0) { angle += TWO_PI; }
	while (angle > TWO_PI) { angle -= TWO_PI; }
}

/*inline Point midpoint(Point p1, Point p2)
{
	Point p3;
	p3.x = (p1.x + p2.x) / 2;
	p3.y = (p1.y + p2.y) / 2;
	return p3;
}*/

inline void swapints(int* x1, int* x2)
{
	int temp = *x1;
	*x1 = *x2;
	*x2 = temp;
}

inline unsigned concatints(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

inline string IntToStr(int number)
{
	ostringstream s;
	s << number;
	return s.str();
}

inline string DblToStr(double number)
{
	ostringstream s;
	s << fixed << number;
	return s.str();
}

inline wstring StrToWstr(string str) 
{
	return wstring(str.begin(), str.end());
}

inline wstring IntToWstr(int number)
{
	return StrToWstr(IntToStr(number));
}

inline UInt3 StrToUInt3(string line)
{
	size_t bpos;
	unsigned int xpos, ypos, zpos;
	string data;
	UInt3 result;
	bpos = line.find(",");
	xpos = stoi(line.substr(0, bpos));
	data = line.substr(bpos+1);
	bpos = data.find(",");
	ypos = stoi(data.substr(0, bpos));
	zpos = stoi(data.substr(bpos+1));
	result.x = xpos;
	result.y = ypos;
	result.z = zpos;
	return result;
}