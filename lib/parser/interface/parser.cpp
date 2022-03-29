#include "parser.h"
#include "common.h"

using namespace std;

bool Parser::reset()
{
	_data.reset();
	_formats.clear();
	_solids.clear();
	_texts.clear();
	_components.clear();
	_borders.clear();
	_issues.clear();
	_covers.clear();

	_enties.clear();
	_tables.clear();
	_axises.clear();

	m_data->m_lineData.clearLineData();

	m_data->m_cornerData.clearCorners();

	m_data->m_textPointData.clearPoints();

	m_data->m_kdtTreeData.kdtendsClear();
	m_data->m_kdtTreeData.kdttextsClear();

	_circlepoints.clear();
	_box.reset();

	m_data->m_endpointData.clearEndpoints();
	_blocks.clear();

	_section.clear();
	_beams.clear();
	_beamSections.clear();
	_beamLongitudinals.clear();
	_spWallVec.clear();
	_spWallLcoationVec.clear();
	return true;
}

bool Parser::crossCorners()
{
	m_data->m_lineData.sortLinesByBindRef(
		LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, compareRefH);
	m_data->m_lineData.sortLinesByBindRef(
		LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, compareRefV);

	int nSlinesSize = m_data->m_lineData.getSLinesSize();
	double min[2]{};
	double max[2]{};

	for (auto it = m_data->m_lineData.sLinesIndices().cbegin();
		 it != m_data->m_lineData.sLinesIndices().cend(); it++)
	{
		const Line& lineTemp = m_data->m_lineData.getLine(*it);
		min[0] = lineTemp.s.x < lineTemp.e.x ? lineTemp.s.x : lineTemp.e.x;
		min[1] = lineTemp.s.y < lineTemp.e.y ? lineTemp.s.y : lineTemp.e.y;
		max[0] = lineTemp.s.x > lineTemp.e.x ? lineTemp.s.x : lineTemp.e.x;
		max[1] = lineTemp.s.y > lineTemp.e.y ? lineTemp.s.y : lineTemp.e.y;
		m_data->m_rtreeData.insertSLines(min, max, *it);

	}

	for (auto v = m_data->m_lineData.vLinesIndices().begin();
		 v != m_data->m_lineData.vLinesIndices().end(); ++v)
	{
		const Line& lineTemp = m_data->m_lineData.getLine(*v);
		double yb = lineTemp.s.y;
		double yu = lineTemp.e.y;
		int i = m_data->m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = m_data->m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			if (j > 0)
				m_data->m_cornerData.crossLinesVH(m_data->m_lineData.lines(), *v,
												  m_data->m_lineData.hLinesIndices()[i]);
		}
	}

	for (auto l = m_data->m_lineData.sLinesIndices().begin();
		 l != m_data->m_lineData.sLinesIndices().end(); ++l)
	{
		const Line& lineTemp = m_data->m_lineData.getLine(*l);
		double yb = std::min(lineTemp.s.y, lineTemp.e.y);
		double yu = std::max(lineTemp.s.y, lineTemp.e.y);
		int i = m_data->m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = m_data->m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			if (j > 0)
				m_data->m_cornerData.crossLinesS(m_data->m_lineData.lines(), *l,
												 m_data->m_lineData.hLinesIndices()[i]);
		}
	}

	for (auto l = m_data->m_lineData.sLinesIndices().begin();
		 l != m_data->m_lineData.sLinesIndices().end(); ++l)
	{
		const Line& lineTemp = m_data->m_lineData.getLine(*l);
		double xl = std::min(lineTemp.s.x, lineTemp.e.x);
		double xr = std::max(lineTemp.s.x, lineTemp.e.x);
		int i = m_data->m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
		int j = m_data->m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);
		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			if (j > 0)
				m_data->m_cornerData.crossLinesS(m_data->m_lineData.lines(), *l,
												 m_data->m_lineData.vLinesIndices()[i]);
		}
	}
	//斜线与斜线的相交关系
	auto s_size = m_data->m_lineData.sLinesIndices().size();
	for (int i = 0; i < s_size - 1; ++i)
	{
		for (int j = i + 1; j < s_size; ++j)
		{
			m_data->m_cornerData.crossLinesSS(m_data->m_lineData.lines(), m_data->m_lineData.sLinesIndices()[i],
											  m_data->m_lineData.sLinesIndices()[j]);
		}
	}


	return true;
}

bool Parser::divisionLineLayerIndex()
{

	return true;
}

bool Parser::isCandidate(const std::string& text)
{
	const string list[] = {
		"图号",
		"图名",
		"图 号",
		"图 名",
		"图 号",
		"图 名",
	};
	for (int i = 0; i < 6; i++)
	{
		if (text.find(list[i]) != std::string::npos)
			return true;
	}
	return false;
}

bool Parser::formats()
{
	const Transforms trans{};

	if (!_data->blockInMode->ents.empty())
	{
		_formats = formatBlock(_data->blockInMode, trans, _box, true);
	}
	else
	{
		for (auto& it : _data->blocks)
		{
			auto formats = formatBlock(it.second, trans, _box, true);
			_formats.insert(_formats.end(), formats.begin(), formats.end());
		}
	}

	crossCorners();

	m_data->m_textPointData.sortByRef(compareY);

	m_data->m_kdtTreeData.buildKDTreeData(m_data->m_endpointData.endpoints(),
										  m_data->m_textPointData.textpoints());

#if 0
	findComponentsByTexts();
#else
	findComponents();
#endif


	findBorders();

	findAxises();

	findTables();

	findBlocks();
	//findColumns();
	//findColumnSections();
	//findBeams();
	//findBeamSection();
	//findBeamLongitudinal();
	//findElevationSymbols();
	//parseLongitudinalSection();
	//serializeTest();


	//findWallLocation();

	findSectionSymbols();
	findWallsSection();

	//parseFloorPlan();


	return true;
}


bool Parser::searchSLinesBack(int id) const
{
	m_data->m_rtreeData.pushBackSBackIndex(id);
	return true;
}

bool Parser::findBlocks()
{
	_blockSet.setComponents(_components, _borders, _axises, _tables, m_data->m_dimensions,
							m_data->m_axisLineData.axisHLines(), m_data->m_axisLineData.axisVLines());
	_blockSet.blocks(*m_data, _blocks, _axises);
	return true;
}

bool Parser::findAxises()
{
	_axisSet.axis(*m_data/*, m_axisVLines, m_axisHLines*/ /*_axises*/);

	return true;
}

bool Parser::findTables()
{
	for (auto& it : _components)
	{
		if (it->hlines.size() + it->vlines.size() < MIN_TABLE_LINES)
			continue;

		auto table = Table::buildFromComponent(*it);
		if (table.get())
		{
			table->steTextPoind(m_data->m_textPointData);
			table->extract(m_data->m_kdtTreeData.kdtTexts(),
						   m_data->m_textPointData.textpoints());
			//剔除类似表格的网格；
			if (table->tableExclude())
			{
				continue;
			}
			//剔除与轴线相交的标注，其被误识别为表格
			bool tableCrossAxis = false;
			for (auto& hAxis : m_data->m_axisLineData.axisHLines())
			{
				if (table->box.cross(hAxis.second))
				{
					tableCrossAxis = true;
					break;
				}
			}
			for (auto i = 0; i < m_data->m_axisLineData.axisVLines().size() && !tableCrossAxis; ++i)
			{
				if (table->box.cross(m_data->m_axisLineData.axisVLines()[i].second))
				{
					tableCrossAxis = true;
					break;
				}
			}
			if (tableCrossAxis)
			{
				continue;
			}
			//添加表格
			_tables.push_back(table);
		}
	}
	return true;
}

