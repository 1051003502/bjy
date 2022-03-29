#pragma once

#include "block.h"
#include "beamLongitudinal.h"

class BeamLongitudinalSet
{
public:
	BeamLongitudinalSet() = default;
	~BeamLongitudinalSet() = default;

	//************************************
	// ����	: SetMember
	// ȫ��	: BeamLongitudinalSet::setMember
	// ����	: public 
	// ����	: bool
	// ����	: _spBlockVec
	// ˵��	: ���ý����������Ŀ�����
	//************************************
	bool setMember(std::vector<std::shared_ptr<Block>>& _spBlockVec);

	//************************************
	// ����	: BeamLongitudianls
	// ȫ��	: BeamLongitudinalSet::beamLongitudianls
	// ����	: public 
	// ����	: void
	// ����	: data
	// ����	: _spBeamLongitudinals
	// ˵��	: ����ÿ�����������������Ϣ
	//************************************
	bool beamLongitudianls(std::shared_ptr<Data> data,
						   std::vector<std::shared_ptr<BeamLongitudinal>>& _spBeamLongitudinals,
						   std::vector<int>& temp_index,
						   std::vector<Line>& temp_lines);


	//************************************
	// ����	: BeamLongitudinal
	// ȫ��	: BeamLongitudinalSet::beamLongitudinal
	// ����	: public 
	// ����	: std::shared_ptr<BeamLongitudinal>
	// ����	: block
	// ����	: data
	// ����	: temp_index
	// ����	: temp_line
	// ˵��	: ���ɿ������Ϣ
	//************************************
	std::shared_ptr<BeamLongitudinal> beamLongitudinal(std::shared_ptr<Block> block,
													   std::shared_ptr<Data> data,
													   std::vector<int>& temp_index,
													   std::vector<Line>& temp_line);

	//************************************
	// ����	: ExcludeBlock
	// ȫ��	: BeamLongitudinalSet::excludeBlock
	// ����	: public 
	// ����	: bool
	// ����	: _spBlockIdxVec
	// ����	: data
	// ˵��	: ��ȡ��Ҫ�Ŀ�
	//************************************
	bool getBlocks(std::vector<std::shared_ptr<Block>>& _blockVec);


	//************************************
	// ����	: GetBlockDimensions
	// ȫ��	: BeamLongitudinalSet::getBlockDimensions
	// ����	: public 
	// ����	: bool
	// ����	: _dimIdxVec
	// ����	: box
	// ����	: data
	// ˵��	: ��ȡ��Ӧ���еı�ע
	//************************************
	bool getBlockDimensions(std::vector<unsigned int >& _dimIdxVec,
							const Box& box,
							std::shared_ptr<Data> data);

	//************************************
	// ����	: FindLongitudianlLayerName
	// ȫ��	: BeamLongitudinalSet::findLongitudianlLayerName
	// ����	: public 
	// ����	: bool
	// ����	: _strLayerName
	// ����	: blockIdxVec
	// ����	: data
	// ˵��	: ������������ͼ��
	//************************************
	bool findLongitudianlSpecialInfo(std::string& _strLayerName,
									 std::shared_ptr<Block> block,
									 Data& data);


public:
	std::vector<std::shared_ptr<Block>> _spBlockVec;//���е�����
};

