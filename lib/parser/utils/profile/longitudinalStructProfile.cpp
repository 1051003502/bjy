#include "longitudinalStructProfile.h"
#include "publicFunction.h"


void LongitudinalStructProfile::setMFCPaintTestVec(std::vector<int>& temp_idx, std::vector<Line>& temp_lines)
{
	this->pIdxVec = &temp_idx;
	this->pLineVec = &temp_lines;
}

void LongitudinalStructProfile::setAnalysisDependencise(const std::shared_ptr<Data>& prerequiseData,
														ElevationSymbolSet& _elevationSymbolSet,
														std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols)
{
	this->data = prerequiseData;
	this->elevationSymbolSet = &_elevationSymbolSet;
	this->elevationSymbols = &_elevationSymbols;
}


int LongitudinalStructProfile::getTheValidBlock(const std::vector<std::shared_ptr<Block>>& blocks)
{
	for (auto it = blocks.begin(); it != blocks.end(); ++it)
	{
		const auto& block = *it;
		if (!block->m_axisVLines.empty() &&
			block->m_axisHLines.empty())
		{
			return std::distance(blocks.begin(), it);
		}
	}
	return -1;
}

void LongitudinalStructProfile::iniSideVerticalDimVec(const std::vector<Axis::AxisLine>& vAxisVec, const  std::vector<unsigned>& dimIdxVec)
{
	const auto leftBound = vAxisVec.front().second.s.x;
	const auto rightBound = vAxisVec.back().second.s.x;
	for (auto idxItm : dimIdxVec)
	{
		const auto& refLine = data->m_dimensions.at(idxItm)->lines.at(2);
		if (refLine.vertical())
		{
			if (refLine.s.x < leftBound)
			{
				leftDimIdxVec.push_back(idxItm);
			}
			if (refLine.s.x > rightBound)
			{
				rightDimIdxVec.push_back(idxItm);
			}
		}
	}
}

std::vector<int> LongitudinalStructProfile::getSideVerticalIdx(const std::shared_ptr<Block>& block) const
{
	const auto leftAxisBound = block->m_axisVLines.front().second.s.x;
	const auto rightAxisBound = block->m_axisVLines.back().second.s.x;

	std::vector<int> result;
	for (auto i = block->box.vf; i < block->box.vt; ++i)
	{
		const auto verticalIdx = data->m_lineData.vLinesIndices().at(i);
		const auto& verticalLine = data->m_lineData.lines().at(verticalIdx);
		const auto xTick = verticalLine.s.x;
		if (xTick <= leftAxisBound ||
			xTick >= rightAxisBound)
		{
			result.push_back(verticalIdx);
		}
	}
	return result;
}

bool LongitudinalStructProfile::findBoardLineIdx()
{
	for (auto i = block->box.vf; i <= block->box.vt; i++)
	{
		auto vIdx = data->m_lineData.vLinesIndices().at(i);
		auto& line = data->m_lineData.lines().at(vIdx);
		if (line.getLayer() == boardLayer)
		{
			//pIdxVec->push_back(vIdx);
			boardLineIdxVec.push_back(vIdx);
		}
	}
	for (auto i = block->box.hf; i <= block->box.ht; i++)
	{
		auto vIdx = data->m_lineData.hLinesIndices().at(i);
		auto& line = data->m_lineData.lines().at(vIdx);
		if (line.getLayer() == boardLayer)
		{
			//pIdxVec->push_back(vIdx);
			boardLineIdxVec.push_back(vIdx);
		}
	}

	double min[2]{ block->box.left, block->box.bottom };
	double max[2]{ block->box.right, block->box.top };
	std::vector<int> result;
	auto num = data->m_rtreeData.sLines().Search(min, max,
												 std::bind(&RTreeData::pushBackSBackIndex, &data->m_rtreeData,
														   std::placeholders::_1));
	const auto& slashLineIdxVec = data->m_rtreeData.SBackIndex();
	for (const auto slashLineIdxItm : slashLineIdxVec)
	{
		const auto& slashLine = data->m_lineData.lines().at(slashLineIdxItm);
		if (slashLine.getLayer() == boardLayer)
		{
			//pIdxVec->push_back(slashLineIdxItm);
			boardLineIdxVec.push_back(slashLineIdxItm);
		}
	}
	if (boardLineIdxVec.empty())
		return false;
	return true;
}

