#include "beamLongitudinal.h"


bool BeamLongitudinal::parseBlock(std::shared_ptr<Block> _spBlock,
								  std::string& _strReinLayer,
								  std::shared_ptr<Data> data,
								  std::vector<unsigned int>& _blockDimIdxVec,
								  std::vector<int>& pTestVec,
								  std::vector<Line>& pTestLineVec)
{
	initReqiuredInfo(_spBlock, _strReinLayer, pTestVec, pTestLineVec);
	// 初始化标注
	initDimMap(_blockDimIdxVec, data);
	// 初始化参考线
	initFirstSupportRef(data);

	std::vector<int> hLineIdxVec, vLineIdxVec, sLineIdxVec;
	// 得到所有的加固线
	findRein(hLineIdxVec, data);
	if (hLineIdxVec.empty())
		return false;
	parseRein(hLineIdxVec, data);

	//得到剖线
	findProfile(*data);
	return true;
}

bool BeamLongitudinal::initReqiuredInfo(std::shared_ptr<Block> _block, std::string& _strReinLayer, std::vector<int>& pTestVec, std::vector<Line>& pTestLinesVec)
{
	this->spBlock = _block;
	this->strReinLayer = _strReinLayer;
	this->pTestIdxVec = &pTestVec;
	this->pTestLineVec = &pTestLinesVec;
	return true;
}

void BeamLongitudinal::initDimMap(std::vector<unsigned int>& _blockDimIdxVec,
								  std::shared_ptr<Data> data)
{

	for (auto& dimIdxItm : _blockDimIdxVec)
	{
		const auto& dim = data->m_dimensions.at(dimIdxItm);
		const auto& refLine = dim->lines.at(2);
		//水平标注端点的x映射
		if (refLine.horizontal())
		{
			auto tickS = pseudoDouble(refLine.s.x);
			auto tickE = pseudoDouble(refLine.e.x);
			//右增
			if (tickS < tickE)
			{
				this->hDimDesMap[tickS].emplace_back(dimIdxItm, true);
				this->hDimDesMap[tickE].emplace_back(dimIdxItm, false);
			}
			//左增
			else
			{
				this->hDimDesMap[tickS].emplace_back(dimIdxItm, false);
				this->hDimDesMap[tickE].emplace_back(dimIdxItm, true);
			}
		}
		//竖直标注y轴映射
		else if (refLine.vertical())
		{
			auto tickS = pseudoDouble(refLine.s.y);
			auto tickE = pseudoDouble(refLine.e.y);
			//上增
			if (tickS < tickE)
			{
				this->vDimDesMap[tickS].emplace_back(dimIdxItm, true);
				this->vDimDesMap[tickE].emplace_back(dimIdxItm, false);
			}
			//左增
			else
			{
				this->vDimDesMap[tickS].emplace_back(dimIdxItm, false);
				this->vDimDesMap[tickE].emplace_back(dimIdxItm, true);
			}
		}
		//斜的标注
		else
		{
			//? there are no decline dimensions?
			Line tpLine(Point(dim->box.left, dim->box.top), Point(dim->box.right, dim->box.bottom));
			pushRTreeLines(tpLine, dimIdxItm, sDimRTree);
		}
		//标注的长度映射
		auto lenTick = pseudoDouble(refLine.length());
		lenDimDesMap[lenTick].push_back(dimIdxItm);
	}
}


bool BeamLongitudinal::findRein(std::vector<int>& _hLineIdxVec,
								const std::shared_ptr<Data>& data)
{
	//std::vector<int> reinLineIdxVec;
	//处理水平线
	for (auto i = spBlock->box.hf; i <= spBlock->box.ht; ++i)
	{
		int idx = data->m_lineData.hLinesIndices()[i];
		auto& line = data->m_lineData.lines()[idx];
		if (Line::Type::DIMENSIONS != line.type &&
			line.getLayer() == strReinLayer &&
			line.s.x >= spBlock->box.left &&
			line.e.x <= spBlock->box.right)
		{
			reinLineIdxVec.push_back(idx);
			_hLineIdxVec.push_back(idx);
		}
	}
	//垂直线
	for (auto i = this->spBlock->box.vf; i <= this->spBlock->box.vt; ++i)
	{
		int idx = data->m_lineData.vLinesIndices()[i];
		auto& line = data->m_lineData.lines()[idx];
		if (Line::Type::DIMENSIONS != line.type &&
			line.getLayer() == strReinLayer &&
			line.s.y >= spBlock->box.bottom &&
			line.e.y <= spBlock->box.top)
		{
			reinLineIdxVec.push_back(idx);
		}
	}

	//斜线
	double min[2]{ this->spBlock->box.left,this->spBlock->box.bottom };
	double max[2]{ this->spBlock->box.right,this->spBlock->box.top };
	auto num = data->m_rtreeData.sLines().Search(min, max,
												 std::bind(&RTreeData::pushBackSBackIndex, &data->m_rtreeData, std::placeholders::_1));
	for (auto itm : data->m_rtreeData.SBackIndex())
	{
		const auto& line = data->m_lineData.lines()[itm];
		if (Line::Type::DIMENSIONS != line.type && line.getLayer() == strReinLayer)
		{
			reinLineIdxVec.push_back(itm);
		}
	}
	data->m_rtreeData.clearSBackIndex();

	//构建加固
	// <y伪坐标，data中线的索引>
	std::map<long long, std::vector<unsigned>> yTickMap;
	std::shared_ptr<BeamLongitudinalRein> rein;
	std::shared_ptr<BeamLongitudinalRein::ReinCompnent> comp;

	//加固线是以横线为标志，故只遍历横线
	for (auto idx : _hLineIdxVec)
	{
		const auto& hLine = data->m_lineData.lines().at(idx);
		int reinIdx = static_cast<int>(this->spReinVec.size());
		rein = std::make_shared<BeamLongitudinalRein>();
		rein->next = -1;
		rein->pre = -1;
		this->reinLineIdxToReinIdxMap[idx] = reinIdx;

		comp = std::make_shared<BeamLongitudinalRein::ReinCompnent>();
		comp->lineIdx = idx;
		comp->reinIdx = reinIdx;
		comp->compnentType = BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Longitudinal;
		rein->addComp(data, comp);

		auto yTick = pseudoDouble(hLine.s.y);
		yTickMap[yTick].push_back(reinIdx);
		//分析水平线的交线，进行分类
		std::vector<int> emptyVec{};
		data->m_cornerData.findCross(reinLineIdxVec, emptyVec, comp->lineIdx, [this, &rein, &data, &hLine](int crossIdx, const Point& pt)
		{

			const auto& crossLine = data->m_lineData.lines().at(crossIdx);
			//交线是相接的
			if (bConnectTowLine(crossLine, hLine))
			{
				//相接的垂线
				if (crossLine.vertical())
				{
					auto vComp(std::make_shared<BeamLongitudinalRein::ReinCompnent>());
					vComp->lineIdx = crossIdx;
					vComp->reinIdx = rein->components.front()->reinIdx;
					rein->addComp(data, vComp);
					return true;
				}
				//相接的斜线
				else if (!crossLine.vertical())
				{
					auto sComp(std::make_shared<BeamLongitudinalRein::ReinCompnent>());
					sComp->lineIdx = crossIdx;
					sComp->reinIdx = rein->components.front()->reinIdx;
					rein->addComp(data, sComp);
					return true;
				}
				spdlog::get("all")->error("[{}][{}]it has horizontal line", __FUNCTION__, __LINE__);
				//不存在平行线的交点
				abort();//不应该运行到这
				return true;
			}
			//不是相接的
			else
			{
				//确定边界加固和内部加固
				if (crossLine.vertical())
				{
					//T 相交方式
					// todo 利用线宽进行调整
					double fixedRangeWidth = 1;
					if (hLine.entity)
					{
						if (hLine.entity->dwgType == DRW::LWPOLYLINE)
						{
							const auto& pl = std::dynamic_pointer_cast<DRW_LWPolyline>(hLine.entity);
							fixedRangeWidth = pl->width / 2;
						}
					}
					if (abs(hLine.s.y - crossLine.e.y) < fixedRangeWidth || abs(hLine.s.y - crossLine.s.y) < fixedRangeWidth)
					{
						//外边界加固类型
						if (rein->reinType != BeamLongitudinalRein::ReinType::BOUNDARY)
							rein->reinType = BeamLongitudinalRein::ReinType::BOUNDARY;
						//加固的边界上下范围
						if (abs(rein->spanHeight() - 0.0) < 1)
							rein->updateVBound(crossLine);
						else
						{
							if (rein->spanHeight() > crossLine.length())
								rein->updateVBound(crossLine);
						}
					}
					else
					{
						//内加固类型
						if (rein->reinType != BeamLongitudinalRein::ReinType::BOUNDARY)
							rein->reinType = BeamLongitudinalRein::ReinType::INNER;
					}
				}
				return true;
			}
			return false;
		});
		spReinVec.push_back(rein);
	}



	//标记交织的加固线
	for (auto& v : yTickMap)
	{
		auto& condidates = v.second;

		//从左往右排序,左边界为依据
		std::sort(condidates.begin(), condidates.end(), [this, &data](int spReinIdx1, int spReinIdx2)
		{
			const std::shared_ptr<BeamLongitudinalRein>& spRein1 = this->spReinVec.at(spReinIdx1);
			const std::shared_ptr<BeamLongitudinalRein>& spRein2 = this->spReinVec.at(spReinIdx2);
			if (spRein1->left < spRein2->left)
				return true;
			else
				return false;
		});

		for (auto i = 0; i < condidates.size() - 1; i++)
		{
			auto iNext = i + 1;
			auto& spRein1 = this->spReinVec.at(condidates.at(i));
			auto& spRein2 = this->spReinVec.at(condidates.at(iNext));
			auto hl1 = data->m_lineData.lines().at(spRein1->components.front()->lineIdx);
			auto hl2 = data->m_lineData.lines().at(spRein2->components.front()->lineIdx);
			if (spRein1->right > spRein2->left)
			{
				spRein1->next = condidates.at(iNext);
				spRein2->pre = condidates.at(i);
				spRein1->updateHBound(spRein1->left, spRein2->right);
				spRein2->updateHBound(spRein1->left, spRein2->right);
			}
		}
	}


#if 0
	for (auto rein : spReinVec)
	{
		if (rein->reinType == BeamLongitudinalRein::BOUNDARY)
		{
			for (const auto& c : rein->components)
			{
				pTestLineVec->push_back(data->m_lineData.lines().at(c->lineIdx));
			}
		}
	}
#endif // boundary

#if 0
	for (auto rein : spReinVec)
	{
		if (rein->reinType == BeamLongitudinalRein::INNER)
		{
			for (auto h : rein->hLineVec)
			{
				pTestIdxVec->push_back(h);
			}
			for (auto v : rein->vLineVec)
			{
				pTestIdxVec->push_back(v);
			}
			for (auto s : rein->sLineVec)
			{
				pTestIdxVec->push_back(s);
			}
		}
	}
#endif //inner
	return true;
}

