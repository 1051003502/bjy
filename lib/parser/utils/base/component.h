#ifndef __PARSER_UTILS_COMPONENT_H__
#define __PARSER_UTILS_COMPONENT_H__
#include "line.h"
#include "box.h"
#include "corner.h"
#include "data.h"
class Component {

public:
	Component(const std::vector<Line>& vl, const std::vector<Line>& hl, const std::vector<Line>&sl, bool combine = true);
	virtual ~Component() {}

	bool combineLines();
	bool cover(const Corner& c) {
		return box.cover(c);
	}

	bool cover(const Component& t) {
		return box.cover(t.box);
	}

	bool moveLeft() {
		double shift = box.width() * -1.1;
		box.move(shift, 0);
		for (int i = 0; i < vlines.size(); ++i) {
			vlines[i].s.x += shift;
			vlines[i].e.x += shift;
		}
		for (int i = 0; i < hlines.size(); ++i) {
			hlines[i].s.x += shift;
			hlines[i].e.x += shift;
		}

		return true;
	}

	bool expandBorder(std::vector<Line>& vlines, std::vector<Line>& hlines) const;
	bool removeShortLines(std::vector<Line>& vlines, std::vector<Line>& hlines) const;

	bool hasBorder() const {
		//left
		if (!vlines.empty()&&vlines[0].s.x - precisionx > box.left)
			return false;

		//right
		if (!vlines.empty()&&vlines[vlines.size() - 1].s.x + precisionx < box.right)
			return false;

		//bottom line
		if (!hlines.empty()&&hlines[0].s.y - precisiony > box.bottom)
			return false;

		//top line
		if (!hlines.empty()&&hlines[hlines.size() - 1].s.y + precisiony < box.top)
			return false;
		return true;
	}

	//std::map<int, std::map<int, std::shared_ptr<DRW_Text>>> extract(const kdt::KDTree<Point>& kdt, const std::vector<Point>& texts);

public:
	Box box;
	std::vector<Line> vlines;
	std::vector<Line> hlines;
	std::vector<Line> slines;
	double precisionx;
	double precisiony;
};
std::vector<std::shared_ptr<Component>> returnComponents(LineData& lineData, const std::map<int, std::vector<Corner>>& corners);
std::shared_ptr<Component> buildComponentFrom(LineData& lineData, const std::map<int, std::vector<Corner>>& corners,
	const Corner &p, std::vector<int> &map);
std::shared_ptr<Component> buildComponentFrom(LineData& lineData, const std::map<int, std::vector<Corner>>& corners,
	int line, std::vector<int> &map);
bool findCrossedLines(int vline, const std::vector<Line> &lines,
	const std::map<int, std::vector<Corner>> &_corners, std::vector<int> &map, std::vector<int> &vlines, std::vector<int> &hlines,
	std::vector<int> &slines);
std::shared_ptr<Component> generateComponent(LineData& m_lineData, const std::vector<int> &vlines, const std::vector<int> &hlines,
	const std::vector<int> &slines);

#endif