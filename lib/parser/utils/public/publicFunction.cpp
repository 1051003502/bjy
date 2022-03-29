#include "publicFunction.h"


void logLine(const Line& line)
{
	spdlog::get("all")->warn("line start pt({},{});end pt({},{})",
							 line.s.x, line.s.y,
							 line.e.x, line.e.y);
};

//todo 将log的打印函数单独移到一个头文件里
//void logPillar(const Pillar& pillar);


std::string getRandom16Chars()
{
	std::random_device rd;
	std::mt19937 g(rd());
	std::string str;
	std::uniform_int_distribution<int> dist(0x20, 0x7A);
	for (auto i = 0; i < 16; i++)
		str.append(std::to_string(dist(rd)));
	std::shuffle(str.begin(), str.end(), g);
	return str;
}

double strimDouble(double d, unsigned char bit)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(bit) << d;
	ss >> d;
	return d;
}

bool b_closedzone(const std::vector<int>& borderline, ClosedZone& temp_borderline, Data& data)
{
	std::vector<int> see_map(borderline.size());
	for (int i = 0; i < borderline.size(); ++i)
	{

		if (find(see_map.begin(), see_map.end(), borderline[i]) != see_map.end())
		{
			continue;
		}
		int start_index = borderline[i];
		std::queue<int> temp_queue;
		temp_queue.push(start_index);
		std::vector<std::pair<int, ClosedZone::Scope>> link;
		//see_map.push_back(start_index);
		while (!temp_queue.empty())
		{
			auto it = temp_queue.front();
			temp_queue.pop();
			Point s = data.m_lineData.lines()[it].s;
			Point e = data.m_lineData.lines()[it].e;
			auto corners = data.m_cornerData.corners().find(it);
			if (corners == data.m_cornerData.corners().end())
				return false;
			else
			{
				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == it)
					{
						l1 = corner.l2;
					}
					auto ite = find(borderline.begin(), borderline.end(), l1);
					if (ite != borderline.end())
					{
						if (ConnectPoint(s, corner))
						{
							if (find(see_map.begin(), see_map.end(), l1) == see_map.end())
							{
								temp_queue.push(l1);
								//see_map.push_back(l1);
							}

							Point s_p, d_p;
							if (ConnectPoint(data.m_lineData.lines()[l1].s, corner))
							{
								s_p = data.m_lineData.lines()[l1].s;
								d_p = data.m_lineData.lines()[l1].e;
							}
							else
							{
								s_p = data.m_lineData.lines()[l1].e;
								d_p = data.m_lineData.lines()[l1].s;
							}
							//求方向
							double a_it = (s.y - e.y);
							double b_it = (s.x - e.x);
							if (find(see_map.begin(), see_map.end(), it) == see_map.end())
							{
								if (data.m_lineData.lines()[it].vertical())
								{
									if (s_p.x < d_p.x)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::R));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::L));
									}
									see_map.push_back(it);
								}
								else if (data.m_lineData.lines()[it].horizontal())
								{

									if (s_p.y > d_p.y)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::D));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::U));
									}
									see_map.push_back(it);
								}
								else
								{
									if (s_p.x > d_p.x)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::D));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::U));
									}
									see_map.push_back(it);
								}
							}

							/*double a_l1 = (s_p.y - d_p.y);
							double b_l1 = (s_p.x - d_p.x);*/
						}
						else if (ConnectPoint(e, corner))
						{
							if (find(see_map.begin(), see_map.end(), l1) == see_map.end())
							{
								temp_queue.push(l1);
								//see_map.push_back(l1);
							}

							Point s_p, d_p;
							if (ConnectPoint(data.m_lineData.lines()[l1].s, corner))
							{
								s_p = data.m_lineData.lines()[l1].s;
								d_p = data.m_lineData.lines()[l1].e;
							}
							else
							{
								s_p = data.m_lineData.lines()[l1].e;
								d_p = data.m_lineData.lines()[l1].s;
							}
							//求方向
							double a_it = (s.y - e.y);
							double b_it = (s.x - e.x);
							if (find(see_map.begin(), see_map.end(), it) == see_map.end())
							{
								if (data.m_lineData.lines()[it].vertical())
								{
									if (s_p.x < d_p.x)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::R));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::L));
									}
									see_map.push_back(it);
								}
								else if (data.m_lineData.lines()[it].horizontal())
								{

									if (s_p.y > d_p.y)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::D));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::U));
									}

									see_map.push_back(it);
								}
								else
								{
									if (s_p.x > d_p.x)
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::D));
									}
									else
									{
										link.push_back(std::pair<int, ClosedZone::Scope>(it, ClosedZone::Scope::U));
									}

									see_map.push_back(it);
								}
							}
						}
					}
				}
			}
		}

		for (auto it : link)
		{
			temp_borderline.borderline.push_back(it);
		}
	}
	return true;
}

