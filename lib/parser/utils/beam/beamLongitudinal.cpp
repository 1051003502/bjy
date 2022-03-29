#include "beamLongitudinal.h"


bool BeamLongitudinal::parseBlock(std::shared_ptr<Block> _spBlock,
								  std::string& _strReinLayer,
								  std::shared_ptr<Data> data,
								  std::vector<unsigned int>& _blockDimIdxVec,
								  std::vector<int>& pTestVec,
								  std::vector<Line>& pTestLineVec)
{
	initReqiuredInfo(_spBlock, _strReinLayer, pTestVec, pTestLineVec);
	// ��ʼ����ע
	initDimMap(_blockDimIdxVec, data);
	// ��ʼ���ο���
	initFirstSupportRef(data);

	std::vector<int> hLineIdxVec, vLineIdxVec, sLineIdxVec;
	// �õ����еļӹ���
	findRein(hLineIdxVec, data);
	if (hLineIdxVec.empty())
		return false;
	parseRein(hLineIdxVec, data);

	//�õ�����
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
		//ˮƽ��ע�˵��xӳ��
		if (refLine.horizontal())
		{
			auto tickS = pseudoDouble(refLine.s.x);
			auto tickE = pseudoDouble(refLine.e.x);
			//����
			if (tickS < tickE)
			{
				this->hDimDesMap[tickS].emplace_back(dimIdxItm, true);
				this->hDimDesMap[tickE].emplace_back(dimIdxItm, false);
			}
			//����
			else
			{
				this->hDimDesMap[tickS].emplace_back(dimIdxItm, false);
				this->hDimDesMap[tickE].emplace_back(dimIdxItm, true);
			}
		}
		//��ֱ��עy��ӳ��
		else if (refLine.vertical())
		{
			auto tickS = pseudoDouble(refLine.s.y);
			auto tickE = pseudoDouble(refLine.e.y);
			//����
			if (tickS < tickE)
			{
				this->vDimDesMap[tickS].emplace_back(dimIdxItm, true);
				this->vDimDesMap[tickE].emplace_back(dimIdxItm, false);
			}
			//����
			else
			{
				this->vDimDesMap[tickS].emplace_back(dimIdxItm, false);
				this->vDimDesMap[tickE].emplace_back(dimIdxItm, true);
			}
		}
		//б�ı�ע
		else
		{
			//? there are no decline dimensions?
			Line tpLine(Point(dim->box.left, dim->box.top), Point(dim->box.right, dim->box.bottom));
			pushRTreeLines(tpLine, dimIdxItm, sDimRTree);
		}
		//��ע�ĳ���ӳ��
		auto lenTick = pseudoDouble(refLine.length());
		lenDimDesMap[lenTick].push_back(dimIdxItm);
	}
}


