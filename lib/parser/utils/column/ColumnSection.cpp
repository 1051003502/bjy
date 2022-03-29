#include "ColumnSection.h"

ColumnSection::ColumnSection(std::vector<BreakPoint> temp_bp, std::vector<Dimension> dimensions)
{
	section_id = -1;
	this->break_point = temp_bp;
	this->dimensions = dimensions;
}

bool ColumnSection::SetBreakPoint(BreakPoint & bp, const Point & p, const double & r)
{
	bp.center_mind = p;
	bp.radius = r;
	return true;
}

double ColumnSection::GetPillarWidth()
{

	return 0.0;
}

bool ColumnSection::SetSectionWidthHeight()
{

	std::string str1;
	std::string str2;
	if (scale != "")
	{
		auto num = scale.find(":");
		str1 = scale.substr(0, num);
		str2 = scale.substr(num + 1, scale.size());
		linear_ration = double(atoi(str1.c_str())) / double(atoi(str2.c_str())) * 100;
	}
	else
	{
		linear_ration = 1;
	}
	Point l_point, r_point, t_point, b_point;
	l_point = Point(section_box.right, 0);//初始化
	r_point = Point(section_box.left, 0);
	t_point = Point(0, section_box.bottom);//初始化
	b_point = Point(0, section_box.top);

	for (auto it : this->dimensions)
	{
		//虽然目前这样整分没有问题，但是总会有隐患需要用准确的角度描述才是最标准的方式

		if (it.direction == 0)//右
		{
			for (auto it1 : it.lines)
			{
				if (it1.e.y > t_point.y)
				{
					t_point = it1.e;
				}
				if (it1.s.y < b_point.y)
				{
					b_point = it1.s;
				}
			}
			r_height.push_back(Dim_Information(it.direction, it.measurement / linear_ration));

		}
		else if (it.direction == 90)//上
		{

			for (auto it1 : it.lines)
			{
				if (it1.e.x > r_point.x)
				{
					r_point = it1.e;
				}
				if (it1.s.x < l_point.x)
				{
					l_point = it1.s;
				}
			}
			d_width.push_back(Dim_Information(it.direction, it.measurement / linear_ration));

		}
		else if (it.direction == 180)//左
		{

			for (auto it1 : it.lines)
			{
				if (it1.e.y > t_point.y)
				{
					t_point = it1.e;
				}
				if (it1.s.y < b_point.y)
				{
					b_point = it1.s;
				}
			}
			l_height.push_back(Dim_Information(it.direction, it.measurement / linear_ration));

		}
		else if (it.direction == 270)//下
		{

			for (auto it1 : it.lines)
			{
				if (it1.e.x > r_point.x)
				{
					r_point = it1.e;
				}
				if (it1.s.x < l_point.x)
				{
					l_point = it1.s;
				}
			}
			d_width.push_back(Dim_Information(it.direction, it.measurement / linear_ration));

		}

	}
	this->all_section_height = (ROUND(t_point.y) - ROUND(b_point.y)) / linear_ration;
	this->all_section_width = (ROUND(r_point.x) - ROUND(l_point.x)) / linear_ration;

	return true;
}

bool ColumnSection::SetSectionbox()
{
	for (auto it : break_point)
	{
		this->section_box.expand(Point(it.center_mind.x + it.radius, it.center_mind.y));
		this->section_box.expand(Point(it.center_mind.x - it.radius, it.center_mind.y));
		this->section_box.expand(Point(it.center_mind.x, it.center_mind.y + it.radius));
		this->section_box.expand(Point(it.center_mind.x, it.center_mind.y - it.radius));
	}
	return true;
}

bool ColumnSection::SetSectionOverallWH()
{
	for (auto it : u_width)
		return true;
}


bool ColumnSection::SetSectionBlockName(const Block & section_block)
{

	this->name = section_block.name;
	std::hash<std::string> stringHasher;
	this->section_id = stringHasher(this->name);
	return true;
}

bool ColumnSection::SetSectionBlockBox(const Block & section_block)
{
	this->section_block_box = section_block.box;
	return true;
}

//bool ColumnSection::MergeLines(std::vector<Line>& lines, const char & ch)
//{
//	std::vector<int>lines_map(lines.size());
//	for (int i = 0; i < lines.size() - 1; ++i)
//	{
//		if (lines.empty())
//			break;
//		if (lines_map[i] == -1)
//			continue;
//		for (int j = i + 1; j < lines.size(); ++j)
//		{
//			if (lines_map[i] == -1 || lines_map[j] == -1)
//				continue;
//			if (ch == 'V')
//			{
//				if (lines[i].s.x + Precision > lines[j].s.x&&
//					lines[i].s.x - Precision < lines[j].s.x)
//				{
//					int num = lines[i].length() < lines[j].length() ? i : j;
//					lines_map[num] = -1;
//				}
//				else
//				{
//					break;
//				}
//			}
//			else if (ch == 'H')
//			{
//				if (lines[i].s.y + Precision > lines[j].s.y&&
//					lines[i].s.y - Precision < lines[j].s.y)
//				{
//					int num = lines[i].length() < lines[j].length() ? i : j;
//					lines_map[num] = -1;
//				}
//				else
//				{
//					break;
//				}
//			}
//			else if (ch == 'S')
//			{
//				;//后续补全
//			}
//
//		}
//
//	}
//	std::vector<Line>temp_line;
//	for (int i = 0; i < lines.size(); ++i)
//	{
//		if (lines_map[i] != -1)
//		{
//			temp_line.push_back(lines[i]);
//		}
//	}
//	lines.clear();
//	lines = temp_line;
//
//	return true;
//}

//bool ColumnSection::LinesSort(std::vector<Line>& lines, const char & ch)
//{
//	int left = 0;
//	int right = lines.size() - 1;
//	//if()
//	QuickSortLine(lines, left, right, ch);
//	return true;
//}

bool ColumnSection::QuickSortLine(std::vector<Line>& lines, int left, int right, const char & ch)
{
	if (left < right)
	{
		int base = DivisionLine(lines, left, right, ch);
		QuickSortLine(lines, left, base - 1, ch);
		QuickSortLine(lines, base + 1, right, ch);
	}
	return true;
}

int ColumnSection::DivisionLine(std::vector<Line>& lines, int left, int right, const char & ch)
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

bool ColumnSection::sortExplanation(std::vector<std::pair<Explanation, Point>>& explanation)
{
	int temp_explantion = explanation.size();
	std::vector<std::pair<Explanation, Point>>s_t_bX = explanation;//由小到大x
	std::vector<std::pair<Explanation, Point>>s_t_bY = explanation;//由小到大Y
	explanation.clear();
	for (int i = 0; i < temp_explantion; ++i)
	{
		std::pair<Explanation, Point> temp;
		for (int j = 0; j < temp_explantion - 1 - i; ++j)
		{
			if (s_t_bX[j].second.x > s_t_bX[j + 1].second.x)
			{
				temp = s_t_bX[j];
				s_t_bX[j] = s_t_bX[j + 1];
				s_t_bX[j + 1] = temp;
			}
			if (s_t_bY[j].second.y > s_t_bY[j + 1].second.y)
			{
				temp = s_t_bY[j];
				s_t_bY[j] = s_t_bY[j + 1];
				s_t_bY[j + 1] = temp;
			}
		}
	}
	int mid = temp_explantion / 2;
	int markY_index = 0;
	for (int i = 0; i < temp_explantion; ++i)
	{
		if (s_t_bY[i].second == s_t_bX[mid].second)
		{
			markY_index = i;
			break;
		}
	}
	explanation.push_back(s_t_bX[mid]);
	for (int i = markY_index + 1; markY_index < temp_explantion && i < temp_explantion; ++i)
	{
		explanation.push_back(s_t_bY[i]);
	}
	for (int i = markY_index - 1; markY_index > 0 && i >= 0; --i)
	{
		explanation.push_back(s_t_bY[i]);
	}

	return true;
}

long ColumnSection::setSectionId(const std::string & sectionName)
{

	return 0;
}

//long ColumnSection::setSectionId(const std::string& sectionName)
//{
//	std::hash<std::string>s_id;//section id
//	
//	return s_id(sectionName);
//}

