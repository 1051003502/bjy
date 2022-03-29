#include "beamSection.h"

bool BeamSection::iniBeamBlock(const Block& _beamBlockVec)
{
	this->beamBlock = _beamBlockVec;
	this->name = _beamBlockVec.name;
	return true;
}

BeamSection BeamSection::retrunBeamSection(Block & block, Data & data)
{
	BeamSection temp_beamSection;
	//!初始化梁断面的box，此处初始化后续会被迁移，目前仅作单图纸的测试用
	temp_beamSection.iniBeamBlock(block);
	//!初始化梁断面的所有纵筋断点
	temp_beamSection.iniBreakPoint(block, data);
	
	//!初始化箍筋信息
	temp_beamSection.iniStriiup(data);
	//!初始化纵筋的引线组
	temp_beamSection.findBpLeadLine(data);
	//!获取标注箍筋信息
	temp_beamSection.iniBeamStirLead(data);
	//!获取钢筋信息，箍筋的，纵筋的
	temp_beamSection.iniRebarInfo(data);
	//!初始化梁断面块内的所有标注,以及断面尺寸
	temp_beamSection.iniBeamDImensions(data, data.m_dimensions);
	temp_beamSection.iniMaxSize();
	//!初始化参照点
	temp_beamSection.iniReferencePoint(data);



	return temp_beamSection;
}


bool BeamSection::iniBreakPoint(Block & block, Data&data)
{
	//!找出所有的纵筋点
	auto block_bok = block.box;
	int hf = block_bok.hf;
	int ht = block_bok.ht;
	int vf = block_bok.vf;
	int vt = block_bok.vt;
	//bool markCircle = false;//用于判断
	//小短横
	//std::fstream of("lineColor.txt", std::ios::out);
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = data.m_lineData.hLinesIndices()[ht1];
		auto &line = data.m_lineData.lines()[idx];
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
			this->bpLayer = line.getLayer();

			BreakPoint bp;
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
	//of << std::endl;
	//小短竖
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = data.m_lineData.vLinesIndices()[vt1];
		auto &line = data.m_lineData.lines()[idx];
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
			this->bpLayer = line.getLayer();

			BreakPoint bp;
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
	//of.close();
	return true;
}

bool BeamSection::findBreakPoint(const Point & p)
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

bool BeamSection::iniBeamDImensions(Data &data, const std::vector<std::shared_ptr<Dimension>>& _spDimensions)
{
	int l_idx = data.m_lineData.vLinesIndices()[beamBlock.box.vf];
	int r_idx = data.m_lineData.vLinesIndices()[beamBlock.box.vt];
	int t_idx = data.m_lineData.hLinesIndices()[beamBlock.box.ht];
	int b_idx = data.m_lineData.hLinesIndices()[beamBlock.box.hf];
	for (auto it : _spDimensions)
	{
		if (it->box.left + Precision > data.m_lineData.lines()[l_idx].s.x &&
			it->box.right - Precision < data.m_lineData.lines()[r_idx].s.x &&
			it->box.bottom + Precision > data.m_lineData.lines()[b_idx].s.y &&
			it->box.top - Precision < data.m_lineData.lines()[t_idx].s.y)
		{
			//初始化尺寸信息
			if (this->seale == -1)
			{
				this->seale = it->scale;
			}
			dimensionsVec.push_back(*it);
		}
	}
	return true;
}

bool BeamSection::iniStirrupLayer(LineData&_lineData)
{
	std::map<std::string, int>candLayerMap;//candidate Layer Map 箍筋的候选图层，用map增加权值
	std::vector<std::string>candLayer;//候选图层
									  //!得到纵筋的包围盒，取出箍筋的图层信息
	auto bpBox = bpSurroundBox(_lineData);
	this->bpBox = bpBox;

	//获取纵筋包围盒范围内的线条索引
	int hf = bpBox.hf;
	int ht = bpBox.ht;
	int vf = bpBox.vf;
	int vt = bpBox.vt;

	//!横线
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto &line = _lineData.lines()[idx];
		if (line.s.x > bpBox.right ||
			line.e.x < bpBox.left ||
			line.length()<std::abs(bpBox.left - bpBox.right)*0.9)
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
		auto &line = _lineData.lines()[idx];
		if (line.s.y > bpBox.top ||
			line.e.y < bpBox.bottom ||
			line.length() < std::abs(bpBox.top - bpBox.bottom)*0.9)
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
			stirrupLayer = it.first;
		}
	}
	return true;
}

Box BeamSection::bpSurroundBox(LineData&_lineData)
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