bool Parser::findIssues()
{
	//
	double area = 0;
	std::vector<char> map(_components.size(), 0);

	for (auto bd : _borders)
	{
		double tmp = bd->box.area();
		if (area == 0 || tmp < area)
			area = tmp;
	}
	area = area * 4 / 5;
	bool got = false;
	for (auto it = _components.begin(); it != _components.end() && !got; ++it)
	{
		for (auto jt = _components.begin(); jt != _components.end() && !got; ++jt)
		{
			if (it - _components.begin() != jt - _components.begin())
			{
				if ((*it)->cover(*(*jt)) && (*jt)->box.area() > area)
				{
					map[it - _components.begin()] = 1;
					map[jt - _components.begin()] = 2;
					got = true;
				}
			}
		}
	}

	Box box1;
	Box box2;

	for (int i = 0; i < _components.size(); ++i)
	{
		if (map[i] == 1)
		{
			_issues.push_back(_components[i]);
			box1 = _components[i]->box;
		}
		else if (map[i] == 2)
		{
			box2 = _components[i]->box;
			std::shared_ptr<Component> cover(new Component(_components[i]->vlines,
														   _components[i]->hlines,
														   _components[i]->slines));
			cover->moveLeft();
			_covers.push_back(cover);
		}
	}
	LOG_INFO << "box1: " << box1.left << "," << box1.top << "," << box1.right
		<< "," << box1.bottom;
	LOG_INFO << "box2: " << box2.left << "," << box2.top << "," << box2.right
		<< "," << box2.bottom;
	if (box1.cover(box2))
	{
		LOG_INFO << "cover1";
	}
	else if (box2.cover(box1))
	{
		LOG_INFO << "cover2";
	}

	return true;
}

bool Parser::findBorders()
{

	_borderSet.borders(_borders, _components);

	return true;
}

bool Parser::coverByComponents(const Corner& p)
{
	for (auto t : _components)
	{
		if (t->cover(p))
			return true;
	}
	return false;
}

bool Parser::findComponents()
{
	_components =
		returnComponents(m_data->m_lineData, m_data->m_cornerData.corners());
	std::sort(_components.begin(), _components.end(),
			  [](const std::shared_ptr<Component>& c1,
				 const std::shared_ptr<Component>& c2)
	{
		return c1->box.area() > c2->box.area();
	});

	return true;
}

std::shared_ptr<Data> Parser::getDataInt()
{
	return this->m_data;
}

bool Parser::findComponentsByTexts()
{
	std::vector<std::shared_ptr<DRW_Text>> candidates;
	char* map = new char[m_data->m_lineData.lines().size()];
	memset(map, 0, m_data->m_lineData.lines().size());
	for (auto it = m_data->m_textPointData.textpoints().begin();
		 it != m_data->m_textPointData.textpoints().end(); ++it)
	{
		DRW_Text* text = static_cast<DRW_Text*>(it->entity.get());
		LOG_INFO << "point: " << it->x << "," << it->y << " : " << text->text;
		if (isCandidate(text->text))
		{
			std::shared_ptr<DRW_Text> tt =
				std::static_pointer_cast<DRW_Text>(it->entity);
			candidates.push_back(tt);
		}
	}


	return true;
}

bool Parser::findDimensionsBlock(std::shared_ptr<Dimension> dim)
{
	return false;
}

std::shared_ptr<dx_ifaceBlock> Parser::getBlock(const std::string& name)
{
	std::shared_ptr<dx_ifaceBlock> r;
	for (auto& block : _data->blocks)
	{
		if (block.second.get()->name == name)
		{
			return block.second;
		}
	}
	return r;
}

bool Parser::FindSectionDIm(const Box& box, std::vector<Dimension>& temp_dim)
{
	int l_idx = m_data->m_lineData.vLinesIndices()[box.vf];
	int r_idx = m_data->m_lineData.vLinesIndices()[box.vt];
	int t_idx = m_data->m_lineData.hLinesIndices()[box.ht];
	int b_idx = m_data->m_lineData.hLinesIndices()[box.hf];
	for (const auto& it : m_data->m_dimensions)
	{
		if (it->box.left + Precision > m_data->m_lineData.lines()[l_idx].s.x &&
			it->box.right - Precision < m_data->m_lineData.lines()[r_idx].s.x &&
			it->box.bottom + Precision > m_data->m_lineData.lines()[b_idx].s.y &&
			it->box.top - Precision < m_data->m_lineData.lines()[t_idx].s.y)
		{
			temp_dim.push_back(*it);
		}
	}
	return true;
}

void Parser::findSectionSymbols()
{
	_sectionSymbolSet.setMFCPaintTestVec(temp_index, temp_line);
	_sectionSymbolSet.setAnalysisDependencise(m_data);
	_sectionSymbolSet.parse(_sectionSybmols, _blocks);
}

bool Parser::findElevationSymbols()
{
	_elevationSymbolSet.setMFCPaintTestVec(temp_index, temp_line);
	_elevationSymbolSet.setAnalysisDependencise(m_data);
	_elevationSymbolSet.parse(_elevationSymbols, _blocks);
	return true;
}

bool Parser::parseLongitudinalSection()
{
	_longitudinalStructProfile.setMFCPaintTestVec(temp_index, temp_line);
	_longitudinalStructProfile.setAnalysisDependencise(m_data, _elevationSymbolSet, _elevationSymbols);
	_longitudinalStructProfile.parse(_blocks);
	return true;
}

void Parser::outputSectionSymbols()
{
	Json::FastWriter writer;
	Json::Value body;
	for (const auto& ssgItm : this->_sectionSybmols)
	{
		Json::Value ssg;
		for (const auto& ssItm : ssgItm->sectionSymbolVec)
		{
			Json::Value ss;
			Json::Value hAxisDistance, vAxisDistance;
			hAxisDistance["axis"] = ssItm->axisHDistance.first;
			hAxisDistance["offset"] = ssItm->axisHDistance.second;
			vAxisDistance["axis"] = ssItm->axisVDistance.first;
			vAxisDistance["offset"] = ssItm->axisVDistance.second;
			ss["relativeHorizontalAxis"] = hAxisDistance;
			ss["relativeVerticalAxis"] = vAxisDistance;
			ss["text"] = ssItm->symbolTextInfo.text;
			ssg.append(ss);
		}
		body.append(ssg);
	}
	std::fstream of("sectionSymbolGroup.json", std::ios::out);
	of << writer.write(body);
	of.close();
}

void Parser::outputBoardInfo()
{
	Json::FastWriter writer;
	Json::Value body;
#if 0
	for (const auto& boardItm : this->_longitudinalSection.longitudianlBoards)
	{
		Json::Value board, top, bottom;
		const auto& topLine = m_data->m_lineData.lines().at(boardItm->vec1.front());
		const auto& btmLine = m_data->m_lineData.lines().at(boardItm->vec2.front());
		top["endX"] = strimDouble(topLine.e.x, 1);
		top["endY"] = strimDouble(topLine.e.y, 1);
		top["startX"] = strimDouble(topLine.s.x, 1);
		top["startY"] = strimDouble(topLine.s.y, 1);
		top["ele"] = strimDouble(boardItm->ele1, 1);
		board["top"] = top;
		bottom["endX"] = strimDouble(btmLine.e.x, 1);
		bottom["endY"] = strimDouble(btmLine.e.y, 1);
		bottom["startX"] = strimDouble(btmLine.s.x, 1);
		bottom["startY"] = strimDouble(btmLine.s.y, 1);
		bottom["ele"] = strimDouble(boardItm->ele2, 1);
		board["bottom"] = bottom;
		body.append(board);
	}
#endif
	std::fstream of("ElevationOfBoard.json", std::ios::out);
	of << writer.write(body);
	of.close();
}
bool Parser::pushLine(const Point& p1, const Point& p2,
					  bool side /* = false*/)
{
	int l = m_data->m_lineData.lines().size();
	m_data->m_lineData.pushBackLines(Line(p1, p2, side));
	//初始化线图层数据
	m_data->m_lineLayerData.setLineLayerMapVec(l, m_data->m_lineData.lines().back());

	if (Line::vertical(p1, p2) || Line::horizontal(p1, p2))
	{
		if (Line::vertical(p1, p2))
		{
			m_data->m_lineData.pushBackVLinesIndices(l);
		}
		else
		{
			m_data->m_lineData.pushBackHLinesIndices(l);
		}
	}
	else
	{
		m_data->m_lineData.pushBackSLinesIndices(l);
	}
	m_data->m_endpointData.pushBackEndpoint(Endpoint(l, p1));
	m_data->m_endpointData.pushBackEndpoint(Endpoint(l, p2));
	return true;
}