bool LongitudinalStructProfile::findBoardLayer()
{
	// 获取两边y刻度上的
	std::set<long long> yTickSet;
	for (const auto dimIdxItm : leftDimIdxVec)
	{
		const auto& refLineInDim = data->m_dimensions.at(dimIdxItm)->lines.at(2);
		yTickSet.insert(yTickSet.end(), pseudoDouble(refLineInDim.s.y));
		yTickSet.insert(yTickSet.end(), pseudoDouble(refLineInDim.e.y));
	}

	for (const auto dimIdxItm : rightDimIdxVec)
	{
		const auto& refLineInDim = data->m_dimensions.at(dimIdxItm)->lines.at(2);
		yTickSet.insert(yTickSet.end(), pseudoDouble(refLineInDim.s.y));
		yTickSet.insert(yTickSet.end(), pseudoDouble(refLineInDim.e.y));
	}
	//去掉最大和最小
	auto yTickIt = yTickSet.begin();
	std::advance(yTickIt, yTickSet.size() - 1);
	yTickSet.erase(yTickIt);
	yTickIt = yTickSet.begin();
	yTickSet.erase(yTickIt);

	//加权y刻度
	std::map<std::string, int> condidateLayerMap;
	std::map<long long, std::string, std::greater<>> lenLayerMap;
	for (auto i = block->box.hf; i <= block->box.ht; ++i)
	{
		const auto hIdx = data->m_lineData.hLinesIndices().at(i);
		const auto& hLine = data->m_lineData.lines().at(hIdx);
		if (hLine.type == Line::DIMENSIONS)
			continue;

		const auto checkYTick = pseudoDouble(hLine.s.y);
		if (yTickSet.find(checkYTick) != yTickSet.end())
		{
			if (hLine.getLayer() != dimLayer)
			{
				condidateLayerMap[hLine.getLayer()]++;
				lenLayerMap[pseudoDouble(hLine.length())] = hLine.getLayer();
			}
		}
	}
	//加权长度
	auto endIt = lenLayerMap.begin();
	std::advance(endIt, static_cast<int>(lenLayerMap.size() * 0.6));
	for (auto it = lenLayerMap.begin(); it != endIt; ++it)
		condidateLayerMap[it->second]++;

	//加权两侧竖线
	std::vector<int> sideIdxVec = getSideVerticalIdx(block);
	for (auto& it : sideIdxVec)
	{
		const auto& verticalLine = data->m_lineData.lines().at(it);
		if (verticalLine.getLayer() != dimLayer)
			condidateLayerMap[verticalLine.getLayer()] += 5;
	}

	int maxCount = 0;
	for (auto& mapIt : condidateLayerMap)
	{
		//spdlog::get("all")->info("layer found:{},count:{}", mapIt.first, mapIt.second);
		if (maxCount < mapIt.second)
		{
			maxCount = mapIt.second;
			boardLayer = mapIt.first;
		}
	}
	spdlog::get("all")->info("layer found:{}", boardLayer);
	return true;
}

bool LongitudinalStructProfile::initDimLayer()
{
	if (rightDimIdxVec.empty())
		return false;
	const auto& sampleDimRefLine = data->m_dimensions.at(leftDimIdxVec.front())->lines.at(2);
	dimLayer = sampleDimRefLine.getLayer();
	return true;
}


bool LongitudinalStructProfile::parseBoard()
{
	findBoardLayer();
	if (!findBoardLineIdx())
		return false;

	std::vector<int> hLineIdxVec;
	std::vector<int> vLineIdxVec;

	for (auto idxItm : boardLineIdxVec)
	{
		const auto& l = data->m_lineData.lines().at(idxItm);
		l.horizontal() ? hLineIdxVec.push_back(idxItm) : vLineIdxVec.push_back(idxItm);
	}
	//分析水平线，找出板
	std::shared_ptr<LongitudinalBoard> board;
	for (auto hLineIdxItm1 : hLineIdxVec)
	{
		const auto& refLine = data->m_lineData.lines().at(hLineIdxItm1);
		board = std::make_shared<LongitudinalBoard>();
		board->boardTopIdx = hLineIdxItm1;

		std::vector<int> condidateIdx;
		for (auto hLineIdxItm2 : hLineIdxVec)
		{
			if (hLineIdxItm1 == hLineIdxItm2)
				continue;

			const auto& checkLine = data->m_lineData.lines().at(hLineIdxItm2);
			if (bHorizontalCross(refLine, checkLine))
				condidateIdx.push_back(hLineIdxItm2);
		}

		if (buildBoard(condidateIdx, refLine, board))
		{
			longitudianlBoards.push_back(board);
		}
	}
	//todo 分析竖直线，找出墙



#if 0
	for (const auto& boardItm : longitudianlBoards)
	{

		for (const auto& lineTp : boardItm->boardLineVec)
		{
			pLineVec->push_back(lineTp);
		}
		spdlog::get("all")->info("board side idx pair:{},{}", boardItm->vec1.front(), boardItm->vec2.front());
	}
#endif

	//解析出板的每个边线的标高
	if (!installElevationForBoards())
		return false;
	return true;
}

