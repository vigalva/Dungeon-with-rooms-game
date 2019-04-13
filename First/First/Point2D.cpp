#include "Point2D.h"
#include <math.h>

Point2D::Point2D()
	: x(0)
{
}


Point2D::~Point2D()
{
}


Point2D::Point2D(int x, int y)
{
	this->x = x;
	this->y = y;
}


int Point2D::GetX() const
{
	return x;
}


int Point2D::GetY() const
{
	return y;
}

double Point2D::Distance(const Point2D & other) const
{
	return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
}

bool Point2D::operator==(const Point2D& other)
{
	return x==other.x && y== other.y;
}
