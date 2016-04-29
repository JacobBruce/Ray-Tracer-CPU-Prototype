#pragma once
#include "MathExt.h"
#include "Vectors.h"
#include <string>
#include <fstream>
#include <assert.h>

struct Material {
	Vect ambient;
	Vect diffuse;
	Vect specular;
	double shininess;
	double glossiness;
	double reflectivity;
	double transparency;
	double refractIndex;
	string name;
};

class MaterialSet 
{
public:
	Material* materials;
	unsigned int count;
public:
	void InitSet()
	{
		materials = nullptr;
		count = 0;
	}
	void FreeSet()
	{
		if (materials != nullptr) {
			delete[] materials;
		}
		materials = nullptr;
	}
	MaterialSet() 
	{
		InitSet();
	}
	MaterialSet(const string filename)
	{
		InitSet();
		Load(filename);
	}
	~MaterialSet() {
		FreeSet();
	}
	Material* GetByIndex(unsigned int index)
	{
		return &(materials[index]);
	}
	Material* GetByName(const string name)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (materials[i].name == name)
			{
				return &(materials[i]);
			}
		}
		return nullptr;
	}
	void Load(const string filename)
	{
		ifstream myfile(filename);
		string line, key, data;
		size_t bpos;
		unsigned int i, l, version;

		try {
			if (myfile.is_open()) {

				for (i = 0; i < 2; i++) {
					getline(myfile, line);
					bpos = line.find(" ");
					key = line.substr(0, bpos);
					data = line.substr(bpos+1);
					switch (i) {
						case 0: version = stoi(data); break;
						case 1: count = stoi(data); break;
					}
				}

				FreeSet();
				materials = new Material[count];

				for (i = 0; i < count; i++) {
					for (l = 0; l < 9; l++) {
						getline(myfile, line);
						switch (l) {
							case 0: materials[i].name = line; break;
							case 1: materials[i].ambient = StrToVect(line); break;
							case 2: materials[i].diffuse = StrToVect(line); break;
							case 3: materials[i].specular = StrToVect(line); break;
							case 4: materials[i].shininess = stod(line); break;
							case 5: materials[i].glossiness = stod(line); break;
							case 6: materials[i].reflectivity = stod(line); break;
							case 7: materials[i].transparency = stod(line); break;
							case 8: materials[i].refractIndex = stod(line); break;
						}
					}
				}

				myfile.close();

			} else {
				throw 110;
			}
		} catch (int ecode) {
			string emsg = ErrorCodeToStr(ecode)+filename;
			HandleFatalError(ecode, emsg);
		}
	}
};