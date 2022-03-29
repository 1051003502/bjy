#include "point.h"


int findLowerBound(double y, const std::vector<Point>& points, double precision /*= Precision*/) {
	y = y - precision;
	int low = 0, high = points.size() - 1;
	while (low < high)
	{
		int mid = (low + high) / 2;
		double v = points[mid].y;

		if (v >= y)
		{
			high = mid;
		}
		else
		{
			low = mid + 1;
		}
	}
	return low;
}

int findUpperBound(double y, const std::vector<Point>& points, double precision /*= Precision*/) {
	y = y + precision;
	int low = 0, high = points.size() - 1;
	while (low < high)
	{
		int mid = (low + high + 1) / 2;
		double v = points[mid].y;
		if (v <= y)
		{
			low = mid;
		}
		else
		{
			high = mid - 1;
		}
	}
	return low;
}
bool ConnectPoint(const Point& p1, const Point& p2)
{

	if (p1.x + Precision > p2.x && p1.x - Precision <p2.x &&
		p1.y + Precision >p2.y && p1.y - Precision < p2.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

double TowPointsLength(const Point& p1, const Point& p2)
{
	double length = 0.0;
	length = sqrt(pow(abs(p1.x - p2.x), 2) + pow(abs(p1.y - p2.y), 2));
	return length;
}