bool BeamLongitudinal::parseRein(std::vector<int>& _hLineIdxVec,
								 std::shared_ptr<Data>& data)
{
	double acWith = 0.0;
	int count = 0;
	// 完善内部加固的信息上下边界
	for (auto i = 0; i < this->spReinVec.size(); i++)
	{
		const auto& rein1 = this->spReinVec.at(i);
		if (rein1->reinType == BeamLongitudinalRein::ReinType::BOUNDARY)
		{
			spdlog::get("all")->info("boundary pseudo span height:{}", rein1->spanHeight());
			const auto& l1 = data->m_lineData.lines().at(rein1->components.front()->lineIdx);
			for (auto j = 0; j < this->spReinVec.size(); j++)
			{
				if (i == j)
					continue;
				auto& rein2 = this->spReinVec.at(j);
				if (rein2->reinType == BeamLongitudinalRein::ReinType::INNER)
				{
					const auto& l2 = data->m_lineData.lines().at(rein2->components.front()->lineIdx);
					const double& fixedWidth = rein1->spanHeight() / 4;
					if (rein1->left - fixedWidth <= rein2->left &&
						rein1->right + fixedWidth >= rein2->right &&
						rein1->spanHeight() / 2 > std::abs(l1.s.y - l2.s.y))
					{
						rein2->top = rein1->top;
						rein2->bottom = rein1->bottom;

						spdlog::get("all")->info("inner pseudo span height:{}", rein2->spanHeight());
						const auto& tpLen = std::abs(l1.s.y - l2.s.y);
						spdlog::get("all")->info("gap size:{}", tpLen);
						acWith += tpLen;
						++count;
					}
				}
			}
			//为每个外加固查找混凝土线
			rein1->findOuterLineIdx(data, this->spBlock, pTestLineVec);
		}
	}
	spdlog::get("all")->info("mean gap size :{}", acWith / count);

	//查找加固标注的信息
	for (auto& reinItm : this->spReinVec)
	{
		double height = reinItm->spanHeight();

		auto& firstComp = reinItm->components.front();
		const auto& longitudinalLine = data->m_lineData.lines().at(firstComp->lineIdx);
		firstComp->dims = std::make_shared<std::vector<DimensionPair>>();
		//找增量标注
		findHVDimension(*firstComp->dims, longitudinalLine.s, true, true, data);
		this->findTruncatedDimPair(reinItm, *firstComp->dims, data);
		//找衰减标注
		std::vector<DimensionPair> tp;
		findHVDimension(tp, longitudinalLine.e, false, true, data);
		this->findTruncatedDimPair(reinItm, tp, data);
		firstComp->dims->insert(firstComp->dims->end(), tp.begin(), tp.end());
		//todo 查找斜线标注
#if 0
		for (auto idxItm : reinItm->sLineVec)
		{
			const auto& sLine = data->m_lineData.lines().at(idxItm);
			if (sLine.length() > height / 2)
			{
				//pTestIdxVec->push_back(idxItm);
				const double left = std::min(sLine.s.x, sLine.e.x);
				const double right = std::max(sLine.s.x, sLine.e.x);
				const double bottom = std::min(sLine.s.y, sLine.e.y);
				const double top = std::max(sLine.s.y, sLine.e.y);
				// RTree 搜索标注
				double min[2]{ left,bottom }, max[2]{ right,top };
				const auto& sDimIdxVec = returnSRtreeDimIndex(min, max);
				for (const auto& dimIdxItm : sDimIdxVec)
				{
					const auto& dim = data->m_dimensions.at(dimIdxItm);
					if (dim->box.left < reinItm->left)
					{
						reinItm->sDimPairVec.emplace_back(dimIdxItm, false);
						pTestLineVec->push_back(dim->lines.at(2));
					}
					if (dim->box.right > reinItm->right)
					{
						reinItm->sDimPairVec.emplace_back(dimIdxItm, true);
						pTestLineVec->push_back(dim->lines.at(2));
					}
				}
			}
			else
			{
				//todo 特殊斜线Lae标注
			}
		}
#endif

		//查找两边相接的竖线标注
		for (auto& compItm : reinItm->components)
		{
			if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Left ||
				compItm->compnentType == BeamLongitudinalRein::ReinCompType::Right)
			{
				const auto& verticalLine = data->m_lineData.lines().at(compItm->lineIdx);
				if (verticalLine.vertical())
				{
					compItm->dims = std::make_shared<std::vector<DimensionPair>>();
					//找增量标注
					findHVDimension(*compItm->dims, verticalLine.s, true, false, data);
					//找衰减标注
					findHVDimension(*compItm->dims, verticalLine.e, false, false, data);
				}
			}
		}
	}


	//解析每个外部加固的长度偏差和角度
	for (auto i = 0; i < this->spReinVec.size(); ++i)
	{
		auto& rein = this->spReinVec.at(i);
		if (rein->reinType == BeamLongitudinalRein::BOUNDARY &&
			rein->bExistOtherComponent())
		{
			this->parseZData(data, rein);
			spdlog::get("all")->error("left  z delta :rad [{}],offsetHeight: [{}],Pt({},{})",
									  rein->leftZDelta.rad, rein->leftZDelta.length, rein->leftZDelta.pt.x, rein->leftZDelta.pt.y);
			spdlog::get("all")->error("right z delta :rad [{}],offsetHeight: [{}],Pt({},{})",
									  rein->rightZDelta.rad, rein->rightZDelta.length, rein->rightZDelta.pt.x, rein->rightZDelta.pt.y);
		}

	}
	return true;
}


