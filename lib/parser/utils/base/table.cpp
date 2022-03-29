#include "table.h"
#include "kdt.h"
#include "corner.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stack>
#include <string>
#include "json/json.h"


int maxArea(const std::vector<int>& arr, int offset, int size, int*f = nullptr, int*t = nullptr) {
	std::stack<std::pair<int, int>> s;
	int res = 0;
	for (int i = 0; i < size; ++i) {
		if (arr[offset + i] < 0) {
			if (s.empty())
				continue;
			int limit = abs(arr[offset + i]);
			int cnt = 0;
			int tmp = 0;
			int idx = 0;
			int tail = s.top().second;
			while (!s.empty() && s.top().first >= limit) {
				++cnt;
				tmp = s.top().first * cnt;
				idx = s.top().second;
				if (tmp > res && cnt > 2 && s.top().first > 2) {
					res = tmp;
					if (f != nullptr) {
						*f = s.top().second;
					}
					if (t != nullptr) {
						*t = tail;
					}
				}
				s.pop();
			}
			while (cnt > 0) {
				s.push(std::make_pair(limit, idx));
				--cnt;
			}
			continue;
		}
		if (s.empty() || s.top().first <= arr[offset + i]) {
			s.push(std::make_pair(arr[offset + i], i));
		}
		else {
			int cnt = 0;
			int tmp = 0;
			int idx = 0;
			int tail = s.top().second;
			while (!s.empty() && s.top().first >= arr[offset + i]) {
				++cnt;
				tmp = s.top().first * cnt;
				idx = s.top().second;
				if (tmp > res && cnt > 2 && s.top().first > 2) {
					res = tmp;
					if (f != nullptr) {
						*f = s.top().second;
					}
					if (t != nullptr) {
						*t = tail;
					}
				}
				s.pop();
			}
			while (cnt >= 0) {
				s.push(std::make_pair(arr[offset + i], idx));
				--cnt;
			}
		}
	}
	int cnt = 0;
	if (!s.empty()) {
		int tail = s.top().second;
		while (!s.empty()) {
			++cnt;
			int tmp = s.top().first * cnt;
			if (tmp > res && cnt > 2 && s.top().first > 2) {
				res = tmp;
				if (f != nullptr) {
					*f = s.top().second;
				}
				if (t != nullptr) {
					*t = tail;
				}
			}
			s.pop();
		}
	}
	return res;
}


bool fillGrid(std::vector<int>& grid, const std::vector<Line>& lines,
	const std::vector<int>& vlines,
	const std::vector<int>& hlines,
	const std::map<int, std::vector<Corner>>& corners) {

	std::vector<Corner> cs(vlines.size() * hlines.size());
	std::vector<int> ct(vlines.size() * hlines.size());

	for (int i = 0; i < hlines.size(); ++i) {
		auto c = corners.find(hlines[i]);
		if (c == corners.end())
			continue;
		for (auto cn : c->second) {
			cs[i * vlines.size() + cn.l1] = cn;
		}
	}

	std::vector<unsigned char> vopen(vlines.size(), 0);
	std::vector<int> pre(vlines.size(), 0);
	std::vector<Corner> pc(vlines.size());

	std::vector<double> deltas(vlines.size(), -1.0);

	//int left;
	int bottom;
	int left;

	for (int i = 0; i < hlines.size(); i++) {
		bool open = false;

		for (int j = 0; j < vlines.size(); j++) {
			bool corner = false;
			//v lines
			auto cn = cs[i*vlines.size() + j];
			switch (cn.type) {
				//left
			case Corner::CornerType::LEFTUP:
			case Corner::CornerType::LEFTRIGHT:
			case Corner::CornerType::DOWNLEFT:
				open = true;
				corner = true;
				break;
				//middle
			case Corner::CornerType::UPDOWN:
			case Corner::CornerType::CROSS:
			case Corner::CornerType::DOWNTUP:
				corner = true;
				open = true;
				break;
				//right
			case Corner::CornerType::UPRIGHT:
			case Corner::CornerType::RIGHTLEFT:
			case Corner::CornerType::RIGHTDOWN:
				open = false;
				corner = true;
				break;
			default:
				//vopen[j] = false;
				break;
			}
			//h lines
			switch (cn.type) {
				//middle
			case Corner::CornerType::LEFTRIGHT:
			case Corner::CornerType::CROSS:
			case Corner::CornerType::RIGHTLEFT:
				vopen[j] = true;
				break;
				//bottom
			case Corner::CornerType::RIGHTDOWN:
			case Corner::CornerType::DOWNTUP:
			case Corner::CornerType::DOWNLEFT:
				vopen[j] = true;
				break;
				//top
			case Corner::CornerType::LEFTUP:
			case Corner::CornerType::UPDOWN:
			case Corner::CornerType::UPRIGHT:
				vopen[j] = false;
				break;
			default:
				break;
			}
			if (corner) {
				if (deltas[j] < 0 || ((cn.y - pc[j].y) / deltas[j] > TableHeightPrecision
					&& (deltas[j] / (cn.y - pc[j].y)) > TableHeightPrecision)) {
					grid[i * vlines.size() + j] += pre[j] + 1;
					if (pc[j].type != Corner::UNKNOWN) {
						deltas[j] = cn.y - pc[j].y;
					}
					pc[j] = cn;
				}
				else {
					grid[i*vlines.size() + j] = 1;
					pc[j].type = Corner::UNKNOWN;
					deltas[j] = -1;
				}
				pre[j] = grid[i * vlines.size() + j];
			}
			else {
				if (vopen[j] || open) {
					grid[i * vlines.size() + j] = -1;
				}
				else {
					pre[j] = 0;
				}
			}
		}
	}
	for (int i = 1; i < hlines.size(); i++) {
		for (int j = 0; j < vlines.size(); j++) {
			if (grid[i*vlines.size() + j] < 0 && grid[(i - 1)*vlines.size() + j] < 0) {
				grid[i*vlines.size() + j] += grid[(i - 1)*vlines.size() + j];
			}
		}
	}
#ifdef PARSER_TEST_TABLE
	FILE* pf = fopen("a.txt", "a");

	for (int i = 0; i < hlines.size(); i++) {
		std::ostringstream ss;
		for (int j = 0; j < vlines.size(); j++) {
			ss << std::setw(4) << std::right << grid[i*vlines.size() + j] << "\t";
		}
		ss << std::endl;
		fwrite(ss.str().c_str(), 1, ss.str().size(), pf);
	}
	fclose(pf);
#endif
	return true;
}

