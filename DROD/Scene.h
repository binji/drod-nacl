// $Id: Scene.h 9742 2011-10-22 16:12:27Z mrimer $


//////////////////////////////////////////////////////////////////////
// Scene.h
//////////////////////////////////////////////////////////////////////

#ifndef SCENE_H
#define SCENE_H

#include "BoundingBox.h"
#include "Point.h"
#include <BackEndLib/Types.h>

#include <vector>
using std::vector;

class Light;
class SceneObject;
class Scene
{
public:
	Scene() {}

	void addObject(SceneObject* pSceneObject);

	void clear();
	bool empty() const;
	void init(const UINT x1, const UINT y1, const UINT x2, const UINT y2);
	void ready();

	bool lightShinesAt(const Point& Ro, Light *pLight);

	void analyzeTree();
	void analyzeNode(CBoundingBox* pBBox, const UINT wDepth);

private:
	CBoundingBox objectHierarchy;	//traversed during intersection tests
};

#endif
