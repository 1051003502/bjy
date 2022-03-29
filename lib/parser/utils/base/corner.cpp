#include "corner.h"
#include "line.h"

//四舍五入宏定义
bool crossLineS(const Line& s, const Line& l, Corner& corner, double xp /* = Precision*/, double yp /* = Precision*/) {

	double dx = s.e.x - s.s.x;
	double dy = s.e.y - s.s.y;

	if (l.vertical()) {
		double cx = l.s.x - s.s.x;
		double cy = (dy / dx) * cx;
		cx += s.s.x;
		cy += s.s.y;
		if (cy > (l.s.y - Precision) && cy < (l.e.y + Precision) &&
			cy >(std::min(s.s.y, s.e.y) - Precision) && cy < (std::max(s.s.y, s.e.y) + Precision)) {
			corner.type = Corner::SLANTV;
			corner.x = cx;
			corner.y = cy;
			return true;
		}
	} else if (l.horizontal()) {
		double cy = l.s.y - s.s.y;
		double cx = (dx / dy) * cy;
		cx += s.s.x;
		cy += s.s.y;

		if (cx > (l.s.x - Precision) && cx < (l.e.x + Precision) &&
			cx >(std::min(s.s.x, s.e.x) - Precision) && cx < (std::max(s.s.x, s.e.x) + Precision)) {
			corner.type = Corner::SLANTH;
			corner.x = cx;
			corner.y = cy;
			return true;
		}
	}
	return false;
}

bool crossLineSSImpl(const double& l1sx, const double& l1sy,
					const double& l1ex, const double& l1ey,
					const double& l2sx, const double& l2sy,
					const double& l2ex, const double& l2ey, Corner& corner) {

	double s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
	s10_x = l1ex - l1sx;
	s10_y = l1ey - l1sy;
	s32_x = l2ex - l2sx;
	s32_y = l2ey - l2sy;

	denom = s10_x * s32_y - s32_x * s10_y;
	if (denom == 0)  //平行或共线
		return false;    // Collinear
	bool denomPositive = denom > 0;

	s02_x = l1sx - l2sx;
	s02_y = l1sy - l2sy;
	s_numer = s10_x * s02_y - s10_y * s02_x;
	if (((s_numer < 0.0)) == denomPositive)  //参数是大于等于0且小于等于1的，分子分母必须同号且分子小于等于分母
		return false;                        // No collision

	t_numer = s32_x * s02_y - s32_y * s02_x;
	if ((t_numer < 0) == denomPositive)
		return false;  // No collision

	if (fabs(s_numer) > fabs(denom) || fabs(t_numer) > fabs(denom))
		return false;  // No collision
	// Collision detected
	t = t_numer / denom;
	/*if (i_x != NULL)*/
	corner.x = l1sx + (t * s10_x);
	//if (i_y != NULL)
	corner.y = l1sy + (t * s10_y);
	corner.type = Corner::UNKNOWN;
	return true;
}

bool crossLineSS(const Line& s1, const Line& s2, Corner& corner, double xp /* = Precision*/, double yp /* = Precision*/)
{

	if (s1.vertical() || s1.horizontal() || s2.horizontal() || s2.vertical())
		return false;

	const auto l1sx = s1.s.x - 1;
	const auto l1sy = knowXToY(s1, l1sx);
	const auto l1ex = s1.e.x + 1;
	const auto l1ey = knowXToY(s1, l1ex);
	const auto l2sx = s2.s.x - 1;
	const auto l2sy = knowXToY(s2, l2sx);
	const auto l2ex = s2.e.x + 1;
	const auto l2ey = knowXToY(s2, l2ex);
	return crossLineSSImpl(l1sx, l1sy, l1ex, l1ey, l2sx, l2sy, l2ex, l2ey, corner);

}

bool crossLineVH(const Line& v, const Line& h, Corner& corner, double xp /* = Precision*/, double yp /*= Precision*/) {
	if (v.s.x + xp < h.s.x)
		return false;
	if (v.s.x - xp > h.e.x)
		return false;
	if (v.s.y - yp > h.s.y)
		return false;
	if (v.e.y + yp < h.s.y)
		return false;
	corner.x = v.s.x;
	corner.y = h.s.y;

	if (abs(v.s.x - h.s.x) < xp) {
		//left
		if (abs(v.s.y - h.s.y) < yp) {
			corner.type = Corner::DOWNLEFT;
		} else if (abs(v.e.y - h.s.y) < yp) {
			corner.type = Corner::LEFTUP;
		} else {
			corner.type = Corner::LEFTRIGHT;
		}
	} else if (abs(v.s.x - h.e.x) < xp) {
		//right
		if (abs(v.s.y - h.s.y) < yp) {
			corner.type = Corner::RIGHTDOWN;
		} else if (abs(v.e.y - h.s.y) < yp) {
			corner.type = Corner::UPRIGHT;
		} else {
			corner.type = Corner::RIGHTLEFT;
		}
	} else {
		//mid
		if (abs(v.s.y - h.s.y) < yp) {
			corner.type = Corner::DOWNTUP;
		} else if (abs(v.e.y - h.s.y) < yp) {
			corner.type = Corner::UPDOWN;
		} else {
			corner.type = Corner::CROSS;
		}
	}
	return true;
}

