#include "NewNode.h"
#include <math.h>



NewNode::NewNode()
	: point(0, 0), target(0, 0)
{
	g = 0;

}


NewNode::~NewNode()
{
}


double NewNode::GetF() const
{
	return 0.3*GetG() + 0.7*GetH();
}

double NewNode::GetG() const
{
	return g;
}

double NewNode::GetH() const
{
	return 0.8*sqrt(pow(point.GetX() - target.GetX(), 2) +
		pow(point.GetY() - target.GetY(), 2) + 0.2* rival.Distance(point));
}


NewNode::NewNode(const Point2D& p, const Point2D& t, double g, const Point2D& rival)
{
	point = p;
	target = t;
	this->g = g;
	this->rival = rival;
}


Point2D NewNode::GetPoint()
{
	return point;
}

bool NewNode::operator==(const NewNode & other)
{
	return point == other.point && target == other.target && g == other.g;
}