std::shared_ptr<DRW_Entity> Parser::formatEntity(
	const std::shared_ptr<DRW_Entity>& entity, const Transforms& trans,
	Box& box)
{
	std::shared_ptr<DRW_Entity> res;
	Point p, p1, p2, p3, p4;
	int first;
	std::fstream of("origin text.txt", std::ios::app);
	/*if (entity->space != DRW::ModelSpace)
	{
		of << "this entity should not be here" << '\n';
		of.close();
	}*/
	switch (entity->dwgType)
	{
		case DRW::ATTRIB:
		{
			DRW_Attrib* pAttrib =
				new DRW_Attrib(*dynamic_cast<DRW_Attrib*>(entity.get()));
			p1 = trans.transR(Point(pAttrib->basePoint.x, pAttrib->basePoint.y));
			p2 = trans.transR(Point(pAttrib->secPoint.x, pAttrib->secPoint.y));
			box.expand(p1);
			// box.expand(p2);
			pAttrib->basePoint.x = p1.x;
			pAttrib->basePoint.y = p1.y;
			pAttrib->secPoint.x = p2.x;
			pAttrib->secPoint.y = p2.y;
			pAttrib->height = trans.trans(pAttrib->height);
			res.reset(dynamic_cast<DRW_Entity*>(pAttrib));
			p1.entity = res;
			p2.entity = res;
			of << pAttrib->text << std::endl;
			// add attrib text point
			m_data->m_textPointData.pushBackPoint(p1);
			break;
		}
		case DRW::LINE:
		{
			auto* line = new DRW_Line(*dynamic_cast<DRW_Line*>(entity.get()));
			p1 = trans.transR(Point(line->basePoint.x, line->basePoint.y));
			p2 = trans.transR(Point(line->secPoint.x, line->secPoint.y));
			box.expand(p1);
			box.expand(p2);

			line->basePoint.x = p1.x;
			line->basePoint.y = p1.y;
			line->secPoint.x = p2.x;
			line->secPoint.y = p2.y;

			res.reset(dynamic_cast<DRW_Entity*>(line));
			p1.entity = res;
			p2.entity = res;

			pushLine(p1, p2);
		} break;
		case DRW::POINT:
		{
			auto* point = new DRW_Point(*static_cast<DRW_Point*>(entity.get()));
			p = trans.trans(Point(point->basePoint.x, point->basePoint.y));
			box.expand(p);
			point->basePoint.x = p.x;
			point->basePoint.y = p.y;
			res.reset(static_cast<DRW_Entity*>(point));

			p.entity = res;
		} break;
		case DRW::ARC:
		{
			auto* arc = new DRW_Arc(*static_cast<DRW_Arc*>(entity.get()));
			p = trans.transR(Point(arc->basePoint.x, arc->basePoint.y));
			box.expand(p);
			arc->basePoint.x = p.x;
			arc->basePoint.y = p.y;
			res.reset(static_cast<DRW_Entity*>(arc));
			p.entity = res;
		} break;
		case DRW::CIRCLE:
		{
			DRW_Circle* circle =
				new DRW_Circle(*static_cast<DRW_Arc*>(entity.get()));
			p = trans.transR(Point(circle->basePoint.x, circle->basePoint.y));
			box.expand(p);
			circle->basePoint.x = p.x;
			circle->basePoint.y = p.y;
			circle->radious = trans.trans(circle->radious);
			res.reset(static_cast<DRW_Entity*>(circle));
			p.entity = res;

			Circle c(circle->basePoint.x, circle->basePoint.y, circle->radious);
			c.entity = res;
			m_data->m_circleData.pushBackCircles(c);
		} break;
		case DRW::ELLIPSE:
		{
			DRW_Ellipse* ellipse =
				new DRW_Ellipse(*static_cast<DRW_Ellipse*>(entity.get()));
			p1 = trans.transR(Point(ellipse->basePoint.x, ellipse->basePoint.y));
			p2 = trans.transR(Point(ellipse->secPoint.x, ellipse->secPoint.y));
			box.expand(p1);
			box.expand(p2);
			ellipse->basePoint.x = p1.x;
			ellipse->basePoint.y = p1.y;
			ellipse->secPoint.x = p2.x;
			ellipse->secPoint.y = p2.y;
			// ellipse->ratio = trans.trans(ellipse->ratio);
			res.reset(static_cast<DRW_Entity*>(ellipse));
			p1.entity = res;
			p2.entity = res;

			m_data->m_circleData.pushBackCircles(
				Circle(ellipse->basePoint.x, ellipse->basePoint.y, ellipse->minormajoratio));
		} break;
		case DRW::RAY:
		{
			DRW_Ray* ray = new DRW_Ray(*static_cast<DRW_Ray*>(entity.get()));
			p1 = trans.transR(Point(ray->basePoint.x, ray->basePoint.y));
			p2 = trans.transR(Point(ray->secPoint.x, ray->secPoint.y));
			box.expand(p1);
			box.expand(p2);
			ray->basePoint.x = p1.x;
			ray->basePoint.y = p1.y;
			ray->secPoint.x = p2.x;
			ray->secPoint.y = p2.y;
			res.reset(static_cast<DRW_Entity*>(ray));
			p1.entity = res;
			p2.entity = res;
			pushLine(p1, p2);
		} break;
		case DRW::XLINE:
		{
			DRW_Xline* xline = new DRW_Xline(*static_cast<DRW_Xline*>(entity.get()));
			p1 = trans.transR(Point(xline->basePoint.x, xline->basePoint.y));
			p2 = trans.transR(Point(xline->secPoint.x, xline->secPoint.y));
			box.expand(p1);
			box.expand(p2);
			xline->basePoint.x = p1.x;
			xline->basePoint.y = p1.y;
			xline->secPoint.x = p2.x;
			xline->secPoint.y = p2.y;
			res.reset(static_cast<DRW_Entity*>(xline));
			p1.entity = res;
			p2.entity = res;
			pushLine(p1, p2);
		} break;
		case DRW::TRACEE:
		{
			DRW_Trace* trace = new DRW_Trace(*static_cast<DRW_Trace*>(entity.get()));
			p1 = trans.transR(Point(trace->basePoint.x, trace->basePoint.y));
			p2 = trans.transR(Point(trace->secPoint.x, trace->secPoint.y));
			p3 = trans.transR(Point(trace->thirdPoint.x, trace->thirdPoint.y));
			p4 = trans.transR(Point(trace->fourPoint.x, trace->fourPoint.y));
			box.expand(p1);
			box.expand(p2);
			box.expand(p3);
			box.expand(p4);
			trace->basePoint.x = p1.x;
			trace->basePoint.y = p1.y;
			trace->secPoint.x = p2.x;
			trace->secPoint.y = p2.y;
			trace->thirdPoint.x = p3.x;
			trace->thirdPoint.y = p3.y;
			trace->fourPoint.x = p4.x;
			trace->fourPoint.y = p4.y;
			res.reset(static_cast<DRW_Entity*>(trace));
			p1.entity = res;
			p2.entity = res;
			p3.entity = res;
			p4.entity = res;
			pushLine(p1, p2, true);
			pushLine(p3, p4, true);
			pushLine(p1, p3, true);
			pushLine(p2, p4, true);
		} break;
		case DRW::SOLID:
		{
			DRW_Solid* solid = new DRW_Solid(*static_cast<DRW_Solid*>(entity.get()));
			p1 = trans.transR(Point(solid->basePoint.x, solid->basePoint.y));
			p2 = trans.transR(Point(solid->secPoint.x, solid->secPoint.y));
			p3 = trans.transR(Point(solid->thirdPoint.x, solid->thirdPoint.y));
			p4 = trans.transR(Point(solid->fourPoint.x, solid->fourPoint.y));
			box.expand(p1);
			box.expand(p2);
			box.expand(p3);
			box.expand(p4);
			solid->basePoint.x = p1.x;
			solid->basePoint.y = p1.y;
			solid->secPoint.x = p2.x;
			solid->secPoint.y = p2.y;
			solid->thirdPoint.x = p3.x;
			solid->thirdPoint.y = p3.y;
			solid->fourPoint.x = p4.x;
			solid->fourPoint.y = p4.y;
			res.reset(static_cast<DRW_Entity*>(solid));
			p1.entity = res;
			p2.entity = res;
			p3.entity = res;
			p4.entity = res;
			pushLine(p1, p2, true);
			pushLine(p3, p4, true);
			pushLine(p1, p3, true);
			pushLine(p2, p4, true);
			_solids.push_back(std::static_pointer_cast<DRW_Solid>(res));
		} break;
		case DRW::POLYLINE_2D:
		case DRW::POLYLINE_3D:
		case DRW::POLYLINE_PFACE:
		case DRW::POLYLINE_MESH:
		{
			DRW_Polyline* polyline =
				new DRW_Polyline(*static_cast<DRW_Polyline*>(entity.get()));

			res.reset(static_cast<DRW_Entity*>(polyline));
			first = 1;
			bool side = polyline->flags & 1;

			for (auto& v : polyline->verts)
			{
				p = trans.transR(Point(v->basePoint.x, v->basePoint.y));
				p.entity = res;
				box.expand(p);
				v->basePoint.x = p.x;
				v->basePoint.y = p.y;
				if (first)
				{
					first = 0;
					p2 = p;
				}
				else
				{
					pushLine(p1, p, side);
				}
				p1 = p;
			}
			if (polyline->flags & 1)
			{
				pushLine(p1, p2, side);
			}
		} break;
		case DRW::LWPOLYLINE:
		{
			DRW_LWPolyline* pLwpolyline =
				new DRW_LWPolyline(*dynamic_cast<DRW_LWPolyline*>(entity.get()));
			res.reset(dynamic_cast<DRW_Entity*>(pLwpolyline));

			first = 1;
			bool bSide = pLwpolyline->flags & 1;

			//闭合处理
			if (!bSide &&
				pLwpolyline->vertlist.front()->x == pLwpolyline->vertlist.back()->x &&
				pLwpolyline->vertlist.front()->y == pLwpolyline->vertlist.back()->y)
			{
				bSide = true;
			}

			for (auto& v : pLwpolyline->vertlist)
			{
				p = trans.transR(Point(v->x, v->y));
				p.entity = res;
				box.expand(p);
				v->x = p.x;
				v->y = p.y;
				if (first)
				{
					first = 0;
					p2 = p;
				}
				else
				{
					pushLine(p1, p, bSide);
				}
				p1 = p;
			}
			if (pLwpolyline->flags & 1)
			{
				pushLine(p1, p2, bSide);
			}
			break;
		}
		case DRW::TEXT:
		{
			DRW_Text* text = new DRW_Text(*dynamic_cast<DRW_Text*>(entity.get()));
			p1 = trans.transR(Point(text->basePoint.x, text->basePoint.y));
			p2 = trans.transR(Point(text->secPoint.x, text->secPoint.y));
			box.expand(p1);
			// box.expand(p2);
			text->basePoint.x = p1.x;
			text->basePoint.y = p1.y;
			text->secPoint.x = p2.x;
			text->secPoint.y = p2.y;
			text->height = trans.trans(text->height);
			res.reset(dynamic_cast<DRW_Entity*>(text));
			p1.entity = res;
			p2.entity = res;

			//_textlines.push_back(Line(p1, p2));
			of << text->text << std::endl;
			m_data->m_textPointData.pushBackPoint(p1);
			break;
		}
		case DRW::MTEXT:
		{
			DRW_MText* mtext = new DRW_MText(*dynamic_cast<DRW_MText*>(entity.get()));
			p1 = trans.transR(Point(mtext->basePoint.x, mtext->basePoint.y));
			p2 = trans.transR(Point(mtext->secPoint.x, mtext->secPoint.y));
			box.expand(p1);
			// box.expand(p2);
			mtext->basePoint.x = p1.x;
			mtext->basePoint.y = p1.y;
			mtext->secPoint.x = p2.x;
			mtext->secPoint.y = p2.y;
			mtext->height = trans.trans(mtext->height);
			res.reset(dynamic_cast<DRW_Entity*>(mtext));
			p1.entity = res;
			p2.entity = res;
			//_textlines.push_back(Line(p1, p2));
			// std::string str2 =mtext->text;
			// del_char(str2);
			of << mtext->text << std::endl;
			//of << str2 << std::endl;
			m_data->m_textPointData.pushBackPoint(p1);
			break;
		}
		case DRW::HATCH:
		{
			DRW_Hatch* hatch = new DRW_Hatch(*dynamic_cast<DRW_Hatch*>(entity.get()));
			res.reset(dynamic_cast<DRW_Entity*>(hatch));
			for (auto path : hatch->paths)
			{
				for (const auto& ent : path->entities)
				{
					if (ent->dwgType == DRW::LINE)
					{
						DRW_Line* line = std::dynamic_pointer_cast<DRW_Line>(ent).get();
						p1 = trans.transR(Point(line->basePoint.x, line->basePoint.y));
						p2 = trans.transR(Point(line->secPoint.x, line->secPoint.y));

						box.expand(p1);
						box.expand(p2);
						p1.entity = res;
						p2.entity = res;
						pushLine(p1, p2);
					}
					else if (ent->dwgType == DRW::LWPOLYLINE)
					{
						//LWPOLYLINE PATH
						DRW_LWPolyline* lwpolyline = std::dynamic_pointer_cast<DRW_LWPolyline>(ent).get();
						first = 1;
						bool side = lwpolyline->flags & 1;
						for (auto& v : lwpolyline->vertlist)
						{
							p = trans.transR(Point(v->x, v->y));
							p.entity = res;
							box.expand(p);
							v->x = p.x;
							v->y = p.y;
							if (first)
							{
								first = 0;
								p2 = p;
							}
							else
							{
								pushLine(p1, p, side);
							}
							p1 = p;
						}
						if (lwpolyline->flags & 1)
						{
							pushLine(p1, p2, side);
						}
					}
					else if (ent->dwgType == DRW::POLYLINE_2D ||
							 ent->dwgType == DRW::POLYLINE_3D ||
							 ent->dwgType == DRW::POLYLINE_PFACE ||
							 ent->dwgType == DRW::POLYLINE_MESH)
					{
						//POLYLINE PATH
						DRW_Polyline* polyline = std::dynamic_pointer_cast<DRW_Polyline>(ent).get();
						first = 1;
						bool side = polyline->flags & 1;
						for (auto& v : polyline->verts)
						{
							p = trans.transR(Point(v->basePoint.x, v->basePoint.y));
							p.entity = res;
							box.expand(p);
							v->basePoint.x = p.x;
							v->basePoint.y = p.y;
							if (first)
							{
								first = 0;
								p2 = p;
							}
							else
							{
								pushLine(p1, p, side);
							}
							p1 = p;
						}
						if (polyline->flags & 1)
						{
							pushLine(p1, p2, side);
						}
					}
					if (ent->dwgType == DRW::ARC)
					{
						// ARC PATH
						DRW_Arc* arc = std::dynamic_pointer_cast<DRW_Arc>(ent).get();
						p = trans.transR(Point(arc->basePoint.x, arc->basePoint.y));
						box.expand(p);
						arc->basePoint.x = p.x;
						arc->basePoint.y = p.y;
						/*	res.reset(dynamic_cast<DRW_Entity*>(arc));*/
						p.entity = res;
					}
					if (ent->dwgType == DRW::ELLIPSE)
					{
						// ELLIPSE PATH
						DRW_Ellipse* ellipse = std::dynamic_pointer_cast<DRW_Ellipse>(ent).get();
						p1 = trans.transR(Point(ellipse->basePoint.x, ellipse->basePoint.y));
						p2 = trans.transR(Point(ellipse->secPoint.x, ellipse->secPoint.y));
						box.expand(p1);
						box.expand(p2);
						ellipse->basePoint.x = p1.x;
						ellipse->basePoint.y = p1.y;
						ellipse->secPoint.x = p2.x;
						ellipse->secPoint.y = p2.y;
						//ellipse->minormajoratio = trans.trans(ellipse->minormajoratio);
						p1.entity = res;
						p2.entity = res;
						m_data->m_circleData.pushBackCircles(
							Circle(ellipse->basePoint.x, ellipse->basePoint.y, ellipse->minormajoratio));
					}
					if (ent->dwgType == DRW::SPLINE)
					{
						// TODO ????
					}
				}
			}
			break;
		}
		case DRW::SPLINE:
		{
			//
			break;
		}
		case DRW::LEADER:
		{
			break;
		}
		case DRW::DIMENSION_LINEAR:
		{
			double angle_rotated = 0;
			double oblique_angle = 0;
			auto* pDimLinear =
				new DRW_DimLinear(*dynamic_cast<DRW_DimLinear*>(entity.get()));

			//< Definition point 1, code 13, 23 & 33
			Point p13 = trans.transR(Point(pDimLinear->getDef1Point().x, pDimLinear->getDef1Point().y));
			//< Definition point 2, code 14, 24 & 34
			Point p14 = trans.transR(Point(pDimLinear->getDef2Point().x, pDimLinear->getDef2Point().y));
			//< dim line location point, code 10, 20 & 30
			Point p10 = trans.transR(Point(pDimLinear->getDimPoint().x, pDimLinear->getDimPoint().y));
			//< Insertion for clones (Baseline & Continue), 12, 22 & 32
			Point p12 = trans.transR(Point(pDimLinear->getTextPoint().x, pDimLinear->getTextPoint().y));

			angle_rotated = pDimLinear->getAngle();
			oblique_angle = pDimLinear->getOblique();
			box.expand(p13);
			box.expand(p14);
			box.expand(p10);
			box.expand(p12);
			pDimLinear->setDef1Point(DRW_Coord(p13.x, p13.y, 0));
			pDimLinear->setDef2Point(DRW_Coord(p14.x, p14.y, 0));
			pDimLinear->setDimPoint(DRW_Coord(p10.x, p10.y, 0));
			pDimLinear->setTextPoint(DRW_Coord(p12.x, p12.y, 0));

			res.reset(dynamic_cast<DRW_DimLinear*>(pDimLinear));
			p13.entity = res;
			p14.entity = res;
			p10.entity = res;

			Point pairPt;
			pairPt.entity = res;
			{
				//计算第四个点
				Point tpPt(p10.x - p14.x + p13.x, p10.y - p14.y + p13.y, p13.entity);
				Line l(tpPt, p13);
				//垂直
				if (abs(pDimLinear->getAngle() - M_PI_2) < 0.01 ||
					abs(pDimLinear->getAngle() - M_PI_2 * 3) < 0.01)
				{
					auto y = knowXToY(l, p10.x);
					pairPt.x = p10.x;
					pairPt.y = y;
				}
				//水平
				else
				{
					auto x = knowYToX(l, p10.y);
					pairPt.y = p10.y;
					pairPt.x = x;
				}
			}

			pushLine(p13, pairPt);
			pushLine(p14, p10);
			pushLine(pairPt, p10);
			std::vector<Line> lines;
			std::vector<int> indices;
			indices.push_back(lines.size());
			lines.emplace_back(p13, pairPt);
			indices.push_back(lines.size());
			lines.emplace_back(p14, p10);
			indices.push_back(lines.size());
			lines.emplace_back(pairPt, p10);
			auto dim = std::make_shared<Dimension>(Dimension::DIMLINEAR,
												   pDimLinear->getActualMeasurement(),
												   lines,
												   indices);
			m_data->m_dimensions.push_back(dim);
			m_data->m_dimData.setDim(dim);
			break;
		}
		case DRW::DIMENSION_ALIGNED:
		{
			auto* pDimAligned =
				new DRW_DimAligned(*dynamic_cast<DRW_DimAligned*>(entity.get()));
			//< Definition point 1, code 13, 23 & 33
			p1 = trans.transR(Point(pDimAligned->getDef1Point().x, pDimAligned->getDef1Point().y));
			//< Definition point 2, code 14, 24 & 34
			p2 = trans.transR(Point(pDimAligned->getDef2Point().x, pDimAligned->getDef2Point().y));
			//< dim line location point, code 10, 20 & 30
			p3 = trans.transR(
				Point(pDimAligned->getDimPoint().x, pDimAligned->getDimPoint().y));
			//< Insertion for clones (Baseline & Continue), 12, 22 & 32
			p4 = trans.transR(Point(pDimAligned->getTextPoint().x, pDimAligned->getTextPoint().y));

			box.expand(p1);
			box.expand(p2);
			box.expand(p3);
			box.expand(p4);
			pDimAligned->setDef1Point(DRW_Coord(p1.x, p1.y, 0));
			pDimAligned->setDef2Point(DRW_Coord(p2.x, p2.y, 0));
			pDimAligned->setDimPoint(DRW_Coord(p3.x, p3.y, 0));
			pDimAligned->setTextPoint(DRW_Coord(p4.x, p4.y, 0));

			res.reset(dynamic_cast<DRW_DimAligned*>(pDimAligned));
			p1.entity = res;
			p2.entity = res;
			p3.entity = res;
			Point tpPt(p3.x - p2.x + p1.x, p3.y - p2.y + p1.y, p1.entity);
			pushLine(p1, tpPt);
			pushLine(p2, p3);
			pushLine(tpPt, p3);
			std::vector<Line> lines;
			std::vector<int> indices;
			indices.push_back(lines.size());
			lines.emplace_back(p1, tpPt);
			indices.push_back(lines.size());
			lines.emplace_back(p2, p3);
			indices.push_back(lines.size());
			lines.emplace_back(tpPt, p3);
			auto dim = std::make_shared<Dimension>(Dimension::DIMALIGNED,
												   pDimAligned->getActualMeasurement(),
												   lines,
												   indices);
			m_data->m_dimensions.push_back(dim);
			m_data->m_dimData.setDim(dim);
			break;
		}
		case DRW::DIMENSION_ANG2LN:
		{
			auto* pDimAngular =
				new DRW_DimAngular(*dynamic_cast<DRW_DimAngular*>(entity.get()));
			Point p13 = trans.transR(Point(pDimAngular->getFirstLine1().x, pDimAngular->getFirstLine1().y));
			Point p14 = trans.transR(Point(pDimAngular->getFirstLine2().x, pDimAngular->getFirstLine2().y));
			Point p15 = trans.transR(Point(pDimAngular->getSecondLine1().x, pDimAngular->getSecondLine1().y));
			Point p10 = trans.transR(Point(pDimAngular->getSecondLine2().x, pDimAngular->getSecondLine2().y));
			Point p11 = trans.transR(Point(pDimAngular->getTextPoint().x, pDimAngular->getTextPoint().y));
			box.expand(p13);
			box.expand(p14);
			box.expand(p15);
			box.expand(p10);
			box.expand(p11);

			pDimAngular->setFirstLine1(DRW_Coord(p13.x, p13.y, 0));
			pDimAngular->setFirstLine2(DRW_Coord(p14.x, p14.y, 0));
			pDimAngular->setSecondLine1(DRW_Coord(p15.x, p15.y, 0));
			pDimAngular->setSecondLine2(DRW_Coord(p10.x, p10.y, 0));
			pDimAngular->setTextPoint(DRW_Coord(p11.x, p11.y, 0));
			res.reset(dynamic_cast<DRW_DimAngular*>(pDimAngular));
			p13.entity = res;
			p14.entity = res;
			p15.entity = res;
			p10.entity = res;
			p11.entity = res;
			pushLine(p13, p14);
			pushLine(p15, p10);
			std::vector<Line> lines;
			std::vector<int> indices;
			indices.push_back(lines.size());
			lines.emplace_back(p13, p14);
			indices.push_back(lines.size());
			lines.emplace_back(p15, p10);
			auto dim = std::make_shared<Dimension>(Dimension::DIMANGULAR,
												   pDimAngular->getActualMeasurement(),
												   lines,
												   indices);
			m_data->m_dimData.setDim(dim);
		}
		case DRW::DIMENSION_ANG3PT:
		{
			auto* pDimAngular3p =
				new DRW_DimAngular3p(*dynamic_cast<DRW_DimAngular3p*>(entity.get()));
			Point p13 = trans.transR(Point(pDimAngular3p->getFirstLine().x, pDimAngular3p->getFirstLine().y));
			Point p14 = trans.transR(Point(pDimAngular3p->getFirstLine().x, pDimAngular3p->getFirstLine().y));
			Point p15 = trans.transR(Point(pDimAngular3p->getVertexPoint().x, pDimAngular3p->getVertexPoint().y));
			Point p10 = trans.transR(Point(pDimAngular3p->getDimPoint().x, pDimAngular3p->getDimPoint().y));
			Point p11 = trans.transR(Point(pDimAngular3p->getTextPoint().x, pDimAngular3p->getTextPoint().y));
			box.expand(p13);
			box.expand(p14);
			box.expand(p15);
			box.expand(p10);
			box.expand(p11);

			pDimAngular3p->setFirstLine(DRW_Coord(p13.x, p13.y, 0));
			pDimAngular3p->setFirstLine(DRW_Coord(p14.x, p14.y, 0));
			pDimAngular3p->SetVertexPoint(DRW_Coord(p15.x, p15.y, 0));
			pDimAngular3p->setDimPoint(DRW_Coord(p10.x, p10.y, 0));
			pDimAngular3p->setTextPoint(DRW_Coord(p11.x, p11.y, 0));
			res.reset(dynamic_cast<DRW_DimAngular*>(pDimAngular3p));
			p13.entity = res;
			p14.entity = res;
			p15.entity = res;
			p10.entity = res;
			p11.entity = res;
			pushLine(p15, p13);
			pushLine(p15, p14);
			std::vector<Line> lines;
			std::vector<int> indices;
			indices.push_back(lines.size());
			lines.emplace_back(p15, p13);
			indices.push_back(lines.size());
			lines.emplace_back(p15, p14);
			auto dim = std::make_shared<Dimension>(Dimension::DIMANGULAR,
												   pDimAngular3p->getActualMeasurement(),
												   lines,
												   indices);
			m_data->m_dimData.setDim(dim);
			break;
		}
		case DRW::DIMENSION_RADIUS:
		{
			auto* pDimRadius =
				new DRW_DimRadius(*dynamic_cast<DRW_DimRadius*>(entity.get()));
			Point p15 = trans.transR(Point(pDimRadius->getDiameterPoint().x, pDimRadius->getDiameterPoint().y));
			Point p10 = trans.transR(Point(pDimRadius->getCenterPoint().x, pDimRadius->getCenterPoint().y));
			Point p11 = trans.transR(Point(pDimRadius->getTextPoint().x, pDimRadius->getTextPoint().y));

			box.expand(p15);
			box.expand(p10);
			box.expand(p11);

			pDimRadius->setDiameterPoint(DRW_Coord(p15.x, p15.y, 0));
			pDimRadius->setCenterPoint(DRW_Coord(p10.x, p10.y, 0));
			pDimRadius->setTextPoint(DRW_Coord(p11.x, p11.y, 0));
			res.reset(dynamic_cast<DRW_DimAngular*>(pDimRadius));
			p15.entity = res;
			p10.entity = res;
			p11.entity = res;

			auto dim = std::make_shared<Dimension>(Dimension::DIMRADIUS,
												   pDimRadius->getActualMeasurement(),
												   p10);
			m_data->m_dimData.setDim(dim);
			break;
		}
		case DRW::DIMENSION_DIAMETER:
		{
			auto* pDimDiameter =
				new DRW_DimDiameter(*dynamic_cast<DRW_DimDiameter*>(entity.get()));
			Point p15 = trans.transR(Point(pDimDiameter->getDiameter1Point().x, pDimDiameter->getDiameter1Point().y));
			Point p10 = trans.transR(Point(pDimDiameter->getDiameter2Point().x, pDimDiameter->getDiameter2Point().y));
			Point p11 = trans.transR(Point(pDimDiameter->getTextPoint().x, pDimDiameter->getTextPoint().y));

			box.expand(p15);
			box.expand(p10);
			box.expand(p11);

			pDimDiameter->setDiameter1Point(DRW_Coord(p15.x, p15.y, 0));
			pDimDiameter->setDiameter2Point(DRW_Coord(p10.x, p10.y, 0));
			pDimDiameter->setTextPoint(DRW_Coord(p11.x, p11.y, 0));
			res.reset(dynamic_cast<DRW_DimAngular*>(pDimDiameter));
			p15.entity = res;
			p10.entity = res;
			p11.entity = res;

			auto dim = std::make_shared<Dimension>(Dimension::DIMDIAMETER,
												   pDimDiameter->getActualMeasurement(),
												   Point((p10.x + p15.x) / 2, (p10.y + p15.y) / 2));
			m_data->m_dimData.setDim(dim);
			break;
		}
		case DRW::DIMENSION_ORDINATE:
		{
			auto* pDimOrdinate =
				new DRW_DimOrdinate(*dynamic_cast<DRW_DimOrdinate*>(entity.get()));
			break;
		}
		default:
			break;
	}
	of.close();
	return res;
}