bool ColumnSection::boolRectangle(bool *type_L)
{
	*type_L = false;
	std::vector<int>vl;
	std::vector<int>sl;
	std::vector<int>hl;
	line_data.lineTypeVHS(this->bp_surround, vl, hl, sl);
	if (!sl.empty())
	{
		//return "special_shaped";
		*type_L = false;
		return false;
	}
	else
	{

		line_data.mergeLinesIndex(vl, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
		line_data.mergeLinesIndex(hl, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
		if (vl.size() > 2 || hl.size() > 2)
		{
			//return "special_shaped";
			*type_L = true;
			return false;
		}
		else
		{
			//return "rectangle";
			*type_L = false;
			return true;
		}

	}

}

std::vector<Point> ColumnSection::sortSurroundLine()
{
	std::vector<Point>temp_point;
	for (auto it : bp_surround)
	{
		temp_point.push_back(line_data.lines()[it].s);
		temp_point.push_back(line_data.lines()[it].e);
	}

	for (int i = 0; i < temp_point.size(); ++i)
	{
		Point temp(0, 0);
		for (int j = i; j < temp_point.size() - 1; ++j)
		{
			if (temp_point[j].y > temp_point[j + 1].y)
			{
				temp = temp_point[j + 1];
				temp_point[j + 1] = temp_point[j];
				temp_point[j] = temp;
			}
			else if (temp_point[j].y == temp_point[j + 1].y)
			{
				if (temp_point[j].x > temp_point[j + 1].x)
				{
					temp = temp_point[j + 1];
					temp_point[j + 1] = temp_point[j];
					temp_point[j] = temp;
				}

			}
		}
	}

	return temp_point;
}

bool ColumnSection::DescribeShape(std::string &str, const bool & type_L)
{
	std::vector<Point>temp_point = sortSurroundLine();
	if (type_L)
	{

	}
	else
	{

	}
	return true;
}

//此函数是在已知柱断面为矩形时，使用的
bool ColumnSection::setRectangleBH(Json::Value& Bars, const bool&collective)
{
	std::vector<int>vl;
	std::vector<int>sl;
	std::vector<int>hl;
	line_data.lineTypeVHS(this->bp_surround, vl, hl, sl);
	line_data.mergeLinesIndex(vl, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	line_data.mergeLinesIndex(hl, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	//默认矩形柱两边的纵筋信息都是一样的
	bool b = false, h = false;
	auto corner_b = corner_data.corners().find(hl.front());
	auto corner_h = corner_data.corners().find(vl.front());
	if (!collective)
	{
		for (auto it : explanation)
		{
			for (auto it1 : it.first.second)
			{
				for (auto c_b : corner_b->second)
				{
					auto l1 = c_b.l1;
					if (l1 == corner_b->first)
					{
						l1 = c_b.l2;
					}
					if (l1 == it1)
					{
						b = true;
						Bars["bSideBar"] = it.first.first;
						break;
					}
				}
				for (auto c_h : corner_h->second)
				{
					auto l1 = c_h.l1;
					if (l1 == corner_h->first)
					{
						l1 = c_h.l2;
					}
					if (l1 == it1)
					{
						h = true;
						Bars["hSideBar"] = it.first.first;
						break;
					}
				}
			}
			if (b&&h)
			{
				break;
			}
		}
	}
	else
	{
		for (auto it : bp_string)
		{
			if (it.second == Direcction::L || it.second == Direcction::R)
			{
				Bars["hSideBar"] = it.first;
				h = true;
			}
			else if (it.second == Direcction::D || it.second == Direcction::U)
			{
				Bars["bSideBar"] = it.first;
				b = true;
			}
			if (b&&h)
			{
				break;
			}
		}
	}


	return true;
}

bool ColumnSection::setRectangleBH(std::string & bSide, std::string & hSide, const bool & type_l)
{
	std::vector<int>vl;
	std::vector<int>sl;
	std::vector<int>hl;
	line_data.lineTypeVHS(this->bp_surround, vl, hl, sl);
	line_data.mergeLinesIndex(vl, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	line_data.mergeLinesIndex(hl, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	//默认矩形柱两边的纵筋信息都是一样的
	bool b = false, h = false;
	auto corner_b = corner_data.corners().find(hl.front());
	auto corner_h = corner_data.corners().find(vl.front());
	if (!type_l)
	{
		for (auto it : explanation)
		{
			for (auto it1 : it.first.second)
			{
				for (auto c_b : corner_b->second)
				{
					auto l1 = c_b.l1;
					if (l1 == corner_b->first)
					{
						l1 = c_b.l2;
					}
					if (l1 == it1)
					{
						b = true;
						bSide = it.first.first;
						break;
					}
				}
				for (auto c_h : corner_h->second)
				{
					auto l1 = c_h.l1;
					if (l1 == corner_h->first)
					{
						l1 = c_h.l2;
					}
					if (l1 == it1)
					{
						h = true;
						hSide = it.first.first;
						break;
					}
				}
			}
			if (b&&h)
			{
				break;
			}
		}
	}
	else
	{
		for (auto it : bp_string)
		{
			if (it.second == Direcction::L || it.second == Direcction::R)
			{
				hSide = it.first;
				h = true;
			}
			else if (it.second == Direcction::D || it.second == Direcction::U)
			{
				bSide = it.first;
				b = true;
			}
			if (b&&h)
			{
				break;
			}
		}
	}

	return true;
}

bool ColumnSection::setStirrupHookCount(int & hCount, int & vCount)
{
	std::vector<Line>h_lines;
	std::vector<Line>v_lines;
	std::vector<Line>s_lines;

	for (auto it : stittups)
	{
		for (auto it1 : it.circle_line)
			if (it1.horizontal())
			{
				h_lines.push_back(it1);
			}
			else if (it1.vertical())
			{
				v_lines.push_back(it1);
			}
	}
	LinesSort(h_lines, 'H');
	LinesSort(v_lines, 'V');
	MergeLines(h_lines, 'H');
	MergeLines(v_lines, 'V');
	hCount = h_lines.size();
	vCount = v_lines.size();

	return true;
}

bool ColumnSection::SetListStittups()
{
	list_stittups.clear();
	std::string st = this->name;
	for (auto it : stittups)
	{
		std::string str;
		if (it.single)
		{
			double length = 0.0;

			if (it.circle_line[0].horizontal())
			{
				double r_w = 0.0, r_h = 0.0;//r removing 距离
				length = it.circle_line[0].length() / linear_ration;
				r_w = (it.circle_line[0].s.x - stirrup_refer_point.x) / linear_ration;
				r_h = (it.circle_line[0].s.y - stirrup_refer_point.y) / linear_ration;
				str.clear();
				str = numToString(length) + "* 0" + " (single)";
				str += "(" + numToString(r_w) + "," + numToString(r_h) + ")";
				list_stittups.push_back(str);
				str.clear();
			}
			else
			{
				double r_w = 0.0, r_h = 0.0;//r removing 距离
				length = it.circle_line[0].length();
				r_w = (it.circle_line[0].s.x - stirrup_refer_point.x) / linear_ration;
				r_h = (it.circle_line[0].s.y - stirrup_refer_point.y) / linear_ration;
				str.clear();
				str = "0 *" + numToString(length) + " (single)";
				str += "(" + numToString(r_w) + "," + numToString(r_h) + ")";
				list_stittups.push_back(str);
				str.clear();
			}
		}
		else
		{
			std::vector<Line>h_lines;
			std::vector<Line>v_lines;
			for (auto it1 : it.circle_line)
			{
				if (it1.horizontal())
				{
					h_lines.push_back(it1);
				}
				else if (it1.vertical())
				{
					v_lines.push_back(it1);
				}
			}

			LinesSort(v_lines, 'V');
			LinesSort(h_lines, 'H');
			MergeLines(v_lines, 'V');
			MergeLines(h_lines, 'H');
			if (v_lines.size() < 2 || h_lines.size() < 2)
			{
				list_stittups.push_back("此处存在问题");
				break;
			}
			double w = abs(v_lines.front().s.x - v_lines.back().s.x);
			double h = abs(h_lines.front().s.y - h_lines.back().s.y);
			Line left = v_lines[0].s.x < v_lines[1].s.x ? v_lines[0] : v_lines[1];
			Line right = h_lines[0].s.y < h_lines[1].s.y ? h_lines[0] : h_lines[1];
			double r_w = left.s.x - stirrup_refer_point.x;
			double r_h = left.s.y - stirrup_refer_point.y;
			str = numToString(w);
			str += "*" + numToString(h);
			str += "(" + numToString(r_w) + "," + numToString(r_h) + ")";
			list_stittups.push_back(str);
		}

	}
	return true;
}

bool ColumnSection::del_char(std::string & str)
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

//std::string ColumnSection::numToString(const double & num)
//{
//	int num1 = ROUND(num);
//	char str_int[30] = { 0 };
//	sprintf(str_int, "%u", num1);
//	return  str_int;
//}

Json::Value ColumnSection::OutPut()
{
	//Json::Value Information;
	if (l_t_c.empty())
	{
		return OutPutCenter();
	}
	else
	{
		return OutPutInSitu();
	}
	//return true;
}

Json::Value ColumnSection::OutPutInSitu()
{
	Json::Value information;
	std::string str;
	information["uuid"] = this->section_id;
	information["name"] = this->name;
	information["scale"] = this->scale;

	//
	str.clear();
	str = numToString(this->all_section_width);
	str += '*' + numToString(this->all_section_height);

	//形状描述
	str.clear();
	Json::Value shape;
	bool b_typeL;
	if (boolRectangle(&b_typeL))
	{
		shape["Style"] = "rectangle";
		str = numToString(all_section_width) + "*" + numToString(all_section_height);
		shape["distributed"] = str;
	}
	else
	{
		/*shape["Style"] = "special_shaped";
		if (b_typeL)
		{
		DescribeShape(str, b_typeL);
		}
		else
		{
		DescribeShape(str, b_typeL);
		}*/

		shape["distributed"] = "";//形状描述
	}


	information["shape"] = shape;

	//纵筋信息									  
	str.clear();
	sortExplanation(explanation);
	for (auto it : explanation)
	{
		str += "|" + it.first.first;
	}
	//全部纵筋  只有矩形柱存在b侧、h侧
	Json::Value Bars;
	Bars["all"] = str;
	if (boolRectangle(&b_typeL))
	{
		Bars["cornerBars"] = "";
		setRectangleBH(Bars, false);
	}
	else
	{
		Bars["cornerBars"] = "";
		Bars["bSideBar"] = "";
		Bars["hSideBar"] = "";
	}

	information["longitudinalBars"] = Bars;

	//箍筋信息
	str.clear();
	Json::Value stirrup;
	for (auto it : stirrup_model)
	{
		str += "|" + it.first;
	}
	stirrup["all"] = str;

	//
	std::vector<Line>h_lines;
	std::vector<Line>v_lines;
	std::vector<Line>s_lines;

	for (auto it : stittups)
	{
		for (auto it1 : it.circle_line)
			if (it1.horizontal())
			{
				h_lines.push_back(it1);
			}
			else if (it1.vertical())
			{
				v_lines.push_back(it1);
			}
	}
	LinesSort(h_lines, 'H');
	LinesSort(v_lines, 'V');
	MergeLines(h_lines, 'H');
	MergeLines(v_lines, 'V');
	str.clear();
	str = numToString(h_lines.size());
	str += "*" + numToString(v_lines.size());
	stirrup["count"] = str;//支数

						   //排列方式
	SetListStittups();
	char ch_temp = '0';
	//std::string ch = "0";
	int count = 0;
	char str_int[30];
	Json::Value stirrup_list;
	for (auto it : list_stittups)
	{

		stirrup_list[numToString(count)] = it;
		count++;
	}
	stirrup["configuration "] = stirrup_list;
	information["stirrups"] = stirrup;
	//others
	str.clear();
	information["others"] = str;
	return information;
}

Json::Value ColumnSection::OutPutCenter()
{
	Json::Value information;

	std::string str;
	information["uuid"] = this->section_id;
	information["name"] = this->name;
	information["scale"] = this->scale;

	//
	str.clear();
	//形状描述
	Json::Value shape;
	bool b_typeL;
	if (boolRectangle(&b_typeL))
	{
		shape["Style"] = "rectangle";
		str = numToString(all_section_width) + "*" + numToString(all_section_height);
	}
	else
	{
		shape["Style"] = "special_shaped";
		if (b_typeL)
		{

		}
		else
		{

		}
	}
	information["shape"] = shape;

	//纵筋信息
	str.clear();
	for (auto it : bp_string)
	{
		str += "|" + it.first;
		if (it.second == Direcction::L)
		{
			std::string str1 = "left";
			str += "(" + str1 + ")";
		}
		else if (it.second == Direcction::R)
		{
			std::string str1 = "right";
			str += "(" + str1 + ")";
		}
		else if (it.second == Direcction::D)
		{
			std::string str1 = "down";
			str += "(" + str1 + ")";
		}
		else if (it.second == Direcction::U)
		{
			std::string str1 = "up";
			str += "(" + str1 + ")";
		}

	}
	//全部纵筋
	Json::Value Bars;
	Bars["all"] = str;

	if (boolRectangle(&b_typeL))
	{
		Bars["cornerBars"] = "";
		setRectangleBH(Bars, true);
	}
	else
	{
		Bars["cornerBars"] = "";
		Bars["bSideBar"] = "";
		Bars["hSideBar"] = "";
	}
	information["longitudinalBars"] = Bars;

	//箍筋信息
	str.clear();
	for (auto it : all_txt)
	{
		if (it.find("@") != std::string::npos)
		{
			str = it;
		}
	}
	Json::Value stirrup;
	stirrup["all"] = str;


	std::vector<Line>h_lines;
	std::vector<Line>v_lines;
	std::vector<Line>s_lines;

	for (auto it : stittups)
	{
		for (auto it1 : it.circle_line)
			if (it1.horizontal())
			{
				h_lines.push_back(it1);
			}
			else if (it1.vertical())
			{
				v_lines.push_back(it1);
			}
	}
	LinesSort(h_lines, 'H');
	LinesSort(v_lines, 'V');
	MergeLines(h_lines, 'H');
	MergeLines(v_lines, 'V');
	str.clear();
	str = numToString(h_lines.size());
	str += "*" + numToString(v_lines.size());
	stirrup["count"] = str;

	//
	SetListStittups();
	char ch_temp = '0';
	std::string ch = "0";
	Json::Value stirrup_list;
	for (auto it : list_stittups)
	{

		stirrup_list[ch] = it;
		ch_temp++;
		ch = ch_temp;

	}
	stirrup["configuration "] = stirrup_list;//箍筋的排列
	information["stirrups"] = stirrup;
	//others 信息
	str.clear();
	for (auto it : all_txt)
	{
		str += "|" + it;
	}
	information["others"] = str;
	return information;
}


bool ColumnSection::SetSectionScale(std::string & scale, const Box & section_box, Data & data)
{
	double left = section_box.left;
	double right = section_box.right;
	int i = data.m_textPointData.findLowerBound(section_box.bottom);
	int j = data.m_textPointData.findUpperBound(section_box.top);

	//通过标注符号查找候选集
	std::fstream of("块内文本.txt", std::ios::out);
	for (i; i <= j; ++i)
	{
		auto &point = data.m_textPointData.textpoints()[i];
		if (point.x > left - Precision && point.x < right + Precision)
		{
			//zaici
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			if (tt->text.find(":") != std::string::npos)
			{
				scale = tt->text;
			}
		}
	}
	of.close();
	return true;
}

bool ColumnSection::SetSectionStirrup(std::vector<ColumnSection::Stirrup>& stirrup, const Box & section_box, Data & data)
{
	double left = section_box.left;
	double right = section_box.right;
	int i = data.m_textPointData.findLowerBound(section_box.bottom);
	int j = data.m_textPointData.findUpperBound(section_box.top);

	//通过标注符号查找候选集
	for (i; i <= j; ++i)
	{
		auto &point = data.m_textPointData.textpoints()[i];
		if (point.x > left - Precision && point.x < right + Precision)
		{
			//zaici
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			if (tt->text.find("@") != std::string::npos)
			{
				stirrup.push_back(ColumnSection::Stirrup(tt->text, 0));
			}
		}
	}
	return true;
}

bool ColumnSection::amendHooks(const int hook1, const int hook2, std::map<int, std::vector<Corner>>::const_iterator corners1,
	std::map<int, std::vector<Corner>>::const_iterator const corners2, std::vector<Corner>& corner1, std::vector<Corner>& corner2, Data & data)
{
	/*std::vector<Corner> extend_cor1;
	std::vector<Corner> extend_cor2;*/

	//保留hook的原始值，为还原做准备
	Line old1 = data.m_lineData.lines()[hook1];
	Line old2 = data.m_lineData.lines()[hook2];
	bool bh1 = false;
	bool bh2 = false;

	//[0]表示length_x 二分之一的长度 [1]表示length_y二分之一的长度
	std::vector<double> length1(2);
	std::vector<double> length2(2);
	Line temp_line1 = data.m_lineData.lines()[hook1];
	Line temp_line2 = data.m_lineData.lines()[hook2];

	if (corners1 == data.m_cornerData.corners().end())
	{
		bh1 = true;

		length1[0] = abs(data.m_lineData.lines()[hook1].s.x - data.m_lineData.lines()[hook1].e.x) / 2;
		length1[1] = abs(data.m_lineData.lines()[hook1].s.y - data.m_lineData.lines()[hook1].e.y) / 2;

		//给起点与终点做当前二分之的延长
		if (data.m_lineData.lines()[hook1].s.y > data.m_lineData.lines()[hook1].e.y)
		{
			Point start(data.m_lineData.lines()[hook1].s.x - length1[0], data.m_lineData.lines()[hook1].s.y + length1[1]);
			Point end(data.m_lineData.lines()[hook1].e.x + length1[0], data.m_lineData.lines()[hook1].e.y - length1[1]);
			temp_line1 = Line(start, end);
		}
		else
		{
			Point start(data.m_lineData.lines()[hook1].s.x - length1[0], data.m_lineData.lines()[hook1].s.y - length1[1]);
			Point end(data.m_lineData.lines()[hook1].e.x + length1[0], data.m_lineData.lines()[hook1].e.y + length1[1]);
			temp_line1 = Line(start, end);
		}
	}

	if (corners2 == data.m_cornerData.corners().end())
	{
		bh2 = true;

		length2[0] = abs(data.m_lineData.lines()[hook2].s.x - data.m_lineData.lines()[hook2].e.x) / 2;
		length2[1] = abs(data.m_lineData.lines()[hook2].s.y - data.m_lineData.lines()[hook2].e.y) / 2;

		//给起点与终点做当前二分之的延长
		if (data.m_lineData.lines()[hook2].s.y > data.m_lineData.lines()[hook2].e.y)
		{
			Point start(data.m_lineData.lines()[hook2].s.x - length2[0], data.m_lineData.lines()[hook2].s.y + length2[1]);
			Point end(data.m_lineData.lines()[hook2].e.x + length2[0], data.m_lineData.lines()[hook2].e.y - length2[1]);
			temp_line2 = Line(start, end);
		}
		else
		{
			Point start(data.m_lineData.lines()[hook2].s.x - length2[0], data.m_lineData.lines()[hook2].s.y - length2[1]);
			Point end(data.m_lineData.lines()[hook2].e.x + length2[0], data.m_lineData.lines()[hook2].e.y + length2[1]);
			temp_line2 = Line(start, end);
		}
	}

	extendCorner(corner1, temp_line1, data.m_lineData);
	extendCorner(corner2, temp_line2, data.m_lineData);
	return true;
}


std::vector<ColumnSection::BreakPoint> ColumnSection::ReturnSectionTopAngle(const ColumnSection & section)
{
	ColumnSection::BreakPoint bp;
	bp.center_mind = Point(section.section_block_box.right, section.section_block_box.top);
	bp.radius = section.break_point.front().radius;
	ColumnSection::BreakPoint l_bp = ColumnSection::BreakPoint(bp);//最左
	bp.center_mind = Point(section.section_block_box.right, section.section_block_box.bottom);
	ColumnSection::BreakPoint u_bp = ColumnSection::BreakPoint(bp);//最上
	bp.center_mind = Point(section.section_block_box.left, section.section_block_box.bottom);
	ColumnSection::BreakPoint r_bp = ColumnSection::BreakPoint(bp);//最右
	bp.center_mind = Point(section.section_block_box.left, section.section_block_box.top);
	ColumnSection::BreakPoint d_bp = ColumnSection::BreakPoint(bp);//最下
	std::vector<ColumnSection::BreakPoint>temp_bp;
	for (auto it : section.break_point)
	{
		if (it.center_mind.x <= l_bp.center_mind.x)//左
		{
			l_bp = it;
		}
		if (it.center_mind.y >= u_bp.center_mind.y)//上
		{
			u_bp = it;
		}
		if (it.center_mind.x >= r_bp.center_mind.x)//右
		{
			r_bp = it;
		}
		if (it.center_mind.y <= d_bp.center_mind.y)//下
		{
			d_bp = it;
		}
	}
	temp_bp.push_back(l_bp);
	temp_bp.push_back(u_bp);
	temp_bp.push_back(r_bp);
	temp_bp.push_back(d_bp);
	return temp_bp;
}

//初始化柱断面的包围线，并返回左下角的参考线，为后续识别做铺垫
bool ColumnSection::SetSectionBreakPointSurround(ColumnSection &section, Data &data)
{
	/*
	-此处考虑了不是矩形柱的情况所以会有两个参照点
	-根据左下角的参照线依次找出相连的箍筋包围线，
	-作用：为后面规约hook做准备
	*/

	/***************************************************************************/
	std::vector<ColumnSection::BreakPoint> corner_point;
	corner_point = ReturnSectionTopAngle(section);
	std::vector<int>candidate_index;
	std::vector<int>surround_index;
	std::vector<int>c_s_l;//candidate_surround_left
	std::vector<int>c_s_r;//candidate_surround_right
	std::vector<int>c_s_u;//candidate_surround_top
	std::vector<int>c_s_d;//candidate_surround_up
	Point corner_l = corner_point[0].center_mind;
	Point corner_u = corner_point[1].center_mind;
	Point corner_r = corner_point[2].center_mind;
	Point corner_d = corner_point[3].center_mind;

	//
	double vf = 0.0;
	double vt = 0.0;
	double hf = 0.0;
	double ht = 0.0;


	double left = section.section_block_box.left;
	double bottom = section.section_block_box.bottom;
	double right = section.section_block_box.right;
	double top = section.section_block_box.top;

	//左
	{

		double min[2] = { left,bottom };
		double max[2] = { corner_l.x,top };

		//RTreeData temp_rtree;
		int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex, &data.m_rtreeData, std::placeholders::_1));

		vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, left, Precision, false);
		vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, corner_l.x, Precision, false);
		for (int i = vf; i <= vt; ++i)
		{
			int index = data.m_lineData.vLinesIndices()[i];
			if (data.m_lineData.lines()[index].e.y + Precision < corner_d.y ||
				data.m_lineData.lines()[index].s.y - Precision > corner_u.y)
				continue;
			candidate_index.push_back(index);
		}
		std::vector<int>h_index;
		std::vector<int>v_index;
		std::vector<int>s_index;

		for (auto it1 : data.m_rtreeData.SBackIndex())
		{
			candidate_index.push_back(it1);
		}
		data.m_rtreeData.clearSBackIndex();
		data.m_lineData.lineTypeVHS(candidate_index, v_index, h_index, s_index);
		candidate_index.clear();

		if (!s_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : s_index)
			{
				double x = knowYToX(data.m_lineData.lines()[it1], corner_l.y);

				Point minx, maxx;
				if (data.m_lineData.lines()[it1].s.x < data.m_lineData.lines()[it1].e.x)
				{
					minx = data.m_lineData.lines()[it1].s;
					maxx = data.m_lineData.lines()[it1].e;
				}
				else
				{
					minx = data.m_lineData.lines()[it1].e;
					maxx = data.m_lineData.lines()[it1].s;
				}
				if (x>minx.x&&x < maxx.x)
				{
					double temp_num = abs(x - corner_l.x);
					if (length > temp_num)
					{
						length = temp_num;
						goal_index = it1;

					}
				}
			}
			if (goal_index != -1)
			{
				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
		//查找边界
		if (!v_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : v_index)
			{
				if (data.m_lineData.lines()[it1].s.x + Precision < corner_l.x)
				{
					double temp_num = abs(data.m_lineData.lines()[it1].s.x - corner_l.x);
					if (temp_num < length)
					{
						length = temp_num;
						goal_index = it1;
					}

				}
			}



			if (goal_index != -1)
			{
				//查找最长边界
				for (auto it1 : v_index)
				{
					if (data.m_lineData.lines()[it1].s.x + Precision * 2 > data.m_lineData.lines()[goal_index].s.x&&
						data.m_lineData.lines()[it1].s.x - Precision * 2< data.m_lineData.lines()[goal_index].s.x&&
						data.m_lineData.lines()[it1].length()> data.m_lineData.lines()[goal_index].length())
					{
						goal_index = it1;
					}
				}

				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}

	}

	//上
	{
		double min[2] = { left, corner_u.y };
		double max[2] = { right,top };
		RTreeData temp_rtree;
		int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex, &data.m_rtreeData, std::placeholders::_1));

		/*hf = findLowerBound(corner_u.y, _hlines, _lines);
		ht = findUpperBound(top, _hlines, _lines);*/
		hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, corner_u.y);
		ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, top);
		for (int i = hf; i <= ht; ++i)
		{
			int index = data.m_lineData.hLinesIndices()[i];
			if (data.m_lineData.lines()[index].e.x + Precision < corner_l.x ||
				data.m_lineData.lines()[index].s.x - Precision > corner_r.x)
			{
				continue;
			}
			candidate_index.push_back(index);
		}
		for (auto it : data.m_rtreeData.SBackIndex())
		{
			candidate_index.push_back(it);
		}
		data.m_rtreeData.clearSBackIndex();
		std::vector<int>h_index;
		std::vector<int>v_index;
		std::vector<int>s_index;

		data.m_lineData.lineTypeVHS(candidate_index, v_index, h_index, s_index);
		candidate_index.clear();


		candidate_index.clear();
		if (!s_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : s_index)
			{
				double y = knowXToY(data.m_lineData.lines()[it1], corner_u.x);

				Point miny, maxy;
				if (data.m_lineData.lines()[it1].s.y < data.m_lineData.lines()[it1].e.y)
				{
					miny = data.m_lineData.lines()[it1].s;
					maxy = data.m_lineData.lines()[it1].e;
				}
				else
				{
					miny = data.m_lineData.lines()[it1].e;
					maxy = data.m_lineData.lines()[it1].s;
				}
				if (y>miny.y&&y < maxy.y)
				{
					double temp_num = abs(y - corner_u.y);
					if (length > temp_num)
					{
						length = temp_num;
						goal_index = it1;

					}
				}
			}
			if (goal_index != -1)
			{
				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
		if (!h_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : h_index)
			{
				if (data.m_lineData.lines()[it1].s.y - Precision > corner_u.y)
				{
					double temp_num = abs(data.m_lineData.lines()[it1].s.y - corner_u.y);
					if (temp_num < length)
					{

						length = temp_num;
						goal_index = it1;
					}

				}
			}


			if (goal_index != -1)
			{
				//查找最长边界
				for (auto it1 : h_index)
				{
					if (data.m_lineData.lines()[it1].s.y + Precision * 2 > data.m_lineData.lines()[goal_index].s.y&&
						data.m_lineData.lines()[it1].s.y - Precision * 2<data.m_lineData.lines()[goal_index].s.y&&
						data.m_lineData.lines()[it1].length()>data.m_lineData.lines()[goal_index].length())
					{
						goal_index = it1;
					}
				}

				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
	}
	//右
	{
		double min[2] = { corner_r.x,bottom };
		double max[2] = { right,top };

		RTreeData temp_rtree;
		int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex, &data.m_rtreeData, std::placeholders::_1));


		//vf = findLowerBound(corner_r.x, _vlines, _lines, Precision, false);
		//vt = findUpperBound(right, _vlines, _lines, Precision, false);

		vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, corner_r.x, Precision, false);
		vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, right, Precision, false);
		for (int i = vf; i <= vt; ++i)
		{
			int index = data.m_lineData.vLinesIndices()[i];
			if (data.m_lineData.lines()[index].e.y + Precision < corner_d.y ||
				data.m_lineData.lines()[index].s.y - Precision > corner_u.y)
				continue;
			candidate_index.push_back(index);
		}

		std::vector<int>h_index;
		std::vector<int>v_index;
		std::vector<int>s_index;
		for (auto it1 : data.m_rtreeData.SBackIndex())
		{
			candidate_index.push_back(it1);
		}
		data.m_rtreeData.clearSBackIndex();

		data.m_lineData.lineTypeVHS(candidate_index, v_index, h_index, s_index);
		candidate_index.clear();


		if (!s_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : s_index)
			{
				double x = knowYToX(data.m_lineData.lines()[it1], corner_r.y);
				Point minx, maxx;
				if (data.m_lineData.lines()[it1].s.x < data.m_lineData.lines()[it1].e.x)
				{
					minx = data.m_lineData.lines()[it1].s;
					maxx = data.m_lineData.lines()[it1].e;
				}
				else
				{
					minx = data.m_lineData.lines()[it1].e;
					maxx = data.m_lineData.lines()[it1].s;
				}
				if (x>minx.x&&x < maxx.x)
				{
					double temp_num = abs(x - corner_r.x);
					if (length > temp_num)
					{
						length = temp_num;
						goal_index = it1;

					}
				}
			}
			if (goal_index != -1)
			{

				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
		if (!v_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : v_index)
			{
				if (data.m_lineData.lines()[it1].s.x - Precision > corner_r.x)
				{
					double temp_num = abs(data.m_lineData.lines()[it1].s.x - corner_r.x);
					if (temp_num < length)
					{
						length = temp_num;
						goal_index = it1;

					}

				}
			}


			if (goal_index != -1)
			{
				for (auto it1 : v_index)
				{
					if (data.m_lineData.lines()[it1].s.x + Precision * 2 > data.m_lineData.lines()[goal_index].s.x&&
						data.m_lineData.lines()[it1].s.x - Precision * 2<data.m_lineData.lines()[goal_index].s.x&&
						data.m_lineData.lines()[it1].length()>data.m_lineData.lines()[goal_index].length())
					{
						goal_index = it1;
					}
				}

				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
	}
	//下
	{
		double min[2] = { left,bottom };
		double max[2] = { right,corner_d.y };
		RTreeData temp_rtree;
		int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex, &data.m_rtreeData, std::placeholders::_1));


		//hf = findLowerBound(bottom, _hlines, _lines);
		//ht = findUpperBound(corner_d.y, _hlines, _lines);
		hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bottom);
		ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, corner_d.y);

		std::vector<int>h_index;
		std::vector<int>v_index;
		std::vector<int>s_index;
		for (int i = hf; i <= ht; ++i)
		{
			int index = data.m_lineData.hLinesIndices()[i];
			if (data.m_lineData.lines()[index].e.x + Precision < corner_l.x ||
				data.m_lineData.lines()[index].s.x - Precision > corner_r.x)
				continue;
			candidate_index.push_back(index);
		}
		for (auto it : data.m_rtreeData.SBackIndex())
		{
			candidate_index.push_back(it);
		}
		data.m_rtreeData.clearSBackIndex();
		data.m_lineData.lineTypeVHS(candidate_index, v_index, h_index, s_index);
		candidate_index.clear();


		if (!s_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : s_index)
			{
				double y = knowXToY(data.m_lineData.lines()[it1], corner_d.x);

				Point miny, maxy;
				if (data.m_lineData.lines()[it1].s.y < data.m_lineData.lines()[it1].e.y)
				{
					miny = data.m_lineData.lines()[it1].s;
					maxy = data.m_lineData.lines()[it1].e;
				}
				else
				{
					miny = data.m_lineData.lines()[it1].e;
					maxy = data.m_lineData.lines()[it1].s;
				}
				if (y>miny.y&&y < maxy.y)
				{
					double temp_num = abs(y - corner_d.y);
					if (length > temp_num)
					{
						length = temp_num;
						goal_index = it1;

					}
				}
			}
			if (goal_index != -1)
			{
				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
		if (!h_index.empty())
		{
			double length = 65535;
			int goal_index = -1;
			for (auto it1 : h_index)
			{
				if (data.m_lineData.lines()[it1].s.y + Precision < corner_d.y)
				{
					double temp_num = abs(data.m_lineData.lines()[it1].s.y - corner_d.y);
					if (temp_num < length)
					{
						length = temp_num;
						goal_index = it1;

					}

				}
			}


			if (goal_index != -1)
			{
				for (auto it1 : h_index)
				{
					if (data.m_lineData.lines()[it1].s.y + Precision * 2 > data.m_lineData.lines()[goal_index].s.y&&
						data.m_lineData.lines()[it1].s.y - Precision * 2<data.m_lineData.lines()[goal_index].s.y&&
						data.m_lineData.lines()[it1].length()>data.m_lineData.lines()[goal_index].length())
					{
						goal_index = it1;
					}
				}

				auto ite = find(surround_index.begin(), surround_index.end(), goal_index);
				if (ite == surround_index.end())
				{
					//temp_index.push_back(goal_index);
					surround_index.push_back(goal_index);
				}
			}

		}
	}


	//查找包围线
	std::queue<int>queue_index;
	std::map<int, bool>map_ide;;//surround map
	std::vector<int>cull_index;//剔除误识别索引
	for (auto it : surround_index)
	{
		//section.bp_surround.push_back(it);
		//temp_index.push_back(it);
		queue_index.push(it);
		map_ide[it] = true;
		cull_index.push_back(it);
	}
	while (!queue_index.empty())
	{
		auto index = queue_index.front();
		queue_index.pop();
		auto corners = data.m_cornerData.corners().find(index);

		int s_index = -1;
		int e_index = -1;
		if (corners != data.m_cornerData.corners().end())
		{
			for (auto it : corners->second)
			{
				auto& l1 = it.l1;
				if (l1 == index)
				{
					l1 = it.l2;
				}

				if (ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].s) ||
					ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].e))

				{
					if (s_index == -1)
					{
						s_index = l1;
					}
					else if (data.m_lineData.lines()[s_index].length() < data.m_lineData.lines()[l1].length())
					{
						s_index = l1;
					}
				}
				if (ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].s) ||
					ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].e))
				{
					if (e_index == -1)
					{
						e_index = l1;
					}
					else if (data.m_lineData.lines()[e_index].length() < data.m_lineData.lines()[l1].length())
					{
						e_index = l1;
					}
				}
			}

			if (!map_ide[s_index] && s_index != -1)
			{
				map_ide[s_index] = true;
				queue_index.push(s_index);
				//temp_index.push_back(s_index);

				cull_index.push_back(s_index);
			}
			if (!map_ide[e_index] && e_index != -1)
			{
				map_ide[e_index] = true;
				queue_index.push(e_index);
				//temp_index.push_back(e_index);

				cull_index.push_back(e_index);
			}
		}
	}


	//剔除操作

	std::vector<int>see_map(cull_index.size());
	std::map<int, bool>map_index;
	for (int i = 0; i < cull_index.size(); ++i)
	{
		if (find(see_map.begin(), see_map.end(), cull_index[i]) != see_map.end())
			continue;


		std::queue<int>temp_queue;
		int start_index = cull_index[i];

		std::vector<int>link;
		std::map<int, bool>map_link;
		temp_queue.push(start_index);
		link.push_back(start_index);
		while (!temp_queue.empty())
		{
			auto it = temp_queue.front();
			temp_queue.pop();

			std::pair<Point, bool>s = std::pair<Point, bool>(data.m_lineData.lines()[it].s, false);
			std::pair<Point, bool> e = std::pair<Point, bool>(data.m_lineData.lines()[it].e, false);

			auto corners = data.m_cornerData.corners().find(it);
			if (corners == data.m_cornerData.corners().end())
			{
				see_map.push_back(start_index);
				link.push_back(start_index);
				map_link[start_index] = false;
			}
			else
			{
				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == it)
					{
						l1 = corner.l2;
					}
					auto ite = find(cull_index.begin(), cull_index.end(), l1);
					if (ite != cull_index.end())
					{
						if (ConnectPoint(s.first, corner))
						{
							if (find(link.begin(), link.end(), l1) == link.end())
							{
								temp_queue.push(l1);
							}
							link.push_back(l1);
							s.second = true;;

						}
						else if (ConnectPoint(e.first, corner))
						{
							if (find(link.begin(), link.end(), l1) == link.end())
							{
								temp_queue.push(l1);
							}
							link.push_back(l1);
							e.second = true;

						}
					}
				}
			}

			if (s.second&&e.second)
			{
				map_link[it] = true;
			}
			else
			{
				map_link[it] = false;
			}


		}

		bool mark = true;
		for (auto it : map_link)
		{
			if (!it.second)
			{
				mark = false;
				break;
			}
		}

		if (mark)
		{
			for (auto it : link)
			{
				map_index[it] = true;
				see_map.push_back(it);
			}
		}
		else
		{
			for (auto it : link)
			{
				map_index[it] = false;
				see_map.push_back(it);
			}
		}

	}

	//获取真实边界
	for (auto it : map_index)
	{

		if (it.second)
		{
			//temp_index.push_back(it.first);
			section.bp_surround.push_back(it.first);
		}
	}
	return true;
}

bool ColumnSection::SetSectionHook(std::vector<ColumnStirrupHook> hooks, ColumnSection & section, Data & data)
{
	/*逻辑流程
	-在所有包围箍筋中找到最长的边界用作，箍筋标志误识别的剔除条件（归类【横纵】，排序，合并）
	-根据范围查找箍筋的标志（Rtree）
	-剔除多余的标志
	*/

	std::vector<int> v_lines;
	std::vector<int> h_lines;
	std::vector<int> ss_lines;
	std::vector<Line> temp;
	std::vector<int> candidate; //候选目标
	std::vector<std::pair<int, Point>> hook_goal;
	ClosedZone borderline;
	//
	//FindStirrupLines(section, h_lines, v_lines, ss_lines);
	if (section.bp_surround.empty())
	{
		std::fstream of("error.txt", std::ios::app);
		of << section.name << ":" << "not find break_point surround" << std::endl;
		of.close();
		return true;
	}
	else
	{
		data.m_lineData.lineTypeVHS(section.bp_surround, v_lines, h_lines, ss_lines);
	}


	//排序
	for (int i = 0; i < v_lines.size(); ++i)
	{
		int temp_line;
		for (int j = 0; j < v_lines.size() - i - 1; ++j)
		{
			if (!compareRefV(data.m_lineData.lines(), v_lines[j], v_lines[j + 1]))
			{
				temp_line = v_lines[j];
				v_lines[j] = v_lines[j + 1];
				v_lines[j + 1] = temp_line;
			}
		}
	}
	for (int i = 0; i < h_lines.size(); ++i)
	{
		int temp_line;
		for (int j = 0; j < h_lines.size() - i - 1; ++j)
		{
			if (!compareRefH(data.m_lineData.lines(), h_lines[j], h_lines[j + 1]))
			{
				temp_line = h_lines[j];
				h_lines[j] = h_lines[j + 1];
				h_lines[j + 1] = temp_line;
			}
		}
	}
	//剔除多余的
	for (int i = 0; i < v_lines.size() - 1; ++i)
	{
		if (v_lines.empty())
			break;
		if (v_lines[i] == -1)
			continue;
		for (int j = i + 1; j < v_lines.size(); ++j)
		{
			if (v_lines[i] == -1 || v_lines[j] == -1)
				continue;
			if (data.m_lineData.lines()[v_lines[i]].s.x + Precision > data.m_lineData.lines()[v_lines[j]].s.x &&
				data.m_lineData.lines()[v_lines[i]].s.x - Precision < data.m_lineData.lines()[v_lines[j]].s.x)
			{
				int num = data.m_lineData.lines()[v_lines[i]].length() < data.m_lineData.lines()[v_lines[j]].length() ? i : j;
				v_lines[num] = -1;
			}
			else
			{
				break;
			}
		}
	}

	for (int i = 0; i < h_lines.size() - 1; ++i)
	{
		if (h_lines.empty())
			break;
		if (h_lines[i] == -1)
			continue;
		for (int j = i + 1; j < h_lines.size(); ++j)
		{
			if (h_lines[i] == -1 || h_lines[j] == -1)
				continue;
			if (data.m_lineData.lines()[h_lines[i]].s.y + Precision > data.m_lineData.lines()[h_lines[j]].s.y &&
				data.m_lineData.lines()[h_lines[i]].s.y - Precision < data.m_lineData.lines()[h_lines[j]].s.y)
			{
				int num = data.m_lineData.lines()[h_lines[i]].length() < data.m_lineData.lines()[h_lines[j]].length() ? i : j;
				h_lines[num] = -1;
			}
			else
			{
				break;
			}
		}
	}
	//确定范围并搜索
	std::vector<int> v_index;
	std::vector<int> h_index;
	for (auto it : v_lines)
	{
		if (it != -1)
		{
			v_index.push_back(it);
		}
	}
	for (auto it : h_lines)
	{
		if (it != -1)
		{
			h_index.push_back(it);
		}
	}
	//搜索箍筋的弯钩——箍筋的标志（）
	if (ss_lines.empty())
	{
		double min[2] = { 65535 };
		double max[2] = { 0 };

		for (int i = 0; i < v_index.size() - 1; ++i)
		{

			min[0] = data.m_lineData.lines()[v_index[i]].s.x;
			min[1] = data.m_lineData.lines()[v_index[i]].s.y;
			max[0] = data.m_lineData.lines()[v_index[i + 1]].s.x;
			max[1] = data.m_lineData.lines()[v_index[i]].e.y < data.m_lineData.lines()[v_index[i + 1]].e.y ? data.m_lineData.lines()[v_index[i]].e.y : data.m_lineData.lines()[v_index[i + 1]].e.y;


			int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex/*&Parser::SearchSLinesBack*/, &data.m_rtreeData/*this*/, std::placeholders::_1));
			//SLine::Iterator it;
			for (auto it : data.m_rtreeData.SBackIndex())
			{
				//temp_index.push_back(it);
				candidate.push_back(it);
			}
			data.m_rtreeData.clearSBackIndex();
		}
	}
	else
	{
		double min[2] = { 65535 };
		double max[2] = { 0 };

		min[0] = section.section_box.left;
		min[1] = section.section_box.bottom;
		max[0] = section.section_box.right;
		max[1] = section.section_box.top;
		int num = data.m_rtreeData.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex/*&Parser::SearchSLinesBack*/, &data.m_rtreeData/*this*/, std::placeholders::_1));
		for (auto it : data.m_rtreeData.SBackIndex())
		{
			//temp_index.push_back(it);
			candidate.push_back(it);
		}
		data.m_rtreeData.clearSBackIndex();

		b_closedzone(section.bp_surround, borderline, data);
		//b_InClosedZone(candidate, section, borderline);

		std::vector<int>candidate_map(candidate.size());
		for (int i = 0; i < candidate.size(); ++i)
		{

			if (b_InClosedZone(candidate[i], section.bp_surround, borderline, data) == false)
			{
				candidate_map[i] = -1;
			}

		}
		std::vector<int>temp = candidate;
		candidate.clear();
		for (int i = 0; i < candidate_map.size(); ++i)
		{
			if (candidate_map[i] != -1)
			{
				candidate.push_back(temp[i]);
			}
		}
	}
	//需外加单只股处理
	//剔除误识别的弯钩
	std::vector<bool> map(candidate.size());
	int count = -1;
	for (auto it : candidate)
	{
		count++;
		auto corners = data.m_cornerData.corners().find(it);
		if (corners == data.m_cornerData.corners().end())
			continue;
		std::vector<bool> c_map(corners->second.size());
		int c_count = -1;
		for (auto it1 : corners->second)
		{
			int l1 = it1.l1;
			if (l1 == it)
			{
				l1 = it1.l2;
			}

			if (data.m_lineData.lines()[l1].vertical())
			{
				//排除引出线
				if (find(v_index.begin(), v_index.end(), l1) != v_index.end() &&
					abs(data.m_lineData.lines()[it].s.x - data.m_lineData.lines()[l1].s.x) > Precision + 5 &&
					abs(data.m_lineData.lines()[it].e.x - data.m_lineData.lines()[l1].s.x) > Precision + 5)
				{
					map[count] = true;
					break;
				}
				//排除箍筋的标志线
				if (abs(data.m_lineData.lines()[it].s.x - data.m_lineData.lines()[l1].s.x) > Precision + 5 && abs(data.m_lineData.lines()[it].e.x - data.m_lineData.lines()[l1].s.x) > Precision + 5)
				{
					c_map[++c_count] = true;
				}
			}
			else if (data.m_lineData.lines()[l1].horizontal())
			{ //排除引出线
				if (find(h_index.begin(), h_index.end(), l1) != h_index.end() &&
					abs(data.m_lineData.lines()[it].s.y - data.m_lineData.lines()[l1].s.y) > Precision + 5 &&
					abs(data.m_lineData.lines()[it].e.y - data.m_lineData.lines()[l1].s.y) > Precision + 5)
				{
					map[count] = true;
					break;
				}
				//排除箍筋的标志线
				if (abs(data.m_lineData.lines()[it].s.y - data.m_lineData.lines()[l1].s.y) > Precision + 5 && abs(data.m_lineData.lines()[it].e.y - data.m_lineData.lines()[l1].s.y) > Precision + 5)
				{
					c_map[++c_count] = true;
				}
			}
		}
		if (find(c_map.begin(), c_map.end(), false) == c_map.end())
		{
			map[count] = 1;
		}
	}
	//结果
	for (int i = 0; i < candidate.size(); ++i)
	{
		if (!map[i])
		{
			//temp_index.push_back(candidate[i]);
			Point mid_point((data.m_lineData.lines()[candidate[i]].s.x + data.m_lineData.lines()[candidate[i]].e.x) / 2,
				(data.m_lineData.lines()[candidate[i]].s.y + data.m_lineData.lines()[candidate[i]].e.y) / 2);
			hook_goal.push_back(std::pair<int, Point>(candidate[i], mid_point));
		}
	}
	//查找箍筋标志——勾（一对）
	if (candidate.empty())
		return true;
	std::vector<bool> hook_map(hook_goal.size());
	for (int i = 0; i < hook_goal.size(); ++i)
	{
		if (hook_map[i])
			continue;

		double length = 65536;
		int temp_num = i;
		for (int j = 0; j < hook_goal.size(); ++j)
		{
			if (i == j || hook_map[i] || hook_map[j])
				continue;
			double temp_length = TowPointsLength(hook_goal[i].second, hook_goal[j].second);
			if (length > temp_length)
			{
				temp_num = j;
				length = temp_length;
			}
		}

		ColumnStirrupHook temp_hook(hook_goal[i].first, hook_goal[temp_num].first /*, false*/);
		section.hooks.push_back(temp_hook);
		hook_map[i] = true;
		hook_map[temp_num] = true;
	}

	return true;
}

