// $Id: Rectangle.h 9742 2011-10-22 16:12:27Z mrimer $


//////////////////////////////////////////////////////////////////////
// Rectangle.h
//////////////////////////////////////////////////////////////////////

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "SceneObject.h"
#include "Point.h"

//Axially-aligned rectangle -- like one face of a bounding box.

class SceneRect : public SceneObject
{
public:
	SceneRect(const Point& p1, const Point& p2);

	virtual bool intersects(const Point& Ro, const Point& Rd, float &min_t);
};

#endif
