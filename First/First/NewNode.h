#pragma once
#include "Point2D.h"
class NewNode
{
public:
	NewNode();
	~NewNode();
private:
	Point2D point, target, rival;
	double g;
public:
	double GetF() const;
	double GetG() const;
	double GetH() const;
	NewNode(const Point2D& p, const Point2D& t, double g, const Point2D& rival);
	Point2D GetPoint();
	bool operator==(const NewNode& other);
}; 