bool ColumnSection::SetSectionBox(ColumnSection & section, Data &data)
{
	Box temp_box;
	for (auto it : section.bp_surround)
	{
		temp_box.expand(data.m_lineData.lines()[it].s);
		temp_box.expand(data.m_lineData.lines()[it].e);
	}
	section.section_box = temp_box;
	return true;
}

bool ColumnSection::SetSectionStirrupReferencePoint(ColumnSection & section, Data &data)
{
	//参考点
	int ref_line; //reference lines
	std::vector<int> goal_index;
	std::vector<int> temp_goal_index;
	int hf;
	int ht;
	int vf;
	int vt;
	std::vector<int> h_index;
	std::vector<int> v_index;
	std::vector<int> s_index;
	data.m_lineData.lineTypeVHS(section.bp_surround, v_index, h_index, s_index);
	bool mark = false;
	if (!s_index.empty())
	{
		mark = true;
	}
	else if (!h_index.empty() && !v_index.empty())
	{
		mark = false;
	}

	if (mark)
	{
		return true; //斜柱后续处理
	}
	else
	{

		data.m_lineData.linesSortIndex(h_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
		data.m_lineData.mergeLinesIndex(h_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
		data.m_lineData.linesSortIndex(v_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
		data.m_lineData.mergeLinesIndex(v_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);

		if (v_index.size() >= 2)
		{
			//左边界
			vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, section.section_block_box.left, Precision, false);
			vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, data.m_lineData.lines()[v_index.front()].s.x, Precision, false);

			for (int i = vf; i <= vt; ++i)
			{
				int index = data.m_lineData.vLinesIndices()[i];
				if (data.m_lineData.lines()[index].e.y + Precision < data.m_lineData.lines()[h_index.front()].s.y ||
					data.m_lineData.lines()[index].s.y - Precision > data.m_lineData.lines()[h_index.back()].s.y)
				{
					continue;
				}
				if (section.section_block_box.cover(data.m_lineData.lines()[index].e) &&
					section.section_block_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].e))
				{
					temp_goal_index.push_back(index);
					//temp_index.push_back(index);
				}
			}
			data.m_lineData.linesSortIndex(temp_goal_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);

			goal_index.push_back(temp_goal_index.back());
			//temp_index.push_back(temp_goal_index.back());
			for (int i = temp_goal_index.size() - 1; i >= 0; --i)
			{
				if (temp_goal_index[i] == temp_goal_index.back())
					continue;
				int num = temp_goal_index.back();
				if (data.m_lineData.lines()[temp_goal_index[i]].s.x + Precision > data.m_lineData.lines()[num].s.x &&
					data.m_lineData.lines()[temp_goal_index[i]].s.x - Precision < data.m_lineData.lines()[num].s.x)
				{
					goal_index.push_back(temp_goal_index[i]);
					//temp_index.push_back(temp_goal_index[i]);
				}
				else
				{
					break;
				}
			}
			temp_goal_index.clear();
			//右边界
			vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, data.m_lineData.lines()[v_index.back()].s.x, Precision, false);
			vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, section.section_block_box.right, Precision, false);

			for (int i = vf; i <= vt; ++i)
			{
				int index = data.m_lineData.vLinesIndices()[i];
				if (data.m_lineData.lines()[index].e.y + Precision < data.m_lineData.lines()[h_index.front()].s.y ||
					data.m_lineData.lines()[index].s.y - Precision > data.m_lineData.lines()[h_index.back()].s.y)
				{
					continue;
				}
				if (section.section_block_box.cover(data.m_lineData.lines()[index].e) &&
					section.section_block_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].e))
				{
					temp_goal_index.push_back(index);
					//temp_index.push_back(index);
				}
			}
			data.m_lineData.linesSortIndex(temp_goal_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
			goal_index.push_back(temp_goal_index.front());
			//temp_index.push_back(temp_goal_index.front());
			for (int i = 0; i < temp_goal_index.size(); ++i)
			{
				if (temp_goal_index[i] == temp_goal_index.front())
					continue;
				int num = temp_goal_index.front();
				if (data.m_lineData.lines()[temp_goal_index[i]].s.x + Precision > data.m_lineData.lines()[num].s.x &&
					data.m_lineData.lines()[temp_goal_index[i]].s.x - Precision < data.m_lineData.lines()[num].s.x)
				{
					goal_index.push_back(temp_goal_index[i]);
					//temp_index.push_back(temp_goal_index[i]);
				}
				else
				{
					break;
				}
			}
			temp_goal_index.clear();
		}
		//else if(v_index.size() > 2)
		//{
		//	;//后续处理
		//}

		if (h_index.size() >= 2)
		{
			//下边界
			int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, section.section_block_box.bottom);
			int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, data.m_lineData.lines()[h_index.front()].s.y);
			for (int i = hf; i <= ht; ++i)
			{
				int index = data.m_lineData.hLinesIndices()[i];
				if (data.m_lineData.lines()[index].e.x + Precision < data.m_lineData.lines()[v_index.front()].s.x ||
					data.m_lineData.lines()[index].s.x - Precision > data.m_lineData.lines()[v_index.back()].s.x)
				{
					continue;
				}
				if (section.section_block_box.cover(data.m_lineData.lines()[index].e) &&
					section.section_block_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].e))
				{
					temp_goal_index.push_back(index);
					//temp_index.push_back(index);
				}
			}
			data.m_lineData.linesSortIndex(temp_goal_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
			goal_index.push_back(temp_goal_index.back());
			//temp_index.push_back(temp_goal_index.back());
			for (int i = temp_goal_index.size() - 1; i >= 0; --i)
			{
				if (temp_goal_index[i] == temp_goal_index.back())
					continue;
				int num = temp_goal_index.back();
				if (data.m_lineData.lines()[temp_goal_index[i]].s.y + Precision > data.m_lineData.lines()[num].s.y &&
					data.m_lineData.lines()[temp_goal_index[i]].s.y - Precision < data.m_lineData.lines()[num].s.y)
				{
					goal_index.push_back(temp_goal_index[i]);
					//temp_index.push_back(temp_goal_index[i]);
				}
				else
				{
					break;
				}
			}
			temp_goal_index.clear();
			//上边界
			hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, data.m_lineData.lines()[h_index.back()].s.y);
			ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, section.section_block_box.top);
			for (int i = hf; i <= ht; ++i)
			{
				int index = data.m_lineData.hLinesIndices()[i];
				if (data.m_lineData.lines()[index].e.x + Precision < data.m_lineData.lines()[v_index.front()].s.x ||
					data.m_lineData.lines()[index].s.x - Precision > data.m_lineData.lines()[v_index.back()].s.x)
				{
					continue;
				}
				if (section.section_block_box.cover(data.m_lineData.lines()[index].e) &&
					section.section_block_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].s) &&
					!section.section_box.cover(data.m_lineData.lines()[index].e))
				{
					temp_goal_index.push_back(index);
					//temp_index.push_back(index);
				}
			}
			data.m_lineData.linesSortIndex(temp_goal_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
			goal_index.push_back(temp_goal_index.front());
			//temp_index.push_back(temp_goal_index.front());
			for (int i = 0; i < temp_goal_index.size(); ++i)
			{
				if (temp_goal_index[i] == temp_goal_index.front())
					continue;
				int num = temp_goal_index.front();
				if (data.m_lineData.lines()[temp_goal_index[i]].s.y + Precision > data.m_lineData.lines()[num].s.y &&
					data.m_lineData.lines()[temp_goal_index[i]].s.y - Precision < data.m_lineData.lines()[num].s.y)
				{
					goal_index.push_back(temp_goal_index[i]);
					//temp_index.push_back(temp_goal_index[i]);
				}
				else
				{
					break;
				}
			}
			temp_goal_index.clear();
		}
		/*else
		{

		}*/
	}
	h_index.clear();
	v_index.clear();
	s_index.clear();
	data.m_lineData.lineTypeVHS(section.bp_surround, v_index, h_index, s_index);

	data.m_lineData.linesSortIndex(h_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	data.m_lineData.linesSortIndex(v_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	//linesSortIndex(s_index, 'S');
	double left = data.m_lineData.lines()[v_index.front()].s.x;
	double right = data.m_lineData.lines()[v_index.back()].s.x;
	double bottom = data.m_lineData.lines()[h_index.front()].s.y;
	double top = data.m_lineData.lines()[h_index.back()].s.y;
	temp_goal_index.clear();

	h_index.clear();
	v_index.clear();
	s_index.clear();
	data.m_lineData.lineTypeVHS(goal_index, v_index, h_index, s_index);
	for (int i = 0; i < v_index.size(); ++i)
	{
		if (data.m_lineData.lines()[v_index[i]].e.y - Precision > bottom || data.m_lineData.lines()[v_index[i]].s.y + Precision < top)
		{
			temp_goal_index.push_back(v_index[i]);
		}
	}
	for (int i = 0; i < h_index.size(); ++i)
	{
		if (data.m_lineData.lines()[h_index[i]].e.x - Precision > left || data.m_lineData.lines()[h_index[i]].s.x + Precision < right)
		{
			temp_goal_index.push_back(h_index[i]);
		}
	}
	section.pillar_surround = temp_goal_index;
	section.stirrup_refer_point = Point(data.m_lineData.lines()[v_index.front()].s.x, data.m_lineData.lines()[h_index.front()].s.y);

	return true;
}

bool ColumnSection::SetSectionStirrupInformation(ColumnSection & section, Data &data)
{
	bool mark;
	mark = InTurnLinkStirrup(section, data);
	if (mark == false)
	{
		DirecctionMatchingStirrup(section, data);
	}
	CheckStirrup(section, data);
	return true;
}

bool ColumnSection::InTurnLinkStirrup(ColumnSection & section, Data &data)
{
	/*识别逻辑
	-经过对比发现，很大一部分箍筋，每个股都有自己独立的闭环，根据此特点依次相连找到每个箍
	-先从每个箍筋的标志对出发，依次查找直到整体闭合为止
	-
	*/
	std::vector<int> goal_map;
	for (auto it : section.hooks)
	{
		if (it.hook_index1 == it.hook_index2)
		{
			continue;
		}
		std::queue<int> queue;
		ColumnSStirrup temp_stirrup;
		auto hook1 = it.hook_index1;
		auto hook2 = it.hook_index2;
		auto corners1 = data.m_cornerData.corners().find(hook1);
		auto corners2 = data.m_cornerData.corners().find(hook2);

		/*初步修正箍筋标志未连接情况所用一下参数*/
		std::map<int, std::vector<Corner>> temp_corner1;
		std::map<int, std::vector<Corner>> temp_corner2;
		//绘图失误
		if (corners1 == data.m_cornerData.corners().end() || corners2 == data.m_cornerData.corners().end())
		{

			std::vector<Corner> extend_cor1;
			std::vector<Corner> extend_cor2;

			section.amendHooks(hook1, hook2, corners1, corners2, extend_cor1, extend_cor2, data);

			if (extend_cor1.empty() || extend_cor2.empty())
			{
				std::fstream of("error.txt", std::ios::app);
				of << "箍筋标志未找到，与之相连的箍" << std::endl;
				of.close();
				continue;
			}
			temp_corner1[hook1] = extend_cor1;
			temp_corner2[hook2] = extend_cor2;
			corners1 = temp_corner1.begin();
			corners2 = temp_corner2.begin();
		}

		//箍筋不会依次相连，但是肉眼直观感觉相连，对此情况做出判断
		if (corners1->second.size() == corners2->second.size())
		{
			//data.m_lineData.lines()[hook1];
			int max_index1 = hook1;
			int max_index2 = hook2;
			double max_length1 = -1;
			double max_length2 = -1;
			Point p1;
			Point p2;
			for (auto it1 : corners1->second)
			{
				auto l1 = it1.l1;
				if (l1 == corners1->first)
				{
					l1 = it1.l2;
				}
				if (data.m_lineData.lines()[l1].length() > max_length1)
				{
					max_length1 = data.m_lineData.lines()[l1].length();
					//max_index1 = l1;
					p1 = abs(TowPointsLength(it1, data.m_lineData.lines()[l1].s)) < abs(TowPointsLength(it1, data.m_lineData.lines()[l1].e)) ? data.m_lineData.lines()[l1].s : data.m_lineData.lines()[l1].e;
				}
			}
			for (auto it1 : corners2->second)
			{
				auto l1 = it1.l1;
				if (l1 == corners2->first)
				{
					l1 = it1.l2;
				}
				if (data.m_lineData.lines()[l1].length() > max_length2)
				{
					max_length2 = data.m_lineData.lines()[l1].length();
					//max_index2 = l1;
					p2 = abs(TowPointsLength(it1, data.m_lineData.lines()[l1].s)) < abs(TowPointsLength(it1, data.m_lineData.lines()[l1].e)) ? data.m_lineData.lines()[l1].s : data.m_lineData.lines()[l1].e;
				}
			}
			if (!ConnectPoint(p1, p2))
			{
				return false;
			}
			//Point cor1= TowPointsLength(,data.m_lineData.lines()[max_index1].s)
			//return false;
		}

		temp_stirrup.hook = it;
		temp_stirrup.single = false;
		auto corners = corners1->second.size() < corners2->second.size() ? corners1 : corners2; //以箍筋标志相交少的为起点（特点）
		int goal_index = corners->first;														//最终的线条索引

																								//查找箍筋内最合适的弯勾接引线（存在多条重复的箍筋线，找最长的为箍筋线）
		for (auto it1 : corners->second)
		{
			auto l1 = it1.l1;
			if (l1 == corners->first)
			{
				l1 = it1.l2;
			}

			if (find(goal_map.begin(), goal_map.end(), l1) != goal_map.end())
				continue;
			if (data.m_lineData.lines()[goal_index].length() < data.m_lineData.lines()[l1].length())
			{
				//goal_length = length;
				goal_index = l1;
			}
		}

		//查找与标志勾直接相连的短线，用做L形异形柱的排除
		std::vector<int>hook1_indexs;
		std::vector<int>hook2_indexs;
		for (auto it_hook : corners1->second)
		{
			auto l1 = it_hook.l1;
			if (l1 == corners1->first)
			{
				l1 = it_hook.l2;
			}
			if (ConnectPoint(it_hook, data.m_lineData.lines()[l1].s) ||
				ConnectPoint(it_hook, data.m_lineData.lines()[l1].e))
			{
				hook1_indexs.push_back(l1);
			}
		}
		for (auto it_hook : corners2->second)
		{
			auto l1 = it_hook.l1;
			if (l1 == corners2->first)
			{
				l1 = it_hook.l2;
			}
			if (ConnectPoint(it_hook, data.m_lineData.lines()[l1].s) ||
				ConnectPoint(it_hook, data.m_lineData.lines()[l1].e))
			{
				hook2_indexs.push_back(l1);
			}
		}

		//temp_index.push_back(goal_index);

		//temp_line.push_back(data.m_lineData.lines()[goal_index]);
		//压入箍筋线
		queue.push(goal_index);
		temp_stirrup.circle_line.push_back(data.m_lineData.lines()[goal_index]);
		goal_map.push_back(goal_index);

		//利用队列的特质，依次连接
		while (!queue.empty())
		{
			auto index = queue.front();
			queue.pop();
			corners = data.m_cornerData.corners().find(index);

			//对于L型柱的特殊处理
			bool bool_l = false;
			section.boolRectangle(&bool_l);
			std::vector<int>connect_start;
			std::vector<int>connect_end;
			for (auto it2 : corners->second)
			{
				auto l1 = it2.l1;
				if (l1 == index)
				{
					l1 = it2.l2;
				}
				//L型柱需要单独处理
				if (bool_l)
				{
					if ((ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].s) ||
						ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].e)) &&
						find(goal_map.begin(), goal_map.end(), l1) == goal_map.end())
					{
						//添加与hook相交的判断逻辑
						if (find(hook1_indexs.begin(), hook1_indexs.end(), l1) == hook1_indexs.end())
						{
							bool b_repeat = false;
							for (auto it1 : temp_stirrup.circle_line)
							{
								if (b_repeat = Line::bRepeatLine(it1, data.m_lineData.lines()[l1]) == true)
								{
									break;
								}
							}
							if (!b_repeat)
								connect_start.push_back(l1);
						}


					}

					if ((ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].s) ||
						ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].e)) &&
						find(goal_map.begin(), goal_map.end(), l1) == goal_map.end())
					{
						//添加与hook相交的判断逻辑
						if (find(hook2_indexs.begin(), hook2_indexs.end(), l1) == hook2_indexs.end())
						{
							bool b_repeat = false;
							for (auto it1 : temp_stirrup.circle_line)
							{
								if (b_repeat = Line::bRepeatLine(it1, data.m_lineData.lines()[l1]) == true)
								{
									break;
								}
							}
							if (!b_repeat)
								connect_end.push_back(l1);
						}
						//connect_end.push_back(l1);

					}

				}
				else
				{
					if ((ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].s) ||
						ConnectPoint(data.m_lineData.lines()[index].s, data.m_lineData.lines()[l1].e) ||
						ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].s) ||
						ConnectPoint(data.m_lineData.lines()[index].e, data.m_lineData.lines()[l1].e)) &&
						find(goal_map.begin(), goal_map.end(), l1) == goal_map.end())
					{

						temp_stirrup.circle_line.push_back(data.m_lineData.lines()[l1]);
						goal_map.push_back(l1);
						queue.push(l1);
					}

				}


			}
			if (bool_l)
			{
				if (connect_start.size() == 1)
				{
					//temp_index.push_back(connect_start.front());
					temp_stirrup.circle_line.push_back(data.m_lineData.lines()[connect_start.front()]);
					goal_map.push_back(connect_start.front());
					queue.push(connect_start.front());
				}
				if (connect_end.size() == 1)
				{
					//temp_index.push_back(connect_end.front());
					temp_stirrup.circle_line.push_back(data.m_lineData.lines()[connect_end.front()]);
					goal_map.push_back(connect_end.front());
					queue.push(connect_end.front());
				}
			}


		}

		section.stittups.push_back(temp_stirrup);
	}

	return true;
}