bool crossLinesS(const std::vector<Line>& lines, int idxs, int idx,
	std::map<int, std::vector<Corner>>& corners, double xp /* = Precision*/, double yp /*= Precision*/) {
	const Line& s = lines[idxs];
	const Line& l = lines[idx];
	Corner corner;

	if (!crossLineS(s, l, corner, xp, yp))
		return false;
	corner.l1 = idxs;
	corner.l2 = idx;

	if (corners.find(idxs) == corners.end()) {
		corners[idxs] = std::vector<Corner>();
	}
	if (corners.find(idx) == corners.end()) {
		corners[idx] = std::vector<Corner>();
	}

	corners[idxs].push_back(corner);
	corners[idx].push_back(corner);

	return true;
}

bool crossLinesVH(const std::vector<Line>& lines, int idxv, int idxh,
	std::map<int, std::vector<Corner>>& corners, double xp /* = Precision*/, double yp /* = Precision*/) {
	const Line& v = lines[idxv];
	const Line& h = lines[idxh];

	Corner corner;

	if (!crossLineVH(v, h, corner, xp, yp))
		return false;

	corner.l1 = idxv;
	corner.l2 = idxh;

	if (corners.find(idxv) == corners.end()) {
		corners[idxv] = std::vector<Corner>();
	}
	if (corners.find(idxh) == corners.end()) {
		corners[idxh] = std::vector<Corner>();
	}
	corners[idxv].push_back(corner);
	corners[idxh].push_back(corner);
	return true;
}

bool TempCheckIntersect(const std::vector<Line>& lines, int idx1, int idx2) {
	Line l1 = lines[idx1];
	Line l2 = lines[idx2];
	//快速排斥排除
	/*if (ROUND(l1.s.x>l1.e.x?l1.s.x:l1.e.x)<ROUND(l2.s.x<l2.e.x ? l2.s.x : l2.e.x)
		|| ROUND(l1.s.y>l1.e.y ? l1.s.y : l1.e.y)<ROUND(l2.s.y<l2.e.y ? l2.s.y : l2.e.y)
		|| ROUND(l2.s.y>l2.e.y ? l2.s.y : l2.e.y)<ROUND(l1.s.y<l1.e.y ? l1.s.y : l1.e.y)
		|| ROUND(l2.s.x>l2.e.x ? l2.s.x : l2.e.x)<ROUND(l1.s.x<l1.e.x ? l1.s.x : l1.e.x))
	{
		return false;
	}*/
	if (((l1.s.x > l1.e.x ? l1.s.x : l1.e.x) + Precision) < (l2.s.x < l2.e.x ? l2.s.x : l2.e.x) || ((l1.s.y > l1.e.y ? l1.s.y : l1.e.y) + Precision) < (l2.s.y < l2.e.y ? l2.s.y : l2.e.y) || ((l2.s.y > l2.e.y ? l2.s.y : l2.e.y) + Precision) < (l1.s.y < l1.e.y ? l1.s.y : l1.e.y) || ((l2.s.x > l2.e.x ? l2.s.x : l2.e.x) + Precision) < (l1.s.x < l1.e.x ? l1.s.x : l1.e.x)) {
		return false;
	}
	//跨立排除
	double u, v, w, z; //分别记录两个向量
	u = (l2.e.x - l1.s.x) * (l1.e.y - l1.s.y) - (l1.e.x - l1.s.x) * (l2.e.y - l1.s.y);
	v = (l2.s.x - l1.s.x) * (l1.e.y - l1.s.y) - (l1.e.x - l1.s.x) * (l2.s.y - l1.s.y);
	w = (l1.s.x - l2.e.x) * (l2.s.y - l2.e.y) - (l2.s.x - l2.e.x) * (l1.s.y - l2.e.y);
	z = (l1.e.x - l2.e.x) * (l2.s.y - l2.e.y) - (l2.s.x - l2.e.x) * (l1.e.y - l2.e.y);
	return (u * v <= 0.00000001 && w * z <= 0.00000001);
	//return true;
}

//bool ConnectPoint(const Point & p1, const Point & p2)
//{
//	auto p1_x = int(p1.x * 10000) / 10000.0;
//	auto p1_y = int(p1.y * 10000) / 10000.0;
//	auto p2_x = int(p2.x * 10000) / 10000.0;
//	auto p2_y = int(p2.y * 10000) / 10000.0;
//	if (p1_x == p2_x&&p1_y == p2_y)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//
//}

bool cornerFindPoint(std::vector<Corner> temp_corner, const Point& temp_p) {
	bool mark = false;
	for (auto it : temp_corner) {
		if (ConnectPoint(it, temp_p)) {
			mark = true;
			break;
		}
	}
	return mark;
}

bool crossLine(const Line& line1, const Line& line2) {

	return false;
}

bool crossLineIndex(const int& index1, const int& index2, const std::vector<Line>lines) {
	return false;
}
int getCrossLineIndexFromCorner(int index, const Corner& corner)
{
	return index == corner.l1 ? corner.l2 : corner.l1;
}
