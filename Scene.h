#pragma once
#include "MathExt.h"
#include "Colors.h"
#include "Vectors.h"
#include "Objects.h"
#include "Lights.h"

class Camera {
public:
	Vect position, orientation;
	Vect forward, right, up;
	int aptrad;
	double foclen;
public:
	Camera() {
		position = Vect(0,0,0);
		orientation = Vect(0,0,0);
		forward = Vect(0,0,1);
		right = Vect(1,0,0);
		up = Vect(0,1,0);
		foclen = 500.0;
		aptrad = 0;
	}
	Camera(Vect pos, Vect ori, Vect fwd, Vect rgt, Vect upv, double fl, int ar) :
		position(pos),
		orientation(ori),
		forward(fwd),
		right(rgt),
		up(upv),
		foclen(fl),
		aptrad(ar)
	{}
	Vect PointRelCam(const Vect& point) const
	{
		return point.vectSub(position).RotateAroundOrigin(orientation);
	}
	void updateDirection()
	{
		normAngle(orientation.x);
		normAngle(orientation.y);
		normAngle(orientation.z);
		Vect ori = orientation.vectNeg();
		forward = Vect::Z1()->RevRotAroundOrigin(ori);
		right = Vect::X1()->RevRotAroundOrigin(ori);
		up = Vect::Y1()->RevRotAroundOrigin(ori);
	}
};

class Scene {
public:
	Camera camera;
	LightSet lightSet;
	ObjectSets objectSets;
public:
	Scene()
	{
		camera = Camera();
		lightSet = LightSet();
		objectSets = ObjectSets();
	}
	Scene(Camera sCam, LightSet sLhts, ObjectSets sObjs)
	{
		camera = sCam;
		lightSet = sLhts;
		objectSets = sObjs;
	}
	void ClearScene()
	{
		lightSet.Clear();
		objectSets.FreeSets();
	}
	void LoadLevel(const string filename, MaterialSet& matSet, MeshSet* meshSet, TextureSet* textSet)
	{
		ifstream levelfile(filename);
		string line, key, data;
		unsigned int i, vers;
		unsigned int mc, tc, oc;
		unsigned int elc, flc;
		Vect ambVec;
		size_t bpos;

		ClearScene();
		meshSet->Clear();
		textSet->Clear();

		camera = Camera();

		if (levelfile.is_open()) {

			for (i = 0; i < 9; i++) {
				getline(levelfile, line);
				bpos = line.find(" ");
				key = line.substr(0, bpos);
				data = line.substr(bpos+1);
				switch (i) {
					case 0: vers = stoi(data); break;
					case 1: mc = stoi(data); break;
					case 2: tc = stoi(data); break;
					case 3: oc = stoi(data); break;
					case 4: elc = stoi(data); break;
					case 5: flc = stoi(data); break;
					case 6: camera.position = StrToVect(data); break;
					case 7: camera.orientation = StrToVect(data); break;
					case 8: ambVec = StrToVect(data); break;
				}
			}

			for (i = 0; i < mc; i++) {
				getline(levelfile, line);
				meshSet->LoadMesh(line);
			}

			for (i = 0; i < tc; i++) {
				getline(levelfile, line);
				textSet->LoadTexture(line);
			}	

			ObjectSet* objects = new ObjectSet[OBJECT_TYPES];
			for (i = 0; i < oc; i++) {
				Object* obj = new Object();
				getline(levelfile, data);
				getline(levelfile, line);
				obj->SetMeshTex(meshSet, textSet, StrToUInt3(line));
				getline(levelfile, line);
				obj->LoadObject(line, data);
				getline(levelfile, line);
				if (line != "null") { obj->ApplyTexMap(line, matSet); }
				getline(levelfile, line);
				obj->position = StrToVect(line);
				getline(levelfile, line);
				obj->orientation = StrToVect(line);
				getline(levelfile, line);
				obj->rotation = StrToVect(line);
				getline(levelfile, line);
				obj->velocity = StrToVect(line);
				if (obj->type < 0) {
					objects[LIGHTS_INDEX].InsertObject(obj);
				} else {
					objects[obj->type].InsertObject(obj);
				}
			}

			Light* e_lights = new Light[elc];
			for (i = 0; i < elc; i++) {
				getline(levelfile, line);
				e_lights[i].LoadLight(line, objects[LIGHTS_INDEX]);
				getline(levelfile, line);
				e_lights[i].SetPosition(StrToVect(line));
				getline(levelfile, line);
				e_lights[i].direction = StrToVect(line);
			}

			Light* f_lights = new Light[flc];
			for (i = 0; i < flc; i++) {
				getline(levelfile, line);
				f_lights[i].LoadLight(line, objects[LIGHTS_INDEX]);
				getline(levelfile, line);
				f_lights[i].SetPosition(StrToVect(line));
				getline(levelfile, line);
				f_lights[i].direction = StrToVect(line);
			}

			objectSets.Initialize(objects, OBJECT_TYPES);
			lightSet.Initialize(e_lights, f_lights, elc, flc, ambVec);

			levelfile.close();
		}
	}
};