bool b_InClosedZone(int& candidate, const std::vector<int>& line__index, const ClosedZone& temp_borderline, Data& data)
{
	if (find(line__index.begin(), line__index.end(), candidate) != line__index.end())
	{
		return false;
	}
	bool mark = true;
	for (auto it : temp_borderline.borderline)
	{
		if (it.second == ClosedZone::Scope::R)
		{
			//temp_index.push_back(it.first);
			if (data.m_lineData.lines()[it.first].vertical() && data.m_lineData.lines()[candidate].s.x + Precision < data.m_lineData.lines()[it.first].s.x)
			{
				mark = false;
			}
		}
		else if (it.second == ClosedZone::Scope::L)
		{
			if (data.m_lineData.lines()[it.first].vertical() && data.m_lineData.lines()[candidate].s.x - Precision > data.m_lineData.lines()[it.first].s.x)
			{
				mark = false;
			}
		}
		else if (it.second == ClosedZone::Scope::U)
		{
			if (data.m_lineData.lines()[it.first].horizontal() && data.m_lineData.lines()[candidate].s.y + Precision < data.m_lineData.lines()[it.first].s.y)
			{
				mark = false;
			}
			else if (!data.m_lineData.lines()[it.first].horizontal() && !data.m_lineData.lines()[it.first].vertical())
			{
				//temp_index.push_back(it.first);
				double y1 = knowXToY(data.m_lineData.lines()[it.first], data.m_lineData.lines()[candidate].s.x);
				double y2 = knowXToY(data.m_lineData.lines()[it.first], data.m_lineData.lines()[candidate].e.x);
				if (data.m_lineData.lines()[candidate].s.y <= y1 || data.m_lineData.lines()[candidate].e.y <= y2)
				{
					mark = false;
				}
			}
		}
		else if (it.second == ClosedZone::Scope::D)
		{
			if (data.m_lineData.lines()[it.first].horizontal() && data.m_lineData.lines()[candidate].s.y - Precision > data.m_lineData.lines()[it.first].s.y)
			{
				mark = false;
			}
			else if (!data.m_lineData.lines()[it.first].horizontal() && !data.m_lineData.lines()[it.first].vertical())
			{
				double y1 = knowXToY(data.m_lineData.lines()[it.first], data.m_lineData.lines()[candidate].s.x);
				double y2 = knowXToY(data.m_lineData.lines()[it.first], data.m_lineData.lines()[candidate].e.x);
				if (data.m_lineData.lines()[candidate].s.y >= y1 || data.m_lineData.lines()[candidate].e.y >= y2)
				{
					mark = false;
				}
				else if (data.m_lineData.lines()[candidate].s.y + Precision > y1&& data.m_lineData.lines()[candidate].s.y - Precision < y1 && data.m_lineData.lines()[candidate].e.y + Precision > y2 && data.m_lineData.lines()[candidate].e.y - Precision < y2)
				{
					mark = false;
				}
			}
		}
	}
	if (mark == false)
	{
		//candidate_map[i] = -1;
		return false;
	}

	return true;
}

bool lineTypeVHS(Data& data, const std::vector<int>& index, std::vector<int>& v_lines, std::vector<int>& h_lines, std::vector<int>& s_lines)
{
	for (auto it : index)
	{
		if (data.m_lineData.lines()[it].vertical())
		{
			v_lines.push_back(it);
		}
		else if (data.m_lineData.lines()[it].horizontal())
		{
			h_lines.push_back(it);
		}
		else
		{
			s_lines.push_back(it);
		}
	}
	return true;
}

//将lines分类并将他们的索引分别存储
bool lineTypeVHSIndex(const std::vector<Line>& lines, std::vector<int>& v_lines, std::vector<int>& h_lines, std::vector<int>& s_lines)
{
	for (int i = 0; i < lines.size(); ++i)
	{
		if (lines[i].vertical())
		{
			v_lines.push_back(i);
		}
		else if (lines[i].horizontal())
		{
			h_lines.push_back(i);
		}
		else
		{
			s_lines.push_back(i);
		}
	}
	return true;
}

//将lines分类并将数据本身复制存储
bool lineTypeVHSEntity(const std::vector<Line>& lines, std::vector<Line>& v_lines, std::vector<Line>& h_lines, std::vector<Line>& s_lines)
{
	for (auto it : lines)
	{
		if (it.vertical())
		{
			v_lines.push_back(it);
		}
		else if (it.horizontal())
		{
			h_lines.push_back(it);
		}
		else
		{
			s_lines.push_back(it);
		}
	}
	return true;
}

bool lineTypeVHS(const std::vector<Line>& lines, const std::vector<int>& index, std::vector<int>& v_lines, std::vector<int>& h_lines, std::vector<int>& s_lines)
{
	for (auto it : index)
	{
		if (lines[it].vertical())
		{
			v_lines.push_back(it);
		}
		else if (lines[it].horizontal())
		{
			h_lines.push_back(it);
		}
		else
		{
			s_lines.push_back(it);
		}
	}
	return true;
}

