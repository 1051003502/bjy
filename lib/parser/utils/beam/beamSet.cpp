#include "beamSet.h"
#include "drw_entities.h"

//
BeamTextPoint::BeamTextPoint(const Point& point, const std::string& name, const bool& blead)
{
	this->pt = point;
	this->strText = name;
	this->bLeadLine = blead;
}

bool BeamSet::setMember(const std::shared_ptr<Axis>& axises,
						const std::vector<std::shared_ptr<Pillar>>& pillars)
{
	spAxis = axises;
	spPillarVec = pillars;
	iniColumnRtree();
	return true;
}

bool BeamSet::findBeam(Data& data/*,
					   std::vector<int>& mark_index,
					   std::vector<Point>& corner,
					   std::vector<Line>& temp_line,
					   std::vector<std::vector<int>>& m_temp_index*/)
{
	std::srand(std::time(nullptr));

	/*临时变量调试索引*/
	std::vector<int> goal_index;

	//初步查找梁图层线
	findBeamLines(data);
	//查找梁图层内的引出线以及以及所有相关联线
	findLeadLinesAndTextPoint(data);
	//初始化梁图层信息
	initializationBeamLinesCorners();

	/*初始化beamData数据*/
	//初始化梁公共数据
	setBeamData();

	//初步将边界线分组
	retrunBorderAverageWidth(goal_index);//为分类边界线做初步处理求成对边界线间近似间距 有些边界间距很大需要依此剔除
	findBeamBorderGrops();
	//剔除第一次误识别的引出线
	removeExcessLeadInBorderGrop(borderGroupVec, beamLines, avgWidth, beamCornerData, avgWidth, leadGroupVec);
	//添加带有断点引出线的配对机制 边界成对 引出线是断点形式
	byConditionExtractBorderGropLeadGrop(borderGroupVec, false, true, leadGroupVec, true, false,
										 condidateBorderIdxVec, condidateLeadIdxVec);
	if (!condidateLeadIdxVec.empty())
	{
		borderLeadPair1(beamData, avgWidth, leadGroupVec, borderGroupVec,
						condidateBorderIdxVec, condidateLeadIdxVec);
	}

	//边界成对无引出线的
	condidateBorderIdxVec.clear(), condidateLeadIdxVec.clear();
	byConditionExtractBorderGropTextPoints(borderGroupVec, false, true, beamTextPointVec, false,
										   condidateBorderIdxVec, condidateBeamTextPointIdxVec);
	if (!condidateBeamTextPointIdxVec.empty())
	{
		borderPointPair(beamData, avgWidth, beamTextPointVec, borderGroupVec,
						condidateBorderIdxVec, condidateBeamTextPointIdxVec);
	}

#if 0
	//根据边界组合与leadLine组合方式显示

	for (int i = 0; i < borderGroupVec.size(); ++i)
	{
#if 1
		if (borderGroupVec[i].bGroup && !borderGroupVec[i].bLead)
		{
			std::vector<int> temp_index;
			for (auto it1 : borderGroupVec[i].borderVec)
			{
				mark_index.push_back(primevalIndexVec[it1]);
				temp_index.push_back(primevalIndexVec[it1]);
			}
			for (auto it1 : borderGroupVec[i].leadGropsIndexVec)
			{
				for (auto it2 : leadGroupVec[it1.first].nowIndexVec)
				{
					mark_index.push_back(primevalIndexVec[it2]);
					temp_index.push_back(primevalIndexVec[it2]);
				}
			}
			m_temp_index.push_back(temp_index);
		}
#endif

#if 0
		/*else*/ if (borderGroupVec[i].bGroup /*&& borderGroupVec[i].bBeamTextPoint*/)
		{
			std::vector<int> temp_index;
			for (auto it1 : borderGroupVec[i].borderVec)
			{
				mark_index.push_back(primevalIndexVec[it1]);
				temp_index.push_back(primevalIndexVec[it1]);
			}
			//m_temp_index.push_back(temp_index);
		}
#endif

#if 0
		else
		{
			for (auto it1 : borderGroupVec[i].borderVec)
			{
				mark_index.push_back(primevalIndexVec[it1]);

			}
		}
#endif
	}
#endif


	//setMFCPaintTestVec(mark_index, temp_line);

#if 0
	//显示引出线
#if 1
	for (int i = 0; i < leadGroupVec.size(); ++i)
	{
		//if (m_leadGrops[i].m_pair)
		{
			for (auto it2 : leadGroupVec[i].nowIndexVec)
			{
				mark_index.push_back(primevalIndexVec[it2]);
			}
		}
	}
#endif

	//显示border成组却无引出线的情况
	for (int i = 0; i < borderGroupVec.size(); ++i)
	{
		//if (it.m_btextPoint)
		{
			for (auto it1 : borderGroupVec[i].borderVec)
			{
				mark_index.push_back(primevalIndexVec[it1]);
			}
		}
	}
#endif



	/*for (auto it : m_primevalIndex)
	{
	mark_index.push_back(it);
	}*/


	//显示无引出线的文本点
	//for (auto it : beamTextPointVec)
	//{
	//	if (!it.bLeadLine)
	//	{
	//		corner.push_back(it.pt);
	//	}
	//	//corner.push_back(this->m_textPoints[it].m_point);
	//}
	return true;
}



bool BeamSet::beams(std::vector<std::shared_ptr<Beam>>& _beamVec, Data& data)
{
	// 遍历连通的边界组
	for (auto& borderGrpItem : this->borderGroupVec)
	{
		std::vector<std::shared_ptr<Beam>> beamVec;

		//解析每一个贯通边界组成的梁
		//有成组的边界线，有引线或文本点([borders and lead]|[borders and text])
		if (!borderGrpItem.bGroup || (!borderGrpItem.bLead && !borderGrpItem.bBeamTextPoint))
			continue;


		//判断连通边线的方向
		Beam::Direction d;
		this->checkDirectionLines(d, borderGrpItem.borderVec);

		//对边界排序，保证柱链的顺序
		std::sort(borderGrpItem.borderVec.begin(), borderGrpItem.borderVec.end(), [this, &d](int idx1, int idx2)
		{
			const auto& l1 = this->beamLines.lines().at(idx1);
			const auto& l2 = this->beamLines.lines().at(idx2);
			if (d == Beam::Direction::V)
			{
				if (l1.e.y < l2.e.y)
					return true;
				else return false;
			}
			else
			{
				if (l1.e.x < l2.e.x)
					return true;
				else return false;
			}
		});

		//打印边界
		std::for_each(borderGrpItem.borderVec.begin(), borderGrpItem.borderVec.end(),
					  std::bind([this](int idx)
		{
			logLine(beamLines.lines().at(idx));
		}, std::placeholders::_1));

		//解析有引线的边界组
		if (borderGrpItem.bLead)
		{
			auto& leadIdxVec = borderGrpItem.leadGropsIndexVec;
			//边界组有多根引线
			if (leadIdxVec.size() > 1)
			{
				//边界组有多条引线,则对引线按照d方向上排序
				std::sort(leadIdxVec.begin(), leadIdxVec.end(), [this, &d](std::pair<int, bool> p1, std::pair<int, bool> p2)
				{
					const auto& l1 = this->beamLines.lines().at(this->leadGroupVec.at(p1.first).cornerIndex.front().second);
					const auto& l2 = this->beamLines.lines().at(this->leadGroupVec.at(p2.first).cornerIndex.front().second);
					Point midPt1((l1.s.x + l1.e.x) / 2, (l1.s.y + l2.e.y) / 2);
					Point midPt2((l2.s.x + l2.e.x) / 2, (l2.s.y + l2.e.y) / 2);
					if (d == Beam::Direction::V)
					{
						if (midPt1.y < midPt2.y)
							return true;
						else
							return false;
					}
					else
					{
						if (midPt1.x < midPt2.x)
							return true;
						else
							return false;
					}
				});
				//遍历引线提取梁
				for (auto it = 0; it < leadIdxVec.size(); it++)
				{

					std::shared_ptr<Beam> beam = std::make_shared<Beam>();
					beam->getUUID(getRandom16Chars());
					const auto idx = leadIdxVec.at(it).first;
					parseBeamInfo(beam, data, idx);

					const auto& lead = leadGroupVec.at(beam->leadIdx);
					//得到引线中与边界线相交的那根线的索引
					int crossLeadLineIdx = lead.cornerIndex.front().second;
					Line leadCrossLine = beamLines.lines().at(crossLeadLineIdx);

					spdlog::get("all")->critical(
						"lead cross line start pt({},{}),end pt({},{})",
						std::to_string(leadCrossLine.s.x), std::to_string(leadCrossLine.s.y),
						std::to_string(leadCrossLine.e.x), std::to_string(leadCrossLine.e.y));


					Axis::AxisLine axisLine;

					//分析该交线是在哪个位置,用于确定边界
					if (d == Beam::Direction::V)
					{
						findSmallHAxis(Point((leadCrossLine.s.x + leadCrossLine.e.x) / 2,
											 (leadCrossLine.s.y + leadCrossLine.e.y) / 2), *this->spAxis, axisLine);
						beam->beamRegion.push_back(axisLine);
						spdlog::get("all")->info("axis scope is from {}",
												 axisLine.first.second);
					}
					else
					{
						findSmallVAxis(Point((leadCrossLine.s.x + leadCrossLine.e.x) / 2,
											 (leadCrossLine.s.y + leadCrossLine.e.y) / 2), *this->spAxis, axisLine);
						beam->beamRegion.push_back(axisLine);
						spdlog::get("all")->info("axis scope is from {}",
												 axisLine.first.second);
					}


					//第一根引线
					if (it == 0)
					{
						if (d == Beam::Direction::H || d == Beam::Direction::S)
						{
							//在确定的轴区间确定上一条梁的边线
							double endAxisX = axisLine.second.s.x;

							for (auto borderIdx : borderGrpItem.borderVec)
							{
								auto line = beamLines.getLine(borderIdx);
								//横线，且有部分或全部长度在轴之间
								if (line.horizontal() &&
									!(line.s.x >= endAxisX && line.e.x > endAxisX))
								{

									if (line.e.x > endAxisX)
										line.e.x = endAxisX;
									//加入柱的添加
									updateColChain(beam, line);
									beam->borderLineVec.push_back(line);
									spdlog::get("all")->info("line start pt({},{});end pt({},{})",
															 std::to_string(line.s.x), std::to_string(line.s.y),
															 std::to_string(line.e.x), std::to_string(line.e.y));
								}
								//横向连通梁不需要垂直线
								else if (line.vertical())
								{
								}
								//斜线 
								else
								{
									//临时取值，后期利用柱的宽度进行调整
									double offsetColumn = 550/*beam->refHeight * 2*/;
									// 确保斜线再两个轴之间
									if (line.e.x <= endAxisX + offsetColumn)
									{
										updateColChain(beam, line);
										beam->borderLineVec.push_back(line);
										spdlog::get("all")->info("line start pt({},{});end pt({},{})",
																 std::to_string(line.s.x), std::to_string(line.s.y),
																 std::to_string(line.e.x), std::to_string(line.e.y));
									}
								}
							}
						}
						//竖直梁
						else if (d == Beam::Direction::V)
						{
							spdlog::get("all")->error("this vertical beam is not handled");
						}
						beamVec.push_back(beam);
						continue;
					}

					//横梁确定边线
					if (d == Beam::Direction::H || d == Beam::Direction::S)
					{
						//在确定的轴区间确定上一条梁的边线
						double startAxisX = beamVec.back()->beamRegion.back().second.s.x;
						double endAxisX = axisLine.second.s.x;

						for (auto borderIdx : borderGrpItem.borderVec)
						{
							auto line = beamLines.getLine(borderIdx);
							//横线，且有部分或全部长度在轴之间
							if (line.horizontal() &&
								!(line.s.x < startAxisX && line.e.x <= startAxisX) &&
								!(line.s.x >= endAxisX && line.e.x > endAxisX))
							{
								//修正长度
								if (line.s.x < startAxisX)
									line.s.x = startAxisX;
								if (line.e.x > endAxisX)
									line.e.x = endAxisX;
								updateColChain(beamVec.back(), line);
								beamVec.back()->borderLineVec.push_back(line);
								spdlog::get("all")->info("line start pt({},{});end pt({},{})",
														 std::to_string(line.s.x), std::to_string(line.s.y),
														 std::to_string(line.e.x), std::to_string(line.e.y));
							}
							//横向连通梁不需要垂直线
							else if (line.vertical())
							{
							}
							//斜线 
							else
							{
								// 临时取值，后期利用柱的宽度进行调整
								double offsetColumn = 550;
								// 确保斜线再两个轴之间
								if (line.s.x >= startAxisX - offsetColumn && line.e.x <= endAxisX + offsetColumn)
								{
									updateColChain(beamVec.back(), line);
									beamVec.back()->borderLineVec.push_back(line);
									spdlog::get("all")->info("line start pt({},{});end pt({},{})",
															 std::to_string(line.s.x), std::to_string(line.s.y),
															 std::to_string(line.e.x), std::to_string(line.e.y));
								}
							}
						}
					}
					// 竖直梁
					else if (d == Beam::Direction::V)
					{
						spdlog::get("all")->error("this vertical beam is not handled");
					}

					parseSpan(beamVec.back(), data);
					spdlog::get("all")->info("\n");

					// 最后一根引线
					if (it == leadIdxVec.size() - 1)
					{
						//确定最后一根梁的边界
						if (d == Beam::Direction::H || d == Beam::Direction::S)
						{
							//在确定的轴区间确定上一条梁的边线
							double startAxisX = beam->beamRegion.back().second.s.x;
							for (auto borderIdx : borderGrpItem.borderVec)
							{
								auto line = beamLines.getLine(borderIdx);
								//横线，且有部分或全部长度在轴之间
								if (line.horizontal() &&
									!(line.s.x < startAxisX && line.e.x <= startAxisX))
								{
									//修正长度
									if (line.s.x < startAxisX)
										line.s.x = startAxisX;
									updateColChain(beam, line);
									beam->borderLineVec.push_back(line);
									spdlog::get("all")->info("line start pt({},{});end pt({},{})",
															 std::to_string(line.s.x), std::to_string(line.s.y),
															 std::to_string(line.e.x), std::to_string(line.e.y));
								}
								//横向连通梁不需要垂直线
								else if (line.vertical())
								{
								}
								//斜线 
								else
								{
									//临时取值，后期利用柱的宽度进行调整
									double offsetColumn = 550;
									// 确保斜线再两个轴之间
									if (line.s.x >= startAxisX - offsetColumn)
									{
										updateColChain(beam, line);
										beam->borderLineVec.push_back(line);
										spdlog::get("all")->info("line start pt({},{});end pt({},{})",
																 std::to_string(line.s.x), std::to_string(line.s.y),
																 std::to_string(line.e.x), std::to_string(line.e.y));
									}
								}
							}
						}
						//竖直梁
						else if (d == Beam::Direction::V)
						{
							spdlog::get("all")->error("this vertical beam is not handled");
						}
						beamVec.back()->beamRegion.push_back(axisLine);
						parseSpan(beam, data);
						beamVec.push_back(beam);
						continue;
					}
					beamVec.back()->beamRegion.push_back(axisLine);
					beamVec.push_back(beam);
				}
			}
			//边界组只有一根引线
#if 1
			else
				beamVec.push_back(parseOneLeadBeamBorderGroup(data, borderGrpItem, d));
#endif
		}
		//解析无引线的边界组
#if 1
		else if (!borderGrpItem.bLead)
			beamVec.push_back(parseNoLeadBeamBorderGroup(data, borderGrpItem, d));
#endif
		_beamVec.insert(_beamVec.end(), beamVec.begin(), beamVec.end());
	}
	return true;
}