Transform transInsert(std::shared_ptr<DRW_Insert>& insert,
					  const Transform& trans)
{
	Transform target;
	// target.x = insert->basePoint.x * trans.xscale + trans.x;
	// target.y = insert->basePoint.y * trans.yscale + trans.y;
	target.x = trans.xscale * trans.x + insert->basePoint.x;
	target.y = trans.yscale * trans.y + insert->basePoint.y;

	target.xscale = insert->xscale * trans.xscale;
	target.yscale = insert->yscale * trans.yscale;
	target.rotation = insert->angle + trans.rotation;
	return target;
}

std::vector<std::shared_ptr<DRW_Entity>> Parser::formatBlock(
	const std::shared_ptr<dx_ifaceBlock>& block,
	const Transforms& trans,
	Box& box,
	bool entry = false)
{
	std::vector<std::shared_ptr<DRW_Entity>> entities;
	for (auto it = block->ents.begin(); it != block->ents.end(); ++it)
	{
		if (it->get()->dwgType != DRW::INSERT /*&& it->get()->visible*/)
		{
			Box b;
			auto et = formatEntity(*it, trans, b);
			box.expand(b);
			if (et)
				entities.push_back(et);

			if (entry)
			{
				Entry e;
				e.box = b;
				e.entity = *it;
				_enties.push_back(e);
			}
		}
		else /* if(it->get()->visible)*/
		{
			Box b;
			std::shared_ptr<DRW_Insert> ent = std::static_pointer_cast<DRW_Insert>(*it);
			std::shared_ptr<dx_ifaceBlock> subblock = getBlock(ent->blockName);
			if (!subblock.get())
			{
				continue;
			}

			auto t = trans;
			t.appendTransform(transInsert(ent, Transform()));
			auto subentities = formatBlock(subblock, t, b);

			box.expand(b);
			entities.insert(entities.end(), subentities.begin(), subentities.end());
			if (entry)
			{
				Entry e;
				e.box = b;
				e.entity = *it;
				_enties.push_back(e);
			}
		}
	}
	return entities;
}