bool ColumnSection::DirecctionMatchingStirrup(ColumnSection &section, Data &data)
{
	std::vector<int> v_lines;
	std::vector<int> h_lines;
	std::vector<int> ss_lines;
	std::vector<Line> temp;
	double left = 0.0;
	double right = 0.0;
	double bottom = 0.0;
	double top = 0.0;
	typedef enum
	{
		L, //左
		R, //右
		U, //上
		D, //下
		N  //空
	} Direcction;
	typedef std::pair<int, int> Hook;
	typedef std::pair<int, int> Tib;
	std::vector<std::pair<Hook, std::pair<int, Direcction>>> h_l; //带有方向属性的横线
	std::vector<std::pair<Hook, std::pair<int, Direcction>>> v_l; //带有方向属性的纵线
	std::map<int, Direcction> h_dir;
	std::map<int, Direcction> v_dir;
	std::vector<std::pair<Tib, Hook>> stirrup_tib; //箍筋的脚筋索引 first竖 second横
	std::vector<int> dir_line;					   //标志被处理后的股线，防止多次被处理 direcction line(记录有方向属性的线)
	std::vector<int> tib;

	std::vector<std::pair<int, Point>> hook_goal;
	//
	data.m_lineData.lineTypeVHS(section.bp_surround, v_lines, h_lines, ss_lines);
	//防止空的判断
	if (v_lines.empty() || h_lines.empty())
		return true;
	//查找边界为后续确定箍筋范围做铺垫
	double max_num = section.section_block_box.left;
	double min_num = section.section_block_box.right;
	for (auto it : v_lines)
	{
		if (data.m_lineData.lines()[it].s.x < min_num)
		{
			min_num = data.m_lineData.lines()[it].s.x;
		}
		if (data.m_lineData.lines()[it].s.x > max_num)
		{
			max_num = data.m_lineData.lines()[it].s.x;
		}
	}

	left = min_num;
	right = max_num;
	max_num = section.section_block_box.bottom;
	min_num = section.section_block_box.top;
	for (auto it : h_lines)
	{
		if (data.m_lineData.lines()[it].s.y < min_num)
		{
			min_num = data.m_lineData.lines()[it].s.y;
		}
		if (data.m_lineData.lines()[it].s.y > max_num)
		{
			max_num = data.m_lineData.lines()[it].s.y;
		}
	}
	bottom = min_num;
	top = max_num;

	//根据hook为所相连的箍筋线添加方向属性
	for (auto it : section.hooks)
	{
		auto hook1 = it.hook_index1;
		auto hook2 = it.hook_index2;
		auto corners1 = data.m_cornerData.corners().find(hook1);
		auto corners2 = data.m_cornerData.corners().find(hook2);
		if (corners1 == data.m_cornerData.corners().end() || corners2 == data.m_cornerData.corners().end())
		{
			continue;
		}
		double max_length1 = -1;
		double max_length2 = -1;
		int max_index1 = hook1;
		int max_index2 = hook2;
		//箍筋端点
		Point p1; //箍筋线的起点
		Point p2;
		Point cor1; //标志勾交点
		Point cor2;
		for (auto it1 : corners1->second)
		{
			auto l1 = it1.l1;
			if (l1 == corners1->first)
			{
				l1 = it1.l2;
			}
			if (data.m_lineData.lines()[l1].length() > max_length1)
			{
				cor1 = it1;
				max_index1 = l1;
				max_length1 = data.m_lineData.lines()[l1].length();
				p1 = abs(TowPointsLength(it1, data.m_lineData.lines()[l1].s)) < abs(TowPointsLength(it1, data.m_lineData.lines()[l1].e)) ? data.m_lineData.lines()[l1].s : data.m_lineData.lines()[l1].e;
			}
		}
		for (auto it1 : corners2->second)
		{
			auto l1 = it1.l1;
			if (l1 == corners2->first)
			{
				l1 = it1.l2;
			}
			if (data.m_lineData.lines()[l1].length() > max_length2)
			{
				cor2 = it1;
				max_index2 = l1;
				max_length2 = data.m_lineData.lines()[l1].length();
				p2 = abs(TowPointsLength(it1, data.m_lineData.lines()[l1].s)) < abs(TowPointsLength(it1, data.m_lineData.lines()[l1].e)) ? data.m_lineData.lines()[l1].s : data.m_lineData.lines()[l1].e;
			}
		}
		//给出箍筋线方向（对单支股无效）
		/*
		-根据标志勾的朝向确定方向
		*/
		if (data.m_lineData.lines()[max_index1].vertical() && data.m_lineData.lines()[max_index2].horizontal()) //箍筋1竖，箍筋2横
		{
			if (ConnectPoint(p1, p2))
			{
				tib.push_back(max_index1);
				tib.push_back(max_index2);
				if (cor1 == data.m_lineData.lines()[hook1].s) //起点与终点的不同是x的不同
				{
					v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
						std::pair<int, Direcction>(max_index1, Direcction::R)));
					v_dir[max_index1] = Direcction::R;
				}
				else
				{
					v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
						std::pair<int, Direcction>(max_index1, Direcction::L)));
					v_dir[max_index1] = Direcction::L;
				}
				if (cor2.y == data.m_lineData.lines()[hook2].s.y) //上下的不同是y的不同
				{
					if (data.m_lineData.lines()[hook2].s.y > data.m_lineData.lines()[hook2].e.y)
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index2, Direcction::D)));
						h_dir[max_index2] = Direcction::D;
					}
					else
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index2, Direcction::U)));
						//dir_line.push_back(max_index2);
						h_dir[max_index2] = Direcction::U;
					}
				}
				else
				{
					if (data.m_lineData.lines()[hook2].s.y < data.m_lineData.lines()[hook2].e.y)
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index2, Direcction::D)));
						h_dir[max_index2] = Direcction::D;
					}
					else
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index2, Direcction::U)));
						h_dir[max_index2] = Direcction::U;
					}
				}
			}
			else
			{
				//镶嵌的情况
				if (abs(TowPointsLength(cor2, p1)) < abs(TowPointsLength(cor1, p2))) //判断谁是真正的可用的箍筋线
				{
					if (cor1 == data.m_lineData.lines()[hook1].s) //起点与终点的不同是x的不同
					{
						v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index1, Direcction::R)));
						v_dir[max_index1] = Direcction::R;
					}
					else
					{
						v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
							std::pair<int, Direcction>(max_index1, Direcction::L)));
						v_dir[max_index1] = Direcction::L;
					}
				}
				else
				{
					if (cor2.y == data.m_lineData.lines()[hook2].s.y) //上下的不同是y的不同
					{
						if (data.m_lineData.lines()[hook2].s.y > data.m_lineData.lines()[hook2].e.y)
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
								std::pair<int, Direcction>(max_index2, Direcction::D)));
							h_dir[max_index2] = Direcction::D;
						}
						else
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
								std::pair<int, Direcction>(max_index2, Direcction::U)));
							h_dir[max_index2] = Direcction::U;
						}
					}
					else
					{
						if (data.m_lineData.lines()[hook2].s.y < data.m_lineData.lines()[hook2].e.y)
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
								std::pair<int, Direcction>(max_index2, Direcction::D)));
							h_dir[max_index2] = Direcction::D;
						}
						else
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook1, hook2),
								std::pair<int, Direcction>(max_index2, Direcction::U)));
							h_dir[max_index2] = Direcction::U;
						}
					}
				}
			}
		}
		else if (data.m_lineData.lines()[max_index1].horizontal() && data.m_lineData.lines()[max_index2].vertical()) //箍筋1是横，箍筋2是竖
		{
			if (ConnectPoint(p1, p2))
			{
				tib.push_back(max_index1);
				tib.push_back(max_index2);
				if (cor2 == data.m_lineData.lines()[hook2].s) //起点与终点的不同是x的不同
				{
					v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
						std::pair<int, Direcction>(max_index2, Direcction::R)));
					v_dir[max_index2] = Direcction::R;
				}
				else
				{
					v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
						std::pair<int, Direcction>(max_index2, Direcction::L)));
					v_dir[max_index2] = Direcction::L;
				}
				if (cor1.y == data.m_lineData.lines()[hook1].s.y) //上下的不同是y的不同
				{
					if (data.m_lineData.lines()[hook1].s.y > data.m_lineData.lines()[hook1].e.y)
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index1, Direcction::D)));
						h_dir[max_index1] = Direcction::D;
					}
					else
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index1, Direcction::U)));
						dir_line.push_back(max_index1);
						h_dir[max_index1] = Direcction::U;
					}
				}
				else
				{
					if (data.m_lineData.lines()[hook1].s.y < data.m_lineData.lines()[hook1].e.y)
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index1, Direcction::D)));
						h_dir[max_index1] = Direcction::D;
					}
					else
					{
						h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index1, Direcction::U)));
						h_dir[max_index1] = Direcction::U;
					}
				}
			}
			else
			{
				if (abs(TowPointsLength(cor1, p2)) < abs(TowPointsLength(cor2, p1))) //判断谁是真正的可用的箍筋线
				{
					if (cor2 == data.m_lineData.lines()[hook2].s) //起点与终点的不同是x的不同
					{
						v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index2, Direcction::R)));

						v_dir[max_index2] = Direcction::R;
					}
					else
					{
						v_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
							std::pair<int, Direcction>(max_index2, Direcction::L)));

						v_dir[max_index2] = Direcction::L;
					}
				}
				else
				{
					if (cor1.y == data.m_lineData.lines()[hook1].s.y) //上下的不同是y的不同
					{
						if (data.m_lineData.lines()[hook1].s.y > data.m_lineData.lines()[hook1].e.y)
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
								std::pair<int, Direcction>(max_index1, Direcction::D)));

							h_dir[max_index1] = Direcction::D;
						}
						else
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
								std::pair<int, Direcction>(max_index1, Direcction::U)));

							h_dir[max_index1] = Direcction::U;
						}
					}
					else
					{
						if (data.m_lineData.lines()[hook1].s.y < data.m_lineData.lines()[hook1].e.y)
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
								std::pair<int, Direcction>(max_index1, Direcction::D)));

							h_dir[max_index1] = Direcction::D;
						}
						else
						{
							h_l.push_back(std::pair<Hook, std::pair<int, Direcction>>(Hook(hook2, hook1),
								std::pair<int, Direcction>(max_index1, Direcction::U)));

							h_dir[max_index1] = Direcction::U;
						}
					}
				}
			}
		}
	}

	//候选箍筋
	int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bottom);
	int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, top);
	int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, left, Precision, false);
	int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, right, Precision, false);
	std::vector<int> v_stirrup;
	std::vector<int> h_stirrup;
	std::vector<int> make_line;
	for (int i = vf; i <= vt; ++i)
	{
		auto index = data.m_lineData.vLinesIndices()[i];

		if (data.m_lineData.lines()[index].s.y - Precision > data.m_lineData.lines()[data.m_lineData.hLinesIndices()[ht]].s.y ||
			data.m_lineData.lines()[index].e.y + Precision < data.m_lineData.lines()[data.m_lineData.hLinesIndices()[hf]].s.y)
			continue;
		v_stirrup.push_back(index);
		//temp_index.push_back(index);
	}
	for (int i = hf; i <= ht; ++i)
	{
		auto index = data.m_lineData.hLinesIndices()[i];
		if (data.m_lineData.lines()[index].s.x - Precision > data.m_lineData.lines()[data.m_lineData.vLinesIndices()[vt]].e.x ||
			data.m_lineData.lines()[index].e.x + Precision < data.m_lineData.lines()[data.m_lineData.vLinesIndices()[vf]].s.x)
			continue;
		h_stirrup.push_back(index);
		//temp_index.push_back(index);
	}
	//为候选箍筋排序
	for (int i = 0; i < v_stirrup.size(); ++i)
	{
		int temp_line;
		for (int j = 0; j < v_stirrup.size() - i - 1; ++j)
		{
			if (!compareRefV(data.m_lineData.lines(), v_stirrup[j], v_stirrup[j + 1]))
			{
				temp_line = v_stirrup[j];
				v_stirrup[j] = v_stirrup[j + 1];
				v_stirrup[j + 1] = temp_line;
			}
		}
	}
	for (int i = 0; i < h_stirrup.size(); ++i)
	{
		int temp_line;
		for (int j = 0; j < h_stirrup.size() - i - 1; ++j)
		{
			if (!compareRefH(data.m_lineData.lines(), h_stirrup[j], h_stirrup[j + 1]))
			{
				temp_line = h_stirrup[j];
				h_stirrup[j] = h_stirrup[j + 1];
				h_stirrup[j + 1] = temp_line;
			}
		}
	}
	//配对
	//横向箍筋

	std::stack<std::pair<int, Direcction>> h_stack;
	for (auto it : h_stirrup)
	{
		//栈为空 压栈
		if (h_stack.empty())
		{
			if (h_dir.find(h_stirrup.front()) == h_dir.end())
			{
				h_stack.push(std::pair<int, Direcction>(h_stirrup.front(), Direcction::N));
			}
			else
			{
				h_stack.push(std::pair<int, Direcction>(h_stirrup.front(), h_dir[h_stirrup.front()]));
			}
		} //栈匹配
		else
		{
			std::pair<int, Direcction> temp_stir = h_stack.top();
			if (temp_stir.second == Direcction::N)
			{
				if (h_dir.find(it) == h_dir.end())
				{
					h_stack.push(std::pair<int, Direcction>(it, Direcction::N));
				}
				else if (h_dir[it] == Direcction::D)
				{
					//循环出栈
					while (!h_stack.empty())
					{

						if (data.m_lineData.lines()[it].length() + Precision > data.m_lineData.lines()[temp_stir.first].length() &&
							data.m_lineData.lines()[it].length() - Precision < data.m_lineData.lines()[temp_stir.first].length())
						{
							for (auto it1 : h_l)
							{
								if (it == it1.second.first)
								{
									//避免重复性压入（主要是最外箍筋的情况）
									bool mark = false;
									for (int i = 0; i < section.stittups.size(); ++i)
									{
										if (section.stittups[i].hook == ColumnStirrupHook(it1.first.first, it1.first.second /*, false*/))
										{
											section.stittups[i].circle_line.push_back(data.m_lineData.lines()[it]);
											section.stittups[i].circle_line.push_back(data.m_lineData.lines()[temp_stir.first]);
											mark = true;
											break;
										}
									}

									if (mark)
										break;

									std::vector<Line> temp_lins;
									ColumnStirrupHook temp_hook(it1.first.first, it1.first.second /*, false*/);
									temp_lins.push_back(data.m_lineData.lines()[it]);
									temp_lins.push_back(data.m_lineData.lines()[temp_stir.first]);
									section.stittups.push_back(ColumnSStirrup(temp_lins, temp_hook, false));
									break;
								}
							}
							h_stack.pop();
							break;
						}
						h_stack.pop();
						if (!h_stack.empty())
							temp_stir = h_stack.top();
					}
				}
				else if (h_dir[it] == Direcction::U)
				{
					h_stack.push(std::pair<int, Direcction>(it, Direcction::U));
				}
			}
			else if (temp_stir.second == Direcction::U)
			{
				if (h_dir.find(it) == h_dir.end())
				{
					if (data.m_lineData.lines()[it].length() + Precision > data.m_lineData.lines()[temp_stir.first].length() &&
						data.m_lineData.lines()[it].length() - Precision < data.m_lineData.lines()[temp_stir.first].length())
					{
						for (auto it1 : h_l)
						{
							if (temp_stir.first == it1.second.first)
							{

								bool mark = false;
								for (int i = 0; i < section.stittups.size(); ++i)
								{
									if (section.stittups[i].hook == ColumnStirrupHook(it1.first.first, it1.first.second /*, false*/))
									{
										section.stittups[i].circle_line.push_back(data.m_lineData.lines()[it]);
										section.stittups[i].circle_line.push_back(data.m_lineData.lines()[temp_stir.first]);
										mark = true;
										break;
									}
								}

								if (mark)
									break;

								std::vector<Line> temp_lins;
								ColumnStirrupHook temp_hook(it1.first.first, it1.first.second /*, false*/);
								temp_lins.push_back(data.m_lineData.lines()[it]);
								temp_lins.push_back(data.m_lineData.lines()[temp_stir.first]);
								section.stittups.push_back(ColumnSStirrup(temp_lins, temp_hook, false));
								break;
							}
						}
						h_stack.pop();
					}
					else
					{
						continue;
					}
				}
				else
				{
					if (h_dir[it] == Direcction::U)
					{
						h_stack.push(std::pair<int, Direcction>(it, Direcction::U));
					}
					else
					{
						; //报错
					}
				}
			}
			//一条费语句理论上永远不能能执行
			else if (temp_stir.second == Direcction::D)
			{
				;
			}
		}
	}

	//配对纵筋

	std::stack<std::pair<int, Direcction>> v_stack;
	for (auto it : v_stirrup)
	{
		//栈为空 压栈
		if (v_stack.empty())
		{
			if (v_dir.find(v_stirrup.front()) == v_dir.end())
			{
				v_stack.push(std::pair<int, Direcction>(v_stirrup.front(), Direcction::N));
			}
			else
			{
				v_stack.push(std::pair<int, Direcction>(v_stirrup.front(), v_dir[v_stirrup.front()]));
			}
		} //栈匹配
		else
		{
			std::pair<int, Direcction> temp_stir = v_stack.top();
			if (temp_stir.second == Direcction::N)
			{
				if (v_dir.find(it) == v_dir.end())
				{
					v_stack.push(std::pair<int, Direcction>(it, Direcction::N));
				}
				else if (v_dir[it] == Direcction::L)
				{
					while (!v_stack.empty())
					{
						if (data.m_lineData.lines()[it].length() + Precision > data.m_lineData.lines()[temp_stir.first].length() &&
							data.m_lineData.lines()[it].length() - Precision < data.m_lineData.lines()[temp_stir.first].length())
						{
							for (auto it1 : v_l)
							{
								if (it == it1.second.first)
								{
									bool mark = false;
									for (int i = 0; i < section.stittups.size(); ++i)
									{
										if (section.stittups[i].hook == ColumnStirrupHook(it1.first.first, it1.first.second /*, false*/))
										{
											section.stittups[i].circle_line.push_back(data.m_lineData.lines()[it]);
											section.stittups[i].circle_line.push_back(data.m_lineData.lines()[temp_stir.first]);
											mark = true;
											break;
										}
									}

									if (mark)
										break;
									std::vector<Line> temp_lins;
									ColumnStirrupHook temp_hook(it1.first.first, it1.first.second /*, false*/);
									temp_lins.push_back(data.m_lineData.lines()[it]);
									temp_lins.push_back(data.m_lineData.lines()[temp_stir.first]);
									section.stittups.push_back(ColumnSStirrup(temp_lins, temp_hook, false));
									break;
								}
							}
							v_stack.pop();
							break;
						}
						v_stack.pop();
						if (!v_stack.empty())
							temp_stir = v_stack.top();
					}
				}
				else if (h_dir[it] == Direcction::R)
				{
					v_stack.push(std::pair<int, Direcction>(it, Direcction::U));
				}
			}
			else if (temp_stir.second == Direcction::R)
			{
				if (v_dir.find(it) == v_dir.end())
				{
					double length1 = data.m_lineData.lines()[it].length();
					double length2 = data.m_lineData.lines()[temp_stir.first].length();
					if (data.m_lineData.lines()[it].length() + Precision > data.m_lineData.lines()[temp_stir.first].length() &&
						data.m_lineData.lines()[it].length() - Precision < data.m_lineData.lines()[temp_stir.first].length())
					{
						for (auto it1 : v_l)
						{
							if (temp_stir.first == it1.second.first)
							{
								bool mark = false;
								for (int i = 0; i < section.stittups.size(); ++i)
								{
									if (section.stittups[i].hook == ColumnStirrupHook(it1.first.first, it1.first.second /*, false*/))
									{
										section.stittups[i].circle_line.push_back(data.m_lineData.lines()[it]);
										section.stittups[i].circle_line.push_back(data.m_lineData.lines()[temp_stir.first]);
										mark = true;
										break;
									}
								}

								if (mark)
									break;
								std::vector<Line> temp_lins;
								ColumnStirrupHook temp_hook(it1.first.first, it1.first.second /*, false*/);
								temp_lins.push_back(data.m_lineData.lines()[it]);
								temp_lins.push_back(data.m_lineData.lines()[temp_stir.first]);
								section.stittups.push_back(ColumnSStirrup(temp_lins, temp_hook, false));
								break;
							}
						}
						v_stack.pop();
					}
					else
					{
						continue;
					}
				}
				else
				{
					if (v_dir[it] == Direcction::R)
					{
						v_stack.push(std::pair<int, Direcction>(it, Direcction::R));
					}
					else
					{
						; //报错
					}
				}

			} //理论上永远不会执行这一步
			else if (temp_stir.second == Direcction::L)
			{
				;
			}
		}
	}

	//再次整理箍筋对只有两只股的情况做补全（即：两横缺两竖，两竖缺两横）
	for (int i = 0; i < section.stittups.size(); ++i)
	{
		if (section.stittups[i].circle_line.size() == 2)
		{
			if (section.stittups[i].circle_line[0].vertical() &&
				section.stittups[i].circle_line[1].vertical())
			{
				Line temp_line1(Line(section.stittups[i].circle_line[0].s,
					section.stittups[i].circle_line[1].s));
				Line temp_line2(Line(section.stittups[i].circle_line[0].e,
					section.stittups[i].circle_line[1].e));

				section.stittups[i].circle_line.push_back(temp_line1);
				section.stittups[i].circle_line.push_back(temp_line2);
			}
			else if (section.stittups[i].circle_line[0].horizontal() &&
				section.stittups[i].circle_line[1].horizontal())
			{
				Line temp_line1(Line(section.stittups[i].circle_line[0].s,
					section.stittups[i].circle_line[1].s));
				Line temp_line2(Line(section.stittups[i].circle_line[0].e,
					section.stittups[i].circle_line[1].e));

				section.stittups[i].circle_line.push_back(temp_line1);
				section.stittups[i].circle_line.push_back(temp_line2);
			}
		}
	}

	return true;
}

bool ColumnSection::CheckStirrup(ColumnSection &section, Data &data)
{
	if (section.stittups.empty())
		return true;
	//std::vector<Line>lines;
	std::vector<Line> h_lines;
	std::vector<Line> v_lines;
	std::vector<Line> s_lines;
	//获取所有股线
	for (auto it : section.stittups)
	{

		for (auto it1 : it.circle_line)
		{
			if (it1.horizontal())
			{
				h_lines.push_back(it1);
			}
			else if (it1.vertical())
			{
				v_lines.push_back(it1);
			}
			else
			{
				s_lines.push_back(it1);
			}
		}
	}
	//对已排序的箍筋进行排序
	data.m_lineData.linesSort(v_lines, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	data.m_lineData.linesSort(h_lines, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	//对已有箍筋进行合并
	data.m_lineData.mergeLines(v_lines, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	data.m_lineData.mergeLines(h_lines, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	//规约遗漏的箍筋 主要是单只箍 和未找到箍筋的hook

	//一，剔除斜线中是箍筋标志的斜线 用于去判断是否是异形柱
	bool alien = false;
	double compare_line = data.m_lineData.lines()[section.stittups.front().hook.hook_index1].length();
	for (auto it : s_lines)
	{
		if (it.length() > 3 * compare_line)
		{
			alien = true;
		}
	}


	if (!alien)
	{
		//纵箍
		for (int i = 0; i < v_lines.size() - 1; ++i)
		{
			if (abs(v_lines[i].length() - v_lines[i + 1].length()) > Precision * 2)
			{
				continue;
			}
			int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, v_lines[i].s.x, Precision, false);
			int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, v_lines[i + 1].s.x, Precision, false);
			for (int j = vf; j < vt; ++j)
			{
				int index = data.m_lineData.vLinesIndices()[j];
				if ((abs(v_lines[i].s.x - data.m_lineData.lines()[index].s.x) < Precision) ||
					(abs(v_lines[i + 1].s.x - data.m_lineData.lines()[index].s.x) < Precision) ||
					data.m_lineData.lines()[index].e.y - Precision > v_lines[i].e.y ||
					data.m_lineData.lines()[index].s.y + Precision < v_lines[i].s.y ||
					/*data.m_lineData.lines()[index].s.x - Precision < v_lines[i].s.x ||
					data.m_lineData.lines()[index].s.x + Precision > v_lines[i].s.x ||*/
					data.m_lineData.lines()[index].length() < v_lines[i].length() * 0.9)
				{
					continue;
				}
				section.stittups.push_back(data.m_lineData.lines()[index]);
			}
		}
		//横箍
		for (int i = 0; i < h_lines.size() - 1; ++i)
		{
			if (abs(h_lines[i].length() - h_lines[i + 1].length()) > Precision * 2)
			{
				continue;
			}
			int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, h_lines[i].s.y);
			int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, h_lines[i + 1].s.y);

			for (int j = hf; j < ht; ++j)
			{
				int index = data.m_lineData.hLinesIndices()[j];
				if ((abs(h_lines[i].s.y - data.m_lineData.lines()[index].s.y) < Precision) ||
					(abs(h_lines[i + 1].s.y - data.m_lineData.lines()[index].s.y) < Precision) ||
					data.m_lineData.lines()[index].e.x - Precision > h_lines[i].e.x ||
					data.m_lineData.lines()[index].s.x + Precision < h_lines[i].s.x ||

					data.m_lineData.lines()[index].length() < h_lines[i].length() * 0.9)
				{
					continue;
				}
				section.stittups.push_back(data.m_lineData.lines()[index]);
			}
		}

	}
	else
	{
		//设定闭合区间
		ClosedZone borderline;
		b_closedzone(section.bp_surround, borderline, data);


		Box section_box = section.section_box;
		//候选箍筋
		SetBlockVHIndex(section_box, data.m_lineData);
		int hf = section_box.hf;
		int ht = section_box.ht;
		int vf = section_box.vf;
		int vt = section_box.vt;
		std::vector<Line>h_compare;
		std::vector<Line>v_compare;
		for (auto it : borderline.borderline)
		{
			if (!data.m_lineData.lines()[it.first].vertical())
			{
				v_compare.push_back(data.m_lineData.lines()[it.first]);
			}

			if (!data.m_lineData.lines()[it.first].horizontal())
			{
				h_compare.push_back(data.m_lineData.lines()[it.first]);
			}
		}
		for (int i = hf; i <= ht; ++i)
		{
			int index = data.m_lineData.hLinesIndices()[i];
			std::vector<double>cornersX;
			if (b_InClosedZone(index, bp_surround, borderline, data) == true && data.m_lineData.lines()[index].length() > compare_line * 3)
			{
				bool b_being = false;
				for (auto it : h_compare)
				{
					//if (data.m_lineData.lines()[index].s == it.s&&data.m_lineData.lines()[index].e == it.e)
					//{
					//	b_being = true;
					//	cornersX.clear();
					//	break;
					//}
					//else//求交点
					//{
					if (!it.horizontal() && !(
						(it.s.y > data.m_lineData.lines()[index].s.y&&it.e.y > data.m_lineData.lines()[index].e.y) ||
						(it.s.y < data.m_lineData.lines()[index].s.y&&it.e.y < data.m_lineData.lines()[index].e.y)))
					{
						cornersX.push_back(knowYToX(it, data.m_lineData.lines()[index].s.y));
					}
					//}
				}
				if (!b_being&&cornersX.size() == 2 && data.m_lineData.lines()[index].length() > abs(cornersX.front() - cornersX.back())*0.8
					/*data.m_lineData.lines()[index].length() > abs(section_box.left - section_box.right)*0.5*/)
				{
					//temp_index.push_back(index);
					section.stittups.push_back(data.m_lineData.lines()[index]);

				}
			}

		}
		for (int i = vf; i <= vt; ++i)
		{
			int index = data.m_lineData.vLinesIndices()[i];
			std::vector<double>cornersY;
			if (b_InClosedZone(index, bp_surround, borderline, data) == true && data.m_lineData.lines()[index].length() > compare_line * 3)
			{
				bool b_being = false;
				for (auto it : v_compare)
				{
					//if (data.m_lineData.lines()[index].s == it.s&&data.m_lineData.lines()[index].e == it.e)
					//{
					//	b_being = true;
					//	start_corner = Point(0, 0);
					//	end_corner = Point(0, 0);
					//	break;
					//}
					//else//求交点
					//{
					if (!it.vertical() && !(
						(it.s.x > data.m_lineData.lines()[index].s.x&&it.e.x > data.m_lineData.lines()[index].e.x) ||
						(it.s.x < data.m_lineData.lines()[index].s.x&&it.e.x < data.m_lineData.lines()[index].e.x)))
					{
						cornersY.push_back(knowXToY(it, data.m_lineData.lines()[index].s.x));
					}
					//}
				}
				if (!b_being&&cornersY.size() == 2 && data.m_lineData.lines()[index].length() > abs(cornersY.front() - cornersY.back())*0.8
					/*&&data.m_lineData.lines()[index].length() > abs(section_box.bottom - section_box.top)*0.7*/)
				{
					//temp_index.push_back(index);
					section.stittups.push_back(data.m_lineData.lines()[index]);
				}
			}
		}
	}



	return true;
}