bool BeamSection::iniStirrupIndex(LineData & _lineData)
{
	std::vector<int>temp_stirrupIndexVec;//!箍筋线的临时索引
										 //!初始化箍筋的图层信息
	iniStirrupLayer(_lineData);

	//!确定梁断面的范围
	int hf = beamBlock.box.hf;
	int ht = beamBlock.box.ht;
	int vf = beamBlock.box.vf;
	int vt = beamBlock.box.vt;

	//!初始化此范围内所有的箍筋线
	//横
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto &line = _lineData.lines()[idx];

		if (line.getLayer() == stirrupLayer)
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
		auto &line = _lineData.lines()[idx];

		if (line.getLayer() == stirrupLayer)
		{
			if (_bpIndexMap.find(idx) == _bpIndexMap.end())
			{
				temp_stirrupIndexVec.push_back(idx);
			}
		}
	}
	//TODO有可能存在斜的梁，后续添加
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
		Box new_box = bpBox;
		double deviation = 3 * break_pointVec.front().radius;
		new_box.left = new_box.left - deviation;
		new_box.right = new_box.right + deviation;
		new_box.bottom = new_box.bottom - deviation;
		new_box.top = new_box.top + deviation;

		for (auto it : temp_stirrupIndexVec)
		{
			Line temp_line = _lineData.getLine(it);
			if (new_box.cover(temp_line.s) && new_box.cover(temp_line.e))
			{
				stirrupIndexVec.push_back(it);
				//stirrupDirMap[it] = Direction::N;

				//!寻找箍筋线的同时初始化由 箍筋线组成的box
				stirBox.expand(temp_line.s);
				stirBox.expand(temp_line.e);
			}
		}
	}


	return true;
}

bool BeamSection::iniBeamHook(LineData &lineData, RTreeData &goalRTree)
{
	std::vector<int>hooks;
	beamBlock;
	double min[2];
	double max[2];
	min[0] = this->bpBox.left;
	min[1] = this->bpBox.bottom;
	max[0] = this->bpBox.right;
	max[1] = this->bpBox.top;
	auto canHooks = returnRtreeIndex(min, max, goalRTree);

	//!筛选箍筋标志
	for (auto it : canHooks)
	{
		if (lineData.getLine(it).getLayer() == stirrupLayer)
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
				beamHookVec.push_back(BeamStirrupHook(hook1, goalIndex));
			}
			
		}

	}

	return false;
}

bool BeamSection::iniStriiup(Data & data)
{
	//!初始化箍筋线的所有索引
	iniStirrupIndex(data.m_lineData);
	//!初始化箍筋标志对
	iniBeamHook(data.m_lineData, data.m_rtreeData);
	//!初始化箍筋信息
	if (beamHookVec.empty())//!存在没有标志勾的情况
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
		//stirDirHookIndexMap;
		//!配对箍筋
		std::vector<std::pair<int, int>>hStirPair;//!保存已经配对好的横向箍筋
		std::vector<std::pair<int, int>>vStirPair;//!保存已经配对好的纵向箍筋

		//!配对箍筋线
		makePairStir(hStirPair, vStirPair, v_lines, h_lines, data);

		//!校准，合并（即当一对标志同时，被横箍或纵箍都使用的情况下进行合并，同时可规避错误）
		//!对箍筋做补全（即，两横缺两竖，或两竖缺两横）
		checkStir(data);
		//!补全单支箍
		findBeamSitrrupSignal(data);

	}
	return true;
}

