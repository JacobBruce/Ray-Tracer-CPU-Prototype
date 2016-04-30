#include "Game.h"
#include <time.h>
#include <cstdlib>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

Game::Game( HWND hWnd,KeyboardServer& kServer,MouseServer& mServer )
:	gfx( hWnd ),
	audio( hWnd ),
	kbd( kServer ),
	mouse( mServer )
{
	// initialize GDI
	Gdiplus::GdiplusStartup( &gdiplusToken,&gdiplusStartupInput,NULL );

	// load default font
	DefFont = new Font("Data\\fonts\\GenericFont.bmp", "generic", 16, 16, 16, BLACK.value);

	// initialize mesh and texture sets
	meshSet = new MeshSet();
	textSet = new TextureSet();

	// load material properties library
	matSet.Load("Data\\materials.mpl");

	// load objects, lights, etc, from level layout file
	scene.LoadLevel("Data\\levels\\level_0.llf", matSet, meshSet, textSet);

	// set some useful variables
	fLights = scene.lightSet.falloff_lights;
	eLights = scene.lightSet.endless_lights;
	eltCount = scene.lightSet.el_count;
	fltCount = scene.lightSet.fl_count;
	objSetCount = scene.objectSets.count;
	pixelCount = WINDOW_WIDTH * WINDOW_HEIGHT;

	widthHalf = (int)(WINDOW_WIDTH / 2);
	heightHalf = (int)(WINDOW_HEIGHT / 2);
	heightSpan = WINDOW_HEIGHT-1;
	widthSpan = WINDOW_WIDTH-1;

	blockCount = WIDTH_BLOCKS * HEIGHT_BLOCKS;
	widthBlock = WINDOW_WIDTH / WIDTH_BLOCKS;
	heightBlock = WINDOW_HEIGHT / HEIGHT_BLOCKS;
	hminBlock = min(widthBlock, heightBlock) / 2.0l;
	widthFrac = 1.0l / widthBlock;
	heightFrac = 1.0l / heightBlock;

	aa_sqr = sqrt(AA_SUB_RAYS);
	aa_inc = 1.0l / aa_sqr;
	aa_lab = aa_inc / 2.0l;

	lLinks = new LightLink[fltCount];
	objdpBuff = new SLinkedList*[WIDTH_BLOCKS];
	sumColor = new Vect[pixelCount];
	triVect = new Vect[3];

	for (cX=0; cX<WIDTH_BLOCKS; cX++) {
		objdpBuff[cX] = new SLinkedList[HEIGHT_BLOCKS];
	}

	// fill all buffers with default values
	for (pixIndex=0; pixIndex < pixelCount; pixIndex++) {
		buffSet.ClearBuffers(pixIndex);
	}
}