std::map<int, std::map<int, std::pair<std::string, Point>/*std::shared_ptr<DRW_Text>*/>>& Table::extract(
	const kdt::KDTree<Point>& kdt,
	const std::vector<Point>& texts) {
	
	std::fstream of("TextLink.txt", std::ios::out);
		int pi = -1;
		int row = 0;//表格的行
		int col = 0;//表格的列
		double height = 0;
		//lefh,right,bottom,height,分别表示，表格的最左边，，最右边，最上边，最下边，的索引（指的都是直线）
		//vlines,hlines,分别存储表格的横线与纵线
		for (int i = left; i <= right; ++i) {
			//pass not cross
			if (vlines[i].s.y > hlines[top].s.y)
				continue;
			if (vlines[i].e.y < hlines[bottom].s.y)
				continue;

			int pj = -1;
			row = 0;

			for (int j = bottom; j <= top; ++j) {
				if (hlines[j].s.x > vlines[right].s.x)
					continue;
				if (hlines[j].e.x < vlines[left].s.x)
					continue;
				if (pj >= 0 && pi >= 0) {
					Line l = vlines[pi];
					Line r = vlines[i];
					Line t = hlines[j];
					Line b = hlines[pj];

					if (t.s.y - b.s.y > height) {
						height = t.s.y - b.s.y;
					}

					Point p((l.s.x + r.s.x) * 0.5, (b.s.y + t.s.y) * 0.5);
					auto cands = kdt.knnSearch(p, 16);//以p点为圆心，16为半径搜索，文本点
					//搜索单元格内的文本
					double cell_l = l.s.x;
					double cell_r = r.s.x;
					//text point index
					int t_p_indexL = textPointData.findLowerBound(b.s.y);
					int t_p_indexU = textPointData.findUpperBound(t.s.y);
					std::string str = "NULL";
					for (int t_p_index = t_p_indexL; t_p_index <= t_p_indexU; ++t_p_index)
					{
						auto &point = textPointData.textpoints()[t_p_index];
						if (point.x > cell_l - Precision&&point.x < cell_r + Precision)
						{
							if (str == "NULL")
								str.clear();
							exist_text = true;
							str += "|" + std::static_pointer_cast<DRW_Text>(point.entity)->text;
						}
					}
					if (str != "NULL")
					{
						auto it = table.find(row);
						if (it == table.end())
						{
							table[row] = std::map<int, std::pair<std::string, Point>/*std::shared_ptr<DRW_Text>*/>();
						}
						//std::shared_ptr<DRW_Text>temp_cell_text(new DRW_Text);
						//temp_cell_text->text = str;
						//cell_text.push_back(temp_cell_text);
						table[row].insert(std::make_pair(col, std::pair<std::string,Point>(str,p)/*temp_cell_text*/));
					}
					//for (auto c : cands) {
					//	Point pt = kdt.points_[c];
					//	DRW_Text* tt = static_cast<DRW_Text*>(pt.entity.get());
					//	if (pt.x > l.s.x && pt.x < r.s.x &&
					//		pt.y > b.s.y && pt.y < t.s.y) {
					//		exist_text = true;
					//		auto it = table.find(row);
					//		if (it == table.end()) {
					//			table[row] = std::map<int, std::shared_ptr<DRW_Text>>();
					//		}
					//		auto temp_txt = std::static_pointer_cast<DRW_Text>(pt.entity);
					//		of << temp_txt->text << std::endl;
					//		table[row].insert(std::make_pair(col, temp_txt/*std::static_pointer_cast<DRW_Text>(pt.entity)*/));
					//	}
					//}
				}
				++row;
				pj = j;
			}
			++col;
			pi = i;
		}
		of.close();
		this->height = height;
		return table;

}

int Table::minValue(const int & a, const int & b, const int & c)
{
	int min_temp = ((a < b) ? a : b) < c ? ((a < b) ? a : b) : c;
	return min_temp;
}

double Table::calculateStringDistance(std::string strA, std::string strB)
{
	int strA_length = strA.length();
	int strB_length = strB.length();

	int **c = NULL;
	c = new int*[strA_length + 1];
	for (int i = 0; i < strA_length + 1; ++i)
	{
		c[i] = new int[strB_length + 1];
	}
	for (int i = 0; i < strA_length + 1; ++i)
	{
		for (int j = 0; j < strB_length + 1; ++j)
		{
			c[i][j] = 0;
		}
	}

	/*下面两个for循环是为了赋初值的，我的理解是
	加入其中一个字符串位空，若要与空穿相同，则需要的变换次数，就是初值*/
	for (int i = 0; i < strA_length; i++)
	{
		c[i][strB_length] = strA_length - i;
	}
	for (int j = 0; j < strB_length; j++)
	{
		c[strA_length][j] = strB_length - j;
	}
	c[strA_length][strB_length] = 0;
	for (int i = strA_length - 1; i >= 0; i--)
		for (int j = strB_length - 1; j >= 0; j--)
		{
			if (strB[j] == strA[i])
				c[i][j] = c[i + 1][j + 1];
			else
				/*分析问题后可以得出四种状态转换过程，
				不同的状态转换过程对应的结果可以能有所不同，因此从不同的转换过程中取最优的*/
				c[i][j] = minValue(c[i][j + 1], c[i + 1][j], c[i + 1][j + 1]) + 1;
		}
	double rate = double(1.0 / (c[0][0] + 1));
	for (int i = 0; i < strA_length; ++i)
	{
		delete[]c[i];
		c[i] = NULL;
	}
	delete c;
	c = NULL;

	return rate;
}