bool ColumnSection::FindLongitudinalTendonCollection(const std::vector<Point> &center_mind,
	std::vector<std::vector<int>> &connect_line, Data &data)
{
	std::vector<int> lead_line;
	for (auto it : center_mind)
	{
		//std::vector<int>lead_line;
		auto ends = data.m_kdtTreeData.kdtEndpoints().radiusSearch(Endpoint(0, it), 45 /*temp_radius*1.5*/);
		for (auto end : ends)
		{
			auto e = data.m_endpointData.getEndpoint(end);
			if (data.m_lineData.lines()[e.index].length() < 90 || data.m_lineData.lines()[e.index].isSide())
				continue;
			else
			{

				auto itea = find(lead_line.begin(), lead_line.end(), e.index);
				bool mark = false;
				int mark_num = -1;
				if (itea == lead_line.end())
				{
					lead_line.push_back(e.index);
					mark = true;
					mark_num = e.index;
				}
				else
				{
					continue;
				}

				auto corners = data.m_cornerData.corners().find(e.index);
				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == e.index)
					{
						l1 = corner.l2;
					}
					auto point = data.m_lineData.lines()[e.index].s;
					if (point == e)
					{
						point = data.m_lineData.lines()[e.index].e;
					}

					/*if ((point.x + Precision * 6 > data.m_lineData.lines()[l1].s.x &&
					point.x - Precision * 6 < data.m_lineData.lines()[l1].s.x) ||
					(point.y + Precision * 6 > data.m_lineData.lines()[l1].s.y &&
					point.y - Precision * 6 < data.m_lineData.lines()[l1].s.y))*/
					if ((point.x + Precision > data.m_lineData.lines()[l1].s.x &&
						point.x - Precision < data.m_lineData.lines()[l1].s.x) ||
						(point.y + Precision > data.m_lineData.lines()[l1].s.y &&
							point.y - Precision < data.m_lineData.lines()[l1].s.y))
					{
						itea = find(lead_line.begin(), lead_line.end(), l1);
						if (itea == lead_line.end())
						{
							lead_line.push_back(l1);
							std::vector<int> temp;
							mark = false;
							temp.push_back(l1);
							temp.push_back(mark_num);
							connect_line.push_back(temp);
						}
						else
						{
							for (auto it1 = 0; it1 < connect_line.size(); ++it1)
							{
								auto itea = find(connect_line[it1].begin(), connect_line[it1].end(), l1);
								if (itea != connect_line[it1].end())
								{
									mark = false;
									connect_line[it1].push_back(mark_num);
								}
							}
						}
					}
				}
			}
		}
	}
	return true;
}

bool ColumnSection::SetLTC(std::vector<std::vector<int>>& l_t_c, const std::vector<std::vector<int>>& l_t_c1)
{
	l_t_c = l_t_c1;
	return true;
}

bool ColumnSection::FindExplanation(const std::vector<std::vector<int>>&lines_index, const Box&temp_box, const Box&block_box,
	std::vector<std::pair<ColumnSection::Explanation, Point>>&explanation, Data &data)
{
	for (auto it : lines_index)
	{
		Point goal_point;//查找本文的目标点
		auto mark = RetrunLeadLineType(it, temp_box, goal_point, data);

		std::string name;
		//此处需外加额外的判断，例如多识别几个点，从里面做筛选
		auto texts = data.m_kdtTreeData.kdtTexts().knnSearch(goal_point, 1);

		for (auto it1 : texts)
		{
			auto point = data.m_textPointData.textpoints()[it1];
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			name = tt->text;
		}
		explanation.push_back(std::pair<ColumnSection::Explanation, Point>(ColumnSection::Explanation(name, it), goal_point));

	}

	return true;
}

bool ColumnSection::RetrunLeadLineType(const std::vector<int> &index, const Box &temp_box, Point &goal_point, Data &data)
{
	//int goal_index;
	Point temp_point;
	for (auto it : index)
	{
		bool p_cover = true;
		Point l1; //默认将了l1当作覆盖的点
		Point l2;
		l1 = data.m_lineData.lines()[it].s;
		l2 = data.m_lineData.lines()[it].e;
		if (index.size() > 1)
		{
			if (!temp_box.cover(l1) && !temp_box.cover(l2))
			{
				//goal_index = it;
				auto corners = data.m_cornerData.corners().find(it);
				auto it1 = corners->second;

				if (cornerFindPoint(it1, l1) && cornerFindPoint(it1, l2))
				{
					if (Line::vertical(l1, l2))
					{
						goal_point = Point(l1.x, (l1.y + l2.y) / 2);
					}
					else
					{
						goal_point = Point((l1.x + l2.x) / 2, l1.y);
					}
					return true;
				}
				else
				{
					if (!cornerFindPoint(it1, l1))
					{
						goal_point = l1;
					}
					else
					{
						goal_point = l2;
					}
					return false;
				}
			}
		}
		else
		{
			//goal_index = it;
			if ((!temp_box.cover(l1) && temp_box.cover(l2)) || (temp_box.cover(l1) && !temp_box.cover(l2)))
			{
				if (!temp_box.cover(l1))
				{
					goal_point = l1;
				}
				else
				{
					goal_point = l2;
				}
			}
			return false;
		}
	}
}

bool ColumnSection::SetSectionExplanation(std::vector<std::pair<ColumnSection::Explanation, Point>>& explanation,
	const std::vector<std::pair<ColumnSection::Explanation, Point>>& lead_line_explanation)
{
	explanation = lead_line_explanation;
	return true;
}

bool ColumnSection::CenterNote(ColumnSection &section, Data &data)
{
	//pillar surround

	std::vector<int> ph_index;
	std::vector<int> pv_index;
	std::vector<int> ps_index;
	data.m_lineData.lineTypeVHS(section.pillar_surround, pv_index, ph_index, ps_index);
	if (!ps_index.empty() || pv_index.empty() || ph_index.empty())
		return true;
	std::vector<int> sh_index;
	std::vector<int> sv_index;
	std::vector<int> ss_index;
	data.m_lineData.lineTypeVHS(section.bp_surround, sv_index, sh_index, ss_index);
	double text_height = 0.0;
	sv_index.clear(), ss_index.clear();
	data.m_lineData.linesSortIndex(ph_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	data.m_lineData.linesSortIndex(pv_index, LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX);
	data.m_lineData.linesSortIndex(sh_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	data.m_lineData.mergeLinesIndex(sh_index, LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX);
	int goal_index = -1; //集中标注的引出线一般会在柱包围线上部
	int num = sh_index.back();
	for (auto it : ph_index)
	{
		if (data.m_lineData.lines()[it].s.y > data.m_lineData.lines()[num].s.y &&
			data.m_lineData.lines()[it].s.x < data.m_lineData.lines()[num].s.x &&
			data.m_lineData.lines()[it].e.x > data.m_lineData.lines()[num].e.x)
		{
			goal_index = it;
		}
	}
	if (goal_index == -1 && !ph_index.empty())
	{
		goal_index = ph_index.back();
	}
	auto corners = data.m_cornerData.corners().find(goal_index);
	std::vector<int> candidate_lead;
	for (auto corner : corners->second)
	{
		auto l1 = corner.l1;
		if (l1 == goal_index)
		{
			l1 = corner.l2;
		}
		if (data.m_lineData.lines()[l1].vertical())
		{
			bool mark = false;
			auto temp_corners = data.m_cornerData.corners().find(l1);
			for (auto it : temp_corners->second)
			{
				/*auto l2 = it.l1;
				if (l2 == l1)
				{
				l2 = it.l2;
				}*/
				if (ConnectPoint(data.m_lineData.lines()[l1].e, Point(it.x, it.y)))
				{
					mark = true;
					break;
				}
			}
			if (mark)
			{
				continue;
			}
			else
			{
				candidate_lead.push_back(l1);
				//temp_index.push_back(l1);
			}
		}
		else if (!data.m_lineData.lines()[l1].horizontal()) //斜线很有可能事引出线
		{
			auto temp_corners = data.m_cornerData.corners().find(l1);
			for (auto it : temp_corners->second)
			{
				auto l2 = it.l1;
				if (l2 == l1)
				{
					l2 = it.l2;
				}
				int num = ph_index.back();
				Point p = data.m_lineData.lines()[l1].s.y > data.m_lineData.lines()[l1].e.y ? data.m_lineData.lines()[l1].s : data.m_lineData.lines()[l1].e;
				if (data.m_lineData.lines()[l2].horizontal() && (ConnectPoint(p, Point(it.x, it.y))))
				{
					candidate_lead.push_back(l2);
					//temp_index.push_back(l2);
				}
			}
		}
	}
	//std::fstream of("柱信息.txt", std::ios::app);
	if (candidate_lead.size() == 1 && data.m_lineData.lines()[candidate_lead.front()].vertical())
	{
		Point p = Point((data.m_lineData.lines()[candidate_lead[0]].s.x + data.m_lineData.lines()[candidate_lead[0]].e.x) / 2,
			(data.m_lineData.lines()[candidate_lead[0]].s.y + data.m_lineData.lines()[candidate_lead[0]].e.y) / 2);
		double r = data.m_lineData.lines()[candidate_lead[0]].length() / 2;
		auto cands = data.m_kdtTreeData.kdtTexts().radiusSearch(p, r); //以p点为圆心，16为半径搜索，文本点
		for (auto c : cands)
		{
			Point pt = data.m_kdtTreeData.kdtTexts().points_[c];
			DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
			if (text_height == 0.0)
			{
				text_height = tt->height;
			}
			if (tt->basePoint.x > data.m_lineData.lines()[candidate_lead[0]].s.x)
			{
				//of << tt->text << std::endl;
				section.all_txt.push_back(tt->text);
			}
			tt->reset();
		}
	}
	else if (candidate_lead.size() == 1 && data.m_lineData.lines()[candidate_lead.front()].horizontal())
	{

		auto cands = data.m_kdtTreeData.kdtTexts().knnSearch(data.m_lineData.lines()[candidate_lead[0]].s, 1); //搜索最近的一个文本点
		Point pt = data.m_kdtTreeData.kdtTexts().points_[cands[0]];
		DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
		double r = tt->height * 7;
		if (text_height == 0.0)
		{
			text_height = tt->height;
		}
		cands.clear();
		cands = data.m_kdtTreeData.kdtTexts().radiusSearch(data.m_lineData.lines()[candidate_lead[0]].s, r); //以p点为圆心，r为半径搜索，文本点
		for (auto c : cands)
		{
			Point pt = data.m_kdtTreeData.kdtTexts().points_[c];
			DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
			if (tt->basePoint.x > data.m_lineData.lines()[candidate_lead[0]].s.x &&
				tt->basePoint.y > data.m_lineData.lines()[candidate_lead[0]].s.y)
			{
				//of << tt->text << std::endl;
				section.all_txt.push_back(tt->text);
			}
			tt->reset();
		}
	}

	//搜索柱外边界信息
	Point pt;
	//左
	Point l_p = Point(data.m_lineData.lines()[pv_index.front()].s.x, (data.m_lineData.lines()[pv_index.front()].s.y + data.m_lineData.lines()[pv_index.front()].e.y) / 2);
	auto l_cands = data.m_kdtTreeData.kdtTexts().knnSearch(l_p, 1); //搜索最近的一个文本点
	pt = data.m_kdtTreeData.kdtTexts().points_[l_cands[0]];
	num = pv_index.front();
	if (abs(pt.x - data.m_lineData.lines()[num].s.x) < text_height * 2 &&
		pt.y > data.m_lineData.lines()[num].s.y && pt.y < data.m_lineData.lines()[num].e.y)
	{

		DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
		section.bp_string.push_back(std::pair<std::string,
			ColumnSection::Direcction>(tt->text, ColumnSection::Direcction::L));
		//of << tt->text << endl;
	}

	//右
	Point r_p = Point(data.m_lineData.lines()[pv_index.back()].s.x, (data.m_lineData.lines()[pv_index.back()].s.y + data.m_lineData.lines()[pv_index.back()].e.y) / 2);
	auto r_cands = data.m_kdtTreeData.kdtTexts().knnSearch(r_p, 1);
	pt = data.m_kdtTreeData.kdtTexts().points_[r_cands[0]];
	num = pv_index.back();
	if (abs(pt.x - data.m_lineData.lines()[num].s.x) < text_height * 2 &&
		pt.y > data.m_lineData.lines()[num].s.y && pt.y < data.m_lineData.lines()[num].e.y)
	{
		DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
		section.bp_string.push_back(std::pair<std::string,
			ColumnSection::Direcction>(tt->text, ColumnSection::Direcction::R));
		//of << tt->text << endl;
	}

	//上
	Point u_p = Point((data.m_lineData.lines()[ph_index.back()].s.x + data.m_lineData.lines()[ph_index.back()].e.x) / 2, data.m_lineData.lines()[ph_index.back()].s.y);
	auto u_cands = data.m_kdtTreeData.kdtTexts().knnSearch(u_p, 1);
	pt = data.m_kdtTreeData.kdtTexts().points_[u_cands[0]];
	num = ph_index.back();
	if (abs(pt.y - data.m_lineData.lines()[num].s.y) < text_height * 2 &&
		pt.x > data.m_lineData.lines()[num].s.x && pt.x < data.m_lineData.lines()[num].e.x)
	{
		DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
		section.bp_string.push_back(std::pair<std::string,
			ColumnSection::Direcction>(tt->text, ColumnSection::Direcction::U));
		//of << tt->text << endl;
	}

	//下
	Point b_p = Point((data.m_lineData.lines()[ph_index.front()].s.x + data.m_lineData.lines()[ph_index.front()].e.x) / 2, data.m_lineData.lines()[ph_index.front()].s.y);
	auto b_cands = data.m_kdtTreeData.kdtTexts().knnSearch(b_p, 1);
	pt = data.m_kdtTreeData.kdtTexts().points_[b_cands[0]];
	num = ph_index.front();
	if (abs(pt.y - data.m_lineData.lines()[num].s.y) < text_height * 2 &&
		pt.x > data.m_lineData.lines()[num].s.x && pt.x < data.m_lineData.lines()[num].e.x)
	{
		DRW_Text *tt = static_cast<DRW_Text *>(pt.entity.get());
		section.bp_string.push_back(std::pair<std::string,
			ColumnSection::Direcction>(tt->text, ColumnSection::Direcction::D));
		//of << tt->text << endl;
	}

	//of.close();

	return true;
}

ColumnSection ColumnSection::retrunColumnSection(Block& block, Data& data)
{
	ColumnSection temp_columnSection;
	//初始化柱截面块
	temp_columnSection.iniColumnBlock(block);
	//!初始化柱截面的所有纵筋断点
	temp_columnSection.iniBreakPoint(block, data);
	//初始化箍筋信息
	temp_columnSection.iniStirrup(data);
	//!初始化纵筋的引线组
	temp_columnSection.findBpLeadLine(data);
	//!获取标注箍筋信息    （箍筋信息出错）
	temp_columnSection.iniColumnStirLead(data);
	//!获取钢筋信息，箍筋的，纵筋的
	temp_columnSection.iniRebarInfo(data);
	//!排序纵筋
	temp_columnSection.sortBreakPointToLine();
	//!截面尺寸
	//!初始化梁断面块内的所有标注,以及断面尺寸
	temp_columnSection.iniColumnDImensions(data, data.m_dimensions);
	temp_columnSection.iniMaxSize();

	return temp_columnSection;
}

bool ColumnSection::testFuction(std::vector<int>& test_indexVec, std::vector<Point>& cornerVec)
{

	test_indexVec.insert(test_indexVec.end(), _singleStirIndex.begin(), _singleStirIndex.end());
	//test_indexVec.insert(test_indexVec.end(), _doubleStirIndex.begin(), _doubleStirIndex.end());

	//test_indexVec.insert(test_indexVec.end(), _bpLeadLineVec.begin(), _bpLeadLineVec.end());
	//test_indexVec.insert(test_indexVec.end(), testIndexVec.begin(), testIndexVec.end());

	for (auto it : break_pointVec)
	{
		cornerVec.push_back(it.center_mind);
	}
	return true;
}

Json::Value ColumnSection::outputColumnSection()
{
	Json::Value columnSection;
	columnSection["ColName"] = _name;
	if (_rectangle)
	{
		columnSection["Shape"] = "Rectangle";
	}
	else
	{
		columnSection["Shape"] = "Alien";
	}
	columnSection["SIze"] = numToString(_maxWidth) + "x" + numToString(_maxHeight);
	columnSection["LongitudinalBar"] = breakPointInfo();
	columnSection["Stirrup"] = stirrupLoc();
	
	return columnSection;
}

bool ColumnSection::iniColumnBlock(const Block& _columnBlock)
{
	this->_columnBlock = _columnBlock;
	this->_name = _columnBlock.name;
	return true;
}

bool ColumnSection::iniBreakPoint(const Block& block, Data& data)
{

	if (_bCircle)
	{
		;//对于断点属于圆形的类型
	}
	else
	{
		//!找出所有的纵筋点
		auto block_bok = block.box;
		int hf = block_bok.hf;
		int ht = block_bok.ht;
		int vf = block_bok.vf;
		int vt = block_bok.vt;

		//小短横
		for (int ht1 = ht; ht1 >= hf; --ht1)
		{
			int idx = data.m_lineData.hLinesIndices()[ht1];
			auto& line = data.m_lineData.lines()[idx];
			
			if (!block_bok.cover(line.s) || !block_bok.cover(line.e))
			{
				continue;
			}

			if (BoolBreakpoint(line, data))
			{
				//!压入纵筋字典索引
				_bpIndexMap[idx] = idx;

				//!初始化梁断面纵筋的图层信息
				this->_bpLayer = line.getLayer();

				ColumnBreakPoint bp;
				Point temp_center_mid = Point((line.e.x + line.s.x) / 2, line.s.y);
				if (findBreakPoint(temp_center_mid))
				{
					continue;
				}

				//mid_point.push_back(temp_center_mid); //圆心
				bp.center_mind = temp_center_mid;
				bp.radius = (ROUND(line.e.x) - ROUND(line.s.x)) / 2.0; //半径
				bp.index = idx;
				break_pointVec.push_back(bp);
			}
		}

		//小短竖
		for (int vt1 = vt; vt1 >= vf; --vt1)
		{
			int idx = data.m_lineData.vLinesIndices()[vt1];
			auto& line = data.m_lineData.lines()[idx];
			/*if (line.entity != nullptr)
				of << line.getLayer() << "  " << line.entity->color << std::endl;*/
			if (!block_bok.cover(line.s) || !block_bok.cover(line.e))
			{
				continue;
			}

			if (BoolBreakpoint(line, data))
			{
				//!压入纵筋字典索引
				_bpIndexMap[idx] = idx;

				//!初始化梁断面纵筋的图层信息
				this->_bpLayer = line.getLayer();

				ColumnBreakPoint bp;
				Point temp_center_mid = Point(line.s.x, (line.e.y + line.s.y) / 2);

				if (findBreakPoint(temp_center_mid))
				{
					continue;
				}
				//mid_point.push_back(temp_center_mid); //圆心
				bp.center_mind = temp_center_mid;
				bp.radius = (ROUND(line.e.y) - ROUND(line.s.y)) / 2.0; //45 半径
				bp.index = idx;
				break_pointVec.push_back(bp);
			}
		}
	}
	return true;
}

bool ColumnSection::findBreakPoint(const Point& p)
{
	for (auto it : break_pointVec)
	{
		if (ConnectPoint(it.center_mind, p))
		{
			return true;
		}
	}
	return false;
}

bool ColumnSection::iniStirrup(Data& data)
{
	//初始话箍筋线的所有索引
	iniStirrupIndex(data.m_lineData);
	//!初始化箍筋标志对
	iniColumnHook(data.m_lineData, data.m_rtreeData);
	//!初始化箍筋信息
	if (_columnHookVec.empty())//!存在没有标志勾的情况
	{
		;//TODO 由于存在的情况较少后续处理
	}
	else
	{
		//! 分类并排序箍线线
		std::vector<int> v_lines;
		std::vector<int> h_lines;
		std::vector<int> ss_lines;
		//!细化处理箍筋线
		divideStirLineType(v_lines, h_lines, ss_lines, data);
		//!为带有标志勾的箍筋设置方向
		setStirrupDirection(v_lines, h_lines, ss_lines, data);

		//!配对箍筋
		std::vector<std::pair<int, int>>hStirPair;//!保存已经配对好的横向箍筋
		std::vector<std::pair<int, int>>vStirPair;//!保存已经配对好的纵向箍筋

		//!配对箍筋线
		makePairStir(hStirPair, vStirPair, v_lines, h_lines, data);
		//!校准，合并（即当一对标志同时，被横箍或纵箍都使用的情况下进行合并，同时可规避错误）
		//!对箍筋做补全（即，两横缺两竖，或两竖缺两横）
		checkStir(data);
		//!补全单支箍
		findColumnSitrrupSignal(data);

	}

	return true;
}

bool ColumnSection::iniStirrupIndex(LineData& _lineData)
{
	std::vector<int>temp_stirrupIndexVec;//!箍筋线的临时索引
	//!初始化箍筋的图层信息
	iniStirrupLayer(_lineData);
	//!确定梁断面的范围
	int hf = _columnBlock.box.hf;
	int ht = _columnBlock.box.ht;
	int vf = _columnBlock.box.vf;
	int vt = _columnBlock.box.vt;

	//!初始化此范围内所有的箍筋线
	//横
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto& line = _lineData.lines()[idx];

		if (line.getLayer() == _stirrupLayer)
		{
			if (_bpIndexMap.find(idx) == _bpIndexMap.end())
			{
				temp_stirrupIndexVec.push_back(idx);
			}
		}
	}
	//竖
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = _lineData.vLinesIndices()[vt1];
		auto& line = _lineData.lines()[idx];

		if (line.getLayer() == _stirrupLayer)
		{
			if (_bpIndexMap.find(idx) == _bpIndexMap.end())
			{
				temp_stirrupIndexVec.push_back(idx);
			}
		}
	}
	//TODO有可能存在异形柱，即箍筋有可能会是斜线
	//！……


	//!剔除误识别的股线
	if (break_pointVec.empty())
	{
		;//TODO 报错此种情况不该出现
	}
	else
	{
		/*此处的策略是，将bpBox边界扩大一点点，搜索扩大后的范围内与箍筋图层相同的线条
		*/
		Box new_box = _bpBox;
		double deviation = 10 * break_pointVec.front().radius;
		new_box.left = new_box.left - deviation;
		new_box.right = new_box.right + deviation;
		new_box.bottom = new_box.bottom - deviation;
		new_box.top = new_box.top + deviation;
		testBox = new_box;
		for (auto it : temp_stirrupIndexVec)
		{
			Line temp_line = _lineData.getLine(it);
			if (new_box.cover(temp_line.s) && new_box.cover(temp_line.e))
			{
				_stirrupIndexVec.push_back(it);
				//stirrupDirMap[it] = Direction::N;

				//!寻找箍筋线的同时初始化由 箍筋线组成的box
				_stirBox.expand(temp_line.s);
				_stirBox.expand(temp_line.e);
			}
		}
	}
	//测试
	//testIndexVec.insert(testIndexVec.end(), _stirrupIndexVec.begin(), _stirrupIndexVec.end());
	return true;
}

bool ColumnSection::iniStirrupLayer(LineData& _lineData)
{
	std::map<std::string, int>candLayerMap;//candidate Layer Map 箍筋的候选图层，用map增加权值
	std::vector<std::string>candLayer;//候选图层
	//!得到纵筋的包围盒，取出箍筋的图层信息
	auto bpBox = bpSurroundBox(_lineData);
	this->_bpBox = bpBox;

	//获取纵筋包围盒范围内的线条索引
	int hf = bpBox.hf;
	int ht = bpBox.ht;
	int vf = bpBox.vf;
	int vt = bpBox.vt;

	//!横线
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto& line = _lineData.lines()[idx];
		if (line.s.x > bpBox.right ||
			line.e.x < bpBox.left ||
			line.length() < std::abs(bpBox.left - bpBox.right) * 0.9)
		{
			continue;
		}
		else
		{
			auto layerIte = std::find(candLayer.begin(), candLayer.end(), line.getLayer());
			if (layerIte == candLayer.end())
			{
				candLayer.push_back(line.getLayer());
				candLayerMap[line.getLayer()]++;
			}
			else
			{
				candLayerMap[*layerIte]++;
			}
		}
	}
	//!竖线
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = _lineData.vLinesIndices()[vt1];
		auto& line = _lineData.lines()[idx];
		if (line.s.y > bpBox.top ||
			line.e.y < bpBox.bottom ||
			line.length() < std::abs(bpBox.top - bpBox.bottom) * 0.9)
		{
			continue;
		}
		else
		{
			auto layerIte = std::find(candLayer.begin(), candLayer.end(), line.getLayer());
			if (layerIte == candLayer.end())
			{
				candLayer.push_back(line.getLayer());
				candLayerMap[line.getLayer()]++;
			}
			else
			{
				candLayerMap[*layerIte]++;
			}
		}
	}

	//!获取图层权值最大的
	int weight = 0;
	for (auto it : candLayerMap)
	{
		if (it.second > weight)
		{
			weight = it.second;
			_stirrupLayer = it.first;
		}
	}

#if 0
	std::fstream of("columnLayer.txt", std::ios::out);
	of <<_name<<"    " <<_stirrupLayer <<u8"   纵筋  "<<_bpLayer << std::endl;
	of.close();
#endif
	return true;
}

Box ColumnSection::bpSurroundBox(LineData& _lineData)
{
	Box temp_box;
	for (auto bpIndex : break_pointVec)
	{
		temp_box.expand(_lineData.lines()[bpIndex.index].s);
		temp_box.expand(_lineData.lines()[bpIndex.index].e);
	}
	SetBoxVHIndex(temp_box, _lineData);
	return temp_box;
}