void Game::Go()
{
	deltaTimer.StopWatch();
	deltaTime = deltaTimer.GetTimeMilli();
	deltaTimer.StartWatch();
	gfx.BeginFrame();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::FillPixBlocks(Vec2& topLeft, Vec2& botRight)
{
	unsigned int x, y;
	bool check;
	// loop through all pixel blocks
	for (y=topLeft.y; y<botRight.y; y++) {
		for (x=topLeft.x; x<botRight.x; x++) {
			// get first node for this pixel block
			objNode = (ObjNode*)objdpBuff[x][y].first;
			lstNode = objNode;
			check = true;
			// loop through list of objects
			while (objNode != nullptr) {
				// skip if object already in list
				if (objNode->object == targObj) { 
					check = false;
					break;
				}
				// check which object is closer
				if (objNode->minDist > objDist) {
					newNode = new ObjNode(targObj, objDist);
					if (lstNode == objNode) {
						// add before first object in list
						newNode->next = objdpBuff[x][y].first;
						objdpBuff[x][y].first = newNode;
					} else {
						// ordered so closest objects first
						objdpBuff[x][y].insert_after(lstNode, newNode);
					}
					check = false;
					break;
				}
				// no insertion, try next node
				lstNode = objNode;
				objNode = objNode->Next();
			}
			// check if list is empty
			if (lstNode == nullptr) {
				objdpBuff[x][y].first = new ObjNode(targObj, objDist);
			} else if (check) {
				// insert at end of list
				newNode = new ObjNode(targObj, objDist);
				objdpBuff[x][y].insert_after(lstNode, newNode);
			}
		}
	}
}

void Game::InterpolateSurf(Object& object)
{
	// if pixel empty then skip to next ray
	if (tempNode.tript == nullptr) { return; }

	// access node data
	cpyTri = *(tempNode.tript);
	pntDist = tempNode.depth;
	triUV = tempNode.uvcrd;

	// use uv coordinates to interpolate texture map
	texCoords = interpolateTexmap(cpyTri.texmap, triUV);
	// get interpolated surface normal for shading pseudo-curved surfaces
	nrmVect = interpolateNormal(cpyTri.normals, triUV, cpyTri.flat);
	// get exact LoD for intersection point
	fltLod = max(sqrt(pntDist/focalLen), 0.0l);
	minLod = floor(fltLod); frcLod = fltLod - (double)minLod;

	// check if we should interpolate between texture LoDs
	if (fltLod > 1.0l && object.textLods > 1) {
		pntColor = CREATE_RGB32(0,0,0,255);
		// use the two best textures LoDs
		for (s=0; s<2; s++) {
			// set the texture LoD index
			object.SetTexLoD(minLod+s-1);
			// get surface texture for this triangle
			texSurf = object.GetTextureMap(cpyTri.surfIndex);
			// get normal map for this triangle
			nrmSurf = object.GetNormalMap(cpyTri.surfIndex);
			// calculate actual xy coordinates on texture
			iX = (int)(texCoords.x*(texSurf->width-1));
			iY = (int)(texCoords.y*(texSurf->height-1));
			// transform 2D xy coords into 1D array index
			arrIndex = iX + (iY * texSurf->width);
			// get weight of this texture based on LoD
			lodWeight = (s==0) ? 1.0l-frcLod : frcLod;
			// add weighted amount of color from texture
			pntColor = ColorAdd(pntColor, ColorScalar(texSurf->colors[arrIndex], lodWeight));
			//TODO: use specular map
			// if normal map exists then use it
			if (nrmSurf != nullptr) {
				// add weighted normal vector from normal map
				nrmVect.addVect(nrmSurf->vectors[arrIndex].scalarMult(lodWeight));
			}
		}
	} else {
		// use first texture
		object.textLod = 0;
		// get surface texture for this triangle
		texSurf = object.GetTextureMap(cpyTri.surfIndex);
		// get normal map for this triangle
		nrmSurf = object.GetNormalMap(cpyTri.surfIndex);
		// calculate actual xy coordinates on texture
		iX = (int)(texCoords.x*(texSurf->width-1));
		iY = (int)(texCoords.y*(texSurf->height-1));
		// transform 2D xy coords into 1D array index
		arrIndex = iX + (iY * texSurf->width);
		// get color from texture surface
		pntColor = texSurf->colors[arrIndex];
		//TODO: use specular map
		// if normal map exists then use it
		if (nrmSurf != nullptr) {
			// get normal vector from normal map
			nrmVect.addVect(nrmSurf->vectors[arrIndex]);
		}
	}

	// calculate normal vector in world space
	nrmVect = object.PointRelOrot(nrmVect).vectNorm();

	// adjust transparency based on material
	pntColor.alpha *= cpyTri.material->transparency;

	// update node data
	tempNode.color = pntColor;
	tempNode.norml = nrmVect;
}

void Game::InterpolateSimple(Triangle& tri, Vec2& uv, Object& object)
{
	// use uv coordinates to interpolate texture map
	texCoords = interpolateTexmap(tri.texmap, uv);
	// use first texture
	object.textLod = 0;
	// get surface texture for this triangle
	texSurf = object.GetTextureMap(tri.surfIndex);
	// calculate actual xy coordinates on texture
	iX = (int)(texCoords.x*(texSurf->width-1));
	iY = (int)(texCoords.y*(texSurf->height-1));
	// transform 2D xy coords into 1D array index
	arrIndex = iX + (iY * texSurf->width);
	// get color from texture surface
	pntColor = texSurf->colors[arrIndex];
	// adjust transparency based on material
	pntColor.alpha *= tri.material->transparency;
}

bool Game::IsOccluding(Object& obj)
{
	// get distance between object and surface
	objDist = obj.position.distance(pntVect) - obj.radius;
	// ensure object isn't behind the light
	if (objDist > pntDist) { return false; }
	// get distance between object and light
	objDist = obj.position.distance(light.position) - obj.radius;
	// ensure object isn't behind the surface
	if (objDist > pntDist) { return false; }
	// check if shadow ray hits object bounding sphere
	if (obj.raySphereIntersect(pntVect, losVect)) {
		// check if object is analytical sphere
		if (obj.type == -1) { return obj.isOccluder; }
		// get copy of object mesh
		lodMesh = *(obj.GetMesh());
		// loop through triangles in object
		for (t=0; t<lodMesh.tCount; t++) {
			// skip if intersection on this triangle
			if (tmpTri == &(lodMesh.triangles[t])) {
				continue;
			} else {
				cpyTri = lodMesh.triangles[t];
			}
			// check if vertices have been cached
			if (obj.vCached) {
				// copy vertices from cache
				CopyTriCache(cpyTri, triVect);
			} else {
				// get triangle vertices in world space
				CopyTriVerts(cpyTri, triVect);
				obj.TriRelWorld(triVect);	
				// cache transformed triangle vertices
				SaveTriCache(lodMesh.triangles[t], triVect);
			}
			// check if shadow ray hits this triangle
			if (secondRayTriIntersect(pntVect, losVect, triVect, triUV, occDist)) {
				// check if intersection behind light
				if (occDist > pntDist) { continue; }
				// get exact point of intersection
				socPnt = interpolatePoint(triVect, triUV);
				// check if intersection behind surface
				if (socPnt.distance(light.position) > pntDist) { continue; }
				// get color at point of intersection
				InterpolateSimple(cpyTri, triUV, obj);
				// check if light can go through surface
				if (pntColor.alpha < 255) {
					// add color to RGB pool for averaging
					shdColor = ColorAddV(pntColor, shdColor);
					// calculate how much light is getting through
					alphaFinal *= double(255 - pntColor.alpha) / 255.0l;
					s++; // increment number of intersections
					// check if light fully occluded
					if (alphaFinal <= 0.0l) {
						return true;
					}		
				} else {
					return true;
				}
			}
		}
		obj.vCached = true;
	}
	// no intersections
	return false;
}

void Game::SurfLighting()
{
	// get distance between light and surface
	pntDist = pntVect.distance(light.position);

	// get light direction ray relative to surface
	losVect = light.position.vectSub(pntVect).vectNorm();

	// loop through list of possible light occluders
	for (s=0; objNode != nullptr; objNode = objNode->Next()) {
		// check if object is occluding light
		rayStop = IsOccluding(*(objNode->object));
		// stop if object is occluder
		if (rayStop) { return; }
	}

	// check if light passed through any surfaces
	if (s > 0) {
		// blend light color with semi-transparent surfaces
		shdColor = shdColor.scalarDiv(s).scalarDiv(255.0l);
		shdColor = shdColor.vectBlend(light.color, alphaFinal);
		// account for light intensity reduced by surfaces
		shdColor.multScalar(alphaFinal);
	} else {
		// get original light color
		shdColor = light.color;
	}

	// get camera direction ray relative to surface
	cosVect = ocpVect.vectSub(pntVect).vectNorm();

	// apply Lambertian diffuse reflection to light intensity
	dotProd = max(losVect.dotProd(nrmVect), 0.0l);
	ltiVect = shdColor.vectMult(tmpMat->diffuse.scalarMult(dotProd));

	// apply Blinn-Phong specular reflection to light intensity
	if (dotProd > 0.0l) {
		hlfVect = losVect.vectAdd(cosVect).scalarMult(0.5l).vectNorm();
		dotProd = pow(hlfVect.dotProd(nrmVect), tmpMat->shininess);
		ltiVect.addVect(tmpMat->specular.scalarMult(dotProd));
	}

	// adjust light intensity based on bulb type
	if (light.bulbtype == 0) {
		// apply constant intensity over any distance
		ltiVect.multScalar(light.power);
	} else {
		// apply inverse square law to light intensity
		ltiVect.multScalar(1.0l-min(pntDist/(light.range*light.power), 1.0l));
	}

	// add final intensity to reflection buffer
	flcVect.addVect(ltiVect);
}

void Game::CalcLighting(Light* elights, Light* flights)
{
	// loop through all endless/infinite lights
	for (l = 0; l < eltCount; l++)
	{
		alphaFinal = 1.0l; // reset alpha tracker
		shdColor.reset(); // reset shadow color sum
		light = elights[l]; // get light

		// skip light if it has no power
		if (light.power > 0.0l) {
			// get first object linked to light
			objNode = (ObjNode*)elrcObjs.first;

			// calculate surface lighting
			SurfLighting();
		}
	}

	// loop through falloff/finite lights in range
	for (l = 0; l < flrCount; l++)
	{
		alphaFinal = 1.0l; // reset alpha tracker
		shdColor.reset(); // reset shadow color sum
		light = flights[lLinks[l].index]; // get light

		// skip light if it has no power
		if (light.power > 0.0l) {
			// get first object linked to light
			objNode = (ObjNode*)lLinks[l].objects.first;

			// calculate surface lighting
			SurfLighting();
		}
	}
}

bool Game::ScanNodeListAS(BuffNode& node)
{
	// get first two nodes in list
	prevNode = &node;
	currNode = node.Next();

	// find position in list for node
	for (l=0; l<TRANS_DEPTH; l++) {
		// check if at end of list
		if (currNode == nullptr) {
			// calculate intersection normal on sphere surface
			nrmVect = pntVect.vectSub(copyObj.position).vectNorm();
			// copy intersection data to result buffer node
			tempNode.Set(targObj, pntDist, pntVect, nrmVect, copyObj.color);
			// update pointer to last node in list
			lastNode = tempNode;
			return true;
		}
		// copy buffer node at current position in list
		listNode = *currNode;
		// check if intersected sphere already in list
		if (listNode.object == targObj) { break; }
		// check if at right position for insertion
		if (pntDist < listNode.depth) {
			// calculate intersection normal on sphere surface
			nrmVect = pntVect.vectSub(copyObj.position).vectNorm();
			// copy intersection data to result buffer node
			tempNode.Set(targObj, pntDist, pntVect, nrmVect, copyObj.color);
			// check if at last valid position in list
			if (l==TRANS_DEPTH-1) { lastNode = tempNode; }
			return true;
		}
		// step through intersection list
		prevNode = currNode;
		currNode = listNode.Next();
	}

	// no place in list
	return false;
}

bool Game::ScanNodeListTM(BuffNode& node)
{
	// check if invisible backfacing surface
	if (pntDist < node.depth) { return false; }

	// get first two nodes in list
	prevNode = &node;
	currNode = node.Next();

	// find position in list for node
	for (l=0; l<TRANS_DEPTH; l++) {
		// check if at end of list
		if (currNode == nullptr) {
			// save intersection data to result buffer node
			tempNode.Set(targObj, triUV, pntDist, pntVect, tmpTri);
			// update pointer to last node in list
			lastNode = tempNode;
			return true;
		}
		// copy buffer node at current position in list
		listNode = *currNode;
		// check if intersected triangle already in list
		if (listNode.tript == tmpTri) { break; }
		// check if at right position for insertion
		if (pntDist < listNode.depth) {
			// save intersection data to result buffer node
			tempNode.Set(targObj, triUV, pntDist, pntVect, tmpTri);
			// check if at last valid position in list
			if (l==TRANS_DEPTH-1) { lastNode = tempNode; }
			return true;
		}
		// step through intersection list
		prevNode = currNode;
		currNode = listNode.Next();
	}

	// no place in list
	return false;
}

void Game::HandleInput()
{
	// keys for left and right tilt
	if (kbd.KeyIsPressed(0x51)) {
		scene.camera.orientation.z -= CAMROT_SPEED*deltaTime;
	} else if (kbd.KeyIsPressed(0x45)) {
		scene.camera.orientation.z += CAMROT_SPEED*deltaTime;
	}

	// handle mouse events
	switch (mouse.ReadMouse().GetType()) {
	case MouseEvent::Move:
		if (kbd.NumlockOn()) {
			mX = mouse.GetMouseX() - widthHalf;
			mY = heightHalf - mouse.GetMouseY();
			dX = (double)mX * MOUSE_SENSI*deltaTime;
			dY = (double)mY * MOUSE_SENSI*deltaTime;
			scene.camera.orientation.x -= dY;
			scene.camera.orientation.y -= dX;
			mouse.SetMousePos(widthHalf, heightHalf);
		}
		break;
	case MouseEvent::WheelUp:
		scene.camera.foclen += 10;
		break;
	case MouseEvent::WheelDown:
		scene.camera.foclen -= 10;
		break;
	}

	// update camera direction
	scene.camera.updateDirection();

	// keys for global ambient light intensity
	if (kbd.KeyIsPressed(VK_UP)) {
		scene.lightSet.ambLight.addScalar(0.02);
		scene.lightSet.ambLight.minVect(1.0);
	} else if (kbd.KeyIsPressed(VK_DOWN)) {
		scene.lightSet.ambLight.subScalar(0.02);
		scene.lightSet.ambLight.maxVect(0.0);
	} 
	
	// keys for sun light intensity
	if (kbd.KeyIsPressed(VK_LEFT)) {
		eLights[0].power = max(eLights[0].power-0.02, 0.0);
	} else if (kbd.KeyIsPressed(VK_RIGHT)) {
		eLights[0].power = min(eLights[0].power+0.02, 1.0);
	}

	// keys for up and down movement
	if (kbd.KeyIsPressed(VK_PRIOR)) {
		posVect = scene.camera.up.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.addVect(posVect);
	} else if (kbd.KeyIsPressed(VK_NEXT)) {
		posVect = scene.camera.up.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.subVect(posVect);
	}

	// keys for forward and backward movement
	if (kbd.KeyIsPressed(0x57)) {
		posVect = scene.camera.forward.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.addVect(posVect);
	} else if (kbd.KeyIsPressed(0x53)) {
		posVect = scene.camera.forward.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.subVect(posVect);
	}

	// keys for right and left movement
	if (kbd.KeyIsPressed(0x44)) {
		posVect = scene.camera.right.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.addVect(posVect);
	} else if (kbd.KeyIsPressed(0x41)) {
		posVect = scene.camera.right.scalarMult(MOVE_SPEED*deltaTime);
		scene.camera.position.subVect(posVect);
	}	
}

void Game::BeginActions()
{
	// make copy of common cam variables
	focalLen = scene.camera.foclen;
	ocpVect = scene.camera.position;
	crtVect = scene.camera.right;
	cupVect = scene.camera.up;

	// calculate screens top left direction ray
	tlpVect = scene.camera.forward.scalarMult(focalLen).
			  vectSub(crtVect.scalarMult(widthHalf-aa_lab)).
			  vectAdd(cupVect.scalarMult(heightHalf-aa_lab));

	// reset light index
	flrCount = 0;

	// loop through limited range lights
	for (l = 0; l < fltCount; l++)
	{
		// clear old object links
		lLinks[l].objects.clear();
		// get distance between light and camera
		pntDist = fLights[l].position.distance(ocpVect);
		// test if light within valid range
		if (pntDist < MAX_LIGHT_DIST) {
			// record light index
			lLinks[flrCount].index = l;
			flrCount++;
		}
		// update object linked to light
		fLights[l].UpdateObject();
	}

	// loop through infinite range lights
	for (l = 0; l < eltCount; l++)
	{
		// update object linked to light
		eLights[l].UpdateObject();
	}

	// clear linked list for all pixel-blocks
	for (cY=0; cY<HEIGHT_BLOCKS; cY++) {
		for (cX=0; cX<WIDTH_BLOCKS; cX++) {
			objdpBuff[cY][cX].clear();
		}
	}

	// clear list of occluders used
	// for shadows of endless lights
	elrcObjs.clear();
}

void Game::ComputeStage1(Object& object)
{
	// apply motion if non-static object
	object.UpdateObject(deltaTime);

	// check if object is visible
	if (!object.isVisible) { return; }

	// get object position relative to world
	pntVect = object.position;

	// get object position relative to cam
	posVect = scene.camera.PointRelCam(pntVect);

	// get maximum radius of current object
	objRad = object.radius;

	// get minimum distance to object bounding sphere
	objDist = max(ocpVect.distance(pntVect) - objRad, 0.0l);

	// skip objects beyond max view distance
	switch (object.maxDist) {
		case 0: break;
		case 1: if (objDist > MAX_DISTANCE1) { return; } break;
		case 2: if (objDist > MAX_DISTANCE2) { return; } break;
		case 3: if (objDist > MAX_DISTANCE3) { return; } break;
		case 4: if (objDist > MAX_DISTANCE4) { return; } break;
	}

	// check if this object blocks light
	if (object.isOccluder) {
		// check if object should occlude infinite lights
		if (objDist < MAX_CSHAD_DIST) {
			// TODO: check if light-object cone intersects frustrum
			// add object to list of infinite light occluders
			objNode = new ObjNode(targObj, pntDist);
			elrcObjs.insert_first(objNode);
		}
		// loop through falloff lights within range
		for (l=0; l < flrCount; l++) {
			// get light from set of lights using link index
			light = fLights[lLinks[l].index];
			// get distance between light and object center
			pntDist = light.position.distance(object.position);
			// get distance from light to closest point on object
			pntDist = max(pntDist - object.radius, 0.0l);
			// test if light will reach object
			if (pntDist < light.range) {
				// link this object to this light
				objNode = new ObjNode(targObj, pntDist);
				lLinks[l].objects.insert_first(objNode);
			}
		}
	}

	// check if object is behind camera
	if (posVect.z+objRad <= 0.0l) { return; }

	// works best when mesh & texture detail halves with each LoD
	object.SetMeshLoD(max(sqrt(posVect.magnitude()/focalLen)-1.0l, 0.0l));

	maxX = maxY = 0.0l;
	minX = minY = DBL_MAX;
	bounds = object.boundBox;

	// transform points on bounding box to screen coordinates
	for (p=0; p<8; p++) {
		// TODO: cache bounding box vertices in world space
		pntVect = scene.camera.PointRelCam(object.PointRelWorld(bounds[p]));
		if (pntVect.z < 0.0l) {
			// fix coordinates for points behind cam
			coords[p].x = (pntVect.x < 0.0l) ? -1.0l : WINDOW_WIDTH;
			coords[p].y = (pntVect.y < 0.0l) ? WINDOW_HEIGHT : -1.0l;
		} else {
			coords[p].x = widthHalf + (pntVect.x / pntVect.z) * focalLen;
			coords[p].y = heightHalf - (pntVect.y / pntVect.z) * focalLen;
		}
		// get top left and bottom right coordinates
		minX = min(coords[p].x, minX);
		maxX = max(coords[p].x, maxX);
		minY = min(coords[p].y, minY);
		maxY = max(coords[p].y, maxY);
	}

	// cull objects not in field of view
	if (((minX < 0.0l && maxX < 0.0l) || (minX > widthSpan && maxX > widthSpan))
	|| ((minY < 0.0l && maxY < 0.0l) || (minY > widthSpan && maxY > widthSpan))) 
	{ return; }

	// clip coordinates if outside screen
	minX = min(max(minX, 0.0l), widthSpan);
	minY = min(max(minY, 0.0l), heightSpan);
	maxX = min(maxX, widthSpan);
	maxY = min(maxY, heightSpan);

	// transform screen coords to buffer coords
	topLeft = Vec2(minX*widthFrac, minY*heightFrac);
	botRight = Vec2(maxX*widthFrac, maxY*heightFrac);

	// update the object list buffer
	FillPixBlocks(topLeft, botRight);
}

void Game::ComputeStage2()
{
	// calculate normalized direction ray
	dirVect = tlpVect.vectAdd(crtVect.scalarMult(dX)).
				vectSub(cupVect.scalarMult(dY)).vectNorm();

	// save direction ray to buffer
	buffSet.raydpBuff[r][pixIndex] = dirVect;

	// reset buffer node
	tempNode.Reset();

	// get first object in pixel-block
	objNode = (ObjNode*)tbp.first;

	// loop through list of objects in pixel-block
	while (objNode != nullptr) {

		// get current object in list
		tbjNode = *objNode;
		targObj = tbjNode.object;
		copyObj = *targObj;
		
		// check if pixel already assigned
		if (tempNode.object != nullptr) {
			// break if no other objects can be closer
			if (tempNode.depth < tbjNode.minDist) { break; }
		}

		// get distance from camera to object center
		objDist = copyObj.position.squaredist(scene.camera.position);

		// check if object is analytical sphere
		if (copyObj.type == -1) {
			if (copyObj.raySphereIntersect(ocpVect, dirVect, &pntVect, &pntDist)) {
				tempNode.color = copyObj.color;
				tempNode.norml = pntVect.vectSub(copyObj.position).vectNorm();
				tempNode.depth = pntDist;
				tempNode.coord = pntVect;
				tempNode.object = tbjNode.object;
				tempNode.tript = nullptr;
				// check if the first object we've intersected
				if (buffSet.objnpBuff[r][pixIndex] == nullptr) {
					buffSet.objnpBuff[r][pixIndex] = objNode;
				}
			}
		} else {
			// check if ray hits bounding sphere of object or we're inside it
			if (objDist < copyObj.radius2 || copyObj.raySphereIntersect(ocpVect, dirVect)) {

				// get mesh with optimal LoD
				lodMesh = *(copyObj.GetMesh());

				// loop through triangles in object mesh
				for (t=0; t<lodMesh.tCount; t++) {
					tmpTri = &(lodMesh.triangles[t]);

					// check if vertices have been cached
					if (copyObj.vCached) {
						// copy vertices from cache
						CopyTriCache(*tmpTri, triVect);
					} else {
						// get triangle vertices relative to world
						CopyTriVerts(*tmpTri, triVect);
						copyObj.TriRelWorld(triVect);
						// cache transformed triangle vertices
						SaveTriCache(*tmpTri, triVect);
					}

					// if ray hits triangle return uv coordinates and distance
					if (primaryRayTriIntersect(ocpVect, dirVect, triVect, triUV, pntDist, copyObj.showBF)) {
						// check if intersection is closest so far
						if (pntDist < tempNode.depth) {
							// get exact point of intersection
							pntVect = interpolatePoint(triVect, triUV);
							// get point relative to camera
							socPnt = scene.camera.PointRelCam(pntVect);
							// check if intersection behind camera
							if (socPnt.z > 0.0l) {
								// save intersection data
								tempNode.tript = tmpTri;
								tempNode.depth = pntDist;
								tempNode.coord = pntVect;
								tempNode.uvcrd = triUV;
								tempNode.object = tbjNode.object;
								// check if the first object we've intersected
								if (buffSet.objnpBuff[r][pixIndex] == nullptr) {
									buffSet.objnpBuff[r][pixIndex] = objNode;
								}
							}
						}
					}
				}
				targObj->vCached = true;
			}
		}
		objNode = tbjNode.Next();
	}

	// check if ray intersected something
	if (tempNode.tript != nullptr) {
		// get color, normal, etc, for intersected surface
		InterpolateSurf(*(tempNode.object));
		// create new node for this ray
		buffNode = new BuffNode(tempNode);
		// make node first in list for this subray
		buffSet.intnlBuff[r][pixIndex].insert_first(buffNode);
	} else if (tempNode.object != nullptr) {
		// create new node for this ray
		buffNode = new BuffNode(tempNode);
		// make node first in list for this subray
		buffSet.intnlBuff[r][pixIndex].insert_first(buffNode);
	}
	// increment subray index
	r++;
}

void Game::ComputeStage3()
{
	// get the closest intersection point for this ray
	strtNode = (BuffNode*)buffSet.intnlBuff[r][pixIndex].first;

	// if node is empty then skip to next ray
	if (strtNode == nullptr) { return; }

	// access node data
	lastNode = *strtNode;
	itpColor = lastNode.color;

	// check if first intersection point is opaque
	if (itpColor.alpha == 255) { return; }

	// access buffer data
	dirVect = buffSet.raydpBuff[r][pixIndex];
	objNode = buffSet.objnpBuff[r][pixIndex];

	// reset ray stopper
	rayStop = false;

	// loop through remaining objects in list
	while (objNode != nullptr) {

		// get current object in list
		tbjNode = *objNode;
		targObj = tbjNode.object;	
		copyObj = *targObj;

		// check if list is full or contains opaque surface
		if (rayStop || buffSet.intnlBuff[r][pixIndex].size >= TRANS_DEPTH) {
			// break if no other objects can be closer
			if (lastNode.depth < tbjNode.minDist) { break; }
		}

		// check if object is analytical sphere
		if (copyObj.type == -1) {
			// check if ray hits bounding sphere of object
			if (copyObj.raySphereIntersect(ocpVect, dirVect, &pntVect, &pntDist)) {
				// check if we should insert new node
				if (ScanNodeListAS(*strtNode)) {
					// create new node for this ray
					buffNode = new BuffNode(tempNode);
					// insert node into list ordered by intersection distance
					buffSet.intnlBuff[r][pixIndex].insert_after(prevNode, buffNode);
					// check if the intersected surface is opaque
					if (tempNode.color.alpha == 255) {
						rayStop = true;
					}
				}
			}
		} else {
			// check if ray hits bounding sphere of object
			if (copyObj.raySphereIntersect(ocpVect, dirVect)) {

				// get mesh with optimal LoD
				lodMesh = *(copyObj.GetMesh());

				// loop through triangles in object mesh
				for (t=0; t<lodMesh.tCount; t++) {
					tmpTri = &(lodMesh.triangles[t]);

					// check if vertices have been cached
					if (copyObj.vCached) {
						// copy vertices from cache
						CopyTriCache(*tmpTri, triVect);
					} else {
						// get triangle vertices relative to world
						CopyTriVerts(*tmpTri, triVect);
						copyObj.TriRelWorld(triVect);
						// cache transformed triangle vertices
						SaveTriCache(*tmpTri, triVect);
					}

					// if ray hits triangle return uv coordinates and distance
					if (secondRayTriIntersect(ocpVect, dirVect, triVect, triUV, pntDist)) {
						// get exact point of intersection
						pntVect = interpolatePoint(triVect, triUV);
						// get point relative to camera
						socPnt = scene.camera.PointRelCam(pntVect);
						// check if we should insert new node
						if (socPnt.z > 0.0l && ScanNodeListTM(*strtNode)) {
							// get color, normal, etc, for intersected surface
							InterpolateSurf(*(tempNode.object));
							// create new node for this ray
							buffNode = new BuffNode(tempNode);
							// insert node into list ordered by intersection distance
							buffSet.intnlBuff[r][pixIndex].insert_after(prevNode, buffNode);
							// check if the intersected surface is opaque
							if (tempNode.color.alpha == 255) { rayStop = true; }
						}
					}
				}
				targObj->vCached = true;
			}
		}
		objNode = tbjNode.Next();
	}
}

void Game::ComputeStage4()
{
	// get first intersection point for this ray
	buffNode = (BuffNode*)buffSet.intnlBuff[r][pixIndex].first;

	// if node is empty then skip to next ray
	if (buffNode == nullptr) { return; }

	// loop through ray intersections
	for (p=0; buffNode != nullptr; p++) {

		// copy buffer node
		tempNode = *buffNode;

		// check transparency depth limit
		if (p >= TRANS_DEPTH) { break; }

		// access node data
		itpColor = tempNode.color;
		pntVect = tempNode.coord;
		nrmVect = tempNode.norml;
		tmpTri = tempNode.tript;
		targObj = tempNode.object;
		flcVect = Vect();

		// check if analytical sphere
		if (tmpTri == nullptr) {
			// use default material
			tmpMat = matSet.GetByIndex(0);
			// check if visualizing light sphere
			if (tempNode.object->isLightObj) {
				blendList[p] = itpColor;
				buffNode = tempNode.Next();
				continue;
			}
		}

		// get surface material
		tmpMat = tmpTri->material;		

		// compute light reflected from this surface
		CalcLighting(eLights, fLights);

		// combine ambient light with reflection intensity
		ltiVect = scene.lightSet.ambLight.vectAdd(flcVect);

		// adjust color based on material (for glowing, etc)
		ColorMultiplyV(&itpColor, tmpMat->ambient);

		// adjust color based on recieved light
		ColorMultiplyV(&itpColor, ltiVect);

		// save color to blend list
		blendList[p] = itpColor;

		// select next node in list
		buffNode = tempNode.Next();
	}

	// get last color in blend list
	itpColor = blendList[p-1];

	// blend transparent surfaces
	for (b=p-2; b>=0; b--) {
		itpColor = AlphaBlend(itpColor, blendList[b]);
	}

	// add subray color to pixel RGB pool
	sumColor[pixIndex].x += itpColor.red;
	sumColor[pixIndex].y += itpColor.green;
	sumColor[pixIndex].z += itpColor.blue;

	// clear buffers for this subray
	buffSet.ClearBuffers(pixIndex, r);
}

void Game::RenderScene()
{
	// STAGE 1: build acceleration structures
	for (s = 0; s < objSetCount; s++) 
	{
		// get pointer to current object set
		objSet = scene.objectSets.GetSetByIndex(s);

		// count number of objects in set
		objCount = objSet->count;
		
		// loop through all objects in this set
		for (o = 0; o < objCount; o++) 
		{
			// get pointer to the current object
			targObj = objSet->ObjectByIndex(o);

			// do object computations
			ComputeStage1(*targObj);
		}
	}

	// STAGE 2: find closest ray intersections
	for (cY = 0; cY < WINDOW_HEIGHT; cY++) {
		for (cX = 0; cX < WINDOW_WIDTH; cX++) {

			// compute pixel index and pixel-block
			pixIndex = cX + (cY * WINDOW_WIDTH);
			iX = double(cX) * widthFrac;
			iY = double(cY) * heightFrac;
			tbp = objdpBuff[iX][iY];
			r = 0;

			// shoot multiple rays through each pixel
			for (dY=cY; r<AA_SUB_RAYS; dY += aa_inc) {
				for (rX=0; rX<(int)aa_sqr; rX++) {

					// compute x-axis of sub ray
					dX = double(cX) + (aa_inc * double(rX));

					// do primary ray computations
					ComputeStage2();
				}
			}
		}
	}

	// STAGE 3: send rays through transparent surfaces
	for (pixIndex = 0; pixIndex < pixelCount; pixIndex++) {

		// loop through each ray in pixel
		for (r=0; r<AA_SUB_RAYS; r++) {

			// do transparency computations
			ComputeStage3();
		}
	}

	// STAGE 4: calculate lighting (pixel/subray shading)
	for (pixIndex = 0; pixIndex < pixelCount; pixIndex++) {

		// reset pixel RGB pool
		sumColor[pixIndex].reset();

		// loop through each ray in pixel
		for (r=0; r<AA_SUB_RAYS; r++) {

			// do lighting computations
			ComputeStage4();
		}

		// average colors from all subrays
		flcVect = sumColor[pixIndex].scalarDiv(AA_SUB_RAYS);

		// write final pixel color to screen
		gfx.PutPixel(pixIndex, ColorClipV(flcVect).value);
	}
}

void Game::ComposeFrame()
{
	// handle keyboard/mouse actions
	HandleInput();

	// reset/update some stuff
	BeginActions();

	// render the 3D scene
	RenderScene();

	// draw some text to the screen
	gfx.DrawString(("Focal Length: "+DblToStr(focalLen)).c_str(), 10, 10, -5, DefFont, WHITE);
	gfx.DrawString(("Sun Intensity: "+DblToStr(eLights[0].power)).c_str(), 10, 30, -5, DefFont, ORANGE);
	gfx.DrawString(("Ambient Light: "+DblToStr(scene.lightSet.ambLight.x)).c_str(), 10, 50, -5, DefFont, GREEN);

	// TODO: pre-compute static light-object lists
	// TODO: use accell structure on object meshes
	// TODO: analytical soft-shadows (penumbras)
	// TODO: reflections via environment mapping
	// TODO: ambient occlusion, global illumination
	// TODO: volumetric lighting (dust, fog, etc)
	// TODO: physically based rendering (better shading)
	// TODO: multi-layer textures for sub-surface effects
	// TODO: subsurface scattering (diffuse profiles?)
	// TODO: bloom, depth of field, motion blur
	// TODO: main menu and level loading screen
}