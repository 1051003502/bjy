#include"elevationSymbolSet.h"
#include "publicFunction.h"

void ElevationSymbolSet::setMFCPaintTestVec(std::vector<int>& temp_idx, std::vector<Line>& temp_lines)
{
	this->pIdxVec = &temp_idx;

}

void ElevationSymbolSet::setAnalysisDependencise(std::shared_ptr<Data> m_data)
{
	data = m_data;
}

void ElevationSymbolSet::parse(std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols,
							   std::vector<std::shared_ptr<Block>>& _blocks)
{
	for (auto& blockItm : _blocks)
	{
		if (bValidBlock(blockItm))
			parseBlock(_elevationSymbols, blockItm);
	}
	sortElevation(_elevationSymbols);
}

void ElevationSymbolSet::sortElevation(std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols)
{
	if (!_elevationSymbols.empty())
	{
		std::sort(_elevationSymbols.begin(),
				  _elevationSymbols.end(),
				  [](const std::shared_ptr<ElevationSymbol>& ele1, const std::shared_ptr<ElevationSymbol>& ele2)
		{
			return ele1->pt.y < ele2->pt.y;
		});
	}
}

bool ElevationSymbolSet::parseBlock(std::vector<std::shared_ptr<ElevationSymbol>>& elevationSymbols,
									std::shared_ptr<Block>& block)
{
	//查找斜的剖线
	double min[2]{ block->box.left,block->box.bottom };
	double max[2]{ block->box.right,block->box.top };
	std::vector<int> result;
	auto num = data->m_rtreeData.sLines().Search(min, max,
												 std::bind(&RTreeData::pushBackSBackIndex, &data->m_rtreeData, std::placeholders::_1));
	const auto& slashLineIdxVec = data->m_rtreeData.SBackIndex();

	std::shared_ptr<ElevationSymbol> elevationSymbol;
	std::vector<int> visitedValidIdxVec;
	for (const auto slashLineIdxItm : slashLineIdxVec)
	{
		if (std::find(visitedValidIdxVec.begin(), visitedValidIdxVec.end(), slashLineIdxItm) != visitedValidIdxVec.end())
			continue;

		const auto& slashLine = data->m_lineData.lines().at(slashLineIdxItm);
		const double slashLineRad = slashLine.rad();
		if (abs(abs(slashLineRad) - M_PI_4) < 1e-2)
		{

			std::vector<int> horizontalIdxVec;
			bool bFound = false;
			const auto& cornerIt = data->m_cornerData.corners().find(slashLineIdxItm);
			for (const auto& cornerItm : cornerIt->second)
			{
				const auto crossIdx = cornerItm.l1 == slashLineIdxItm ? cornerItm.l2 : cornerItm.l1;
				const auto& crossLine = data->m_lineData.lines().at(crossIdx);
				if (crossLine.horizontal())
				{
					horizontalIdxVec.push_back(crossIdx);
					continue;
				}
				else if (crossLine.vertical())
				{
					continue;
				}

				const auto crossLineRad = crossLine.rad();
				//相互垂直
				if (slashLine.perpendicularTo(crossLine) &&
					bConnectTowLine(slashLine, crossLine) &&
					abs(slashLine.length() - crossLine.length()) < 1e-3)
				{
#if 0
					pIdxVec->push_back(slashLineIdxItm);
					pIdxVec->push_back(crossIdx);
#endif

					elevationSymbol = std::make_shared<ElevationSymbol>();
					elevationSymbol->pairSlash.first = slashLineIdxItm;
					elevationSymbol->pairSlash.second = crossIdx;
					visitedValidIdxVec.push_back(slashLineIdxItm);
					visitedValidIdxVec.push_back(crossIdx);
					bFound = true;
			}
		}
			if (bFound)
			{

				// 查找合适的标高横线
				if (elevationSymbol->findHorizontalIdx(horizontalIdxVec, data))
				{
					//pIdxVec->push_back(elevationSymbol->horizontalIdx);
				}
				elevationSymbol->updateElevationPoint(data);
				if (elevationSymbol->findElevationDescription(data))
					spdlog::get("all")->info("found text :{}", elevationSymbol->elevationDescription);
				else
				{
					spdlog::get("all")->info("not found any text");
				}
				elevationSymbols.push_back(elevationSymbol);
			}
	}
}
	data->m_rtreeData.clearSBackIndex();


	return true;
}

bool ElevationSymbolSet::bValidBlock(std::shared_ptr<Block>& block)
{
	if (!block->m_axisVLines.empty() &&
		block->m_axisHLines.empty())
		return true;
	return false;
}

int ElevationSymbolSet::getClosestElevationSymbol(double yTick, std::vector<std::shared_ptr<ElevationSymbol>>& elevationSymbols)
{
	auto frontEle = elevationSymbols.begin();
	auto backEle = elevationSymbols.end() - 1;
	auto yTickCoordinate = yTick;
	auto lowCoordinate = (*frontEle)->pt.y;
	auto hightCoordinate = (*backEle)->pt.y;
	if (yTick <= lowCoordinate)
		return std::distance(elevationSymbols.begin(), frontEle);
	else if (yTickCoordinate >= hightCoordinate)
		return std::distance(elevationSymbols.begin(), backEle);
	else
	{
		auto midEle = elevationSymbols.begin() + elevationSymbols.size() / 2;
		while (backEle - frontEle > 1)
		{
			if (yTickCoordinate <= (*midEle)->pt.y)
				backEle = midEle;
			else
				frontEle = midEle;
			midEle = frontEle + (backEle - frontEle) / 2;
		}
		if (abs((*frontEle)->pt.y - yTickCoordinate) < abs((*backEle)->pt.y - yTickCoordinate))
			return std::distance(elevationSymbols.begin(), frontEle);
		else
			return std::distance(elevationSymbols.begin(), backEle);
	}
	return -1;
}
