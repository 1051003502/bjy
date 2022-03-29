#include "beamLongitudinalSet.h"

bool BeamLongitudinalSet::setMember(std::vector<std::shared_ptr<Block>>& _spBlockVec)
{
	this->_spBlockVec = _spBlockVec;
	return true;
}

bool BeamLongitudinalSet::beamLongitudianls(std::shared_ptr<Data> data,
											std::vector<std::shared_ptr<BeamLongitudinal>>& _spBeamLongitudinals,
											std::vector<int>& temp_index,
											std::vector<Line>& temp_lines)
{
	//拿到需要的块
	std::vector<std::shared_ptr<Block>> blockVec;
	getBlocks(blockVec);
	if (!blockVec.empty())
	{
		for (const auto& it : blockVec)
		{
			if (!it->name.empty())
			{

				auto tp = this->beamLongitudinal(it, data, temp_index, temp_lines);
				if (tp)
					_spBeamLongitudinals.push_back(tp);
			}
		}
	}
	return true;
}

std::shared_ptr<BeamLongitudinal> BeamLongitudinalSet::beamLongitudinal(std::shared_ptr<Block> block,
																		std::shared_ptr<Data> data,
																		std::vector<int>& temp_index,
																		std::vector<Line>& temp_line)
{
	std::string strLayer;
	//获取图层
	this->findLongitudianlSpecialInfo(strLayer, block, *data);
	std::vector<unsigned int> dimIdxVec;
	this->getBlockDimensions(dimIdxVec, block->box, data);
	if (dimIdxVec.empty())
		return nullptr;

	std::shared_ptr<BeamLongitudinal> spbeamLongitudinal = std::make_shared<BeamLongitudinal>();
	spbeamLongitudinal->parseBlock(block, strLayer, data, dimIdxVec, temp_index, temp_line);
	return spbeamLongitudinal;
}

bool BeamLongitudinalSet::getBlocks(std::vector<std::shared_ptr<Block>>& _blockVec)
{
	for (auto i = 0; i < _spBlockVec.size(); i++)
	{
		auto& blockType = _spBlockVec.at(i)->type;
		if (blockType == Block::TABLE || blockType == Block::AXIS)
			continue;

		_blockVec.push_back(_spBlockVec.at(i));
	}
	if (_blockVec.empty())
		return false;
	return true;
}
 

bool BeamLongitudinalSet::getBlockDimensions(std::vector<unsigned int>& _dimIdxVec,
											 const Box& box,
											 std::shared_ptr<Data> data)
{
	for (auto i = 0; i < data->m_dimensions.size(); i++)
	{
		if (box.cover(data->m_dimensions.at(i)->box))
		{
			_dimIdxVec.push_back(i);
		}
	}
	return true;
}

bool BeamLongitudinalSet::findLongitudianlSpecialInfo(std::string& _strLayerName,
													  std::shared_ptr<Block> block,
													  Data& data)
{
	//横线下标，长度对
	std::vector<std::pair<int, double>> hLineIdxLenPair;
	//图层，次数
	std::map<std::string, int> blockLayerMap;

	//所有平行线
	for (auto j = block->box.ht; j >= block->box.hf; j--)
	{
		auto lineIdx = data.m_lineData.hLinesIndices().at(j);
		auto line = data.m_lineData.lines().at(lineIdx);
		if (line.type != Line::Type::DIMENSIONS)
		{
			auto len = std::abs(line.s.x - line.e.x);
			hLineIdxLenPair.emplace_back(lineIdx, len);
		}
	}
	//从长到短
	std::sort(hLineIdxLenPair.begin(), hLineIdxLenPair.end(), [](std::pair<int, double> p1, std::pair<int, double> p2)
	{ return p1.second > p2.second; });

	auto size = hLineIdxLenPair.size();

	//得到范围内的线idx
	std::vector<int> rangeIdxVec;
	//过滤标注线，后面优化
	for (auto s = 3; s < size / 2; s++)
	{
		rangeIdxVec.push_back(hLineIdxLenPair.at(s).first);
	}

	bool bAck = false;

	double acGapLen = hLineIdxLenPair.at(size / 2).second * 2;
	for (auto idxItm1 : rangeIdxVec)
	{
		auto line1 = data.m_lineData.lines().at(idxItm1);
		for (auto idxItm2 : rangeIdxVec)
		{
			if (idxItm1 == idxItm2)
				continue;

			auto line2 = data.m_lineData.lines().at(idxItm2);
			if (line1.length() > line2.length())
			{
				auto tp = std::abs(line1.s.y - line2.s.y);
				if (line1.s.x <= line2.s.x && line1.e.x >= line2.e.x &&
					line1.getLayer() == line2.getLayer() &&
					!(tp < 0.001))
				{
					if (tp <= acGapLen)
					{
						acGapLen = tp;
						const auto& layer = data.m_lineData.lines().at(idxItm1).getLayer();
						blockLayerMap[layer]++;
					}
				}
			}
		}
	}

	auto maxCount = 0;
	for (auto& it : blockLayerMap)
	{
		if (it.second > maxCount)
		{
			maxCount = it.second;
			_strLayerName = it.first;
		}
	}

	return true;
}