bool LongitudinalStructProfile::buildBoard(const std::vector<int>& condidateIdx,
										   const Line& refLine,
										   std::shared_ptr<LongitudinalBoard>& board)
{
	const auto& l1 = data->m_lineData.lines().at(board->boardTopIdx);
	auto minThickness = getMinVerticalDist(condidateIdx, l1);
	adjustDynamicThickness(minThickness);
	for (const auto hIdxItm : condidateIdx)
	{
		const auto& line = data->m_lineData.lines().at(hIdxItm);
		const auto thickness = pseudoDouble(abs(refLine.s.y - line.s.y));

		if (minThickness == thickness &&
			thickness <= dynamicThickness)
		{
			//board->vec2.push_back(hIdxItm);
			board->boardBtmIdx = hIdxItm;
			auto findIt = std::find_if(longitudianlBoards.begin(), longitudianlBoards.end(), [this, &board](const std::shared_ptr<LongitudinalBoard>& b)
			{

				if ((board->boardTopIdx == b->boardTopIdx && board->boardBtmIdx == b->boardBtmIdx) ||
					(board->boardBtmIdx == b->boardTopIdx && board->boardTopIdx == b->boardBtmIdx))
				{
					return true;
				}
				return false;
			});
			if (findIt != longitudianlBoards.end())
				return false;

			const auto& l2 = data->m_lineData.lines().at(board->boardBtmIdx);

			board->thickness = abs(l1.s.y - l2.s.y);
			//保证板顶线在vec1
			if (l1.s.y < l2.s.y)
				std::swap(board->boardTopIdx, board->boardBtmIdx);

			//短线范围
			auto left = std::max(l1.s.x, l2.s.x);
			auto right = std::min(l1.e.x, l2.e.x);
			board->boardLineVec.emplace_back(Point(left, l1.s.y), Point(right, l2.e.y));
			board->boardLineVec.emplace_back(Point(left, l2.s.y), Point(right, l1.e.y));

			//长线范围box
			board->box.expandH(std::min(l1.s.x, l2.s.x));
			board->box.expandH(std::max(l1.e.x, l2.e.x));
			board->box.expandV(std::min(l1.s.y, l2.s.y));
			board->box.expandV(std::max(l1.e.y, l2.e.y));
			return true;
		}
	}
	return false;
}

long long LongitudinalStructProfile::getMinVerticalDist(const std::vector<int>& condidateIdx, const Line& refLine)
{
	auto minThickness = std::numeric_limits<long long>::max();

	for (const auto hIdxItm : condidateIdx)
	{
		const auto& tpHLine = data->m_lineData.lines().at(hIdxItm);
		const auto thickness = pseudoDouble(abs(refLine.s.y - tpHLine.s.y));
		if (minThickness > thickness)
			minThickness = thickness;
	}
	return minThickness;
}


bool bHorizontalCrossOfTwoLines(const Line& refLine, const Line& checkLine)
{
	return refLine.s.x < checkLine.e.x&& checkLine.s.x < refLine.e.x;
}

bool LongitudinalStructProfile::bHorizontalCross(const Line& refLine, const Line& checkLine)
{
	// 判断是否有相交部分
	if (bHorizontalCrossOfTwoLines(refLine, checkLine))
	{
		const auto interArea = std::min(refLine.e.x, checkLine.e.x) -
			std::max(refLine.s.x, checkLine.s.x);
		if (refLine.length() > checkLine.length())
		{
			if (interArea / checkLine.length() > 0.5)
				return true;
		}
		else
		{
			if (interArea / refLine.length() > 0.5)
				return true;
		}
	}
	return false;
}

void LongitudinalStructProfile::adjustDynamicThickness(long long minThickness)
{
	//调整动态的厚度
	if (dynamicThickness == 0)
		dynamicThickness = minThickness * 2;
	else
	{
		if (dynamicThickness * 1.5 > minThickness)
		{
			double rate = static_cast<double>(minThickness * 1.5) / dynamicThickness;
			dynamicThickness += static_cast<long long>((rate - 1) * dynamicThickness);
		}
	}
}


bool LongitudinalStructProfile::getBlock(const std::vector<std::shared_ptr<Block>>& blocks)
{
	const auto blockIdx = getTheValidBlock(blocks);
	if (blockIdx == -1)
		return false;
	block = blocks.at(blockIdx);
	return true;
}

bool LongitudinalStructProfile::parse(const std::vector<std::shared_ptr<Block>>& blocks)
{
	if (!getBlock(blocks))
		return false;

	if (!iniDimMap())
		return false;

	if (!parseBoard())
		return false;

	if (!parseFloor())
		return false;

	if (!parseBeamOutline())
		return false;

	return true;
}

