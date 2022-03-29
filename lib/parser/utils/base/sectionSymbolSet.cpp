#include"sectionSymbolSet.h"


bool SectionSymbolSet::parse(std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols,
									  std::vector<std::shared_ptr<Block>>& blocks)
{
	for (const auto& blockItm : blocks)
	{
		if (validateBlock(blockItm))
		{
			parseBlock(blockItm, sectionSymbols);
		}
	}
	return true;
}

bool SectionSymbolSet::parseBlock(std::shared_ptr<Block> block, std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols)
{
	//��������ͼ��
	if (!this->sectionSymbolLayer.empty() || !findSectionSymbolLayer(block))
		return false;

	//�������е�������
	std::vector<int> sectionLineIdxVec = findSectionLineIdx(block);

	//����ÿ�����������
	std::map<int, std::pair<int, int>> indicatorMap;//<line idx,line group idx,cross line idx>
	std::vector<SectionLineGroup> sectionLineGrpVec;
	for (const auto idxItm : sectionLineIdxVec)
	{
		// ���˷��ʹ���������
		auto indicatorIt = indicatorMap.find(idxItm);
		if (indicatorIt != indicatorMap.end())
			continue;

		const Line& tpLine = data->m_lineData.lines().at(idxItm);

		// ���˳���Ϊ0����
		if (tpLine.length() < 1)
			continue;

		const auto& corners = data->m_cornerData.corners().find(idxItm);
		// ���ڽ���
		if (corners != data->m_cornerData.corners().end())
		{
			//�Ƿ��ڸ����ļ������ҵ��ഹֱ����
			bool bFound = false;
			for (const auto& cornerItm : corners->second)
			{
				const auto crossIdx = cornerItm.l1 == idxItm ? cornerItm.l2 : cornerItm.l1;

				if (std::find(sectionLineIdxVec.begin(), sectionLineIdxVec.end(), crossIdx)
					== sectionLineIdxVec.end())
					continue;

				const auto& crossLine = data->m_lineData.lines().at(crossIdx);

				//������ӵ�
				if (!bConnectTowLine(tpLine, crossLine))
					continue;

				//�໥��ֱ
				if (tpLine.perpendicularTo(crossLine))
				{
					bFound = true;
					//���ȳ�
					if (abs(tpLine.length() - crossLine.length()) > Precision)
					{
						if (tpLine.length() > crossLine.length())
							sectionLineGrpVec.emplace_back(idxItm, crossIdx, SectionLineGroup::SectionDirection{});
						else  if (tpLine.length() < crossLine.length())
							sectionLineGrpVec.emplace_back(crossIdx, idxItm, SectionLineGroup::SectionDirection{});
					}
					//�ȳ�
					else
					{
						//�Դ�ֱ��Ϊ��׼
						if (tpLine.vertical())
							sectionLineGrpVec.emplace_back(idxItm, crossIdx, SectionLineGroup::SectionDirection{});
						else
							sectionLineGrpVec.emplace_back(crossIdx, idxItm, SectionLineGroup::SectionDirection{});
					}
					indicatorMap[idxItm] = std::pair<int, int>(sectionLineGrpVec.size(), -1);
					indicatorMap[crossIdx] = std::pair<int, int>(sectionLineGrpVec.size(), -1);
					break;
				}
				//�Ǵ�ֱ
				else
				{
					bFound = true;
					indicatorMap[idxItm] = std::pair<int, int>(sectionLineGrpVec.size(), crossIdx);
					sectionLineGrpVec.emplace_back(idxItm);
					indicatorMap[crossIdx] = std::pair<int, int>(sectionLineGrpVec.size(), idxItm);
					sectionLineGrpVec.emplace_back(crossIdx);
				}
			}
			if (bFound)
				continue;
		}
		indicatorMap[idxItm] = std::pair<int, int>(sectionLineGrpVec.size(), -1);
		sectionLineGrpVec.emplace_back(idxItm);
	}



	// ����ͬһ�ӳ��ߵ����������ͳ��
	std::map<int, std::vector<int>> groupedLineGroupMap;

	for (auto i = 0; i < sectionLineGrpVec.size(); ++i)
	{
		auto& refLineGrp1 = sectionLineGrpVec.at(i);
		const auto& refLine1 = data->m_lineData.lines().at(refLineGrp1.sectionBaseLineIdx);
		for (auto j = 0; j < sectionLineGrpVec.size(); ++j)
		{
			if (i == j)
				continue;

			auto& refLineGrp2 = sectionLineGrpVec.at(j);
			const auto& refLine2 = data->m_lineData.lines().at(refLineGrp2.sectionBaseLineIdx);
			if (bAtExtendedLine(refLine1, refLine2))
			{
				groupedLineGroupMap[i].push_back(j);
			}
			else if (refLineGrp1.isExistSecondDirectionLineIdx() &&
					 refLineGrp2.isExistSecondDirectionLineIdx())
			{
				const auto& refLine3 = data->m_lineData.lines().at(refLineGrp1.sectionDirectionLineIdx);
				const auto& refLine4 = data->m_lineData.lines().at(refLineGrp2.sectionDirectionLineIdx);
				if (bAtExtendedLine(refLine3, refLine4))
					groupedLineGroupMap[i].push_back(j);
			}
		}
	}


	//�����νӵ�������
	for (auto& mapItm : groupedLineGroupMap)
	{
		for (const auto subIdxItm : mapItm.second)
		{
			//�Ƿ�����֮���νӵ�������
			const auto& refLineGrp = sectionLineGrpVec.at(subIdxItm);
			const auto& indicatorMapIt = indicatorMap.find(refLineGrp.sectionBaseLineIdx);
			if (indicatorMapIt == indicatorMap.end() || indicatorMapIt->second.second == -1)
				continue;
			const auto& adjecentLineGrpIdx = indicatorMap.find(indicatorMapIt->second.second)->second.first;
			auto grpIt = groupedLineGroupMap.find(adjecentLineGrpIdx);
			if (grpIt == groupedLineGroupMap.end())
			{
				continue;
			}
			else
			{
				mapItm.second.push_back(grpIt->first);
				for (const auto idx : grpIt->second)
				{
					mapItm.second.push_back(idx);
				}
			}
			groupedLineGroupMap.erase(grpIt);
		}
	}



	//�������з���
	auto counter = 0;
	std::shared_ptr<SectionSymbolGroup> ssg;
	std::shared_ptr<SectionSymbol> ssHeader;
	std::vector<bool> sectionLineGrpVisitedFlagVec(sectionLineGrpVec.size(), false);
	auto mapIt = groupedLineGroupMap.begin();
	while (counter != sectionLineGrpVec.size())
	{
		if (mapIt == groupedLineGroupMap.end())break;
		if (sectionLineGrpVisitedFlagVec.at(mapIt->first))
		{
			++mapIt;
			continue;
		}

		//�������飬ֻ�����ѵ����ı�����ͷ��ͬ��
		auto& sLGHeader = sectionLineGrpVec.at(mapIt->first);
		ssHeader = std::make_shared<SectionSymbol>();
		ssHeader->sectionLineGroup = sLGHeader;
		ssg = std::make_shared<SectionSymbolGroup>();
		ssg->sectionSymbolVec.push_back(ssHeader);
		counter++;
		sectionLineGrpVisitedFlagVec.at(mapIt->first) = true;
		for (auto subIdxItm : mapIt->second)
		{
			if (sectionLineGrpVisitedFlagVec.at(subIdxItm))
				continue;

			auto& sub = sectionLineGrpVec.at(subIdxItm);
			ssHeader = std::make_shared<SectionSymbol>();
			ssHeader->sectionLineGroup = sub;
			//�Ƚ��������з�
			if (ssg->checkAndAdd(data, ssHeader))
			{
				counter++;
				sectionLineGrpVisitedFlagVec.at(subIdxItm) = true;
				//���������ߵ�����
				ssg->updateChainBaseIdx(data);
			}
		}
		sectionSymbols.push_back(ssg);
		++mapIt;
	}


	for (const auto& grpItm : sectionSymbols)
	{
		spdlog::get("all")->info("section group");
		for (auto& ssItm : grpItm->sectionSymbolVec)
		{
			spdlog::get("all")->info("tag name :{}", ssItm->symbolTextInfo.text);
		}
	}

	// �������ߵ�λ��
	for (auto& ssgItm : sectionSymbols)
	{
		for (auto& ssItm : ssgItm->sectionSymbolVec)
		{
			const auto& line = data->m_lineData.lines().at(ssItm->sectionLineGroup.sectionBaseLineIdx);
			const Point midPt((line.s.x + line.e.x) / 2, (line.s.y + line.e.y) / 2);
			findClosestVAxis(ssItm->axisVDistance, block->m_axisVLines, midPt.x);
			findClosestHAxis(ssItm->axisHDistance, block->m_axisHLines, midPt.y);
		}
	}

	return true;
}


