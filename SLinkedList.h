#pragma once
#include "Objects.h"

class SLListNode {
public:
	SLListNode* next;
public:
	SLListNode()
	{
		next = nullptr;
	}
};

class ObjNode : public SLListNode {
public:
	Object* object;
	double minDist;
public:
	ObjNode()
	{
		next = nullptr;
		object = nullptr;
		minDist = 0.0;
	}
	ObjNode(Object* obj, double min)
	{
		object = obj; 
		minDist = min;
		next = nullptr;
	}
	ObjNode* Next()
	{
		return (ObjNode*)next;
	}
};

class BuffNode : public SLListNode {
public:
	RGB32 color;
	double depth;
	Vect coord;
	Vect norml;
	Vec2 uvcrd;
	Object* object;
	Triangle* tript;
	BuffNode()
	{
		Reset();
	}
	void Reset()
	{
		next = nullptr;
		tript = nullptr;
		object = nullptr;
		depth = DBL_MAX;
		color = BLACK;
	}
	void Set(Object* obj, Vec2 uvc, double dpt, Vect crd, Triangle* tri)
	{
		object = obj;
		uvcrd = uvc;
		depth = dpt;
		coord = crd;
		tript = tri; 
		next = nullptr;
	}
	void Set(Object* obj, double dpt, Vect crd, Vect nrm, RGB32 clr)
	{
		object = obj;
		color = clr;
		norml = nrm;
		depth = dpt;
		coord = crd;
		tript = nullptr; 
		next = nullptr;
	}
	BuffNode* Next()
	{
		return (BuffNode*)next;
	}
};

class SLinkedList {
public:
	SLListNode* first;
	unsigned int size;
public:
	SLinkedList()
	{
		first = nullptr; 
		size = 0;
	}
	inline void insert_first(SLListNode* input) 
	{
		input->next = first;
		first = input;
		size++;
	}
	inline void insert_after(SLListNode* target, SLListNode* input) 
	{
		input->next = target->next;
		target->next = input;
		size++;
	}
	inline void clear()
	{
		SLListNode* fnp = first;
		SLListNode* nnp = fnp;
		while (nnp != nullptr) {
			nnp = fnp->next;
			delete fnp;
			fnp = nnp;
		}
		first = nullptr;
		size = 0;
	}
};

struct LightLink {
	unsigned int index;
	SLinkedList objects;
};