bool LongitudinalStructProfile::getBlockDimensions(std::vector<unsigned int>& _dimIdxVec,
												   const Box& box) const
{
	for (auto it = data->m_dimensions.begin(); it != data->m_dimensions.end(); ++it)
	{
		const auto& dim = *it;
		if (box.cover(dim->box))
		{
			_dimIdxVec.push_back(std::distance(data->m_dimensions.begin(), it));
		}
	}
	return true;
}

bool LongitudinalStructProfile::buildDimDesMap(const std::vector<unsigned>& dimIdxVec)
{
	for (auto dimIdxItm : dimIdxVec)
	{
		const auto& refLineInDim = data->m_dimensions.at(dimIdxItm)->lines.at(2);
		if (refLineInDim.vertical())
		{
			const auto tickS = pseudoDouble(refLineInDim.s.y);
			const auto tickE = pseudoDouble(refLineInDim.e.y);
			if (tickS < tickE)
			{
				vDimDesMap[tickS].emplace_back(dimIdxItm, true);//上增
				vDimDesMap[tickE].emplace_back(dimIdxItm, false);
			}
			else
			{
				vDimDesMap[tickS].emplace_back(dimIdxItm, false);
				vDimDesMap[tickE].emplace_back(dimIdxItm, true);
			}
		}
	}
	return true;
}

bool LongitudinalStructProfile::iniDimMap()
{
	std::vector<unsigned int> dimIdxVec;
	getBlockDimensions(dimIdxVec, block->box);

	if (dimIdxVec.empty())
		return false;

	iniSideVerticalDimVec(block->m_axisVLines, dimIdxVec);
	if (!initDimLayer())
		return false;

	buildDimDesMap(dimIdxVec);
	return true;
}

bool LongitudinalStructProfile::findBeamOutlineLayer()
{
	if (boardLayer.empty())
		return false;

	std::map<std::string, int> layerCounterMap;
	std::set<int> excludedIdxSet;
	for (const auto& boardItm : longitudianlBoards)
	{
		std::pair <std::string, int> layerCounter;
		traverseNonBoardConnectedLineChain(boardItm->boardTopIdx, true, layerCounterMap, excludedIdxSet);
		traverseNonBoardConnectedLineChain(boardItm->boardBtmIdx, true, layerCounterMap, excludedIdxSet);
	}
	for (const auto& layerCounterItm : layerCounterMap)
	{
		spdlog::get("all")->info("layer :{},count:{}", layerCounterItm.first, layerCounterItm.second);
	}
	//获取梁线轮廓的图层
	int maxCount = 0;
	for (const auto& layerIt : layerCounterMap)
	{
		if (maxCount < layerIt.second)
		{
			maxCount = layerIt.second;
			beamOutlineLayer = layerIt.first;
		}
	}
	if (beamOutlineLayer.empty())
		return false;

	return true;
}


bool LongitudinalStructProfile::traverseNonBoardConnectedLineChain(const int idx,
																   bool firstLevel,
																   std::map<std::string, int>& layerCounterMap,
																   std::set<int>& excludedIdxSet)
{
	const auto& refLine = data->m_lineData.lines().at(idx);
	const auto cornerIt = data->m_cornerData.corners().find(idx);
	for (const auto& cornerItm : cornerIt->second)
	{
		const auto crossIdx = idx == cornerItm.l1 ? cornerItm.l2 : cornerItm.l1;
		const auto crossLine = data->m_lineData.lines().at(crossIdx);
		if (excludedIdxSet.find(crossIdx) != excludedIdxSet.end() &&
			crossLine.getLayer() == dimLayer)
			continue;


		if (firstLevel)
		{
			if (crossLine.getLayer() != boardLayer &&
				!crossLine.vertical())
			{
				auto y = knowXToY(refLine, crossLine.s.x);
				if (abs(y - crossLine.s.y) < 1)
				{
					//pIdxVec->push_back(crossIdx);
					excludedIdxSet.insert(excludedIdxSet.end(), crossIdx);
					++layerCounterMap[crossLine.getLayer()];
					traverseNonBoardConnectedLineChain(crossIdx, false, layerCounterMap, excludedIdxSet);
				}
				y = knowXToY(refLine, crossLine.e.x);
				if (abs(y - crossLine.s.y) < 1)
				{
					//pIdxVec->push_back(crossIdx);
					excludedIdxSet.insert(excludedIdxSet.end(), crossIdx);
					++layerCounterMap[crossLine.getLayer()];
					traverseNonBoardConnectedLineChain(crossIdx, false, layerCounterMap, excludedIdxSet);
				}
			}
		}
		else if (bConnectTowLine(crossLine, refLine))
		{
			if (crossLine.getLayer() == refLine.getLayer())
			{
				//pIdxVec->push_back(crossIdx);
				++layerCounterMap[crossLine.getLayer()];
			}
		}
	}
	return true;
}

