#include"elevationSymbol.h"

bool ElevationSymbol::updateElevationPoint(const std::shared_ptr<Data>& data)
{
	const auto& firstLine = data->m_lineData.lines().at(pairSlash.first);
	const auto& secondLine = data->m_lineData.lines().at(pairSlash.second);
	//first[e],second[s]:second[s],first[e]
	pt = firstLine.s.x < secondLine.s.x ? firstLine.e : firstLine.s;
	return true;
}

bool ElevationSymbol::findElevationDescription(const std::shared_ptr<Data>& data)
{
	if (horizontalIdx == -1)
		return false;

	const auto& horLine = data->m_lineData.lines().at(horizontalIdx);
	const std::vector<int>& ptIdxVec = data->m_kdtTreeData.kdtTexts().radiusSearch(
		Point((horLine.s.x + horLine.e.x) / 2, horLine.s.y), horLine.length() / 2);
	for (const auto& ptIdx : ptIdxVec)
	{
		const auto& t = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(ptIdx).entity);
		const auto place = t->text.find('.');
		if (place != std::string::npos)
		{
			elevationDescription = t->text;
			return true;
		}
	}
	return false;
}

bool ElevationSymbol::findHorizontalIdxImp(const std::vector<int>& horizontalIdxVec,
										   const std::shared_ptr<Data>& data,
										   const double y1,
										   const double y2)
{
	for (auto idxItm : horizontalIdxVec)
	{
		const auto line = data->m_lineData.lines().at(idxItm);
		const double lineY = line.s.y;
		if (abs(y1 - lineY) < 1 && abs(y2 - lineY) < 1)
		{
			horizontalIdx = idxItm;
			return true;
		}
	}
	return false;
}

bool ElevationSymbol::updateSlashPriority(const std::shared_ptr<Data>& data)
{
	if (horizontalIdx == -1)
	{
		return false;
	}
	bool result = false;
	const auto& firstLine = data->m_lineData.lines().at(pairSlash.first);
	const auto& secondLine = data->m_lineData.lines().at(pairSlash.second);
	const auto& horLine = data->m_lineData.lines().at(horizontalIdx);
	if ((firstLine.s.x - secondLine.s.x) > 1)
	{
		//second[s],first[e]
		if (ConnectPoint(horLine.s, secondLine.s))
		{
			std::swap(pairSlash.first, pairSlash.second);
			result = true;
		}
	}
	else
	{
		//first[s],second[e]
		if (ConnectPoint(horLine.s, secondLine.e))
		{
			std::swap(pairSlash.first, pairSlash.second);
			result = true;
		}
	}
	return result;
}

bool ElevationSymbol::findHorizontalIdx(const std::vector<int>& horizontalIdxVec, const std::shared_ptr<Data>& data)
{
	if (pairSlash.first == -1 || pairSlash.second == -1)
		return false;

	const auto& firstLine = data->m_lineData.lines().at(pairSlash.first);
	const auto& secondLine = data->m_lineData.lines().at(pairSlash.second);
	if ((firstLine.s.x - secondLine.s.x) > 1)
	{
		//second[s],first[e]
		const bool bCheck = findHorizontalIdxImp(horizontalIdxVec, data, firstLine.e.y, secondLine.s.y);
		if (bCheck)
		{
			//调整first idx及second idx
			updateSlashPriority(data);
			return true;
		}
	}
	else
	{
		//fisrt[s],second[e]
		const bool bCheck = findHorizontalIdxImp(horizontalIdxVec, data, firstLine.s.y, secondLine.e.y);
		if (bCheck)
		{
			//调整first idx及second idx
			updateSlashPriority(data);
			return true;
		}
	}

	return false;
}