bool BeamSection::setStirrupDirection(std::vector<int> &_vLines,
	std::vector<int> &_hLines,
	std::vector<int> &_sLines,
	Data&_data)
{
	if (beamHookVec.empty())
	{
		;//!此情况不应该出现
		return true;
	}
	int countHook = -1;
	for (auto hook : beamHookVec)
	{
		++countHook;//形成beamHookVec索引
					//将hook写成数组的形式，便于统一化编码用同一套规则即可，减少代码的冗余
		if (hook.hook_index1 == -1 || hook.hook_index2 == -1)
		{
			;//此情况不因该出现，报错
			continue;
		}

		std::map<int, Direction>checkStirDir;//!check stirrup direction
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
				if (std::find(stirrupIndexVec.begin(), stirrupIndexVec.end(), l1) != stirrupIndexVec.end())
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
				;//
			}
			//为标志勾相交线中最合适的附上方向
			if (markVHSVec.size() > 1 || markVHSVec.front() == 'S')
			{
				;//Todo 报错
			}
			else if (markVHSVec.front() == 'H')
			{
				Line temep_hookLine = _data.m_lineData.getLine(it);
				std::pair<int, Direction> temp_dir(-1, Direction::N);
				double cornerIPLength = 0.0;//用来记录箍筋标志与箍筋间的相对距离
				for (auto stirIndex : cornerIPVec)
				{
					double cornerIPLength1 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).s);
					double cornerIPLength2 = TowPointsLength(stirIndex.second, _data.m_lineData.getLine(stirIndex.first).e);
					double minLength = cornerIPLength1 < cornerIPLength2 ? cornerIPLength1 : cornerIPLength2;
					if (temp_dir.first == -1 )
					{
						cornerIPLength = minLength;

						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.y > otherPoint.y)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::D;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::U;
						}
					}
					else if (stirrupDirMap.find(stirIndex.first) != stirrupDirMap.end() && 
						cornerIPLength > minLength)
					{
						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.y > otherPoint.y)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::D;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::U;
						}
					}


				}
				checkStirDir[temp_dir.first] = temp_dir.second;
				temp_stirIndex.push_back(temp_dir.first);
			}
			else if (markVHSVec.front() == 'V')
			{
				Line temep_hookLine = _data.m_lineData.getLine(it);
				std::pair<int, Direction> temp_dir(-1, Direction::N);
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
							temp_dir.second = Direction::L;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::R;
						}
					}
					else if (stirrupDirMap.find(stirIndex.first)!=stirrupDirMap.end()&&
						cornerIPLength > minLength)
					{
						double distance1 = TowPointsLength(temep_hookLine.s, stirIndex.second);
						double distance2 = TowPointsLength(temep_hookLine.e, stirIndex.second);

						Point goalPoint = distance1 < distance2 ? temep_hookLine.s : temep_hookLine.e;
						Point otherPoint = distance1 < distance2 ? temep_hookLine.e : temep_hookLine.s;
						if (goalPoint.x > otherPoint.x)
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::L;
						}
						else
						{
							temp_dir.first = stirIndex.first;
							temp_dir.second = Direction::R;
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
			if (checkStirDir[temp_stirIndex[0]] == Direction::D ||
				checkStirDir[temp_stirIndex[0]] == Direction::U)
			{
				hIndex = temp_stirIndex[0];
			}
			else
			{
				vIndex = temp_stirIndex[0];

			}
			if (checkStirDir[temp_stirIndex[1]] == Direction::L ||
				checkStirDir[temp_stirIndex[1]] == Direction::R)
			{
				vIndex = temp_stirIndex[1];

			}
			else
			{
				hIndex = temp_stirIndex[1];

			}
			if (hIndex != -1 && vIndex != -1)
			{
				if (checkStirDir[hIndex] == Direction::D &&
					_data.m_lineData.getLine(vIndex).e.y + Precision >
					_data.m_lineData.getLine(hIndex).s.y)
				{
					stirrupDirMap[vIndex] = checkStirDir[vIndex];
					stirDirHookIndexMap[vIndex] = countHook;
				}
				else if (checkStirDir[hIndex] == Direction::U &&
					_data.m_lineData.getLine(vIndex).s.y - Precision <
					_data.m_lineData.getLine(hIndex).s.y)
				{
					stirrupDirMap[vIndex] = checkStirDir[vIndex];
					stirDirHookIndexMap[vIndex] = countHook;
				}
				if (checkStirDir[vIndex] == Direction::L &&
					_data.m_lineData.getLine(hIndex).e.x - Precision <
					_data.m_lineData.getLine(vIndex).s.x)
				{
					stirrupDirMap[hIndex] = checkStirDir[hIndex];
					stirDirHookIndexMap[hIndex] = countHook;
				}
				else if (checkStirDir[vIndex] == Direction::R &&
					_data.m_lineData.getLine(hIndex).s.x + Precision >
					_data.m_lineData.getLine(vIndex).s.x)
				{
					stirrupDirMap[hIndex] = checkStirDir[hIndex];
					stirDirHookIndexMap[hIndex] = countHook;
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

bool BeamSection::divideStirrupDir(const std::vector<int> &lineIndex,
	std::map<int, Direction> &goalIndex,
	const char & ch)
{
	if (ch == 'H')
	{
		for (auto it : lineIndex)
		{
			if (stirrupDirMap[it] != Direction::N)
			{
				goalIndex[it] = stirrupDirMap[it];
			}
		}
	}
	else if (ch == 'V')
	{
		for (auto it : lineIndex)
		{
			if (stirrupDirMap[it] != Direction::N)
			{
				goalIndex[it] = stirrupDirMap[it];
			}
		}
	}
	else if (ch == 'S')
	{
		;//暂不处理
	}

	return true;
}

bool BeamSection::divideStirLineType(std::vector<int> &v_lines, std::vector<int> &h_lines, std::vector<int> &ss_lines, Data & data)
{
	lineTypeVHS(data, stirrupIndexVec, v_lines, h_lines, ss_lines);//为候选箍筋做类型划分
	LinesSort(v_lines, 'V', data.m_lineData);
	LinesSort(h_lines, 'H', data.m_lineData);
	MergeLines(v_lines, 'V', data.m_lineData);
	MergeLines(h_lines, 'H', data.m_lineData);

	//!初始化_bStirPair	stirrupDirMap
	for (auto it : v_lines)
	{
		_bStirPair[it] = false;
		stirrupDirMap[it] = Direction::N;
	}
	for (auto it : h_lines)
	{
		_bStirPair[it] = false;
		stirrupDirMap[it] = Direction::N;
	}
	for (auto it : ss_lines)
	{
		_bStirPair[it] = false;
		stirrupDirMap[it] = Direction::N;
	}

	//为候选箍筋进行排序
	//!此处目前只处理箍筋线是横线与竖线的形式
	LinesSort(v_lines, 'V', data.m_lineData);//排序竖线
	LinesSort(h_lines, 'H', data.m_lineData);//排序横线
	return true;
}

bool BeamSection::makePairStir(std::vector<std::pair<int, int>>& hStirPair,
	std::vector<std::pair<int, int>> &vStirPair,
	const std::vector<int>& v_lines,
	const std::vector<int>& h_lines,
	Data & data)
{

	//!配对箍筋
	std::map<int, Direction>v_dir;
	std::map<int, Direction>h_dir;
	divideStirrupDir(v_lines, v_dir, 'V');
	divideStirrupDir(h_lines, h_dir, 'H');

	//!配对横向箍筋
	std::stack<std::pair<int, Direction>>h_stack;//!横向栈
	for (auto hStirIndex : h_lines)
	{
		//栈为空压栈
		if (h_stack.empty())
		{
			if (h_dir.find(h_lines.front()) == h_dir.end())
			{
				h_stack.push(std::pair<int, Direction>(h_lines.front(), Direction::N));
			}
			else
			{
				h_stack.push(std::pair<int, Direction>(h_lines.front(), h_dir[h_lines.front()]));
			}
		}
		else
		{
			std::pair<int, Direction> temp_stir = h_stack.top();
			if (temp_stir.second == Direction::N)
			{
				if (h_dir.find(hStirIndex) == h_dir.end())
				{
					h_stack.push(std::pair<int, Direction>(hStirIndex, Direction::N));
				}
				else if (h_dir[hStirIndex] == Direction::D)
				{
					//std::stack<std::pair<int, Direction>>temp_stack = h_stack;
					while (!h_stack.empty())
					{
						std::pair<int, Direction>temp_stirDir = h_stack.top();
						if (temp_stirDir.second != Direction::N)
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
								auto hoolIndexIte = stirDirHookIndexMap.find(hStirIndex);
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
				else if (h_dir[hStirIndex] == Direction::U)
				{
					h_stack.push(std::pair<int, Direction>(hStirIndex, Direction::U));
				}
			}
			else if (temp_stir.second == Direction::U)
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


						auto hookIndexIte = stirDirHookIndexMap.find(temp_stir.first);
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
					if (h_dir[hStirIndex] == Direction::U)
					{
						h_stack.push(std::pair<int, Direction>(hStirIndex, Direction::U));
					}
				}
			}

		}
	}

	//!纵向匹配箍筋
	std::stack<std::pair<int, Direction>>v_stack;//!纵向栈
	for (auto vStirIndex : v_lines)
	{
		//栈为空压栈
		if (v_stack.empty())
		{
			if (v_dir.find(v_lines.front()) == v_dir.end())
			{
				v_stack.push(std::pair<int, Direction>(v_lines.front(), Direction::N));
			}
			else
			{
				v_stack.push(std::pair<int, Direction>(v_lines.front(), v_dir[v_lines.front()]));
			}
		}
		else
		{
			std::pair<int, Direction> temp_stir = v_stack.top();
			if (temp_stir.second == Direction::N)
			{
				if (v_dir.find(vStirIndex) == v_dir.end())
				{
					v_stack.push(std::pair<int, Direction>(vStirIndex, Direction::N));
				}
				else if (v_dir[vStirIndex] == Direction::L)
				{
					//std::stack<std::pair<int, Direction>>temp_stack = h_stack;
					while (!v_stack.empty())
					{
						std::pair<int, Direction>temp_stirDir = v_stack.top();
						if (temp_stirDir.second != Direction::N)
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
								auto hookIndexIte = stirDirHookIndexMap.find(vStirIndex);
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
				else if (v_dir[vStirIndex] == Direction::R)
				{
					v_stack.push(std::pair<int, Direction>(vStirIndex, Direction::U));
				}
			}
			else if (temp_stir.second == Direction::R)
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
						auto hoolIndexIte = stirDirHookIndexMap.find(temp_stir.first);
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
					if (v_dir[vStirIndex] == Direction::R)
					{
						v_stack.push(std::pair<int, Direction>(vStirIndex, Direction::R));
					}
				}
			}

		}
	}

	return true;
}

bool BeamSection::checkStir(Data&data)
{
	for (auto it : _hookStirrupIndexMap)
	{
		BeamSStirrup temp_stir;
		temp_stir.hook = beamHookVec[it.first];
		//!只有对边的情况
		if (it.second.size() == 2)
		{
			temp_stir = checkStirPair1(it.second, data.m_lineData);
			//!压入箍筋
			beamStirrupVec.push_back(temp_stir);
		}
		//!两个hook都找到对边的情况，一般出现在箍筋的最外圈
		else if (it.second.size() > 2)
		{
			temp_stir = checkStirPair2(it.second, data);
			beamStirrupVec.push_back(temp_stir);
		}
		//!hook 未找到箍筋
		else
		{
			;//查错
		}
	}
	return true;
}

BeamSStirrup BeamSection::checkStirPair1(std::vector<int> stirDirIndex, LineData & _lineData)
{
	BeamSStirrup temp_stir;
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

BeamSStirrup BeamSection::checkStirPair2(std::vector<int> stirDirIndex, Data & data)
{
	BeamSStirrup temp_stir;
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
		else if (!t&&b)
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
		else if (!r&&l)
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

bool BeamSection::findBeamSitrrupSignal(Data & data)
{
	for (auto it : stirrupIndexVec)
	{
		BeamSStirrup temp_stir;
		//用纵筋的半径做对比，排除单支箍的例外情况
		double length = break_pointVec.front().radius;
		if (!_bStirPair[it])
		{
			//!正常的箍筋线至少会大于纵筋半径的10倍
			if (data.m_lineData.getLine(it).length()<10 * length)
				continue;
			temp_stir.circle_line.push_back(data.m_lineData.getLine(it));
			temp_stir.single = true;

			//!初始化单支箍索引
			singleStirIndex.push_back(it);
			beamStirrupVec.push_back(temp_stir);
		}
		else
		{
			//!初始化双支箍索引
			doubleStirIndex.push_back(it);
		}
		
	}
	return true;
}

bool BeamSection::findSameStir(const Line & _line, int & lineIndex, LineData & _lineData)
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
			if (temp_line.s.y + Precision>_line.s.y&&
				temp_line.e.y - Precision<_line.e.y&&
				temp_line.s.x + Precision > _line.s.x&&
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
		if (i == j&&i == -1)
			return false;
		for (int low = i; low <= j; ++low)
		{
			int h_index = _lineData.hLinesIndices()[low];
			Line temp_line = _lineData.getLine(h_index);
			if (temp_line.s.x + Precision>_line.s.x&&
				temp_line.e.x - Precision<_line.e.x&&
				temp_line.s.y + Precision > _line.s.y&&
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

bool BeamSection::findBpLeadLine(Data & data)
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
			if (data.m_lineData.getLine(e.index).getLayer() == bpLayer ||
				data.m_lineData.getLine(e.index).getLayer() == stirrupLayer ||
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

					if ((point.x + Precision > data.m_lineData.lines()[l1].s.x &&
						point.x - Precision < data.m_lineData.lines()[l1].s.x) ||
						(point.y + Precision > data.m_lineData.lines()[l1].s.y &&
							point.y - Precision < data.m_lineData.lines()[l1].s.y))
					{
						itea = find(leadLineVec.begin(), leadLineVec.end(), l1);
						if (itea == leadLineVec.end())
						{
							BeamBPInfo temp_beamBpInfo;
							leadLineVec.push_back(l1);
							std::vector<int> temp;
							mark = false;
							temp.push_back(l1);
							temp.push_back(mark_num);

							temp_beamBpInfo.bpLeadVec = temp;
							//!添加与因出现相关的纵筋截面
							temp_beamBpInfo.bpVec.push_back(bpCount);
							beamBPinfoVec.push_back(temp_beamBpInfo);
						}
						else
						{
							for (auto it1 = 0; it1 < beamBPinfoVec.size(); ++it1)
							{
								auto itea = find(beamBPinfoVec[it1].bpLeadVec.begin(), beamBPinfoVec[it1].bpLeadVec.end(), l1);
								if (itea != beamBPinfoVec[it1].bpLeadVec.end())
								{
									mark = false;
									beamBPinfoVec[it1].bpLeadVec.push_back(mark_num);
									beamBPinfoVec[it1].bpVec.push_back(bpCount);
								}
							}
						}
					}
				}
				//!处理单独一条引出线的情况
				if (mark_num != -1 && mark == true)
				{
					BeamBPInfo temp_beamBpInfo;
					temp_beamBpInfo.bpLeadVec.push_back(mark_num);
					temp_beamBpInfo.bpVec.push_back(bpCount);
					beamBPinfoVec.push_back(temp_beamBpInfo);
					
				}
			}
		}

	}
	bpLeadLineVec = leadLineVec;
	return true;
}

bool BeamSection::iniBeamStirLead(Data & data)
{
	auto stirLead = divideStirLead(data.m_lineData);

	//testLineIndex = stirLead;
	//!分类箍筋的引出线
	BeamStirLeadInfo stirLeadType;
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
							if (bConnectTowLine(v_line,now_line)&&
								v_line.getLayer()==now_line.getLayer())
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
							if (bConnectTowLine(h_line, now_line)&&
								h_line.getLayer()==now_line.getLayer())
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
					leadMap.find(l1) == leadMap.end()&&
					bPointBelongLineEnd(data.m_lineData.getLine(l1),corner)&&
					bPointBelongLineEnd(data.m_lineData.getLine(index),corner))
				{
					leadType.push_back(l1);
					_queue.push(l1);
					leadMap[l1] = l1;
				}
			}


		}
		stirLeadType.stirLeadVec = leadType;
		//!初始化引出线的箍筋的引出线
		beamStirInfo.push_back(stirLeadType);
	}

	return true;
}

