#include "pillarSet.h"

ColumnsTextPoint::ColumnsTextPoint(const Point& point, const std::string& name, bool bLeadLine)
{
	this->m_point = point;
	this->m_name = name;
	this->m_bLeadLine = bLeadLine;
}
ColumnsLeadGroup::ColumnsLeadGroup(/*const std::vector<int>& primevalIndex,*/
								   const std::vector<int>& nowIndex, int textPointIndex, bool pair)
{
	//this->m_primevalIndex = primevalIndex;
	this->m_nowIndex = nowIndex;
	this->m_textPointIndex = textPointIndex;
	this->m_pair = pair;
}
ColumnsBorderGroup::ColumnsBorderGroup(bool bTextPoint, bool bgrop, bool blead)
{
	this->m_btextPoint = bTextPoint;
	this->m_bgrop = bgrop;
	this->m_blead = blead;
}


bool PillarSet::findColumns(Data& data, std::vector<std::shared_ptr<Pillar>>& _pillars)
{
	//查找柱边界图层
	findColumnLayer(data.m_lineData,data.m_cornerData);
	//查找所有柱图层线
	findColumnLines(data.m_lineData);
	//查找柱文本的标志点
	findTextCandidatesPoints(data.m_textPointData);
	//!有些图纸没有柱的相关信息就不要去做下面的操作，可以节省大量时间
	//if (!m_textPoint.empty())
	{
		//查找引出线组的数据
		findLeadLines(data);
		initializationLeadLineMap();
		//初始化柱图层数据 
		initializationBeamLinesCorners();
		//查找柱boar数据
		findBorderGrop(data);
		//初始化leadGrop,borderGrop,textPoint数据状态
		initializationColumnType();
		//后续还需继续补充完整几类情况
		returnColumnAverageWidthHeight();//柱的平均宽高
		//优化不成组的情况								 
		std::vector<int>test_index;
		//optFBorderGrop(test_index);//优化未成组且引出线未找到引出线的情况
		std::vector<int>test1_index;
		//优化文本点没有找到引出线的情况
		optFLTextPoint(test_index);

		/*柱信息整理*/
		columnInformation(_pillars);
	}





	return false;
}

std::vector<std::pair<int,int>> PillarSet::returnColumnBorderGroupIndex()
{
	std::vector<std::pair<int, int>>borderGropIndexVec;//第二层数组一般情况下只会有两条索引，且第一天表示上部，第二表示下部
	for (auto it : m_BorderGrops)
	{
		//std::pair<int, int>tempPair;
		std::vector<int>tempBorders;
		int topIndex = -1;
		int bottomIndex = -1;
		if (it.m_bgrop)
		{
			for (auto index : it.m_borders)
			{
				//取出其中的横线，给梁识别用于寻找梁图层
				if (m_columnLines.getLine(index).horizontal())
				{
					tempBorders.push_back(index);

				}
			}

			for (auto border : tempBorders)
			{
				if (topIndex == -1)
				{
					topIndex = border;
				}
				else
				{
					if (m_columnLines.getLine(topIndex).s.y < m_columnLines.getLine(border).s.y)
					{
						topIndex = border;
					}
				}

				if (bottomIndex == -1)
				{
					bottomIndex = border;
				}
				else
				{
					if (m_columnLines.getLine(bottomIndex).s.y > m_columnLines.getLine(border).s.y)
					{
						bottomIndex = border;
					}
				}
			}
			if (topIndex != -1 && bottomIndex != -1)
				borderGropIndexVec.push_back(std::pair<int, int>(m_primevalIndex[topIndex], m_primevalIndex[bottomIndex]));
		}
	}
	return borderGropIndexVec;
}

bool PillarSet::findColumnLayer(LineData& lineData,CornerData& cornerData)
{
	std::map<std::string, int>candidateColumnLayer;
	for (auto it:m_axises->vlines_coo)
	{
		const Line& axisLine = it.first.second;
		const int& axisIndex = findLineIndex(lineData, axisLine,'V');
		if (-1 != axisIndex)
		{
			auto corners = cornerData.corners().find(axisIndex);
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				if (l1 == axisIndex)
				{
					l1 = corner.l2;
				}
				const Line& tempLine = lineData.getLine(l1);
				if (tempLine.horizontal())
				{
					const double& midX = (tempLine.s.x + tempLine.e.x) / 2;
					const Point& midPoint = Point(midX, tempLine.s.y);//!获取某线的中点
					if (ConnectPoint(midPoint, corner))
					{
						candidateColumnLayer[tempLine.getLayer()]++;
					}
				}

			}

		}
	}
	for (auto it : m_axises->hlines_coo)
	{
		const Line& axisLine = it.first.second;
		const int& axisIndex = findLineIndex(lineData, axisLine, 'H');

		if (-1 != axisIndex)
		{
			auto corners = cornerData.corners().find(axisIndex);
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				if (l1 == axisIndex)
				{
					l1 = corner.l2;
				}
				const Line& tempLine = lineData.getLine(l1);
				if (tempLine.vertical())
				{
					const double& midY = (tempLine.s.y + tempLine.e.y) / 2;
					const Point& midPoint = Point(tempLine.s.x, midY);//!获取某线的中点
					if (ConnectPoint(midPoint, corner))
					{
						candidateColumnLayer[tempLine.getLayer()]++;
					}
				}

			}

		}
	}
	

	for (auto it : candidateColumnLayer)
	{
		if (_columnLayer.empty())
		{
			_columnLayer = it.first;
		}
		else if (candidateColumnLayer[_columnLayer] < it.second)
		{
			_columnLayer = it.first;
		}
	}

	return true;
}