std::shared_ptr<Beam> BeamSet::parseOneLeadBeamBorderGroup(Data& data,
														   BorderGroup& borderGrpItem,
														   Beam::Direction d)
{
	double min[2]{ -1,-1 }, max[2]{ -1,-1 };
	std::shared_ptr<Beam> beam = std::make_shared<Beam>();
	auto str = getRandom16Chars();
	beam->getUUID(str);
	const auto& leadIdxVec = borderGrpItem.leadGropsIndexVec;
	parseBeamInfo(beam, data, leadIdxVec.front().first);
	//根据连通的方向性，压入边界即可
	for (auto borderIdx : borderGrpItem.borderVec)
	{
		auto line = beamLines.lines().at(borderIdx);
		//横向
		if (d == Beam::Direction::H && !line.vertical())
		{
			beam->borderLineVec.push_back(line);
		}
		//纵向
		else if (d == Beam::Direction::V && !line.horizontal())
		{
			beam->borderLineVec.push_back(line);
		}
		//斜向
		else
		{
			beam->borderLineVec.push_back(line);
		}
		updateColChain(beam, line);
	}
	parseSpan(beam, data);
	return beam;
}

std::shared_ptr<Beam> BeamSet::parseNoLeadBeamBorderGroup(Data& data, BorderGroup& borderGrpItem,
														  Beam::Direction d)
{
	std::shared_ptr<Beam> beam = std::make_shared<Beam>();
	auto str = getRandom16Chars();
	beam->getUUID(str);
	spdlog::get("all")->info("no lead,only beam name text");
	std::string beamText = beamTextPointVec[borderGrpItem.beamTextPointVec.front()].strText;
	spdlog::get("all")->info("text:{}", beamText);
	beam->extractBeamInfo(beamText);
	for (auto borderIdx : borderGrpItem.borderVec)
	{
		const auto& line = beamLines.lines().at(borderIdx);
		updateColChain(beam, line);
		beam->borderLineVec.push_back(line);
		spdlog::get("all")->info(
			"line start pt({},{});end pt({},{})",
			std::to_string(line.s.x), std::to_string(line.s.y),
			std::to_string(line.e.x), std::to_string(line.e.y));
	}

	if (d == Beam::Direction::V)
		spdlog::get("all")->info("type:vertical beam\n");
	else if (d == Beam::Direction::H)
		spdlog::get("all")->info("type:horizontal beam\n");
	else
		spdlog::get("all")->info("type:slash beam\n");

	//no lead
	parseSpan(beam, data);
	return beam;
}

bool BeamSet::findBeamLayer(const std::vector<std::pair<int,int>>& _columnHBorderIndex, LineData& lineData)
{
	std::map<std::string, int>borderLayerMap;
	for (auto hBorders : _columnHBorderIndex)
	{
		//上边界
		const Line& tempLineT = lineData.getLine(hBorders.first);
		const double tmepSizeT = tempLineT.length() * 0.25;
		auto tempIndexT = findLineAdjacentIndex(lineData, tempLineT, tmepSizeT, 'H', 'T');
		//下边界
		const Line& tempLineB = lineData.getLine(hBorders.second);
		const double tmepSizeB = tempLineB.length() * 0.25;
		auto tempIndexB = findLineAdjacentIndex(lineData, tempLineB, tmepSizeB, 'H', 'B');
		
		if (tempIndexT != -1)
		{
			borderLayerMap[lineData.getLine(tempIndexT).getLayer()]++;
		}
		if (tempIndexB != -1)
		{
			borderLayerMap[lineData.getLine(tempIndexB).getLayer()]++;
		}
	}

	int count = 0;
	for (auto it : borderLayerMap)
	{
		if (beamLayer.empty())
		{
			beamLayer = it.first;
			count = it.second;
		}
		else if (count < it.second)
		{
			beamLayer = it.first;
			count = it.second;
		}
	}
#if 1
	std::fstream of("beamLayer.txt", std::ios::out);

	of << beamLayer << std::endl;
	of << "***************" << std::endl;
	for (auto it : borderLayerMap)
	{
		of << it.first <<"   "<<it.second<<std::endl;
	}

	of.close();
#endif
	return true;
}

//所有梁图层的线
bool BeamSet::findBeamLines(Data& data)
{
	//std::fstream of("laryer.txt", std::ios::out);
	for (int i = 0; i < data.m_lineData.lines().size(); ++i)
	{
		std::string str = data.m_lineData.lines()[i].getLayer();

		//std::string ch1 = GbkToUtf8("梁");
		////std::string ch2 = "梁";
		//std::string ch3 = u8"BEAM";

		//if (str.find(ch1) != std::string::npos ||
		//	//str.find(ch2) != std::string::npos ||
		//	str.find(ch3) != std::string::npos)

		if(beamLayer == str)
		{
			if (std::find(strBorderLayerVec.begin(),
						  strBorderLayerVec.end(), str) ==
				strBorderLayerVec.end())
			{
				strBorderLayerVec.push_back(str);
			}
			//borderLine.push_back(data.m_lineData.lines()[i]);
			beamLines.pushBackLines(data.m_lineData.lines()[i]);
			primevalIndexVec.push_back(i);
			pushRTreeLines(data.m_lineData.lines()[i], primevalIndexVec.size() - 1, borderLineRTree);
		}
		//of << str << std::endl;
	}
	//of.close();
	return true;

}

//引出线集合，以及textPoint，另外将leadlines也要当作breamlines做处理
bool BeamSet::findLeadLinesAndTextPoint(Data& data)
{
	//std::vector<Point> candidates;
	BeamSet::findTextCandidatesPoints(data.m_textPointData);
	for (int i = 0; i < beamTextPointVec.size(); ++i)
	{
		int goal_index = -1;
		std::vector<int>dlines;
		std::shared_ptr<DRW_Text>tt;

		Point textPoint = beamTextPointVec[i].pt;
		tt = std::static_pointer_cast<DRW_Text>(textPoint.entity);
		auto ends = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, textPoint), 3);
		findLeadLineIndex(goal_index, ends, i, data);
		if (goal_index != -1 && goal_index != -2)
		{

			beamTextPointVec[i].bLeadLine = true;
			Lead temp_leadGrop;
			temp_leadGrop.beamTextPointIndex = i;


			temp_leadGrop.bPair = false;
			std::vector<int>lead_corner;
			//判断是否存在插入点
			temp_leadGrop.bSide = findRelatedLineIndex(goal_index, data, dlines, lead_corner);
			//存在插入点初始化InsertPoint
			if (temp_leadGrop.bSide)
			{
				addLeadGroupInsertPoint(temp_leadGrop, data, dlines);
			}

			//temp_leadGrop.m_cornerIndex.first = goal_index;
			for (auto it : dlines)
			{
				auto ite = std::find(primevalIndexVec.begin(), primevalIndexVec.end(), it);
				if (ite == primevalIndexVec.end())
				{

					int border_size = beamLines.lines().size();
					if (std::find(lead_corner.begin(), lead_corner.end(), it) != lead_corner.end())
					{
						std::pair<int, int>cornerLineIndex;
						cornerLineIndex.first = it;
						cornerLineIndex.second = border_size;
						temp_leadGrop.cornerIndex.push_back(cornerLineIndex);

					}

					temp_leadGrop.primevalIndexVec.push_back(it);
					temp_leadGrop.nowIndexVec.push_back(border_size);
					beamLines.pushBackLines(data.m_lineData.lines()[it]);
					primevalIndexVec.push_back(it);
					pushRTreeLines(data.m_lineData.lines()[it], border_size, borderLineRTree);

				}
				else
				{
					int num = std::distance(primevalIndexVec.begin(), ite);
					if (std::find(lead_corner.begin(), lead_corner.end(), it) != lead_corner.end())
					{
						std::pair<int, int>cornerLineIndex;
						cornerLineIndex.first = it;
						cornerLineIndex.second = num;
						temp_leadGrop.cornerIndex.push_back(cornerLineIndex);
					}
					temp_leadGrop.primevalIndexVec.push_back(it);
					temp_leadGrop.nowIndexVec.push_back(num);
				}

			}
			leadGroupVec.push_back(temp_leadGrop);
			//leadType[LeadType::P].push_back(leadLines.size() - 1);
		}
		else
		{
			if (goal_index != -2)
				beamTextPointVec[i].bLeadLine = false;
		}
	}

	return true;
}

bool BeamSet::findTextCandidatesPoints(TextPointData& textPointData)
{
	double left = spAxis->box.left;
	double right = spAxis->box.right;
	int i = textPointData.findLowerBound(spAxis->box.bottom);
	int j = textPointData.findUpperBound(spAxis->box.top);

	//通过标注符号查找候选集
	for (i; i <= j && j != 0; ++i)
	{
		auto& point = textPointData.textpoints()[i];
		if (point.x > left - Precision && point.x < right + Precision)
		{
			auto tt = std::static_pointer_cast<DRW_Text>(point.entity);
			if (tt->text.find("L") != std::string::npos)
			{

				beamTextPointVec.push_back(BeamTextPoint(point, tt->text));
				//assert(!(tt->text.find("ZL2") != std::string::npos));
			}
		}
	}
	return true;
}

bool BeamSet::findLeadLineIndex(int& goal_index, const std::vector<int>& point_index, const int& textPointIndex, Data& data)
{
	for (auto end : point_index)
	{
		auto e = data.m_endpointData.getEndpoint(end);
		auto corners = data.m_cornerData.corners().find(e.index);
		if (corners == data.m_cornerData.corners().end())
			continue;

		auto& line = data.m_lineData.lines()[e.index];

		bool b_connect_s = false;
		bool b_connect_e = false;

		if (line.vertical())
		{
			int i = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, line.s.x - 1, Precision, false);
			int j = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, line.s.x + 1, Precision, false);
			for (int low = i; low <= j; ++low)
			{
				int index = data.m_lineData.vLinesIndices()[low];
				Line temp_line = data.m_lineData.lines()[index];
				if (temp_line.e.y + Precision < line.s.y ||
					temp_line.s.y - Precision > line.e.y)
				{
					continue;
				}
				else
				{
					if (ConnectPoint(temp_line.e, line.s))
					{
						b_connect_s = true;
					}
					else if (ConnectPoint(temp_line.s, line.e))
					{
						b_connect_e = true;
					}
				}

			}
		}
		else if (line.horizontal())
		{
			int i = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, line.s.y - 1);
			int j = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, line.s.y + 1);
			for (int low = i; low <= j; ++low)
			{
				int index = data.m_lineData.hLinesIndices()[low];
				Line temp_line = data.m_lineData.lines()[index];
				if (temp_line.e.x + Precision < line.s.x ||
					temp_line.s.x - Precision > line.e.x)
				{
					continue;
				}
				else
				{
					if (ConnectPoint(temp_line.e, line.s))
					{
						b_connect_s = true;
					}
					else if (ConnectPoint(temp_line.s, line.e))
					{
						b_connect_e = true;
					}
				}
			}
		}
		else
		{
			;//斜线重复的概率很小候面补齐
		}
		/*************/
		for (auto corner : corners->second)
		{
			if (b_connect_s && b_connect_e)
				break;
			if (ConnectPoint(corner, line.s))
				b_connect_s = true;
			if (ConnectPoint(corner, line.e))
				b_connect_e = true;
		}
		//!判断所找引出线是否符合引出线的特性
		if ((b_connect_s && b_connect_e) || line.isSide() ||
			(std::find(strBorderLayerVec.begin(), strBorderLayerVec.end(),
					   line.getLayer()) != strBorderLayerVec.end()) ||
			line.getLayer() == "AXIS")
			continue;
		//!判断引出线是否已经存在，若存在辨别出真正的对应关系
		int leadLineIndex = 0;

		auto bLead = bLeadLineAllLines(leadGroupVec, e.index, leadLineIndex);
		if (!bLead)
		{
			goal_index = e.index;
		}
		else
		{
			//!取出原始引出线组所对应的文本点
			auto beforeTextPoint = beamTextPointVec[leadGroupVec[leadLineIndex].beamTextPointIndex].pt;
			double beforeDistance = TowPointsLength(beforeTextPoint, e);
			double nowDistance = TowPointsLength(beamTextPointVec[textPointIndex].pt, e);
			if (nowDistance < beforeDistance)
			{
				//!因为当前的文本点离此引出线更近，因此应此引线组为当前文本的对应配对
				beamTextPointVec[textPointIndex].bLeadLine = true;
				//!还原从前文本所处的状态
				beamTextPointVec[leadGroupVec[leadLineIndex].beamTextPointIndex].bLeadLine = false;
				leadGroupVec[leadLineIndex].beamTextPointIndex = textPointIndex;
				goal_index = -2;//!返回一个特殊值，用作此情况的标记
			}
			else
			{
				continue;
			}
		}


		break;
	}
	return true;
}