int Table::returnAttributeArrangementPlus()
{
	//初始化table_cell
	setTableCell();

	//经过初始化后的table_cell，拆分单元格，并重新初始化
	splitCell();


	std::fstream of("charconert.txt", std::ios::out);
	for (int r = 0; r < row; ++r)
	{
		for (int c = 0; c < col; ++c)
		{
			of << table_cell[r][c].c_text << "\t";
		}
		of << std::endl;
	}
	of.close();

	//计算列的整齐度(计算行列的整齐的概率)
	double average_col = 0.0;
	double average_row = 0.0;
	calculationRowColRate(average_col, average_row);

	//判断属性的分布方式 
	if (average_col > average_row)			//列分布
	{

		//用于记录不为零且没有相同属性的行
		int row_top = 0;			//顶部行
		int row_bottom = 0;			//底部行
		findReadyRowAttribute(row_bottom, row_top);
		if (b_attr_repeat.begin()->first)
		{
			if (attr_repeat == HORIZONTAL_LEAD_PROERTY_TOP)
			{
				property_row[std::string("row")] = row_top + 1;
				property_col[std::string("col")] = 0;

				return HORIZONTAL_LEAD_PROERTY_TOP;
			}
			else
			{
				property_row[std::string("row")] = row_bottom + 1;
				property_col[std::string("col")] = 0;

				return HORIZONTAL_LEAD_PROERTY_BOTTOM;
			}
		}
		else
		{
			//求顶部行
			double average_temp_top_rate = 0.0;
			double average_temp_bottom_rate = 0.0;
			bool recheck_attribute = false;

			for (int c = 0; c < col; ++c)
			{
				average_temp_top_rate += table_cell[row_top][c].col_rate;
			}
			average_temp_top_rate /= col;

			//求底部行
			for (int c = 0; c < col; ++c)
			{
				average_temp_bottom_rate += table_cell[row_bottom][c].col_rate;
			}
			average_temp_bottom_rate /= col;

			/***********************/
			std::vector<int>potential_col;					//潜在的列有可能成为列属性
			std::vector<int>potential_col_step;				//潜在列所隔
			bool b_potential_col = false;					//潜在列是否存在默认不存在

															//初始化潜在的可能成为属性的列信息
			setColAttributeCorrectionInformation(potential_col, potential_col_step, b_potential_col);
			//迭代检测
			if (0 != potential_col.size())
			{
				for (int i = 0; i < potential_col.size(); ++i)
				{
					iterationCorrectionAttributeCol(i, potential_col, potential_col_step, b_potential_col);

					if (b_potential_col)
					{
						property_row[std::string("row")] = 0;
						property_col[std::string("col")] = potential_col[i] + 1;

						b_attr_repeat.clear();
						b_attr_repeat.insert({ b_potential_col,potential_col_step[i] });

						if (potential_col[i] < col / 2)
						{
							return COLUMN_LEAD_PROERTY_LEFT;
						}
						else
						{
							return COLUMN_LEAD_PROERTY_RIGHT;
						}
					}
				}
			}

			//属性在顶部的情况
			if (average_temp_top_rate < average_temp_bottom_rate)
			{
				property_row[std::string("row")] = row_top + 1;
				property_col[std::string("col")] = 0;

				return HORIZONTAL_LEAD_PROERTY_TOP;

			}
			else     //属性在底部的情况
			{
				property_row[std::string("row")] = row_bottom + 1;
				property_col[std::string("col")] = 0;

				return HORIZONTAL_LEAD_PROERTY_BOTTOM;
			}
		}

	}
	//行分布
	else
	{
		//用于记录需不相同且不为零的列
		int col_left = 0;	//最左边列
		int col_right = 0;	//最右边列
		findReadyColAttribute(col_left, col_right);

		if (b_attr_repeat.begin()->first)
		{
			if (attr_repeat == COLUMN_LEAD_PROERTY_LEFT)
			{
				property_row[std::string("x")] = 0;
				property_col[std::string("y")] = col_left + 1;

				return COLUMN_LEAD_PROERTY_LEFT;
			}
			else
			{
				property_row[std::string("x")] = 0;
				property_col[std::string("y")] = col_right + 1;

				return COLUMN_LEAD_PROERTY_RIGHT;
			}
		}
		else
		{

			double average_temp_left_rate = 0.0;
			double average_temp_right_rate = 0.0;
			//求左部列
			for (int r = 0; r < row; ++r)
			{
				average_temp_left_rate += table_cell[r][col_left].row_rate;
			}
			average_temp_left_rate /= col;

			//求右部列
			for (int r = 0; r < row; ++r)
			{
				average_temp_right_rate += table_cell[r][col_right].row_rate;
			}
			average_temp_right_rate /= col;

			/**************************************/
			std::vector<int>potential_row;					//潜在的列有可能成为行属性
			std::vector<int>potential_row_step;				//潜在行间隔
			bool b_potential_row = false;					//潜在行是否存在默认不存在

															//初始化潜在的可能成为属性的行信息
			setColAttributeCorrectionInformation(potential_row, potential_row_step, b_potential_row);
			//迭代检测
			if (0 != potential_row.size())
			{
				for (int i = 0; i < potential_row.size(); ++i)
				{
					iterationCorrectionAttributeRow(i, potential_row, potential_row_step, b_potential_row);

					if (b_potential_row)
					{
						property_row[std::string("x")] = potential_row[i] + 1;
						property_col[std::string("y")] = 0;

						b_attr_repeat.clear();
						b_attr_repeat.insert({ b_potential_row,potential_row_step[i] });

						if (potential_row[i] < row / 2)
						{
							return HORIZONTAL_LEAD_PROERTY_BOTTOM;
						}
						else
						{
							return HORIZONTAL_LEAD_PROERTY_TOP;
						}
					}
				}
			}

			//属性在左部的情况
			if (average_temp_left_rate <= average_temp_right_rate)
			{

				property_row[std::string("row")] = 0;
				property_col[std::string("col")] = col_left + 1;

				return COLUMN_LEAD_PROERTY_LEFT;
			}
			else     //属性在右部的情况
			{
				property_row[std::string("row")] = 0;
				property_col[std::string("col")] = col_right + 1;

				return COLUMN_LEAD_PROERTY_RIGHT;
			}
		}
	}
}

