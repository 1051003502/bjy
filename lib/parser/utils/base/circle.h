
#ifndef __PARSER_UTILS_CIRCLE_H__
#define __PARSER_UTILS_CIRCLE_H__

#include "point.h"

class Circle :public Point {

public:
	Circle() {}
	Circle(const Point& p) :Point(p) {}
	Circle(const Point& p, double r) :Point(p), r(r) {}
	Circle(double x, double y, double r) :Point(x, y), r(r) {}

	bool cover(const Point& p, double mul = 1.0) {
		auto dis = distance(p);
		return dis - Precision < r * mul;
	}
public:
	double r;
};
#endif