bool LongitudinalStructProfile::findBeamOutlineIdx(std::vector<int>& beamOutlineIdxVec)
{
	for (auto i = block->box.vf; i <= block->box.vt; i++)
	{
		auto vIdx = data->m_lineData.vLinesIndices().at(i);
		auto& line = data->m_lineData.lines().at(vIdx);
		if (line.getLayer() == beamOutlineLayer)
		{
			//pIdxVec->push_back(vIdx);
			beamOutlineIdxVec.push_back(vIdx);
		}
	}
	for (auto i = block->box.hf; i <= block->box.ht; i++)
	{
		auto vIdx = data->m_lineData.hLinesIndices().at(i);
		auto& line = data->m_lineData.lines().at(vIdx);
		if (line.getLayer() == beamOutlineLayer)
		{
			//pIdxVec->push_back(vIdx);
			beamOutlineIdxVec.push_back(vIdx);
		}
	}

	double min[2]{ block->box.left, block->box.bottom };
	double max[2]{ block->box.right, block->box.top };
	std::vector<int> result;
	auto num = data->m_rtreeData.sLines().Search(min, max,
												 std::bind(&RTreeData::pushBackSBackIndex, &data->m_rtreeData,
														   std::placeholders::_1));
	const auto& slashLineIdxVec = data->m_rtreeData.SBackIndex();
	for (const auto slashLineIdxItm : slashLineIdxVec)
	{
		const auto& slashLine = data->m_lineData.lines().at(slashLineIdxItm);
		if (slashLine.getLayer() == beamOutlineLayer)
		{
			//pIdxVec->push_back(slashLineIdxItm);
			beamOutlineIdxVec.push_back(slashLineIdxItm);
		}
	}

	if (beamOutlineIdxVec.empty())
		return false;
	return true;
}

bool LongitudinalStructProfile::installElevationForBoards()
{
	for (auto& boardItm : longitudianlBoards)
	{
		boardItm->ele1 = getEle(boardItm->boardTopIdx);
		boardItm->ele2 = getEle(boardItm->boardBtmIdx);
	}
	return true;
}

double LongitudinalStructProfile::getEle(const int& lineIdx)
{
	const auto& line = data->m_lineData.lines().at(lineIdx);
	auto eleSymbolIdx = elevationSymbolSet->getClosestElevationSymbol(line.s.y, *elevationSymbols);
	const auto ele = elevationSymbols->at(eleSymbolIdx);
	std::stringstream ss;
	ss << ele->elevationDescription;
	double eleVal;
	ss >> eleVal;
	return strimDouble(eleVal + (line.s.y - ele->pt.y) / 1000);
}

bool LongitudinalStructProfile::parseFloor()
{
	std::map<int, std::list<int>> idxChain;

	if (!findConnectedBoardsMap(idxChain))
		return false;

	if (!buildFloors(idxChain))
		return false;

	//找出主板
	for (auto& flrItm : longitudinalFloors)
	{
		findBoardMainIdx(flrItm);
		//对每个楼层中所有的板排序
		std::sort(flrItm->boardIdxVec.begin(), flrItm->boardIdxVec.end(), [this](int idx1, int idx2)
		{
			const auto& l1 = longitudianlBoards.at(idx1)->boardLineVec.front();
			const auto& l2 = longitudianlBoards.at(idx2)->boardLineVec.front();
			return l1.s.x < l2.s.x;
		});
	}
	return true;
}


bool LongitudinalStructProfile::buildFloors(std::map<int, std::list<int>>& idxChain)
{
	auto idxChainIt = idxChain.begin();
	std::set<int> bVisitedSet;
	while (bVisitedSet.size() != idxChain.size())
	{
		while (bVisitedSet.find(idxChainIt->first) != bVisitedSet.end())
			std::advance(idxChainIt, 1);

		auto it = idxChainIt;
		auto floor = std::make_shared<LongitudinalFloor>();
		initFloor(floor, it->second);
		bVisitedSet.insert(bVisitedSet.end(), it->first);
		std::advance(it, 1);
		for (auto& chainItm : idxChain)
		{
			if (bVisitedSet.find(chainItm.first) != bVisitedSet.end())
				continue;
			const auto& condVec = chainItm.second;
			if (appendBoardIdxVec2Floor(floor, condVec))
			{
				bVisitedSet.insert(bVisitedSet.end(), chainItm.first);
			}
		}
		longitudinalFloors.push_back(floor);
		idxChainIt = it;
	}
	if (longitudinalFloors.empty())
		return false;

	//从上到下对楼层排序
	std::sort(longitudinalFloors.begin(), longitudinalFloors.end(),
			  [this](const std::shared_ptr<LongitudinalFloor>& flr1,
					 const std::shared_ptr<LongitudinalFloor>& flr2)
	{
		return flr1->box.top + flr1->box.bottom > flr2->box.top + flr2->box.bottom;
	});


#if 0
	for (const auto& boardIdxItm : longitudinalFloors.at(2)->boardIdxVec)
	{
		const auto& b = longitudianlBoards.at(boardIdxItm);
		pLineVec->push_back(Line(Point(b->box.left, b->box.top), Point(b->box.right, b->box.bottom)));
	}
#endif
	return true;
}

