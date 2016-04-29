#pragma once
#include "MathExt.h"
#include "Colors.h"
#include "Vectors.h"
#include "Objects.h"

using namespace std;

class Light {
public:
	Vect direction;
	Vect color;
	Vect position;
	Vect* boundBox;
	unsigned int bulbtype;
	double range, power;
	double radius, origRad;
	Object* object;
	string id;
public:
	Light() {
		position = Vect(0,0,0);
		direction = Vect(0,1,0);
		color = Vect(0.5,0.5,0.5);
		bulbtype = 0.0l;
		range = 0.0l;
		power = 1.0l;
		radius = 1.0l;
		origRad = 1.0l;
		object = nullptr;
		boundBox = nullptr;
		id = "";
	}
	void FreeLight()
	{
		if (boundBox != nullptr) {
			delete[] boundBox;
			boundBox = nullptr;
		}
	}
	void SetPosition(const Vect pos)
	{
		position = pos;
		if (object != nullptr) {
			object->position = position;
		}
	}
	void SetRadius(const double rad)
	{
		radius = rad;
		if (object != nullptr) {
			object->SetRadius(radius);
		}
	}
	void UpdateObject()
	{
		if (object != nullptr) {
			object->position = position;
			if (object->radius != radius) {
				object->SetRadius(radius);
				object->scale = radius / origRad;
			}
		}
	}
	void LoadObject(const string objID, ObjectSet* objGroup=nullptr)
	{
		if (objID == "null") {
			object = new Object();
			object->name = "Invisible Light";
			object->id = id+"_object";
			object->isVisible = false;
			object->isStatic = true;
			object->isSolid = false;
		} else {
			object = objGroup->ObjectByID(objID);
		}
		if (object->boundBox != nullptr) {
			boundBox = object->boundBox;
		} else if (boundBox == nullptr) {
			boundBox = new Vect[8];
			boundBox[0] = Vect(radius,radius,radius);
			boundBox[1] = Vect(-radius,radius,radius);
			boundBox[2] = Vect(radius,radius,-radius);
			boundBox[3] = Vect(-radius,radius,-radius);
			boundBox[4] = Vect(radius,-radius,radius);
			boundBox[5] = Vect(-radius,-radius,radius);
			boundBox[6] = Vect(radius,-radius,-radius);
			boundBox[7] = Vect(-radius,-radius,-radius);
		}
		object->isLightObj = true;
		object->boundBox = boundBox;
		object->color = VectToColor(color);
		UpdateObject();
	}
	void LoadLight(const string filename, ObjectSet& objGroup)
	{
		ifstream lightfile(filename);
		string lline, lkey, ldata;
		unsigned int i, lvers;
		size_t lbpos;
		string objID;

		if (lightfile.is_open()) {

			for (i = 0; i < 8; i++) {
				getline(lightfile, lline);
				lbpos = lline.find(" ");
				lkey = lline.substr(0, lbpos);
				ldata = lline.substr(lbpos+1);
				switch (i) {
					case 0: lvers = stoi(ldata); break;
					case 1: color = StrToVect(ldata); break;
					case 2: bulbtype = stoi(ldata); break;
					case 3: range = stod(ldata); break;
					case 4: power = stod(ldata); break;
					case 5: radius = stod(ldata); break;
					case 6: objID = ldata; break;
					case 7: id = ldata; break;
				}
			}

			origRad = radius;
			LoadObject(objID, &objGroup);

			lightfile.close();
		}
	}
};

class LightSet {
public:
	Light* endless_lights;
	Light* falloff_lights;
	Vect ambLight;
	unsigned int el_count;
	unsigned int fl_count;
public:
	LightSet()
	{
		endless_lights = nullptr;
		falloff_lights = nullptr;
		el_count = 0;
		fl_count = 0;
	}
	void Initialize(Light* els, Light* fls, unsigned int ect, unsigned int fct, Vect alt)
	{
		endless_lights = els;
		falloff_lights = fls;
		el_count = ect;
		fl_count = fct;
		ambLight = alt;
	}
	void Clear()
	{
		if (endless_lights != nullptr) { delete[] endless_lights; }
		if (falloff_lights != nullptr) { delete[] falloff_lights; }
		endless_lights = nullptr;
		falloff_lights = nullptr;
	}
	Light* LightByID(string setID)
	{
		for (unsigned int i = 0; i < el_count; i++)
		{
			if (falloff_lights[i].id == setID)
			{
				return &(falloff_lights[i]);
			}
		}
		return nullptr;
	}
};