bool Table::tableExclude()
{
	return (!exist_text);
}

bool Table::steTextPoind(const TextPointData & textPointData)
{
	this->textPointData = textPointData;
	return true;
}

void Table::setCellBorder(std::map<int, int>& t_left, std::map<int, int>& t_right, std::map<int, int>& t_bottom, std::map<int, int>& t_top,
	const int &l, const int &r, const int &b, const int &t)
{
	//判断左边
	if (vlines[l].s.y - Precision < hlines[b].s.y&&vlines[l].e.y + Precision > hlines[t].s.y)
	{
		t_left[l] = 1;

	}
	else
	{
		t_left[l] = 0;

	}
	//判断右边
	if (vlines[r].s.y - Precision < hlines[b].s.y&&vlines[r].e.y + Precision > hlines[t].s.y)
	{
		t_right[r] = 1;

	}
	else
	{
		t_right[r] = 0;

	}
	//判断下边
	if (hlines[b].s.x - Precision < vlines[l].s.x&&hlines[b].e.x + Precision > vlines[r].s.x)
	{
		t_bottom[b] = 1;

	}
	else
	{
		t_bottom[b] = 0;

	}
	//判断上边
	if (hlines[t].s.x - Precision < vlines[l].s.x&&hlines[t].e.x + Precision > vlines[r].s.x)
	{
		t_top[t] = 1;

	}
	else
	{
		t_top[t] = 0;

	}
}

void Table::findReadyColAttribute(int & col_left, int & col_right)
{
	//搜寻左部列left
	for (int c = 0; c < col / 2; ++c)
	{
		int count = 0;
		bool same = false;
		std::vector<bool> same_step;
		int count_step = 0;
		std::vector<int> same_interval;	//属性纵向重复情况判断
		bool temp_step = false;

		for (int r = 0; r < row - 1; ++r)
		{
			if ((table_cell[r][c].col_rate == 0 && table_cell[r][c].c_text == "") || (table_cell[r + 1][c].col_rate == 0 && table_cell[r + 1][c].c_text == ""))
			{
				break;
			}
			for (int k = r + 1; k < row; ++k)
			{
				//根据行的第一个位置判断重复属性的步长
				if (r == 0)
				{
					count_step++;
					if (table_cell[r][c].c_text == table_cell[k][c].c_text)
					{
						same_step.push_back(true);
						same_interval.push_back(count_step);
					}
				}
				else if (table_cell[r][c].c_text == table_cell[k][c].c_text)
				{
					same = true;
					break;
				}
			}
			if (r == 0 && 0 != same_step.size())
			{
				//纵向属性重复处理
				//倍数关系

				dealColAttributeRrepeat(temp_step, same_interval, c);
			}
			else
			{
				if (same)
				{
					break;
				}
				else
				{
					count++;
				}
			}
		}
		if (temp_step)
		{
			//b_attr_repeat[temp_step] = same_interval[0];
			attr_repeat = COLUMN_LEAD_PROERTY_LEFT;
			col_left = c;
			break;
		}
		if (count + 1 == row)
		{
			col_left = c;
			break;
		}
	}


	//搜寻右部列right
	for (int c = col - 1; c >= col / 2 - 1; --c)
	{
		int count = 0;
		bool same = false;
		std::vector<bool> same_step;
		int count_step = 0;
		std::vector<int> same_interval;	//属性纵向重复情况判断
		bool temp_step = false;
		for (int r = 0; r < row - 1; ++r)
		{
			if ((table_cell[r][c].col_rate == 0 && table_cell[r][c].c_text == "") || (table_cell[r + 1][c].col_rate == 0 && table_cell[r + 1][c].c_text == ""))
			{
				break;
			}
			for (int k = r + 1; k < row; ++k)
			{
				//根据行的第一个位置判断重复属性的步长
				if (r == 0)
				{
					count_step++;
					if (table_cell[r][c].c_text == table_cell[k][c].c_text)
					{
						same_step.push_back(true);
						same_interval.push_back(count_step);
					}
				}
				else if (table_cell[r][c].c_text == table_cell[k][c].c_text)
				{
					same = true;
					break;
				}
			}
			if (r == 0 && 0 != same_step.size())
			{
				//纵向属性重复处理
				//倍数关系

				dealColAttributeRrepeat(temp_step, same_interval, c);
			}
			else
			{
				if (same)
				{
					break;
				}
				else
				{
					count++;
				}
			}

		}
		if (temp_step)
		{
			//b_attr_repeat[temp_step] = same_interval[0];
			attr_repeat = COLUMN_LEAD_PROERTY_RIGHT;
			col_left = c;
			break;
		}
		if (count + 1 == col)
		{
			col_right = c;
			break;
		}
	}
}

