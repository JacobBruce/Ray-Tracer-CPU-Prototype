#pragma once
#include "MathExt.h"
#include "Vectors.h"
#include "Textures.h"
#include "Meshes.h"
#include <queue>

using namespace std;

class Object {
private:
	Texture** texture;
	Mesh** mesh;
public:
	RGB32 color;
	Vect* boundBox;
	Vect center, lastPos, lastOri;
	Vect position, orientation;
	Vect velocity, rotation;
	bool isVisible, isSolid;
	bool isStatic, vCached;
	bool isOccluder, isLightObj;
	bool showBF; int type;
	unsigned int maxDist;
	unsigned int meshLod, textLod;
	unsigned int meshLods, textLods;
	double radius, radius2, scale;
	double mass, origMass;
	string name, id;
public:
	void FreeObject()
	{
		if (mesh != nullptr) { 
			delete[] mesh;
			mesh = nullptr;
		}
		if (texture != nullptr) { 
			delete[] texture;
			texture = nullptr;
		}
	}
	void InitObject()
	{
		center = Vect(0, 0, 0);
		lastPos = Vect(0, 0, 0);
		position = Vect(0, 0, 0);
		orientation = Vect(0, 0, 0);
		velocity = Vect(0, 0, 0);
		rotation = Vect(0, 0, 0);
		color = WHITE;
		mesh = nullptr;
		texture = nullptr;
		boundBox = nullptr;
		isVisible = true;
		isSolid = true;
		isStatic = true;
		isOccluder = true;
		isLightObj = false;
		vCached = false;
		showBF = false;
		maxDist = 0;
		type = 0;
		name = "";
		id = "";
		scale = 1;
		meshLods = 0;
		textLods = 0;
		meshLod = 0;
		radius = 0.0l;
		radius2 = 0.0l;
		mass = 0.0l;
		origMass = 0.0l;
	}
	Object()
	{
		InitObject();
	}
	Mesh* GetMesh()
	{
		return mesh[meshLod];
	}
	Surface* GetTextureMap(unsigned int sIndex)
	{
		return &(texture[textLod]->surface[sIndex]);
	}
	VectSurf* GetNormalMap(unsigned int sIndex)
	{
		VectSurf* nm = texture[textLod]->normalMap;
		return (nm == nullptr) ? nullptr : &(nm[sIndex]);
	}
	void SetTexLoD(const double lod)
	{
		if (lod < textLods) {
			textLod = floor(lod);
		} else {
			textLod = textLods-1;
		}
	}
	void SetMeshLoD(const double lod)
	{
		if (lod < meshLods) {
			meshLod = floor(lod);
		} else {
			meshLod = meshLods-1;
		}
	}
	void SetMesh(unsigned int index, Mesh* pMeshes)
	{
		mesh[index] = pMeshes;
	}
	void SetTexture(unsigned int index, Texture* pTexts)
	{
		texture[index] = pTexts;
	}
	void SetMeshTex(MeshSet* meshSet, TextureSet* textSet, UInt3 targ)
	{
		if (targ.x == 0 || targ.y == 0) { 
			meshLods = 0;
			textLods = 0;
			return;
		} else {
			--targ.x;
			--targ.y;
		}

		Mesh** msh = meshSet->GetMesh(targ.x);
		Texture** txt = textSet->GetTexture(targ.y);
		FreeObject();
		meshLods = meshSet->CountLoDs(targ.x);
		textLods = textSet->CountLoDs(targ.y);

		if (targ.z) {
			mesh = new Mesh*[meshLods];
			texture = new Texture*[textLods];
			for (unsigned int i=0; i<meshLods; i++) {
				*(mesh[i]) = *(msh[i]);
			}
			for (unsigned int i=0; i<textLods; i++) {
				*(texture[i]) = *(txt[i]);
			}
		} else {
			texture = txt;
			mesh = msh;
		}
	}
	Vect PointRelCenter(const Vect& pnt)
	{
		return pnt.vectSub(center);
	}
	Vect PointRelOrot(const Vect& pnt)
	{
		return pnt.RotateAroundOrigin(orientation);
	}
	Vect PointRelWorld(const Vect& pnt)
	{
		return PointRelOrot(PointRelCenter(pnt).scalarMult(scale)).vectAdd(position);
	}
	void TriRelWorld(Vect* tri)
	{
		for (unsigned int i=0; i<3; i++) {
			tri[i] = PointRelWorld(tri[i]);
		}
	}
	void SetNameAndID(string n, string i)
	{
		name = n; id = i;
	}
	void SetRadius(const double newRad)
	{
		radius = newRad;
		radius2 = pow(radius, 2);
	}
	void SetScale(const double newScale)
	{
		scale = newScale;
		mass = pow(scale,3) * origMass;
		SetRadius(scale * mesh[0]->radius);
		vCached = false;
	}
	void LoadObject(const string filename, const string objID)
	{
		ifstream objfile(filename);
		string oline, okey, odata;
		string meshfile, textfile;
		unsigned int i, version;
		size_t obpos;

		if (objfile.is_open()) {

			for (i = 0; i < 11; i++) {
				getline(objfile, oline);
				obpos = oline.find(" ");
				okey = oline.substr(0, obpos);
				odata = oline.substr(obpos+1);
				switch (i) {
					case 0: version = stoi(odata); break;
					case 1: isStatic = stoi(odata); break;
					case 2: isSolid = stoi(odata); break;
					case 3: isVisible = stoi(odata); break;
					case 4: isOccluder = stoi(odata); break;
					case 5: showBF = stoi(odata); break;
					case 6: maxDist = stoi(odata); break;
					case 7: scale = stod(odata); break;
					case 8: origMass = stod(odata); break; 
					case 9: type = stoi(odata); break;
					case 10: name = odata; break;
				}
			}

			id = objID;
			mass = origMass;

			if (type >= 0) {
				SetRadius(scale * mesh[0]->radius);
				boundBox = mesh[0]->boundBox;
				center = mesh[0]->center;
			}
			
			objfile.close();
		}
	}
	void ApplyTexMap(const string filename, MaterialSet& mtrls) 
	{
		ifstream mapfile(filename);
		string line;
		string key, data;
		unsigned int i, ti, index;
		unsigned int version, format;
		Vec2* tmpVec;
		size_t bpos;

		try {
			if (mapfile.is_open()) {

				for (i = 0; i < 2; i++) {
					getline(mapfile, line);
					bpos = line.find(" ");
					key = line.substr(0, bpos);
					data = line.substr(bpos+1);
					switch (i) {
						case 0: version = stoi(data); break;
						case 1: format = stoi(data); break;
					}
				}
				if (format == 0) {
					for (index=0; index<meshLods; index++) {
						for (i=0; i<mesh[index]->tCount; i++) {
							getline(mapfile, line);
							bpos = line.find(" ");
							key = line.substr(0, bpos);
							data = line.substr(bpos+1);
							ti = stoi(key);
							tmpVec = new Vec2[3];
							getline(mapfile, line);
							tmpVec[0] = StrToVec2(line);
							getline(mapfile, line);
							tmpVec[1] = StrToVec2(line);
							getline(mapfile, line);
							tmpVec[2] = StrToVec2(line);
							mesh[index]->triangles[i].UpdateTex(ti, tmpVec, mtrls.GetByName(data));
						}
					}
				} else {
					//TODO: read binary format (fast)
				}
			} else {
				throw 150;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
	inline bool raySphereIntersect(const Vect orig, const Vect dir, Vect* pos, double* dst)
	{
		double t0, t1;
		Vect L = position.vectSub(orig);
		double tca = L.dotProd(dir);
		if (tca < 0.0l) { return false; }
		double d2 = L.dotProd(L) - tca * tca;
		if (d2 > radius2) { return false; }
		double thc = sqrt(radius2 - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		if (t0 > t1) { swap(t0, t1); }

		if (t0 < 0.0l) {
			t0 = t1;
			if (t0 < 0.0l) { return false; }
		}

		*dst = t0;
		*pos = orig.vectAdd(dir.scalarMult(t0));

		return true;
	}
	inline bool raySphereIntersect(const Vect orig, const Vect dir)
	{
		double t0, t1;
		Vect L = position.vectSub(orig);
		double tca = L.dotProd(dir);
		if (tca < 0.0l) { return false; }
		double d2 = L.dotProd(L) - tca * tca;
		if (d2 > radius2) { return false; }
		double thc = sqrt(radius2 - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		if (t0 > t1) { swap(t0, t1); }

		if (t0 < 0.0l) {
			if (t1 < 0.0l) { return false; }
		}

		return true;
	}
	void UpdateObject(const float deltaTime) {
		lastPos = position;
		lastOri = orientation;
		if (!isStatic) {
			orientation.addVect(rotation.scalarMult(deltaTime));
			position.addVect(velocity.scalarMult(deltaTime));
			// reset vertex cache if object moved or rotated
			if (position != lastPos || orientation != lastOri) {
				vCached = false;
			}
		}
	}
};

class ObjectSet {
private:
	Object* objects[MAX_OBJECTS];
	queue<unsigned int> emptySlots;
public:
	string name, id;
	unsigned int count;
public:
	ObjectSet()
	{
		count = 0;
	}
	ObjectSet(Object* obsPtr, unsigned int oCnt)
	{
		SetObjects(obsPtr, oCnt);
	}
	void ClearObjects()
	{
		for (unsigned int i = 0; i < count; i++) {
			if (objects[i] != nullptr) {
				objects[i]->FreeObject();
			}
		}
		emptySlots = queue<unsigned int>();
		count = 0;
	}
	Object* ObjectByIndex(const unsigned int index)
	{
		return objects[index];
	}
	Object* ObjectByName(string objectName)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (objects[i]->name == objectName)
			{
				return objects[i];
			}
		}
		return nullptr;
	}
	Object* ObjectByID(string objectID)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (objects[i]->id == objectID)
			{
				return objects[i];
			}
		}
		return nullptr;
	}
	void SetObjects(Object* objArray, const unsigned int nCount)
	{
		count = nCount;
		*objects = objArray;
	}
	void InsertObject(Object* obj)
	{
		if (emptySlots.empty()) {
			objects[count] = obj;
		} else {
			objects[emptySlots.front()] = obj;
			emptySlots.pop();
		}
		count++;
	}
	void RemoveObject(unsigned int index)
	{
		emptySlots.push(index);
		delete objects[index];
		objects[index] = nullptr;
		count--;
	}
	void SetNameAndID(string n, string i)
	{
		name = n;
		id = i;
	}
};

class ObjectSets {
private:
	ObjectSet* sets;
public:
	unsigned int count;
public:
	ObjectSets()
	{
		sets = nullptr;
		count = 0;
	}
	void FreeSets()
	{
		delete[] sets;
		count = 0;
	}
	void Initialize(ObjectSet* sts, unsigned int cnt)
	{
		sets = sts;
		count = cnt;
	}
	ObjectSet* GetSetByIndex(unsigned int index)
	{
		return &sets[index];
	}
	ObjectSet* GetSetByName(string setName)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (sets[i].name == setName)
			{
				return &(sets[i]);
			}
		}
		return nullptr;
	}
	ObjectSet* GetSetByID(string setID)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (sets[i].id == setID)
			{
				return &(sets[i]);
			}
		}
		return nullptr;
	}
};