bool ColumnSection::iniColumnHook(LineData& lineData, RTreeData& goalRTree)
{

	std::vector<int>hooks;
	//beamBlock;
	double min[2];
	double max[2];
	min[0] = this->_bpBox.left;
	min[1] = this->_bpBox.bottom;
	max[0] = this->_bpBox.right;
	max[1] = this->_bpBox.top;
	auto canHooks = returnRtreeIndex(min, max, goalRTree);

	//!筛选箍筋标志
	for (auto it : canHooks)
	{
		if (lineData.getLine(it).getLayer() == _stirrupLayer)
		{
			hooks.push_back(it);

		}
	}

	//!初始化梁的箍筋标志结构
	std::map<int, int>temp_hookMap;
	for (auto hook1 : hooks)
	{
		if (temp_hookMap.find(hook1) == temp_hookMap.end())
		{
			temp_hookMap[hook1] = hook1;

			Line line1 = lineData.getLine(hook1);
			Point mid_point1((line1.s.x + line1.e.x) / 2, (line1.s.y + line1.e.y) / 2);
			double distance = 65535;
			int goalIndex = -1;
			for (auto hook2 : hooks)
			{
				if (temp_hookMap.find(hook2) == temp_hookMap.end())
				{
					Line line2 = lineData.getLine(hook2);
					Point mid_point2((line2.s.x + line2.e.x) / 2, (line2.s.y + line2.e.y) / 2);
					double towPointDistance = TowPointsLength(mid_point1, mid_point2);

					if (distance > towPointDistance)
					{
						distance = towPointDistance;
						goalIndex = hook2;
					}
				}
			}
			//!加上goalIndex的判错处理
			temp_hookMap[goalIndex] = goalIndex;
			//!添加不相交判断，因为有的图纸，单支箍用多段线绘制弯钩处由两条斜线相互连接组成
			if (!bConnectTowLine(lineData.getLine(hook1), lineData.getLine(goalIndex)))
			{
				_columnHookVec.push_back(ColumnStirrupHook(hook1, goalIndex));

				//testIndexVec.push_back(hook1);
				//testIndexVec.push_back(goalIndex);
			}

		}

	}

	return true;
}

bool ColumnSection::divideStirLineType(std::vector<int>& v_lines, std::vector<int>& h_lines, std::vector<int>& ss_lines, Data& data)
{
	lineTypeVHS(data, _stirrupIndexVec, v_lines, h_lines, ss_lines);//为候选箍筋做类型划分
	LinesSort(v_lines, 'V', data.m_lineData);
	LinesSort(h_lines, 'H', data.m_lineData);
	MergeLines(v_lines, 'V', data.m_lineData);
	MergeLines(h_lines, 'H', data.m_lineData);
	_stirrupIndexVec.clear();
	_stirrupIndexVec.insert(_stirrupIndexVec.end(), h_lines.begin(), h_lines.end());
	_stirrupIndexVec.insert(_stirrupIndexVec.end(), v_lines.begin(), v_lines.end());
	_stirrupIndexVec.insert(_stirrupIndexVec.end(), ss_lines.begin(), ss_lines.end());
	//!初始化_bStirPair	stirrupDirMap
	for (auto it : v_lines)
	{
		_bStirPair[it] = false;
		_stirrupDirMap[it] = ColumnDirection::N1;
	}
	for (auto it : h_lines)
	{
		_bStirPair[it] = false;
		_stirrupDirMap[it] = ColumnDirection::N1;
	}
	for (auto it : ss_lines)
	{
		_bStirPair[it] = false;
		_stirrupDirMap[it] = ColumnDirection::N1;
	}

	//为候选箍筋进行排序
	//!此处目前只处理箍筋线是横线与竖线的形式
	LinesSort(v_lines, 'V', data.m_lineData);//排序竖线
	LinesSort(h_lines, 'H', data.m_lineData);//排序横线
	//初始化箍筋支数
	_countH = h_lines.size();
	_countV = v_lines.size();
	return true;
}

bool ColumnSection::setStirrupDirection(std::vector<int>& _vLines, std::vector<int>& _hLines, std::vector<int>& _sLines, Data& _data)
{
	if (_columnHookVec.empty())
	{
		;//!此情况不应该出现
		return true;
	}
	int countHook = -1;
	for (auto hook : _columnHookVec)
	{
		++countHook;//形成beamHookVec索引
					//将hook写成数组的形式，便于统一化编码用同一套规则即可，减少代码的冗余
		if (hook.hook_index1 == -1 || hook.hook_index2 == -1)
		{
			;//此情况不因该出现，报错
			continue;
		}

		std::map<int, ColumnDirection>checkStirDir;//!check stirrup direction
		std::vector<int>temp_stirIndex;
		std::vector<int>temp_hook;
		temp_hook.push_back(hook.hook_index1), temp_hook.push_back(hook.hook_index2);

		//!确定hook所对应的箍筋方向
		for (auto it : temp_hook)
		{
			auto hookCorners = _data.m_cornerData.corners().find(it);
			std::vector<std::pair<int, Point>>cornerIPVec;//暂时存放相交线以及交点corner index point
			std::vector<char>markVHSVec;//存放相交线的类型

										//std::map<int, BeamStirrupDir>checkStirDir;//校验箍筋方向的最终是否合适

			if (hookCorners == _data.m_cornerData.corners().end())
			{
				continue;
			}

			for (auto corner : hookCorners->second)
			{
				auto l1 = corner.l1;
				if (l1 == it)
				{
					l1 = corner.l2;
				}


				//!选出标志勾相交线中的箍筋线，避免与其他线产生误识别
				if (std::find(_stirrupIndexVec.begin(), _stirrupIndexVec.end(), l1) != _stirrupIndexVec.end())
				{
					//除去合并后没有的线条
					if (std::find(_vLines.begin(), _vLines.end(), l1) == _vLines.end() &&
						std::find(_hLines.begin(), _hLines.end(), l1) == _hLines.end() &&
						std::find(_sLines.begin(), _sLines.end(), l1) == _sLines.end())
					{
						continue;
					}
					//压入相交线以及交点
					cornerIPVec.push_back(std::make_pair(l1, corner));
					//设置线类型
					char lineType = 'N';
					if (_data.m_lineData.getLine(l1).vertical())
					{
						lineType = 'V';
					}
					else if (_data.m_lineData.getLine(l1).horizontal())
					{
						lineType = 'H';
					}
					else
					{
						lineType = 'S';
					}
					if (std::find(markVHSVec.begin(), markVHSVec.end(), lineType) == markVHSVec.end())
					{
						markVHSVec.push_back(lineType);
					}

				}

			}
			//TODO:若cornerIndex为空报错
			if (cornerIPVec.empty())
			{
				continue;//
			}
			//为标志勾相交线中最合适的附上方向
			if (markVHSVec.empty()||markVHSVec.size() > 1 || markVHSVec.front() == 'S')
			{
				continue;//;//Todo 报错
			}
			else if (markVHSVec.front() == 'H')
			{
				Line temep_hookLine = _data.m_lineData.getLine(it);
				std::pair<int, ColumnDirection> temp_dir(-1, ColumnDirection::N1);
				double cornerIPLength = 0.0;//用来记录箍筋标志与箍筋间的相对距离
				for (auto stirIndex : cornerIPVec)
				{
					double cornerIPLength1 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).s);
					double cornerIPLength2 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).e);
					double minLength = cornerIPLength1 < cornerIPLength2 ? cornerIPLength1 : cornerIPLength2;
					if (temp_dir.first == -1)
					{
						cornerIPLength = minLength;

						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.y > otherPoint.y)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::D1;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::U1;
						}
					}
					else if (_stirrupDirMap.find(stirIndex.first) != _stirrupDirMap.end() &&
						cornerIPLength > minLength)
					{
						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.y > otherPoint.y)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::D1;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::U1;
						}
					}


				}
				checkStirDir[temp_dir.first] = temp_dir.second;
				temp_stirIndex.push_back(temp_dir.first);
			}
			else if (markVHSVec.front() == 'V')
			{
				Line temep_hookLine = _data.m_lineData.getLine(it);
				std::pair<int, ColumnDirection> temp_dir(-1, ColumnDirection::N1);
				double cornerIPLength = 0.0;//用来记录箍筋标志与箍筋间的相对距离
				for (auto stirIndex : cornerIPVec)
				{
					double cornerIPLength1 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).s);
					double cornerIPLength2 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).e);
					double minLength = cornerIPLength1 < cornerIPLength2 ? cornerIPLength1 : cornerIPLength2;
					if (temp_dir.first == -1)
					{
						cornerIPLength = minLength;

						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.x > otherPoint.x)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::L1;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::R1;
						}
					}
					else if (_stirrupDirMap.find(stirIndex.first) != _stirrupDirMap.end() &&
						cornerIPLength > minLength)
					{
						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.x > otherPoint.x)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::L1;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = ColumnDirection::R1;
						}
					}


				}
				checkStirDir[temp_dir.first] = temp_dir.second;
				temp_stirIndex.push_back(temp_dir.first);
			}
		}

		//!剔除不合适的箍筋，有些箍筋边是公用的
	//!剔除不合适的箍筋，有些箍筋边是公用的
		if (checkStirDir.size() == 2)
		{
			int hIndex = -1, vIndex = -1;
			if (checkStirDir[temp_stirIndex[0]] == ColumnDirection::D1 ||
				checkStirDir[temp_stirIndex[0]] == ColumnDirection::U1)
			{
				hIndex = temp_stirIndex[0];
			}
			else
			{
				vIndex = temp_stirIndex[0];

			}
			if (checkStirDir[temp_stirIndex[1]] == ColumnDirection::L1 ||
				checkStirDir[temp_stirIndex[1]] == ColumnDirection::R1)
			{
				vIndex = temp_stirIndex[1];

			}
			else
			{
				hIndex = temp_stirIndex[1];

			}
			if (hIndex != -1 && vIndex != -1)
			{
				if (checkStirDir[hIndex] == ColumnDirection::D1 &&
					_data.m_lineData.getLine(vIndex).e.y + Precision >
					_data.m_lineData.getLine(hIndex).s.y)
				{
					_stirrupDirMap[vIndex] = checkStirDir[vIndex];
					_stirDirHookIndexMap[vIndex] = countHook;
				}
				else if (checkStirDir[hIndex] == ColumnDirection::U1 &&
					_data.m_lineData.getLine(vIndex).s.y - Precision <
					_data.m_lineData.getLine(hIndex).s.y)
				{
					_stirrupDirMap[vIndex] = checkStirDir[vIndex];
					_stirDirHookIndexMap[vIndex] = countHook;
				}
				if (checkStirDir[vIndex] == ColumnDirection::L1 &&
					_data.m_lineData.getLine(hIndex).e.x - Precision <
					_data.m_lineData.getLine(vIndex).s.x)
				{
					_stirrupDirMap[hIndex] = checkStirDir[hIndex];
					_stirDirHookIndexMap[hIndex] = countHook;
				}
				else if (checkStirDir[vIndex] == ColumnDirection::R1 &&
					_data.m_lineData.getLine(hIndex).s.x + Precision >
					_data.m_lineData.getLine(vIndex).s.x)
				{
					_stirrupDirMap[hIndex] = checkStirDir[hIndex];
					_stirDirHookIndexMap[hIndex] = countHook;
				}
			}
			else
			{
				;//报错
			}

		}
		else
		{
			;//!有问题
		}

	}
	return true;
}

bool ColumnSection::makePairStir(std::vector<std::pair<int, int>>& hStirPair, 
	std::vector<std::pair<int, int>>& vStirPair, 
	const std::vector<int>& v_lines, 
	const std::vector<int>& h_lines, Data& data)
{
	//!配对箍筋
	std::map<int, ColumnDirection>v_dir;
	std::map<int, ColumnDirection>h_dir;
	divideStirrupDir(v_lines, v_dir, 'V');
	divideStirrupDir(h_lines, h_dir, 'H');

	//!配对横向箍筋
	std::stack<std::pair<int, ColumnDirection>>h_stack;//!横向栈
	for (auto hStirIndex : h_lines)
	{
		//栈为空压栈
		if (h_stack.empty())
		{
			if (h_dir.find(h_lines.front()) == h_dir.end())
			{
				h_stack.push(std::pair<int, ColumnDirection>(h_lines.front(), ColumnDirection::N1));
			}
			else
			{
				h_stack.push(std::pair<int, ColumnDirection>(h_lines.front(), h_dir[h_lines.front()]));
			}
		}
		else
		{
			std::pair<int, ColumnDirection> temp_stir = h_stack.top();
			if (temp_stir.second == ColumnDirection::N1)
			{
				if (h_dir.find(hStirIndex) == h_dir.end())
				{
					h_stack.push(std::pair<int, ColumnDirection>(hStirIndex, ColumnDirection::N1));
				}
				else if (h_dir[hStirIndex] == ColumnDirection::D1)
				{
					//std::stack<std::pair<int, Direction>>temp_stack = h_stack;
					while (!h_stack.empty())
					{
						std::pair<int, ColumnDirection>temp_stirDir = h_stack.top();
						if (temp_stirDir.second != ColumnDirection::N1)
						{
							break;
						}
						else
						{
							if (std::abs(data.m_lineData.getLine(hStirIndex).length() -
								data.m_lineData.getLine(temp_stirDir.first).length()) > 4 * Precision)//比较长度
							{
								h_stack.pop();
							}
							else
							{
								//!初始化_hookStirrupIndexMap用于后续的校验，用空间换时间，后面相同
								auto hoolIndexIte = _stirDirHookIndexMap.find(hStirIndex);
								if (_hookStirrupIndexMap.find(hoolIndexIte->second) == _hookStirrupIndexMap.end())
								{
									_hookStirrupIndexMap[hoolIndexIte->second].push_back(hStirIndex);
									_hookStirrupIndexMap[hoolIndexIte->second].push_back(temp_stirDir.first);
								}
								else
								{
									_hookStirrupIndexMap[hoolIndexIte->second].push_back(hStirIndex);
									_hookStirrupIndexMap[hoolIndexIte->second].push_back(temp_stirDir.first);
								}
								//if(_hookStirrupIndexMap.find())
								hStirPair.push_back(std::pair<int, int>(hStirIndex, temp_stirDir.first));
								h_stack.pop();
								break;
							}
						}
					}
				}
				else if (h_dir[hStirIndex] == ColumnDirection::U1)
				{
					h_stack.push(std::pair<int, ColumnDirection>(hStirIndex, ColumnDirection::U1));
				}
			}
			else if (temp_stir.second == ColumnDirection::U1)
			{
				if (h_dir.find(hStirIndex) == h_dir.end())
				{
					if (std::abs(data.m_lineData.getLine(hStirIndex).length() -
						data.m_lineData.getLine(temp_stir.first).length()) > 4 * Precision)//比较长度
					{
						continue;
					}
					else
					{


						auto hookIndexIte = _stirDirHookIndexMap.find(temp_stir.first);
						if (_hookStirrupIndexMap.find(hookIndexIte->second) == _hookStirrupIndexMap.end())
						{
							_hookStirrupIndexMap[hookIndexIte->second].push_back(hStirIndex);
							_hookStirrupIndexMap[hookIndexIte->second].push_back(temp_stir.first);
						}
						else
						{
							_hookStirrupIndexMap[hookIndexIte->second].push_back(hStirIndex);
							_hookStirrupIndexMap[hookIndexIte->second].push_back(temp_stir.first);
						}
						hStirPair.push_back(std::pair<int, int>(hStirIndex, temp_stir.first));
						h_stack.pop();
						//break;
					}
				}
				else
				{
					if (h_dir[hStirIndex] == ColumnDirection::U1)
					{
						h_stack.push(std::pair<int, ColumnDirection>(hStirIndex, ColumnDirection::U1));
					}
				}
			}

		}
	}

	//!纵向匹配箍筋
	std::stack<std::pair<int, ColumnDirection>>v_stack;//!纵向栈
	for (auto vStirIndex : v_lines)
	{
		//栈为空压栈
		if (v_stack.empty())
		{
			if (v_dir.find(v_lines.front()) == v_dir.end())
			{
				v_stack.push(std::pair<int, ColumnDirection>(v_lines.front(), ColumnDirection::N1));
			}
			else
			{
				v_stack.push(std::pair<int, ColumnDirection>(v_lines.front(), v_dir[v_lines.front()]));
			}
		}
		else
		{
			std::pair<int, ColumnDirection> temp_stir = v_stack.top();
			if (temp_stir.second == ColumnDirection::N1)
			{
				if (v_dir.find(vStirIndex) == v_dir.end())
				{
					v_stack.push(std::pair<int, ColumnDirection>(vStirIndex, ColumnDirection::N1));
				}
				else if (v_dir[vStirIndex] == ColumnDirection::L1)
				{
					//std::stack<std::pair<int, Direction>>temp_stack = h_stack;
					while (!v_stack.empty())
					{
						std::pair<int, ColumnDirection>temp_stirDir = v_stack.top();
						if (temp_stirDir.second != ColumnDirection::N1)
						{
							break;
						}
						else
						{
							if (std::abs(data.m_lineData.getLine(vStirIndex).length() -
								data.m_lineData.getLine(temp_stirDir.first).length()) > 4 * Precision)//比较长度
							{
								v_stack.pop();
							}
							else
							{
								auto hookIndexIte = _stirDirHookIndexMap.find(vStirIndex);
								if (_hookStirrupIndexMap.find(hookIndexIte->second) == _hookStirrupIndexMap.end())
								{
									_hookStirrupIndexMap[hookIndexIte->second].push_back(vStirIndex);
									_hookStirrupIndexMap[hookIndexIte->second].push_back(temp_stirDir.first);
								}
								else
								{
									_hookStirrupIndexMap[hookIndexIte->second].push_back(vStirIndex);
									_hookStirrupIndexMap[hookIndexIte->second].push_back(temp_stirDir.first);
								}

								vStirPair.push_back(std::pair<int, int>(vStirIndex, temp_stirDir.first));
								v_stack.pop();
								break;
							}
						}
					}
				}
				else if (v_dir[vStirIndex] == ColumnDirection::R1)
				{
					v_stack.push(std::pair<int, ColumnDirection>(vStirIndex, ColumnDirection::U1));
				}
			}
			else if (temp_stir.second == ColumnDirection::R1)
			{
				if (v_dir.find(vStirIndex) == v_dir.end())
				{
					if (std::abs(data.m_lineData.getLine(vStirIndex).length() -
						data.m_lineData.getLine(temp_stir.first).length()) > 4 * Precision)//比较长度
					{
						continue;
					}
					else
					{
						auto hoolIndexIte = _stirDirHookIndexMap.find(temp_stir.first);
						if (_hookStirrupIndexMap.find(hoolIndexIte->second) == _hookStirrupIndexMap.end())
						{
							_hookStirrupIndexMap[hoolIndexIte->second].push_back(vStirIndex);
							_hookStirrupIndexMap[hoolIndexIte->second].push_back(temp_stir.first);
						}
						else
						{
							_hookStirrupIndexMap[hoolIndexIte->second].push_back(vStirIndex);
							_hookStirrupIndexMap[hoolIndexIte->second].push_back(temp_stir.first);
						}
						vStirPair.push_back(std::pair<int, int>(vStirIndex, temp_stir.first));
						v_stack.pop();
						//break;
					}
				}
				else
				{
					if (v_dir[vStirIndex] == ColumnDirection::R1)
					{
						v_stack.push(std::pair<int, ColumnDirection>(vStirIndex, ColumnDirection::R1));
					}
				}
			}

		}
	}

	return true;
}

bool ColumnSection::divideStirrupDir(const std::vector<int>& lineIndex, std::map<int, ColumnDirection>& goalIndex, const char& ch)
{
	if (ch == 'H')
	{
		for (auto it : lineIndex)
		{
			if (_stirrupDirMap[it] != ColumnDirection::N1)
			{
				goalIndex[it] = _stirrupDirMap[it];
			}
		}
	}
	else if (ch == 'V')
	{
		for (auto it : lineIndex)
		{
			if (_stirrupDirMap[it] != ColumnDirection::N1)
			{
				goalIndex[it] = _stirrupDirMap[it];
			}
		}
	}
	else if (ch == 'S')
	{
		;//暂不处理
	}

	return true;
}

bool ColumnSection::checkStir(Data& data)
{
	for (auto it : _hookStirrupIndexMap)
	{
		ColumnSStirrup temp_stir;
		temp_stir.hook = _columnHookVec[it.first];
		//!只有对边的情况
		if (it.second.size() == 2)
		{
			temp_stir = checkStirPair1(it.second, data.m_lineData);
			//!压入箍筋
			_columnStirrupVec.push_back(temp_stir);
		}
		//!两个hook都找到对边的情况，一般出现在箍筋的最外圈
		else if (it.second.size() > 2)
		{
			temp_stir = checkStirPair2(it.second, data);
			_columnStirrupVec.push_back(temp_stir);
		}
		//!hook 未找到箍筋
		else
		{
			;//查错
		}
	}

	return true;
}

ColumnSStirrup ColumnSection::checkStirPair1(std::vector<int> stirDirIndex, LineData& _lineData)
{
	ColumnSStirrup temp_stir;
	auto line1 = _lineData.getLine(stirDirIndex[0]);
	auto line2 = _lineData.getLine(stirDirIndex[1]);

	_bStirPair[stirDirIndex[0]] = true;
	_bStirPair[stirDirIndex[1]] = true;

	if (line1.vertical() && line2.vertical())
	{
		Line temp_line1(Line(line1.s, line2.s));
		Line temp_line2(Line(line1.e, line2.e));
		int goalIndex;
		if (findSameStir(temp_line1, goalIndex, _lineData))
		{
			_bStirPair[goalIndex] = true;
		}
		if (findSameStir(temp_line2, goalIndex, _lineData))
		{
			_bStirPair[goalIndex] = true;
		}
		temp_stir.circle_line.push_back(line1);
		temp_stir.circle_line.push_back(line2);
		temp_stir.circle_line.push_back(temp_line1);
		temp_stir.circle_line.push_back(temp_line2);
	}
	else if (line1.horizontal() && line2.horizontal())
	{
		Line temp_line1(Line(line1.s, line2.s));
		Line temp_line2(Line(line1.e, line2.e));
		int goalIndex;
		if (findSameStir(temp_line1, goalIndex, _lineData))
		{
			_bStirPair[goalIndex] = true;
		}
		if (findSameStir(temp_line2, goalIndex, _lineData))
		{
			_bStirPair[goalIndex] = true;
		}

		temp_stir.circle_line.push_back(line1);
		temp_stir.circle_line.push_back(line2);
		temp_stir.circle_line.push_back(temp_line1);
		temp_stir.circle_line.push_back(temp_line2);
	}
	return temp_stir;
}

ColumnSStirrup ColumnSection::checkStirPair2(std::vector<int> stirDirIndex, Data& data)
{
	ColumnSStirrup temp_stir;
	std::vector<int>vStir, hStir, sStir;
	lineTypeVHS(data, stirDirIndex, vStir, hStir, sStir);//为候选箍筋做类型划分
	if (vStir.empty() || hStir.empty())
	{
		;//报错
		return temp_stir;
	}
	//!确定箍筋各个边界位置
	double left = data.m_lineData.getLine(hStir.front()).s.x;
	double right = data.m_lineData.getLine(hStir.front()).e.x;
	double top = data.m_lineData.getLine(vStir.front()).e.y;
	double bottom = data.m_lineData.getLine(vStir.front()).s.y;

	//!判断箍筋的顶部与底部是否正确，有时候会出现匹配失败的情况
	bool isTop = false;
	bool isBottom = false;
	Line topLine;
	Line bottomLine;
	//!check 箍筋的横线是否正确
	for (auto hIndex : hStir)
	{
		bool b = true;
		bool t = true;
		auto temp_line = data.m_lineData.getLine(hIndex);
		if (temp_line.s.y - Precision > bottom)
		{
			b = false;
		}

		if (temp_line.s.y + Precision < top)
		{
			t = false;
		}
		if (!t && !b)
		{
			continue;
		}
		else if (t && !b)
		{
			isTop = true;
			_bStirPair[hIndex] = true;
			topLine = temp_line;
		}
		else if (!t && b)
		{
			isBottom = true;
			_bStirPair[hIndex] = true;
			bottomLine = temp_line;
		}
	}
	if (isBottom)
	{
		temp_stir.circle_line.push_back(bottomLine);

	}
	else
	{
		int goalIndex;
		Line temp_line = Line(Point(left, bottom), Point(right, bottom));
		if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
			_bStirPair[goalIndex] = true;
		}

		temp_stir.circle_line.push_back(temp_line);
	}
	if (isTop)
	{
		temp_stir.circle_line.push_back(topLine);
	}
	else
	{
		int goalIndex;
		Line temp_line = Line(Point(left, top), Point(right, top));
		if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
			_bStirPair[goalIndex] = true;
		}

		temp_stir.circle_line.push_back(temp_line);
	}

	//判断箍筋的左边与右边是否正确
	bool isLeft = false;
	bool isRight = false;
	Line leftLine;
	Line rightLine;
	//!check 箍筋的纵筋是否正确
	for (auto vIndex : vStir)
	{
		bool l = true;
		bool r = true;
		auto temp_line = data.m_lineData.getLine(vIndex);

		if (temp_line.s.x - Precision > left)
		{
			l = false;
		}

		if (temp_line.s.x + Precision < right)
		{
			r = false;
		}
		if (!r && !l)
		{
			continue;
		}
		else if (r && !l)
		{
			isRight = true;
			_bStirPair[vIndex] = true;
			rightLine = temp_line;
		}
		else if (!r && l)
		{
			isLeft = true;
			_bStirPair[vIndex] = true;
			leftLine = temp_line;
		}

	}

	if (isLeft)
	{
		temp_stir.circle_line.push_back(leftLine);
	}
	else
	{
		int goalIndex;
		Line temp_line = Line(Point(left, bottom), Point(left, top));
		if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
			_bStirPair[goalIndex] = true;
		}
		/*if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
		_bStirPair[goalIndex] = true;
		}*/
		temp_stir.circle_line.push_back(temp_line);
	}
	if (isRight)
	{
		temp_stir.circle_line.push_back(rightLine);
	}
	else
	{
		int goalIndex;
		Line temp_line = Line(Point(right, bottom), Point(right, top));
		if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
			_bStirPair[goalIndex] = true;
		}
		/*if (findSameStir(temp_line, goalIndex, data.m_lineData))
		{
		_bStirPair[goalIndex] = true;
		}*/
		temp_stir.circle_line.push_back(temp_line);
	}
	return temp_stir;
}