std::vector<int> SectionSymbolSet::findSectionLineIdx(std::shared_ptr<Block> block)
{
	// ����ͼ����ҵ����е�����
	std::vector<int> sectionLineIdxVec;

	//�ҵ���ֱ������
	for (auto i = block->box.vf; i <= block->box.vt; ++i)
	{
		const auto vLineIdx = data->m_lineData.vLinesIndices().at(i);
		const auto& vLine = data->m_lineData.lines().at(vLineIdx);
		if (vLine.getLayer() == sectionSymbolLayer &&
			vLine.entity->dwgType == DRW::LWPOLYLINE &&
			!vLine.isSide())
		{
			if (std::dynamic_pointer_cast<DRW_LWPolyline>(vLine.entity)->bExistWithFlag())
				sectionLineIdxVec.push_back(vLineIdx);
		}
	}

	//�ҵ�ˮƽ������
	for (auto i = block->box.hf; i <= block->box.ht; ++i)
	{
		const auto hLineIdx = data->m_lineData.hLinesIndices().at(i);
		const auto& hLine = data->m_lineData.lines().at(hLineIdx);
		if (hLine.getLayer() == sectionSymbolLayer &&
			hLine.entity->dwgType == DRW::LWPOLYLINE &&
			!hLine.isSide())
		{
			if (std::dynamic_pointer_cast<DRW_LWPolyline>(hLine.entity)->bExistWithFlag())
				sectionLineIdxVec.push_back(hLineIdx);
		}
	}
	//����б������
	double min[2]{ block->box.left,block->box.bottom };
	double max[2]{ block->box.right,block->box.top };
	auto num = data->m_rtreeData.sLines().Search(min, max,
												 std::bind(&RTreeData::pushBackSBackIndex, &data->m_rtreeData, std::placeholders::_1));
	for (auto sLineIdxItm : data->m_rtreeData.SBackIndex())
	{
		const auto& sLine = data->m_lineData.lines()[sLineIdxItm];
		if (Line::Type::DIMENSIONS != sLine.type &&
			sLine.getLayer() == sectionSymbolLayer &&
			sLine.entity->dwgType == DRW::LWPOLYLINE &&
			!sLine.isSide())
		{
			if (std::dynamic_pointer_cast<DRW_LWPolyline>(sLine.entity)->bExistWithFlag())
				sectionLineIdxVec.push_back(sLineIdxItm);
		}
	}

	data->m_rtreeData.clearSBackIndex();
#if 0
	for (int& it : sectionLineIdxVec)
	{
		pIdxVec->push_back(it);
	}
#endif//��ӡ����ͼ�����
	return sectionLineIdxVec;
}

