#include "Road.h"

Road::Road()
{
}

Road::~Road()
{
}

Road::Road(Point2D p_1, Point2D p_2)
{
	p1 = p_1;
	p2 = p_2;
}


Point2D Road::GetCurrent()
{
	return p1;
}

Point2D Road::GetPrev()
{
	return p2;
}

bool Road::operator==(Road & other)
{
	return (p1==other.p1 && p2==other.p2) || (p1==other.p2 && p2==other.p1);
}

bool Road::operator<(Road & other)
{
	return (p1.GetX()*p1.GetY() + p2.GetX()*p2.GetY()) < (other.p1.GetX()*other.p1.GetY() + other.p2.GetX()*other.p2.GetY()) ;
}
