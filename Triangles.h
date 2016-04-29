#pragma once
#include "MathExt.h"
#include "Vectors.h"
#include "Vec2.h"
#include <string>

class Triangle {
public:
	Vect vertCache[3];
	Vect** vertices;
	Vect** normals;
	Material* material;
	Vec2* texmap;
	bool flat;
	unsigned int surfIndex;
public:
	Triangle()
	{
		vertices = nullptr;
		texmap = nullptr;
		normals = nullptr;
		material = nullptr;
		flat = true;
		surfIndex = 0;
	}
	void Free()
	{
		if (texmap != nullptr) { 
			delete[] texmap;
			texmap = nullptr;
		}
	}
	void UpdateTex(unsigned int tind, Vec2* txmp, Material* mtrl)
	{
		surfIndex = tind;
		texmap = txmp;
		material = mtrl;
	}
};

inline void CopyTriVerts(Triangle& triobj, Vect* trivec)
{
	trivec[0] = *(triobj.vertices[0]);
	trivec[1] = *(triobj.vertices[1]);
	trivec[2] = *(triobj.vertices[2]);
}

inline void CopyTriCache(Triangle& triobj, Vect* trivec)
{
	trivec[0] = triobj.vertCache[0];
	trivec[1] = triobj.vertCache[1];
	trivec[2] = triobj.vertCache[2];
}

inline void SaveTriCache(Triangle& triobj, Vect* trivec)
{
	triobj.vertCache[0] = trivec[0];
	triobj.vertCache[1] = trivec[1];
	triobj.vertCache[2] = trivec[2];
}

inline Vect interpolatePoint(Vect* tri, Vec2& uv)
{
	return tri[1].scalarMult(uv.x).
	vectAdd(tri[2].scalarMult(uv.y)).
	vectAdd(tri[0].scalarMult(1.0l - uv.x - uv.y));
}

inline Vec2 interpolateTexmap(Vec2* map, Vec2& uv)
{
	return (map[1] * uv.x) + (map[2] * uv.y) +
		   (map[0] * (1.0l - uv.x - uv.y));
}

inline Vect interpolateNormal(Vect** map, Vec2& uv, const bool flat)
{
	switch (flat) {
		case false: 
			return map[1]->scalarMult(uv.x).
			vectAdd(map[2]->scalarMult(uv.y)).
			vectAdd(map[0]->scalarMult(1.0l - uv.x - uv.y));
			break;
		default:
			return *(map[0]); break;
	}
}

// test primary ray against triangle for intersection
inline bool primaryRayTriIntersect(const Vect orig, const Vect dir, Vect* tri, Vec2& uv, double& t, const bool b)
{
	Vect v0v1 = tri[1].vectSub(tri[0]);
	Vect v0v2 = tri[2].vectSub(tri[0]);
	Vect pvec = dir.vectCross(v0v2);
	double det = v0v1.dotProd(pvec);
	
	// if the determinant is negative the triangle is backfacing
	if (!b && det < 0.0l) { return false; }
	// if the determinant is close to 0, ray is parallel to triangle
	if (labs(det) < EPSILON) { return false; }

	double invDet = 1.0l / det;

	Vect tvec = orig.vectSub(tri[0]);
	uv.x = tvec.dotProd(pvec) * invDet;
	if (uv.x < 0.0l || uv.x > 1.0l) { return false; }

	Vect qvec = tvec.vectCross(v0v1);
	uv.y = dir.dotProd(qvec) * invDet;
	if (uv.y < 0.0l || uv.x + uv.y > 1.0l) { return false; }

	t = v0v2.dotProd(qvec) * invDet;

	return true;
}

// test secondary ray against triangle for intersection
inline bool secondRayTriIntersect(const Vect orig, const Vect dir, Vect* tri, Vec2& uv, double& t)
{
	Vect v0v1 = tri[1].vectSub(tri[0]);
	Vect v0v2 = tri[2].vectSub(tri[0]);
	Vect pvec = dir.vectCross(v0v2);
	double det = v0v1.dotProd(pvec);

	if (labs(det) < EPSILON) { return false; }

	double invDet = 1.0l / det;

	Vect tvec = orig.vectSub(tri[0]);
	uv.x = tvec.dotProd(pvec) * invDet;
	if (uv.x < 0.0l || uv.x > 1.0l) { return false; }

	Vect qvec = tvec.vectCross(v0v1);
	uv.y = dir.dotProd(qvec) * invDet;
	if (uv.y < 0.0l || uv.x + uv.y > 1.0l) { return false; }

	t = v0v2.dotProd(qvec) * invDet;

	return true;
}