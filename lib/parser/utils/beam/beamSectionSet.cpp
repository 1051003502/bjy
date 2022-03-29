#include "beamSectionSet.h"

//bool BeamSectionSet::setMember(const std::vector<std::shared_ptr<Block>>& spVlocksVec)
//{
//	
//	return true;
//}

bool BeamSectionSet::findBeamSectionBlock(const std::vector<std::string>& _blockNmaeVec)
{
	for (auto it : spBlocksVec)
	{
		auto ite = std::find(_blockNmaeVec.begin(), _blockNmaeVec.end(), it->name);
		if (ite != _blockNmaeVec.end())
		{
			goalBlockVec.push_back(*it);
		}
	}
	return true;
}

bool BeamSectionSet::setMember(const std::vector<std::shared_ptr<Block>>& _spBlocksVec,
	const std::vector<std::shared_ptr<Dimension>> &_spDimensions)
{
	this->spBlocksVec = _spBlocksVec;
	this->spDimensions = _spDimensions;

	return true;
}

std::vector<BeamSection> BeamSectionSet::beamSections(Data &data,
	std::vector<std::shared_ptr<BeamSection>>&_beamSections)
{
	//std::vector<bool> map(spBlocksVec.size(), 0);
	//iniGoalBlockVec(map, data);

	//TODO:后面需要完善的
	/*for (auto block : goalBlockVec)
	{
	}*/

	//for (int i = 0; i < spBlocksVec.size(); ++i)
	//{

	//	if (map[i])
	//	{
	//		auto temp_block = BeamSection::retrunBeamSection(*spBlocksVec[i], data);
	//		std::shared_ptr<BeamSection>pBeamSection(new BeamSection(temp_block));
	//		//*pBeamSection = temp_block;
	//		_beamSections.push_back(pBeamSection);
	//	}
	//}
	
	//!最终的执行程序
	for (int i = 0; i < spBlocksVec.size(); ++i)
	{
		if (bBeamSectionBlock(spBlocksVec[i], data))
		{
			auto temp_block = BeamSection::retrunBeamSection(*spBlocksVec[i], data);
			std::shared_ptr<BeamSection>pBeamSection(new BeamSection(temp_block));
			
			_beamSections.push_back(pBeamSection);
		}
	}

	std::fstream of(u8"beamSectionInfo.txt", std::ios::out);
	for (auto it : _beamSections)
	{
		of << "*******************" << std::endl;
		of << "name:" << it->name << std::endl;
		of << "seale" << it->seale << std::endl;
		of << "weight:" << it->maxWidth << "  height:" << it->maxHeight << std::endl;
		of << std::endl << u8"纵筋: ";
		for (auto it1 : it->beamBPinfoVec)
		{
			for (auto it2 : it1.bpInfoVec)
			{
				of << it2 << "  ";
			}
		}
		of << std::endl << u8"箍筋：";
		for (auto it1 : it->beamStirInfo)
		{
			of << it1.stirInfo << "  ";
		}
		of << std::endl;
	}
	of.close();


	return std::vector<BeamSection>();
}

bool BeamSectionSet::iniGoalBlockVec(std::vector<bool>&mapVec, Data &data)
{
	for (int i = 0; i<this->spBlocksVec.size(); ++i)
	{
		bool bBeamSectionMark = false;
		if (spBlocksVec[i]->type == Block::AXIS || spBlocksVec[i]->type == Block::TABLE)
			continue;
		if (spBlocksVec[i]->name.find("-") == std::string::npos)
		{
			continue;
			//goalBlockVec.push_back(*block);
		}
		int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, spBlocksVec[i]->box.bottom);
		int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, spBlocksVec[i]->box.top);
		int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, spBlocksVec[i]->box.left, Precision, false);
		int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, spBlocksVec[i]->box.right, Precision, false);

		//箍筋断面横线
		for (ht; ht >= hf; --ht)
		{
			int idx = data.m_lineData.hLinesIndices()[ht];
			auto &line = data.m_lineData.lines()[idx];
			if (!spBlocksVec[i]->box.cover(line.s) || !spBlocksVec[i]->box.cover(line.e))
			{
				continue;
			}

			if (BoolBreakpoint(line, data))
			{
				mapVec[i] = true;
				bBeamSectionMark = true;
				break;
			}
		}
		//当已经判断出是断面时直接跳过
		if (bBeamSectionMark)
			continue;
		//箍筋断面纵线
		for (vt; vt >= vf && !mapVec[i]; --vt)
		{
			int idx = data.m_lineData.vLinesIndices()[vt];
			auto &line = data.m_lineData.lines()[idx];
			if (!spBlocksVec[i]->box.cover(line.s) || !spBlocksVec[i]->box.cover(line.e))
			{
				continue;
			}
			if (BoolBreakpoint(line, data))
			{
				mapVec[i] = true;
				bBeamSectionMark = true;
				break;
			}
		}
		if (bBeamSectionMark)
			continue;
		//后续添加判圈（断面图的钢筋截面会有一部分用圆绘制）
		data.m_circleData;
	}
	return true;
}

bool BeamSectionSet::bBeamSectionBlock(const std::shared_ptr<Block>& _block, Data& data)
{
	bool bBeamSectionMark = false;
	if (_block->type == Block::AXIS || _block->type == Block::TABLE)
		return false;
	if (_block->name.find("-") == std::string::npos)
	{
		return false;
		//goalBlockVec.push_back(*block);
	}
	int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, _block->box.bottom);
	int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, _block->box.top);
	int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, _block->box.left, Precision, false);
	int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, _block->box.right, Precision, false);

	//箍筋断面横线
	for (ht; ht >= hf; --ht)
	{
		int idx = data.m_lineData.hLinesIndices()[ht];
		auto& line = data.m_lineData.lines()[idx];
		if (!_block->box.cover(line.s) || !_block->box.cover(line.e))
		{
			continue;
		}

		if (BoolBreakpoint(line, data))
		{

			return true;
		}
	}
	
	//箍筋断面纵线
	for (vt; vt >= vf; --vt)
	{
		int idx = data.m_lineData.vLinesIndices()[vt];
		auto& line = data.m_lineData.lines()[idx];
		if (!_block->box.cover(line.s) || !_block->box.cover(line.e))
		{
			continue;
		}
		if (BoolBreakpoint(line, data))
		{
			return true;
		}
	}
	
	//后续添加判圈（断面图的钢筋截面会有一部分用圆绘制）
	auto circle = data.m_circleData;
	return false;
}