//判断引出线是否存在插入点
bool BeamSet::findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, std::vector<int>& leadCorner)
{
#if 1
	auto& line = data.m_lineData.lines()[relateLine];
	const auto& layer = line.getLayer();
	dline.push_back(relateLine);

	bool b_side = false;
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
					if (data.m_lineData.lines()[l1].isSide())
					{
						b_side = true;
						//引出线的相交线

						leadCorner.push_back(index);
					}
				}
			}
		}
		//平行处理
		if (line_queue_front.horizontal())
		{
			double yb = line_queue_front.s.y - 25.0;
			double yu = line_queue_front.s.y + 25.0;
			int i = data.m_lineData.findLowerBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
			int j = data.m_lineData.findUpperBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
			if (i == j && i == -1)
				return false;
			for (int low = i; low <= j; ++low)
			{
				int h_index = data.m_lineData.hLinesIndices()[low];
				Line temp_line = data.m_lineData.lines()[h_index];
				if (!temp_line.isSide() || temp_line.e.x<line_queue_front.s.x || temp_line.s.x>line_queue_front.e.x)
					continue;
				else if (std::find(dline.begin(), dline.end(), h_index) == dline.end() &&
						 temp_line.getLayer() == layer)
				{
					b_side = true;
					dline.push_back(h_index);
					lead_queue.push(h_index);
					//引出线相交线
					leadCorner.push_back(index);


				}
			}
		}
		else if (line_queue_front.vertical())
		{
			double xb = line_queue_front.s.x - 25.0;
			double xu = line_queue_front.s.x + 25.0;
			int i = data.m_lineData.findLowerBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xb, Precision, false);
			int j = data.m_lineData.findUpperBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xu, Precision, false);
			if (i == j && i == -1)
				return false;
			for (int low = i; low <= j; ++low)
			{
				int v_index = data.m_lineData.vLinesIndices()[low];
				Line temp_line = data.m_lineData.lines()[v_index];
				if (!temp_line.isSide() || temp_line.e.y<line_queue_front.s.y || temp_line.s.y>line_queue_front.e.y)
					continue;
				else if (std::find(dline.begin(), dline.end(), v_index) == dline.end() &&
						 temp_line.getLayer() == layer)
				{
					b_side = true;
					dline.push_back(v_index);
					lead_queue.push(v_index);
					//引出线相交线
					leadCorner.push_back(index);
				}

			}
		}
	}

	if (b_side == true)
	{
		return true;
	}
	else
	{
		//无断点的引出线判断
		if (dline.size() == 1)
		{
			leadCorner.push_back(relateLine);
		}
		else
		{
			int goal_size = -1;
			int goal_index = -1;
			for (auto it : dline)
			{
				int size = data.m_cornerData.corners().find(it)->second.size();
				if (goal_size == -1)
				{
					goal_size = size;
					goal_index = it;
				}
				else if (goal_size < size)
				{
					goal_size = size;
					goal_index = it;
				}
			}
			leadCorner.push_back(goal_index);
		}
		return false;
	}
#elif 0
	auto& line = data.m_lineData.lines()[relateLine];
	const auto& layer = line.getLayer();
	dline.push_back(relateLine);
	auto corners = data.m_cornerData.corners().find(relateLine);
	bool b_side = false;
	//相交处理
	if (corners != data.m_cornerData.corners().end())
	{
		for (auto corner : corners->second)
		{
			auto l1 = corner.l1;
			if (l1 == relateLine)
			{
				l1 = corner.l2;
			}
			if (data.m_lineData.lines()[l1].getLayer() == layer/* && !data.m_lineData.lines()[l1].isSide()*/)
			{
				dline.push_back(l1);
				if (data.m_lineData.lines()[l1].isSide())
				{
					b_side = true;
				}
			}
		}
	}
	//平行处理
	if (line.horizontal())
	{
		double yb = line.s.y - 25.0;
		double yu = line.s.y + 25.0;
		int i = data.m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = data.m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		if (i == j && i == -1)
			return false;
		for (int low = i; low <= j; ++low)
		{
			int index = data.m_lineData.hLinesIndices()[low];
			Line temp_line = data.m_lineData.lines()[index];
			if (!temp_line.isSide() || temp_line.e.x<line.s.x || temp_line.s.x>line.e.x)
				continue;
			else
			{
				b_side = true;
				dline.push_back(index);

			}
		}
	}
	else if (line.vertical())
	{
		double xb = line.s.x - 25.0;
		double xu = line.s.x + 25.0;
		int i = data.m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xb, Precision, false);
		int j = data.m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xu, Precision, false);
		if (i == j && i == -1)
			return false;
		for (int low = i; low <= j; ++low)
		{
			int index = data.m_lineData.vLinesIndices()[low];
			Line temp_line = data.m_lineData.lines()[index];
			if (!temp_line.isSide() || temp_line.e.y<line.s.y || temp_line.s.y>line.e.y)
				continue;
			else
			{
				b_side = true;
				dline.push_back(index);
			}

		}
	}


	if (b_side == true)
	{
		return true;
	}
	else
	{
		return false;
	}
#endif 

}

bool BeamSet::addLeadGroupInsertPoint(Lead& leadGrop, Data& data, std::vector<int>& dlines)
{
	std::vector<Point>mid_point;
	double width = 0.0;
	for (auto it : dlines)
	{
		Line temp_line = data.m_lineData.lines()[it];
		if (temp_line.isSide())
		{
			width = temp_line.length() / 2.0;
			Point temp_point = Point((temp_line.s.x + temp_line.e.x) / 2, (temp_line.s.y + temp_line.e.y) / 2);
			bool b_mark = false;
			for (auto point : mid_point)
			{
				if (ConnectPoint(point, temp_point))
				{
					b_mark = true;
					break;
				}
			}
			if (!b_mark)
			{
				mid_point.push_back(temp_point);
			}
		}
	}
	leadGrop.insertPointVec = mid_point;
	leadGrop.polyWidth = width;
	return true;
}

bool BeamSet::initializationBeamLinesCorners()
{
	std::vector<int>hLinesIndex;
	std::vector<int>vLinesIndex;
	std::vector<int>sLinesIndex;
	lineTypeVHSIndex(beamLines.lines(), vLinesIndex, hLinesIndex, sLinesIndex);
	beamLines.setVLinesIndex(vLinesIndex);
	beamLines.setHLinesIndex(hLinesIndex);
	beamLines.setSLinesIndex(sLinesIndex);
	beamLines.sortLinesByBindRef(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, compareRefH);
	beamLines.sortLinesByBindRef(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, compareRefV);
	hLinesIndex.clear(), vLinesIndex.clear(), sLinesIndex.clear();
	std::map<int, std::vector<Corner>> temp_corners;
	bulidLinesCorner(beamLines.lines(), beamLines.vLinesIndices(), beamLines.hLinesIndices(), beamLines.sLinesIndices(), temp_corners);
	beamCornerData.setCorners(temp_corners), temp_corners.clear();
	return true;
}

bool BeamSet::setBeamData()
{
	beamData.m_lineData = beamLines;
	beamData.m_cornerData = beamCornerData;

	return true;
}

bool BeamSet::retrunBorderAverageWidth(std::vector<int>& goal_index)
{

	/************/
	//有个漏洞，不能所有线都是引出线，不可以出现这样的情况否则会陷入死循环//
	/****************/
	double temp_averageWidth = 0.0;
	std::vector<int>borderMap;

	if (beamLines.lines().size() < 20)
	{

		temp_averageWidth = borderWidthAverage(3, borderMap, goal_index);
		//averageWidth = sum_num / 2.0;
	}
	else if (beamLines.lines().size() < 40 && beamLines.lines().size() >= 20)
	{

		temp_averageWidth = borderWidthAverage(5, borderMap, goal_index);
		//averageWidth = sum_num / 5.0;

	}
	else
	{
		temp_averageWidth = borderWidthAverage(15, borderMap, goal_index);
		//averageWidth = sum_num / 8.0;

	}
	avgWidth = temp_averageWidth;
	//return averageWidth;
	return true;
}

double BeamSet::borderWidthAverage(const int& conut, std::vector<int>& borderMap, std::vector<int>& goal_index)
{
	double sum_num = 0.0;
	std::vector<std::pair<int, int>>width_border;
	std::vector<double>width;
	std::vector<double>sort_width;
	std::vector<std::pair<int, int>>_pair;

	for (int i = 0; i < conut; ++i)
	{
		double num = 0.0;
		std::pair<int, int> temp_pair;
		do
		{
			findPairedBorder(temp_pair);

		} while (std::find(borderMap.begin(), borderMap.end(), temp_pair.first) != borderMap.end() &&
				 std::find(borderMap.begin(), borderMap.end(), temp_pair.second) != borderMap.end() && temp_pair.first == temp_pair.second);

		_pair.push_back(temp_pair);

		if (beamLines.lines()[temp_pair.first].vertical())
		{
			num = abs(beamLines.lines()[temp_pair.first].s.x - beamLines.lines()[temp_pair.second].s.x);
		}
		else if (beamLines.lines()[temp_pair.first].horizontal())
		{
			num = abs(beamLines.lines()[temp_pair.first].s.y - beamLines.lines()[temp_pair.second].s.y);
		}
		else
		{
			num = slantingDistanceLine(beamLines.lines()[temp_pair.first], beamLines.lines()[temp_pair.second]);
		}
		/**********************/
		width.push_back(num);
		sort_width.push_back(num);
		width_border.push_back(temp_pair);

	}
	//排序
	for (int i = 0; i < sort_width.size(); ++i)
	{
		for (int j = i; j < sort_width.size() - 1; ++j)
		{
			double temp = 0.0;
			if (sort_width[j] > sort_width[j + 1])
			{
				temp = sort_width[j];
				sort_width[j] = sort_width[j + 1];
				sort_width[j + 1] = temp;
			}
		}
	}
	//加权
	//int count = 0;
	double goal_value = 0.0;
	for (int i = 0; i < sort_width.size(); ++i)
	{
		int count = 0;
		for (int j = 0; j < sort_width.size(); ++j)
		{
			if (i != j)
			{
				if (sort_width[i] * 2.5 > sort_width[j])
					count++;
			}
		}
		if (count >= sort_width.size() / 2)
		{
			goal_value = sort_width[i];
			break;
		}

	}
	//计算均值
	int count = 0;
	for (int i = 0; i < width.size(); ++i)
	{
		if (goal_value * 2.5 > width[i])
		{
			sum_num += width[i];
			count++;
			goal_index.push_back(width_border[i].first);
			goal_index.push_back(width_border[i].second);
		}
	}
	return (sum_num / count);
}

bool BeamSet::findPairedBorder(std::pair<int, int>& pairedBorder)
{

	bool b_mark = false;
	int num1 = 0;
	int num2 = 0;
	Line mark_line;
	double width = 0.0;

	//随机产生一条边界

	do
	{
		b_mark = false;
		num1 = std::rand() % beamLines.lines().size();
		for (auto it : leadGroupVec)
		{
			auto ite = std::find(it.nowIndexVec.begin(), it.nowIndexVec.end(), num1);
			if (ite != it.nowIndexVec.end())
			{
				b_mark = true;
			}
		}
	} while (b_mark);

	mark_line = beamLines.lines()[num1];
	//需要错误处理


	num2 = returnGroupPairIndex(num1, width);
	assert(num2 != -1);
	pairedBorder = std::pair<int, int>(num1, num2);
	return true;
}

int BeamSet::findPairedBorder(const int& goal_index, const double& relative_value, LineData& beamLines,
							  const std::vector<Lead>& leadGrops)
{
	Line mark_line = beamLines.lines()[goal_index];
	int paired_index = 0;
	double length = -1;

	paired_index = returnGroupPairIndex(goal_index, length);

	if (length > 2.5 * relative_value || paired_index == -1)
	{
		return goal_index;
	}
	else
	{
		return paired_index;
	}
}

int BeamSet::findGropPairedBorder(const int& original_index, const double& relative_value,
								  LineData& beamLines, const std::vector<int>& borderGrop)
{
	auto line = beamLines.lines()[original_index];
	std::vector<int>hindex;
	std::vector<int>vindex;
	std::vector<int>sindex;
	Point mid_point1 = Point((line.s.x + line.e.x) / 2, (line.s.y + line.e.y) / 2);
	int goal_index;
	double length = 888888;
	for (auto it : borderGrop)
	{
		auto temp_line = beamLines.lines()[it];
		if (temp_line.horizontal())
		{
			hindex.push_back(it);
		}
		else if (temp_line.vertical())
		{
			vindex.push_back(it);
		}
		else
		{
			sindex.push_back(it);
		}
	}
	if (line.horizontal())
	{
		for (auto it : hindex)
		{
			if (it == original_index)
				continue;
			auto temp_line = beamLines.lines()[it];
			auto temp_point = Point((temp_line.s.x + temp_line.e.x) / 2, (temp_line.s.y + temp_line.e.y) / 2);

			auto temp_length = TowPointsLength(mid_point1, temp_point);
			if (length > temp_length&& it != original_index)
			{
				length = temp_length;
				goal_index = it;
			}
		}
	}
	else if (line.vertical())
	{
		for (auto it : vindex)
		{
			if (it == original_index)
				continue;
			auto temp_line = beamLines.lines()[it];
			auto temp_point = Point((temp_line.s.x + temp_line.e.x) / 2, (temp_line.s.y + temp_line.e.y) / 2);

			auto temp_length = TowPointsLength(mid_point1, temp_point);
			if (length > temp_length)
			{
				length = temp_length;
				goal_index = it;
			}
		}
	}
	else
	{
		for (auto it : sindex)
		{
			if (it == original_index)
				continue;
			auto temp_line = beamLines.lines()[it];
			if (IsParallelLines(line, temp_line))
			{

				auto temp_point = Point((temp_line.s.x + temp_line.e.x) / 2, (temp_line.s.y + temp_line.e.y) / 2);

				auto temp_length = TowPointsLength(mid_point1, temp_point);
				if (length > temp_length)
				{
					length = temp_length;
					goal_index = it;
				}
			}

		}
	}
	if (length > relative_value * 2.5)
	{
		//在没找到合适的配对时返回自己
		return original_index;
	}
	else
	{
		return goal_index;
	}
}

int BeamSet::returnGroupPairIndex(const int& goal_index, double& width)
{
	Line goal_line = beamLines.lines()[goal_index];
	int pair_index = -1;
	double distance = -1;
	if (goal_line.vertical())
	{
		double mid1_y = (goal_line.s.y + goal_line.e.y) / 2;
		Point mid1_num1(goal_line.s.x, mid1_y);


		for (int i = 0; i < beamLines.vLinesIndices().size(); ++i)
		{
			int index = beamLines.vLinesIndices()[i];
			bool b_lead = false;
			for (auto it : leadGroupVec)
			{
				auto ite = std::find(it.nowIndexVec.begin(), it.nowIndexVec.end(), index);
				if (ite != it.nowIndexVec.end())
				{
					b_lead = true;
				}

			}
			if (index == goal_index || b_lead || (beamLines.lines()[index].s.x + Precision > beamLines.lines()[goal_index].s.x&&
												  beamLines.lines()[index].s.x - Precision < beamLines.lines()[goal_index].s.x))
				continue;

			double mid2_y = (beamLines.lines()[index].s.y + beamLines.lines()[index].e.y) / 2;
			Point mid2_num2 = Point(beamLines.lines()[index].s.x, mid2_y);
			double length2 = TowPointsLength(mid1_num1, mid2_num2);
			if (distance == -1)
			{
				pair_index = index;
				distance = length2;
			}
			else
			{
				if (distance > length2)
				{
					distance = length2;
					pair_index = index;
				}
			}

		}
	}
	else if (goal_line.horizontal())
	{


		double mid1_x = (beamLines.lines()[goal_index].s.x + beamLines.lines()[goal_index].e.x) / 2;
		Point mid1_num1(mid1_x, beamLines.lines()[goal_index].s.y);
		//double length = -1;
		for (int i = 0; i < beamLines.hLinesIndices().size(); ++i)
		{
			int index = beamLines.hLinesIndices()[i];
			bool b_lead = false;
			for (auto it : leadGroupVec)
			{
				auto ite = std::find(it.nowIndexVec.begin(), it.nowIndexVec.end(), index);
				if (ite != it.nowIndexVec.end())
				{
					b_lead = true;
				}

			}
			if (index == goal_index || b_lead || (beamLines.lines()[index].s.y + Precision > beamLines.lines()[goal_index].s.y&&
												  beamLines.lines()[index].s.y - Precision < beamLines.lines()[goal_index].s.y))
				continue;
			double mid2_x = (beamLines.lines()[index].s.x + beamLines.lines()[index].e.x) / 2;
			Point mid2_num2 = Point(mid2_x, beamLines.lines()[index].s.y);
			double length2 = TowPointsLength(mid1_num1, mid2_num2);
			if (distance == -1)
			{
				pair_index = index;
				distance = length2;
			}
			else
			{
				if (distance > length2)
				{
					distance = length2;
					pair_index = index;
				}
			}

		}

	}
	else
	{


		for (auto it : beamLines.sLinesIndices())
		{
			if (IsParallelLines(beamLines.lines()[goal_index], beamLines.lines()[it]))
			{
				double y1 = (goal_line.s.y + goal_line.e.y) / 2;
				double x1 = (goal_line.s.x + goal_line.e.x) / 2;
				double y2 = (beamLines.lines()[it].s.y + beamLines.lines()[it].e.y) / 2;
				double x2 = (beamLines.lines()[it].s.x + beamLines.lines()[it].e.x) / 2;
				double temp_distance = ConnectPoint(Point(x1, y1), Point(x2, y2));
				if (distance == -1)
				{
					pair_index = it;
					distance = temp_distance;
				}
				else
				{
					if (distance > temp_distance)
					{
						pair_index = it;
						distance = temp_distance;
					}
				}
			}
		}
	}
	width = distance;
	if (pair_index == -1)
	{
		return goal_index;
	}
	else
	{
		return pair_index;
	}

}