bool BeamLongitudinal::parseZData(const std::shared_ptr<Data>& data,
								  std::shared_ptr<BeamLongitudinalRein>& rein)
{
	std::function<BeamPosZDelta& (const std::shared_ptr<BeamLongitudinalRein>&)> accessZDelta, accessAnotherZDelta;

	for (auto& compItm : rein->components)
	{
		//查找左侧是否存在偏差
		if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Left)
		{
			accessZDelta = &BeamLongitudinalRein::leftZDelta;
			accessAnotherZDelta = &BeamLongitudinalRein::rightZDelta;
		}
		//查找右侧是否存在偏差
		else if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Right)
		{
			accessZDelta = &BeamLongitudinalRein::rightZDelta;
			accessAnotherZDelta = &BeamLongitudinalRein::leftZDelta;
		}
		else
			continue;

		std::vector<int> excludeVec{ rein->components.front()->lineIdx };
		//查找左右的加固的交线，并返回合法的外加固
		int relativeLineIdx = -1;
		Point crossPt{};
		bool bRet = data->m_cornerData.findCross(reinLineIdxVec, excludeVec, compItm->lineIdx, [this, &relativeLineIdx, &crossPt](int crossIdx, const Point& pt)
		{
			auto reinIdxIt = this->reinLineIdxToReinIdxMap.find(crossIdx);
			if (reinIdxIt == this->reinLineIdxToReinIdxMap.end())
				return false;
			const auto& rein = this->spReinVec.at(reinIdxIt->second);
			if (rein->reinType == BeamLongitudinalRein::BOUNDARY)
			{
				crossPt = pt;
				relativeLineIdx = reinIdxIt->second;
				return true;
			}
			return false;
		});

		if (bRet)
		{
			const auto& crossRein = this->spReinVec.at(relativeLineIdx);

			const auto& crossReinRefLine = data->m_lineData.lines().at(crossRein->components.front()->lineIdx);
			const auto& crossOuterLine = data->m_lineData.lines().at(crossRein->outerIdx);
			const auto& reinRefLine = data->m_lineData.lines().at(rein->components.front()->lineIdx);
			const auto& reinOuterLine = data->m_lineData.lines().at(rein->outerIdx);

			const auto& cliffLine = data->m_lineData.lines().at(compItm->lineIdx);

			double rad = 0.0, rad2 = 0.0;
			rad = std::atan2(cliffLine.s.y - cliffLine.e.y, cliffLine.s.x - cliffLine.e.x);
			rad2 = std::atan2(cliffLine.e.y - cliffLine.s.y, cliffLine.e.x - cliffLine.s.x);
			if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Left)
			{
				rein->leftZDelta.rad = rad;
				rein->leftZDelta.pt = reinOuterLine.s;
				rein->leftZDelta.length = crossOuterLine.s.y - reinOuterLine.s.y;
			}
			else if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Right)
			{
				rein->rightZDelta.rad = rad2;
				rein->rightZDelta.pt = reinOuterLine.e;
				rein->rightZDelta.length = crossOuterLine.s.y - reinOuterLine.s.y;
			}
		}
	}
	return true;
}

std::vector<int> BeamLongitudinal::returnSRtreeDimIndex(const double min[], const double max[])
{
	int num = sDimRTree.sLines().Search(min, max,
										std::bind(&RTreeData::pushBackSBackIndex,
												  &sDimRTree, std::placeholders::_1));
	const auto& backIndex = sDimRTree.SBackIndex();
	sDimRTree.clearSBackIndex();
	return backIndex;
}


bool BeamLongitudinal::findHVDimension(std::vector<DimensionPair>& _dimPairVec,
									   const Point& tick,
									   bool bIncrement,
									   bool bHorizontal,
									   const std::shared_ptr<Data>& data)
{
	long long tickKey = 0;
	if (bHorizontal)
	{
		tickKey = pseudoDouble(tick.x);
		for (const auto& dimDesItm : this->hDimDesMap[tickKey])
		{
			const auto& dim = data->m_dimensions.at(dimDesItm.first);
			if (dimDesItm.second == bIncrement)
			{
				_dimPairVec.push_back(dimDesItm);
				//pTestLineVec->push_back(dim->lines.at(2));
			}
		}
	}
	else
	{
		tickKey = pseudoDouble(tick.y);
		for (const auto& dimDesItm : this->vDimDesMap[tickKey])
		{
			const auto& dim = data->m_dimensions.at(dimDesItm.first);
			if (dimDesItm.second == bIncrement)
			{
				_dimPairVec.push_back(dimDesItm);
				//pTestLineVec->push_back(dim->lines.at(2));
			}
		}
	}
	return true;
}




bool BeamLongitudinal::findTruncatedDimPair(std::shared_ptr<BeamLongitudinalRein> rein,
											std::vector<DimensionPair>& dimPairVec,
											const std::shared_ptr<Data>& data)
{

	if (dimPairVec.empty())
		return true;

	std::function<const Point& (const Line&)> accessPt, accessCheckPt;

	if (dimPairVec.front().second)
	{
		accessPt = &Line::e;
		accessCheckPt = &Line::s;
	}
	else
	{
		accessPt = &Line::s;
		accessCheckPt = &Line::e;
	}
	//确保找到匹配截断的标注
	const auto& miniLen = pseudoDouble(supportDim->lines.at(2).length());
	std::pair<DimensionPair, double> defRef(std::make_pair(dimPairVec.front(),
														   data->m_dimensions.at(dimPairVec.front().first)->lines.at(2).length()));

	for (const auto& dimPairItm : dimPairVec)
	{
		const auto& dim = data->m_dimensions.at(dimPairItm.first);
		auto& dimLine = dim->lines.at(2);

		if (defRef.second > dimLine.length())
		{
			defRef.first = dimPairItm;
			defRef.second = dimLine.length();
		}

		//以对齐点的另一个点查找标记
		const auto& checkDimVecIt = hDimDesMap.find(pseudoDouble(accessPt(dimLine).x));
		if (checkDimVecIt != hDimDesMap.end())
		{
			for (auto pairItm : checkDimVecIt->second)
			{
				const auto& checkDim = data->m_dimensions.at(pairItm.first);
				auto& checkDimLine = checkDim->lines.at(2);
				//长度匹配
				if (abs(pseudoDouble(checkDimLine.length()) - miniLen) < 1)
				{
					//和标志相接
					if (abs(accessPt(dimLine).x - accessCheckPt(checkDimLine).x) < 1)
					{
						//pTestLineVec->push_back(dimLine);
						rein->truncatedLenDimDes.push_back(std::make_pair(dimPairItm, 0.0));
						return true;
					}
					//和标志另一点对齐
					else if (abs(accessPt(dimLine).x - accessPt(checkDimLine).x) < 1)
					{
						//pTestLineVec->push_back(dimLine);
						rein->truncatedLenDimDes.push_back(std::make_pair(dimPairItm, 0.0 - checkDimLine.length()));
						return true;
					}
				}
			}
		}
	}
	//pTestLineVec->push_back(data->m_dimensions.at(defRef.first.first)->lines.at(2));
	rein->truncatedLenDimDes.push_back(std::make_pair(defRef.first, 0.0));
	return true;
}