bool BeamLongitudinal::findRein(std::vector<int>& _hLineIdxVec,
								const std::shared_ptr<Data>& data)
{
	//std::vector<int> reinLineIdxVec;
	//����ˮƽ��
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
	//��ֱ��
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

	//б��
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

	//�����ӹ�
	// <yα���꣬data���ߵ�����>
	std::map<long long, std::vector<unsigned>> yTickMap;
	std::shared_ptr<BeamLongitudinalRein> rein;
	std::shared_ptr<BeamLongitudinalRein::ReinCompnent> comp;

	//�ӹ������Ժ���Ϊ��־����ֻ��������
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
		//����ˮƽ�ߵĽ��ߣ����з���
		std::vector<int> emptyVec{};
		data->m_cornerData.findCross(reinLineIdxVec, emptyVec, comp->lineIdx, [this, &rein, &data, &hLine](int crossIdx, const Point& pt)
		{

			const auto& crossLine = data->m_lineData.lines().at(crossIdx);
			//��������ӵ�
			if (bConnectTowLine(crossLine, hLine))
			{
				//��ӵĴ���
				if (crossLine.vertical())
				{
					auto vComp(std::make_shared<BeamLongitudinalRein::ReinCompnent>());
					vComp->lineIdx = crossIdx;
					vComp->reinIdx = rein->components.front()->reinIdx;
					rein->addComp(data, vComp);
					return true;
				}
				//��ӵ�б��
				else if (!crossLine.vertical())
				{
					auto sComp(std::make_shared<BeamLongitudinalRein::ReinCompnent>());
					sComp->lineIdx = crossIdx;
					sComp->reinIdx = rein->components.front()->reinIdx;
					rein->addComp(data, sComp);
					return true;
				}
				spdlog::get("all")->error("[{}][{}]it has horizontal line", __FUNCTION__, __LINE__);
				//������ƽ���ߵĽ���
				abort();//��Ӧ�����е���
				return true;
			}
			//������ӵ�
			else
			{
				//ȷ���߽�ӹ̺��ڲ��ӹ�
				if (crossLine.vertical())
				{
					//T �ཻ��ʽ
					// todo �����߿���е���
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
						//��߽�ӹ�����
						if (rein->reinType != BeamLongitudinalRein::ReinType::BOUNDARY)
							rein->reinType = BeamLongitudinalRein::ReinType::BOUNDARY;
						//�ӹ̵ı߽����·�Χ
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
						//�ڼӹ�����
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



	//��ǽ�֯�ļӹ���
	for (auto& v : yTickMap)
	{
		auto& condidates = v.second;

		//������������,��߽�Ϊ����
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
	// �����ڲ��ӹ̵���Ϣ���±߽�
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
			//Ϊÿ����ӹ̲��һ�������
			rein1->findOuterLineIdx(data, this->spBlock, pTestLineVec);
		}
	}
	spdlog::get("all")->info("mean gap size :{}", acWith / count);

	//���Ҽӹ̱�ע����Ϣ
	for (auto& reinItm : this->spReinVec)
	{
		double height = reinItm->spanHeight();

		auto& firstComp = reinItm->components.front();
		const auto& longitudinalLine = data->m_lineData.lines().at(firstComp->lineIdx);
		firstComp->dims = std::make_shared<std::vector<DimensionPair>>();
		//��������ע
		findHVDimension(*firstComp->dims, longitudinalLine.s, true, true, data);
		this->findTruncatedDimPair(reinItm, *firstComp->dims, data);
		//��˥����ע
		std::vector<DimensionPair> tp;
		findHVDimension(tp, longitudinalLine.e, false, true, data);
		this->findTruncatedDimPair(reinItm, tp, data);
		firstComp->dims->insert(firstComp->dims->end(), tp.begin(), tp.end());
		//todo ����б�߱�ע
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
				// RTree ������ע
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
				//todo ����б��Lae��ע
			}
		}
#endif

		//����������ӵ����߱�ע
		for (auto& compItm : reinItm->components)
		{
			if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Left ||
				compItm->compnentType == BeamLongitudinalRein::ReinCompType::Right)
			{
				const auto& verticalLine = data->m_lineData.lines().at(compItm->lineIdx);
				if (verticalLine.vertical())
				{
					compItm->dims = std::make_shared<std::vector<DimensionPair>>();
					//��������ע
					findHVDimension(*compItm->dims, verticalLine.s, true, false, data);
					//��˥����ע
					findHVDimension(*compItm->dims, verticalLine.e, false, false, data);
				}
			}
		}
	}


	//����ÿ���ⲿ�ӹ̵ĳ���ƫ��ͽǶ�
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
		//��������Ƿ����ƫ��
		if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Left)
		{
			accessZDelta = &BeamLongitudinalRein::leftZDelta;
			accessAnotherZDelta = &BeamLongitudinalRein::rightZDelta;
		}
		//�����Ҳ��Ƿ����ƫ��
		else if (compItm->compnentType == BeamLongitudinalRein::ReinCompType::Right)
		{
			accessZDelta = &BeamLongitudinalRein::rightZDelta;
			accessAnotherZDelta = &BeamLongitudinalRein::leftZDelta;
		}
		else
			continue;

		std::vector<int> excludeVec{ rein->components.front()->lineIdx };
		//�������ҵļӹ̵Ľ��ߣ������غϷ�����ӹ�
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
	//ȷ���ҵ�ƥ��ضϵı�ע
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

		//�Զ�������һ������ұ��
		const auto& checkDimVecIt = hDimDesMap.find(pseudoDouble(accessPt(dimLine).x));
		if (checkDimVecIt != hDimDesMap.end())
		{
			for (auto pairItm : checkDimVecIt->second)
			{
				const auto& checkDim = data->m_dimensions.at(pairItm.first);
				auto& checkDimLine = checkDim->lines.at(2);
				//����ƥ��
				if (abs(pseudoDouble(checkDimLine.length()) - miniLen) < 1)
				{
					//�ͱ�־���
					if (abs(accessPt(dimLine).x - accessCheckPt(checkDimLine).x) < 1)
					{
						//pTestLineVec->push_back(dimLine);
						rein->truncatedLenDimDes.push_back(std::make_pair(dimPairItm, 0.0));
						return true;
					}
					//�ͱ�־��һ�����
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
	//�Ƚ������Ƿ񳤶���ͬ�����滹Ҫ������ĳ����Χ����������
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


	//! <xα���꣬data��������>
	std::map<long long, int> vCrossLineIdxMap;
	// ! <xα���꣬<longitudinalProfileVec�����ߵ��������ҵ���ͬx�Ĵ��������2�Σ�>>
	std::map<long long, std::pair<int, int>> xTickMap;
	//���޳��ķ�ʽ�ҵ����е�����
	for (auto i = this->spBlock->box.vt; i >= this->spBlock->box.vf; i--)
	{
		auto vLineIdx = data.m_lineData.vLinesIndices().at(i);
		const auto& l = data.m_lineData.lines().at(vLineIdx);
		if (l.e.y<this->spBlock->box.bottom || l.s.y>this->spBlock->box.top)
		{
			continue;
		}
		auto cornersIt = data.m_cornerData.corners().find(vLineIdx);
		//�н��ߵĴ���
		if (cornersIt != data.m_cornerData.corners().end())
		{
			//���ݴ�����������Xα����
			vCrossLineIdxMap[pseudoDouble(l.s.x)] = vLineIdx;
		}
		//�޽��ߵĴ���
		else
		{
			auto pseudoX = pseudoDouble(l.s.x);
			auto it = xTickMap.find(pseudoX);
			// �õ�������Ե�����
			if (it != xTickMap.end())
			{
				auto& pair = this->longitudinalProfileVec.at(it->second.first)->profileLineIdxPair;
				// �������߳�����֤����
				if (checkProfilePair(data.m_lineData.lines().at(pair.second), l))
				{
					pair.first = vLineIdx;
					it->second.second++;//aka ��ֵΪ2
				}
			}
			// �õ����ߵ�Xα���꣬ѹ��profilePos�����������������������������߶���
			else
			{
				xTickMap[pseudoX].first = static_cast<int>(this->longitudinalProfileVec.size());
				xTickMap[pseudoX].second++;
				this->longitudinalProfileVec.push_back(std::make_shared<BeamLongitudinalProfile>());
				this->longitudinalProfileVec.back()->profileLineIdxPair.second = vLineIdx;
			}
		}
	}

	//�ֲ�����һ�����߱��ཻ����������������߽�������
	for (auto xTickit = xTickMap.begin(); xTickit != xTickMap.end();)
	{
		//�������������߶�
		if (xTickit->second.second != 2)
		{
			//���н��ߵĴ������ҳ���©������
			auto vCrossIdxIt = vCrossLineIdxMap.find(xTickit->first);
			//�ҵ���©�н��ߵ�����
			if (vCrossIdxIt != vCrossLineIdxMap.end())
			{
				auto& idxPair = this->longitudinalProfileVec.at(xTickit->second.first)->profileLineIdxPair;
				if (checkProfilePair(data.m_lineData.lines().at(vCrossIdxIt->second),
									 data.m_lineData.lines().at(idxPair.second)))
				{
					idxPair.first = vCrossIdxIt->second;
					xTickit->second.second = 2;
					//�����ı�
					this->longitudinalProfileVec.at(xTickit->second.first)->findTextAttribute(data);
				}
				++xTickit;
			}
			//û���ҵ�
			else
			{
				//!�������߲���ͬһ��x tick �ϵ����
				//����ǰ���Tick���ҵ������Tick
				std::map<long long, std::pair<int, int>>::iterator nowIt, preIt;
				long long preOffset, nextOffset;
				if (xTickit == xTickMap.begin())
				{
					nowIt = xTickit;
					std::advance(xTickit, 1);
					//��һ��
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
					//��һ������Ч������
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
					//��һ������Ч������
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
		//���������߶�
		else
		{
			//�����ı�
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
	//todo �����������߶����ཻ�ˣ�Ŀǰ��û����,�ݲ�ʵ��
	return true;
}

bool BeamLongitudinal::initFirstSupportRef(std::shared_ptr<Data> data)
{
	//�õ���С���ȵı�ע��
	std::vector<unsigned int> lenDimVec = lenDimDesMap.begin()->second;
	//������������
	std::sort(lenDimVec.begin(), lenDimVec.end(), [&data](unsigned int dimIdx1, unsigned int dimIdx2)
	{
		const auto& d1 = data->m_dimensions.at(dimIdx1);
		const auto& d2 = data->m_dimensions.at(dimIdx2);
		if (d1->lines.at(2).s.x < d2->lines.at(2).s.x)
			return true;
		else
			return false;
	});

	//�ҵ���һ��֧��������ƫ���ע
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
				//������ע
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
	//���ҿ����������λ�����Ϣ
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
			//�ҵ�ƥ�������
			if (col1->getVPDistance().first == vAxisSymbolItm.first.second)
			{
				// ����֧�ŵļ��
				double supportNetDist = col2->getBox().left - col1->getBox().right;
				spdlog::get("all")->info("distance of 2 cols:{}", supportNetDist);
				// ��1�ұ߽絽�ҵ�����ľ��루��Ծ��룩
				auto col1RightToAxisFoundOffset = col1->getVPDistance().second.first;
				//��������
				col1RightToAxisFoundOffset = col1RightToAxisFoundOffset + col1->getBox().width();
				spdlog::get("all")->info("right border to axis found:{}", col1RightToAxisFoundOffset);

				//�ҵ��Ŀ��е����ο��ߵľ���
				vAxisFoundToRefDimLineLeftOffsetInFrame = vAxisSymbolItm.second.s.x - firstColRightBorder.s.x;
				spdlog::get("all")->info("current v axis in block to ref line:{}", vAxisFoundToRefDimLineLeftOffsetInFrame);
				//������е�λ������
				//��1���ұ߽�
				double col1RightInFrame = firstColRightBorder.s.x + col1RightToAxisFoundOffset + vAxisFoundToRefDimLineLeftOffsetInFrame;
				double col2LeftInFrame = col1RightInFrame + supportNetDist;
				spdlog::get("all")->info("find profile in span id {} among {} and {}",
										 spanItm->spanId,
										 col1RightInFrame,
										 col2LeftInFrame);

				//���ҿ��е�����
				this->installProfileInfo(data, col1RightInFrame, col2LeftInFrame, spanItm);
				for (const auto& reinItm : this->spReinVec)
				{
					//���ҿ��нضϳ���
					this->installTruncatedInfo(reinItm, data, col1RightInFrame, col2LeftInFrame, spanItm);

					//����ê�̳���
					this->installAnchorageInfo(reinItm, data, col1RightInFrame, col2LeftInFrame, col1, col2, spanItm);

					// �����Ϸ����·�����
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
		//��������
		if (refCompLine.s.x < col1RightInFrame &&
			refCompLine.s.x>col1RightInFrame - offset &&
			refCompLine.e.x > col1RightInFrame)
		{
			//����ZDelta
			if (refCompLine.s.y > boundaryY)
			{
				for (const auto& comp : reinItm->components)
				{
					//�������ZDelta
					if (comp->compnentType == BeamLongitudinalRein::ReinCompnent::ReinCompnentType::Left)
					{
						if (abs(reinItm->leftZDelta.length) > 0)
						{
							spanItm->left.topZDelta = reinItm->leftZDelta;
							if (refCompLine.s.x < reinItm->leftZDelta.pt.x)
							{
								//����λ��
								const auto& outerLine = data->m_lineData.lines().at(reinItm->outerIdx);
								const double fixedSize = outerLine.s.y - refCompLine.s.y;
								assert(fixedSize > 0);
								spanItm->left.topZDelta.pt.x -= fixedSize;
							}
							//�����������׵�λ��
							double offset2Col1LeftBorder = spanItm->left.topZDelta.pt.x - col1RightInFrame;
							spanItm->left.topZDelta.axisDistance = col1->getVPDistance();
							spanItm->left.topZDelta.axisDistance.second.first += offset2Col1LeftBorder;
							break;
						}
					}
				}
			}
			//�ײ�ZDelta
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
					//�������ZDelta
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
							//�����������׵�λ��
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
		//��������
		if (refCompLine.e.x > col2LeftInFrame &&
			refCompLine.e.x < col2LeftInFrame + offset &&
			refCompLine.s.x < col2LeftInFrame)
		{
			//����ZDelta
			if (refCompLine.s.y > boundaryY)
			{
				for (const auto& comp : reinItm->components)
				{
					//�����Ҳ�ZDelta
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
							//�����������׵�λ��
							double offset2Col2LeftBorder = spanItm->right.topZDelta.pt.x - col2LeftInFrame;
							spanItm->right.topZDelta.axisDistance = col2->getVPDistance();
							spanItm->right.topZDelta.axisDistance.second.first += offset2Col2LeftBorder;
							break;
						}
					}
				}
			}
			//�ײ�ZDelta
			else
			{
				for (const auto& comp : reinItm->components)
				{
					//�����Ҳ�ZDelta
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
							//�����������׵�λ��
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
	//�ӹ̵ĸ߶�
	const double& zDelta = reinItm->spanHeight();
	//�ӹ̵����߻�׼
	const double& boundaryY = zDelta / 2 + reinItm->bottom;
	//��侻��
	const double& supportNetDist = col2LeftInFrame - col1RightInFrame;
	std::function<const Point(const Line&)> accessPt, accessAnotherPt;

	const auto& refComp = reinItm->components.front();
	const auto& refCompLine = data->m_lineData.lines().at(refComp->lineIdx);
	//��������
	if (refCompLine.s.x<col1RightInFrame &&
		refCompLine.s.x>col1RightInFrame - col1->getBox().width() * 3 / 2 &&
		refCompLine.e.x > col1RightInFrame)
	{
		double offset = col1->getBox().width() * 3 / 2;
		std::stringstream ss;
		//ȡˮƽ��ע
		for (const auto& hDimPairItm : *refComp->dims)
		{
			const auto& hDim = data->m_dimensions.at(hDimPairItm.first);
			const auto& hDimRefLine = hDim->lines.at(2);
			//ȡ������ע
			if (hDimPairItm.second &&
				abs(hDimRefLine.s.x - refCompLine.s.x) < 1/* &&
				hDimRefLine.s.x > col1RightInFrame - offset &&
				hDimRefLine.e.x < col1RightInFrame + offset*/)

			{
				//��ȡ��ע����
				ss << this->getDimInfo(hDim);
				spdlog::get("all")->info("left horizontal anchorage length:{} at {}", ss.str(), refCompLine.s.y);
				break;
			}
		}

		//����
		if (refCompLine.s.y > boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//ֻȡ���
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Left)
				{
					const auto& leftRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//����
					if (leftRefLine.vertical())
					{
						//���߳ߴ��ע
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (vDimPairItm.second &&//����
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
					//б��
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
		//�ײ�
		else if (refCompLine.s.y < boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//���
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Left)
				{
					const auto& leftRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//����
					if (leftRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (!vDimPairItm.second &&//�ݼ�
								vDimRefLine.s.x<leftRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>leftRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//б��
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
	//��������
	if (refCompLine.e.x > col2LeftInFrame &&
		refCompLine.e.x < col2LeftInFrame + col2->getBox().width() * 3 / 2 &&
		refCompLine.s.x < col2LeftInFrame)
	{
		double offset = col2->getBox().width() * 3 / 2;
		std::stringstream ss;
		//ȡˮƽ��ע
		for (const auto& hDimPairItm : *refComp->dims)
		{
			const auto& hDim = data->m_dimensions.at(hDimPairItm.first);
			const auto& hDimRefLine = hDim->lines.at(2);
			//ȡ�ݼ���ע
			if (!hDimPairItm.second &&
				abs(hDimRefLine.e.x - refCompLine.e.x) < 1/* &&
				hDimRefLine.s.x > col2LeftInFrame - offset &&
				hDimRefLine.e.x < col2LeftInFrame + offset*/)
			{
				//��ȡ��ע����
				ss << this->getDimInfo(hDim);
				spdlog::get("all")->info("left horizontal anchorage length:{} at {}", ss.str(), refCompLine.s.y);
				break;
			}
		}

		//����
		if (refCompLine.s.y > boundaryY)
		{
			for (const auto& comp : reinItm->components)
			{
				//ֻȡ�Ҷ�
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Right)
				{
					const auto& rightRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//����
					if (rightRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (vDimPairItm.second &&//����
								vDimRefLine.s.x<rightRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>rightRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//б��
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
		//�ײ�
		else/* if (refCompLine.s.y < boundaryY)*/
		{
			for (const auto& comp : reinItm->components)
			{
				//�Ҷ�
				if (comp->compnentType == BeamLongitudinalRein::ReinCompType::Right)
				{
					const auto& rightRefLine = data->m_lineData.lines().at(comp->lineIdx);
					//����
					if (rightRefLine.vertical())
					{
						for (const auto& vDimPairItm : *comp->dims)
						{
							const auto& vDim = data->m_dimensions.at(vDimPairItm.first);
							const auto& vDimRefLine = vDim->lines.at(2);
							if (!vDimPairItm.second &&//�ݼ�
								vDimRefLine.s.x<rightRefLine.s.x + supportNetDist / 2 &&
								vDimRefLine.s.x>rightRefLine.s.x - supportNetDist / 2)
							{
								spdlog::get("all")->info("vertical dim located at {} ", vDimRefLine.s.x);
								ss << '+';
								ss << this->getDimInfo(vDim);
							}
						}
					}
					//б��
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
	//����ÿ���е�����
	for (const auto& profileItm : this->longitudinalProfileVec)
	{
		//���ߵ�λ��
		const auto& profileX = data->m_lineData.lines().at(profileItm->profileLineIdxPair.first).s.x;
		if (profileX >= col1RightInFrame && profileX <= col2RightInFrame)
		{
			//!������
			const auto& profileTag = data->m_textPointData.textpoints().at(profileItm->tagPointIdx);
			const auto& text = std::dynamic_pointer_cast<DRW_Text>(profileTag.entity)->text;
			spdlog::get("all")->info("found a profile {}-{} at {} in span {}",
									 text, text, profileX,
									 spanItm->spanId);
			//���
			if (profileX >= col1RightInFrame && profileX <= col1RightInFrame + pieceWidth)
			{
				spanItm->left.spBeamSection = std::make_shared<BeamSection>();
				spanItm->left.spBeamSection->name = text;
			}
			//�м�
			if (profileX >= col1RightInFrame + pieceWidth && profileX <= col1RightInFrame + 2 * pieceWidth)
			{
				spanItm->mid.spBeamSection = std::make_shared<BeamSection>();
				spanItm->mid.spBeamSection->name = text;
			}
			//�ұ�
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
		//���е�����߽�
		double boundaryY = rein->spanHeight() / 2 + rein->bottom;
		//�������ܵĽضϼ���
		for (const auto& truncatedDimPairItm : rein->truncatedLenDimDes)
		{
			//������ע
			if (truncatedDimPairItm.first.second)
			{
				accessPt = &Line::s;
				accessSpanPos = &BeamSpan::right;
			}
			//�ݼ���ע
			else
			{
				accessPt = &Line::e;
				accessSpanPos = &BeamSpan::left;
			}
			//ȡ�ýضϱ�ע
			const auto& truncatedDim = data->m_dimensions.at(truncatedDimPairItm.first.first);
			//�ضϵĳ���
			double tctedLen = strimDouble(truncatedDim->lines.at(2).length() + truncatedDimPairItm.second);
			//��ע�Ĳο���
			const Line& reinHLine = data->m_lineData.lines().at(rein->components.front()->lineIdx);
			//������
			double netLong = col2LeftInFrame - col1RightInFrame;
			//�ضϵĵ��xֵ
			double truncatedAbscissa = accessPt(truncatedDim->lines.at(2)).x;
			//Ԥ���˹��������Ľ϶̵��ߣ�����������0.2���£�
			if (abs(truncatedAbscissa - col1RightInFrame) / netLong < 0.2 ||
				abs(truncatedAbscissa - col2LeftInFrame) / netLong < 0.2)
				continue;

			//�ضϵĶ˵��Ƿ��ڿ�����
			if (truncatedAbscissa >= col1RightInFrame &&
				truncatedAbscissa <= col2LeftInFrame)
			{
				//����
				if (reinHLine.s.y > boundaryY)
				{
					//������泤��
					accessSpanPos(spanItm).topTruncatedLength.insert(std::make_pair(pseudoDouble(reinHLine.s.y), tctedLen));
#if 0
					//��¼�����Ľض���Ϣ
					if (truncatedDimPairItm.first.second)
						spdlog::get("all")->info("right top truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
					else
						spdlog::get("all")->info("left top truncated length:{} at y coordinate {}", tctedLen, pseudoDouble(reinHLine.s.y));
#endif
				}
				//�ײ�
				if (reinHLine.s.y < boundaryY)
				{
					//������泤��
					accessSpanPos(spanItm).btmTruncatedLength.insert(std::make_pair(pseudoDouble(reinHLine.s.y), tctedLen));
#if 0
					//��¼�����Ľض���Ϣ
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
	// �����߳���Ϊ�뾶���������������ı���
	std::vector<int> tVec1, tVec2;
	findText(tVec1, data, this->profileLineIdxPair.first);
	findText(tVec2, data, this->profileLineIdxPair.second);
	if (tVec1.empty() || tVec2.empty())
		return false;

	bool bFound = false;
	//���ҵ����ı��㣬������֤���õ���Ҫ����Ե��ı���
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
	//���յ���
	auto tp = data.m_kdtTreeData.kdtTexts().radiusSearch(line.e, lineLen);
	if (!tp.empty())
		_textPtIdxVec = std::move(tp);
	//�������
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
	//!�������Χ����
	const auto refIdx = this->components.front()->lineIdx;
	const auto& refLine = data->m_lineData.lines().at(refIdx);
	const auto& offsetV = this->spanHeight() / 4;
	double deltaDis = offsetV;
	double spanVSize = this->spanHeight();
	//�����ӹ�
	if (refLine.s.y > bottom + spanVSize / 2)
	{
		const auto uY = refLine.s.y + offsetV;//upper Y
		const auto bY = refLine.s.y;// bottom Y
		const int uIdx = data->m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, uY);
		const int bIdx = data->m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bY);
		for (int i = bIdx; i <= uIdx; i++)
		{
			const int hIdx = data->m_lineData.hLinesIndices().at(i);
			//��������
			if (hIdx == refIdx)
				continue;
			//���˱�ע����
			const auto& outerLine = data->m_lineData.lines().at(hIdx);
			if (outerLine.type == Line::Type::DIMENSIONS)
				continue;
			//�Ϸ��������
			if (outerLine.getLayer() != refLine.getLayer() &&
				outerLine.s.y > refLine.s.y)
			{
				if (outerIdx == 0)
				{
					outerIdx = hIdx;
				}
				else
				{
					//ɸѡ
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
	//�ײ��ӹ�
	else if (refLine.s.y < bottom + spanVSize / 2)
	{
		const auto uY = refLine.s.y - Precision;
		const auto bY = refLine.s.y - offsetV;
		const int uIdx = data->m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, uY);
		const int bIdx = data->m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, bY);
		for (int i = bIdx; i <= uIdx; i++)
		{
			const int hIdx = data->m_lineData.hLinesIndices().at(i);
			//��������
			if (hIdx == refIdx)
				continue;
			//���˱�ע����
			const auto& outerLine = data->m_lineData.lines().at(hIdx);
			if (outerLine.type == Line::Type::DIMENSIONS)
				continue;
			//�Ϸ��������
			if (outerLine.getLayer() != refLine.getLayer() &&
				outerLine.s.y < refLine.s.y)
			{
				if (outerIdx == 0)
					outerIdx = hIdx;
				else
				{
					//ɸѡ
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
		//��λ��������
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