std::vector<int> BeamSection::divideStirLead(LineData & _lineData)
{
	std::vector<int>stirLineIndex;
	auto bpBox = bpSurroundBox(_lineData);
	this->bpBox = bpBox;

	//获取纵筋包围盒范围内的线条索引
	int hf = bpBox.hf;
	int ht = bpBox.ht;
	int vf = bpBox.vf;
	int vt = bpBox.vt;

	//!横线
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = _lineData.hLinesIndices()[ht1];
		auto &line = _lineData.lines()[idx];
		if (line.s.x > bpBox.right ||
			line.e.x < bpBox.left)
		{
			continue;
		}
		else
		{
			if (line.getLayer() != bpLayer&&
				line.getLayer() != stirrupLayer&&
				std::find(bpLeadLineVec.begin(), bpLeadLineVec.end(), idx) == bpLeadLineVec.end())
			{
				stirLineIndex.push_back(idx);
			}
		}
	}
	//!竖线
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = _lineData.vLinesIndices()[vt1];
		auto &line = _lineData.lines()[idx];
		if (line.s.y > bpBox.top ||
			line.e.y < bpBox.bottom)
		{
			continue;
		}
		else
		{
			if (line.getLayer() != bpLayer&&
				line.getLayer() != stirrupLayer&&
				std::find(bpLeadLineVec.begin(), bpLeadLineVec.end(), idx) == bpLeadLineVec.end())
			{
				stirLineIndex.push_back(idx);
			}
		}
	}

	return stirLineIndex;
}

