#pragma once
#include "D3DGraphics.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Sound.h"
#include "Timer.h"
#include "FrameTimer.h"
#include "Resource.h"
#include "ReadWrite.h"
#include "MathExt.h"
#include "Colors.h"
#include "Fonts.h"
#include "Vectors.h"
#include "Materials.h"
#include "Objects.h"
#include "Scene.h"
#include "Buffers.h"
#include "SLinkedList.h"

class Game
{
public:
	Game();
	Game(HWND hWnd, KeyboardServer& kServer, MouseServer& mServer);
	void Go();
	void ComputeStage1(Object& object);
	void ComputeStage2();
	void ComputeStage3();
	void ComputeStage4();
	void SurfLighting();
	bool ScanNodeListAS(BuffNode& node);
	bool ScanNodeListTM(BuffNode& node);
	bool IsOccluding(Object& obj);
	void CalcLighting(Light* elights, Light* flights);	
	void FillPixBlocks(Vec2& topLeft, Vec2& botRight);
	void InterpolateSurf(Object& object);
	void InterpolateSimple(Triangle& tri, Vec2& uv, Object& object);
private:
	void RenderScene();
	void HandleInput();
	void BeginActions();
	void ComposeFrame();
private:
	D3DGraphics gfx;
	KeyboardClient kbd;
	MouseClient mouse;
	DSound audio;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	Scene scene;
	Font* DefFont;

	BufferSet buffSet;
	MaterialSet matSet;
	ObjectSet* objSet;
	MeshSet* meshSet;
	TextureSet* textSet;

	ObjNode* objNode;
	ObjNode tbjNode;
	BuffNode listNode;
	BuffNode tempNode;
	BuffNode lastNode;
	BuffNode* strtNode;
	BuffNode* buffNode;
	BuffNode* prevNode;
	BuffNode* currNode;
	SLListNode* lstNode;
	SLListNode* newNode;
	SLinkedList **objdpBuff;
	SLinkedList elrcObjs;
	SLinkedList tbp;

	LightLink* lLinks;
	Object* targObj;
	Light* pLight;
	Light* fLights;
	Light* eLights;
	Material* tmpMat;
	Triangle* tmpTri;
	VectSurf* nrmSurf;
	Surface* texSurf;

	Mesh lodMesh;
	Texture lodText;
	Object copyObj;
	Triangle cpyTri;
	Light light;

	Vec2 coords[8];
	Vec2 triUV, texCoords;
	Vec2 topLeft, botRight;
	RGB32 itpColor, pntColor;
	RGB32 blendList[TRANS_DEPTH];

	Vect* sumColor;
	Vect* bounds;
	Vect* triVect;
	Vect shdColor, socPnt;
	Vect crnVect, nrmVect;
	Vect ltiVect, ocpVect;
	Vect flcVect, hlfVect;
	Vect losVect, cosVect;
	Vect posVect, pntVect;
	Vect tlpVect, dirVect;
	Vect crtVect, cupVect;

	bool rayStop;
	int mX, mY, b;
	unsigned int objSetCount, pixelCount;
	unsigned int heightHalf, widthHalf;
	unsigned int fltCount, flrCount, eltCount;
	unsigned int objCount, blockCount;
	unsigned int minLod, pixIndex, arrIndex;
	unsigned int heightSpan, widthSpan;
	unsigned int s, o, p, l, t, r;
	unsigned int cX, cY, iX, iY, rX;
	double aa_inc, aa_lab, aa_sqr;
	double objDist, occDist;
	double focalLen, alphaFinal;
	double objRad, pntDist, dotProd;
	double widthFrac, heightFrac, dX, dY;
	double hminBlock, widthBlock, heightBlock;
	double minX, maxX, minY, maxY;
	double fltLod, frcLod, lodWeight;

	float deltaTime;
	Timer deltaTimer;
};