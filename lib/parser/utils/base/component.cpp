#include "component.h"


Component::Component(const std::vector<Line>& vl, const std::vector<Line>& hl,
	const std::vector<Line>& sl, bool combine/* = true*/) : vlines(vl), hlines(hl), slines(sl) {
	std::sort(vlines.begin(), vlines.end(), comparev);
	std::sort(hlines.begin(), hlines.end(), compareh);
	for (auto v : vlines) {
		box.expand(v.s);
		box.expand(v.e);
	}
	for (auto h : hlines) {
		box.expand(h.s);
		box.expand(h.e);
	}

	precisionx = Precision;
	precisiony = Precision;
	//int cntx = 0;
	//int cnty = 0;
	std::vector<double> xx;
	std::vector<double> yy;
	/*for (int i = 1; i < vlines.size(); ++i) {
		double tmp = vlines[i].s.x - vlines[i - 1].s.x;
		xx.push_back(tmp);
	}
	for (int i = 1; i < hlines.size(); ++i) {
		double tmp = hlines[i].s.y - hlines[i - 1].s.y;
		yy.push_back(tmp);
	}
	if (xx.size()) {
		sort(xx.begin(), xx.end());
		precisionx = xx[xx.size() / 2];
		precisionx *= 0.1;
	}
	if (yy.size()) {
		sort(yy.begin(), yy.end());
		precisiony = yy[yy.size() / 2];
		precisiony *= 0.1;
	}*/

	if (combine) {
		combineLines();
	}

}

bool Component::combineLines() {
	std::vector<Line> vl;
	std::vector<Line> hl;

	//combine vlines
	Line pre;
	if (!vlines.empty())
	{
		pre = vlines[0];
		for (int i = 1; i < vlines.size(); ++i) {
			if (vlines[i].s.x - vlines[i - 1].s.x > precisionx) {
				vl.push_back(pre);
				pre = vlines[i];
				continue;
			}

			if (vlines[i].s.y - vlines[i - 1].e.y > precisiony) {
				vl.push_back(pre);
				pre = vlines[i];
				continue;
			}
			if (pre.e.y < vlines[i].e.y) {
				pre.e = vlines[i].e;
			}
		}
		vl.push_back(pre);
	}
	

	//combine hlines
	if (!hlines.empty())
	{
		pre = hlines[0];
		for (int i = 1; i < hlines.size(); ++i) {
			if (hlines[i].s.y - hlines[i - 1].s.y > precisiony) {
				hl.push_back(pre);
				pre = hlines[i];
				continue;
			}
			if (hlines[i].s.x - hlines[i - 1].e.x > precisionx) {
				hl.push_back(pre);
				pre = hlines[i];
				continue;
			}
			if (pre.e.x < hlines[i].e.x) {
				pre.e = hlines[i].e;
			}
		}
		hl.push_back(pre);
	}
	vlines = vl;
	hlines = hl;
	return true;
}

//
//std::map<int, std::map<int, std::shared_ptr<DRW_Text>>> Component::extract(
//	const kdt::KDTree<Point>& kdt,
//	const std::vector<Point>& texts) {
//	std::map<int, std::map<int, std::shared_ptr<DRW_Text>>> component;
//	for (int i = 1; i < vlines.size(); ++i) {
//		Line v1 = vlines[i - 1];
//		Line v2 = vlines[i];
//		if (v1.s.x + Precision >= v2.s.x)
//			continue;
//		for (int j = 1; j < hlines.size(); ++j) {
//			Line h1 = hlines[j - 1];
//			Line h2 = hlines[j];
//			if (h1.s.y + Precision >= h2.s.y)
//				continue;
//			Corner p((v2.s.x + v1.s.x) / 2, (h2.s.y + h1.s.y) / 2);
//			auto cands = kdt.knnSearch(p, 6);
//
//			for (auto c : cands) {
//				Point t = texts[c];
//				DRW_Text* tt = static_cast<DRW_Text*>(t.entity.get());
//				if (t.x > v1.s.x && t.x < v2.s.x &&
//					t.y > h1.s.y && t.y < h2.s.y) {
//					auto it = component.find(i - 1);
//					if (it == component.end()) {
//						component[i - 1] = std::map<int, std::shared_ptr<DRW_Text>>();
//					}
//					component[i - 1].insert(std::make_pair(j - 1, std::static_pointer_cast<DRW_Text>(t.entity)));
//				}
//			}
//		}
//	}
//	return component;
//}