bool BeamSection::iniBeamLeadInfo(std::vector<int> & textPointIndex,
	Point & refPoint,
	const std::vector<int>& _lineIndex,
	const bool & _bpLead, Data & data)
{
	std::vector<int>texts1;//根据个数搜索文本信息
	std::vector<int>texts2;//根据范围搜索文本信息
	std::vector<int>texts;//候选文本信息
						  //根据个数搜索
						  //Point temp_point;//!查找文本的搜索点
	double searchR = 0.0;
	auto mark = findBeamLeadInfoPoint(refPoint, searchR, _lineIndex, _bpLead, data);

	texts = data.m_kdtTreeData.kdtTexts().radiusSearch(refPoint, searchR);


	for (auto it1 : texts)
	{

		auto point = data.m_textPointData.textpoints()[it1];
		auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
		if (_bpLead&&tt->text.find("@") != std::string::npos)
			continue;
		textPointIndex.push_back(it1);
	}

	return true;
}

bool BeamSection::findBeamLeadInfoPoint(Point & point, double &searchR, const std::vector<int>& _lineIndex, const bool &_bpBox, Data & data)
{
	//int goal_index;
	//Point temp_point;
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
			if (!stirBox.cover(l1) && !stirBox.cover(l2))
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
							searchR = minLength*Radius / 2;
							point = Point((l1.x + temp_point.x) / 2, l1.y);
						}
						else
						{
							searchR = minLength*Radius / 2;
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
							searchR = minLength*Radius / 2;
							point = Point(l2.x, (l2.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength*Radius / 2;
							point = Point((l2.x + temp_point.x) / 2, l2.y);
						}
						//searchR = minLength*1.2 / 2;
					}
					return false;
				}
			}
			else if (!_bpBox)//只针对箍筋的引出线
			{//引出线一边在箍筋内部，一边在古今外部
				if ((!stirBox.cover(l1) && stirBox.cover(l2)) || (stirBox.cover(l1) && !stirBox.cover(l2)))
				{

					if (!stirBox.cover(l1))
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
							searchR = minLength*Radius / 2;
							point = Point(l1.x, (l1.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength*Radius / 2;
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
							searchR = minLength*Radius / 2;
							point = Point(l2.x, (l2.y + temp_point.y) / 2);
						}
						else
						{
							searchR = minLength*Radius / 2;
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
			if ((!stirBox.cover(l1) && stirBox.cover(l2)) || (stirBox.cover(l1) && !stirBox.cover(l2)))
			{

				if (!stirBox.cover(l1))
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
						searchR = minLength*Radius / 2;
						point = Point(l1.x, (l1.y + temp_point.y) / 2);
					}
					else
					{
						searchR = minLength*Radius / 2;
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
						searchR = minLength*Radius / 2;
						point = Point(l2.x, (l2.y + temp_point.y) / 2);
					}
					else
					{
						searchR = minLength*Radius / 2;
						point = Point((l2.x + temp_point.x) / 2, l2.y);
					}
					//searchR = minLength*1.2 / 2;
				}
			}
			return false;
		}
	}
}