bool PillarSet::findColumnLines(LineData& lineData)
{
	std::fstream of("laryer.txt", std::ios::out);
	for (int i = 0; i < lineData.lines().size(); ++i)
	{
		std::string str = lineData.lines()[i].getLayer();

		//std::string ch1 = GbkToUtf8("柱");
		////std::string ch2 = "Z";
		//std::string ch3 = "Column";
		//std::string ch4 = "COLU";
		//std::string ch5 = "COLUMN";
		//if (str.find(ch1) != std::string::npos ||
		//	/*str.find(ch2) != std::string::npos ||*/
		//	str.find(ch3) != std::string::npos ||
		//	str.find(ch4) != std::string::npos||
		//	str.find(ch5) != std::string::npos)
		if(str==_columnLayer)
		{
			//borderLine.push_back(data.m_lineData.lines()[i]);
			int num = m_columnLines.lines().size();
			m_columnLines.pushBackLines(lineData.lines()[i]);
			m_primevalIndex.push_back(i);
			pushRTreeLines(lineData.lines()[i], num, m_rTreeColumnLines);
		}
		of << str << std::endl;
	}
	of.close();
	return true;
}

bool PillarSet::findTextCandidatesPoints(TextPointData& textPointData)
{
	
		double left = m_axises->box.left;
		double right = m_axises->box.right;
		int i = textPointData.findLowerBound(m_axises->box.bottom);
		int j = textPointData.findUpperBound(m_axises->box.top);

		//通过标注符号查找候选集
		for (i; i <= j; ++i)
		{
			auto& point = textPointData.textpoints()[i];
			if (point.x > left - Precision && point.x < right + Precision)
			{
				auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
				if (tt->text.find("Z") != std::string::npos && tt->text.find("L") == std::string::npos)
				{
					m_textPoint.push_back(ColumnsTextPoint(point, tt->text));
				}
			}
		}
	
	return true;
}

bool PillarSet::findLeadLines(Data& data)
{
	for (int i = 0; i < m_textPoint.size(); ++i)
	{
		int goal_index = -1;

		//文本附近最近的三个点
		auto ends = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, m_textPoint[i].m_point), 3);
		for (auto end : ends)
		{
			auto e = data.m_endpointData.getEndpoint(end);
			auto corners = data.m_cornerData.corners().find(e.index);
			if (corners == data.m_cornerData.corners().end())
				continue;

			auto& line = data.m_lineData.lines()[e.index];
			bool b_connect_s = false;
			bool b_connect_e = false;


			for (auto corner : corners->second)
			{

				if (b_connect_s && b_connect_e)
					break;
				if (corner.distance(line.s) < Precision)
					b_connect_s = true;
				if (corner.distance(line.e) < Precision)
					b_connect_e = true;
			}

			//判断是否为引出线 根据引出线的特性（必然有一端悬空进行判断）

			if ((b_connect_s && b_connect_e) ||
				(!b_connect_s && !b_connect_e)/*此处有问题*/
				)
				continue;

			goal_index = e.index;
			break;
		}
		if (goal_index != -1)
		{
			m_textPoint[i].m_bLeadLine = true;
			//std::vector<int>primevalIndex;
			std::vector<int>nowIndex;
			std::pair<int, int>cornerLeadIndex;
			std::vector<int>dlines;//引出线索引
			int cornerLead = 0;

			//查找leadGrop
			findRelatedLineIndex(goal_index, data, dlines, cornerLead);
			for (auto it : dlines)
			{
				auto ite = std::find(m_primevalIndex.begin(), m_primevalIndex.end(), it);
				if (ite != m_primevalIndex.end())
				{
					int num = std::distance(m_primevalIndex.begin(), ite);
					nowIndex.push_back(num);
				}
				else
				{
					int num = m_columnLines.lines().size();
					m_columnLines.pushBackLines(data.m_lineData.lines()[it]);
					m_primevalIndex.push_back(it);
					nowIndex.push_back(num);
					pushRTreeLines(data.m_lineData.lines()[it], num, m_rTreeColumnLines);
				}

			}

			//初始化信息
			ColumnsLeadGroup temp_leadGrop(/*primevalIndex, */nowIndex, i);
			int leadGropSize = m_leadGrops.size();
			for (auto it : nowIndex)
			{
				m_mapLeadGropIndex[it] = leadGropSize;
			}
			m_leadGrops.push_back(temp_leadGrop);


		}

	}
	return true;
}

