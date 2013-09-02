// $Id: Sphere.h 9742 2011-10-22 16:12:27Z mrimer $


//////////////////////////////////////////////////////////////////////
// Sphere.h
//////////////////////////////////////////////////////////////////////

#ifndef SPHERE_H
#define SPHERE_H

#include "SceneObject.h"

class Sphere : public SceneObject
{
public:
	Sphere(const Point& center, const float radius);

	virtual bool intersects(const Point& Ro, const Point& Rd, float &min_t);

	Point center;
	float radius, radiusSquared;
};

#endif