bool BeamLongitudinal::findProfile(Data& data)
{
	//比较剖线是否长度相同。后面还要加上在某个范围的两个竖线
	auto checkProfilePair = [](const Line& l1, const Line& l2) ->bool
	{
		auto h1 = strimDouble(std::abs(l1.s.y - l1.e.y));
		auto h2 = strimDouble(std::abs(l2.s.y - l2.e.y));
		if (abs(h1 - h2) < 0.00001)
			return true;
		else
			return false;
	};


	auto mergeTwoPair = [this, &data, &checkProfilePair](
		std::map<long long, std::pair<int, int>>::iterator first,
		std::map<long long, std::pair<int, int>>::iterator second)->bool
	{
		auto& firstIdxPair = this->longitudinalProfileVec.at(first->second.first)->profileLineIdxPair;
		auto& secondIdxPair = this->longitudinalProfileVec.at(second->second.first)->profileLineIdxPair;
		if (checkProfilePair(data.m_lineData.lines().at(firstIdxPair.second),
							 data.m_lineData.lines().at(secondIdxPair.second)))
		{
			first->second.second = 2;
			firstIdxPair.first = secondIdxPair.second;
			this->longitudinalProfileVec.at(first->second.first)->findTextAttribute(data);
			this->longitudinalProfileVec.erase(this->longitudinalProfileVec.begin() + second->second.first);
			return true;
		}
		return false;
	};


	//! <x伪坐标，data中线索引>
	std::map<long long, int> vCrossLineIdxMap;
	// ! <x伪坐标，<longitudinalProfileVec中剖线的索引，找到相同x的次数（最大2次）>>
	std::map<long long, std::pair<int, int>> xTickMap;
	//以剔除的方式找到所有的剖线
	for (auto i = this->spBlock->box.vt; i >= this->spBlock->box.vf; i--)
	{
		auto vLineIdx = data.m_lineData.vLinesIndices().at(i);
		const auto& l = data.m_lineData.lines().at(vLineIdx);
		if (l.e.y<this->spBlock->box.bottom || l.s.y>this->spBlock->box.top)
		{
			continue;
		}
		auto cornersIt = data.m_cornerData.corners().find(vLineIdx);
		//有交线的垂线
		if (cornersIt != data.m_cornerData.corners().end())
		{
			//备份垂线索引，和X伪坐标
			vCrossLineIdxMap[pseudoDouble(l.s.x)] = vLineIdx;
		}
		//无交线的垂线
		else
		{
			auto pseudoX = pseudoDouble(l.s.x);
			auto it = xTickMap.find(pseudoX);
			// 得到可能配对的剖线
			if (it != xTickMap.end())
			{
				auto& pair = this->longitudinalProfileVec.at(it->second.first)->profileLineIdxPair;
				// 根据剖线长度验证剖线
				if (checkProfilePair(data.m_lineData.lines().at(pair.second), l))
				{
					pair.first = vLineIdx;
					it->second.second++;//aka 赋值为2
				}
			}
			// 得到垂线的X伪坐标，压入profilePos的索引，递增触发次数，构造剖线对象
			else
			{
				xTickMap[pseudoX].first = static_cast<int>(this->longitudinalProfileVec.size());
				xTickMap[pseudoX].second++;
				this->longitudinalProfileVec.push_back(std::make_shared<BeamLongitudinalProfile>());
				this->longitudinalProfileVec.back()->profileLineIdxPair.second = vLineIdx;
			}
		}
	}

	//弥补，有一个剖线被相交的情况或者两个剖线交错的情况
	for (auto xTickit = xTickMap.begin(); xTickit != xTickMap.end();)
	{
		//不是完整的剖线对
		if (xTickit->second.second != 2)
		{
			//从有交线的垂线中找出遗漏的剖线
			auto vCrossIdxIt = vCrossLineIdxMap.find(xTickit->first);
			//找到遗漏有交线的剖线
			if (vCrossIdxIt != vCrossLineIdxMap.end())
			{
				auto& idxPair = this->longitudinalProfileVec.at(xTickit->second.first)->profileLineIdxPair;
				if (checkProfilePair(data.m_lineData.lines().at(vCrossIdxIt->second),
									 data.m_lineData.lines().at(idxPair.second)))
				{
					idxPair.first = vCrossIdxIt->second;
					xTickit->second.second = 2;
					//搜索文本
					this->longitudinalProfileVec.at(xTickit->second.first)->findTextAttribute(data);
				}
				++xTickit;
			}
			//没有找到
			else
			{
				//!存在剖线不在同一个x tick 上的情况
				//遍历前后的Tick，找到最近的Tick
				std::map<long long, std::pair<int, int>>::iterator nowIt, preIt;
				long long preOffset, nextOffset;
				if (xTickit == xTickMap.begin())
				{
					nowIt = xTickit;
					std::advance(xTickit, 1);
					//下一个
					if (nowIt->second.second != 2 &&
						xTickit->second.second != 2)
					{
						if (mergeTwoPair(nowIt, xTickit))
						{
							xTickMap.erase(xTickit++);
							continue;
						}
					}
				}
				else if (xTickit != xTickMap.begin() && std::distance(xTickit, xTickMap.end()) > 1)
				{
					nowIt = xTickit;
					std::advance(xTickit, -1);
					preIt = xTickit;
					std::advance(xTickit, 2);
					preOffset = nowIt->first - preIt->first;
					nextOffset = xTickit->first - nowIt->first;
					//下一个是有效的剖线
					if (nowIt->second.second != 2 &&
						xTickit->second.second != 2 &&
						preOffset > nextOffset)
					{
						if (mergeTwoPair(nowIt, xTickit))
						{
							xTickMap.erase(xTickit++);
							continue;
						}
					}
					//上一个是有效的剖线
					else if (preIt->second.second != 2 && nowIt->second.second != 2)
					{
						if (mergeTwoPair(preIt, nowIt))
						{
							xTickit = nowIt;
							xTickMap.erase(xTickit++);
							continue;
						}
					}
				}
				else //dis==1
				{
					nowIt = xTickit;
					std::advance(xTickit, -1);
					preIt = xTickit;
					if (preIt->second.second != 2 && nowIt->second.second != 2)
					{
						if (mergeTwoPair(preIt, nowIt))
						{
							xTickit = nowIt;
							xTickMap.erase(xTickit++);
							continue;
						}
						else
							xTickit = xTickMap.end();
					}
				}

			}
		}
		//完整的剖线对
		else
		{
			//搜索文本
			this->longitudinalProfileVec.at(xTickit->second.first)->findTextAttribute(data);
			++xTickit;
		}
	}

#if 0
	for (const auto& pair : longitudinalProfileVec)
	{
		const auto& p = pair;
		//if (p->profileLineIdxPair.first == 0)
		{
			pTestIdxVec->push_back(p->profileLineIdxPair.first);
			pTestIdxVec->push_back(p->profileLineIdxPair.second);
		}
	}
#endif
	//todo 可能两个剖线都被相交了，目前还没遇到,暂不实现
	return true;
}