void Table::dealColAttributeRrepeat(bool & temp_step, std::vector<int> same_interval, const int & col)
{
	//纵向属性重复处理
	//倍数关系
	for (int i = 0; i < same_interval.size(); ++i)
	{
		if (same_interval[i] % same_interval[0] == 0 && same_interval[i] != 1 && same_interval[0] != 1)
		{
			temp_step = true;
			//break;
		}
		else
		{
			temp_step = false;
			break;
		}
	}
	if (!temp_step)
	{
		return;
	}
	for (int r = 0; r < row - same_interval.back(); ++r)
	{
		for (int step = 0; step < same_interval.size(); ++step)
		{
			if (table_cell[r][col].c_text == table_cell[r + step*same_interval[0]][col].c_text)
			{
				temp_step = true;
				//break;
			}
			else
			{
				temp_step = false;
				break;
			}
		}
		if (!temp_step)
		{
			break;
		}
	}
	if (!temp_step)
	{
		return;
	}
}

void Table::setColAttributeCorrectionInformation(std::vector<int>& potential_col, std::vector<int>& potential_col_step, bool & b_potential_col)
{
	for (int c = 0; c < col; ++c)
	{
		for (int r = 1; r < row / 2 + 1; ++r)
		{
			if (table_cell[0][c].c_text == table_cell[r][c].c_text)
			{
				potential_col.push_back(c);
				potential_col_step.push_back(r);
				break;
			}
		}
	}
}

void Table::setrowAttributeCorrectionInformation(std::vector<int>& potential_row, std::vector<int>& potential_row_step, bool & b_potential_row)
{
	for (int r = 0; r < row; ++r)
	{
		for (int c = 1; c < col / 2 + 1; ++c)
		{
			if (table_cell[r][0].c_text == table_cell[r][c].c_text)
			{
				potential_row.push_back(r);
				potential_row_step.push_back(c);
				break;
			}
		}
	}
}

void Table::iterationCorrectionAttributeRow(const int &num, std::vector<int>&potential_row,
	std::vector<int>&potential_row_step, bool &b_potential_row)
{
	if (0 != col % potential_row_step[num] && potential_row_step[num] == 1)
	{
		return;
	}
	else
	{
		for (int c = 0; c < potential_row_step[num]; ++c)
		{
			for (int step = 0; step < col / potential_row_step[num]; ++step)
			{
				if (table_cell[potential_row[num]][c].c_text == table_cell[potential_row[num]][c + step * potential_row_step[num]].c_text)
				{
					b_potential_row = true;
				}
				else
				{
					b_potential_row = false;
					break;
				}
			}
			if (!b_potential_row)
			{
				break;
			}
		}
	}

}

void Table::iterationCorrectionAttributeCol(const int & num, std::vector<int>& potential_col,
	std::vector<int>& potential_col_step, bool & b_potential_col)
{
	if (0 != row%potential_col_step[num] && potential_col_step[num] == 1)
	{
		return;
	}
	else
	{
		for (int r = 0; r < potential_col_step[num]; ++r)
		{
			for (int step = 0; step < row / potential_col_step[num]; ++step)
			{
				if (table_cell[r][potential_col[num]].c_text == table_cell[r + step * potential_col_step[num]][potential_col[num]].c_text)
				{
					b_potential_col = true;
				}
				else
				{
					b_potential_col = false;
					break;
				}
			}
			if (!b_potential_col)
			{
				break;
			}
		}
	}
}


void Table::setTableCell()
{
	row = top;
	col = right;
	int num = vlines.size();
	table_cell = std::vector<std::vector<cell>>(row, std::vector<cell>(col));

	for (int r = 1; r <= top; ++r)
	{

		for (int c = 1; c <= right; ++c)
		{
			std::map<int, int>t_left;
			std::map<int, int>t_right;
			std::map<int, int>t_bottom;
			std::map<int, int>t_top;
			cell temp_table;
			setCellBorder(t_left, t_right, t_bottom, t_top, c - 1, c, r - 1, r);

			if (table.find(r) != table.end() && table.find(r)->second.find(c) != table.find(r)->second.end())
			{
				table_cell[r - 1][c - 1].setCell(t_left, t_right, t_bottom, t_top, table[r][c].first);
			}
			else
			{
				table_cell[r - 1][c - 1].setCell(t_left, t_right, t_bottom, t_top, "");
			}
		}
	}


}

void Table::splitCell()
{
	int t_row = 0;
	int t_col = 0;
	int num1 = top;
	int num2 = bottom;
	int num3 = left;
	int num4 = right;
	int csize = vlines.size();
	int rsize = hlines.size();
	for (int r = 0; r < row; ++r)
	{
		for (int c = 0; c < col; ++c)
		{
			t_row = r;
			t_col = c;
			if (table_cell[t_row][t_col].c_text != "") {
				continue;
			}

			if (table_cell[r][c].c_left[t_col] != 0 &&
				table_cell[r][c].c_right[t_col + 1] != 0 &&
				table_cell[r][c].c_bottom[t_row] != 0 &&
				table_cell[r][c].c_top[t_row + 1] != 0) {
				continue;
			}
			//if()
			//向左查找
			while (table_cell[t_row][t_col].c_left[t_col] == 0 && t_col > 0)
			{
				t_col--;
				if (table_cell[t_row][t_col].c_text != "") {
					table_cell[r][c].c_text = table_cell[t_row][t_col].c_text;
					break;
				}
			}
			if (table_cell[t_row][t_col].c_text != "") {
				continue;
			}
			//向右查找
			while (table_cell[t_row][t_col].c_right[t_col + 1] == 0 && t_col + 1 < right - 1)
			{
				t_col++;
				if (table_cell[t_row][t_col].c_text != "")
				{
					table_cell[r][c].c_text = table_cell[t_row][t_col].c_text;
					break;
				}
			}
			if (table_cell[t_row][t_col].c_text != "")
			{
				continue;
			}
			//向下查找
			while (table_cell[t_row][t_col].c_bottom[t_row] == 0 && t_row > 0)
			{
				t_row--;
				if (table_cell[t_row][t_col].c_text != "")
				{
					table_cell[r][c].c_text = table_cell[t_row][t_col].c_text;
					break;
				}
			}
			if (table_cell[t_row][t_col].c_text != "")
			{
				continue;
			}
			//向上查找
			while (table_cell[t_row][t_col].c_top[t_row + 1] == 0 && t_row < top - 1)
			{
				t_row++;
				if (table_cell[t_row][t_col].c_text != "")
				{
					table_cell[r][c].c_text = table_cell[t_row][t_col].c_text;
					break;
				}
			}

		}
	}

}

