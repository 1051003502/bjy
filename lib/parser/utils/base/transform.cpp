#include "transform.h"


bool Transform::setTransform(const Point& point)
{
	this->x = point.x;
	this->y = point.y;
	return true;
}

Point Transform::referencePoint(const Point& point)
{
	Point tempPoint;
	tempPoint.x = ROUND(point.x - this->x);
	tempPoint.y = ROUND(point.y - this->y);
	return tempPoint;
}

Point Transform::trans(const Point& point) const {
	//return point;
	Point p;
	p.x = x + point.x * xscale;
	p.y = y + point.y * yscale;
	return p;
}

Point Transform::transR(const Point& point) const {
	//return point;
	Point p;
	p.x = x + point.x * xscale * cos(rotation) - point.y * yscale * sin(rotation);
	p.y = y + point.x * xscale * sin(rotation) + point.y * yscale * cos(rotation);
	return p;
}

double Transform::trans(double r) const{
	return r * yscale;
}