bool BeamLongitudinal::initFirstSupportRef(std::shared_ptr<Data> data)
{
	//得到最小长度的标注集
	std::vector<unsigned int> lenDimVec = lenDimDesMap.begin()->second;
	//从左往右排序
	std::sort(lenDimVec.begin(), lenDimVec.end(), [&data](unsigned int dimIdx1, unsigned int dimIdx2)
	{
		const auto& d1 = data->m_dimensions.at(dimIdx1);
		const auto& d2 = data->m_dimensions.at(dimIdx2);
		if (d1->lines.at(2).s.x < d2->lines.at(2).s.x)
			return true;
		else
			return false;
	});

	//找到第一个支撑柱的右偏差标注
	bool bFound = false;
	for (auto i = 0; !bFound && i < lenDimVec.size(); ++i)
	{
		auto lenDimIdx = lenDimVec.at(i);
		const auto& lenDim = data->m_dimensions.at(lenDimIdx);
		auto lenRefLine = lenDim->lines.at(2);
		auto checkHDimDesVec = hDimDesMap[pseudoDouble(lenRefLine.e.x)];
		if (checkHDimDesVec.empty())
			continue;
		else
		{
			for (auto j = 0; j < checkHDimDesVec.size(); ++j)
			{
				const auto& checkDimPair = checkHDimDesVec.at(j);
				const auto& checkDim = data->m_dimensions.at(checkDimPair.first);
				const auto& checkDimRefLine = checkDim->lines.at(2);
				//增量标注
				if (checkDimPair.second && std::abs(checkDimRefLine.s.y - lenRefLine.s.y) < 0.001)
				{
					supportDim = lenDim;
					bFound = true;
					break;
				}
			}
		}
	}
	pTestLineVec->push_back(supportDim->lines.at(2));
	return true;
}

bool BeamLongitudinal::installFrameInfo(std::shared_ptr<Beam> spBeam,
										std::shared_ptr<Data> data,
										std::vector<std::shared_ptr<Pillar>>& _pillarVec)
{
	auto firstColRightBorder = this->supportDim->lines.at(2);
	//查找跨的描述柱定位框架信息
	for (auto& spanItm : spBeam->spSpanVec)
	{
		const auto& col1 = _pillarVec.at(spanItm->supportPair.first);
		const auto& col2 = _pillarVec.at(spanItm->supportPair.second);
		spdlog::get("all")->info("col1 location v: name {},offset {};h: name {},offset {}",
								 col1->getVPDistance().first, col1->getVPDistance().second.first,
								 col1->getHPDistance().first, col1->getHPDistance().second.first);
		spdlog::get("all")->info("col2 location v: name {},offset {};h: name {},offset {}",
								 col2->getVPDistance().first, col2->getVPDistance().second.first,
								 col2->getHPDistance().first, col2->getHPDistance().second.first);

		double vAxisFoundToRefDimLineLeftOffsetInFrame = 0;
		auto refLineToFirstVAxis = firstColRightBorder.s.x - this->spBlock->m_axisVLines.front().second.s.x;

		spdlog::get("all")->info("offset of dim line to first v axis in block of beam frame:{}", refLineToFirstVAxis);

		for (auto& vAxisSymbolItm : this->spBlock->m_axisVLines)
		{
			//找到匹配的轴线
			if (col1->getVPDistance().first == vAxisSymbolItm.first.second)
			{
				// 两个支撑的间距
				double supportNetDist = col2->getBox().left - col1->getBox().right;
				spdlog::get("all")->info("distance of 2 cols:{}", supportNetDist);
				// 柱1右边界到找到的轴的距离（相对距离）
				auto col1RightToAxisFoundOffset = col1->getVPDistance().second.first;
				//修正距离
				col1RightToAxisFoundOffset = col1RightToAxisFoundOffset + col1->getBox().width();
				spdlog::get("all")->info("right border to axis found:{}", col1RightToAxisFoundOffset);

				//找到的块中的轴距参考线的距离
				vAxisFoundToRefDimLineLeftOffsetInFrame = vAxisSymbolItm.second.s.x - firstColRightBorder.s.x;
				spdlog::get("all")->info("current v axis in block to ref line:{}", vAxisFoundToRefDimLineLeftOffsetInFrame);
				//框架梁中的位置区间
				//柱1的右边界
				double col1RightInFrame = firstColRightBorder.s.x + col1RightToAxisFoundOffset + vAxisFoundToRefDimLineLeftOffsetInFrame;
				double col2LeftInFrame = col1RightInFrame + supportNetDist;
				spdlog::get("all")->info("find profile in span id {} among {} and {}",
										 spanItm->spanId,
										 col1RightInFrame,
										 col2LeftInFrame);

				//查找跨中的剖线
				this->installProfileInfo(data, col1RightInFrame, col2LeftInFrame, spanItm);
				for (const auto& reinItm : this->spReinVec)
				{
					//查找跨中截断长度
					this->installTruncatedInfo(reinItm, data, col1RightInFrame, col2LeftInFrame, spanItm);

					//查找锚固长度
					this->installAnchorageInfo(reinItm, data, col1RightInFrame, col2LeftInFrame, col1, col2, spanItm);

					// 查找上翻或下翻数据
					installZDeltaInfoo(reinItm, data, col1, col2, col1RightInFrame, col2LeftInFrame, spanItm);
				}
				break;
			}
		}
	}
	spdlog::get("all")->info("found all\n");
	return true;
}