bool ColumnSection::findSameStir(const Line& _line, int& lineIndex, LineData& _lineData)
{
	if (_line.vertical())
	{
		double xb = _line.s.x - Precision;
		double xu = _line.s.x + Precision;
		int i = _lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xb, Precision, false);
		int j = _lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xu, Precision, false);
		if (i == j && i == -1)
			return false;
		for (int low = i; low <= j; ++low)
		{
			int v_index = _lineData.vLinesIndices()[low];
			Line temp_line = _lineData.getLine(v_index);
			if (temp_line.s.y + Precision > _line.s.y&&
				temp_line.e.y - Precision<_line.e.y &&
				temp_line.s.x + Precision > _line.s.x &&
				temp_line.s.x - Precision < _line.s.x)//!近似可以看作同一条直线
			{
				lineIndex = v_index;
				return true;
			}
		}
	}
	else if (_line.horizontal())
	{
		double yb = _line.s.y - Precision;
		double yu = _line.s.y + Precision;
		int i = _lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = _lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		if (i == j && i == -1)
			return false;
		for (int low = i; low <= j; ++low)
		{
			int h_index = _lineData.hLinesIndices()[low];
			Line temp_line = _lineData.getLine(h_index);
			if (temp_line.s.x + Precision > _line.s.x&&
				temp_line.e.x - Precision<_line.e.x &&
				temp_line.s.y + Precision > _line.s.y &&
				temp_line.s.y - Precision < _line.s.y)//!近似可以看作同一条直线
			{
				lineIndex = h_index;
				return true;
			}
		}
	}
	else
	{
		;//!理论上不会出现
	}
}

bool ColumnSection::findColumnSitrrupSignal(Data& data)
{
	for (auto it : _stirrupIndexVec)
	{
		ColumnSStirrup temp_stir;
		//用纵筋的半径做对比，排除单支箍的例外情况
		double length = break_pointVec.front().radius;
		if (!_bStirPair[it])
		{
			//!正常的箍筋线至少会大于纵筋半径的10倍
			if (data.m_lineData.getLine(it).length() < 10 * length)
				continue;
			temp_stir.circle_line.push_back(data.m_lineData.getLine(it));
			temp_stir.single = true;

			//!初始化单支箍索引
			_singleStirIndex.push_back(it);
			_columnStirrupVec.push_back(temp_stir);
		}
		else
		{
			//!初始化双支箍索引
			_doubleStirIndex.push_back(it);
		}

	}
	return true;
}

bool ColumnSection::findBpLeadLine(Data& data)
{
	std::vector<int>leadLineVec;
	int bpCount = -1;
	for (auto it : break_pointVec)
	{
		bpCount++;
		auto ends = data.m_kdtTreeData.kdtEndpoints().radiusSearch(Endpoint(0, it.center_mind), 45);

		for (auto end : ends)
		{
			auto e = data.m_endpointData.getEndpoint(end);
			if (data.m_lineData.getLine(e.index).getLayer() == _bpLayer ||
				data.m_lineData.getLine(e.index).getLayer() == _stirrupLayer ||
				data.m_lineData.getLine(e.index).length() < 90 ||
				data.m_lineData.getLine(e.index).isSide())
			{
				continue;
			}
			else
			{
				auto itea = find(leadLineVec.begin(), leadLineVec.end(), e.index);
				bool mark = false;
				int mark_num = -1;
				if (itea == leadLineVec.end())
				{
					leadLineVec.push_back(e.index);
					mark = true;
					mark_num = e.index;
				}
				else
				{
					continue;
				}

				auto corners = data.m_cornerData.corners().find(e.index);
				//!保护措施
				if (corners == data.m_cornerData.corners().end())
					continue;

				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == e.index)
					{
						l1 = corner.l2;
					}
					auto point = data.m_lineData.getLine(e.index).s;
					if (point == e)
					{
						point = data.m_lineData.lines()[e.index].e;
					}

					if ((point.x + Precision > data.m_lineData.lines()[l1].s.x&&
						point.x - Precision < data.m_lineData.lines()[l1].s.x) ||
						(point.y + Precision > data.m_lineData.lines()[l1].s.y&&
							point.y - Precision < data.m_lineData.lines()[l1].s.y))
					{
						itea = find(leadLineVec.begin(), leadLineVec.end(), l1);
						if (itea == leadLineVec.end())
						{
							ColumnBPInfo temp_columnBpInfo;
							leadLineVec.push_back(l1);
							std::vector<int> temp;
							mark = false;
							temp.push_back(l1);
							temp.push_back(mark_num);

							temp_columnBpInfo.bpLeadVec = temp;
							//!添加与因出现相关的纵筋截面
							temp_columnBpInfo.bpVec.push_back(bpCount);
							_columnBPinfoVec.push_back(temp_columnBpInfo);
						}
						else
						{
							for (auto it1 = 0; it1 < _columnBPinfoVec.size(); ++it1)
							{
								auto itea = find(_columnBPinfoVec[it1].bpLeadVec.begin(), _columnBPinfoVec[it1].bpLeadVec.end(), l1);
								if (itea != _columnBPinfoVec[it1].bpLeadVec.end())
								{
									mark = false;
									_columnBPinfoVec[it1].bpLeadVec.push_back(mark_num);
									_columnBPinfoVec[it1].bpVec.push_back(bpCount);
								}
							}
						}
					}
				}
				//!处理单独一条引出线的情况
				if (mark_num != -1 && mark == true)
				{
					ColumnBPInfo temp_columnBpInfo;
					temp_columnBpInfo.bpLeadVec.push_back(mark_num);
					temp_columnBpInfo.bpVec.push_back(bpCount);
					_columnBPinfoVec.push_back(temp_columnBpInfo);

				}
			}
		}

	}
	_bpLeadLineVec = leadLineVec;
	return true;
}

bool ColumnSection::iniColumnStirLead(Data& data)
{
	auto stirLead = divideStirLead(data.m_lineData);

	//testLineIndex = stirLead;
	//!分类箍筋的引出线
	ColumnStirLeadInfo stirLeadType;
	std::queue<int> _queue;//用队列去做相接性判断
	std::map<int, int>leadMap;//防止重复压入
	for (auto it : stirLead)
	{
		if (leadMap.find(it) != leadMap.end())
		{
			continue;
		}
		std::vector<int>leadType;
		leadType.push_back(it);
		_queue.push(it);
		leadMap[it] = it;

		while (!_queue.empty())
		{
			auto index = _queue.front();
			_queue.pop();

			//!平行线的判定，存在箍筋引出线同一条线由多个线段组成
			{
				Line now_line = data.m_lineData.getLine(index);
				if (now_line.vertical())
				{
					double xl = now_line.s.x - Precision;
					double xr = now_line.s.x + Precision;
					int xi = data.m_lineData.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
					int xj = data.m_lineData.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);
					if (xi != xj)
					{
						for (int i = xi; i <= xj; ++i)
						{
							int v_index = data.m_lineData.vLinesIndices()[i];
							if (v_index == index)
								continue;
							Line v_line = data.m_lineData.getLine(v_index);
							if (bConnectTowLine(v_line, now_line) &&
								v_line.getLayer() == now_line.getLayer())
							{
								if (leadMap.find(v_index) == leadMap.end())
								{
									_queue.push(v_index);
									leadMap[v_index] = v_index;
									leadType.push_back(v_index);

								}
							}
						}
					}


				}
				else if (now_line.horizontal())
				{
					double yb = now_line.s.y - Precision;
					double yu = now_line.s.y + Precision;
					int yi = data.m_lineData.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
					int yj = data.m_lineData.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
					if (yi != yj)
					{
						for (int j = yi; j <= yj; ++j)
						{
							int h_index = data.m_lineData.hLinesIndices()[j];
							if (h_index == index)
								continue;
							Line h_line = data.m_lineData.getLine(h_index);
							if (bConnectTowLine(h_line, now_line) &&
								h_line.getLayer() == now_line.getLayer())
							{
								if (leadMap.find(h_index) == leadMap.end())
								{
									_queue.push(h_index);
									leadMap[h_index] = h_index;
									leadType.push_back(h_index);

								}
							}
						}
					}

				}
				else
				{
					//findPairedBorder(,)
					;//斜线很少出现后续处理
				}
			}

			auto corners = data.m_cornerData.corners().find(index);
			if (corners == data.m_cornerData.corners().end())
			{
				continue;
			}

			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				if (l1 == index)
				{
					l1 = corner.l2;
				}
				if (std::find(stirLead.begin(), stirLead.end(), l1) != stirLead.end() &&
					leadMap.find(l1) == leadMap.end() &&
					bPointBelongLineEnd(data.m_lineData.getLine(l1), corner) &&
					bPointBelongLineEnd(data.m_lineData.getLine(index), corner))
				{
					leadType.push_back(l1);
					_queue.push(l1);
					leadMap[l1] = l1;
				}
			}


		}
		stirLeadType.stirLeadVec = leadType;
		//!初始化引出线的箍筋的引出线
		_columnStirInfo.push_back(stirLeadType);
	}


	return true;
}

std::vector<int> ColumnSection::divideStirLead(LineData& _lineData)
{
	std::vector<int>stirLineIndex;
	auto bpBox = bpSurroundBox(_lineData);
	this->_bpBox = bpBox;

	//获取纵筋包围盒范围内的线条索引
	int hf = bpBox.hf;
	int ht = bpBox.ht;
	int vf = bpBox.vf;
	int vt = bpBox.vt;

	//!横线
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto& line = _lineData.lines()[idx];
		if (line.s.x > bpBox.right ||
			line.e.x < bpBox.left)
		{
			continue;
		}
		else
		{
			if (line.getLayer() != _bpLayer &&
				line.getLayer() != _stirrupLayer &&
				std::find(_bpLeadLineVec.begin(), _bpLeadLineVec.end(), idx) == _bpLeadLineVec.end())
			{
				stirLineIndex.push_back(idx);
			}
		}
	}
	//!竖线
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = _lineData.vLinesIndices()[vt1];
		auto& line = _lineData.lines()[idx];
		if (line.s.y > bpBox.top ||
			line.e.y < bpBox.bottom)
		{
			continue;
		}
		else
		{
			if (line.getLayer() != _bpLayer &&
				line.getLayer() != _stirrupLayer &&
				std::find(_bpLeadLineVec.begin(), _bpLeadLineVec.end(), idx) == _bpLeadLineVec.end())
			{
				stirLineIndex.push_back(idx);
			}
		}
	}

	return stirLineIndex;
}

bool ColumnSection::iniRebarInfo(Data& data)
{
	//字符串信息字典，用于校验，第一个表示文本点索引，第二个string 表示，纵筋还是箍筋 分别用“Z” “G”表示，Point 表示文本的参照点,int 表示所在纵筋或箍筋信息的索引
	std::map<int, std::pair<std::string, std::pair<Point, int>>>strInfoMap;
	//初始化梁断面内所有的纵筋信息
	int bpInfoCount = 0;
	for (auto it = _columnBPinfoVec.begin(); it != _columnBPinfoVec.end(); ++it)
	{
		std::vector<int>temp_textPIndex;
		Point refPoint;//查找引出线的参照点
		iniBeamLeadInfo(temp_textPIndex, refPoint, it->bpLeadVec, true, data);

		//auto  temp_textStirIte = temp_textStr.begin();
		for (auto pIndex : temp_textPIndex)
		{
			//获取文字实体，以便于获取其方向
			auto textPoint = data.m_textPointData.textpoints()[pIndex];
			auto tt = std::static_pointer_cast<DRW_Text>(textPoint.entity);

			if (strInfoMap.find(pIndex) == strInfoMap.end())
			{
				char ch = 'N';
				returnLeadLineDir(ch, true, it->bpLeadVec, data);
				if (ch == 'N'||tt->text.find("%%")==std::string::npos)
				{
					//TODO报错，这种情况不允许出现
					continue;
				}
				if ((ch == 'H' && (tt->angle == 0 || (int((tt->angle) * 100)) / 100.0 == 3.14)) ||
					(ch = 'V' && (int((tt->angle) * 100) / 100.0 == 1.57 || (int((tt->angle) * 100)) / 100.0 == 4.71)))
				{
					strInfoMap[pIndex] = std::pair<std::string, std::pair<Point, int>>("Z", std::pair<Point, int>(refPoint, bpInfoCount));
					it->bpInfoPointIndex.push_back(pIndex);
					it->bpInfoVec.push_back(tt->text);
				}

			}
			else
			{
				auto strInfoIte = strInfoMap[pIndex];
				double length1 = TowPointsLength(textPoint, strInfoIte.second.first);
				double length2 = TowPointsLength(textPoint, refPoint);
				//!只有文本到参照距离更小时，才交换否则不会交换
				if (length1 > length2)
				{

					//!删除无添加的信息
					if (strInfoIte.first == "Z")
					{
						auto strIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
							_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
							tt->text);
						_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
						auto indexIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
							_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
							pIndex);
						_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

					}
					else if (strInfoIte.first == "G")
					{
						_columnStirInfo[strInfoIte.second.second].stirInfo.clear();
						_columnStirInfo[strInfoIte.second.second].textPoint = Point();
					}
					//!对现有的进行初始化
					it->bpInfoPointIndex.push_back(pIndex);
					it->bpInfoVec.push_back(tt->text);
					//!更新字典
					strInfoMap[pIndex] = std::pair<std::string, std::pair<Point, int>>("Z", std::pair<Point, int>(refPoint, bpInfoCount));
				}

			}
		}
		bpInfoCount++;
	}
	

#if 0
	int stirInfoCount = 0;
	for (auto it = _columnStirInfo.begin(); it != _columnStirInfo.end(); ++it)
	{
		std::vector<int>temp_textPIndex;
		std::string goal_textStr;
		Point goal_point;
		int goal_textPIndex;


		Point refPoint;//reference point 参照点
		iniBeamLeadInfo(temp_textPIndex, refPoint, it->stirLeadVec, false, data);

		if (temp_textPIndex.size() > 1)
		{
			double refLength = 65536;//!相对长度，箍筋的文本注释一般只有一个，选文本点离参照点最近的一个

			for (auto pIndex : temp_textPIndex)
			{
				//获取文字实体，以便于获取其方向
				auto textPoint = data.m_textPointData.textpoints()[pIndex];
				auto tt = std::static_pointer_cast<DRW_Text>(textPoint.entity);

				if (strInfoMap.find(pIndex) == strInfoMap.end())
				{
					char ch = 'N';
					returnLeadLineDir(ch, false, it->stirLeadVec, data);
					if (ch == 'N')
					{
						//TODO报错，这种情况不允许出现
						continue;
					}
					if ((ch == 'H' && (tt->angle == 0 || (int((tt->angle) * 100)) / 100.0 == 3.14)) ||
						(ch = 'V' && (int((tt->angle) * 100) / 100.0 == 1.57 || (int((tt->angle) * 100)) / 100.0 == 4.71)))
					{
						//strInfoMap[pIndex] = std::pair<std::string, std::pair<Point, int>>("G", std::pair<Point, int>(refPoint, stirInfoCount));
						//!对比选取最合适的信息
						auto temp_length = TowPointsLength(textPoint, refPoint);
						if (refLength > temp_length)
						{
							goal_point = textPoint;
							goal_textStr = tt->text;
							refLength = temp_length;
							goal_textPIndex = pIndex;
						}

					}

				}
				else
				{
					auto strInfoIte = strInfoMap[pIndex];
					double length1 = TowPointsLength(textPoint, strInfoIte.second.first);
					double length2 = TowPointsLength(textPoint, refPoint);
					if (length1 > length2)
					{
						//!删除无添加的信息
						if (strInfoIte.first == "Z")
						{
							auto strIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
								_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
								tt->text);
							_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
							auto indexIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
								_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
								pIndex);
							_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

						}
						else if (strInfoIte.first == "G")
						{
							_columnStirInfo[strInfoIte.second.second].stirInfo.clear();
							_columnStirInfo[strInfoIte.second.second].textPoint = Point();
						}
						//!对现有的进行初始化
						goal_point = textPoint;
						goal_textStr = tt->text;
						goal_textPIndex = pIndex;
						//!更新字典
						//strInfoMap[pIndex] = std::pair<std::string, std::pair<Point, int>>("G", std::pair<Point, int>(refPoint, bpInfoCount));
					}
				}
			}

			//为箍筋信息赋值
			it->textPoint = goal_point;
			it->stirInfo = goal_textStr;
			strInfoMap[goal_textPIndex] = std::pair<std::string, std::pair<Point, int>>("G", std::pair<Point, int>(refPoint, stirInfoCount));

		}
		else if (!temp_textPIndex.empty())
		{
			auto textPoint = data.m_textPointData.textpoints()[temp_textPIndex.front()];
			auto tt = std::static_pointer_cast<DRW_Text>(textPoint.entity);
			if (strInfoMap.find(temp_textPIndex.front()) == strInfoMap.end())
			{
				strInfoMap[temp_textPIndex.front()] = std::pair<std::string, std::pair<Point, int>>("G", std::pair<Point, int>(refPoint, stirInfoCount));
				it->textPoint = textPoint;
				it->stirInfo = tt->text;
			}
			else
			{
				auto strInfoIte = strInfoMap[temp_textPIndex.front()];
				double length1 = TowPointsLength(textPoint, strInfoIte.second.first);
				double length2 = TowPointsLength(textPoint, refPoint);
				if (length1 > length2)
				{
					//!删除无添加的信息
					if (strInfoIte.first == "Z")
					{
						auto strIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
							_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
							tt->text);
						_columnBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
						auto indexIte = std::find(_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
							_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
							temp_textPIndex.front());
						_columnBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

					}
					else if (strInfoIte.first == "G")
					{
						_columnStirInfo[strInfoIte.second.second].stirInfo.clear();
						_columnStirInfo[strInfoIte.second.second].textPoint = Point();
					}
					//!对现有的进行初始化
					it->textPoint = textPoint;
					it->stirInfo = tt->text;
					//!更新字典
					strInfoMap[temp_textPIndex.front()] = std::pair<std::string, std::pair<Point, int>>("G", std::pair<Point, int>(refPoint, bpInfoCount));
				}
			}
		}
		else
		{
			;//!为空的情况，理论上不允许存在
		}

		stirInfoCount++;
	}
#endif
	//初始化柱断面内所有的箍筋信息
	double left = _columnBlock.box.left;
	double right = _columnBlock.box.right;
	int i = data.m_textPointData.findLowerBound(_columnBlock.box.bottom);
	int j = data.m_textPointData.findUpperBound(_columnBlock.box.top);

	//通过标注符号查找候选集
	for (i; i <= j; ++i)
	{
		auto& point = data.m_textPointData.textpoints()[i];
		if (point.x > left - Precision && point.x < right + Precision)
		{
			//zaici
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			if (tt->text.find("@") != std::string::npos)
			{
				_stirrup = tt->text;
			}
		}
	}

	//!初始化箍筋描述信息所描述的是单支箍还是双支箍
	setStirInfoType(data);
	//!初始化纵筋信息的方向
	setBPLeadInfoDirection(data.m_lineData);
	return true;
	
}

bool ColumnSection::iniBeamLeadInfo(std::vector<int>& textPointIndex, Point& refPoint, const std::vector<int>& _lineIndex, const bool& _bpLead, Data& data)
{

	std::vector<int>texts1;//根据个数搜索文本信息
	std::vector<int>texts2;//根据范围搜索文本信息
	std::vector<int>texts;//候选文本信息
						  //根据个数搜索
						  //Point temp_point;//!查找文本的搜索点
	double searchR = 0.0;
	auto mark = findColumnLeadInfoPoint(refPoint, searchR, _lineIndex, _bpLead, data);

	texts = data.m_kdtTreeData.kdtTexts().radiusSearch(refPoint, searchR);


	for (auto it1 : texts)
	{

		auto point = data.m_textPointData.textpoints()[it1];
		auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
		if (_bpLead && tt->text.find("@") != std::string::npos)
			continue;
		textPointIndex.push_back(it1);
	}

	return true;
}