bool BeamSection::iniRebarInfo(Data & data)
{
	//字符串信息字典，用于校验，第一个表示文本点索引，第二个string 表示，纵筋还是箍筋 分别用“Z” “G”表示，Point 表示文本的参照点,int 表示所在纵筋或箍筋信息的索引
	std::map<int, std::pair<std::string, std::pair<Point, int>>>strInfoMap;
	//初始化梁断面内所有的纵筋信息
	int bpInfoCount = 0;
	for (auto it = beamBPinfoVec.begin(); it != beamBPinfoVec.end(); ++it)
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
				if (ch == 'N')
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
						auto strIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
							beamBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
							tt->text);
						beamBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
						auto indexIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
							beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
							pIndex);
						beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

					}
					else if (strInfoIte.first == "G")
					{
						beamStirInfo[strInfoIte.second.second].stirInfo.clear();
						beamStirInfo[strInfoIte.second.second].textPoint = Point();
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
	//初始化梁断面内所有的箍筋信息
	int stirInfoCount = 0;
	for (auto it = beamStirInfo.begin(); it != beamStirInfo.end(); ++it)
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
							auto strIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
								beamBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
								tt->text);
							beamBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
							auto indexIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
								beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
								pIndex);
							beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

						}
						else if (strInfoIte.first == "G")
						{
							beamStirInfo[strInfoIte.second.second].stirInfo.clear();
							beamStirInfo[strInfoIte.second.second].textPoint = Point();
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
						auto strIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoVec.begin(),
							beamBPinfoVec[strInfoIte.second.second].bpInfoVec.end(),
							tt->text);
						beamBPinfoVec[strInfoIte.second.second].bpInfoVec.erase(strIte);
						auto indexIte = std::find(beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.begin(),
							beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.end(),
							temp_textPIndex.front());
						beamBPinfoVec[strInfoIte.second.second].bpInfoPointIndex.erase(indexIte);

					}
					else if (strInfoIte.first == "G")
					{
						beamStirInfo[strInfoIte.second.second].stirInfo.clear();
						beamStirInfo[strInfoIte.second.second].textPoint = Point();
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

	//!初始化箍筋描述信息所描述的是单支箍还是双支箍
	setStirInfoType(data);
	//!初始化纵筋信息的方向
	setBPLeadInfoDirection(data.m_lineData);
	return true;
}