bool bulidLinesCorner(const std::vector<Line>& lines, const std::vector<int>& vIndex,
					  const std::vector<int>& hIndex, const std::vector<int>& sIndex,
					  std::map<int, std::vector<Corner>>& corners)
{
	LineData temp_line;
	CornerData temp_corner;
	temp_line.setLines(lines);
	temp_line.setVLinesIndex(vIndex);
	temp_line.setHLinesIndex(hIndex);
	temp_line.setSLinesIndex(sIndex);

	for (auto v = vIndex.begin(); v != vIndex.end(); ++v)
	{
		const Line& lineTemp = lines[*v];
		double yb = lineTemp.s.y;
		double yu = lineTemp.e.y;

		int i = temp_line.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = temp_line.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);

		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			//if (j > 0)
			temp_corner.crossLinesVH(temp_line.lines(), *v, temp_line.hLinesIndices()[i]);
		}
	}

	for (auto l = sIndex.begin(); l != sIndex.end(); ++l)
	{
		const Line& lineTemp = lines[*l];
		double yb = std::min(lineTemp.s.y, lineTemp.e.y);
		double yu = std::max(lineTemp.s.y, lineTemp.e.y);
		int i = temp_line.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = temp_line.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);

		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			// if (j > 0)
			temp_corner.crossLinesS(temp_line.lines(), *l, temp_line.hLinesIndices()[i]);
		}
	}

	for (auto l = sIndex.begin(); l != sIndex.end(); ++l)
	{
		const Line& lineTemp = lines[*l];
		double yl = std::min(lineTemp.s.x, lineTemp.e.x);
		double yr = std::max(lineTemp.s.x, lineTemp.e.x);
		int i = temp_line.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yl, Precision, false);
		int j = temp_line.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yr, Precision, false);

		if (i == j && i == -1)
			continue;
		for (i; i <= j; ++i)
		{
			//if (j > 0)
			temp_corner.crossLinesS(temp_line.lines(), *l, temp_line.vLinesIndices()[i]);
		}

	}
	//斜线与斜线的相交关系
	for (int i = 0; i < sIndex.size() - 1; ++i)
	{
		for (int j = i + 1; j < sIndex.size(); ++j)
		{
			temp_corner.crossLinesSS(temp_line.lines(), temp_line.sLinesIndices()[i], temp_line.sLinesIndices()[j]);
		}
	}

	corners = temp_corner.corners();
	return true;
}

bool nearestLine(const std::vector<Line>& lines, const std::vector<int>& linesIndex, const Point& point, std::pair<int, int> goal_index)
{
	if (lines.empty() || linesIndex.empty())
		return true;
	auto referenceLine = lines[linesIndex.front()];
	if (referenceLine.horizontal())
	{
		double b_nearest = -1;
		double t_nearest = -1;
		for (auto it : linesIndex)
		{
			;//此函数未完成后续补全
		}
	}
	else if (referenceLine.vertical())
	{

	}
	else
	{
		;//斜线后续解决
	}

	return true;
}

bool del_char(std::string& str)
{
	std::string temp_str = str;
	str.clear();
	for (auto it : temp_str)
	{
		if (it != '\0')
		{
			str.push_back(it);
		}
	}
	return true;
}

bool pushRTreeLines(const Line& line, const int& index, RTreeData& rTree)
{
	double min[2]{};
	double max[2]{};
	min[0] = line.s.x < line.e.x ? line.s.x : line.e.x;
	min[1] = line.s.y < line.e.y ? line.s.y : line.e.y;
	max[0] = line.s.x > line.e.x ? line.s.x : line.e.x;
	max[1] = line.s.y > line.e.y ? line.s.y : line.e.y;
	rTree.insertSLines(min, max, index);
	return true;
}

int returnLineType(const Line& line)
{
	if (line.horizontal())
	{
		return 1;
	}
	else if (line.vertical())
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

bool bLineConnect(const Line& line1, const Line& line2)
{
	if (ConnectPoint(line1.s, line2.s) ||
		ConnectPoint(line1.s, line2.e) ||
		ConnectPoint(line1.e, line2.s) ||
		ConnectPoint(line1.e, line2.e))
	{
		return true;
	}
	else
	{
		return false;
	}
}





bool BoolBreakpoint(const Line& line, Data& data)
{
	//对箍筋断面的判断
	if (line.side)
	{
		auto ends1 = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, line.s), 1);
		auto e = data.m_endpointData.getEndpoint(ends1[0]);
		auto corners = data.m_cornerData.corners().find(e.index);
		bool mark = false;
		if (corners != data.m_cornerData.corners().end())
		{
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				auto l2 = corner.l2;
				int index = 0;
				if (e.index == l1)
					index = l2;
				else
					index = l1;
				if (data.m_lineData.lines()[index].isSide())
				{
					mark = true;
					break;
				}
			}
		}
		if (mark)
			return false;
		else
			return true;
	}
	return false;
}