Json::Value Parser::serializePillar(const std::shared_ptr<Pillar>& pillar)
{
	Json::Value lines;
	Json::Value p;
	p["name"] = pillar->getName();
	for (auto line : pillar->getHlines())
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}

	for (auto line : pillar->getVlines())
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}

	for (auto line : pillar->getSlines())
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	return p;
}

Json::Value Parser::serializeBlock(const std::shared_ptr<Block>& block)
{
	Json::Value title;
	Json::Value box;
	/*Json::Value box_r;
	  Json::Value box_t;
	  Json::Value box_b;*/
	box["block_left"] = block->box.left;
	box["block_right"] = block->box.right;
	box["block_top"] = block->box.top;
	box["block_bottom"] = block->box.bottom;
	title[block->name] = box;
	Json::Value blk;
	blk["title"] = title;
	return blk;
}

Json::Value Parser::serializePillarSection(
	const std::shared_ptr<ColumnSection>& section)
{
	return section->OutPut();
}

Json::Value Parser::serializeText()
{
	Json::FastWriter writer;
	Json::Value body;
	Json::Value texts;
	body["type"] = "text";
	Json::Value border;
	if (!_borders.empty())
	{
		Box box = _borders.front()->box;
		border["bottom"] = box.bottom;
		border["top"] = box.top;
		border["left"] = box.left;
		border["right"] = box.right;
	}
	body["border"].append(border);

	for (auto textPoint : m_data->m_textPointData.textpoints())
	{
		std::shared_ptr<DRW_Text> tt1 =
			std::static_pointer_cast<DRW_Text>(textPoint.entity);
		texts["insertion_ptX"] = textPoint.x;
		texts["insertion_ptY"] = textPoint.y;
		texts["text"] = tt1->text;
		body["text_information"].append(texts);
	}
	std::fstream of("dxfTextInformation.json", std::ios::out);
	of << writer.write(body);
	of.close();
	return Json::Value();
}


