#pragma once
#include "MathExt.h"
#include "Vectors.h"
#include "Materials.h"
#include "Triangles.h"
#include <string>
#include <fstream>
#include <assert.h>

using namespace std;

class Mesh
{
public:
	Vect* vertices;
	Vect* normals;
	Triangle* triangles;
	Vect center;
	string id;
	unsigned int tCount, vCount, nCount;
	double radius;
	Vect* boundBox;
public:
	void InitMesh()
	{
		vertices = nullptr;
		normals = nullptr;
		triangles = nullptr;
		boundBox = nullptr;
		vCount = 0;
		tCount = 0;
		radius = 0;
		center = Vect(0, 0, 0);
		id = "";
	}
	void FreeMesh()
	{
		if (vertices != nullptr)
		{
			delete[] vertices;
		}
		if (normals != nullptr)
		{
			delete[] normals;
		}
		if (triangles != nullptr)
		{
			delete[] triangles;
		}
		if (boundBox != nullptr)
		{
			delete[] boundBox;
		}
		boundBox = nullptr;
		vertices = nullptr;
		normals = nullptr;
		triangles = nullptr;
	}
	Mesh()
	{
		InitMesh();
	}
	Mesh(const string filename)
	{
		InitMesh();
		Load(filename);
	}
	void LoadMesh(const string filename)
	{
		FreeMesh();
		InitMesh();
		Load(filename);
	}
	void Load(const string filename)
	{
		ifstream myfile(filename);
		string line, key, data;
		size_t bpos;
		Vect tempVec;
		unsigned int format, version;
		unsigned int i, c, index = 0;
		unsigned int mid, vi1,vi2,vi3;
		double vmag, x, y, z;
		char tChar = '\n';
		radius = 0;

		try {
			if (myfile.is_open()) {
				for (i = 0; i < 8; i++) {
					getline(myfile, line);
					bpos = line.find(" ");
					key = line.substr(0, bpos);
					data = line.substr(bpos+1);
					switch (i) {
						case 0: version = stoi(data); break;
						case 1: format = stoi(data); break;
						case 2: vCount = stoi(data); break;
						case 3: nCount = stoi(data); break;
						case 4: tCount = stoi(data); break;
						case 5: center = StrToVect(data); break;
						case 6: radius = stod(data); break;
						case 7: id = data; break;
					}
				}

				FreeMesh();
				vertices = new Vect[vCount];
				normals = new Vect[nCount];
				triangles = new Triangle[tCount];

				for (i = 0; i < vCount; i++) {
					getline(myfile, line);
					vertices[i] = StrToVect(line);
				}

				for (i = 0; i < nCount; i++) {
					getline(myfile, line);
					normals[i] = StrToVect(line);
				}

				if (format == 0) { // text format
					for (i = 0; i < tCount; i++) {
						getline(myfile, line);
						bpos = line.find(",");
						vi1 = stoi(line.substr(0, bpos));
						line = line.substr(bpos+1);
						bpos = line.find(",");
						vi2 = stoi(line.substr(0, bpos));
						vi3 = stoi(line.substr(bpos+1));
						triangles[i].vertices = new Vect*[3];
						triangles[i].vertices[0] = &(vertices[vi1]);
						triangles[i].vertices[1] = &(vertices[vi2]);
						triangles[i].vertices[2] = &(vertices[vi3]);
						getline(myfile, line);
						bpos = line.find(",");
						if (bpos == string::npos) { // flat
							triangles[i].normals = new Vect*[1];
							triangles[i].flat = true;
							triangles[i].normals[0] = &(normals[stoi(line)]);
						} else {
							triangles[i].normals = new Vect*[3];
							triangles[i].flat = false;
							vi1 = stoi(line.substr(0, bpos));
							line = line.substr(bpos+1);
							bpos = line.find(",");
							vi2 = stoi(line.substr(0, bpos));
							vi3 = stoi(line.substr(bpos+1));
							triangles[i].normals[0] = &(normals[vi1]);
							triangles[i].normals[1] = &(normals[vi2]);
							triangles[i].normals[2] = &(normals[vi3]);
						}
					}
					boundBox = new Vect[8];
					for (i = 0; i < 8; i++) {
						getline(myfile, line);	
						boundBox[i] = StrToVect(line);
					}
				} else if (format == 1) { // binary format
					//TODO: read binary file
				}

				myfile.close();
			} else {
				throw 130;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
};

class MeshSet {
private:
	vector<Mesh**> meshes;
	vector<unsigned int> lodMap;
public:
	unsigned int count;
public:
	MeshSet()
	{
		count = 0;
	}
	~MeshSet()
	{
		Clear();
	}
	void Clear()
	{
		if (count > 0) {
			meshes.clear();
			lodMap.clear();
			count = 0;
		}
	}
	Mesh** GetMesh(unsigned int index)
	{
		return meshes[index];
	}
	unsigned int CountLoDs(unsigned int index)
	{
		return lodMap[index];
	}
	void InsertMesh(Mesh** ot, unsigned int LoD)
	{
		meshes.push_back(ot);
		lodMap.push_back(LoD);
		count++;
	}
	void LoadMesh(const string filename)
	{
		ifstream meshfile(filename);
		string mline, mkey, mdata;
		unsigned int i, mvers, mc;
		size_t mbpos;
		string mid;

		if (meshfile.is_open()) {

			for (i = 0; i < 3; i++) {
				getline(meshfile, mline);
				mbpos = mline.find(" ");
				mkey = mline.substr(0, mbpos);
				mdata = mline.substr(mbpos+1);
				switch (i) {
					case 0: mvers = stoi(mdata); break;
					case 1: mc = stoi(mdata); break;
					case 2: mid = mdata; break;
				}
			}

			Mesh** mesh = new Mesh*[mc];

			for (i = 0; i < mc; i++) {
				getline(meshfile, mline);
				mesh[i] = new Mesh();
				mesh[i]->LoadMesh(mline);
				mesh[i]->id = mid+"_LoD"+IntToStr(i);
			}

			InsertMesh(mesh, mc);
			meshfile.close();
		}
	}
};