void Table::calculationRowColRate(double & average_col, double & average_row)
{
	for (int c = 0; c < col; ++c)
	{
		int row_count = 0;
		double row_rate = 0.0;
		for (int r = 0; r < row; ++r)
		{
			double average_col_rate = 0.0;
			int count = 0;
			if (table_cell[r][c].c_text != "")
			{
				row_count++;
				for (int k = 0; k < row; ++k)
				{
					if (table_cell[k][c].c_text != ""&&r != k)
					{
						count++;
						average_col_rate += calculateStringDistance(table_cell[r][c].c_text, table_cell[k][c].c_text);
					}
				}
			}
			//求每个单元格其同一列其他单元格之间相似度的
			if (count != 0)
			{
				average_col_rate = average_col_rate / count;
			}
			else
			{
				average_col_rate = 0;
			}
			table_cell[r][c].col_rate = average_col_rate;
			row_rate += average_col_rate;
		}
		if (row_count != 0)
		{
			row_rate = row_rate / row_count;
		}
		else
		{
			row_rate = 0;
		}
		average_col = average_col + row_rate;
	}
	average_col = average_col / col;//用于判别表头的朝向
									//计算行的整齐度

	for (int r = 0; r < row; ++r)
	{
		int col_count = 0;
		double col_rate = 0.0;
		for (int c = 0; c < col; ++c)
		{
			double average_row_rate = 0.0;
			int count = 0;

			if (table_cell[r][c].c_text != "")
			{
				col_count++;
				for (int k = 0; k < col; ++k)
				{
					if (table_cell[r][k].c_text != ""&&c != k)
					{
						count++;
						average_row_rate += calculateStringDistance(table_cell[r][c].c_text, table_cell[r][k].c_text);
					}
				}
			}
			//求每个单元格其同一列其他单元格之间相似度的
			if (count != 0)
			{
				average_row_rate = average_row_rate / count;
			}
			else
			{
				average_row_rate = 0;
			}
			table_cell[r][c].row_rate = average_row_rate;
			col_rate = col_rate + average_row_rate;
		}
		if (col_count != 0)
		{
			col_rate = col_rate / col_count;
		}
		else
		{
			col_rate = 0;
		}
		average_row = average_row + col_rate;
	}
	average_row = average_row / row;
}

void Table::findReadyRowAttribute(int & row_bottom, int & row_top)
{
	//搜寻行底部行row_bottom
	for (int r = 0; r < row / 2; ++r)
	{
		int count = 0;
		bool same = false;
		std::vector<bool> same_step;
		int count_step = 0;
		std::vector<int> same_interval;	//属性横向重复情况判断
		bool temp_step = false;

		for (int c = 0; c < col - 1; ++c)
		{
			if ((table_cell[r][c].col_rate == 0 && table_cell[r][c].c_text == "") || (table_cell[r][c + 1].col_rate == 0 && table_cell[r][c + 1].c_text == ""))
			{
				break;
			}
			for (int k = c + 1; k < col; ++k)
			{
				//根据第一个位置判断属性重复步长
				if (c == 0)
				{
					count_step++;
					if (table_cell[r][c].c_text == table_cell[r][k].c_text)
					{
						same_step.push_back(true);
						same_interval.push_back(count_step);
					}
				}
				else if (table_cell[r][c].c_text == table_cell[r][k].c_text)
				{
					same = true;
					break;
				}
			}
			if (c == 0 && 0 != same_step.size())
			{
				//横向属性重复处理
				//需要向table里面添加“步长”以及“是否属性重复”的数据成员
				dealRowAttributeRrepeat(temp_step, same_interval, r);
				//break;
			}
			else
			{
				if (same)
				{
					break;
				}
				else
				{
					count++;
				}
			}

		}
		if (temp_step)
		{
			//b_attr_repeat[temp_step] = same_interval[0];
			b_attr_repeat.clear();
			b_attr_repeat.insert({ temp_step,same_interval[0] });

			row_bottom = r;
			attr_repeat = HORIZONTAL_LEAD_PROERTY_BOTTOM;
			break;
		}
		if (count + 1 == col)
		{
			row_bottom = r;
			break;
		}
	}


	//搜寻顶部行row_top

	for (int r = row - 1; r >= row / 2 - 1; --r)
	{
		int count = 0;
		bool same = false;
		std::vector<bool> same_step;
		int count_step = 0;
		std::vector<int> same_interval;	//属性横向重复情况判断
		bool temp_step = false;

		for (int c = 0; c < col - 1; ++c)
		{
			if ((table_cell[r][c].col_rate == 0 && table_cell[r][c].c_text == "") || (table_cell[r][c + 1].col_rate == 0 && table_cell[r][c + 1].c_text == ""))
			{
				break;
			}
			for (int k = c + 1; k < col; ++k)
			{
				//根据第一个位置判断属性重复步长
				if (c == 0)
				{
					count_step++;
					if (table_cell[r][c].c_text == table_cell[r][k].c_text)
					{
						same_step.push_back(true);
						same_interval.push_back(count_step);
					}
				}
				else if (table_cell[r][c].c_text == table_cell[r][k].c_text)
				{
					same = true;
					break;
				}
			}
			if (c == 0 && 0 != same_step.size())
			{
				//横向属性重复处理
				//需要向table里面添加“步长”以及“是否属性重复”的数据成员

				dealRowAttributeRrepeat(temp_step, same_interval, r);
				//break;
			}
			else
			{
				if (same)
				{
					break;
				}
				else
				{
					count++;
				}
			}

		}

		if (temp_step)
		{
			//b_attr_repeat[temp_step] = same_interval[0];
			b_attr_repeat.clear();
			b_attr_repeat.insert({ temp_step,same_interval[0] });

			row_top = r;
			attr_repeat = HORIZONTAL_LEAD_PROERTY_TOP;
			break;
		}

		if (count + 1 == col)
		{
			row_top = r;
			break;
		}
	}
}