bool PillarSet::findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, int& cornerIndex)
{
	auto& line = data.m_lineData.lines()[relateLine];
	const auto& layer = line.getLayer();
	dline.push_back(relateLine);

	std::queue<int>lead_queue;
	lead_queue.push(relateLine);
	while (!lead_queue.empty())
	{
		int index = lead_queue.front();
		Line line_queue_front = data.m_lineData.lines()[index];
		lead_queue.pop();

		auto corners = data.m_cornerData.corners().find(index);

		//相交处理
		if (corners != data.m_cornerData.corners().end())
		{
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				if (l1 == index)
				{
					l1 = corner.l2;
				}
				if (data.m_lineData.lines()[l1].getLayer() == layer &&
					std::find(dline.begin(), dline.end(), l1) == dline.end())
				{
					lead_queue.push(l1);
					dline.push_back(l1);

				}
			}
		}
	}
	//给出引出线与柱图层线相交的leadLine
	if (dline.size() == 1)
	{
		cornerIndex = dline.front();
	}
	else
	{
		int cornerNum = 0;
		int cornerLeadIndex = -1;
		for (auto it : dline)
		{
			auto corners = data.m_cornerData.corners().find(it);
			auto cornerSize = corners->second.size();
			if (cornerLeadIndex == -1)
			{
				cornerIndex = cornerSize;
				cornerLeadIndex = it;
			}
			else if (cornerIndex < cornerSize)
			{
				cornerIndex = cornerSize;
				cornerLeadIndex = it;
			}
		}
	}
	return true;
}

bool PillarSet::initializationBeamLinesCorners()
{
	std::vector<int>hLinesIndex;
	std::vector<int>vLinesIndex;
	std::vector<int>sLinesIndex;
	lineTypeVHSIndex(m_columnLines.lines(), vLinesIndex, hLinesIndex, sLinesIndex);
	m_columnLines.setVLinesIndex(vLinesIndex);
	m_columnLines.setHLinesIndex(hLinesIndex);
	m_columnLines.setSLinesIndex(sLinesIndex);
	m_columnLines.sortLinesByBindRef(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, compareRefH);
	m_columnLines.sortLinesByBindRef(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, compareRefV);
	hLinesIndex.clear(), vLinesIndex.clear(), sLinesIndex.clear();
	std::map<int, std::vector<Corner>> temp_corners;
	bulidLinesCorner(m_columnLines.lines(), m_columnLines.vLinesIndices(), m_columnLines.hLinesIndices(), m_columnLines.sLinesIndices(), temp_corners);
	m_colunmCorners.setCorners(temp_corners), temp_corners.clear();
	return true;
}

bool PillarSet::findBorderGrop(Data& data)
{
	std::map<int, int>colunm_map;
	for (int i = 0; i < m_columnLines.lines().size(); ++i)
	{
		ColumnsBorderGroup temp_borderGrop;
		std::queue<int>queue_index;
		//此处使用遍历的方式访问是否存在某种类型的线，是为了安全考虑的，因此肯定会降低一下效率
		//若能保证，因引出线与柱边界线的图层信息以同，则可大大提高效率
		auto colunm_lead = m_mapLeadLines.find(i);
		auto colunm_border = colunm_map.find(i);
		if (colunm_lead != m_mapLeadLines.end() ||
			colunm_border != colunm_map.end())
		{
			continue;
		}
		queue_index.push(i);
		colunm_map[i] = i;
		temp_borderGrop.m_borders.push_back(i);
		while (!queue_index.empty())
		{
			int border_index = queue_index.front();
			queue_index.pop();
			auto corners = m_colunmCorners.corners().find(border_index);

			if (corners != m_colunmCorners.corners().end())
			{

				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == border_index)
					{
						l1 = corner.l2;
					}
					auto b_colunm_map = colunm_map.find(l1);
					auto b_lead_map = m_mapLeadLines.find(l1);
					if (b_colunm_map == colunm_map.end() && b_lead_map == m_mapLeadLines.end())
					{
						temp_borderGrop.m_borders.push_back(l1);
						temp_borderGrop.m_bgrop = true;
						queue_index.push(l1);
						colunm_map[l1] = l1;
					}
					else if (b_colunm_map == colunm_map.end() && b_lead_map != m_mapLeadLines.end())
					{
						temp_borderGrop.m_blead = true;
						auto leadIte = returnLeadGropIndex(l1);

						temp_borderGrop.m_leadGropsIndex.push_back(leadIte);
						m_leadGrops[leadIte].m_pair = true;
					}
				}
			}
		}


		int gropIndex = m_BorderGrops.size();
		for (auto it : temp_borderGrop.m_borders)
		{
			m_mapBorderGropIndex[it] = gropIndex;
		}
		m_BorderGrops.push_back(temp_borderGrop);


	}
	return true;
}