bool SectionSymbolSet::findSectionSymbolLayer(std::shared_ptr<Block> block)
{
	std::shared_ptr<SectionLineGroup> firstSectionLineGroup;
	std::map<long long, std::shared_ptr<SectionLineGroup>> sectionLineGroupMap;

	for (auto i = block->box.vf; i <= block->box.vt; ++i)
	{
		const auto vLineIdx = data->m_lineData.vLinesIndices().at(i);
		const auto& vLine = data->m_lineData.lines().at(vLineIdx);
		if (!bVerticalSectionLine(block, vLine)) continue;

		//spdlog::get("all")->info("v Line idx :{},its layer {}", vLineIdx, vLine.getLayer());

		auto pseudoX = pseudoDouble(vLine.s.x);

		firstSectionLineGroup = std::make_shared<SectionLineGroup>(vLineIdx);

		const auto& corners = data->m_cornerData.corners().find(vLineIdx);
		//�н���
		if (corners != data->m_cornerData.corners().end())
		{
			auto bCheck = false;
			auto horizontalCounter = 0;
			for (const auto& cornerItm : corners->second)
			{
				const auto crossLineIdx = vLineIdx == cornerItm.l1 ? cornerItm.l2 : cornerItm.l1;
				const auto& crossLine = data->m_lineData.lines().at(crossLineIdx);
				//ƽ������ӵĽ��ߣ�ͼ�����,ƽ���߳��ȶ�����ֱ���ȣ��ߵ�ʵ��Ϊ�����
				if (crossLine.type == Line::Type::DIMENSIONS)
					continue;

				if (!crossLine.horizontal())
					continue;

				if (!bConnectTowLine(crossLine, vLine))
					continue;

				if (crossLine.getLayer() != vLine.getLayer())
					continue;

				if (crossLine.length() - vLine.length() > Precision)
					continue;

				if (!(crossLine.entity->dwgType == DRW::LWPOLYLINE))
					continue;

				//��ֹ���εĶ����
				horizontalCounter++;
				if (horizontalCounter == 2)
					break;

				SectionLineGroup::SectionDirection d{};
				if (abs(crossLine.s.x - vLine.s.x) < Precision)
					d = SectionLineGroup::SectionDirection::Right;
				else if (abs(crossLine.e.x - vLine.s.x) < Precision)
					d = SectionLineGroup::SectionDirection::Left;
				else
				{
					assert("invalid horizontal line");
					break;
				}
				// ��ˮƽ�߽����ⲻ�����ٽ���
				if (bNoOhterCrossLineAtAnotherDirection(d, crossLineIdx, vLineIdx))
				{
					firstSectionLineGroup->sectionDirectionLineIdx = crossLineIdx;
					firstSectionLineGroup->direction = d;
					auto mapIt = sectionLineGroupMap.find(pseudoX);
					if (mapIt != sectionLineGroupMap.end())
					{
						//��������߸������ı��Ƿ���һ����
						auto secondSectionLineGroup = mapIt->second;
						if (bValidSectionGroupPair(firstSectionLineGroup, secondSectionLineGroup))
						{
							spdlog::get("all")->info("found the layer name :{}", vLine.getLayer());

							this->sectionSymbolLayer = vLine.getLayer();
							return true;
							break;
						}
					}
					//pIdxVec->push_back(firstSectionLineGroup->sectionBaseLineIdx);
					sectionLineGroupMap[pseudoX] = firstSectionLineGroup;
				}
			}
		}
		else
		{
			auto mapIt = sectionLineGroupMap.find(pseudoX);
			//�ҵ����ܵ���Ե���
			if (mapIt != sectionLineGroupMap.end())
			{
				//��������߸������ı��Ƿ���һ����
				auto& secondSectionLineGroup = mapIt->second;

				if (bValidSectionGroupPair(firstSectionLineGroup, secondSectionLineGroup))
				{
					spdlog::get("all")->info("found the layer name :{}", vLine.getLayer());
					this->sectionSymbolLayer = vLine.getLayer();
					return true;
					break;
				}
				continue;
			}
			sectionLineGroupMap[pseudoX] = firstSectionLineGroup;
			//pIdxVec->push_back(firstSectionLineGroup->sectionBaseLineIdx);
		}
	}
	return false;
}