bool ColumnSection::findColumnLeadInfoPoint(Point& point, double& searchR, const std::vector<int>& _lineIndex, const bool& _bpBox, Data& data)
{
#define Radius 1.8

	for (auto it : _lineIndex)
	{
		bool p_cover = true;
		Point l1; //默认将了l1当作覆盖的点
		Point l2;
		l1 = data.m_lineData.lines()[it].s;
		l2 = data.m_lineData.lines()[it].e;
		auto corners = data.m_cornerData.corners().find(it);
		if (corners == data.m_cornerData.corners().end())
			continue;
		auto it1 = corners->second;

		if (_lineIndex.size() > 1)
		{
			//!引出线都在箍筋的外部
			if (!_stirBox.cover(l1) && !_stirBox.cover(l2))
			{
				//goal_index = it;
				if (cornerFindPoint(it1, l1) && cornerFindPoint(it1, l2))
				{
					if (Line::vertical(l1, l2))
					{
						point = Point(l1.x, (l1.y + l2.y) / 2);
						searchR = data.m_lineData.lines()[it].length() * Radius / 2;
					}
					else
					{
						point = Point((l1.x + l2.x) / 2, l1.y);
						searchR = data.m_lineData.lines()[it].length() * Radius / 2;
					}
					return true;
				}
				else
				{
					if (!cornerFindPoint(it1, l1))
					{
						//point = l1;
						Point temp_point;
						double minLength = -1;
						for (auto p : it1)
						{
							auto length = TowPointsLength(p, l1);
							if (minLength == -1)
							{

								minLength = length;
								temp_point = p;
							}
							else if (minLength > length)
							{
								minLength = length;
								temp_point = p;
							}
						}
						if (Line::vertical(l1, temp_point))
						{
							searchR = minLength * Radius / 2;
							point = Point((l1.x + temp_point.x) / 2, l1.y);
						}
						else
						{
							searchR = minLength * Radius / 2;
							point = Point(l1.x, (l1.y + temp_point.y) / 2);
						}

					}
					else
					{
						//point = l2;
						double minLength = -1;
						Point temp_point;
						for (auto p : it1)
						{
							auto length = TowPointsLength(p, l2);
							if (minLength == -1)
							{

								minLength = length;
								temp_point = p;
							}
							else if (minLength > length)
							{
								minLength = length;
								temp_point = p;
							}
						}
						if (Line::vertical(l2, temp_point))
						{
							searchR = minLength * Radius / 2;
							point = Point(l2.x, (l2.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength * Radius / 2;
							point = Point((l2.x + temp_point.x) / 2, l2.y);
						}
						//searchR = minLength*1.2 / 2;
					}
					return false;
				}
			}
			else if (!_bpBox)//只针对箍筋的引出线
			{//引出线一边在箍筋内部，一边在古今外部
				if ((!_stirBox.cover(l1) && _stirBox.cover(l2)) || (_stirBox.cover(l1) && !_stirBox.cover(l2)))
				{

					if (!_stirBox.cover(l1))
					{
						//point = l1;
						Point temp_point;
						double minLength = -1;
						for (auto p : it1)
						{
							auto length = TowPointsLength(p, l1);
							if (minLength == -1)
							{
								temp_point = p;
								minLength = length;
							}
							else if (minLength > length)
							{
								temp_point = p;
								minLength = length;
							}
						}

						if (Line::vertical(l1, temp_point))
						{
							searchR = minLength * Radius / 2;
							point = Point(l1.x, (l1.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength * Radius / 2;
							point = Point((l1.x + temp_point.x) / 2, l1.y);
						}
						//searchR = minLength*1.2 / 2;

					}
					else
					{
						//point = l2;
						Point temp_point;
						double minLength = -1;
						for (auto p : it1)
						{
							auto length = TowPointsLength(p, l2);
							if (minLength == -1)
							{
								temp_point = p;
								minLength = length;
							}
							else if (minLength > length)
							{
								temp_point = p;
								minLength = length;
							}
						}
						if (Line::vertical(l2, temp_point))
						{
							searchR = minLength * Radius / 2;
							point = Point(l2.x, (l2.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength * Radius / 2;
							point = Point((l2.x + temp_point.x) / 2, l2.y);
						}
						//searchR = minLength*1.2 / 2;
					}
				}
				return false;
			}
		}
		else
		{
			//goal_index = it;
			if ((!_stirBox.cover(l1) && _stirBox.cover(l2)) || (_stirBox.cover(l1) && !_stirBox.cover(l2)))
			{

				if (!_stirBox.cover(l1))
				{
					//point = l1;
					Point temp_point;
					double minLength = -1;
					for (auto p : it1)
					{
						auto length = TowPointsLength(p, l1);
						if (minLength == -1)
						{
							temp_point = p;
							minLength = length;
						}
						else if (minLength > length)
						{
							temp_point = p;
							minLength = length;
						}
					}
					if (Line::vertical(l1, temp_point))
					{
						searchR = minLength * Radius / 2;
						point = Point(l1.x, (l1.y + temp_point.y) / 2);
					}
					else
					{
						searchR = minLength * Radius / 2;
						point = Point((l1.x + temp_point.x) / 2, l1.y);
					}
					//searchR = minLength*1.2 / 2;

				}
				else
				{
					//point = l2;
					double minLength = -1;
					Point temp_point;
					for (auto p : it1)
					{
						auto length = TowPointsLength(p, l2);
						if (minLength == -1)
						{
							temp_point = p;
							minLength = length;
						}
						else if (minLength > length)
						{
							temp_point = p;
							minLength = length;
						}
					}
					if (Line::vertical(l2, temp_point))
					{
						searchR = minLength * Radius / 2;
						point = Point(l2.x, (l2.y + temp_point.y) / 2);
					}
					else
					{
						searchR = minLength * Radius / 2;
						point = Point((l2.x + temp_point.x) / 2, l2.y);
					}
					//searchR = minLength*1.2 / 2;
				}
			}
			return false;
		}
	}
}

bool ColumnSection::returnLeadLineDir(char& ch, const bool& _bpBox, const std::vector<int> _leadLineIndex, Data& data)
{
	for (auto it : _leadLineIndex)
	{
		bool p_cover = true;
		Point l1; //默认将了l1当作覆盖的点
		Point l2;
		l1 = data.m_lineData.lines()[it].s;
		l2 = data.m_lineData.lines()[it].e;
		auto corners = data.m_cornerData.corners().find(it);
		auto it1 = corners->second;

		if (_leadLineIndex.size() > 1)
		{
			//!引出线都在箍筋的外部
			if (!_stirBox.cover(l1) && !_stirBox.cover(l2))
			{
				//goal_index = it;
				if (data.m_lineData.lines()[it].vertical())
				{
					ch = 'V';
				}
				else
				{
					ch = 'H';
				}
				return false;
			}
			else if (!_bpBox)//只针对箍筋的引出线
			{//引出线一边在箍筋内部，一边在古今外部
				if ((!_stirBox.cover(l1) && _stirBox.cover(l2)) || (_stirBox.cover(l1) && !_stirBox.cover(l2)))
				{


					if (data.m_lineData.lines()[it].vertical())
					{
						ch = 'V';
					}
					else
					{
						ch = 'H';
					}
					return false;
				}
			}
		}
		else
		{
			//goal_index = it;
			if ((!_stirBox.cover(l1) && _stirBox.cover(l2)) || (_stirBox.cover(l1) && !_stirBox.cover(l2)))
			{
				if (data.m_lineData.lines()[it].vertical())
				{
					ch = 'V';
				}
				else
				{
					ch = 'H';
				}
			}
			return false;
		}
	}
}

bool ColumnSection::setStirInfoType(Data& data)
{
	for (auto i = 0; i < _columnStirInfo.size(); ++i)//!多个引出线标注
	{
		bool singleStir = false, doubleStir = false;
		for (auto leadIndex : _columnStirInfo[i].stirLeadVec)//!每个箍筋的标注可能会有条因出现
		{
			auto corners = data.m_cornerData.corners().find(leadIndex);
			if (corners != data.m_cornerData.corners().end())//!每条引出线会有多条相交线
			{
				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == leadIndex)
					{
						l1 = corner.l2;
					}
					if (!singleStir && std::find(_singleStirIndex.begin(), _singleStirIndex.end(), l1) != _singleStirIndex.end())
					{
						singleStir = true;
					}
					else if (!doubleStir && std::find(_doubleStirIndex.begin(), _doubleStirIndex.end(), l1) != _doubleStirIndex.end())
					{
						doubleStir = true;
					}
					if (singleStir && doubleStir)
					{
						break;
					}
				}
			}
			if (singleStir && doubleStir)
			{
				break;
			}
		}

		if (singleStir && doubleStir)
		{
			_columnStirInfo[i].bSingle = true;
		}
		else
		{
			_columnStirInfo[i].bSingle = false;
		}
	}
	return true;
}

bool ColumnSection::setBPLeadInfoDirection(LineData& _lineData)
{
	/*此处以纵筋断点，在梁断面的区域位置判断纵筋描述的是属于那部分钢筋，
			主要思想是，有纵筋断点构成的box在，箍筋box的上侧，下侧，还是左侧，右侧,用其与箍筋边界相交的权值进行比较
			*/
	for (auto bpInfo = _columnBPinfoVec.begin(); bpInfo != _columnBPinfoVec.end(); ++bpInfo)
	{
		if (!bpInfo->bpInfoVec.empty())
		{
			int l = 0, r = 0, t = 0, b = 0;
			for (auto it : bpInfo->bpLeadVec)
			{
				Line line = _lineData.getLine(it);
				Corner tempCorner;
				if (line.vertical())
				{
					if (crossLineVH(line, Line(Point(_stirBox.left, _stirBox.top),
						Point(_stirBox.right, _stirBox.top)), tempCorner))
					{
						t++;
					}
					else if (crossLineVH(line, Line(Point(_stirBox.left, _stirBox.bottom),
						Point(_stirBox.right, _stirBox.bottom)), tempCorner))
					{
						b++;
					}
				}
				else if (line.horizontal())
				{
					if (crossLineVH(Line(Point(_stirBox.left, _stirBox.bottom),
						Point(_stirBox.left, _stirBox.top)), line, tempCorner))
					{
						l++;
					}
					else if (crossLineVH(Line(Point(_stirBox.right, _stirBox.bottom),
						Point(_stirBox.right, _stirBox.top)), line, tempCorner))
					{
						r++;
					}
				}
				else
				{
					if (crossLineS(line, Line(Point(_stirBox.left, _stirBox.top),
						Point(_stirBox.right, _stirBox.top)), tempCorner))
					{
						t++;
					}
					else if (crossLineS(line, Line(Point(_stirBox.left, _stirBox.bottom),
						Point(_stirBox.right, _stirBox.bottom)), tempCorner))
					{
						b++;
					}
					else if (crossLineS(line, Line(Point(_stirBox.left, _stirBox.bottom),
						Point(_stirBox.left, _stirBox.top)), tempCorner))
					{
						l++;
					}
					else if (crossLineS(line, Line(Point(_stirBox.right, _stirBox.bottom),
						Point(_stirBox.right, _stirBox.top)), tempCorner))
					{
						r++;
					}
				}

			}
			if (l > r&& l > t&& l > b)
			{
				bpInfo->dir = ColumnBPInfo::Direction::LEFT;
			}
			else if (r > l&& r > t&& r > b)
			{
				bpInfo->dir = ColumnBPInfo::Direction::RIGHT;
			}
			else if (t > r&& t > l&& t > b)
			{
				bpInfo->dir = ColumnBPInfo::Direction::TOP;
			}
			else if (b > t&& b > l&& b > r)
			{
				bpInfo->dir = ColumnBPInfo::Direction::BOTTOM;
			}
		}
	}
	return true;
}

bool ColumnSection::sortBreakPointToLine()
{
	//将可以成为一行或一列的纵筋初始化
	auto hBreakPointLine = setHBreakPointLine();
	auto vBreakPointLine = setVBreakPointLine();
	//将可以成为一列的纵筋组成一条知线
	auto hLines = setBreakPointVecToLine(hBreakPointLine, 'H');
	auto vLines = setBreakPointVecToLine(vBreakPointLine, 'V');
	
	//为纵筋所形成的线条排序
	std::vector<Line>temp_hLines, temp_vLines;
	temp_hLines = hLines;
	temp_vLines = vLines;
	LinesSort(temp_hLines, 'H');
	LinesSort(temp_vLines, 'V');
	//为每条纵筋线做纵筋映射
	for (auto line : temp_hLines)
	{
		for (int i = 0; i < hLines.size(); ++i)
		{
			if (line.s.y == hLines[i].s.y)
			{
				_bpHLine.push_back(std::pair<Line, std::vector<int>>(line, hBreakPointLine[i]));
				break;
			}
		}
	}
	for (auto line : temp_vLines)
	{
		for (int i = 0; i < vLines.size(); ++i)
		{
			if (line.s.x == vLines[i].s.x)
			{
				_bpVLine.push_back(std::pair<Line, std::vector<int>>(line, vBreakPointLine[i]));
				break;
			}
		}
	}
	return true;
}

bool ColumnSection::iniColumnDImensions(Data& data, const std::vector<std::shared_ptr<Dimension>>& _spDimensions)
{
	int l_idx = data.m_lineData.vLinesIndices()[_columnBlock.box.vf];
	int r_idx = data.m_lineData.vLinesIndices()[_columnBlock.box.vt];
	int t_idx = data.m_lineData.hLinesIndices()[_columnBlock.box.ht];
	int b_idx = data.m_lineData.hLinesIndices()[_columnBlock.box.hf];
	for (auto it : _spDimensions)
	{
		if (it->box.left + Precision > data.m_lineData.lines()[l_idx].s.x&&
			it->box.right - Precision < data.m_lineData.lines()[r_idx].s.x &&
			it->box.bottom + Precision > data.m_lineData.lines()[b_idx].s.y &&
			it->box.top - Precision < data.m_lineData.lines()[t_idx].s.y)
		{
			//初始化尺寸信息
			if (this->_seale == -1)
			{
				this->_seale = it->scale;
			}
			_dimensionsVec.push_back(*it);
		}
	}
	return true;
}

bool ColumnSection::iniMaxSize()
{
	Box candidateBox = _stirBox;
	for (auto dim : _dimensionsVec)
	{
		if (dim.direction == 90 || dim.direction == 270)
		{
			if (dim.box.left<_stirBox.left && dim.box.right>_stirBox.right)
			{
				_maxWidth = dim.measurement;
			}
			//!后备选择，用来描述截面的宽
			if (dim.box.left < _stirBox.left)
			{
				candidateBox.left = dim.box.left;
			}
			if (dim.box.right > _stirBox.right)
			{
				candidateBox.right = dim.box.right;
			}
		}
		else if (dim.direction == 0 || dim.direction == 180)
		{
			if (dim.box.bottom<_stirBox.bottom && dim.box.top>_stirBox.top)
			{
				_maxHeight = dim.measurement;
			}
			//!后备选择，用来描述截面的高
			if (dim.box.bottom < _stirBox.bottom)
			{
				candidateBox.bottom = dim.box.bottom;
			}
			if (dim.box.top > _stirBox.top)
			{
				candidateBox.top = dim.box.top;
			}
		}
	}

	if (_maxHeight == 0.0)
	{
		_maxHeight = std::abs(candidateBox.top - candidateBox.bottom) * _seale;
	}
	if (_maxWidth == 0.0)
	{
		_maxWidth = std::abs(candidateBox.right - candidateBox.left) * _seale;
	}

	return true;
}

std::vector<std::vector<int>> ColumnSection::setHBreakPointLine()
{
	std::vector<int>breakPointHMap;
	std::vector<std::vector<int>>hBreakPoint;
	for (int i = 0; i < break_pointVec.size() - 1; ++i)
	{
		if (std::find(breakPointHMap.begin(), breakPointHMap.end(), i) != breakPointHMap.end())
		{
			continue;
		}
		std::vector<int>tempHBP;
		tempHBP.push_back(i);
		breakPointHMap.push_back(i);
		for (int j = i; j < break_pointVec.size(); ++j)
		{
			if (i != j)
			{
				auto tempPointI = break_pointVec[i].center_mind;
				auto tempPointJ = break_pointVec[j].center_mind;
				double radius = break_pointVec[j].radius;

				if (std::abs(tempPointI.y - tempPointJ.y) < radius &&
					std::find(breakPointHMap.begin(), breakPointHMap.end(), j) == breakPointHMap.end())
				{
					tempHBP.push_back(j);
					breakPointHMap.push_back(j);
				}
			}
		}
		hBreakPoint.push_back(tempHBP);
	}
	return hBreakPoint;
}

std::vector<std::vector<int>> ColumnSection::setVBreakPointLine()
{
	std::vector<int>breakPointVMap;
	std::vector<std::vector<int>>vBreakPoint;
	for (int i=0;i< break_pointVec.size() - 1;++i)
	{
		if (std::find(breakPointVMap.begin(), breakPointVMap.end(), i) != breakPointVMap.end())
		{
			continue;
		}
		std::vector<int>tempVBP;
		tempVBP.push_back(i);
		breakPointVMap.push_back(i);
		for (int j = i; j < break_pointVec.size(); ++j)
		{
			if (i != j)
			{
				auto tempPointI = break_pointVec[i].center_mind;
				auto tempPointJ = break_pointVec[j].center_mind;
				double radius = break_pointVec[j].radius;

				if (std::abs(tempPointI.x - tempPointJ.x) < radius&&
					std::find(breakPointVMap.begin(), breakPointVMap.end(),j)== breakPointVMap.end())
				{
					tempVBP.push_back(j);
					breakPointVMap.push_back(j);
				}
			}
		}
		vBreakPoint.push_back(tempVBP);
	}

	return vBreakPoint;
}

std::vector<Line> ColumnSection::setBreakPointVecToLine(const std::vector<std::vector<int>>& BreakPointVec, const char& ch)
{
	std::vector<Line>lineVec;
	for (auto pointVec : BreakPointVec)
	{
		int startIndex = -1;
		int endIndex = -1;
		for (auto it : pointVec)
		{
			if (startIndex == -1)
			{
				startIndex = it;
				endIndex = it;
			}
			else
			{
				if (ch == 'H')
				{
					if (break_pointVec[startIndex].center_mind.x > break_pointVec[it].center_mind.x)
					{
						startIndex = it;
					}
					if (break_pointVec[endIndex].center_mind.x < break_pointVec[it].center_mind.x)
					{
						endIndex = it;
					}
				}
				else if(ch=='V')
				{
					if (break_pointVec[startIndex].center_mind.y > break_pointVec[it].center_mind.y)
					{
						startIndex = it;
					}
					if (break_pointVec[endIndex].center_mind.y < break_pointVec[it].center_mind.y)
					{
						endIndex = it;
					}
				}
				
			}
		}
		if (ch == 'H')
		{
			double xStart = break_pointVec[startIndex].center_mind.x;
			double xEnd = break_pointVec[endIndex].center_mind.x;
			double y = break_pointVec[startIndex].center_mind.y;
			lineVec.push_back(Line(Point(xStart, y), Point(xEnd, y)));
		}
		else if (ch == 'V')
		{
			double yStart = break_pointVec[startIndex].center_mind.y;
			double yEnd = break_pointVec[endIndex].center_mind.y;
			double x = break_pointVec[startIndex].center_mind.x;
			lineVec.push_back(Line(Point(x, yStart), Point(x, yEnd)));
		}

	}
	
	return lineVec;
}

Json::Value ColumnSection::stirrupVaule()
{
	Json::Value stirrup;
	//箍筋型号
	stirrup["all"] = _stirrup;
	//支数
	std::vector<Line>hLine;
	std::vector<Line>vLine;
	std::vector<Line>sLine;
	for (auto it : _columnStirrupVec)
	{
		for (auto line : it.circle_line)
		{
			if (line.horizontal())
			{
				hLine.push_back(line);
			}
			else if (line.vertical())
			{
				vLine.push_back(line);
			}
			else
			{
				sLine.push_back(line);
			}
		}
	}
	MergeLines(hLine, 'H');
	MergeLines(vLine, 'V');
	stirrup["count"] = numToString(hLine.size()) + "*" + numToString(vLine.size());
	//箍筋相对位置


	return Json::Value();
}

Json::Value ColumnSection::stirrupLoc()
{
	Json::Value stirrupLoc;
	Json::Value Single;
	Json::Value Double;
	if (_rectangle)
	{
		stirrupLoc["Count"] = numToString(_countV) + "x" + numToString(_countH);
		for (auto it : _columnStirrupVec)
		{
			if (it.single)
			{
				Line stirrupLine = it.circle_line.front();
				if (stirrupLine.horizontal())
				{
					Single.append(lineRelativeBPLoc(stirrupLine, 'H'));
				}
				else if (stirrupLine.vertical())
				{
					Single.append(lineRelativeBPLoc(stirrupLine, 'V'));
				}
			}
			else
			{
				Json::Value loc;
				for (auto line : it.circle_line)
				{
					if (line.horizontal())
					{
						loc.append(lineRelativeBPLoc(line, 'H'));
					}
					else if (line.vertical())
					{
						loc.append(lineRelativeBPLoc(line, 'V'));
					}
				}
				Double.append(loc);
			}
		}
		stirrupLoc["Single"] = Single;
		stirrupLoc["Double"] = Double;
	}
	return stirrupLoc;
}

Json::Value ColumnSection::lineRelativeBPLoc(const Line& line, const char& ch)
{
	Json::Value loc;
	double minLength = 65535;
	int markNum = 0;
	if (line.vertical())
	{
		for (int i = 0; i < _bpVLine.size(); ++i)
		{
			double length = std::abs(_bpVLine[i].first.s.x - line.s.x);
			if (length < minLength)
			{
				markNum = i;
				minLength = length;
			}
			else
			{
				break;
			}
		}
		if (line.s.x < _bpVLine[markNum].first.s.x)
		{
			loc["V"] = markNum;
			loc["direction"] = "L";
		}
		else
		{
			loc["V"] = markNum;
			loc["direction"] = "R";
		}
	}
	else if (line.horizontal())
	{
		for (int i = 0; i < _bpHLine.size(); ++i)
		{
			double length = std::abs(_bpHLine[i].first.s.y - line.s.y);
			if (length < minLength)
			{
				markNum = i;
				minLength = length;
			}
			else
			{
				break;
			}
		}
		if (line.s.y < _bpHLine[markNum].first.s.y)
		{
			loc["H"] = markNum;
			loc["direction"] = "B";
		}
		else
		{
			loc["H"] = markNum;
			loc["direction"] = "T";
		}
	}
	return loc;
}

Json::Value ColumnSection::breakPointInfo()
{
	Json::Value longitudinalBar;
	std::string corner, BBar, HBar;
	if (_rectangle)
	{
		if (_bpLeadLineVec.empty())
		{
			;//集中注写
		}
		else
		{
			//原位注写
			for (auto it : _columnBPinfoVec)
			{
				bool hDirection = false, vDirection = false;
				auto bpVec = it.bpVec;
				int goalIndex = 0, goalStartIndex = 0, goalEndIndex = 0;
				if (bpVec.size() > 1)
				{
					auto sIndex = bpVec.front();
					auto eIndex = bpVec.back();
					auto startP = break_pointVec[sIndex];
					auto endP = break_pointVec[eIndex];
					if (std::abs(startP.center_mind.y - endP.center_mind.y) < startP.radius)//横向
					{
						bool sMark = false;
						bool eMark = false;
						hDirection = true;
						//找出所在行
						for (int i = 0; i < _bpHLine.size(); ++i)
						{
							if (std::find(_bpHLine[i].second.begin(), _bpHLine[i].second.end(), sIndex) != _bpHLine[i].second.end())
							{
								goalIndex = i;
								break;
							}
						}
						//找出起始与终止的纵筋索引
						double minX = -1, maxX = -1;
						for (auto bpIndex : it.bpVec)
						{
							if (minX == -1)
							{
								minX = break_pointVec[bpIndex].center_mind.x;
								maxX = break_pointVec[bpIndex].center_mind.x;
								sIndex = bpIndex;
								eIndex = bpIndex;
							}
							else
							{
								if (minX > break_pointVec[bpIndex].center_mind.x)
								{
									minX = break_pointVec[bpIndex].center_mind.x;
									sIndex = bpIndex;

								}
								if (maxX < break_pointVec[bpIndex].center_mind.x)
								{
									maxX = break_pointVec[bpIndex].center_mind.x;
									eIndex = bpIndex;
								}
							}

						}
						//找出纵筋所在列的起始与终止
						for (int i = 0; i < _bpVLine.size(); ++i)
						{
							if (std::find(_bpVLine[i].second.begin(), _bpVLine[i].second.end(), sIndex) != _bpVLine[i].second.end())
							{
								goalStartIndex = i;
								sMark = true;
							}
							if (std::find(_bpVLine[i].second.begin(), _bpVLine[i].second.end(), eIndex) != _bpVLine[i].second.end())
							{
								goalEndIndex = i;
								eMark = true;
							}
							if (sMark && eMark)
							{
								break;
							}
						}

					}
					else if (std::abs(startP.center_mind.x - endP.center_mind.x) < startP.radius)//纵向
					{
						bool sMark = false;
						bool eMark = false;
						vDirection = true;
						for (int i = 0; i < _bpVLine.size(); ++i)
						{
							if (std::find(_bpVLine[i].second.begin(), _bpVLine[i].second.end(), sIndex) != _bpVLine[i].second.end())
							{
								goalIndex = i;
								break;
							}
						}

						//找出起始与终止的纵筋索引
						double minY = -1, maxY = -1;
						for (auto bpIndex : it.bpVec)
						{
							if (minY == -1)
							{
								minY = break_pointVec[bpIndex].center_mind.y;
								maxY = break_pointVec[bpIndex].center_mind.y;
								sIndex = bpIndex;
								eIndex = bpIndex;
							}
							else
							{
								if (minY > break_pointVec[bpIndex].center_mind.y)
								{
									minY = break_pointVec[bpIndex].center_mind.y;
									sIndex = bpIndex;

								}
								if (maxY < break_pointVec[bpIndex].center_mind.y)
								{
									maxY = break_pointVec[bpIndex].center_mind.y;
									eIndex = bpIndex;
								}
							}

						}

						for (int i = 0; i < _bpHLine.size(); ++i)
						{
							if (std::find(_bpHLine[i].second.begin(), _bpHLine[i].second.end(), sIndex) != _bpHLine[i].second.end())
							{
								goalStartIndex = i;
								sMark = true;
							}
							if (std::find(_bpHLine[i].second.begin(), _bpHLine[i].second.end(), eIndex) != _bpHLine[i].second.end())
							{
								goalEndIndex = i;
								eMark = true;
							}
							if (sMark && eMark)
							{
								break;
							}
						}
					}
				}
				else
				{
					;//一般情况不会出现
				}

				if (hDirection && !vDirection)
				{
					/*Json::Value barValue;
					barValue["Bar"] = it.bpInfoVec.front();
					barValue["H"] = numToString(goalIndex);
					barValue["V"] = numToString(goalStartIndex) + "-" + numToString(goalEndIndex);
					longitudinalBar.append(barValue);*/

					if (goalIndex == 0)
					{
						if (goalStartIndex == 0)
						{
							if (corner.empty())
							{
								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									corner = "4" + bpInfo.substr((long long)found + 1);
								}
								
							}
							if (BBar.empty())
							{
								if (goalEndIndex == _bpVLine.size() - 1)
								{
									if (!it.bpInfoVec.empty())
									{
										auto bpInfo = it.bpInfoVec.front();
										int found = static_cast<int>(bpInfo.find_first_of("%"));
										auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
										BBar = numToString(count - 2) + bpInfo.substr((long long)found + 1);
									}
									
								}
								else
								{
									if (!it.bpInfoVec.empty())
									{
										auto bpInfo = it.bpInfoVec.front();
										int found = static_cast<int>(bpInfo.find_first_of("%"));
										auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
										BBar = numToString(count - 1) + bpInfo.substr((long long)found + 1);
									}
									
								}
							}

						}
						else if (goalEndIndex == _bpVLine.size() - 1)
						{
							if (corner.empty())
							{
								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									corner = "4" + bpInfo.substr((long long)found + 1);
								}
								
							}
							if (BBar.empty())
							{
								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
									BBar = numToString(count - 1) + bpInfo.substr((long long)found + 1);
								}
							}
						}
						else
						{
							if (BBar.empty())
							{
								if (!it.bpInfoVec.empty())
									BBar = it.bpInfoVec.front();
							}
						}
					}
					else if (goalIndex == _bpHLine.size() - 1)
					{
						;
					}
				}
				else if (!hDirection && vDirection)
				{
					/*Json::Value barValue;
					barValue["Bar"] = it.bpInfoVec.front();
					barValue["H"] = numToString(goalStartIndex) + "-" + numToString(goalEndIndex);
					barValue["V"] = numToString(goalIndex);
					longitudinalBar.append(barValue);*/

					if (goalIndex == 0)
					{
						if (goalStartIndex == 0)
						{
							if (corner.empty())
							{
								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									corner = "4" + bpInfo.substr((long long)found + 1);
								}
								
							}
							if (HBar.empty())
							{
								if (goalEndIndex == _bpHLine.size() - 1)
								{
									if (!it.bpInfoVec.empty())
									{
										auto bpInfo = it.bpInfoVec.front();
										int found = static_cast<int>(bpInfo.find_first_of("%"));
										auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
										HBar = numToString(count - 2) + bpInfo.substr((long long)found + 1);
									}
									
								}
								else
								{
									if (!it.bpInfoVec.empty())
									{
										auto bpInfo = it.bpInfoVec.front();
										int found = static_cast<int>(bpInfo.find_first_of("%"));
										auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
										HBar = numToString(count - 1) + bpInfo.substr((long long)found + 1);
									}
									
								}
							}

						}
						else if (goalEndIndex == _bpVLine.size() - 1)
						{
							if (corner.empty())
							{
								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									corner = "4" + bpInfo.substr((long long)found + 1);
								}
								
							}
							if (HBar.empty())
							{

								if (!it.bpInfoVec.empty())
								{
									auto bpInfo = it.bpInfoVec.front();
									int found = static_cast<int>(bpInfo.find_first_of("%"));
									auto count = atoi(bpInfo.substr(0, (long long)found).c_str());
									HBar = numToString(count - 1) + bpInfo.substr((long long)found + 1);
								}
								
							}
						}
						else
						{
							if (!it.bpInfoVec.empty())
								HBar = it.bpInfoVec.front();
						}
					}
					else if (goalIndex == _bpVLine.size() - 1)
					{
						;
					}
				}

			}
		}
		longitudinalBar["AngleBar"] = corner;
		longitudinalBar["B_Bar"] = BBar;
		longitudinalBar["H_Bar"] = HBar;
	}
	return longitudinalBar;
}





