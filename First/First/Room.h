#pragma once
#include "Point2D.h"
class Room
{
public:
	Room();
	~Room();
private:
	Point2D center, exit[10];
	int width, height, exits;
	
public:
	Room(const Point2D& center_point, int w, int h);
	Point2D GetCenter() const;
	int GetWidth();
	int GetHeight();
	int GetNumOfExits();
	Point2D* GetExitPoints();
	void InsertExitPoint(Point2D newPt);
	bool IsOverlap(const Room& other);
};