Json::Value Parser::serializeWall()
{
	Json::Value body;

#if 0
	Json::FastWriter writer;

	int id = 1;
	for (auto p_wall : _wallSet.wallVec)
	{
		Json::Value wall;
		wall["id"] = id++;
		Json::Value location;
		Json::Value point1;
		Json::Value point2;
		location["point1"] = point1;
		location["point2"] = point2;
		point1["nameHAxis"] = p_wall->relativeLocation1.nameHAxis;
		point1["distanceHAxis"] = p_wall->relativeLocation1.distanceHAxis;
		point1["nameVAxis"] = p_wall->relativeLocation1.nameVAxis;
		point1["distanceVAxis"] = p_wall->relativeLocation1.distanceVAxis;
		location["point1"] = point1;
		point2["nameHAxis"] = p_wall->relativeLocation2.nameHAxis;
		point2["distanceHAxis"] = p_wall->relativeLocation2.distanceHAxis;
		point2["nameVAxis"] = p_wall->relativeLocation2.nameVAxis;
		point2["distanceVAxis"] = p_wall->relativeLocation2.distanceVAxis;
		location["point2"] = point2;
		location["thick"] = p_wall->getThick();
		wall["location"] = location;

		wall["height"] = 0;
		wall["rebar"] = "";

		body.append(wall);
		//wall.clear();
	}
	std::fstream of("ParserserializeWall.json", std::ios::out);
	of << writer.write(body);
	of.close();
#endif
	return body;
}