//优化边界线不成组的情况
bool PillarSet::optFBorderGrop(std::vector<int>& temp_index)
{
	std::vector<int>opt_groupsIndex;//本图层borderGrop索引
	std::vector<int>fGroupIndex;//本图层线索引
	std::vector<int>opt_fGroupIndex;//被优化字典索引
	//std::vector<int>opt_leadGroupIndex;//被优化的引出线

	for (int i = 0; i < m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].size(); ++i)
	{
		int num = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP][i];
		int now_index = m_BorderGrops[num].m_borders.front();

		Line now_line = m_columnLines.lines()[now_index];
		ColumnsBorderGroup temp_borderGrop;
		bool bFindGropPair = false;
		std::vector<int>temp_gropsIndex;//本图层borderGrop索引
		std::vector<int>temp_colunmIndex;//本图层线索引
		std::vector<int>temp_fGropIndex;//被优化字典索引

		temp_gropsIndex.push_back(num);
		temp_colunmIndex.push_back(now_index);//未成组线的索引
		temp_fGropIndex.push_back(i);//未成组线的组索引

		if (std::find(fGroupIndex.begin(), fGroupIndex.end(), i) != fGroupIndex.end())
			continue;

		if (m_columnLines.lines()[now_index].vertical())
		{
			double left = m_columnLines.lines()[now_index].s.x - m_averageWidth * 2;
			double bottom = m_columnLines.lines()[now_index].s.y - m_averageHeight;
			double reght = m_columnLines.lines()[now_index].e.x + m_averageWidth * 2;
			double top = m_columnLines.lines()[now_index].e.y + m_averageHeight;
			double min[2] = { left,bottom };
			double max[2] = { reght,top };
			int num = m_rTreeColumnLines.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
																			 &m_rTreeColumnLines, std::placeholders::_1));
			for (auto it : m_rTreeColumnLines.SBackIndex())
			{
				temp_index.push_back(it);
				if (it != now_index)
				{
					temp_colunmIndex.push_back(it);
				}
			}
			m_rTreeColumnLines.clearSBackIndex();
		}
		else if (m_columnLines.lines()[now_index].horizontal())
		{
			double left = m_columnLines.lines()[now_index].s.x - m_averageWidth;
			double bottom = m_columnLines.lines()[now_index].s.y - m_averageHeight * 2;
			double reght = m_columnLines.lines()[now_index].e.x + m_averageWidth;
			double top = m_columnLines.lines()[now_index].s.y + m_averageHeight * 2;
			double min[2] = { left,bottom };
			double max[2] = { reght,top };
			int num = m_rTreeColumnLines.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
																			 &m_rTreeColumnLines, std::placeholders::_1));
			for (auto it : m_rTreeColumnLines.SBackIndex())
			{
				temp_index.push_back(it);
				if (it != now_index)
				{
					temp_colunmIndex.push_back(it);
				}
			}
			m_rTreeColumnLines.clearSBackIndex();
		}
		else
		{
			double left = m_columnLines.lines()[now_index].s.x - m_averageWidth;
			double bottom = m_columnLines.lines()[now_index].s.y - m_averageHeight;
			double reght = m_columnLines.lines()[now_index].e.x + m_averageWidth;
			double top = m_columnLines.lines()[now_index].s.y + m_averageHeight;
			double min[2] = { left,bottom };
			double max[2] = { reght,top };
			int num = m_rTreeColumnLines.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
																			 &m_rTreeColumnLines, std::placeholders::_1));
			for (auto it : m_rTreeColumnLines.SBackIndex())
			{
				temp_index.push_back(it);
				if (it != now_index)
				{
					temp_colunmIndex.push_back(it);
				}
			}
			m_rTreeColumnLines.clearSBackIndex();
		}
		if (temp_colunmIndex.size() == 1)
		{
			//最最特殊的情况
			//opt_colunmIndex.push_back(temp_colunmIndex.front());
			auto begin = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].begin();
			auto end = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].end();
			auto ite = std::find(begin, end, temp_colunmIndex.front());
			fGroupIndex.push_back(std::distance(begin, ite));

		}
		else
		{
			int bIndex = -1;//判断所找到的边界线是否为已成组的
			std::vector<int>borderLinesIndex;//先将找到的边界线存在borderLinesIndex里面，避免有已经分组的边界的情况

			for (auto it : temp_colunmIndex)
			{

				auto colunmGropIte = m_mapBorderGropIndex.find(it);//查找边界线所在的组
				if (colunmGropIte != m_mapBorderGropIndex.end())//判断其确实是边界线而不是引出线
				{
					auto colunmGropIndex = m_mapBorderGropIndex[it];
					auto begin = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].begin();
					auto end = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].end();
					auto ite = std::find(begin, end, colunmGropIndex);
					if (ite == end)//边界线不属于未分组的类型
					{

						bIndex = colunmGropIndex;
						bFindGropPair = true;
					}
					else
					{
						/*边界属于未分组类*/
						/*if (bIndex != -1)
						{
							m_BorderGrops[bIndex].m_borders.push_back(it);
						}
						else
						{
							temp_borderGrop.m_borders.push_back(m_mapLineGropIndex[it]);
						}*/
						borderLinesIndex.push_back(it);
						opt_groupsIndex.push_back(colunmGropIndex);
						opt_fGroupIndex.push_back(std::distance(begin, ite));
						fGroupIndex.push_back(std::distance(begin, ite));

					}
				}
				else//区域线为引出线的形式
				{
					auto begin = m_leadGropsType[false].begin();
					auto end = m_leadGropsType[false].end();
					auto leadGropIndex = m_mapLeadGropIndex[it];
					auto ite = std::find(begin, end, leadGropIndex);
					if (ite == end)
					{
						//auto leadGropIndex = m_mapLeadGropIndex[it];
						temp_borderGrop.m_blead = true;
						temp_borderGrop.m_leadGropsIndex.push_back(leadGropIndex);
						m_leadGropsType[false].erase(ite);
					}
				}
			}
			if (bFindGropPair)
			{
				for (auto it : borderLinesIndex)
				{
					temp_borderGrop.m_borders.push_back(it);
				}
			}
			else
			{
				for (auto it : borderLinesIndex)
				{
					temp_borderGrop.m_borders.push_back(it);
				}

			}
		}

		if (!bFindGropPair)
		{
			m_BorderGrops.push_back(temp_borderGrop);
		}

	}

	//更新字典到最新
	auto temp_borderGrops = m_BorderGrops;
	auto temp_fGropIndex = m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP];
	m_BorderGrops.clear();
	m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].clear();


	for (int i = 0; i < temp_borderGrops.size(); ++i)
	{
		if (std::find(opt_groupsIndex.begin(), opt_groupsIndex.end(), i) == opt_groupsIndex.end())
		{
			m_BorderGrops.push_back(temp_borderGrops[i]);
		}
	}
	for (int i = 0; i < temp_fGropIndex.size(); ++i)
	{
		if (std::find(opt_fGroupIndex.begin(), opt_fGroupIndex.end(), i) == opt_fGroupIndex.end())
		{
			m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].push_back(temp_fGropIndex[i]);
		}
	}


	return true;
}