bool SectionSymbolSet::bVerticalSectionLine(std::shared_ptr<Block> block,
											const Line& vLine)
{
	//���˲��ڿ�����߼��պϵĶ���
	if (vLine.e.x < block->box.left ||
		vLine.e.y < block->box.bottom ||
		vLine.s.x>block->box.right ||
		vLine.s.y>block->box.top ||
		vLine.isSide())
		return false;

	//���˲��Ƕ���ߵ�����
	if (vLine.entity->dwgType != DRW::LWPOLYLINE)
	{
		//spdlog::get("all")->info("this is not LwPolyLine");
		return false;
	}

	return true;
}

bool SectionSymbolSet::bValidSectionGroupPair(const std::shared_ptr<SectionLineGroup>& first,
											  const std::shared_ptr<SectionLineGroup>& second)
{
	if (first->direction != second->direction ||
		first->isExistSecondDirectionLineIdx() != second->isExistSecondDirectionLineIdx())
		return false;

	//�к�����
	if (first->isExistSecondDirectionLineIdx())
	{
		spdlog::get("all")->info("h section line check:");
		if (first->direction == SectionLineGroup::SectionDirection::Left)
		{
			const auto& l1 = data->m_lineData.lines().at(first->sectionDirectionLineIdx);
			const auto& l2 = data->m_lineData.lines().at(second->sectionDirectionLineIdx);

			if (checkExistsSameText(l1.s, l1.length() * 1.5, l2.s, l2.length() * 1.5))
				return true;
		}
		else if (first->direction == SectionLineGroup::SectionDirection::Right)
		{
			const auto& l1 = data->m_lineData.lines().at(first->sectionDirectionLineIdx);
			const auto& l2 = data->m_lineData.lines().at(second->sectionDirectionLineIdx);

			if (checkExistsSameText(l1.e, l1.length() * 1.5, l2.e, l2.length() * 1.5))
				return true;
		}

	}
	//�޺�����
	else
	{

		spdlog::get("all")->info("no h section line check:");
		const auto& l1 = data->m_lineData.lines().at(first->sectionBaseLineIdx);
		const auto& l2 = data->m_lineData.lines().at(second->sectionBaseLineIdx);
		if (checkExistsSameText(l1.s, l1.length(),
								l2.s, l2.length()))
			return true;
	}
	return false;
}