Json::Value Parser::serializeAxises()
{
	Json::FastWriter writer;
	Json::Value temp_axises;
	Json::Value reference;

	if (!_axises.empty())
	{
		auto axises = _axises.front();
		reference["referenceX"] = axises->referenceAxisX.front().first.first.second;
		reference["referenceY"] = axises->referenceAxisY.front().first.first.second;
		temp_axises["reference"] = reference;
		Json::Value axisH;
		Json::Value axisV;
		for (auto it : axises->hlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisH.append(axis);
		}
		for (auto it : axises->vlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisV.append(axis);
		}
		temp_axises["axisH"] = axisH;
		temp_axises["axisV"] = axisV;
	}

	std::fstream of("AxisInformation.json", std::ios::out);
	of << writer.write(temp_axises);
	of.close();
	return Json::Value();
}

Json::Value Parser::serializeLinehandle()
{
	Json::FastWriter writer;
	Json::Value lines;

	for (auto it : m_data->m_lineData.lines())
	{
		Json::Value tempLine;
		Json::Value PointS, PointE;
		tempLine["handle"] = it.entity->handle;
		PointS["X"] = it.s.x;
		PointS["Y"] = it.s.y;
		PointE["X"] = it.e.x;
		PointE["Y"] = it.e.y;
		tempLine["pointS"] = PointS;
		tempLine["pointE"] = PointE;
		lines.append(tempLine);
	}

	std::fstream of("lineHandle.json", std::ios::out);
	of << writer.write(lines);
	of.close();
	return Json::Value();
}

Json::Value Parser::serializeColumnSection()
{
	Json::FastWriter writer;

	std::fstream of("columnSection.json", std::ios::out);
	of << writer.write(_sectionSet.testSection);
	of.close();
	return Json::Value();
}

void Parser::serializeTest()
{
	Json::FastWriter writer;
	Json::Value body;

	for (const auto& flrItm : _longitudinalStructProfile.longitudinalFloors)
	{
		Json::Value floor;
		const auto& b = _longitudinalStructProfile.longitudianlBoards.at(flrItm->mainIdx);
		floor["ele1"] = b->ele1;
		floor["ele2"] = b->ele2;
		body.append(floor);
	}

	std::fstream of("floor.json", std::ios::out);
	of << writer.write(body);
	of.close();
}