bool PillarSet::optFLTextPoint(std::vector<int>& test_index)
{
	std::vector<int>opt_textPointIndex;//被优化的文本点

	for (auto textPoint : m_textPointType[false])
	{
		double left = m_textPoint[textPoint].m_point.x - m_averageWidth * 2;
		double bottom = m_textPoint[textPoint].m_point.y - m_averageHeight * 2;
		double reght = m_textPoint[textPoint].m_point.x + m_averageWidth * 2;
		double top = m_textPoint[textPoint].m_point.y + m_averageHeight * 2;
		double min[2] = { left,bottom };
		double max[2] = { reght,top };
		int num = m_rTreeColumnLines.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
																		 &m_rTreeColumnLines, std::placeholders::_1));
		for (auto borderLineIndex : m_rTreeColumnLines.SBackIndex())
		{
			if (m_mapLeadLines.find(borderLineIndex) == m_mapLeadLines.end())
			{
				auto borderGroupIndex = m_mapBorderGropIndex[borderLineIndex];
				if (!m_BorderGrops[borderGroupIndex].m_blead ||
					!m_BorderGrops[borderGroupIndex].m_bgrop)
					continue;
				else if (m_BorderGrops[borderGroupIndex].m_btextPoint)
				{
					test_index.push_back(borderGroupIndex);
					auto begin = m_textPointType[false].begin();
					auto end = m_textPointType[false].end();
					auto ite = std::find(m_textPointType[false].begin(), m_textPointType[false].end(), textPoint);
					opt_textPointIndex.push_back(std::distance(begin, ite));
					m_BorderGrops[borderGroupIndex].m_btextPoint = true;
					m_BorderGrops[borderGroupIndex].m_textPoints.push_back(textPoint);

				}
			}
		}
		m_rTreeColumnLines.clearSBackIndex();

	}
	auto fTextPointIndex = m_textPointType[false];
	m_textPointType[false].clear();
	auto begin = opt_textPointIndex.begin();
	auto end = opt_textPointIndex.end();
	for (int i = 0; i < fTextPointIndex.size(); ++i)
	{
		if (std::find(begin, end, i) == end)
		{
			m_textPointType[false].push_back(i);
		}
	}
	return true;
}

int PillarSet::returnLeadGropIndex(const int& leadLineIndex)
{
	for (int i = 0; i < m_leadGrops.size(); ++i)
	{
		auto  LeadGropIte = std::find(m_leadGrops[i].m_nowIndex.begin(), m_leadGrops[i].m_nowIndex.end(), leadLineIndex);
		bool b_isLeadGrop = LeadGropIte == m_leadGrops[i].m_nowIndex.end() ? false : true;
		if (b_isLeadGrop)
		{
			return i;
		}
	}
}

bool PillarSet::initializationColumnType()
{
	for (int i = 0; i < m_BorderGrops.size(); ++i)
	{
		if (!m_BorderGrops[i].m_bgrop)
		{
			m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FGROP].push_back(i);
		}
		/*else if (!m_BorderGrops[i].m_blead)
		{
			m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FLEAD].push_back(i);
		}
		else if (!m_BorderGrops[i].m_btextPoint)
		{
			m_borderGropsType[ColumnsBorderGroup::BorderGroupType::FTEXTPOINT].push_back(i);
		}*/
	}
	for (int i = 0; i < m_leadGrops.size(); ++i)
	{
		if (!m_leadGrops[i].m_pair)
		{
			m_leadGropsType[false].push_back(i);
		}
	}
	for (int i = 0; i < m_textPoint.size(); ++i)
	{
		if (!m_textPoint[i].m_bLeadLine)
		{
			m_textPointType[false].push_back(i);
		}

	}
	return true;
}