void BeamLongitudinal::installZDeltaInfoo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
										  std::shared_ptr<Data> data,
										  const std::shared_ptr<Pillar>& col1,
										  const std::shared_ptr<Pillar>& col2,
										  double col1RightInFrame,
										  double col2LeftInFrame,
										  std::shared_ptr<BeamSpan>& spanItm)
{
	if (reinItm->reinType == BeamLongitudinalRein::BOUNDARY)
	{
		const auto& refComp = reinItm->components.front();
		const auto& refCompLine = data->m_lineData.lines().at(refComp->lineIdx);
		const double boundaryY = reinItm->bottom + reinItm->spanHeight() / 2;
		double offset = col1->getBox().width() * 2;
		//伸入左柱
		if (refCompLine.s.x < col1RightInFrame &&
			refCompLine.s.x>col1RightInFrame - offset &&
			refCompLine.e.x > col1RightInFrame)
		{
			//顶部ZDelta
			if (refCompLine.s.y > boundaryY)
			{
				for (const auto& comp : reinItm->components)
				{
					//设置左侧ZDelta
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Left)
					{
						if (abs(reinItm->leftZDelta.length) > 0)
						{
							spanItm->left.topZDelta = reinItm->leftZDelta;
							if (refCompLine.s.x < reinItm->leftZDelta.pt.x)
							{
								//修正位置
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const double fixedSize = outerLine.s.y - refCompLine.s.y;
								assert(fixedSize > 0);
								spanItm->left.topZDelta.pt.x -= fixedSize;
							}
							//换算成相对梁底的位置
							double offset2Col1LeftBorder = spanItm->left.topZDelta.pt.x - col1RightInFrame;
							spanItm->left.topZDelta.axisDistance = col1->getVPDistance();
							spanItm->left.topZDelta.axisDistance.second.first += offset2Col1LeftBorder;
							break;
						}
					}
				}
			}
			//底部ZDelta
			else
			{
#if 0
				std::function<BeamPosZDelta& (BeamPosition&)> getBeamPosZDelta;
				getBeamPosZDelta = &BeamPosition::btmZDelta;
				for (const auto& comp : reinItm->components)
				{
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Left)
					{
						if (abs(reinItm->leftZDelta.length) > 0)
						{
							getBeamPosZDelta(spanItm->left) = reinItm->leftZDelta;
							if (refCompLine.s.x < reinItm->leftZDelta.pt.x)
							{
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const auto fixedSize = abs(refCompLine.s.y - outerLine.s.y);
								assert(fixedSize > 0);
								getBeamPosZDelta(spanItm->left).pt.x -= fixedSize;
							}
							double offset2Col1LeftBorder = getBeamPosZDelta(spanItm->left).pt.x - col1RightInFrame;
							getBeamPosZDelta(spanItm->left).axisDistance = col1->vdistance;
							getBeamPosZDelta(spanItm->left).axisDistance.second.first += offset2Col1LeftBorder;
						}
					}
				}
#endif
#if 1
				// todo finish 
				for (const auto& comp : reinItm->components)
				{
					//设置左侧ZDelta
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Left)
					{
						if (abs(reinItm->leftZDelta.length) > 0)
						{
							spanItm->left.btmZDelta = reinItm->leftZDelta;
							if (refCompLine.s.x < reinItm->leftZDelta.pt.x)
							{
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const auto fixedSize = refCompLine.s.y - outerLine.s.y;
								assert(fixedSize > 0);
								spanItm->left.btmZDelta.pt.x -= fixedSize;
							}
							//换算成相对梁底的位置
							double offset2Col1LeftBorder = spanItm->left.btmZDelta.pt.x - col1RightInFrame;
							spanItm->left.btmZDelta.axisDistance = col1->getVPDistance();
							spanItm->left.btmZDelta.axisDistance.second.first += offset2Col1LeftBorder;
							break;
						}
					}
				}
#endif
			}
		}
		offset = col2->getBox().width() * 2;
		//伸入右柱
		if (refCompLine.e.x > col2LeftInFrame &&
			refCompLine.e.x < col2LeftInFrame + offset &&
			refCompLine.s.x < col2LeftInFrame)
		{
			//顶部ZDelta
			if (refCompLine.s.y > boundaryY)
			{
				for (const auto& comp : reinItm->components)
				{
					//设置右侧ZDelta
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Right)
					{
						if (abs(reinItm->rightZDelta.length) > 0)
						{
							spanItm->right.topZDelta = reinItm->rightZDelta;
							if (refCompLine.e.x > reinItm->rightZDelta.pt.x)
							{
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const auto fixedSize = outerLine.s.y - refCompLine.s.y;
								assert(fixedSize > 0);
								spanItm->right.topZDelta.pt.x += fixedSize;
							}
							//换算成相对梁底的位置
							double offset2Col2LeftBorder = spanItm->right.topZDelta.pt.x - col2LeftInFrame;
							spanItm->right.topZDelta.axisDistance = col2->getVPDistance();
							spanItm->right.topZDelta.axisDistance.second.first += offset2Col2LeftBorder;
							break;
						}
					}
				}
			}
			//底部ZDelta
			else
			{
				for (const auto& comp : reinItm->components)
				{
					//设置右侧ZDelta
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Right)
					{
						if (abs(reinItm->rightZDelta.length) > 0)
						{
							spanItm->right.btmZDelta = reinItm->rightZDelta;
							if (refCompLine.e.x > reinItm->rightZDelta.pt.x)
							{
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const auto fixedSize = refCompLine.s.y - outerLine.s.y;
								assert(fixedSize > 0);
								spanItm->right.btmZDelta.pt.x += fixedSize;
							}
							//换算成相对梁底的位置
							double offset2Col2LeftBorder = spanItm->right.btmZDelta.pt.x - col2LeftInFrame;
							spanItm->right.btmZDelta.axisDistance = col2->getVPDistance();
							spanItm->right.btmZDelta.axisDistance.second.first += offset2Col2LeftBorder;
							break;
						}
					}
				}
			}
		}

	}
}


