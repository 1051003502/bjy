#ifndef __PARSER_UTILS_AXIS_H__
#define __PARSER_UTILS_AXIS_H__

//#define PARSER_TEST_AXIS
#include <set>
#include "box.h"
//#include "data.h"
#include"common.h"
#include "circle.h"
#include "line.h"

class Axis
{
public:
	
	typedef std::pair<Circle, std::string> AxisSymbol;
	typedef std::pair<Axis::AxisSymbol, Line> AxisLine;            //轴线
	typedef std::pair<Axis::AxisLine, double> AxisLineCoordinate;  //轴线坐标

public:
	Axis() {}
	Axis(const std::vector<AxisLine>& vs, const std::vector<AxisLine>& hs) : vlines(vs), hlines(hs) {
		for (auto l : vs)
		{
			box.expand(l.first.first);
			box.expand(l.second.s);
			box.expand(l.second.e);
		}
		for (auto l : hs)
		{
			box.expand(l.first.first);
			box.expand(l.second.s);
			box.expand(l.second.e);
		}
		for (auto l : vs)
		{
			layers.insert(l.first.first.getLayer());
			layers.insert(l.second.getLayer());
		}
		for (auto l : hs)
		{
			layers.insert(l.first.first.getLayer());
			layers.insert(l.second.getLayer());
		}
	}
	Axis::AxisLineCoordinate findAxisLine(std::string name, const char& type);

public:
	std::vector<AxisLine> vlines;
	std::vector<AxisLine> hlines;
	std::vector<AxisLineCoordinate> vlines_coo;//从左到右
	std::vector<AxisLineCoordinate> hlines_coo;//从下到上

	std::vector<AxisLineCoordinate> referenceAxisX;
	std::vector<AxisLineCoordinate> referenceAxisY;

	std::set<std::string> layers;
	Box box;
	std::string name;
};

#endif