void Table::dealRowAttributeRrepeat(bool & temp_step, std::vector<int> same_interval, const int & row)
{
	for (int i = 0; i < same_interval.size(); ++i)
	{
		if (same_interval[i] % same_interval[0] == 0 && same_interval[i] != 1 && same_interval[0] != 1)
		{
			temp_step = true;
			//break;
		}
		else
		{
			temp_step = false;
			break;
		}
	}
	if (!temp_step)
	{
		return;
	}
	for (int c = 0; c < col - same_interval.back(); ++c)
	{
		for (int step = 0; step < same_interval.size(); ++step)
		{
			if (table_cell[row][c].c_text == table_cell[row][c + step*same_interval[0]].c_text)
			{
				temp_step = true;
				//break;
			}
			else
			{
				temp_step = false;
				break;
			}
		}
		if (!temp_step)
		{
			break;
		}
	}

	return;
}






std::shared_ptr<Table> Table::buildFromCrossLines(std::vector<Line>& vlines,
	std::vector<Line>& hlines, double px, double py, const Box& box) {

	std::vector<Line> lines;
	std::map<int, std::vector<Corner>> corners;
	std::vector<int> vs;
	std::vector<int> hs;
	typedef struct {
		int area;
		int left;
		int right;
		int top;
		int bottom;
	}CandTable;

	//cross corners
	lines.insert(lines.end(), vlines.begin(), vlines.end());
	lines.insert(lines.end(), hlines.begin(), hlines.end());

	for (int i = 0; i < hlines.size(); ++i) {
		hs.push_back(vlines.size() + i);
	}
	for (int i = 0; i < vlines.size(); ++i) {
		vs.push_back(i);
	}
	for (int i = 0; i < vs.size(); ++i) {
		double yb = lines[vs[i]].s.y;
		double yu = lines[vs[i]].e.y;
		int f = findLowerBound(yb, hs, lines, py);
		int t = findUpperBound(yu, hs, lines, py);
		for (f; f <= t && f < hs.size(); ++f) {
			crossLinesVH(lines, i, hs[f], corners, px, py);
		}
	}

	std::vector<int> grid(vlines.size() * hlines.size());

	//1.fill grid
	fillGrid(grid, lines, vs, hs, corners);

	//2.c max area per line
	std::vector<CandTable> cands;
	for (int i = 2; i < hlines.size(); ++i) {
		CandTable cand;
		cand.left = -1;
		cand.right = -1;
		//int l = -1;
		//int r = -1;
		//grid.
		int area = maxArea(grid, i * vlines.size(), vlines.size(), &cand.left, &cand.right);
		if (cand.left < 0 || cand.right < 0)
			continue;
		cand.area = area;
		cand.top = i;
		cands.push_back(cand);
		for (int j = cands.size() - 1; j > 0; --j) {
			if (cands[j].area > cands[j - 1].area) {
				auto t = cands[j];
				cands[j] = cands[j - 1];
				cands[j - 1] = t;
			}
		}
	}

	//suite table
	std::shared_ptr<Table> table;
	for (int k = 0; /*k < 2 &&*/ k < cands.size(); ++k) {
		auto c = cands[k];
		int max = c.area;
		int left = c.left;
		int right = c.right;
		int top = c.top;
		int bottom = c.bottom;
		if (max > MIN_TABLE_LINES) {
			int i;
			int min = grid[top * vlines.size() + left];
			int l = left;
			for (int i = left; i < right; ++i) {
				if ((grid[top * vlines.size() + i] > 0 && grid[top * vlines.size() + i] < min) || min < 0) {
					min = grid[top * vlines.size() + i];
					l = i;
				}
			}
			for (i = top; grid[i * vlines.size() + l] != 1 && i > 0; --i);
			bottom = i;

			if (bottom < 0 || bottom >= hlines.size() || top < 0 || top >= hlines.size() ||
				left < 0 || left >= vlines.size() || right <= 0 || right >= vlines.size()) {

				bottom = bottom;
			}

			double ar = (vlines[right].s.x - vlines[left].s.x) * (hlines[top].s.y - hlines[bottom].s.y);
			//table.reset(new Table(vlines, hlines, left, top, right, bottom));
			//table->cells = max;
			//pass small tables
			if (ar > box.area() * 0.1) {
				/*table.reset(new Table(vlines, hlines, left, top, right, bottom));
				table->cells = max;
				break;*/

				findBorder(vlines, hlines, top, bottom, left, right, corners);
				table.reset(new Table(vlines, hlines, left, top, right, bottom));
				table->cells = max;
				break;
			}
			else {
				const double percent = 0.70;
				auto width = box.width();
				auto height = box.height();
				/*if ((vlines[left].s.x > box.left + (width *  percent) ||
				hlines[top].s.y < box.bottom + (height *(1 - percent))) &&
				(vlines[right].s.x + px >= box.right || hlines[bottom].s.y - py <= box.bottom)) {
				table.reset(new Table(vlines, hlines, left, top, right, bottom));
				table->cells = max;
				table->side = true;
				break;
				}*/

				findBorder(vlines, hlines, top, bottom, left, right, px, py, box, corners);
				table.reset(new Table(vlines, hlines, left, top, right, bottom));
				table->cells = max;
				table->side = true;
				break;


			}
		}
	}

	return table;
}