bool SectionSymbolSet::checkExistsSameText(const Point& p1,
										   const double radius1,
										   const Point& p2,
										   const double radius2)
{
	std::vector<int> vec1 = data->m_kdtTreeData.kdtTexts().radiusSearch(p1, radius1);
	std::vector<int> vec2 = data->m_kdtTreeData.kdtTexts().radiusSearch(p2, radius2);
	for (auto i = 0; i < vec1.size(); ++i)
	{
		const auto textIdx1 = vec1.at(i);
		auto te1 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx1).entity);
		const auto& t1 = te1->text;
		for (auto j = 0; j < vec2.size(); ++j)
		{
			const auto textIdx2 = vec2.at(j);
			if (textIdx1 == textIdx2)
				continue;
			auto te2 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx2).entity);
			const auto& t2 = te2->text;
			spdlog::get("all")->info("compare {} with {}", t1, t2);
			if (t1 == t2)
				return true;
		}
	}
	return false;
}

bool SectionSymbolSet::bNoOhterCrossLineAtAnotherDirection(SectionLineGroup::SectionDirection d,
														   int hIdx,
														   int vIdx)
{
	const auto& hLine = data->m_lineData.lines().at(hIdx);
	const auto& corners = data->m_cornerData.corners().find(hIdx);
	bool bNoConnected = true;

#if 1
	if (corners != data->m_cornerData.corners().end())
	{
		for (const auto& cornerItm : corners->second)
		{
			int crossIdx = cornerItm.l1 == hIdx ? cornerItm.l2 : cornerItm.l1;
			if (crossIdx == vIdx)
				continue;

			/*pIdxVec->push_back(crossIdx);*/

			//��������
			if (d == SectionLineGroup::SectionDirection::Left)
			{
				//�������ڻ��ߵ����
				if (abs(cornerItm.x - hLine.e.x) > 1)
					bNoConnected = false;
			}
			//������
			else if (d == SectionLineGroup::SectionDirection::Right)
			{
				//�������ڻ��ߵ��ұ�
				if (abs(cornerItm.x - hLine.s.x) > 1)
					bNoConnected = false;
			}
		}
	}
#endif
	return bNoConnected;
};

void SectionSymbolSet::setAnalysisDependencise(std::shared_ptr<Data> data)
{
	this->data = data;
}

void SectionSymbolSet::setMFCPaintTestVec(std::vector<int>& IdxVec,
										  std::vector<Line>& lineVec)
{
	this->pIdxVec = &IdxVec;
	this->pLineVec = &lineVec;
}

bool SectionSymbolSet::validateBlock(const std::shared_ptr<Block>& blockItm)
{
	return blockItm->bAxis;
}

int SectionSymbolSet::getSectionSymbolGroup(const Line& l,
											const std::vector<std::shared_ptr<SectionSymbolGroup>>& ssgVec)
{
	if (!data)return -1;//��ֹδ����setAnalysisDependencise��ʼ��data
	for (auto i = 0; i < ssgVec.size(); ++i)
	{
		const auto& ssgItm = ssgVec.at(i);
		int j = 1;
		auto ssFront = ssgItm->sectionSymbolVec.at(0);
		while (j < ssgItm->sectionSymbolVec.size())
		{
			const auto ssNext = ssgItm->sectionSymbolVec.at(j);
			int testIndex=ssFront->sectionLineGroup.sectionBaseLineIdx;
			const auto& ssFrontBaseLine = data->m_lineData.lines().at(ssFront->sectionLineGroup.sectionBaseLineIdx);
			const auto& ssNextBaseLine = data->m_lineData.lines().at(ssNext->sectionLineGroup.sectionBaseLineIdx);
			Line refLine(ssFrontBaseLine.s, ssNextBaseLine.s);

			if (l.vertical() &&
				!refLine.vertical())
			{
				const auto y = knowXToY(refLine, l.s.x);
				const auto minY = std::min(l.s.y, l.e.y);
				const auto maxY = std::max(l.s.y, l.e.y);
				if ((y > minY && y < maxY))
				{
					return i;
				}
			}
			else if (l.horizontal() &&
					 !refLine.horizontal())
			{
				const auto x = knowYToX(refLine, l.s.y);
				const auto minX = std::min(l.s.x, l.e.x);
				const auto maxX = std::max(l.s.x, l.e.x);
				if (x > minX && x < maxX)
				{
					return i;
				}
			}
			else
			{
				Corner corner;
				if (crossLineSS(refLine, l, corner))
					return i;
			}
			j++;
			ssFront = ssNext;
		}
	}
	return -1;
}