void LongitudinalStructProfile::initFloor(std::shared_ptr<LongitudinalFloor>& flr, std::list<int>& second)
{
	for (const auto idxItm : second)
	{
		const auto& b = longitudianlBoards.at(idxItm);
		flr->appendBoardIdx(idxItm, b);
	}
}

bool LongitudinalStructProfile::appendBoardIdxVec2Floor(std::shared_ptr<LongitudinalFloor>& floor,
														const std::list<int>& condVec)
{
	bool bCheck = checkBoardIdxForFloor(condVec, floor);
	if (bCheck)
	{
		for (const auto idxItm : condVec)
		{
			floor->appendBoardIdx(idxItm, longitudianlBoards.at(idxItm));
		}
	}
	return bCheck;
}

bool LongitudinalStructProfile::checkBoardIdxForFloor(const std::list<int>& condVec,
													  std::shared_ptr<LongitudinalFloor>& floor)
{
	for (const auto condIdxItm : condVec)
	{
		const auto& b = longitudianlBoards.at(condIdxItm);
		if (floor->checkBoard(b))
			return true;
	}
	return false;
}

void LongitudinalStructProfile::findBoardMainIdx(std::shared_ptr<LongitudinalFloor>& flrItm)
{
	if (flrItm->boardIdxVec.empty())
		return;

	double maxFactor = -1;
	for (const auto& boardIdxItm : flrItm->boardIdxVec)
	{
		const auto& b = longitudianlBoards.at(boardIdxItm);
		auto calcFactor = b->boardLineVec.front().length() / flrItm->box.width();
		if (calcFactor > maxFactor)
		{
			flrItm->mainIdx = boardIdxItm;
			maxFactor = calcFactor;
		}
	}

#if 0
	const auto& b = longitudianlBoards.at(flrItm->mainIdx);
	for (const auto& l : b->boardLineVec)
	{
		pLineVec->push_back(l);
	}
#endif//画出主板

}

void LongitudinalStructProfile::appendBeamOutlineIdxVec2Floor(const int beamOutlineIdxItm)
{
	const auto& beamOutlineLine = data->m_lineData.lines().at(beamOutlineIdxItm);
	//过滤误识别的标注线
	if (beamOutlineLine.type == Line::Type::DIMENSIONS)
		return;

	int targetIdx = -1;
	double minDist = std::numeric_limits<double>::max();
	for (auto flrIt = longitudinalFloors.begin(); flrIt != longitudinalFloors.end(); ++flrIt)
	{
		auto& flr = *flrIt;
		const auto& box = flr->box;
		double checkYTick = beamOutlineLine.s.y;
		//梁的轮廓线在楼层的box之间
		if (box.top + Precision > checkYTick &&
			box.bottom - Precision < checkYTick)
		{
			targetIdx = std::distance(longitudinalFloors.begin(), flrIt);
			break;
		}
		//在box之外，找到离轮廓线最近的floor的box
		else
		{
			double checkDist = beamOutlineLine.s.y - box.top;
			const double minDist2Box = checkDist > 0 ? checkDist : box.bottom - beamOutlineLine.s.y;
			if (minDist2Box < minDist)
			{
				minDist = minDist2Box;
				targetIdx = std::distance(longitudinalFloors.begin(), flrIt);
			}
		}
	}

	if (targetIdx != -1)
		longitudinalFloors.at(targetIdx)->beamOutlineIdxVec.push_back(beamOutlineIdxItm);
}

