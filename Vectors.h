#pragma once
#include "MathExt.h"
#include <string>

class Vect {
public:
	double x;
	double y;
	double z;
public:
	inline Vect()
	{
		x = 0;
		y = 0;
		z = 0; 
	}
	inline Vect(double X, double Y, double Z)
	{
		x = X;
		y = Y;
		z = Z;
	}
	inline Vect(Float3& f3) 
	{
		x = f3.x;
		y = f3.y;
		z = f3.z;
	}
	inline void set(Vect& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
	inline void set(const double X, const double Y, const double Z)
	{
		x = X;
		y = Y;
		z = Z;
	}
	inline void reset()
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}
	static const Vect* X1()
	{
		static const Vect result = Vect(1,0,0);
		return &result;
	}
	static const Vect* Y1()
	{
		static const Vect result = Vect(0,1,0);
		return &result;
	}
	static const Vect* Z1()
	{
		static const Vect result = Vect(0,0,1);
		return &result;
	}
	static const Vect* V0()
	{
		static const Vect result = Vect(0,0,0);
		return &result;
	}
	inline Vect reflection(Vect& norm) const
	{
		return norm.scalarMult(-2).scalarMult(dotProd(norm)).vectAdd(*this); 
	}
	inline bool isZero() const
	{
		if (x == 0.0 && y == 0.0 && z == 0.0) {
			return true;
		} else {
			return false;
		}
	}
	inline double magnitude() const
	{
		return sqrt((x*x) + (y*y) + (z*z));
	}
	inline double squaredist(const Vect& v) const
	{
		return pow(v.x-x, 2) + pow(v.y-y, 2) + pow(v.z-z, 2);
	}
	inline double distance(const Vect& v) const
	{
		return sqrt(pow(v.x-x, 2) + pow(v.y-y, 2) + pow(v.z-z, 2));
	}
	inline double dotProd(const Vect& v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}
	inline double angle(const Vect& v) const
	{
		return acos(v.dotProd(*this));
	}
	inline Vect* normVect() 
	{
		double mag = magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
		return this;
	}
	inline Vect* negVect()
	{
		x = -x;
		y = -y;
		z = -z;
		return this;
	}
	inline Vect* avgVect(const Vect& v) 
	{
		x = (x+v.x) / 2.0l;
		y = (y+v.y) / 2.0l;
		z = (z+v.z) / 2.0l;
		return this;
	}
	inline Vect* blendVect(const Vect& v, const double alpha)
	{
		double invAlpha = 1.0l - alpha;
		x = v.x * alpha + x * invAlpha;
		y = v.y * alpha + y * invAlpha;
		z = v.z * alpha + z * invAlpha;
		return this;
	}
	inline Vect* crossVect(const Vect& v)
	{
		x = y*v.z - z*v.y;
		y = z*v.x - x*v.z;
		z = x*v.y - y*v.x;
		return this;
	}
	inline Vect* multVect(const Vect& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return this;
	}
	inline Vect* addVect(const Vect& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return this;
	}
	inline Vect* subVect(const Vect& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return this;
	}
	inline Vect* maxVect(const double& m)
	{
		x = max(x, m);
		y = max(y, m);
		z = max(z, m);
		return this;
	}
	inline Vect* minVect(const double& m)
	{
		x = min(x, m);
		y = min(y, m);
		z = min(z, m);
		return this;
	}
	inline Vect* multScalar(const double& scalar) 
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return this;
	}
	inline Vect* divScalar(const double& scalar) 
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return this;
	}
	inline Vect* addScalar(const double& scalar) 
	{
		x += scalar;
		y += scalar;
		z += scalar;
		return this;
	}
	inline Vect* subScalar(const double& scalar) 
	{
		x -= scalar;
		y -= scalar;
		z -= scalar;
		return this;
	}
	inline Vect vectNorm() const
	{
		double mag = magnitude();
		return Vect(x/mag, y/mag, z/mag);
	}
	inline Vect vectNeg() const
	{
		return Vect(-x, -y, -z);
	}
	inline Vect vectAvg(const Vect& v) const
	{
		return Vect((x+v.x) / 2.0l, (y+v.y) / 2.0l, (z+v.z) / 2.0l);
	}
	inline Vect vectBlend(const Vect& v, const double alpha) const
	{
		double invAlpha = 1.0l - alpha;
		return Vect(v.x*alpha + x*invAlpha, v.y*alpha + y*invAlpha, v.z*alpha + z*invAlpha);
	}
	inline Vect vectCross(const Vect& v) const
	{
		return Vect(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
	inline Vect vectMult(const Vect& v) const
	{
		return Vect(x * v.x, y * v.y, z * v.z);
	}
	inline Vect vectAdd(const Vect& v) const
	{
		return Vect(x + v.x, y + v.y, z + v.z);
	}
	inline Vect vectSub(const Vect& v) const
	{
		return Vect(x - v.x, y - v.y, z - v.z);
	}
	inline Vect vectMax(const Vect& v, const double& m)
	{
		return Vect(max(v.x,m), max(v.y,m), max(v.z,m));
	}
	inline Vect vectMin(const Vect& v, const double& m)
	{
		return Vect(min(v.x,m), min(v.y,m), min(v.z,m));
	}
	inline Vect scalarMult(const double& scalar) const
	{
		return Vect(x*scalar, y*scalar, z*scalar);
	}
	inline Vect scalarDiv(const double& scalar) const
	{
		return Vect(x/scalar, y/scalar, z/scalar);
	}
	inline Vect scalarAdd(const double& scalar) const
	{
		return Vect(x+scalar, y+scalar, z+scalar);
	}
	inline Vect scalarSub(const double& scalar) const
	{
		return Vect(x-scalar, y-scalar, z-scalar);
	}
	inline Vect RotateAroundX(const double& angle) const
	{
		return Vect(x, (z*sin(angle)) + (y*cos(angle)), (z*cos(angle)) - (y*sin(angle)));
	}
	inline Vect RotateAroundY(const double& angle) const
	{
		return Vect((z*sin(angle)) + (x*cos(angle)), y, (z*cos(angle)) - (x*sin(angle)));
	}
	inline Vect RotateAroundZ(const double& angle) const
	{
		return Vect((x*cos(angle)) - (y*sin(angle)), (x*sin(angle)) + (y*cos(angle)), z);
	}
	inline Vect RotateAroundOrigin(const Vect& rot) const
	{
		return RotateAroundY(rot.y).RotateAroundZ(rot.z).RotateAroundX(rot.x);
	}
	inline Vect RevRotAroundOrigin(const Vect& rot) const
	{
		return RotateAroundX(rot.x).RotateAroundZ(rot.z).RotateAroundY(rot.y);
	}
	inline bool operator==(const Vect &rhs ) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}
	inline bool operator!=(const Vect &rhs) const
	{
		return !(*this == rhs);
	}
};

inline string VectToStr(const Vect* v)
{
	return DblToStr(v->x) +","+ DblToStr(v->y) +","+ DblToStr(v->z);
}

inline Vect StrToVect(const string line)
{
	size_t bpos;
	double xpos, ypos, zpos;
	string data;
	bpos = line.find(",");
	xpos = stod(line.substr(0, bpos));
	data = line.substr(bpos+1);
	bpos = data.find(",");
	ypos = stod(data.substr(0, bpos));
	zpos = stod(data.substr(bpos+1));
	return Vect(xpos, ypos, zpos);
}