bool PillarSet::returnColumnAverageWidthHeight()
{
	int borderGrop_size = m_BorderGrops.size();
	int check_size = borderGrop_size * 0.3;
	std::map<int, int>indexMap;
	double sumWidth = 0.0, sumHeight = 0.0;
	for (int i = 0; i < check_size; ++i)
	{
		Box borderBox;
		int borderGropIndex = -1;
		do
		{
			borderGropIndex = std::rand() % borderGrop_size;
		} while (indexMap.find(borderGropIndex) != indexMap.end() ||
				 !m_BorderGrops[borderGropIndex].m_bgrop);
		indexMap[borderGropIndex] = borderGropIndex;
		for (auto it : m_BorderGrops[borderGropIndex].m_borders)
		{
			borderBox.expand(m_columnLines.lines()[it].s);
			borderBox.expand(m_columnLines.lines()[it].e);
		}

		sumWidth += std::abs(borderBox.right - borderBox.left);
		sumHeight += std::abs(borderBox.top - borderBox.bottom);
	}
	m_averageWidth = sumWidth / check_size;
	m_averageHeight = sumHeight / check_size;

	return true;
}

bool PillarSet::columnInformation(std::vector<std::shared_ptr<Pillar>>& _pillars)
{
	for (auto it : m_BorderGrops)
	{
		std::vector<int>vl;
		std::vector<int>hl;
		std::vector<int>sl;
		std::vector<int>leadL;

		if (it.m_bgrop)
		{
			lineTypeVHS(m_columnLines.lines(), it.m_borders, vl, hl, sl);
			if (it.m_blead)
			{
				
				columnPush(_pillars, m_textPoint[m_leadGrops[it.m_leadGropsIndex.front()].m_textPointIndex].m_name,
						   vl, hl, sl, m_leadGrops[it.m_leadGropsIndex.front()].m_nowIndex, m_columnLines);
			}
			else if (it.m_btextPoint)
			{
				//lineTypeVHS(m_columnLines.lines(), it.m_borders, vl, hl, sl);
				columnPush(_pillars, m_textPoint[it.m_textPoints.front()].m_name,
						   vl, hl, sl, leadL, m_columnLines);
			}
			else
			{
				//lineTypeVHS(m_columnLines.lines(), it.m_borders, vl, hl, sl);
				columnPush(_pillars, "",
						   vl, hl, sl, leadL, m_columnLines);
			}

		}
		//else if (it.m_btextPoint&&it.m_bgrop)
		//{
			//;
		//}
	}

	std::fstream of("angle.txt", std::ios::out);
	for (auto it : _pillars)
	{
		of << it->getName() << "  " << it->getHPDistance().first << "  " << it->getVPDistance().first << "   " << it->getAngle() << std::endl;
	}
	of.close();

	return true;
}

//bool PillarSet::pushRTreeColumnLines(const Line & line,const int &index)
//{
//	double min[2]{};
//	double max[2]{};
//	min[0] = line.s.x < line.e.x ? line.s.x : line.e.x;
//	min[1] = line.s.y < line.e.y ? line.s.y : line.e.y;
//	max[0] = line.s.x > line.e.x ? line.s.x : line.e.x;
//	max[1] = line.s.y > line.e.y ? line.s.y : line.e.y;
//	m_rTreeColumnLines.insertSLines(min, max, index);
//	return true;
//}

bool PillarSet::initializationLeadLineMap()
{
	for (auto leadLines : m_leadGrops)
	{
		for (auto it : leadLines.m_nowIndex)
		{
			m_mapLeadLines[it] = it;
		}
	}
	return true;
}

bool PillarSet::initializationBorderLineMap()
{
	for (auto borderLines : m_BorderGrops)
	{
		for (auto it : borderLines.m_borders)
		{
			m_mapBorderLines[it] = it;
		}
	}
	return true;
}

bool PillarSet::setMember(const std::shared_ptr<Axis>& axises)
{
	m_axises = axises;
	return true;
}

bool PillarSet::columns(Data& data, std::vector<std::shared_ptr<Pillar>>& _pillars/*, std::vector<int>& temp_index,
	std::vector<Line>& temp_line, std::vector<Point>& corner*/)
{
	
	findColumns(data, _pillars/*, temp_index, temp_line, corner*/);
	
	for (int i = 0; i < _pillars.size(); ++i)
	{
		FindPillarNearAxis(*_pillars[i]);
	}
	return true;
}