bool LongitudinalStructProfile::bSameFloorCheck(std::shared_ptr<LongitudinalBoard>& pre,
												std::shared_ptr<LongitudinalBoard>& ne)
{
	int refIdx = pre->boardBtmIdx;
	const auto& cornerVecIt = data->m_cornerData.corners().find(refIdx);
	for (const auto& cornerItm : cornerVecIt->second)
	{
		const int crossIdx = cornerItm.l1 == refIdx ? cornerItm.l2 : cornerItm.l1;
		const auto& crossLine = data->m_lineData.lines().at(crossIdx);
		//斜线作为两平行线的中间线
		if (!crossLine.vertical() &&
			!crossLine.horizontal())
		{
			const auto& cornerVecIt2 = data->m_cornerData.corners().find(crossIdx);
			for (const auto& cornerItm2 : cornerVecIt2->second)
			{
				const auto crossIdx2 = cornerItm2.l1 == crossIdx ? cornerItm2.l2 : cornerItm2.l1;
				if (refIdx == crossIdx2) continue;

				const auto& crossLine2 = data->m_lineData.lines().at(crossIdx2);
				if (crossLine2.horizontal())
				{
					if (ne->boardTopIdx == crossIdx2 ||
						ne->boardBtmIdx == crossIdx2)
						return true;
				}
			}
		}
	}
	return false;
}

bool LongitudinalStructProfile::findConnectedBoardsMap(std::map<int, std::list<int>>& idxChain)
{
	std::sort(longitudianlBoards.begin(), longitudianlBoards.end(),
			  [this](std::shared_ptr<LongitudinalBoard> b1, std::shared_ptr<LongitudinalBoard> b2)
	{
		const auto& l1 = b1->boardLineVec.front();
		const auto& l2 = b2->boardLineVec.front();
		return l1.s.x < l2.s.x;
	});

	auto ne = longitudianlBoards.begin();
	auto begin = longitudianlBoards.begin();
	auto end = longitudianlBoards.end();
	std::vector<bool> bVisited(longitudianlBoards.size(), false);
	for (auto it = longitudianlBoards.begin(); it != longitudianlBoards.end(); ++it)
	{
		if (bVisited.at(it - begin))
			continue;

		auto pre = it;
		ne = pre + 1;
		idxChain[it - begin].push_back(static_cast<int>(it - begin));
		bVisited.at(it - begin) = true;
		while (ne != end && bVisited.at(ne - begin))
			ne++;

		while (ne != end)
		{
			while (ne != end && !bVisited.at(ne - begin) &&
				   !((*pre)->box.cross((*ne)->box) || bSameFloorCheck(*pre, *ne)))
			{
				++ne;
			}
			if (ne == end)
				break;
			if (ne != end && !bVisited.at(ne - begin))
			{
				bVisited.at(it - begin) = true;
				bVisited.at(ne - begin) = true;
				idxChain[it - begin].push_back(ne - begin);
				pre = ne;
			}
			++ne;
		}
	}


#if 0

#if 1
	for (const auto idx : idxChain.at(0))
	{
		const auto& b = longitudianlBoards.at(idx);
		pLineVec->push_back(Line(Point(b->box.left, b->box.top), Point(b->box.right, b->box.bottom)));
	}
#endif
	const auto& b = longitudianlBoards.at(0);
	pLineVec->push_back(Line(Point(b->box.left, b->box.top), Point(b->box.right, b->box.bottom)));
#endif

	if (idxChain.empty())
		return false;
	return true;
}



bool LongitudinalStructProfile::installRelativeElvationForBeam(std::shared_ptr<Beam> beam,
															   std::shared_ptr<LongitudinalBeamOutline> beamOutline)
{
	for (auto& spanItm : beam->spSpanVec)
	{
		const auto heightRef = std::stod(spanItm->strSecHeight, nullptr);
		for (const auto& boardIdx : beamOutline->boardIdxVec)
		{
			const auto& board = this->longitudianlBoards.at(boardIdx);
			const auto& beamOutlineMainLine = data->m_lineData.lines().at(beamOutline->mainIdx);
			if (beamOutline->settingType == LongitudinalBeamOutline::UP)
			{
				const auto& boardBtmLine = data->m_lineData.lines().at(board->boardBtmIdx);
				const auto heightCheck = abs(beamOutlineMainLine.s.y - boardBtmLine.s.y);
				if (abs(heightCheck - heightRef) < 1)
				{

					const auto& boardTopLine = data->m_lineData.lines().at(board->boardTopIdx);
					const auto relativeEle = abs(beamOutlineMainLine.s.y - boardTopLine.s.y) / 1000;
					if (relativeEle > spanItm->relativeElevation)
						spanItm->relativeElevation = relativeEle;
					return true;
				}
			}
			else if (beamOutline->settingType == LongitudinalBeamOutline::DOWN)
			{
				const auto& boardTopLine = data->m_lineData.lines().at(board->boardTopIdx);
				const auto heightCheck = abs(beamOutlineMainLine.s.y - boardTopLine.s.y);
				if (abs(heightCheck - heightRef) < 1)
				{

					const auto& boardBtmLine = data->m_lineData.lines().at(board->boardBtmIdx);
					const auto relativeEle = abs(beamOutlineMainLine.s.y - boardBtmLine.s.y) / 1000;
					if (relativeEle > spanItm->relativeElevation)
						spanItm->relativeElevation = relativeEle;
					return true;
				}
			}
		}
	}
	return true;
}