bool BeamSection::returnLeadLineDir(char & ch, const bool &_bpBox, const std::vector<int> _leadLineIndex, Data & data)
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
			if (!stirBox.cover(l1) && !stirBox.cover(l2))
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
				if ((!stirBox.cover(l1) && stirBox.cover(l2)) || (stirBox.cover(l1) && !stirBox.cover(l2)))
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
			if ((!stirBox.cover(l1) && stirBox.cover(l2)) || (stirBox.cover(l1) && !stirBox.cover(l2)))
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

bool BeamSection::iniMaxSize()
{
	beamBlock;
	Box candidateBox = stirBox;
	for (auto dim : dimensionsVec)
	{
		if (dim.direction == 90 || dim.direction == 270)
		{
			if (dim.box.left<stirBox.left&&dim.box.right>stirBox.right)
			{
				maxWidth = dim.measurement;
			}
			//!后备选择，用来描述截面的宽
			if (dim.box.left < stirBox.left)
			{
				candidateBox.left = dim.box.left;
			}
			if (dim.box.right > stirBox.right)
			{
				candidateBox.right = dim.box.right;
			}
		}
		else if (dim.direction == 0 || dim.direction == 180)
		{
			if (dim.box.bottom<stirBox.bottom&&dim.box.top>stirBox.top)
			{
				maxHeight = dim.measurement;
			}
			//!后备选择，用来描述截面的高
			if (dim.box.bottom < stirBox.bottom)
			{
				candidateBox.bottom = dim.box.bottom;
			}
			if (dim.box.top > stirBox.top)
			{
				candidateBox.top = dim.box.top;
			}
		}
	}

	if (maxHeight == 0.0)
	{
		maxHeight = std::abs(candidateBox.top - candidateBox.bottom) * seale;
	}
	if (maxWidth == 0.0)
	{
		maxWidth = std::abs(candidateBox.right - candidateBox.left) * seale;
	}

	return true;
}

