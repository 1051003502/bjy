#ifndef __PARSER_UTILS_CORNER_H__
#define __PARSER_UTILS_CORNER_H__

#include "point.h"
#include "line.h"
#include "box.h"


class Corner : public Point {
public:
	typedef enum {
		// v&h
		LEFTUP,
		UPDOWN,
		UPRIGHT,
		//
		RIGHTLEFT,
		RIGHTDOWN,
		DOWNTUP,
		//
		DOWNLEFT,
		LEFTRIGHT,
		CROSS,
		//v&s | h&s
		SLANTV,
		SLANTH,
		//s&s
		SLANTS,
		UNKNOWN,
		NUMBER
	}CornerType;

	 
	Corner() :Point(), type(UNKNOWN) {}
	Corner(double x, double y) :Point(x, y), type(UNKNOWN) {}
	Corner(double x, double y, CornerType type, int l1, int l2) :Point(x, y), type(UNKNOWN), l1(l1), l2(l2) {}
	CornerType type;
	int l1;
	int l2;
};

bool crossLineS(const Line& s, const Line& l, Corner& corner, double xp = Precision, double yp = Precision);

bool crossLineSS(const Line& s1, const Line& s2, Corner& corner, double xp = Precision, double yp = Precision);

bool crossLineVH(const Line& v, const Line& h, Corner& corner, double xp = Precision, double yp = Precision);
bool crossLineSSImpl(const double& l1sx, const double& l1sy,
	const double& l1ex, const double& l1ey,
	const double& l2sx, const double& l2sy,
	const double& l2ex, const double& l2ey, Corner& corner);


bool crossLinesS(const std::vector<Line>& lines, int idxs, int idx,
	std::map<int, std::vector<Corner>>& corners, double xp = Precision, double yp = Precision);
bool crossLinesVH(const std::vector<Line>& lines, int idxv, int idxh,
	std::map<int, std::vector<Corner>>& corners, double xp = Precision, double yp = Precision);
bool TempCheckIntersect(const std::vector<Line>&lines, int idx1, int idx2);
//bool ConnectPoint(const Point& p1, const Point&p2);
bool cornerFindPoint(std::vector<Corner>temp_corner, const Point&temp_p);

bool crossLine(const Line&line1, const Line&line2);
bool crossLineIndex(const int &index1, const int &index2,  const std::vector<Line>lines);
int getCrossLineIndexFromCorner(int index, const Corner& corner);
#endif