bool LongitudinalStructProfile::parseBeamOutline()
{
	if (!findBeamOutlineLayer())
		return false;

	//std::vector<int> beamOutlineIdxVec;
	if (!findBeamOutlineIdx(beamOutlineIdxVec))
		return false;


	//将轮廓线加入到相应的floor中
	for (const auto beamOutlineIdxItm : beamOutlineIdxVec)
		appendBeamOutlineIdxVec2Floor(beamOutlineIdxItm);

#if 0
	for (const auto& idx : longitudinalFloors.at(3)->beamOutlineIdxVec)
	{
		pIdxVec->push_back(idx);
	}
#endif//梁轮廓线



	//for (auto& flr : longitudinalFloors)
	auto& flr = longitudinalFloors.at(0);
	{
		//排序楼层中的梁轮廓线
		std::sort(flr->beamOutlineIdxVec.begin(), flr->beamOutlineIdxVec.end(), [this](int idx1, int idx2)
		{
			const auto& l1 = data->m_lineData.lines().at(idx1);
			const auto& l2 = data->m_lineData.lines().at(idx2);
			return l1.s.x < l2.s.x;
		});

		//对梁轮廓线分析
		for (const auto beamOutlineIdxItm : flr->beamOutlineIdxVec)
		{
			const auto& beamOutline = data->m_lineData.lines().at(beamOutlineIdxItm);
			//剔除额外的线
			if (beamOutline.horizontal())
			{
				//遍历楼层中的板
				for (const auto boardIdxItm : flr->boardIdxVec)
				{
					const auto& b = longitudianlBoards.at(boardIdxItm);
					//不包含在该板内
					if (!b->box.cross(beamOutline))
					{
						const auto& boardLine = b->boardLineVec.front();
						//水平交错
						if (beamOutline.s.x<boardLine.e.x - 1 && beamOutline.e.x>boardLine.s.x + 1)
						{
							if (!flr->beamOutlines.empty() &&
								flr->beamOutlines.back()->mainIdx == beamOutlineIdxItm)
							{
								flr->beamOutlines.back()->boardIdxVec.push_back(boardIdxItm);
								continue;
							}

							auto longitudinalBeamOutline = std::make_shared<LongitudinalBeamOutline>();
							longitudinalBeamOutline->mainIdx = beamOutlineIdxItm;
							//todo 查找梁轮廓线水平线两侧是否还有边线
							const bool bUpSetting = beamOutline.s.y - b->box.top > 0;
							longitudinalBeamOutline->settingType = bUpSetting ?
								LongitudinalBeamOutline::UP :
								LongitudinalBeamOutline::DOWN;
							//加入覆盖的板
							longitudinalBeamOutline->boardIdxVec.push_back(boardIdxItm);
							findClosestVAxis(longitudinalBeamOutline->start, block->m_axisVLines, beamOutline.s.x);
							findClosestVAxis(longitudinalBeamOutline->end, block->m_axisVLines, beamOutline.e.x);
							flr->beamOutlines.push_back(longitudinalBeamOutline);
							pIdxVec->push_back(beamOutlineIdxItm);
						}
					}
				}
			}
		}
	}
	return true;
}

void LongitudinalFloor::appendBoardIdx(const int boardIdx,
									   const std::shared_ptr<LongitudinalBoard>& board)
{
	box.expand(board->box);
	this->boardIdxVec.push_back(boardIdx);
}

std::shared_ptr<LongitudinalBeamOutline> LongitudinalFloor::findLongitudinalBeamOutline(std::shared_ptr<Beam> beam)
{
	const auto& startVAxis = beam->spSpanVec.front()->midlineStart.relativeVDistance.first;
	const auto& endVAxis = beam->spSpanVec.back()->midlineEnd.relativeVDistance.first;
	for (const auto& beamOutlineItm : this->beamOutlines)
	{
		if (beamOutlineItm->start.first == startVAxis &&
			beamOutlineItm->end.first == endVAxis)
		{
			return beamOutlineItm;
		}
	}
	return nullptr;
}

bool LongitudinalFloor::checkBoard(const std::shared_ptr<LongitudinalBoard>& board) const
{
	if (box.top > board->box.bottom && box.bottom < board->box.top)
		return true;
	return false;
}