bool SetBoxVHIndex(Box & box, LineData & lineData)
{
	box.hf = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom);
	box.ht = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.top);
	box.vf = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, box.left, Precision, false);
	box.vt = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, box.right, Precision, false);
	return true;
}

std::vector<int> returnRtreeIndex(const double min[], const double max[], RTreeData & goalRTree)
{
	int num = goalRTree.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
		&goalRTree, std::placeholders::_1));
	auto backIndex = goalRTree.SBackIndex();
	goalRTree.clearSBackIndex();

	return backIndex;
}

bool LinesSort(std::vector<Line>& lines, const char & ch)
{
	int left = 0;
	int right = lines.size() - 1;
	//if()
	QuickSortLine(lines, left, right, ch);
	return true;
}

bool QuickSortLine(std::vector<Line>& lines, int left, int right, const char & ch)
{
	if (left < right)
	{
		int base = DivisionLine(lines, left, right, ch);
		QuickSortLine(lines, left, base - 1, ch);
		QuickSortLine(lines, base + 1, right, ch);
	}
	return true;
}

int DivisionLine(std::vector<Line>& lines, int left, int right, const char & ch)
{
	Line base = lines[left];
	while (left < right)
	{
		if (ch == 'V')
		{
			while (left < right&&lines[right].s.x >= base.s.x)
				right--;
			lines[left] = lines[right];
			while (left < right&&lines[left].s.x <= base.s.x)
				left++;
			lines[right] = lines[left];
		}
		else if (ch == 'H')
		{
			while (left < right&&lines[right].s.y >= base.s.y)
				right--;
			lines[left] = lines[right];
			while (left < right&&lines[left].s.y <= base.s.y)
				left++;
			lines[right] = lines[left];
		}

	}
	lines[left] = base;
	return left;
}

bool LinesSort(std::vector<int>& linesIndex, const char & ch, LineData & _linedata)
{
	int left = 0;
	int right = linesIndex.size() - 1;
	//if()
	QuickSortLine(linesIndex, left, right, ch, _linedata);
	return true;
}

bool QuickSortLine(std::vector<int>& linesIndex, int left, int right, const char & ch, LineData & _linedata)
{
	if (left < right)
	{
		int base = DivisionLine(linesIndex, left, right, ch, _linedata);
		QuickSortLine(linesIndex, left, base - 1, ch, _linedata);
		QuickSortLine(linesIndex, base + 1, right, ch, _linedata);
	}
	return true;
}

int DivisionLine(std::vector<int>& linesIndex, int left, int right, const char & ch, LineData & _linedata)
{
	int base = linesIndex[left];
	while (left < right)
	{
		if (ch == 'V')
		{
			while (left < right&&_linedata.getLine(linesIndex[right]).s.x >= _linedata.getLine(base).s.x)
				right--;
			linesIndex[left] = linesIndex[right];
			while (left < right&&_linedata.getLine(linesIndex[left]).s.x <= _linedata.getLine(base).s.x)
				left++;
			linesIndex[right] = linesIndex[left];
		}
		else if (ch == 'H')
		{
			while (left < right&&_linedata.getLine(linesIndex[right]).s.y >= _linedata.getLine(base).s.y)
				right--;
			linesIndex[left] = linesIndex[right];
			while (left < right&&_linedata.getLine(linesIndex[left]).s.y <= _linedata.getLine(base).s.y)
				left++;
			linesIndex[right] = linesIndex[left];
		}

	}
	linesIndex[left] = base;
	return left;
}

bool MergeLines(std::vector<Line>& lines, const char & ch)
{
	std::vector<int>lines_map(lines.size());
	for (int i = 0; i < lines.size() - 1; ++i)
	{
		if (lines.empty())
			break;
		if (lines_map[i] == -1)
			continue;
		for (int j = 0; j < lines.size(); ++j)
		{
			if (i == j || (lines_map[i] == -1 || lines_map[j] == -1))
				continue;
			if (ch == 'V')
			{
				if (lines[i].s.x + Precision > lines[j].s.x &&
					lines[i].s.x - Precision < lines[j].s.x)
				{
					int num = lines[i].length() < lines[j].length() ? i : j;
					lines_map[num] = -1;
				}
				/*else
				{
					break;
				}*/
			}
			else if (ch == 'H')
			{
				if (lines[i].s.y + Precision > lines[j].s.y&&
					lines[i].s.y - Precision < lines[j].s.y)
				{
					int num = lines[i].length() < lines[j].length() ? i : j;
					lines_map[num] = -1;
				}
				/*else
				{
					break;
				}*/
			}
			else if (ch == 'S')
			{
				;//后续补全
			}

		}

	}
	std::vector<Line>temp_line;
	for (int i = 0; i < lines.size(); ++i)
	{
		if (lines_map[i] != -1)
		{
			temp_line.push_back(lines[i]);
		}
	}
	lines.clear();
	lines = temp_line;

	return true;
}