void BeamLongitudinal::installAnchorageInfo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
											std::shared_ptr<Data> data,
											double col1RightInFrame,
											double col2LeftInFrame,
											const std::shared_ptr<Pillar>& col1,
											const std::shared_ptr<Pillar>& col2,
											std::shared_ptr<BeamSpan>& spanItm)
{
	//加固的高度
	const double& zDelta = reinItm->spanHeight();
	//加固的中线基准
	const double& boundaryY = zDelta / 2 + reinItm->bottom;
	//跨间净长
	const double& supportNetDist = col2LeftInFrame - col1RightInFrame;
	std::function<const Point(const Line&)> accessPt, accessAnotherPt;

	const auto& refComp = reinItm->components.front();
	const auto& refCompLine = data->m_lineData.lines().at(refComp->lineIdx);
	//伸入左柱
	if (refCompLine.s.x<col1RightInFrame &&
		refCompLine.s.x>col1RightInFrame - col1->getBox().width() * 3 / 2 &&
		refCompLine.e.x > col1RightInFrame)
	{
		double offset = col1->getBox().width() * 3 / 2;
		std::stringstream ss;
		//取水平标注
		for (const auto& hDimPairItm : *refComp->dims)
		{
			const auto& hDim = data->m_dimensions.at(hDimPairItm.first);
			const auto& hDimRefLine = hDim->lines.at(2);
			//取增量标注
			if (hDimPairItm.second &&
				abs(hDimRefLine.s.x - refCompLine.s.x) < 1/* &&
				hDimRefLine.s.x > col1RightInFrame - offset &&
				hDimRefLine.e.x < col1RightInFrame + offset*/)

			{
				//获取标注长度
				ss << this->getDimInfo(hDim);
				spdlog::get("all")->info("left horizontal anchorage length:{} at {}", ss.str(), refCompLine.s.y);
				break;
			}
		}

		//顶部
		if (refCompLine.s.y > boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//只取左端
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Left)
				{
					const auto& leftRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//竖线
					if (leftRefLine.vertical())
					{
						//竖线尺寸标注
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (vDimPairItm.second &&//递增
								vDimRefLine.s.x<leftRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>leftRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ",
														 vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//斜线
					else
					{
						/*if (abs(leftRefLine.e.x - refCompLine.s.x) < 1 &&
							leftRefLine.length() > zDelta / 2)*/
						{

							ss << '+';
							ss << std::fixed << std::setprecision(2) << leftRefLine.length();
						}
					}
					spanItm->left.topAnchorageLength.insert(std::make_pair<long long, std::string >(pseudoDouble(refCompLine.s.y), ss.str()));
					spdlog::get("all")->info("left top anchorage length:{} at {}", ss.str(), refCompLine.s.y);
					break;
				}
			}
		}
		//底部
		else if (refCompLine.s.y < boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//左端
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Left)
				{
					const auto& leftRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//竖线
					if (leftRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (!vDimPairItm.second &&//递减
								vDimRefLine.s.x<leftRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>leftRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//斜线
					else
					{
						/*if (abs(leftRefLine.e.x - refCompLine.s.x) < 1 &&
							leftRefLine.length() > zDelta / 2)*/
						{

							ss << '+';
							ss << std::fixed << std::setprecision(2) << leftRefLine.length();
						}
					}
					spanItm->left.btmAnchorageLength.insert(std::make_pair<long long, std::string >(pseudoDouble(refCompLine.s.y), ss.str()));
					spdlog::get("all")->info("left btm anchorage length:{} at {}", ss.str(), refCompLine.s.y);
					break;
				}
			}
		}

	}
	//伸入右柱
	if (refCompLine.e.x > col2LeftInFrame &&
		refCompLine.e.x < col2LeftInFrame + col2->getBox().width() * 3 / 2 &&
		refCompLine.s.x < col2LeftInFrame)
	{
		double offset = col2->getBox().width() * 3 / 2;
		std::stringstream ss;
		//取水平标注
		for (const auto& hDimPairItm : *refComp->dims)
		{
			const auto& hDim = data->m_dimensions.at(hDimPairItm.first);
			const auto& hDimRefLine = hDim->lines.at(2);
			//取递减标注
			if (!hDimPairItm.second &&
				abs(hDimRefLine.e.x - refCompLine.e.x) < 1/* &&
				hDimRefLine.s.x > col2LeftInFrame - offset &&
				hDimRefLine.e.x < col2LeftInFrame + offset*/)
			{
				//获取标注长度
				ss << this->getDimInfo(hDim);
				spdlog::get("all")->info("left horizontal anchorage length:{} at {}", ss.str(), refCompLine.s.y);
				break;
			}
		}

		//顶部
		if (refCompLine.s.y > boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//只取右端
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Right)
				{
					const auto& rightRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//竖线
					if (rightRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (vDimPairItm.second &&//递增
								vDimRefLine.s.x<rightRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>rightRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//斜线
					else
					{
						/*if (abs(rightRefLine.s.x - refCompLine.e.x) < 1 &&
							rightRefLine.length() > zDelta / 2)*/
						{

							ss << '+';
							ss << std::fixed << std::setprecision(2) << rightRefLine.length();
						}
					}
					spanItm->right.topAnchorageLength.insert(std::make_pair<long long, std::string >(pseudoDouble(refCompLine.s.y), ss.str()));
					spdlog::get("all")->info("right top other anchorage length:{} at {}", ss.str(), refCompLine.s.y);
					break;
				}
			}
		}
		//底部
		else/* if (refCompLine.s.y < boundaryY)*/
		{
			for (const auto& comp : reinItm->components)
			{
				//右端
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Right)
				{
					const auto& rightRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//竖线
					if (rightRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (!vDimPairItm.second &&//递减
								vDimRefLine.s.x<rightRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>rightRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//斜线
					else
					{
						/*if (abs(rightRefLine.s.x - refCompLine.e.x) < 1 &&
							rightRefLine.length() > zDelta / 2)*/
						{
							ss << '+';
							ss << std::fixed << std::setprecision(2) << rightRefLine.length();
						}
					}
					spanItm->right.btmAnchorageLength.insert(std::make_pair<long long, std::string >(pseudoDouble(refCompLine.s.y), ss.str()));
					spdlog::get("all")->info("right btm other anchorage length:{} at {}", ss.str(), refCompLine.s.y);
					break;
				}
			}
		}

	}
}



std::string BeamLongitudinal::getDimInfo(const std::shared_ptr<Dimension>& dim)
{
	std::stringstream ss;
	const auto& d = std::dynamic_pointer_cast<DRW_DimLinear>(dim->lines.at(2).entity);
	if (d->getText().empty())
		ss << std::fixed << std::setprecision(2) << dim->measurement;
	else
		ss << d->getText();

	return ss.str();
}

bool BeamLongitudinal::installProfileInfo(std::shared_ptr<Data> data,
										  double col1RightInFrame,
										  double col2RightInFrame,
										  std::shared_ptr<BeamSpan>& spanItm)
{
	double pieceWidth = (col2RightInFrame - col1RightInFrame) / 3;
	//区分每跨中的剖线
	for (const auto& profileItm : this->longitudinalProfileVec)
	{
		//剖线的位置
		const auto& profileX = data->m_lineData.lines().at(profileItm->profileLineIdxPair.first).s.x;
		if (profileX >= col1RightInFrame && profileX <= col2RightInFrame)
		{
			//!剖面名
			const auto& profileTag = data->m_textPointData.textpoints().at(profileItm->tagPointIdx);
			const auto& text = std::dynamic_pointer_cast<DRW_Text>(profileTag.entity)->text;
			spdlog::get("all")->info("found a profile {}-{} at {} in span {}",
									 text, text, profileX,
									 spanItm->spanId);
			//左边
			if (profileX >= col1RightInFrame && profileX <= col1RightInFrame + pieceWidth)
			{
				spanItm->left.spBeamSection = std::make_shared<BeamSection>();
				spanItm->left.spBeamSection->name = text;
			}
			//中间
			if (profileX >= col1RightInFrame + pieceWidth && profileX <= col1RightInFrame + 2 * pieceWidth)
			{
				spanItm->mid.spBeamSection = std::make_shared<BeamSection>();
				spanItm->mid.spBeamSection->name = text;
			}
			//右边
			if (profileX >= col2RightInFrame - pieceWidth && profileX <= col2RightInFrame)
			{
				spanItm->right.spBeamSection = std::make_shared<BeamSection>();
				spanItm->right.spBeamSection->name = text;
			}
		}
	}
	return true;
}

void BeamLongitudinal::installTruncatedInfo(const std::shared_ptr<BeamLongitudinalRein>& rein,
											const std::shared_ptr<Data>& data,
											const double col1RightInFrame,
											const double col2LeftInFrame,
											std::shared_ptr<BeamSpan>& spanItm)
{
	std::function<Point(const Line&)> accessPt;
	std::function<BeamPosition& (std::shared_ptr<BeamSpan>&)> accessSpanPos;

	if (rein->reinType == BeamLongitudinalRein::INNER)
	{
		//跨中的纵向边界
		double boundaryY = rein->spanHeight() / 2 + rein->bottom;
		//遍历可能的截断集合
		for (const auto& truncatedDimPairItm : rein->truncatedLenDimDes)
		{
			//递增标注
			if (truncatedDimPairItm.first.second)
			{
				accessPt = &Line::s;
				accessSpanPos = &BeamSpan::right;
			}
			//递减标注
			else
			{
				accessPt = &Line::e;
				accessSpanPos = &BeamSpan::left;
			}
			//取得截断标注
			const auto& truncatedDim = data->m_dimensions.at(truncatedDimPairItm.first.first);
			//截断的长度
			double tctedLen = strimDouble(truncatedDim->lines.at(2).length() + truncatedDimPairItm.second);
			//标注的参考线
			const Line& reinHLine = data->m_lineData.lines().at(rein->components.front()->lineIdx);
			//跨间距离
			double netLong = col2LeftInFrame - col1RightInFrame;
			//截断的点的x值
			double truncatedAbscissa = accessPt(truncatedDim->lines.at(2)).x;
			//预先滤过伸入柱的较短的线（比例控制在0.2以下）
			if (abs(truncatedAbscissa - col1RightInFrame) / netLong < 0.2 ||
				abs(truncatedAbscissa - col2LeftInFrame) / netLong < 0.2)
				continue;

			//截断的端点是否在跨区间
			if (truncatedAbscissa >= col1RightInFrame &&
				truncatedAbscissa <= col2LeftInFrame)
			{
				//顶部
				if (reinHLine.s.y > boundaryY)
				{
					//加入截面长度
					accessSpanPos(spanItm).topTruncatedLength.insert(std::make_pair(pseudoDouble(reinHLine.s.y), tctedLen));
#if 0
					//记录解析的截断信息
					if (truncatedDimPairItm.first.second)
						spdlog::get("all")->info("right top truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
					else
						spdlog::get("all")->info("left top truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
#endif
				}
				//底部
				if (reinHLine.s.y < boundaryY)
				{
					//加入截面长度
					accessSpanPos(spanItm).btmTruncatedLength.insert(std::make_pair(pseudoDouble(reinHLine.s.y), tctedLen));
#if 0
					//记录解析的截断信息
					if (truncatedDimPairItm.first.second)
						spdlog::get("all")->info("right btm  truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
					else
						spdlog::get("all")->info("left btm truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
#endif
				}
			}
		}
	}
}

bool BeamLongitudinalProfile::findTextAttribute(Data& data)
{
	// 以剖线长度为半径，在剖线两端找文本点
	std::vector<int> tVec1, tVec2;
	findText(tVec1, data, this->profileLineIdxPair.first);
	findText(tVec2, data, this->profileLineIdxPair.second);
	if (tVec1.empty() || tVec2.empty())
		return false;

	bool bFound = false;
	//对找到的文本点，进行验证，得到需要的配对的文本点
	for (auto it1 = tVec1.begin(); !bFound && it1 != tVec1.end(); ++it1)
	{
		auto pt1 = data.m_textPointData.textpoints().at(*it1);
		auto t1 = std::dynamic_pointer_cast<DRW_Text>(pt1.entity);
		for (auto it2 = tVec2.begin(); !bFound && it2 != tVec2.end(); ++it2)
		{
			auto pt2 = data.m_textPointData.textpoints().at(*it2);
			auto t2 = std::dynamic_pointer_cast<DRW_Text>(pt2.entity);

			if (t1->text == t2->text)
			{
				bFound = true;
				this->tagPointIdx = pt1.y > pt2.y ? *it1 : *it2;
				spdlog::get("all")->info("found matched text :{}", t1->text);
			}
		}
	}
	return true;
}

bool BeamLongitudinalProfile::findText(std::vector<int>& _textPtIdxVec, Data& data, const int idx)
{
	const auto& line = data.m_lineData.lines().at(idx);
	const auto& lineLen = line.length();
	//在终点找
	auto tp = data.m_kdtTreeData.kdtTexts().radiusSearch(line.e, lineLen);
	if (!tp.empty())
		_textPtIdxVec = std::move(tp);
	//在起点找
	tp = data.m_kdtTreeData.kdtTexts().radiusSearch(line.s, lineLen);
	if (!tp.empty())
		_textPtIdxVec.insert(_textPtIdxVec.end(), tp.begin(), tp.end());
	return true;
}

bool BeamLongitudinalRein::updateHBound(const Line& line)
{
	if (this->right == 0 || line.e.x > this->right)
		this->right = line.e.x;
	if (this->left == 0 || line.s.x < this->left)
		this->left = line.s.x;
	return true;
}

bool BeamLongitudinalRein::findOuterLineIdx(std::shared_ptr<Data>& data,
											std::shared_ptr<Block>& block,
											std::vector<Line>* pTestLineVec)
{
	//!查找外包围索引
	const auto refIdx = this->components.front()->lineIdx;
	const auto& refLine = data->m_lineData.lines().at(refIdx);
	const auto& offsetV = this->spanHeight() / 4;
	double deltaDis = offsetV;
	double spanVSize = this->spanHeight();
	//顶部加固
	if (refLine.s.y > bottom + spanVSize / 2)
	{
		const auto uY = refLine.s.y + offsetV;//upper Y
		const auto bY = refLine.s.y;// bottom Y
		const int uIdx = data->m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, uY);
		const int bIdx = data->m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bY);
		for (int i = bIdx; i <= uIdx; i++)
		{
			const int hIdx = data->m_lineData.hLinesIndices().at(i);
			//过滤自身
			if (hIdx == refIdx)
				continue;
			//过滤标注横线
			const auto& outerLine = data->m_lineData.lines().at(hIdx);
			if (outerLine.type == Line::Type::DIMENSIONS)
				continue;
			//合法的外边线
			if (outerLine.getLayer() != refLine.getLayer() &&
				outerLine.s.y > refLine.s.y)
			{
				if (outerIdx == 0)
				{
					outerIdx = hIdx;
				}
				else
				{
					//筛选
					double leftAbscissa = refLine.s.x > outerLine.s.x ? refLine.s.x : outerLine.s.x;
					double rightAbscissa = refLine.e.x < outerLine.e.x ? refLine.e.x : outerLine.e.x;
					if (rightAbscissa - leftAbscissa > 0 &&
						(rightAbscissa - leftAbscissa) / refLine.length() > 0.5 &&
						deltaDis > outerLine.s.y - refLine.s.y)
					{
						deltaDis = outerLine.s.y - refLine.s.y;
						outerIdx = hIdx;
					}
				}
			}
		}
	}
	//底部加固
	else if (refLine.s.y < bottom + spanVSize / 2)
	{
		const auto uY = refLine.s.y - Precision;
		const auto bY = refLine.s.y - offsetV;
		const int uIdx = data->m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, uY);
		const int bIdx = data->m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bY);
		for (int i = bIdx; i <= uIdx; i++)
		{
			const int hIdx = data->m_lineData.hLinesIndices().at(i);
			//过滤自身
			if (hIdx == refIdx)
				continue;
			//过滤标注横线
			const auto& outerLine = data->m_lineData.lines().at(hIdx);
			if (outerLine.type == Line::Type::DIMENSIONS)
				continue;
			//合法的外边线
			if (outerLine.getLayer() != refLine.getLayer() &&
				outerLine.s.y < refLine.s.y)
			{
				if (outerIdx == 0)
					outerIdx = hIdx;
				else
				{
					//筛选
					double leftAbscissa = refLine.s.x > outerLine.s.x ? refLine.s.x : outerLine.s.x;
					double rightAbscissa = refLine.e.x < outerLine.e.x ? refLine.e.x : outerLine.e.x;
					if (rightAbscissa - leftAbscissa > 0 &&
						(rightAbscissa - leftAbscissa) / refLine.length() > 0.5 &&
						deltaDis > abs(outerLine.s.y - refLine.s.y))
					{
						deltaDis = abs(outerLine.s.y - refLine.s.y);
						outerIdx = hIdx;
					}
				}
			}
		}
	}
	/*if (outerIdx != 0)
		pTestLineVec->push_back(data->m_lineData.lines().at(outerIdx));*/
	return true;
}


bool BeamLongitudinalRein::updateVBound(const Line& line)
{
	this->bottom = line.s.y;
	this->top = line.e.y;
	return true;
}

bool BeamLongitudinalRein::addComp(const std::shared_ptr<Data>& data, std::shared_ptr<BeamLongitudinalRein::ReinCompnent>& comp)
{
	if (comp->compnentType == ReinCompnent::ReinCompnentType::Longitudinal)
	{
		const auto& l = data->m_lineData.lines().at(comp->lineIdx);
		this->updateHBound(l);
	}
	else if (comp->compnentType == ReinCompnent::ReinCompnentType::Unknown)
	{
		const auto& l1 = data->m_lineData.lines().at(comp->lineIdx);
		assert(!this->components.empty());
		const auto& longitudinalLine = data->m_lineData.lines().at(this->components.front()->lineIdx);
		//定位左右类型
		if (abs(l1.e.x - longitudinalLine.s.x) < 1)
			comp->compnentType = ReinCompnent::ReinCompnentType::Left;
		else if (abs(l1.s.x - longitudinalLine.e.x) < 1)
			comp->compnentType = ReinCompnent::ReinCompnentType::Right;
	}
	this->components.push_back(comp);
	return true;
}

bool BeamLongitudinalRein::updateHBound(double l, double r)
{
	this->left = l;
	this->right = r;
	return true;
}

