#include "Room.h"
#include <math.h>



Room::Room()
{
}


Room::~Room()
{
}


Room::Room(const Point2D& center_point, int w, int h)
{
	center = center_point;
	width = w;
	height = h;
	exits = 0;
}


Point2D Room::GetCenter() const
{
	return center;
}


int Room::GetWidth()
{
	return width;
}


int Room::GetHeight()
{
	return height;
}

int Room::GetNumOfExits()
{
	return exits;
}

Point2D * Room::GetExitPoints()
{
	return exit;
}


void Room::InsertExitPoint(Point2D newPt)
{
	exit[exits] = newPt;
	exits++;
}

bool Room::IsOverlap(const Room& other)
{
	return abs(center.GetX()-other.GetCenter().GetX())
		< (width+other.width)/2+5 && abs(center.GetY() - 
			other.GetCenter().GetY()) < (height+other.height)/2+5  ;
}