bool PillarSet::columnPush(std::vector<std::shared_ptr<Pillar>>& pillars,
						   const std::string& str, const std::vector<int>& vl,
						   const std::vector<int>& hl, const std::vector<int>& sl,
						   const std::vector<int>& ls, LineData& lineData)
{
	std::vector<Line> sls;
	std::vector<Line> vls;
	std::vector<Line> hls;
	std::vector<Line> dls;
	for (auto l : sl)
	{
		sls.push_back(lineData.lines()[l]);
	}
	for (auto l : vl)
	{
		vls.push_back(lineData.lines()[l]);
	}
	for (auto l : hl)
	{
		hls.push_back(lineData.lines()[l]);
	}

	for (auto l : ls)
	{
		dls.push_back(lineData.lines()[l]);
	}
	//初始化角度
	std::shared_ptr<Pillar>tempPillar = std::make_shared<Pillar>(str, vls, hls, sls, dls);
	tempPillar->setAngle();
	//压入柱列表
	pillars.push_back(tempPillar);

	

	

	return true;
}
#if 0
bool PillarSet::extendLeadLine(std::vector<Line>& lines, const std::vector<int>& ls, LineData& lineData, CornerData& cornerData)
{
	for (auto idx : ls)
	{

		auto line = lineData.lines()[idx];
		auto corners = cornerData.corners().find(idx);
		if (corners == cornerData.corners().end())
		{
			//left right
			Point p1(line.s.x + (line.s.x - line.e.x) * 0.2, line.s.y + (line.s.y - line.e.y) * 0.2);
			Point p2(line.e.x + (line.e.x - line.s.x) * 0.2, line.e.y + (line.e.y - line.s.y) * 0.2);
			lines.push_back(Line(p1, line.s));
			lines.push_back(Line(p2, line.e));
		}
		else
		{
			//left or right
			bool left = false;
			for (auto corner : corners->second)
			{
				if (corner.distance(line.s) < Precision)
					left = true;
				break;
			}
			if (left)
			{
				Point p1(line.e);
				Point p2(line.e.x + (line.e.x - line.s.x) * 0.2, line.e.y + (line.e.y - line.s.y) * 0.2);
				lines.push_back(Line(p1, p2));
			}
			else
			{
				//p s
				Point p1(line.s);
				Point p2(line.s.x + (line.s.x - line.e.x) * 0.2, line.s.y + (line.s.y - line.e.y) * 0.2);
				lines.push_back(Line(p1, p2));
			}
		}
	}
	return true;
}
#endif

bool PillarSet::SetHaxis(AxisMark& axis_mark_h, const std::string& str, const double& length)
{
	axis_mark_h.first = str;
	axis_mark_h.second = length;
	return true;
}

bool PillarSet::SetVaxis(AxisMark& axis_mark_v, const std::string& str, const double& length)
{
	axis_mark_v.first = str;
	axis_mark_v.second = length;
	return true;
}

bool PillarSet::FindPillarNearAxis(Pillar& pill)
{
	//还是会有一点点小问题，对于不再边界区域外的柱子，临近区会有小问题
	//对于识别错误的柱子直接跳过

	if (pill.getError())
	{
		//需要错误处理
		return true;
	}
	//初始化柱包围盒
	pill.SetPillarBox();

	//轴标记
	std::vector<AxisMark> haxis_mark(2);
	std::vector<AxisMark> vaxis_mark(2);

	for (int i = 0; i < 2; ++i)
	{
		haxis_mark[i].second = -1;
		vaxis_mark[i].second = -1;
	}
	
	{
		//查找较小横轴
		findSmallHColumn(pill, m_axises, haxis_mark);
		//查找较大横轴
		findBigHColumn(pill, m_axises, haxis_mark);
		/**********************/
		//查找较小纵轴
		findSmallVColumn(pill, m_axises, vaxis_mark);
		//查找较大纵轴
		findBigVColumn(pill, m_axises, vaxis_mark);
	}

	/***************/
	if (pill.getHAxis().empty())
	{
		std::vector<AxisMark> _haxis;
		for (auto it : haxis_mark)
		{
			if (it.first != "")
			{
				_haxis.push_back(it);
			}
		}
		pill.setHAxis(_haxis);
	}
	/****************/
	if (pill.getVAxis().empty())
	{
		std::vector<AxisMark> _vaxis;
		for (auto it : vaxis_mark)
		{
			if (it.first != "")
			{
				_vaxis.push_back(it);
			}
		}
		pill.setVAxis(_vaxis);
	}
	//距离信息初始化
	pill.SetHsize();
	pill.SetVsize();

	return true;
}

bool PillarSet::findSmallHColumn(Pillar& pill, const std::shared_ptr<Axis> axise,
								 std::vector<AxisMark>& haxis_mark)
{
	int mid = 0, low = 0, high = axise->hlines_coo.size() - 1;
	while (low <= high)
	{
		if (!pill.getHAxis().empty())
			break;

		mid = (low + high) / 2;
		//if(_axises[i]->hlines_coo[mid]<pill.)
		std::vector<bool> temp_record;
		for (auto it : pill.getHlines())
		{
			double num1 = strimDouble(it.s.y - axise->hlines_coo[0].first.second.s.y);
			double num2 = axise->hlines_coo[mid].second;

			if (num1 > num2)
			{
				temp_record.push_back(true);
			}
			else
			{
				temp_record.push_back(false);
			}
		}
		if (find(temp_record.begin(), temp_record.end(), false) == temp_record.end())
		{
			if (haxis_mark[0].second == -1 || axise->hlines_coo[mid].second > haxis_mark[0].second)
			{
				SetHaxis(haxis_mark[0], axise->hlines_coo[mid].first.first.second, axise->hlines_coo[mid].first.second.s.y);
			}
			//hmark_index[0] = mid;
			low = mid + 1;
		}
		else if (find(temp_record.begin(), temp_record.end(), false) != temp_record.end() &&
				 find(temp_record.begin(), temp_record.end(), true) != temp_record.end())
		{
			pill.setHAxis(AxisMark(axise->hlines_coo[mid].first.first.second,
										  axise->hlines_coo[mid].first.second.s.y));
			break;
		}
		else if (find(temp_record.begin(), temp_record.end(), true) == temp_record.end())
		{
			high = mid - 1;
		}
	}
	return true;
}

