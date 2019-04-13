#pragma once
#include "Point2D.h"
class Road
{
public:
	Road();
	~Road();
private:
	Point2D p1, p2;
public:
	Road(Point2D p1, Point2D p2);
	Point2D GetCurrent();
	Point2D GetPrev();
	bool operator==(Road& other);
	bool operator<(Road& other);
};