void Table::findBorder(std::vector<Line>& vlines, std::vector<Line>& hlines, int & top, int & bottom, int & left, int & right, std::map<int, std::vector<Corner>> corners)
{
	int vlsize = vlines.size();
	int hsize = hlines.size();
	while (corners.find(vlsize + top) != corners.end() &&
		corners[vlsize + top].front().type != Corner::CornerType::LEFTUP &&
		(corners[vlsize + top].end() - 1)->type != Corner::CornerType::UPRIGHT) {
		if (top < hlines.size()) {
			top++;
			if (top >= hlines.size()) {
				top = hlines.size() - 1;
				break;
			}
		}

		/*else {
		top = hlines.size() - 1;
		break;
		}*/

	}
	//存在错误
	while (corners.find(vlsize + bottom) != corners.end() &&
		corners[vlsize + bottom].front().type != Corner::CornerType::DOWNLEFT &&
		(corners[vlsize + bottom].end() - 1)->type != Corner::CornerType::RIGHTDOWN) {

		if (bottom >= 0) {
			bottom--;
			if (bottom <= 0) {
				bottom = 0;
				break;
			}
		}
		/*else{
		bottom = 0;
		break;
		}*/
	}
	while (corners.find(left) != corners.end() &&
		corners[left].front().type != Corner::CornerType::DOWNLEFT &&
		(corners[left].end() - 1)->type != Corner::CornerType::LEFTUP) {

		if (left >= 0) {
			left--;
			if (left <= 0) {
				left = 0;
				break;
			}
		}
		/*else {
		left = 0;
		break;
		}*/
	}
	while (corners.find(right) != corners.end() && corners[right].front().type != Corner::CornerType::RIGHTDOWN &&
		(corners[right].end() - 1)->type != Corner::CornerType::UPRIGHT) {

		if (right < vlines.size()) {
			right++;
			if (right >= vlines.size()) {
				right = vlines.size() - 1;
				break;
			}
		}
		/*else {
		right = vlines.size() - 1;
		break;
		}*/
	}
}

void Table::findBorder(std::vector<Line>& vlines, std::vector<Line>& hlines, int & top, int & bottom, int & left, int & right, double px, double py, const Box & box, std::map<int, std::vector<Corner>> corners)
{
	const double percent = 0.70;
	auto width = box.width();
	auto height = box.height();
	if (vlines[left].s.x > box.left + (width *  percent) && vlines[right].s.x + px >= box.right)
	{
		//遇到后添加处理
		;
	}
	else if (vlines[left].s.x > box.left + (width *  percent) && hlines[bottom].s.y - py <= box.bottom)
	{
		int vsize = vlines.size();
		top = hlines.size() - 1;
		hlines[top].s.x = vlines[left].e.x;
		hlines[bottom].s.x = vlines[left].s.x;
		vlines.erase(vlines.begin(), vlines.begin() + left);
		left = 0;
		right = right;
		;

	}
	else if (hlines[top].s.y < box.bottom + (height *(1 - percent)) && vlines[right].s.x + px >= box.right)
	{
		//遇到后添加处理
		;

	}
	else if (hlines[top].s.y < box.bottom + (height *(1 - percent)) && hlines[bottom].s.y - py <= box.bottom)
	{
		//遇到后添加处理程序
		;

	}
	else
	{
		findBorder(vlines, hlines, top, bottom, left, right, corners);
	}
}



std::shared_ptr<Table> Table::buildFromComponent(const Component& component) {
	std::vector<Line> vlines;
	std::vector<Line> hlines;

	component.expandBorder(vlines, hlines);

	return Table::buildFromCrossLines(vlines, hlines, component.precisionx, component.precisiony, component.box);
}



//离散化并求最大面积？
std::shared_ptr<Table> buildFromCrossLinesT(const std::vector<Line>& lines,
	const std::vector<int>& vlines,
	const std::vector<int>& hlines,
	const std::map<int, std::vector<Corner>>& corners) {
	size_t size = lines.size();

	std::vector<char> map(size* Corner::NUMBER);
	std::vector<char> mark(size);

	for (int i = 0; i < lines.size(); ++i) {
		auto it = corners.find(i);
		if (it == corners.end())
			continue;
		for (auto jt : it->second) {
			++map[i * Corner::NUMBER + jt.type];
		}
	}

	struct Tb {
		Tb() :left(-1), right(-1), bottom(-1), top(-1) {}
		int left;
		int right;
		int bottom;
		int top;
	};
	for (int i = 0; i < vlines.size(); ++i) {
		//int idx = vlines[i];
		//if ) {
		//	continue;
		//}

		//auto cs = corners.find(idx);
		//bool bottom = false;
		//Corner cn;
		//double delta = -1.0;
		//for (auto it : cs->second) {
		//	if (!bottom && map[it.l2]) {
		//		
		//	}
		//}
	}

	return std::shared_ptr<Table>();
}

Table::cell::cell()
{
}

void Table::cell::setCell(const std::map<int, int> & t_left, const std::map<int, int> & t_right,
	const std::map<int, int> & t_bottom, const std::map<int, int> & t_top, const std::string t_text)
{

	this->c_left = t_left;
	this->c_right = t_right;
	this->c_top = t_top;
	this->c_bottom = t_bottom;
	this->c_text = t_text;
}
