#pragma once
#include "Colors.h"
#include "Vectors.h"
#include "SLinkedList.h"

class BufferSet {
public:
	Vect* raydpBuff[AA_SUB_RAYS];
	ObjNode** objnpBuff[AA_SUB_RAYS];
	SLinkedList* intnlBuff[AA_SUB_RAYS];
public:
	BufferSet() 
	{
		unsigned int pixCount = WINDOW_HEIGHT * WINDOW_WIDTH;
		for (int i=0; i<AA_SUB_RAYS; i++) {
			raydpBuff[i] = new Vect[pixCount];
			objnpBuff[i] = new ObjNode*[pixCount];
			intnlBuff[i] = new SLinkedList[pixCount];
		}
	}
	void FreeBuffers() 
	{
		for (int i=0; i<AA_SUB_RAYS; i++) {
			delete[] raydpBuff[i];
			delete[] objnpBuff[i];
			delete[] intnlBuff[i];
		}
	}
	void ClearBuffers(unsigned int pix_index)
	{
		for (unsigned int ray_index=0; ray_index<AA_SUB_RAYS; ray_index++) {
			// clear object node buffer
			objnpBuff[ray_index][pix_index] = nullptr;

			// clear the ray direction buffer
			raydpBuff[ray_index][pix_index] = Vect(0,0,0);

			// clear the intersection list buffer
			intnlBuff[ray_index][pix_index].clear();
		}
	}
	void ClearBuffers(unsigned int pix_index, unsigned int ray_index)
	{
		// clear object node buffer
		objnpBuff[ray_index][pix_index] = nullptr;

		// clear the ray direction buffer
		raydpBuff[ray_index][pix_index] = Vect(0,0,0);

		// clear the intersection list buffer
		intnlBuff[ray_index][pix_index].clear();
	}
};