Json::Value Parser::serializeComponent(
	const std::shared_ptr<Component>& border)
{
	Json::Value lines;
	// Json::Value texts;
	for (auto line : border->vlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	for (auto line : border->hlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	Json::Value tb;
	// tb["entities"] = exportBorder(border);
	// Json::Value entities;
	// for (auto entity : _formats) {
	//	Json::Value ent;
	//	switch (entity->eType) {

	//	default:
	//		break;
	//	}
	//}

	// tb["entities"] = entities;
	tb["lines"] = lines;
	return tb;
}

Json::Value Parser::serializeBorder(const std::shared_ptr<Component>& border)
{
	Json::Value lines;
	// Json::Value texts;
	for (auto line : border->vlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	for (auto line : border->hlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	Json::Value tb;
	// tb["entities"] = exportBorder(border);
	Json::Value entities;
	for (auto entity : _formats)
	{
		Json::Value ent;
		/*switch (entity->eType)
		{
			default:
				break;
		}*/
	}

	tb["entities"] = entities;
	tb["lines"] = lines;
	return tb;
}

Json::Value Parser::serializeTable(const std::shared_ptr<Table>& table)
{
	Json::Value lines;
	Json::Value texts;
	Json::Value attributeInformation;
	for (auto line : table->vlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}
	for (auto line : table->hlines)
	{
		Json::Value l;
		Json::Value s;
		Json::Value e;
		s["x"] = line.s.x;
		s["y"] = line.s.y;
		e["x"] = line.e.x;
		e["y"] = line.e.y;
		l["s"] = s;
		l["e"] = e;
		lines.append(l);
	}

	for (auto kv : table->table)
	{
		auto row = kv.first;
		for (auto pair : kv.second)
		{
			auto col = pair.first;
			auto text = pair.second;
			// if (text->text.empty())
			//	continue;
			Json::Value t;
			t["text"] = text.first /*->text*/;
			t["row"] = row;
			t["col"] = col;
			t["x"] = text.second.x /*->basePoint.x*/;
			t["y"] = text.second.y /*->basePoint.y*/;

			texts.append(t);
		}
	}
	int property_location = table->returnAttributeArrangementPlus();
	if (property_location == HORIZONTAL_LEAD_PROERTY_TOP ||
		property_location == HORIZONTAL_LEAD_PROERTY_BOTTOM)
	{
		attributeInformation[table->property_row.begin()->first] =
			table->property_row.begin()->second;
		Json::Value repeat;
		repeat["Is_or_not"] = table->b_attr_repeat.begin()->first;
		repeat["step"] = table->b_attr_repeat.begin()->second;
		attributeInformation["repeat"] = repeat;
	}
	else if (property_location == COLUMN_LEAD_PROERTY_LEFT ||
			 property_location == COLUMN_LEAD_PROERTY_RIGHT)
	{
		attributeInformation[table->property_col.begin()->first] =
			table->property_col.begin()->second;
		Json::Value repeat;
		repeat["Is_or_not"] = table->b_attr_repeat.begin()->first;
		repeat["step"] = table->b_attr_repeat.begin()->second;
		attributeInformation["repeat"] = repeat;
	}
	Json::Value tb;
	tb["texts"] = texts;
	tb["lines"] = lines;
	tb["attributeInformation"] = attributeInformation;
	return tb;
}

std::shared_ptr<dx_data> Parser::cutBy(const Box& box)
{


	std::shared_ptr<dx_data> data(new dx_data());
	data->lineTypes = _data->lineTypes;
	data->layers = _data->layers;
	data->dimStyles = _data->dimStyles;
	data->VPorts = _data->VPorts;
	data->textStyles = _data->textStyles;
	data->appIds = _data->appIds;
	// data->images

	// TODO ignore block struct or keep it
	for (auto entry : _enties)
	{
		if (!box.cover(entry.box))
			continue;

		data->blockInMode->ents.push_back(entry.entity);
		if (entry.entity->dwgType == DRW::INSERT)
		{
			//
			std::shared_ptr<DRW_Insert> ent =
				std::dynamic_pointer_cast<DRW_Insert>(entry.entity);
			std::shared_ptr<dx_ifaceBlock> subblock = getBlock(ent->blockName);
			bool found = false;
			for (const auto& blkItm : data->blocks)
			{
				if (blkItm.second->name == ent->blockName)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				data->blocks[subblock->handle] = subblock;
			}
		}
	}
	return data;
}

bool Parser::findColumns()
{
	if (!_axises.empty())
	{
		_pillarSet.setMember(_axises.front());
		//_pillarSet.columns(*m_data, _pillars/*, temp_index, temp_line, _corners*/);
		//tmep_pair_index = _pillarSet.returnColumnBorderGroupIndex();
		_pillarSet.findColumns(*m_data, _pillars);
	}


#if 0
	std::fstream of("pillar.txt", std::ios::out);
	for (int i = 0; i < _pillars.size(); ++i)
	{
		of << "柱名：" << _pillars[i]->name << std::endl;
		if (_pillars[i]->error)
		{
			of << "此柱可能存在问题" << std::endl;
		}
		else
		{
			of << "横轴距离--->"
				<< "横轴名：" << _pillars[i]->hdistance.first << "\t"
				<< "底间距：" << _pillars[i]->hdistance.second.first << "\t"
				<< "顶间距：" << _pillars[i]->hdistance.second.second << std::endl;

			of << "纵轴距离--->"
				<< "纵轴名：" << _pillars[i]->vdistance.first << "\t"
				<< "左间距：" << _pillars[i]->vdistance.second.first << "\t"
				<< "右间距：" << _pillars[i]->vdistance.second.second << std::endl;
		}
	}
	of.close();
#endif
	return true;
}

bool Parser::findColumnSections()
{

#if 1
	for (auto it : _blocks)
	{
		auto ite = _sectionSet.columnSection(*m_data, it);
		if (ite != nullptr)
			_section.push_back(ite);
		testBoxVec.push_back(_sectionSet.testBox);
	}
	temp_index = _sectionSet.testIndexVec;
	_corners = _sectionSet.testPoint;

	serializeColumnSection();

#endif
	return true;
}


bool Parser::findBeams()
{
	if (!_axises.empty())
	{
		std::shared_ptr<Axis> temp_axis = _axises.at(0);
		for (auto i = 1; i < _axises.size(); i++)
		{
			if (temp_axis->vlines_coo.size() + temp_axis->hlines_coo.size() <
				_axises[i]->vlines_coo.size() + _axises[i]->hlines_coo.size())
			{
				temp_axis = _axises[i];
			}
		}
		_beamSet.setMember(temp_axis, _pillars);
		auto temp = _pillarSet.returnColumnBorderGroupIndex();
		_beamSet.findBeamLayer(temp, m_data->m_lineData);
		_beamSet.findBeam(*m_data/*, temp_index, _corners, temp_line, m_temp_index*/);
		_beamSet.setMFCPaintTestVec(temp_index, temp_line);
		_beamSet.beams(_beams, *m_data);
	}

	return true;
}


std::shared_ptr<Axis> Parser::getCompleteAxis()
{

	std::shared_ptr<Axis> temp_axis = _axises.at(0);
	for (auto i = 1; i < _axises.size(); i++)
	{
		if (temp_axis->vlines_coo.size() + temp_axis->hlines_coo.size() <
			_axises[i]->vlines_coo.size() + _axises[i]->hlines_coo.size())
		{
			temp_axis = _axises[i];
		}
	}
	return temp_axis;
}

bool Parser::findBeamSection()
{
	_beamSectionSet.setMember(_blocks, m_data->m_dimensions);
	_beamSectionSet.beamSections(*m_data, _beamSections);
	return true;
}

bool Parser::findBeamLongitudinal()
{
	_beamLongitudinalSet.setMember(_blocks);
	_beamLongitudinalSet.beamLongitudianls(m_data, _beamLongitudinals, temp_index, temp_line);
	return true;
}

bool Parser::findWallsSection()
{
	for (auto block : _blocks)
	{
		WallSectionSet wallSectionSet(m_data, block);
		wallSectionSet.iniDrawing(&temp_index_colorBlocks, &temp_line_colorBlocks);
		
		wallSectionSet.paserSectionData();
		auto wallVec = wallSectionSet.getWallSection();
		
		_spWallVec.insert(_spWallVec.end(),
			wallVec.begin(), wallVec.end());
	}

	return true;
}

bool Parser::SetBlockInsideInformation(const std::shared_ptr<Block>& block)
{
	auto axis = block->COMPONENT;
	std::vector<Point> candidates;

	double left = block->box.left;
	double right = block->box.right;
	int i = m_data->m_textPointData.findLowerBound(block->box.bottom);
	int j = m_data->m_textPointData.findUpperBound(block->box.top);

	//通过标注符号查找候选集
	for (i; i < j; ++i)
	{
		auto& point = m_data->m_textPointData.textpoints()[i];
		if (point.x > left - Precision && point.x < right + Precision)
		{
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			block->block_information.push_back(tt->text);
		}
	}
	return true;
}

bool Parser::findWallLocation()
{
	for (auto& block : _blocks)
	{
		WallLocationSet wallLocationSet;//组织墙的位置信息
		wallLocationSet.iniDrawing(&temp_index_colorBlocks, &temp_line_colorBlocks);
		auto wallLlocVec = wallLocationSet.returnWallLocation(m_data, block,this->_sectionSybmols);

		_spWallLcoationVec.insert(_spWallLcoationVec.end(),
								  wallLlocVec.begin(), wallLlocVec.end());

		/*temp_index.insert(temp_index.end(),
			wallLocationSet._wallLineIndex.begin(),
			wallLocationSet._wallLineIndex.end());*/
	}

	return true;
}

bool Parser::parseFloorPlan()
{
	for (auto spBlock : _blocks)
	{
		if (spBlock->bAxis)
		{
			FloorPlanParser floorPlanParser(m_data, spBlock);
			floorPlanParser.initDrawing(&temp_index_colorBlocks, &temp_line_colorBlocks);
			floorPlanParser.parse();
		}
	}
	return false;
}