bool BeamSection::iniReferencePoint(Data&_data)
{
	//!确定梁断面的范围
	int hf = beamBlock.box.hf;
	int ht = beamBlock.box.ht;
	int vf = beamBlock.box.vf;
	int vt = beamBlock.box.vt;
	std::vector<int>hLineIndex;
	std::vector<int>vLineIndex;
	std::vector<int>sLineIndex;
	//!为箍筋线分类
	lineTypeVHS(_data, stirrupIndexVec, vLineIndex, hLineIndex, sLineIndex);
	//!为箍筋线排序
	LinesSort(hLineIndex, 'H', _data.m_lineData);
	LinesSort(vLineIndex, 'V', _data.m_lineData);

	//!测试
	int left = 0;
	int bottom = 0;

	Point temp_point(-1, -1);
	//横
	for (int ht1 = hf; ht1 < ht; ++ht1)
	{
		int idx = _data.m_lineData.hLinesIndices()[ht1];
		auto &line = _data.m_lineData.lines()[idx];

		if (temp_point.y == -1)
		{
			bottom = idx;
			temp_point.y = line.s.y;
		}
		else if (((line.e.x>stirBox.left&&line.s.x<stirBox.left) ||
			(line.s.x<stirBox.right&&line.e.x>stirBox.right)) &&
			line.s.y + Precision < _data.m_lineData.getLine(hLineIndex.front()).s.y&&
			temp_point.y < line.s.y)
		{
			bottom = idx;
			temp_point.y = line.s.y;
		}
	}

	//竖
	for (int vt1 = vf; vt1 < vt; ++vt1)
	{
		int idx = _data.m_lineData.vLinesIndices()[vt1];
		auto &line = _data.m_lineData.lines()[idx];

		if (temp_point.x == -1)
		{
			left = idx;
			temp_point.x = line.s.x;
		}
		else if (((line.s.y<stirBox.bottom&&line.e.y>stirBox.bottom) ||
			(line.s.y<stirBox.top&&line.e.y>stirBox.top)) &&
			line.s.x + Precision < _data.m_lineData.getLine(vLineIndex.front()).s.x&&
			temp_point.x < line.s.x)
		{
			left = idx;
			temp_point.x = line.s.x;
		}
	}
	referencePoint = temp_point;
	this->testLeft = left;
	this->testBottom = bottom;
	return true;
}

bool BeamSection::setStirInfoType(Data& data)
{
	for (auto i = 0; i < beamStirInfo.size(); ++i)//!多个引出线标注
	{
		bool singleStir = false, doubleStir = false;
		for (auto leadIndex : beamStirInfo[i].stirLeadVec)//!每个箍筋的标注可能会有条因出现
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
					if (!singleStir && std::find(singleStirIndex.begin(), singleStirIndex.end(), l1) != singleStirIndex.end())
					{
						singleStir = true;
					}
					else if (!doubleStir && std::find(doubleStirIndex.begin(), doubleStirIndex.end(), l1) != doubleStirIndex.end())
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
			beamStirInfo[i].bSingle = true;
		}
		else
		{
			beamStirInfo[i].bSingle = false;
		}
	}
	return true;
}

bool BeamSection::setBPLeadInfoDirection(LineData&_lineData)
{
	/*此处以纵筋断点，在梁断面的区域位置判断纵筋描述的是属于那部分钢筋，
			主要思想是，有纵筋断点构成的box在，箍筋box的上侧，下侧，还是左侧，右侧,用其与箍筋边界相交的权值进行比较
			*/
	for (auto bpInfo=beamBPinfoVec.begin();bpInfo!= beamBPinfoVec.end();++bpInfo)
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
					if (crossLineVH(line, Line(Point(stirBox.left, stirBox.top), 
						Point(stirBox.right, stirBox.top)),tempCorner))
					{
						t++;
					}
					else if (crossLineVH(line, Line(Point(stirBox.left, stirBox.bottom),
						Point(stirBox.right, stirBox.bottom)), tempCorner))
					{
						b++;
					}
				}
				else if (line.horizontal())
				{
					if (crossLineVH(Line(Point(stirBox.left, stirBox.bottom),
						Point(stirBox.left, stirBox.top)), line,tempCorner))
					{
						l++;
					}
					else if (crossLineVH(Line(Point(stirBox.right, stirBox.bottom),
						Point(stirBox.right, stirBox.top)), line, tempCorner))
					{
						r++;
					}
				}
				else
				{
					if(crossLineS(line, Line(Point(stirBox.left, stirBox.top),
						Point(stirBox.right, stirBox.top)), tempCorner))
					{
						t++;
					}
					else if (crossLineS(line, Line(Point(stirBox.left, stirBox.bottom),
						Point(stirBox.right, stirBox.bottom)),tempCorner))
					{
						b++;
					}
					else if (crossLineS(line, Line(Point(stirBox.left, stirBox.bottom),
						Point(stirBox.left, stirBox.top)),tempCorner))
					{
						l++;
					}
					else if (crossLineS(line, Line(Point(stirBox.right, stirBox.bottom),
						Point(stirBox.right, stirBox.top)),tempCorner))
					{
						r++;
					}
				}

			}
			if (l > r&& l > t&& l > b)
			{
				bpInfo->dir = BeamBPInfo::Direction::LEFT;
			}
			else if (r > l&& r > t&& r > b)
			{
				bpInfo->dir = BeamBPInfo::Direction::RIGHT;
			}
			else if (t > r&& t > l&& t > b)
			{
				bpInfo->dir = BeamBPInfo::Direction::TOP;
			}
			else if (b > t&& b > l&& b > r)
			{
				bpInfo->dir = BeamBPInfo::Direction::BOTTOM;
			}
		}
	}
	return true;
}

