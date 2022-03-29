#include "line.h"

const std::string Line::defaultLayer = "";

int findLowerBound(double y, const std::vector<int>& list, const std::vector<Line>& lines, double precision /*= Precision*/, bool h /*= true*/) {
	y = y - precision;
	int low = 0, high = list.size() - 1;
	while (low < high) {
		int mid = (low + high) / 2;
		double v = lines[list[mid]].s.y;
		if (!h) {
			v = lines[list[mid]].s.x;
		}
		if (v >= y) {
			high = mid;
		}
		else {
			low = mid + 1;
		}
	}
	return low;
}

int findUpperBound(double y, const std::vector<int>& list, const std::vector<Line>& lines, double precision/* = Precision*/, bool h/* = true*/) {
	y = y + precision;
	int low = 0, high = list.size() - 1;
	while (low < high) {
		int mid = (low + high + 1) / 2;
		double v = lines[list[mid]].s.y;
		if (!h) {
			v = lines[list[mid]].s.x;
		}
		if (v <= y) {
			low = mid;
		}
		else {
			high = mid - 1;
		}
	}
	return low;
}



double knowXToY(const Line & line, const double & x)
{
	double x1 = line.s.x;
	double x2 = line.e.x;
	double y1 = line.s.y;
	double y2 = line.e.y;;
	double m1 = y2 - y1;
	double m2 = x2 - x1;
	if (abs(m1) < 0.00001)
	{
		return line.s.y;
	}
	double k = m1 / m2;
	double b = (m2*y1 - m1*x1) / m2;
	double y = k*x + b;
	return y;
}

double knowYToX(const Line & line, const double & y)
{
	double x1 = line.s.x;
	double x2 = line.e.x;
	double y1 = line.s.y;
	double y2 = line.e.y;;
	double m1 = y2 - y1;
	double m2 = x2 - x1;
	if (abs(m2) < 0.0001)
	{
		return line.s.x;
	}
	double k = m2 / m1;
	double b = (m1*x1 - m2*y1) / m1;
	double x = k*y + b;
	return x;
}

double GetTanOfLine(const Line& line)
{
	double x_dis = line.s.x - line.e.x;
	if (x_dis == 0.0) return 10e9;
	return (line.s.y - line.e.y) / x_dis;
}

bool IsParallelLines(const Line& line0, const Line& line1)
{
	if (line0.horizontal() && line1.horizontal())return true;
	if (line0.vertical() && line1.vertical())return true;
#define LIMIT (1.1547 / 3) //近似平行线的斜率之差的范围

	double angleTan0 = GetTanOfLine(line0);
	double angleTan1 = GetTanOfLine(line1);
	if (fabs(angleTan0 - angleTan1) < (double)LIMIT)
	{
		return true;
	}
	return false;
}

double slantingDistanceLine(const Line& line0, const Line& line1)
{
	double x = knowYToX(line0, line1.s.y);
	double y = knowXToY(line0, line1.s.x);
	Point p1 = Point(x, line1.s.y);
	Point p2 = Point(line1.s.x, y);
	double l1 = TowPointsLength(p1, line1.s);
	double l2 = TowPointsLength(p2, line1.s);
	double h = (l1 * l2) / sqrt(l1 * l1 + l2 * l2);
	return h;
}

bool isAbove(const Line& l1, const Line& l2)
{
	double yInLine1 = knowXToY(l1,l2.s.x);
	return yInLine1 > l2.s.y;
}

bool bAtExtendedLine(const Line& l1, const Line& l2)
{
	if (l1.vertical() && l2.vertical())
	{
		if (abs(l1.s.x - l2.s.x) < Precision)
			return true;
		return false;
	}
	else if (l1.horizontal() && l2.horizontal())
	{
		if (abs(l1.s.y - l2.s.y) < Precision)
			return true;
		return false;
	}
	else if (l1.perpendicularTo(l2))
		return false;

	double y1 = knowXToY(l1, l2.s.x);
	double y2 = knowXToY(l1, l2.e.x);
	if (abs(y1 - l2.s.y) < 1 && abs(y2 - l2.e.y) < Precision)
		return true;
	return false;
}

bool Line::bRepeatLine(const int & index1, const int & index2)
{
	return false;
}

bool Line::bRepeatLine(const Line & line1, const Line & line2)
{
	if (line1.vertical() && line2.vertical())
	{
		if (line1.s.x + Precision > line2.s.x&&
			line1.s.x - Precision < line2.s.x)
		{
			return true;
		}
	}
	else if (line1.horizontal() && line2.horizontal())
	{
		if (line1.s.y + Precision > line2.s.y&&
			line1.s.y - Precision < line2.s.y)
		{
			return true;
		}
	}
	else if(!(line1.horizontal() && line2.horizontal())
		&&!( line1.vertical() && line2.vertical()))
	{
		;
	}
	return false;
}

//bool ClosedZone::b_closedzone(const std::vector<int>& borderline, ClosedZone & temp_borderline, Data & data)
//{
//	return false;
//}