bool BeamSet::findBeamBorderGrops()
{

	//std::vector<int>goal_index;
	std::map<int, int>beam_map;
	std::queue<int>queue_index;
	int leadGropIndex = 0;
	for (int i = 0; i < beamLines.lines().size(); ++i)
	{
		bool b_lead = bLeadLineBeamLayer(leadGroupVec, i, leadGropIndex);
		bool b_beam = beam_map.find(i) != beam_map.end() ? true : false;
		if (b_lead || b_beam)
		{
			continue;
		}
		BorderGroup temp_borderGrop;
		std::map<int, int>lead_map;
		queue_index.push(i);
		beam_map[i] = i;
		temp_borderGrop.borderVec.push_back(i);

		while (!queue_index.empty())
		{
			int index = queue_index.front();
			queue_index.pop();
			//在此添加验证部分，验证是否成对
#if 0
//			if (!temp_borderGrop.bGroup)
//			{
//				Line queue_line = beamLines.getLine(index);
//				if (queue_line.horizontal())
//				{
//					double yb = queue_line.s.y - 2 * avgWidth;
//					double yu = queue_line.s.y + 2 * avgWidth;
//					double blength = -1, tlength = -1;
//					int bindex = -1, tindex = -1;
//					int yi = beamLines.findLowerBound(
//						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
//					int yj = beamLines.findUpperBound(
//						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
//					//下
//					for (auto i = yi; i <= yj; i++)
//					{
//						int pair_index = beamLines.hLinesIndices()[i];
//
//						if (beamLines.getLine(pair_index).s.y >= queue_line.s.y)
//							break;
//						//排除引出线
//						int temp_leadIndex;
//						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
//						if (b_lead)
//							continue;
//
//						Line pair_line = beamLines.getLine(pair_index);
//						if (queue_line.e.x + Precision > pair_line.s.x&&
//							queue_line.s.x - Precision < pair_line.e.x)
//						{
//							double length = queue_line.s.y - pair_line.s.y;
//							if (blength == -1)
//							{
//								blength = length;
//								bindex = pair_index;
//							}
//							else if (blength > length)
//							{
//								blength = length;
//								bindex = pair_index;
//							}
//						}
//
//					}
//					//上
//					for (auto i = yj; i >= yi; --i)
//					{
//						int pair_index = beamLines.hLinesIndices()[i];
//						if (beamLines.getLine(pair_index).s.y <= queue_line.s.y)
//							break;
//
//						//排除引出线
//						int temp_leadIndex;
//						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
//						if (b_lead)
//							continue;
//
//						Line pair_line = beamLines.getLine(pair_index);
//						if (queue_line.e.x + Precision > pair_line.s.x&&
//							queue_line.s.x - Precision < pair_line.e.x)
//						{
//							double length = pair_line.s.y - queue_line.s.y;
//							if (tlength == -1)
//							{
//								tlength = length;
//								tindex = pair_index;
//							}
//							else if (tlength > length)
//							{
//								tlength = length;
//								tindex = pair_index;
//							}
//						}
//
//					}
//					if ((blength < 2 * avgWidth) &&
//						blength != -1)
//					{
//						temp_borderGrop.bGroup = true;
//						temp_borderGrop.borderVec.push_back(bindex);
//						queue_index.push(bindex);
//						beam_map[bindex] = bindex;
//					}
//					else if (tlength < 2 * avgWidth &&
//						(blength > 2 * avgWidth || blength == -1) &&
//							 tlength != -1)
//					{
//						temp_borderGrop.bGroup = true;
//						temp_borderGrop.borderVec.push_back(tindex);
//						queue_index.push(tindex);
//						beam_map[tindex] = tindex;
//					}
//
//				}
//				else if (queue_line.vertical())
//				{
//					double xl = queue_line.s.x - 2 * avgWidth;
//					double xr = queue_line.s.x + 2 * avgWidth;
//
//					double llength = -1, rlength = -1;
//					int lindex = -1, rindex = -1;
//
//					int xi = beamLines.findLowerBound(
//						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
//					int xj = beamLines.findUpperBound(
//						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);
//					for (int j = xi; j <= xj; ++j)
//					{
//						int pair_index = beamLines.vLinesIndices()[j];
//						if (beamLines.getLine(pair_index).s.x >= queue_line.s.x)
//							break;
//
//						//排除引出线
//						int temp_leadIndex;
//						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
//						if (b_lead)
//							continue;
//
//						Line pair_line = beamLines.getLine(pair_index);
//						if (queue_line.e.y + Precision > pair_line.s.y&&
//							queue_line.s.y - Precision < pair_line.e.y)
//						{
//							double length = queue_line.s.x - pair_line.s.x;
//							if (llength == -1)
//							{
//								llength = length;
//								lindex = pair_index;
//							}
//							else if (rlength > length)
//							{
//								llength = length;
//								lindex = pair_index;
//							}
//						}
//
//					}
//					for (int j = xj; j >= xi; --j)
//					{
//						int pair_index = beamLines.vLinesIndices()[j];
//						if (beamLines.getLine(pair_index).s.x <= queue_line.s.x)
//							break;
//
//						//排除引出线
//						int temp_leadIndex;
//						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
//						if (b_lead)
//							continue;
//
//						Line pair_line = beamLines.getLine(pair_index);
//						if (queue_line.e.y + Precision > pair_line.s.y&&
//							queue_line.s.y - Precision < pair_line.e.y)
//						{
//							double length = pair_line.s.x - queue_line.s.x;
//							if (rlength == -1)
//							{
//								rlength = length;
//								rindex = pair_index;
//							}
//							else if (rlength > length)
//							{
//								rlength = length;
//								rindex = pair_index;
//							}
//						}
//
//					}
//					if ((llength < 2 * avgWidth &&
//						 llength != -1))
//					{
//						temp_borderGrop.bGroup = true;
//						temp_borderGrop.borderVec.push_back(lindex);
//						queue_index.push(lindex);
//						beam_map[lindex] = lindex;
//					}
//					else if (rlength < 2 * avgWidth &&
//						(llength > 2 * avgWidth || llength == -1)
//							 && rlength != -1)
//					{
//						temp_borderGrop.bGroup = true;
//						temp_borderGrop.borderVec.push_back(rindex);
//						queue_index.push(rindex);
//						beam_map[rindex] = rindex;
//					}
//				}
#endif
			if (!temp_borderGrop.bGroup)
			{
				Line queue_line = beamLines.getLine(index);
				//TODO此处的配对还存在一些问题不够完善，体现最明显的是在北沙滩图纸当中
				if (queue_line.horizontal())
				{
					double yb = queue_line.s.y - 2 * avgWidth;
					double yu = queue_line.s.y + 2 * avgWidth;
					double blength = -1, tlength = -1;
					int bindex = -1, tindex = -1;
					std::vector<int>candidateIndex;//!候选对边的索引

					int yi = beamLines.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
					int yj = beamLines.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
					//下
					for (auto i = yi; i <= yj; i++)
					{
						int pair_index = beamLines.hLinesIndices()[i];

						if (beamLines.getLine(pair_index).s.y >= queue_line.s.y)
							break;
						//排除引出线
						int temp_leadIndex;
						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
						if (b_lead)
							continue;

						Line pair_line = beamLines.getLine(pair_index);
						if (queue_line.e.x + Precision > pair_line.s.x&&
							queue_line.s.x - Precision < pair_line.e.x)
						{
							double length = queue_line.s.y - pair_line.s.y;
							if (blength == -1)
							{
								blength = length;
								bindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
							else if (blength > length)
							{
								blength = length;
								bindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
						}

					}
					//上
					for (auto i = yj; i >= yi; --i)
					{
						int pair_index = beamLines.hLinesIndices()[i];
						if (beamLines.getLine(pair_index).s.y <= queue_line.s.y)
							break;

						//排除引出线
						int temp_leadIndex;
						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
						if (b_lead)
							continue;

						Line pair_line = beamLines.getLine(pair_index);
						if (queue_line.e.x + Precision > pair_line.s.x&&
							queue_line.s.x - Precision < pair_line.e.x)
						{
							double length = pair_line.s.y - queue_line.s.y;
							if (tlength == -1)
							{
								tlength = length;
								tindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
							else if (tlength > length)
							{
								tlength = length;
								tindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
						}

					}
					/*if ((blength < 2 * avgWidth) &&
						blength != -1)
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(bindex);
						queue_index.push(bindex);
						beam_map[bindex] = bindex;
					}
					else if (tlength < 2 * avgWidth &&
						(blength > 2 * avgWidth || blength == -1) &&
						tlength != -1)
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(tindex);
						queue_index.push(tindex);
						beam_map[tindex] = tindex;
					}*/
					double goal_distance = -1;
					int goal_index = -1;
					for (auto it : candidateIndex)
					{
						//!求候选对边与当前线之间的宽度
						double pair_width = std::abs(queue_line.s.y - beamLines.getLine(it).s.y);
						if (pair_width < avgWidth * 0.5 || pair_width > 2 * avgWidth)
							continue;
						//!求候选线与当前线之间的公共长度
						Point maxSPoint = queue_line.s.x > beamLines.getLine(it).s.x ? queue_line.s : beamLines.getLine(it).s;
						Point minEPoint = queue_line.e.x < beamLines.getLine(it).e.x ? queue_line.e : beamLines.getLine(it).e;
						double publicLength = std::abs(maxSPoint.x - minEPoint.x);

						if (publicLength < beamLines.getLine(it).length() * 0.5 &&
							publicLength < queue_line.length() * 0.5)
						{
							continue;
						}
						else if (publicLength == beamLines.getLine(it).length() &&
								 publicLength < queue_line.length() * 0.5)
						{
							continue;
						}
						else if (publicLength < beamLines.getLine(it).length() * 0.5 &&
								 publicLength == queue_line.length())
						{
							continue;
						}
						if (goal_distance == -1)
						{
							goal_distance = pair_width;
							goal_index = it;
						}
						else if (goal_distance > pair_width)
						{
							goal_distance = pair_width;
							goal_index = it;
						}
					}
					//!压入目标对边
					if (goal_index != -1)
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(goal_index);
						queue_index.push(goal_index);
						beam_map[goal_index] = goal_index;
					}
				}
				else if (queue_line.vertical())
				{
					double xl = queue_line.s.x - 2 * avgWidth;
					double xr = queue_line.s.x + 2 * avgWidth;

					double llength = -1, rlength = -1;
					int lindex = -1, rindex = -1;
					std::vector<int>candidateIndex;//!候选对边的索引

					int xi = beamLines.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
					int xj = beamLines.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);
					for (int j = xi; j <= xj; ++j)
					{
						int pair_index = beamLines.vLinesIndices()[j];
						if (beamLines.getLine(pair_index).s.x >= queue_line.s.x)
							break;

						//排除引出线
						int temp_leadIndex;
						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
						if (b_lead)
							continue;

						Line pair_line = beamLines.getLine(pair_index);
						if (queue_line.e.y + Precision > pair_line.s.y&&
							queue_line.s.y - Precision < pair_line.e.y)
						{
							double length = queue_line.s.x - pair_line.s.x;
							if (llength == -1)
							{
								llength = length;
								lindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
							else if (rlength > length)
							{
								llength = length;
								lindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
						}

					}
					for (int j = xj; j >= xi; --j)
					{
						int pair_index = beamLines.vLinesIndices()[j];
						if (beamLines.getLine(pair_index).s.x <= queue_line.s.x)
							break;

						//排除引出线
						int temp_leadIndex;
						bool b_lead = bLeadLineBeamLayer(leadGroupVec, pair_index, temp_leadIndex);
						if (b_lead)
							continue;

						Line pair_line = beamLines.getLine(pair_index);
						if (queue_line.e.y + Precision > pair_line.s.y&&
							queue_line.s.y - Precision < pair_line.e.y)
						{
							double length = pair_line.s.x - queue_line.s.x;
							if (rlength == -1)
							{
								rlength = length;
								rindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
							else if (rlength > length)
							{
								rlength = length;
								rindex = pair_index;
								candidateIndex.push_back(pair_index);
							}
						}

					}
					/*if ((llength < 2 * avgWidth &&
						llength != -1))
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(lindex);
						queue_index.push(lindex);
						beam_map[lindex] = lindex;
					}
					else if (rlength < 2 * avgWidth &&
						(llength > 2 * avgWidth || llength == -1)
						&& rlength != -1)
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(rindex);
						queue_index.push(rindex);
						beam_map[rindex] = rindex;
					}*/
					double goal_distance = -1;
					int goal_index = -1;
					for (auto it : candidateIndex)
					{
						//!求候选对边与当前线之间的宽度
						double pair_width = std::abs(queue_line.s.x - beamLines.getLine(it).s.x);
						if (pair_width < avgWidth * 0.5 || pair_width > 2 * avgWidth)
							continue;
						//!求候选线与当前线之间的公共长度
						Point maxSPoint = queue_line.s.y > beamLines.getLine(it).s.y ? queue_line.s : beamLines.getLine(it).s;
						Point minEPoint = queue_line.e.y < beamLines.getLine(it).e.y ? queue_line.e : beamLines.getLine(it).e;
						double publicLength = std::abs(maxSPoint.y - minEPoint.y);

						if (publicLength < beamLines.getLine(it).length() * 0.5 &&
							publicLength < queue_line.length() * 0.5)
						{
							continue;
						}
						else if (publicLength == beamLines.getLine(it).length() &&
								 publicLength < queue_line.length() * 0.5)
						{
							continue;
						}
						else if (publicLength < beamLines.getLine(it).length() * 0.5 &&
								 publicLength == queue_line.length())
						{
							continue;
						}
						if (goal_distance == -1)
						{
							goal_distance = pair_width;
							goal_index = it;
						}
						else if (goal_distance > pair_width)
						{
							goal_distance = pair_width;
							goal_index = it;
						}
					}
					//!压入目标对边
					if (goal_index != -1)
					{
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(goal_index);
						queue_index.push(goal_index);
						beam_map[goal_index] = goal_index;
					}

				}
				else
				{
					//确定矩形范围
					double left = beamLines.lines()[index].s.x;
					double bottom = beamLines.lines()[index].s.y <
						beamLines.lines()[index].e.y ?
						beamLines.lines()[index].s.y - avgWidth :
						beamLines.lines()[index].e.y - avgWidth;
					double reght = beamLines.lines()[index].e.x;
					double top = beamLines.lines()[index].s.y <
						beamLines.lines()[index].e.y ?
						beamLines.lines()[index].e.y + avgWidth :
						beamLines.lines()[index].s.y + avgWidth;
					double min[2] = { left,bottom };
					double max[2] = { reght,top };
					int num = borderLineRTree.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
																				  &borderLineRTree, std::placeholders::_1));

					//在矩形范围内读取最适合的对边
					int goal_pairIndex = -1;
					double min_distance = 0.0;

					for (auto it : borderLineRTree.SBackIndex())
					{

						if (beamLines.lines()[it].horizontal() ||
							beamLines.lines()[it].vertical() ||
							(beam_map.find(it) != beam_map.end()) ||
							/*slashCornerJudgment(it,index,Point())||*/
							it == index)
						{
							continue;
						}
						double y1 = (queue_line.s.y + queue_line.e.y) / 2;
						double x1 = (queue_line.s.x + queue_line.e.x) / 2;
						double y2 = (beamLines.lines()[it].s.y + beamLines.lines()[it].e.y) / 2;
						double x2 = (beamLines.lines()[it].s.x + beamLines.lines()[it].e.x) / 2;
						double temp_distance = TowPointsLength(Point(x1, y1), Point(x2, y2));
						if (goal_pairIndex == -1)
						{
							goal_pairIndex = it;
							min_distance = temp_distance;
						}
						else if (min_distance > temp_distance)
						{
							goal_pairIndex = it;
							min_distance = temp_distance;
						}
					}
					//m_rTreeBorderLines.clearSBackIndex();
					if (goal_pairIndex != -1)
					{
						//beamLines.lines()[goal_pairIndex];
						//queue_line;
						temp_borderGrop.bGroup = true;
						temp_borderGrop.borderVec.push_back(goal_pairIndex);
						queue_index.push(goal_pairIndex);
						beam_map[goal_pairIndex] = goal_pairIndex;
					}

				}


			}
			//平行模块
			{
				Line now_line = beamLines.getLine(index);
				if (now_line.vertical())
				{
					double xl = now_line.s.x - Precision;
					double xr = now_line.s.x + Precision;
					int xi = beamLines.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
					int xj = beamLines.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);
					if (xi != xj)
					{
						for (int i = xi; i <= xj; ++i)
						{
							int v_index = beamLines.vLinesIndices()[i];
							if (v_index == index)
								continue;
							Line v_line = beamLines.getLine(v_index);
							if (v_line.e.y + Precision > now_line.s.y&&
								v_line.s.y - Precision < now_line.e.y)
							{
								if (beam_map.find(v_index) == beam_map.end())
								{
									queue_index.push(v_index);
									beam_map[v_index] = v_index;
									temp_borderGrop.borderVec.push_back(v_index);

								}
							}
						}
					}


				}
				else if (now_line.horizontal())
				{
					double yb = now_line.s.y - Precision;
					double yu = now_line.s.y + Precision;
					int yi = beamLines.findLowerBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
					int yj = beamLines.findUpperBound(
						LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
					if (yi != yj)
					{
						for (int j = yi; j <= yj; ++j)
						{
							int h_index = beamLines.hLinesIndices()[j];
							if (h_index == index)
								continue;
							Line h_line = beamLines.getLine(h_index);
							if (h_line.e.x + Precision > now_line.s.x&&
								h_line.s.x - Precision < now_line.e.x)
							{
								if (beam_map.find(h_index) == beam_map.end())
								{
									queue_index.push(h_index);
									beam_map[h_index] = h_index;
									temp_borderGrop.borderVec.push_back(h_index);

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

			//相交模块
			auto corners = beamCornerData.corners().find(index);
			if (corners != beamCornerData.corners().end())
			{
				for (auto corner : corners->second)
				{
					auto l1 = corner.l1;
					if (l1 == index)
					{
						l1 = corner.l2;
					}

					auto b_Beam_mark = beam_map.find(l1) != beam_map.end() ? true : false;
					auto b_lead_map = lead_map.find(l1) == lead_map.end() ? true : false;
					if (!((beamLines.lines()[index].vertical() && beamLines.lines()[l1].horizontal() ||
						   beamLines.lines()[index].horizontal() && beamLines.lines()[l1].vertical()
						   )) &&
						   (/*ConnectPoint(beamLines.lines()[index].s, beamLines.lines()[l1].s) ||
							   ConnectPoint(beamLines.lines()[index].s, beamLines.lines()[l1].e) ||
							   ConnectPoint(beamLines.lines()[index].e, beamLines.lines()[l1].s) ||
							   ConnectPoint(beamLines.lines()[index].e, beamLines.lines()[l1].e) ||*/
							slashCornerJudgment(index, l1, Point(corner.x, corner.y))
							) && !b_Beam_mark)
					{

						queue_index.push(l1);
						temp_borderGrop.borderVec.push_back(l1);
						beam_map[l1] = l1;
					}
					else if (bLeadLineBeamLayer(leadGroupVec, l1, leadGropIndex) && b_lead_map)
					{
						temp_borderGrop.leadGropsIndexVec.emplace_back(leadGropIndex, true);
						leadGroupVec[leadGropIndex].bPair = true;
						leadGroupVec[leadGropIndex].refCount++;
						//leadGrops[leadGropIndex].m_pair = true;
						temp_borderGrop.bLead = true;

						for (auto lead_ite : leadGroupVec[leadGropIndex].nowIndexVec)
						{
							lead_map[lead_ite] = lead_ite;
							beam_map[lead_ite] = lead_ite;
							//assert(!(lead_ite == 334));

						}

					}
				}
			}
			//并在此期间做对边查找，直到找到一条对边为止

		}
		borderGroupVec.push_back(temp_borderGrop);

	}


	return true;
}

bool BeamSet::slashCornerJudgment(const int& sLineIndex, const int& lineIndex, const Point& point)
{
#if 1
	/*if (bLineConnect(beamLines.lines()[sLineIndex], beamLines.lines()[lineIndex]))
	{
		return true;
	}
	else*/
	{
		auto sIndexType = returnLineType(beamLines.lines()[sLineIndex]);
		auto lIndexType = returnLineType(beamLines.lines()[lineIndex]);
		//!两条线都是斜线,相交点到线端点之间的距离小于自身的0.01，就算相接
		if (sIndexType == 3 && lIndexType == 3)
		{
			double sLineDistance = beamLines.lines()[sLineIndex].length();
			double LineDistance = beamLines.lines()[lineIndex].length();
			double sLength1 = TowPointsLength(beamLines.lines()[sLineIndex].s, point);
			double sLength2 = TowPointsLength(beamLines.lines()[sLineIndex].e, point);
			double lLength1 = TowPointsLength(beamLines.lines()[lineIndex].s, point);
			double lLength2 = TowPointsLength(beamLines.lines()[lineIndex].e, point);
			double sGoalLength = sLength1 < sLength2 ? sLength1 : sLength2;
			double lGoalLength = lLength1 < lLength2 ? lLength1 : lLength2;
			if (sGoalLength < sLineDistance * 0.01 && lGoalLength < LineDistance * 0.01)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else if (sIndexType == 3)//!sLine为斜线
		{
			//!计算sLine的斜率
			double k = GetTanOfLine(beamLines.lines()[sLineIndex]);
			//!Line 为横线，只要sLine的斜率满足0到45，135到180且交点到端点距离小于百分之一
			double sLineDistance = beamLines.lines()[sLineIndex].length();
			double sLength1 = TowPointsLength(beamLines.lines()[sLineIndex].s, point);
			double sLength2 = TowPointsLength(beamLines.lines()[sLineIndex].e, point);
			double sGoalLength = sLength1 < sLength2 ? sLength1 : sLength2;
			if (lIndexType == 1)
			{
				if (((k <= 1 && k > 0) || (k<0 && k>-1)) &&
					sGoalLength < sLineDistance * 0.01)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else//!Line 为竖线，只要sLine的斜率满足45到135且交点到端点距离小于百分之一
			{
				if (((k >= 1) || (k <= -1)) &&
					sGoalLength < sLineDistance * 0.01)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		else//!sLine不是斜线
		{
			//!计算sLine的斜率
			double k = GetTanOfLine(beamLines.lines()[lineIndex]);
			//!Line 为横线，只要Line的斜率满足0到45，135到180且交点到端点距离小于百分之一
			double LineDistance = beamLines.lines()[lineIndex].length();
			double lLength1 = TowPointsLength(beamLines.lines()[lineIndex].s, point);
			double lLength2 = TowPointsLength(beamLines.lines()[lineIndex].e, point);
			double lGoalLength = lLength1 < lLength2 ? lLength1 : lLength2;
			if (sIndexType == 1)
			{
				if (((k <= 1 && k > 0) || (k<0 && k>-1)) &&
					lGoalLength < LineDistance * 0.01)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else//!Line 为竖线，只要Line的斜率满足45到135且交点到端点距离小于百分之一
			{
				if (((k >= 1) || (k <= -1)) &&
					lGoalLength < LineDistance * 0.01)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
#endif
#if 0
	auto sCorners = beamCornerData.corners().find(sLineIndex);
	auto begin = sCorners->second.begin();
	auto end = sCorners->second.end();
	bool b_mark = false;
	Point goal_point;
	for (auto it = begin; it != end; ++it)
	{
		auto l1 = it->l1;
		if (l1 == sLineIndex)
		{
			l1 = it->l2;
		}
		if (l1 == lineIndex)
		{
			b_mark = true;
			goal_point = Point(it->x, it->y);
			break;
		}
	}
	if (b_mark)
	{
		double length = TowPointsLength(beamLines.lines()[sLineIndex].s, beamLines.lines()[sLineIndex].e);
		double distance1 = TowPointsLength(goal_point, beamLines.lines()[sLineIndex].s);
		double distance2 = TowPointsLength(goal_point, beamLines.lines()[sLineIndex].e);
		double goal_distance = distance1 < distance2 ? distance1 : distance2;
		if (ConnectPoint(beamLines.lines()[sLineIndex].s, goal_point) ||
			ConnectPoint(beamLines.lines()[sLineIndex].e, goal_point) ||
			goal_distance < length * 0.01)
		{
			return true;
		}
	}
	return false;
#endif
}


bool BeamSet::bLeadLineBeamLayer(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex)
{
	//for (auto it = leadGrops.begin(); it != leadGrops.end(); ++it);
	for (int i = 0; i < leadGrops.size(); ++i)
	{
		auto ite = std::find(leadGrops[i].nowIndexVec.begin(), leadGrops[i].nowIndexVec.end(), index);
		if (ite != leadGrops[i].nowIndexVec.end())
		{
			LeadGropIndex = i;
			return true;
		}

	}
	return false;
}

bool BeamSet::bLeadLineAllLines(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex)
{
	for (int i = 0; i < leadGrops.size(); ++i)
	{
		auto ite = std::find(leadGrops[i].primevalIndexVec.begin(), leadGrops[i].primevalIndexVec.end(), index);
		if (ite != leadGrops[i].primevalIndexVec.end())
		{
			LeadGropIndex = i;
			return true;
		}

	}
	return false;
}

bool BeamSet::removeExcessLeadInBorderGrop(std::vector<BorderGroup>& borderGrops, LineData& beamLines, const double& relative_value,
										   CornerData& borderCorners, const double& average_distance, std::vector<Lead>& leadGrops)
{
	for (int it = 0; it < borderGrops.size(); ++it)
	{
		if (!borderGrops[it].leadGropsIndexVec.empty())
		{

			//std::vector<LeadGrop>::const_iterator lead_line_ite;

			bLeadlineBorderPaired(beamLines, average_distance, borderCorners, borderGrops[it], leadGrops);


		}
	}
	return true;
}

//判断引出线是否描述当前边界
bool BeamSet::bLeadlineBorderPaired(LineData& beamLines, const double& relative_value,
									CornerData& borderCorners, BorderGroup& borderGrop, std::vector<Lead>& leadgrops)
{
	std::vector<Lead> leadGrops;
	std::vector<std::pair<int, bool>>leadGrops_index;
	for (auto bream_grop : borderGrop.leadGropsIndexVec)
	{
		int goal_index = -1;
		int lead_corner_size = 0;
		//CornerData temp_corner;
		std::map<int, std::vector<Corner>>::const_iterator temp_corner = borderCorners.corners().end();

		//判断引出线相交直线
		Lead temp_leadGrop = leadgrops[bream_grop.first];

		for (auto cornerLines : temp_leadGrop.cornerIndex)
		{
			temp_corner = borderCorners.corners().find(cornerLines.second);
			/********/
			if (temp_corner == borderCorners.corners().end())
				continue;
			std::pair<int, int>pair_border;

			for (auto it : temp_corner->second)
			{
				auto l1 = it.l1;
				if (l1 == temp_corner->first)
				{
					l1 = it.l2;
				}
				if (std::find(temp_leadGrop.nowIndexVec.begin(), temp_leadGrop.nowIndexVec.end(), l1)
					== temp_leadGrop.nowIndexVec.end())
				{

					//int temp_index = findPairedBorder(l1, relative_value, beamLines, borderCorners.m_borders);
					int temp_index = findGropPairedBorder(l1, relative_value, beamLines, borderGrop.borderVec);
					if (temp_index == l1)
						continue;
					pair_border = std::pair<int, int>(l1, temp_index);

				}

			}
			//有闭合点与没有闭合点需要单独处理
			if (pair_border.first == pair_border.second)
				continue;
			if (temp_leadGrop.bSide)
			{
				std::vector<Point> mid_point = temp_leadGrop.insertPointVec;
				double width = temp_leadGrop.polyWidth;

				bool b_mark = false;
				for (auto it : mid_point)
				{
					auto temp_line = beamLines.lines()[pair_border.first];
					if (temp_line.vertical())
					{
						//此处先做简单的判断
						Line left, right;
						if (beamLines.lines()[pair_border.first].s.x < beamLines.lines()[pair_border.second].s.x)
						{
							left = beamLines.lines()[pair_border.first];
							right = beamLines.lines()[pair_border.second];
						}
						else
						{
							left = beamLines.lines()[pair_border.second];
							right = beamLines.lines()[pair_border.first];
						}

						if (left.s.x - width > it.x || right.s.x + width < it.x)
						{
							b_mark = true;
						}

					}
					else if (temp_line.horizontal())
					{
						//此处先做简单的判断
						Line bottom, up;

						if (beamLines.lines()[pair_border.first].s.y < beamLines.lines()[pair_border.second].s.y)
						{
							up = beamLines.lines()[pair_border.second];
							bottom = beamLines.lines()[pair_border.first];

						}
						else
						{
							up = beamLines.lines()[pair_border.first];
							bottom = beamLines.lines()[pair_border.second];;
						}
						if (bottom.s.y - width > it.y || up.s.y + width < it.y)
						{
							b_mark = true;
						}

					}
					else
					{
						;//斜边暂不考虑
					}
				}
				if (!b_mark)
				{
					leadGrops.push_back(temp_leadGrop);
					leadGrops_index.push_back(bream_grop);

				}

			}
			else
			{
				Line lead_line = beamLines.lines()[temp_corner->first];

				bool b_mark = false;

				auto temp_line = beamLines.lines()[pair_border.first];
				if (temp_line.vertical())
				{
					//此处先做简单的判断
					Line left, right;
					if (beamLines.lines()[pair_border.first].s.x < beamLines.lines()[pair_border.second].s.x)
					{
						left = beamLines.lines()[pair_border.first];
						right = beamLines.lines()[pair_border.second];
					}
					else
					{
						left = beamLines.lines()[pair_border.second];
						right = beamLines.lines()[pair_border.first];
					}

					if ((left.s.x > lead_line.s.x&& right.s.x < lead_line.e.x) ||
						(left.s.x > lead_line.e.x&& right.s.x < lead_line.e.x))
					{
						b_mark = true;
					}


				}
				else if (temp_line.horizontal())
				{
					//此处先做简单的判断
					Line bottom, up;

					if (beamLines.lines()[pair_border.first].s.y < beamLines.lines()[pair_border.second].s.y)
					{
						up = beamLines.lines()[pair_border.second];
						bottom = beamLines.lines()[pair_border.first];

					}
					else
					{
						up = beamLines.lines()[pair_border.first];
						bottom = beamLines.lines()[pair_border.second];;
					}
					if ((up.s.y < lead_line.e.y && bottom.s.y > lead_line.s.y) ||
						(up.s.y < lead_line.s.y && bottom.s.y > lead_line.e.y))
					{
						b_mark = true;
					}

				}
				else
				{
					;//斜边暂不考虑
				}

				if (!b_mark)
				{
					leadGrops.push_back(temp_leadGrop);
					leadGrops_index.push_back(bream_grop);
				}


			}
		}


	}
	for (auto it = borderGrop.leadGropsIndexVec.begin(); it != borderGrop.leadGropsIndexVec.end(); ++it)
	{

		//auto b_pair = std::find(temp_leadGrop.begin(), temp_leadGrop.end(), it);
		bool b_mark = false;
		for (auto it1 : leadGrops)
		{
			if (leadgrops[(*it).first] == it1)
			{
				b_mark = true;
				break;
			}
		}

		if (!b_mark)
		{
			leadgrops[(*it).first].bPair = false;
		}
		else
		{
			leadgrops[(*it).first].bPair = true;
		}

	}

	if (leadGrops.empty())
	{
		borderGrop.bLead = false;
		borderGrop.leadGropsIndexVec.clear();
	}
	else
	{
		borderGrop.bLead = true;
		borderGrop.leadGropsIndexVec = leadGrops_index;

	}

	return true;
}

bool BeamSet::byConditionExtractBorderGropLeadGrop(const std::vector<BorderGroup>& borderGrops, const bool& bLead, const bool& bGrop,
												   const std::vector<Lead>& leadGrops, const bool& bSide, const bool& bPair, std::vector<int>& conditationBorder, std::vector<int>& conditationLead)
{
	for (int i = 0; i < borderGrops.size(); ++i)
	{
		if (borderGrops[i].bLead == bLead && borderGrops[i].bGroup == bGrop)
		{
			conditationBorder.push_back(i);
		}
	}
	for (int i = 0; i < leadGrops.size(); ++i)
	{
		if (leadGrops[i].bSide == bSide && leadGrops[i].bPair == bPair)
		{
			conditationLead.push_back(i);
		}
	}

	return true;
}

bool BeamSet::borderLeadPair1(Data& beamData, const double& averageWidth, std::vector<Lead>& leadGrops, std::vector<BorderGroup>& borderGrops,
							  std::vector<int>& conditationBorder, std::vector<int>& conditationLead)
{
	for (auto it : conditationLead)
	{


		for (auto insertPoint : leadGrops[it].insertPointVec)
		{

			int bottomIndex = -1, topIndex = -1, leftIndex = -1, reghtIndex = -1;
			double bottomLength = 0, topLength = 0, leftLength = 0, reghtLength = 0;
			bool b_xSame = false, b_ySame = false;


			double yb = insertPoint.y - averageWidth;
			double yu = insertPoint.y + averageWidth;
			double xl = insertPoint.x - averageWidth;
			double xr = insertPoint.x + averageWidth;
			int yi = beamData.m_lineData.findLowerBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
			int yj = beamData.m_lineData.findUpperBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
			int xi = beamData.m_lineData.findLowerBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
			int xj = beamData.m_lineData.findUpperBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);

			if (yi != yj)
			{
				//横下
				for (int bY = yi; bY <= yj; ++bY)
				{
					int index = beamData.m_lineData.hLinesIndices()[bY];
					Line temp_line = beamData.m_lineData.getLine(index);
					double temp_length = abs(beamData.m_lineData.getLine(index).s.y - insertPoint.y);
					if (beamData.m_lineData.getLine(index).s.y < insertPoint.y)
					{
						if (insertPoint.x > temp_line.s.x&& insertPoint.x < temp_line.e.x)
						{
							if (bottomLength == 0)
							{
								bottomLength = temp_length;
								bottomIndex = index;
							}
							else if (bottomLength > temp_length)
							{
								bottomLength = temp_length;
								bottomIndex = index;
							}
						}
					}
					else
					{
						break;
					}

				}
				//横上
				for (int tY = yj; tY >= yi; --tY)
				{
					int index = beamData.m_lineData.hLinesIndices()[tY];
					Line temp_line = beamData.m_lineData.getLine(index);
					double temp_length = abs(beamData.m_lineData.getLine(index).s.y - insertPoint.y);
					if (beamData.m_lineData.getLine(index).s.y > insertPoint.y)
					{
						if (insertPoint.x > temp_line.s.x&& insertPoint.x < temp_line.e.x)
						{
							if (topLength == 0)
							{
								topLength = temp_length;
								topIndex = index;
							}
							else if (topLength > temp_length)
							{
								topLength = temp_length;
								topIndex = index;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				b_ySame = true;
			}

			if (xj != xi)
			{
				//纵左
				for (int lX = xi; lX <= xj; ++lX)
				{
					int index = beamData.m_lineData.vLinesIndices()[lX];
					Line temp_line = beamData.m_lineData.getLine(index);
					double temp_length = abs(beamData.m_lineData.getLine(index).s.x - insertPoint.x);
					if (beamData.m_lineData.getLine(index).s.x < insertPoint.x)
					{
						if (insertPoint.y > temp_line.s.y&& insertPoint.y < temp_line.e.y)
						{
							if (leftLength == 0)
							{
								leftLength = temp_length;
								leftIndex = index;
							}
							else if (leftLength > temp_length)
							{
								leftLength = temp_length;
								leftIndex = index;
							}
						}
					}
					else
					{
						break;
					}
				}
				//纵右
				for (int rX = xj; rX >= xi; --rX)
				{
					int index = beamData.m_lineData.vLinesIndices()[rX];
					Line temp_line = beamData.m_lineData.getLine(index);
					double temp_length = abs(beamData.m_lineData.getLine(index).s.x - insertPoint.x);
					if (beamData.m_lineData.getLine(index).s.x > insertPoint.x)
					{
						if (insertPoint.y > temp_line.s.y&& insertPoint.y < temp_line.e.y)
						{
							if (reghtLength == 0)
							{
								reghtLength = temp_length;
								reghtIndex = index;
							}
							else if (reghtLength > temp_length)
							{
								reghtLength = temp_length;
								reghtIndex = index;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				b_xSame = true;
			}
			if ((((bottomLength + topLength) < 2 * averageWidth &&
				(leftLength + reghtLength) < 2 * averageWidth) ||
				  ((bottomLength + topLength) > 2 * averageWidth &&
				 (leftLength + reghtLength) > 2 * averageWidth)) &&
				b_ySame&& b_xSame)
			{
				;
			}

			else if ((bottomLength + topLength) < 2 * averageWidth && !b_ySame && (bottomIndex != -1 || topIndex != -1))
			{
				int borderGropsIndex = -1;
				bool b_inBorder = bBorderGrop(borderGrops, std::pair<int, int>(bottomIndex, topIndex), borderGropsIndex);
				leadGrops[it].bPair = true;
				leadGrops[it].bSide = true;
				//borderGrops[borderGropsIndex].m_blead = true;
				if (borderGropsIndex != -1)
				{
					conditationBorder.push_back(borderGropsIndex);
					borderGrops[borderGropsIndex].leadGropsIndexVec.push_back(std::pair<int, bool>(it, true));
					borderGrops[borderGropsIndex].bLead = true;
					leadGrops[it].bPair = true;
				}


			}
			else if ((leftLength + reghtLength) < 2 * averageWidth && !b_xSame && (leftIndex != -1 || reghtIndex != -1))
			{
				int borderGropsIndex = -1;
				bool b_inBorder = bBorderGrop(borderGrops, std::pair<int, int>(leftIndex, reghtIndex), borderGropsIndex);
				leadGrops[it].bPair = true;
				leadGrops[it].bSide = true;

				if (borderGropsIndex != -1)
				{
					conditationBorder.push_back(borderGropsIndex);
					borderGrops[borderGropsIndex].leadGropsIndexVec.push_back(std::pair<int, bool>(it, true));
					borderGrops[borderGropsIndex].bLead = true;
					leadGrops[it].bPair = true;
				}
			}
		}



	}
	return true;
}

bool BeamSet::bBorderGrop(std::vector<BorderGroup>& borderGrops, const std::pair<int, int>& borderPair, int& borderGroupsIndex)
{
	bool b_mark = false;
	for (auto ite = borderGrops.begin(); ite != borderGrops.end(); ++ite)
	{
		auto ite_first = std::find(ite->borderVec.begin(), ite->borderVec.end(), borderPair.first);
		if (ite_first != ite->borderVec.end())
		{
			auto ite_second = std::find(ite->borderVec.begin(), ite->borderVec.end(), borderPair.second);
			if (ite_second != ite->borderVec.end())
			{
				b_mark = true;
				borderGroupsIndex = std::distance(borderGrops.begin(), ite);
				return true;
				//break;
			}
		}
	}
	if (!b_mark)
	{
		borderGroupsIndex = -1;
		return false;
	}

	//return false;
}

bool BeamSet::byConditionExtractBorderGropTextPoints(const std::vector<BorderGroup>& borderGrops, const bool& bLead1, const bool& bGrop,
													 const std::vector<BeamTextPoint>& textPoints, const bool& bLead2, std::vector<int>& conditationBorder, std::vector<int>& conditationPoint)
{
	for (int i = 0; i < borderGrops.size(); ++i)
	{
		if (borderGrops[i].bGroup == bGrop && borderGrops[i].bLead == bLead1)
		{
			conditationBorder.push_back(i);
		}
	}
	for (int i = 0; i < textPoints.size(); ++i)
	{
		if (textPoints[i].bLeadLine == bLead2)
		{
			conditationPoint.push_back(i);
		}
	}
	return true;
}

bool BeamSet::borderPointPair(Data& beamData, const double& averageWidth, std::vector<BeamTextPoint>& textPoints, std::vector<BorderGroup>& borderGrops, std::vector<int>& conditationBorder, std::vector<int>& conditationTextPint)
{
	for (auto it : conditationTextPint)
	{
		int bottomIndex = -1, topIndex = -1, leftIndex = -1, reghtIndex = -1;
		double bottomLength = 0, topLength = 0, leftLength = 0, reghtLength = 0;
		bool b_xSame = false, b_ySame = false;
		Point textPoint = textPoints[it].pt;
		double yb = textPoint.y - averageWidth;
		double yu = textPoint.y + averageWidth;
		double xl = textPoint.x - averageWidth;
		double xr = textPoint.x + averageWidth;
		int yi = beamData.m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int yj = beamData.m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		int xi = beamData.m_lineData.findLowerBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xl, Precision, false);
		int xj = beamData.m_lineData.findUpperBound(
			LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, xr, Precision, false);

		if (yi != -1 && yj != -1 && yi != yj)
		{
			//横下
			for (int bY = yi; bY <= yj; ++bY)
			{
				int index = beamData.m_lineData.hLinesIndices()[bY];
				Line temp_line = beamData.m_lineData.getLine(index);
				double temp_length = abs(beamData.m_lineData.getLine(index).s.y - textPoint.y);
				if (beamData.m_lineData.getLine(index).s.y < textPoint.y)
				{
					if (textPoint.x > temp_line.s.x&& textPoint.x < temp_line.e.x)
					{
						if (bottomLength == 0)
						{
							bottomLength = temp_length;
							bottomIndex = index;
						}
						else if (bottomLength > temp_length)
						{
							bottomLength = temp_length;
							bottomIndex = index;
						}
					}
				}
				else
				{
					break;
				}

			}
			//横上
			for (int tY = yj; tY >= yi; --tY)
			{
				int index = beamData.m_lineData.hLinesIndices()[tY];
				Line temp_line = beamData.m_lineData.getLine(index);
				double temp_length = abs(beamData.m_lineData.getLine(index).s.y - textPoint.y);
				if (beamData.m_lineData.getLine(index).s.y > textPoint.y)
				{
					if (textPoint.x > temp_line.s.x&& textPoint.x < temp_line.e.x)
					{
						if (topLength == 0)
						{
							topLength = temp_length;
							topIndex = index;
						}
						else if (topLength > temp_length)
						{
							topLength = temp_length;
							topIndex = index;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			b_ySame = true;
		}

		if (xi != -1 && xj != -1 && xj != xi)
		{
			//纵左
			for (int lX = xi; lX <= xj; ++lX)
			{
				int index = beamData.m_lineData.vLinesIndices()[lX];
				Line temp_line = beamData.m_lineData.getLine(index);
				double temp_length = abs(beamData.m_lineData.getLine(index).s.x - textPoint.x);
				if (beamData.m_lineData.getLine(index).s.x < textPoint.x)
				{
					if (textPoint.y > temp_line.s.y&& textPoint.y < temp_line.e.y)
					{
						if (leftLength == 0)
						{
							leftLength = temp_length;
							leftIndex = index;
						}
						else if (leftLength > temp_length)
						{
							leftLength = temp_length;
							leftIndex = index;
						}
					}
				}
				else
				{
					break;
				}
			}
			//纵右
			for (int rX = xj; rX >= xi; --rX)
			{
				int index = beamData.m_lineData.vLinesIndices()[rX];
				Line temp_line = beamData.m_lineData.getLine(index);
				double temp_length = abs(beamData.m_lineData.getLine(index).s.x - textPoint.x);
				if (beamData.m_lineData.getLine(index).s.x > textPoint.x)
				{
					if (textPoint.y > temp_line.s.y&& textPoint.y < temp_line.e.y)
					{
						if (reghtLength == 0)
						{
							reghtLength = temp_length;
							reghtIndex = index;
						}
						else if (reghtLength > temp_length)
						{
							reghtLength = temp_length;
							reghtIndex = index;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			b_xSame = true;
		}
		if ((((bottomLength + topLength) < 2 * averageWidth &&
			(leftLength + reghtLength) < 2 * averageWidth) ||
			  ((bottomLength + topLength) > 2 * averageWidth &&
			 (leftLength + reghtLength) > 2 * averageWidth)) &&
			b_ySame&& b_xSame)
		{
			;
		}
		else if ((bottomLength + topLength) < 2 * averageWidth && !b_ySame && (bottomIndex != -1 || topIndex != -1))
		{
			int borderGropsIndex = -1;
			bool b_inBorder = bBorderGrop(borderGrops, std::pair<int, int>(bottomIndex, topIndex), borderGropsIndex);

			//borderGrops[borderGropsIndex].m_blead = true;
			if (borderGropsIndex != -1)
			{
				conditationBorder.push_back(borderGropsIndex);
				//borderGrops[borderGropsIndex].m_leadGropsIndex.push_back(std::pair<int, bool>(it, true));
				borderGrops[borderGropsIndex].bBeamTextPoint = true;
				borderGrops[borderGropsIndex].beamTextPointVec.push_back(it);
				//textPoints[it].bLeadLine = true;

			}


		}
		else if ((leftLength + reghtLength) < 2 * averageWidth && !b_xSame && (leftIndex != -1 || reghtIndex != -1))
		{
			int borderGropsIndex = -1;
			bool b_inBorder = bBorderGrop(borderGrops, std::pair<int, int>(leftIndex, reghtIndex), borderGropsIndex);

			if (borderGropsIndex != -1)
			{
				conditationBorder.push_back(borderGropsIndex);
				//borderGrops[borderGropsIndex].m_leadGropsIndex.push_back(std::pair<int, bool>(it, true));
				borderGrops[borderGropsIndex].bBeamTextPoint = true;
				borderGrops[borderGropsIndex].beamTextPointVec.push_back(it);
				//textPoints[it].bLeadLine = true;

			}
		}

	}
	return true;
}

bool BeamSet::returnBeamSpanNumber(const std::string& beam_name, bool& b_axis, int& number)
{
	bool b_brackets = false;
	//添加查找括号函数 判断是否找到

	if (b_brackets)
	{
		//查看是否包含轴字段的文本，有使b_axis=true,否则b_axis=false;
		//分析括号内的文本信息，返回梁跨数

		return true;
	}
	else
	{
		//返回false表示当前文本没有具体说明此梁有多少跨 即从当前算起，碰到约束条件为止
		return false;
	}
}

bool BeamSet::returnLocationRange(const std::string& beam_name, std::string& begin, std::string& end)
{
	//根据梁号的命名特点取出“轴”符号前的“~”符号的前后两个数字或字母符符号的梁的位置范围
	//横梁从左到右为起始结束
	//纵梁由下到上为起始结束
	return true;
}

bool BeamSet::setPillarsRtree()
{
	//定义柱子的Rtree用于快速搜索
	for (int i = 0; i < spPillarVec.size(); ++i)

	{
		double min[2]{ 0 };
		double max[2]{ 0 };
		min[0] = spPillarVec[i]->getBox().bottom;
		min[1] = spPillarVec[i]->getBox().left;
		max[0] = spPillarVec[i]->getBox().top;
		max[1] = spPillarVec[i]->getBox().right;
		columnRTree.insertSLines(min, max, i);
	}
	return false;
}

bool BeamSet::returnPointInAxisRegion(const Point& point, const Axis& axise,
									  std::vector<Axis::AxisLine>& point_mark)
{
	std::vector<Axis::AxisLine>temp_PointMark(4);
	findSmallHAxis(point, axise, temp_PointMark[0]);
	findBigHAxis(point, axise, temp_PointMark[1]);
	findSmallVAxis(point, axise, temp_PointMark[2]);
	findBigVAxis(point, axise, temp_PointMark[3]);
	point_mark = temp_PointMark;
	return true;
}


bool BeamSet::findCrossLinesExceptItself(std::set<int>& _crossLineSet,
										 std::vector<int>& fromLineVec,
										 const std::vector<int>& excludeLineVec,
										 int targetIdx)
{
	int idx = -1;
	auto cornerVecPair = beamCornerData.corners().find(targetIdx);
	if (cornerVecPair != beamCornerData.corners().end())
	{
		//遍历交点
		for (const auto& cornerItm : cornerVecPair->second)
		{
			//过滤自身
			idx = (cornerItm.l2 == targetIdx) ? cornerItm.l1 : cornerItm.l2;
			//过滤需要排除的线
			if (std::find(excludeLineVec.begin(), excludeLineVec.end(), idx) == excludeLineVec.end() &&
				std::find(fromLineVec.begin(), fromLineVec.end(), idx) != fromLineVec.end())
				_crossLineSet.insert(idx);
		}
	}
	if (_crossLineSet.empty())
		return false;
	else
		return true;
}

bool BeamSet::matchPairIdx(int& _pairIdx, std::vector<int>& fromLineVec, std::vector<int>& excludeVec, int targetIdx)
{
	//配对
	Line line{};
	line = beamLines.getLine(targetIdx);
	double minDist = line.length() * 9, tpDist;//线的长度
	Point p1 = Point((line.s.x + line.e.x) / 2, (line.s.y + line.e.y) / 2);
	int pairIdx = -1;
	//找匹配的对边线
	for (auto borderIdx : fromLineVec)
	{
		//非自身和排除的，则求中点间距离
		if (borderIdx != targetIdx &&
			std::find(excludeVec.begin(), excludeVec.end(), borderIdx) == excludeVec.end())
		{
			line = beamLines.getLine(borderIdx);
			tpDist = p1.distance(Point((line.s.x + line.e.x) / 2, (line.s.y + line.e.y) / 2));
			if (tpDist < minDist)
			{
				minDist = tpDist;
				pairIdx = borderIdx;
			}
		}
	}
	if (pairIdx == -1)
		return false;
	_pairIdx = pairIdx;
	return true;
}

bool BeamSet::findBeamBorders(std::queue<std::pair<int, int>>& _queue,
							  std::vector<int>& fromLineVec,
							  std::vector<int>& excludeVec,
							  int targetIdx)
{
	std::set<int> crossLineSet;
	//查找与边线相接的border
	findCrossLinesExceptItself(crossLineSet, fromLineVec, excludeVec, targetIdx);

	for (auto crossBorderIdxItm : crossLineSet)
	{
		if (std::find(excludeVec.begin(), excludeVec.end(), crossBorderIdxItm) == excludeVec.end())
		{
			int pairIdx = -1;
			if (matchPairIdx(pairIdx, fromLineVec, excludeVec, crossBorderIdxItm))
			{
				_queue.push(std::make_pair(crossBorderIdxItm, pairIdx));
			}

		}
	}

	if (_queue.empty())
		return false;
	return true;
}

bool BeamSet::excludeLeadIdx(int boundIdx, std::vector<int>& fromLineVec, std::vector<std::pair<int, bool>>& allLeadIdxVec)
{
	if (allLeadIdxVec.empty())
		return false;
	std::set<int> crossLineSet;
	if (!findCrossLinesExceptItself(crossLineSet, /*std::vector<int>{}*/fromLineVec, std::vector<int>{}, boundIdx))
		return false;

	for (auto cross : crossLineSet)
	{
		//与给定的引线集合有相交
		for (auto it : allLeadIdxVec)
		{
			auto& lead = leadGroupVec[it.first];
			if (lead.cornerIndex.front().second == cross)
			{
				return true;
				//TODO 梁信息匹配
			}
		}

	}
	return false;
}



bool BeamSet::extractTextAnnotationInfo(std::shared_ptr<Beam> _spBeam, Data& data, int leadIdx)
{
	//根据离梁文本最近的引线中的线查找周围的其他文本
	//取引线交线
	Lead tpLead = leadGroupVec[leadIdx];
	// todo 确定是离文本点最近的线
#if 1
	int closestIdx = -1;
	getClosestLineIdxFrom(closestIdx, tpLead.nowIndexVec, beamTextPointVec.at(tpLead.beamTextPointIndex).pt);
	Line line = beamLines.getLine(closestIdx);
#endif
	/*Line line = beamLines.lines().at(tpLead.nowIndexVec.front());*/
	spdlog::get("all")->info(
		"assembly lead line start pt({},{}),end pt({},{})",
		std::to_string(line.s.x), std::to_string(line.s.y),
		std::to_string(line.e.x), std::to_string(line.e.y));

	// 先以引线中点为圆心引线的一半长为半径找文本点
	Point midPt((line.s.x + line.e.x) / 2, (line.s.y + line.e.y) / 2);
	this->searchText(_spBeam, data, midPt, line.s.distance(midPt));
	//再根据以文字高度为半径在引线的两边查找可能遗漏的文本点
	//需要找到至少一个文本点确定半径范围
	if (_spBeam->refHeight > 0.0)
	{
		//开始端
		this->searchText(_spBeam, data, line.s, _spBeam->refHeight * (1 + 0.25));
		//结束端
		this->searchText(_spBeam, data, line.e, _spBeam->refHeight * (1 + 0.25));

		int jinCount = 0;
		std::string text;
		for (auto textIdx : _spBeam->textPtIdxSet)
		{
			auto tp = data.m_textPointData.textpoints().at(textIdx);
			auto t = std::dynamic_pointer_cast<DRW_Text>(tp.entity);
			std::string strText = t->text;
			text += strText;
			bool mark = _spBeam->extractBeamInfo2(strText);
			if (mark)jinCount++;
			mark = _spBeam->extractBeamInfo3(strText);
			if (mark)jinCount++;
			mark = _spBeam->extractBeamInfo4(strText);
			if (mark)jinCount++;
		}

		if (jinCount >= 2)_spBeam->bAssemblyAnnotation = true;
		else _spBeam->bAssemblyAnnotation = false;
		return true;
	}
	else
	{
		auto tp = beamTextPointVec.at(tpLead.beamTextPointIndex).pt;
		auto t = std::dynamic_pointer_cast<DRW_Text>(tp.entity);
		spdlog::get("all")->info("text:{}; angle:{};height:{}", t->text, t->angle, _spBeam->refHeight = t->height);
		//解析当前文本
		_spBeam->extractBeamInfo(t->text);
		//加入该梁所拥有的集中标注的文本点
		_spBeam->textPtIdxSet.insert(tpLead.beamTextPointIndex);
	}
	//zxzxzx
	int jinCount = 0;
	std::string text;
	if (_spBeam->strBeamName == "");
	for (auto textIdx : _spBeam->textPtIdxSet)
	{
		auto tp = data.m_textPointData.textpoints().at(textIdx);
		auto t = std::dynamic_pointer_cast<DRW_Text>(tp.entity);
		std::string strText = t->text;
		text += strText;
		bool mark = _spBeam->extractBeamInfo2(strText);
		if (mark)jinCount++;
		mark = _spBeam->extractBeamInfo3(strText);
		if (mark)jinCount++;
		mark = _spBeam->extractBeamInfo4(strText);
		if (mark)jinCount++;
	}
	if (jinCount >= 2)_spBeam->bAssemblyAnnotation = true;
	else _spBeam->bAssemblyAnnotation = false;
	//zxzxzx
	spdlog::get("all")->error("can not determine the size of radius on the sides of line");
	return false;
}



bool BeamSet::getClosestLineIdxFrom(int& _idx, const std::vector<int>& fromLineVec, const Point& pt)
{
	double refH = std::dynamic_pointer_cast<DRW_Text>(pt.entity)->height;
	auto getDis = [&refH](const Line& line, const Point& p) ->double
	{
		double tpCoordinate;
		tpCoordinate = knowYToX(line, p.y);
		if (std::isnan(tpCoordinate))
			if (std::min(line.s.x, line.e.x) > p.x || std::max(line.s.x, line.e.x) < p.x)
				return refH;
			else
				return std::abs(p.y - line.s.y);
		Point p1(tpCoordinate, p.y);
		tpCoordinate = knowXToY(line, p.x);
		if (isnan(tpCoordinate))
			if (std::min(line.s.y, line.e.y) > p.y || std::max(line.s.y, line.e.y) < p.y)
				return refH;
			else
				return std::abs(p.x - line.s.x);
		Point p2(p.x, tpCoordinate);
		double side1 = p.distance(p1);
		double side2 = p.distance(p2);
		double side3 = p1.distance(p2);
		return side1 * side2 / side3;
	};
	_idx = fromLineVec.front();
	double min = getDis(beamLines.lines().at(_idx), pt);
	for (auto i = 1; i < fromLineVec.size(); i++)
	{
		double tp = getDis(beamLines.lines().at(i), pt);
		if (min > tp)
		{
			min = tp;
			_idx = i;
		}
	}
	return true;
}

bool BeamSet::searchText(std::shared_ptr<Beam> _spBeam, Data& data, Point& pt, double radius)
{
	std::vector<int> textPointIdxVec = data.m_kdtTreeData.kdtTexts().radiusSearch(pt, radius);
	if (!textPointIdxVec.empty())
	{
		spdlog::get("all")->info("found texts count:{} texts", textPointIdxVec.size());
		for (auto textIdx : textPointIdxVec)
		{
			auto tp = data.m_textPointData.textpoints().at(textIdx);
			auto t = std::dynamic_pointer_cast<DRW_Text>(tp.entity);
			spdlog::get("all")->info("text:{}; angle:{};height:{}", t->text, t->angle, _spBeam->refHeight = t->height);
			//解析当前文本
			_spBeam->extractBeamInfo(t->text);
			//加入该梁所拥有的集中标注的文本点
			_spBeam->textPtIdxSet.insert(textIdx);
		}
		return true;
	}
	spdlog::get("all")->error("not found any text points on the middle position of lead info line");
	return false;
}

bool BeamSet::checkDirectionLines(Beam::Direction& d, const std::vector<int>& lineIdxVec)
{
	if (lineIdxVec.empty())
		return false;

#if 1
	unsigned int hCount = 0, vCount = 0, sCount = 0;
	for (auto lineIdx : lineIdxVec)
	{
		const auto& l = beamLines.lines().at(lineIdx);
		if (l.horizontal())
			hCount++;
		else if (l.vertical())
			vCount++;
		else
			sCount++;
	}
	unsigned int maxCount = hCount;
	maxCount = std::max(maxCount, vCount);
	maxCount = std::max(maxCount, sCount);

	if (maxCount == hCount)
		d = Beam::Direction::H;
	else if (maxCount == vCount)
		d = Beam::Direction::V;
	else
		d = Beam::Direction::S;
#endif
	return true;
}

bool BeamSet::iniColumnRtree()
{
	for (auto i = 0; i < spPillarVec.size(); ++i)
	{
		Line temp_line = Line(Point(spPillarVec[i]->getBox().left, spPillarVec[i]->getBox().top),
							  Point(spPillarVec[i]->getBox().right, spPillarVec[i]->getBox().bottom));
		pushRTreeLines(temp_line, i, columnRTree);
	}
	return true;
}

std::vector<int> BeamSet::returnRtreeColumnIndex(const double min[], const double max[])
{
	int num = columnRTree.sLines().Search(min, max, std::bind(&RTreeData::pushBackSBackIndex,
															  &columnRTree, std::placeholders::_1));
	auto backIndex = columnRTree.SBackIndex();
	columnRTree.clearSBackIndex();
	return backIndex;
}

bool BeamSet::updateMinMax(double* min, double* max, const Line line)
{
	double minX = std::min(line.s.x, line.e.x);
	double minY = std::min(line.s.y, line.e.y);
	double maxX = std::max(line.s.x, line.e.x);
	double maxY = std::max(line.s.y, line.e.y);
	if (min[0] == -1)
	{
		min[0] = minX;
		min[1] = minY;
		max[0] = maxX;
		max[1] = maxY;
		return true;
	}
	else
	{
		min[0] = std::min(min[0], minX);
		min[1] = std::min(min[1], minY);
		max[0] = std::max(max[0], maxX);
		max[1] = std::max(max[1], maxY);
	}
	return true;
}

bool BeamSet::updateColChain(std::shared_ptr<Beam> spBeam, const Line& line)
{
	double min[2], max[2];
	min[0] = std::min(line.s.x, line.e.x) - spBeam->refHeight;
	min[1] = std::min(line.s.y, line.e.y) - spBeam->refHeight;
	max[0] = std::max(line.s.x, line.e.x) + spBeam->refHeight;
	max[1] = std::max(line.s.y, line.e.y) + spBeam->refHeight;
	std::vector<int> tpColIdxVec = returnRtreeColumnIndex(min, max);
	std::for_each(tpColIdxVec.begin(), tpColIdxVec.end(), [&tpColIdxVec, &spBeam](int idx)
	{
		if (std::find(spBeam->colChain.begin(), spBeam->colChain.end(), idx) == spBeam->colChain.end())
			spBeam->colChain.push_back(idx);
	});
	return true;
}

bool BeamSet::parseSpan(std::shared_ptr<Beam> _spBeam, Data& data)
{
	auto printPillarInfo = [this](std::vector<int> colIdxVec)
	{
		for (auto colIdx : colIdxVec)
		{
			auto& col = this->spPillarVec[colIdx];
			spdlog::get("all")->info("column idx:{},horizontal axis:{},vertical axis:{}",
									 colIdx,
									 col->getHAxis().front().first,
									 col->getVAxis().front().first);
		}
	};

	//得到确切的宽度和方向
	_spBeam->updateDirectionWidth();

	// 按照某个方向排序柱索引
	auto sortRef = [this](int idx1, int idx2, Beam::Direction d)
	{
		if (d == Beam::Direction::H)
		{
			if (this->spPillarVec.at(idx1)->getBox().bottom <
				this->spPillarVec.at(idx2)->getBox().bottom)
				return true;
			else
				return false;
		}
		else if (d == Beam::Direction::V)
		{
			if (this->spPillarVec.at(idx1)->getBox().left <
				this->spPillarVec.at(idx2)->getBox().left)
				return true;
			else
				return false;
		}
		else
		{
			if (this->spPillarVec.at(idx1)->getBox().left <
				this->spPillarVec.at(idx2)->getBox().left)
				return true;
			else
				return false;
		}
	};

	//排序柱
	//横向梁，柱从左往右
	if (_spBeam->direction == Beam::Direction::H || _spBeam->direction == Beam::Direction::S)
	{
		std::sort(_spBeam->colChain.begin(),
				  _spBeam->colChain.end(),
				  std::bind(sortRef,
							std::placeholders::_1,
							std::placeholders::_2,
							Beam::Direction::V));
		printPillarInfo(_spBeam->colChain);
	}
	//竖直梁，柱由下往上
	else if (_spBeam->direction == Beam::Direction::V)
	{
		std::sort(_spBeam->colChain.begin(),
				  _spBeam->colChain.end(),
				  std::bind(sortRef,
							std::placeholders::_1,
							std::placeholders::_2,
							Beam::Direction::H));
		printPillarInfo(_spBeam->colChain);
	}


	std::shared_ptr<BeamSpan> span;
	// 柱支撑
	for (auto it = 1; it < _spBeam->colChain.size(); it++)
	{
		span = std::make_shared<BeamSpan>();

		//跨id
		span->spanId = it - 1;
		auto prIt = it - 1;
		const auto& col1 = this->spPillarVec.at(_spBeam->colChain.at(prIt));
		const auto& col2 = this->spPillarVec.at(_spBeam->colChain.at(it));
		//设置支撑柱的索引
		span->supportPair.first = _spBeam->colChain.at(prIt);
		span->supportPair.second = _spBeam->colChain.at(it);

		span->searchBorders(col2, col1, _spBeam);
		span->updateWithAndDirection();
		span->updateNetLength(col2, col1);
		span->updateSpanMidlinePosition(col1, col2, spAxis, *pIdxVec, *pLineVec);

		//搜索是否在柱的两边有额外的标注（优先选取原位标注）
		std::set<int> priorTextPtIdxSet;
		//左边
		span->searchAnnotationPriority(priorTextPtIdxSet, _spBeam, col1->getBox(), data, BeamSpan::SpanPosition::Left);
		//右边
		span->searchAnnotationPriority(priorTextPtIdxSet, _spBeam, col2->getBox(), data, BeamSpan::SpanPosition::Right);
		//中部
		span->searchMidPriority(priorTextPtIdxSet, _spBeam, col1->getBox(), col2->getBox(), data);

		_spBeam->spSpanVec.push_back(span);
		//_spBeam->appendSpan(span);
	}


	//两端都没有柱
	if (_spBeam->colChain.empty())
	{
		span = std::make_shared<BeamSpan>();
		span->spanId = 0;
		double min[2]{ -1,-1 }, max[2]{ -1,-1 };
		for (const auto& line : _spBeam->borderLineVec)
			updateMinMax(min, max, line);


		if (_spBeam->borderLineVec.size() == 2)
		{
			bool bAck = false;
			auto& l1 = _spBeam->borderLineVec.at(0);
			auto& l2 = _spBeam->borderLineVec.at(1);

			//trim 2 lines
			if (l1.horizontal() && l2.horizontal())
			{
				Point s((l1.s.x + l2.s.x) / 2, (l1.s.y + l2.s.y) / 2);
				Point e((l1.e.x + l2.e.x) / 2, (l1.e.y + l2.e.y) / 2);

				span->midlineStart.pt = s;
				findClosestHAxis(span->midlineStart.relativeHDistance, spAxis->hlines, s.y);
				findClosestVAxis(span->midlineStart.relativeVDistance, spAxis->vlines, s.x);
				span->midlineEnd.pt = e;
				findClosestHAxis(span->midlineEnd.relativeHDistance, spAxis->hlines, e.y);
				findClosestVAxis(span->midlineEnd.relativeVDistance, spAxis->vlines, e.x);
				bAck = true;
			}
			else if (l1.vertical() && l2.vertical())
			{
				Point s((l1.s.x + l2.s.x) / 2, (l1.s.y + l2.s.y) / 2);
				Point e((l1.e.x + l2.e.x) / 2, (l1.e.y + l2.e.y) / 2);

				span->midlineStart.pt = s;
				findClosestHAxis(span->midlineStart.relativeHDistance, spAxis->hlines, s.y);
				findClosestVAxis(span->midlineStart.relativeVDistance, spAxis->vlines, s.x);
				span->midlineEnd.pt = e;
				findClosestHAxis(span->midlineEnd.relativeHDistance, spAxis->hlines, e.y);
				findClosestVAxis(span->midlineEnd.relativeVDistance, spAxis->vlines, e.x);
				bAck = true;
			}
			/*if (bAck)
				pLineVec->push_back(Line(span->midlineStart.pt, span->midlineEnd.pt));*/
		}
		else
		{
			spdlog::get("all")->error("invalid border lines in the no col beam of one lead");
		}
		//只有一跨
		if (_spBeam->direction == Beam::Direction::H)
			span->spanNetLength = max[0] - min[0];
		else
			span->spanNetLength = max[1] - min[1];

		span->left.supportType = BeamPosition::SupportType::Beam;
		span->right.supportType = BeamPosition::SupportType::Beam;
		_spBeam->spSpanVec.push_back(span);
		//_spBeam->appendSpan(span);
	}

	// 只找到一个柱，可能为错
	else if (_spBeam->colChain.size() <= 1)
	{
		spdlog::get("all")->error("has an error on col idx size of pillar");
		return false;
	}

	//计算跨数
	_spBeam->spanNum = _spBeam->spSpanVec.size();
	return true;
}

bool BeamSet::parseBeamInfo(std::shared_ptr<Beam> _spBeam, Data& data, int leadIdx)
{
	_spBeam->leadIdx = leadIdx;
	//!尽可能得到该梁的标注信息
	extractTextAnnotationInfo(_spBeam, data, leadIdx);
	return true;
};

bool BeamSet::findSmallHAxis(const Point& point, const Axis& axis,
							 Axis::AxisLine& haxis_mark)
{
	auto hlines_coo = axis.hlines_coo;
	double refer_x = axis.referenceAxisX.front().second;
	double refer_y = axis.referenceAxisY.front().second;
	double relatively_x = point.x - refer_y;
	double relatively_y = point.y - refer_x;

	int mid = 0, low = 0, high = axis.hlines_coo.size() - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;

		if (hlines_coo[mid].second < relatively_y)
		{
			int temp_low = mid + 1;
			if (temp_low == hlines_coo.size() || hlines_coo[temp_low].second >= relatively_y)
			{
				haxis_mark = hlines_coo[mid].first;
				return true;
			}
			else
			{
				low = temp_low;
			}

		}
		if (hlines_coo[mid].second > relatively_y)
		{
			high = mid - 1;
		}
	}
	return true;
}

bool BeamSet::findBigHAxis(const Point& point, const Axis& axis,
						   Axis::AxisLine& haxis_mark)
{
	auto hlines_coo = axis.hlines_coo;
	double refer_x = axis.referenceAxisX.front().second;
	double refer_y = axis.referenceAxisY.front().second;
	double relatively_x = point.x - refer_y;
	double relatively_y = point.y - refer_x;

	int mid = 0, low = 0, high = axis.hlines_coo.size() - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;

		if (hlines_coo[mid].second < relatively_y)
		{
			low = mid + 1;
		}
		if (hlines_coo[mid].second > relatively_y)
		{
			double temp_high = mid - 1;
			if (temp_high < 0 || hlines_coo[temp_high].second <= relatively_y)
			{
				haxis_mark = hlines_coo[mid].first;
				return true;
			}
			else
			{
				high = mid - 1;
			}
		}
	}
	return true;
}

bool BeamSet::findSmallVAxis(const Point& point, const Axis& axis,
							 Axis::AxisLine& vaxis_mark)
{
	auto vlines_coo = axis.vlines_coo;
	double refer_x = axis.referenceAxisX.front().second;
	double refer_y = axis.referenceAxisY.front().second;
	double relatively_x = point.x - refer_y;
	double relatively_y = point.y - refer_x;

	int mid = 0, low = 0, high = axis.vlines_coo.size() - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;

		if (vlines_coo[mid].second < relatively_x)
		{
			int temp_low = mid + 1;
			if (temp_low == vlines_coo.size() || vlines_coo[temp_low].second >= relatively_x)
			{
				vaxis_mark = vlines_coo[mid].first;
				return true;
			}
			else
			{
				low = temp_low;
			}

		}
		if (vlines_coo[mid].second > relatively_x)
		{
			high = mid - 1;
		}
	}
	return true;
}

bool BeamSet::findBigVAxis(const Point& point, const Axis& axis,
						   Axis::AxisLine& vaxis_mark)
{
	auto vlines_coo = axis.vlines_coo;
	double refer_x = axis.referenceAxisX.front().second;
	double refer_y = axis.referenceAxisY.front().second;
	double relatively_x = point.x - refer_y;
	double relatively_y = point.y - refer_x;

	int mid = 0, low = 0, high = axis.vlines_coo.size() - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;

		if (vlines_coo[mid].second < relatively_x)
		{
			int temp_low = mid + 1;

			low = temp_low;
		}
		if (vlines_coo[mid].second > relatively_x)
		{
			double temp_high = mid - 1;
			if (temp_high < 0 || vlines_coo[temp_high].second <= relatively_x)
			{
				vaxis_mark = vlines_coo[mid].first;
				return true;
			}
			else
			{
				high = mid - 1;
			}
		}
	}
	return true;
}

void BeamSet::setMFCPaintTestVec(std::vector<int>& testIdxVec, std::vector<Line>& testLineVec)
{
	this->pIdxVec = &testIdxVec;
	this->pLineVec = &testLineVec;
}


bool operator==(const Lead& leadGrop1, const Lead& leadGrop2)
{
	//bool m_mark = true;
	if (leadGrop1.cornerIndex != leadGrop2.cornerIndex &&
		leadGrop1.bPair != leadGrop2.bPair &&
		leadGrop1.bSide != leadGrop2.bSide &&
		leadGrop1.nowIndexVec.size() != leadGrop2.nowIndexVec.size() &&
		leadGrop1.primevalIndexVec.size() != leadGrop2.primevalIndexVec.size() &&
		leadGrop1.insertPointVec.size() != leadGrop2.insertPointVec.size())
	{
		return false;
	}


	for (int i = 0; i < leadGrop2.nowIndexVec.size(); ++i)
	{
		if (leadGrop1.nowIndexVec[i] != leadGrop2.nowIndexVec[i])
			return false;
	}
	for (int i = 0; i < leadGrop2.primevalIndexVec.size(); ++i)
	{
		if (leadGrop1.primevalIndexVec[i] != leadGrop2.primevalIndexVec[i])
		{
			return false;
		}
	}
	for (int i = 0; i < leadGrop2.insertPointVec.size(); ++i)
	{
		if (leadGrop1.insertPointVec[i] != leadGrop2.insertPointVec[i])
		{
			return false;
		}
	}

	return true;

}