bool Component::removeShortLines(std::vector<Line>& vls, std::vector<Line>& hls) const {

	//TODO
	return true;
	std::vector<Line> vl;
	std::vector<Line> hl;

	for (auto l : vls) {
		if (l.length() > 0.1 * box.height()) {
			vl.emplace_back(l);
		}
	}
	for (auto l : hls) {
		if (l.length() > 0.1 * box.width()) {
			hl.emplace_back(l);
		}
	}
	vls = vl;
	hls = hl;
	return true;
}

bool Component::expandBorder(std::vector<Line>& vls, std::vector<Line>& hls) const {
	//left
	if (vlines[0].s.x - precisionx > box.left) {
		vls.push_back(Line(Point(box.left, box.bottom), Point(box.left, box.top)));
	}
	vls.insert(vls.end(), vlines.begin(), vlines.end());

	//right
	if (vlines[vlines.size() - 1].s.x + precisionx < box.right) {
		vls.push_back(Line(Point(box.right, box.bottom), Point(box.right, box.top)));
	}

	//bottom line
	if (hlines[0].s.y - precisiony > box.bottom) {
		hls.push_back(Line(Point(box.left, box.bottom), Point(box.right, box.bottom)));
	}
	hls.insert(hls.end(), hlines.begin(), hlines.end());

	//top line
	if (hlines[hlines.size() - 1].s.y + precisiony < box.top) {
		hls.push_back(Line(Point(box.left, box.top), Point(box.right, box.top)));
	}
	return true;
}

std::vector<std::shared_ptr<Component>> returnComponents(LineData & lineData, const std::map<int, std::vector<Corner>>& corners)
{
	std::vector<std::shared_ptr<Component>> temp_component;
	std::vector<int> map(lineData.lines().size(), 0);

	for (auto it : lineData.vLinesIndices())
	{
		//bool mark = false;
		if (!map[it])
		{

			auto component = buildComponentFrom(lineData, corners, it, map);
			if (component.get())
			{
				temp_component.push_back(component);
			}
		}
	}

	for (auto it : lineData.hLinesIndices())
	{
		if (!map[it])
		{
			auto component = buildComponentFrom(lineData, corners, it, map);
			if (component.get())
			{
				temp_component.push_back(component);
			}
		}
	}
	return temp_component;

}

std::shared_ptr<Component> buildComponentFrom(LineData& lineData, const std::map<int, std::vector<Corner>>& corners,
	const Corner & p, std::vector<int>& map)
{

	return std::shared_ptr<Component>();
}

std::shared_ptr<Component> buildComponentFrom(LineData& lineData, const std::map<int, std::vector<Corner>>& corners,
	int line, std::vector<int>& map)
{
	std::vector<int> vlines;
	std::vector<int> hlines;
	std::vector<int> slines;

	findCrossedLines(line, lineData.lines(), corners, map, vlines, hlines, slines);

	return generateComponent(lineData, vlines, hlines, slines);
}

bool findCrossedLines(int vline, const std::vector<Line>& lines, const std::map<int, std::vector<Corner>>& _corners,
	std::vector<int>& map, std::vector<int>& vlines, std::vector<int>& hlines, std::vector<int>& slines)
{
	map[vline] = 1;
	std::queue<int> q;
	q.push(vline);

	while (!q.empty())
	{
		int idx = q.front();
		q.pop();
		auto line = lines[idx];
		if (line.vertical())
		{
			vlines.push_back(idx);
		}
		else if (line.horizontal())
		{
			hlines.push_back(idx);
		}
		else
		{
			slines.push_back(idx);
		}
		auto it = _corners.find(idx);
		if (it != _corners.end())
		{
			for (auto cs : it->second)
			{
				if (!map[cs.l1])
				{
					map[cs.l1] = 1;
					q.push(cs.l1);
				}
				if (!map[cs.l2])
				{
					map[cs.l2] = 1;
					q.push(cs.l2);
				}
			}
		}
	}
	return true;
}

std::shared_ptr<Component> generateComponent(LineData& m_lineData, const std::vector<int>& vlines,
	const std::vector<int>& hlines, const std::vector<int>& slines)
{
	if (vlines.size() && hlines.size())
	{

		std::vector<Line> vls;
		std::vector<Line> hls;
		std::vector<Line> sls;
		for (auto v : vlines)
		{
			vls.push_back(m_lineData.getLine(v));
		}
		for (auto h : hlines)
		{
			hls.push_back(m_lineData.getLine(h));
		}
		for (auto s : slines)
		{
			sls.push_back(m_lineData.getLine(s));
		}
		return std::shared_ptr<Component>(new Component(vls, hls, sls));
	}

	return std::shared_ptr<Component>();

}