bool MergeLines(std::vector<int>& linesIndexVec, const char & ch, LineData & _lineData)
{

	std::vector<int>lines_map(linesIndexVec.size());
	for (int i = 0; i < linesIndexVec.size() - 1; ++i)
	{
		if (linesIndexVec.empty())
			break;
		if (lines_map[i] == -1)
			continue;
		for (int j = 0; j < linesIndexVec.size(); ++j)
		{
			if (i==j||(lines_map[i] == -1 || lines_map[j] == -1))
				continue;
			if (ch == 'V')
			{
				Line lineI = _lineData.getLine(linesIndexVec[i]);
				Line lineJ = _lineData.getLine(linesIndexVec[j]);
				if (lineI.s.x + Precision > lineJ.s.x&&
					lineI.s.x - Precision < lineJ.s.x)
				{
					int maxSIndex = lineI.s.y > lineJ.s.y ? i : j;
					int minEIndex = lineI.e.y < lineJ.e.y ? i : j;
					if (maxSIndex == minEIndex)
					{
						lines_map[maxSIndex] = -1;
					}

					/*int num = lines[i].length() < lines[j].length() ? i : j;
					lines_map[num] = -1;*/
				}
				/*else
				{
					break;
				}*/
			}
			else if (ch == 'H')
			{
				Line lineI = _lineData.getLine(linesIndexVec[i]);
				Line lineJ = _lineData.getLine(linesIndexVec[j]);

				if (lineI.s.y + Precision > lineJ.s.y&&
					lineI.s.y - Precision < lineJ.s.y)
				{
					int maxSIndex = lineI.s.x > lineJ.s.x ? i : j;
					int minEIndex = lineI.e.x < lineJ.e.x ? i : j;
					if (maxSIndex == minEIndex)
					{
						lines_map[maxSIndex] = -1;
					}
				}
				/*else
				{
					break;
				}*/
			}
			else if (ch == 'S')
			{
				;//后续补全
			}

		}

	}
	std::vector<int>temp_line;
	for (int i = 0; i < linesIndexVec.size(); ++i)
	{
		if (lines_map[i] != -1)
		{
			temp_line.push_back(linesIndexVec[i]);
		}
	}
	linesIndexVec.clear();
	linesIndexVec = temp_line;

	return true;
}


bool bPointBelongLineEnd(const Line & _line, const Point & point)
{
	if (ConnectPoint(_line.s, point) || ConnectPoint(_line.e, point))
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

bool bConnectTowLine(const Line & _line1, const Line & _line2)
{
	if (ConnectPoint(_line1.s, _line2.s) ||
		ConnectPoint(_line1.s, _line2.e) ||
		ConnectPoint(_line1.e, _line2.s) ||
		ConnectPoint(_line1.e, _line2.e))
	{
		return true;
	}
	else
	{
		return false;
	}
}


std::string numToString(const double& num)
{
	int num1 = ROUND(num);
	char str_int[30] = { 0 };
	sprintf(str_int, "%u", num1);
	return  str_int;
}

int findLineIndex(LineData& lineData, const Line& line, const char& ch)
{

	if (ch == 'H')
	{
		double yb = line.s.y - Precision;
		double yu = line.e.y + Precision;
		int i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);

		if (i == -1 || j == -1)
		{
			return -1;
		}

		for (int hI = i; hI <=j; ++hI)
		{
			const int& lineIndex = lineData.hLinesIndices()[hI];
			const Line& tempLine = lineData.getLine(lineIndex);

			if (tempLine.s.y + Precision > line.s.y&&
				tempLine.s.y - Precision < line.s.y&&
				/*tempLine.s.x >= line.s.x && 
				tempLine.e.x <= line.e.x && */
				tempLine.length() > line.length() * 0.7)
			{
				return lineIndex;
			}
		}
	}
	else if (ch == 'V')
	{
		double yb = line.s.x - Precision;
		double yu = line.e.x + Precision;
		int i = lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yb, Precision, false);
		int j = lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yu, Precision, false);

		if (i == -1 || j == -1)
		{
			return -1;
		}

		for (int VI = i; VI <=j; ++VI)
		{
			const int& lineIndex = lineData.vLinesIndices()[VI];
			const Line& tempLine = lineData.getLine(lineIndex);

			if (tempLine.length() > line.length() * 0.9&&
				tempLine.length()<line.length()*1.1)
			{
				return lineIndex;
			}
		}


	}
	else
	{

	}

	return -1;//查找失败返回-1
}


