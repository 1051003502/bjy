#ifndef __PARSER_UTILS_BOX_H__
#define __PARSER_UTILS_BOX_H__
#include "point.h"
#include "line.h"

class Box {
public:
	Box() :initv(false), inith(false) {}
	Box(const Point& p) :initv(true), inith(true) {
		left = right = p.x;
		bottom = top = p.y;
	}

	//用两个点更新box
	bool iniBox(const Point& p1, const Point& p2) {
		initv = inith = true;
		left = right = p1.x;
		bottom = top = p1.y;
		this->expandH(p2.x);
		this->expandV(p2.y);
		return true;
	}

	bool reset() {
		initv = inith = false;
		return true;
	}

	bool expandH(double x) {
		if (!inith) {
			left = right = x;
			inith = true;
		}
		else {
			if (left > x)
				left = x;
			if (right < x)
				right = x;
		}
		return true;
	}

	bool expandV(double y) {
		if (!initv) {
			top = bottom = y;
			initv = true;
		}
		else {
			if (top < y)
				top = y;
			if (bottom > y)
				bottom = y;
		}
		return true;
	}

	bool expand(const Point& p) {

		expandH(p.x);
		expandV(p.y);
		return true;
	}
	bool expand(const Line& line)
	{
		expand(line.s);
		expand(line.e);
		return true;
	}
	bool expand(const std::vector<Line>& lineVec)
	{
		for (const Line& line : lineVec)
		{
			expand(line);
		}
		return true;
	}
	bool expand(const Box& b) {
		if (b.inith) {
			expandH(b.left);
			expandH(b.right);
		}
		if (b.initv) {
			expandV(b.top);
			expandV(b.bottom);
		}
		return true;
	}
	Point getLeftTopPoint()const
	{
		return Point(left, top);
	}
	Point getLeftBottomPoint()const
	{
		return Point(left, bottom);
	}
	Point getRightTopPoint()const
	{
		return  Point(right, top);
	}
	Point getRightBottomPoint()const
	{
		return Point(right, bottom);
	}
	Point getCenterPoint()const
	{
		return Point(getCenterX(), getCenterY());
	}
	double getCenterX()const
	{
		return (left + right) / 2;
	}
	double getCenterY()const
	{
		return (top + bottom) / 2;
	}
	Line getTopLine()const
	{
		Line lineTop(Point(left, top), Point(right, top));
		return lineTop;
	}
	Line getBottomLine()const
	{
		Line lineBottom(Point(left, bottom), Point(right, bottom));
		return lineBottom;
	}
	Line getLeftLine()const
	{
		Line lineLeft(Point(left, bottom), Point(left, top));
		return lineLeft;
	}
	Line getRightLine()const
	{
		Line lineRight(Point(right, bottom), Point(right, top));
		return lineRight;
	}
	std::vector<Line> getBorder()const
	{
		std::vector<Line> border;
		border.push_back(this->getTopLine());
		border.push_back(this->getBottomLine());
		border.push_back(this->getLeftLine());
		border.push_back(this->getRightLine());
		return border;
	}
	
	double left;
	double right;
	double top;
	double bottom;

	bool initv;
	bool inith;
	//区块横线，纵线的区域范围索引
	int hf;
	int ht;
	int vf;
	int vt;
	//
	bool move(double x, double y) {
		if (!initv || !inith)
			return false;
		left += x;
		right += x;
		top += y;
		bottom += y;
		return true;
	}

	bool cover(const Point& p)const {
		if (!initv || !inith)
			return false;
		return p.x + Precision >= left && p.x - Precision <= right && p.y + Precision >= bottom && p.y - Precision <= top;
	}

	bool cover(const Line& line)const {
		if (cover(line.s) && cover(line.e)) {
			return true;
		}
		else return false;
	}

	bool cover(const Box& b)const {
		if (!initv || !inith || !b.initv || !b.inith)
			return false;
		return left - Precision <= b.left && top + Precision >= b.top && right + Precision >= b.right && bottom - Precision <= b.bottom;
	}

	bool cross(const Box& b) const;

	bool cross(const Line&line)const;

	double area() const {
		if (!initv || !inith)
			return 0.0;
		return (right - left) * (top - bottom);
	}

	double width() const {
		if (!initv || !inith)
			return 0.0;
		return (right - left);
	}

	double height() const {
		if (!initv || !inith)
			return 0.0;
		return (top - bottom);
	}
	friend bool operator == (const Box& box1, const Box& box2)
	{
		if (box1.left != box2.left)
			return false;
		if (box1.right != box2.right)
			return false;
		if (box1.bottom != box2.bottom)
			return false;
		if (box1.top != box2.top)
			return false;
		return true;
	}

	
};


#endif