bool PillarSet::findBigHColumn(Pillar& pill, const std::shared_ptr<Axis> axise,
							   std::vector<AxisMark>& haxis_mark)
{
	int mid = 0, low = 0, high = axise->hlines_coo.size() - 1;
	while (low <= high)
	{
		if (!pill.getHAxis().empty())
			break;

		mid = (low + high) / 2;

		std::vector<bool> temp_record;
		for (auto it : pill.getHlines())
		{
			double num1 = strimDouble(it.s.y - axise->hlines_coo[0].first.second.s.y);
			double num2 = axise->hlines_coo[mid].second;

			if (num1 < num2)
			{
				temp_record.push_back(true);
			}
			else
			{
				temp_record.push_back(false);
			}
		}
		if (find(temp_record.begin(), temp_record.end(), false) == temp_record.end())
		{
			if (haxis_mark[1].second == -1 || axise->hlines_coo[mid].second < haxis_mark[1].second)
			{
				SetHaxis(haxis_mark[1], axise->hlines_coo[mid].first.first.second, axise->hlines_coo[mid].first.second.s.y);
			}

			high = mid - 1;
		}
		else if (find(temp_record.begin(), temp_record.end(), false) != temp_record.end() &&
				 find(temp_record.begin(), temp_record.end(), true) != temp_record.end())
		{
			pill.setHAxis(AxisMark(axise->hlines_coo[mid].first.first.second,
										  axise->hlines_coo[mid].first.second.s.y));
			break;
		}
		else if (find(temp_record.begin(), temp_record.end(), true) == temp_record.end())
		{
			low = mid + 1;
		}
	}
	return true;
}

bool PillarSet::findSmallVColumn(Pillar& pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark>& vaxis_mark)
{
	int mid = 0, low = 0, high = axise->vlines_coo.size() - 1;
	while (low <= high)
	{
		if (!pill.getVAxis().empty())
			break;

		mid = (low + high) / 2;
		//if(_axises[i]->hlines_coo[mid]<pill.)
		std::vector<bool> temp_record;
		for (auto it : pill.getVlines())
		{
			double num1 = strimDouble(it.s.x - axise->vlines_coo[0].first.second.s.x);
			double num2 = axise->vlines_coo[mid].second;
			if (num1 > num2)
			{
				temp_record.push_back(true);
			}
			else
			{
				temp_record.push_back(false);
			}
		}
		if (find(temp_record.begin(), temp_record.end(), false) == temp_record.end())
		{

			if (vaxis_mark[0].second == -1 || axise->vlines_coo[mid].second > vaxis_mark[0].second)
			{
				SetVaxis(vaxis_mark[0], axise->vlines_coo[mid].first.first.second, axise->vlines_coo[mid].first.second.s.x);
			}

			//vmark_index[0] = mid;
			low = mid + 1;
		}
		else if (find(temp_record.begin(), temp_record.end(), false) != temp_record.end() &&
				 find(temp_record.begin(), temp_record.end(), true) != temp_record.end())
		{
			pill.setVAxis(AxisMark(axise->vlines_coo[mid].first.first.second,
										  axise->vlines_coo[mid].first.second.s.x));
			break;
		}
		else if (find(temp_record.begin(), temp_record.end(), true) == temp_record.end())
		{
			high = mid - 1;
		}
	}
	return true;
}

bool PillarSet::findBigVColumn(Pillar& pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark>& vaxis_mark)
{
	int mid = 0, low = 0, high = axise->vlines_coo.size() - 1;
	while (low + 1 < high)
	{
		if (!pill.getVAxis().empty())
			break;

		mid = (low + high) / 2;
		std::vector<bool> temp_record;
		for (auto it : pill.getVlines())
		{
			double num1 = strimDouble(it.s.x - axise->vlines_coo[0].first.second.s.x);
			double num2 = axise->vlines_coo[mid].second;
			if (num1 < num2)
			{
				temp_record.push_back(true);
			}
			else
			{
				temp_record.push_back(false);
			}
		}
		if (find(temp_record.begin(), temp_record.end(), false) == temp_record.end())
		{
			if (vaxis_mark[1].second == -1 || axise->vlines_coo[mid].second < vaxis_mark[1].second)
			{
				SetVaxis(vaxis_mark[1], axise->vlines_coo[mid].first.first.second, axise->vlines_coo[mid].first.second.s.x);
			}

			high = mid/* - 1*/;
		}
		else if (find(temp_record.begin(), temp_record.end(), false) != temp_record.end() &&
				 find(temp_record.begin(), temp_record.end(), true) != temp_record.end())
		{
			pill.setHAxis(AxisMark(axise->vlines_coo[mid].first.first.second,
										  axise->vlines_coo[mid].first.second.s.x));
			break;
		}
		else if (find(temp_record.begin(), temp_record.end(), true) == temp_record.end())
		{
			low = mid/* + 1*/;
		}
	}
	return true;
}