int findLineAdjacentIndex(LineData& lineData, const Line& line, const double& size, const char& type, const char& loc)
{
	int goalIndex = -1;//目标索引
	if (type == 'H')
	{
		double yb = line.s.y - size;
		double yu = line.e.y + size;
		int i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		if (i == -1 || j == -1)
		{
			return -1;
		}

		for (int hI = i; hI < j; ++hI)
		{

			const int& lineIndex = lineData.hLinesIndices()[hI];
			const Line& tempLine = lineData.getLine(lineIndex);

			double distance = 0.0;
			if (loc == 'T')
			{
				if (tempLine.s.x - Precision > line.e.x || 
					tempLine.e.x + Precision < line.s.x ||
					(tempLine.s.y - Precision < line.s.y/*&&
					tempLine.s.y + Precision > line.s.y*/))
				{
					continue;
				}

				auto length = std::abs(tempLine.s.y - line.s.y);
				if (goalIndex == -1)
				{
					goalIndex = lineIndex;
					distance = length;
				}
				else if (distance > length)
				{
					goalIndex = lineIndex;
					distance = length;
				}


			}
			else if (loc == 'B')
			{
				if (tempLine.s.x - Precision > line.e.x ||
					tempLine.e.x + Precision < line.s.x ||
					(/*tempLine.s.y - Precision < line.s.y &&*/
						tempLine.s.y + Precision > line.s.y))
				{
					continue;
				}

				auto length = std::abs(tempLine.s.y - line.s.y);
				if (goalIndex == -1)
				{
					goalIndex = lineIndex;
					distance = length;
				}
				else if (distance > length)
				{
					goalIndex = lineIndex;
					distance = length;
				}
			}
		}

	}
	else if (type == 'V')
	{
		double yb = line.s.x - size;
		double yu = line.e.x + size;
		int i = lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yb, Precision, false);
		int j = lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yu, Precision, false);

		if (i == -1 || j == -1)
		{
			return -1;
		}

		for (int VI = i; VI < j; ++VI)
		{
			const int& lineIndex = lineData.hLinesIndices()[VI];
			const Line& tempLine = lineData.getLine(lineIndex);

			double distance = 0.0;
			if (loc == 'R')
			{
				if (tempLine.s.y - Precision < line.e.y ||
					tempLine.e.y + Precision < line.s.y ||
					(tempLine.s.x - Precision < line.s.x &&
						tempLine.s.x + Precision > line.s.x))
				{
					continue;
				}

				auto length = std::abs(tempLine.s.x - line.s.x);
				if (goalIndex == -1)
				{
					goalIndex = lineIndex;
					distance = length;
				}
				else if (distance > length)
				{
					goalIndex = lineIndex;
					distance = length;
				}


			}
			else if (loc == 'L')
			{
				if (tempLine.s.y - Precision < line.e.y ||
					tempLine.e.y + Precision < line.s.y ||
					(tempLine.s.x - Precision < line.s.x &&
						tempLine.s.x + Precision > line.s.x))
				{
					continue;
				}

				auto length = std::abs(tempLine.s.x - line.s.x);
				if (goalIndex == -1)
				{
					goalIndex = lineIndex;
					distance = length;
				}
				else if (distance > length)
				{
					goalIndex = lineIndex;
					distance = length;
				}
			}
		}

	}
	else
	{
		;
	}
	return goalIndex;
}





std::string GbkToUtf8(const std::string& __strGbk)
{
	DRW_ExtConverter conv("GB2312");
	return conv.toUtf8(const_cast<std::string*>(&__strGbk));
}

std::string Utf8ToGbk(const std::string& __strUtf8)
{
	DRW_ExtConverter conv("UTF-8");
	return conv.toUtf8(const_cast<std::string*>(&__strUtf8));
}
namespace publicFunction
{
	int findPointBaseLine(Data& data, const Point& p)
	{
		auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(p.entity);
		double autoHeight = dTPoint->autoHeight;
		autoHeight = 200;//!!!!!!!!!!!这里写成固定值  因为release版本无法获取autoheight 
		auto& vLinesIndices = data.m_lineData.vLinesIndices();
		auto& hLinesIndices = data.m_lineData.hLinesIndices();
		auto& lineData = data.m_lineData.lines();
		if (p.getDirection() == 1)
		{
			double textX = p.x + autoHeight * dTPoint->text.length() / 3;
			double textY = p.y + autoHeight;
			int lowerIndex = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, textY - autoHeight * 3, Precision, true);
			int upperIndex = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, textY + autoHeight * 3, Precision, true);
			int resLineIndex = -1;
			double distance;
			for (int index = lowerIndex;index <= upperIndex;index++)
			{
				int realIndex = hLinesIndices[index];
				const Line& line = lineData[realIndex];
				if (line.isDimension())continue;
				if (std::min(textX, line.e.x) - std::max(p.x, line.s.x) > 0)
				{
					if (resLineIndex == -1)
					{
						resLineIndex = realIndex;
						distance = std::abs(p.y - line.s.y);
					}
					else if (std::abs(p.y - line.s.y) < distance)
					{
						distance = std::abs(p.y - line.s.y);
						resLineIndex = realIndex;
					}

				}

			}
			return resLineIndex;
		}
		else if (p.getDirection() == 2)
		{
			double textX = p.x - autoHeight;
			double textY = p.y + autoHeight * dTPoint->text.length() / 3;
			int lowerIndex = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, textX - autoHeight * 2, Precision, false);
			int upperIndex = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, textX + autoHeight * 2, Precision, false);
			int resLineIndex = -1;
			double distance;
			for (int index = lowerIndex;index <= upperIndex;index++)
			{
				int realIndex = vLinesIndices[index];
				const Line& line = lineData[realIndex];
				if (line.isDimension())continue;
				if (std::min(line.e.y, textY) - std::max(p.y, line.s.y) > 0)
				{
					if (resLineIndex == -1)
					{
						resLineIndex = realIndex;
						distance = std::abs(p.x - line.s.x);
					}
					else if (std::abs(p.x - line.s.x) < distance)
					{
						distance = std::abs(p.x - line.s.x);
						resLineIndex = realIndex;
					}
				}

			}
			return resLineIndex;
		}
		return -1;
	}



	std::vector<int> findAllCrossingLine(Data& data, int lineIndex)
	{
		std::vector<int> resLineIndexVec;
		auto corners = data.m_cornerData.corners();
		if (corners.count(lineIndex) == 0)return resLineIndexVec;
		const auto& cornerVec = corners[lineIndex];
		for (auto& corner : cornerVec)
		{
			int crossLineIndex = getCrossLineIndexFromCorner(lineIndex, corner);
			resLineIndexVec.push_back(crossLineIndex);
		}
		return resLineIndexVec;
	}

	std::vector<int> findCrossing2(Data& data, int lineIndex)
	{
		std::vector<int> resLineIndexVec;
		auto corners = data.m_cornerData.corners();
		if (corners.count(lineIndex) == 0)return resLineIndexVec;
		const auto& lineData = data.m_lineData.lines();
		const auto& cornerVec = corners[lineIndex];
		const Line& line = lineData.at(lineIndex);
		for (auto& corner : cornerVec)
		{
			if (line.checkEndPoint(corner))
			{
				int crossLineIndex = getCrossLineIndexFromCorner(lineIndex, corner);
				const Line& crossLine = lineData[crossLineIndex];
				if (crossLine.checkEndPoint(corner))
				{
					resLineIndexVec.push_back(crossLineIndex);

				}

			}
		}
		return resLineIndexVec;
	}
	//说明：返回一跟线的交线，需满足1交点是两线端点 2两线分别是水平、竖直线
	std::vector<int> findCrossing3(Data& data, int lineIndex1)
	{
		std::vector<int> resultLineIndexVec;
		const auto& lineData = data.m_lineData.lines();
		auto corners = data.m_cornerData.corners();
		const auto& cornerVec = corners[lineIndex1];
		const Line& line1 = lineData.at(lineIndex1);
		if (returnLineType(line1) == 3)return resultLineIndexVec;
		for (const auto& corner : cornerVec)
		{
			if (line1.checkEndPoint(corner))
			{
				int crossLineIndex = getCrossLineIndexFromCorner(lineIndex1, corner);
				const Line& crossLine = lineData[crossLineIndex];
				if (crossLine.checkEndPoint(corner))
				{
					if (line1.vertical() && crossLine.horizontal() || line1.horizontal() && crossLine.vertical())
					{
						resultLineIndexVec.push_back(crossLineIndex);
						continue;
					}
				}
			}

		}
		return resultLineIndexVec;
	}

	std::string findMaxAmountLayer(std::vector<int> lineIndices, Data& data)
	{
		if (lineIndices.empty() || data.m_lineData.lines().empty())return "";
		auto& lineData = data.m_lineData.lines();
		std::map<std::string, int> layerCount;
		for (auto index : lineIndices)
		{
			Line& line = lineData[index];
			layerCount[line.getLayer()]++;
		}
		std::pair<std::string, int> maxPair("", -1);
		for (auto& pair : layerCount)
		{
			if (pair.second > maxPair.second)
			{
				maxPair = pair;
			}
		}
		return maxPair.first;
	}

	std::vector<Line> generateBoxBorder(const Box& box)
	{
		return box.getBorder();
		/*std::vector<Line> border;
		Line lineLeft(Point(box.left, box.bottom), Point(box.left, box.top));
		Line lineRight(Point(box.right, box.bottom), Point(box.right, box.top));
		Line lineBottom(Point(box.left, box.bottom), Point(box.right, box.bottom));
		Line lineTop(Point(box.left, box.top), Point(box.right, box.top));
		border.push_back(lineLeft);
		border.push_back(lineRight);
		border.push_back(lineBottom);
		border.push_back(lineTop);
		return border;*/
	}
	bool checkCrossingAndGetCorner(const Line& line1, const Line& line2, Corner& corner)
	{
		int type1 = returnLineType(line1);
		int type2 = returnLineType(line2);
		if (type1 == 1)
		{
			if (type2 == 1)
			{
				return false;
			}
			else if (type2 == 2)
			{
				return crossLineVH(line2, line1, corner);
			}
			else if (type2 == 3)
			{
				return crossLineS(line2, line1,corner);
			}
		}
		if (type1 == 2)
		{
			if (type2 == 1)
			{
				return crossLineVH(line1, line2, corner);
			}
			else if (type2 == 2)
			{
				return false;
			}
			else if (type2 == 3)
			{
				return crossLineS(line2, line1, corner);
			}
		}
		if (type1 == 3)
		{
			if (type2 == 1)
			{
				return crossLineS(line1, line2, corner);
			}
			else if (type2 == 2)
			{
				return crossLineS(line1, line2, corner);
			}
			else if (type2 == 3)
			{
				return crossLineSS(line1, line2, corner);
			}
		}
		//应该不会运行至此处
		return false;
	}
	double calculateParallelDistance(Line& l1, Line& l2)
	{
		if (l1.horizontal() && l2.horizontal())
		{
			return std::abs(l1.s.y - l2.s.y);
		}
		else if (l1.vertical() && l2.vertical()) 
		{
			return std::abs(l1.s.x - l2.s.x);
		}
		else 
		{
			return slantingDistanceLine(l1, l2);
		}
	}
	double twoLineCrossLength(const Line& l1, const Line& l2)
	{
		if (l1.horizontal() && l2.horizontal()) {
			int maxSX = std::max(l1.s.x, l2.s.x);
			int minEX = std::min(l1.e.x, l2.e.x);
			return minEX - maxSX < 0 ? 0 : minEX - maxSX;
		}
		else if (l1.vertical() && l2.vertical()) {
			int maxSY = std::max(l1.s.y, l2.s.y);
			int minEY = std::min(l1.e.y, l2.e.y);
			return minEY - maxSY < 0 ? 0 : minEY - maxSY;
		}
		else
		{
			int maxSX = std::max(l1.s.x, l2.s.x);
			int minEX = std::min(l1.e.x, l2.e.x);
			return minEX - maxSX < 0 ? 0 : minEX - maxSX;

		}
		//return 0;
	}
	double calculateTwoLineCrossRatio(const Line& l1, const Line& l2)
	{
		double crossLength = publicFunction::twoLineCrossLength(l1, l2);
		double minLength = (std::min)(l1.length(), l2.length());
		if (minLength < Precision)
		{
			return 0.0;
		}
		else
		{
			double ratio = crossLength / minLength;
			return ratio;
		}
	}
	int roundToInt(double data)
	{
		return (int)(data+0.5);
	}
	Line findMatchLine(std::vector<Line> lineFamily, Line lonelyLine)
	{
		double ratio_short = 0.4;
		std::vector<Line> matchedLineVec;
		for (auto line : lineFamily)
		{
			if (line == lonelyLine)continue;
			if (line.parallelTo(lonelyLine))
			{
				if (calculateTwoLineCrossRatio(line, lonelyLine) > ratio_short)
				{
					matchedLineVec.push_back(line);
				}
			}
		}
		Line nearestLine = Line::getEmptyLine();
		for (auto line : matchedLineVec)
		{
			if (nearestLine.isEmpty())
			{
				nearestLine = line;
			}
			else if (calculateParallelDistance(line, lonelyLine) < calculateParallelDistance(nearestLine, lonelyLine))
			{
				nearestLine = line;
			}
		}
		return nearestLine;
	}
	int findMatchLineIndex(Data& data, std::vector<int> indices, int lonelyIndex)
	{
		auto& lineData = data.m_lineData.lines();
		if (lineData.size() <= lonelyIndex)return -1;
		Line lonelyLine = lineData[lonelyIndex];
		double ratio_short = 0.4;
		std::vector<int> matchedIndices;
		for (auto index : indices)
		{
			if (index==lonelyIndex)continue;
			Line line = lineData[index];
			if (line.parallelTo(lonelyLine))
			{
				if (calculateTwoLineCrossRatio(line, lonelyLine) > ratio_short)
				{
					matchedIndices.push_back(index);
				}
			}
		}

		int nearestLineIndex = -1;
		for (auto index : matchedIndices)
		{
			Line line = lineData[index];
			if (nearestLineIndex=-1)
			{
				nearestLineIndex = index;
			}
			else if (calculateParallelDistance(line, lonelyLine) < calculateParallelDistance(lineData[nearestLineIndex], lonelyLine))
			{
				nearestLineIndex = index;
			}
		}
		return nearestLineIndex;
	}
}

