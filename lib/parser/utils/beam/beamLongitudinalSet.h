#pragma once

#include "block.h"
#include "beamLongitudinal.h"

class BeamLongitudinalSet
{
public:
	BeamLongitudinalSet() = default;
	~BeamLongitudinalSet() = default;

	//************************************
	// 方法	: SetMember
	// 全名	: BeamLongitudinalSet::setMember
	// 访问	: public 
	// 返回	: bool
	// 参数	: _spBlockVec
	// 说明	: 设置解析纵梁配筋的块数据
	//************************************
	bool setMember(std::vector<std::shared_ptr<Block>>& _spBlockVec);

	//************************************
	// 方法	: BeamLongitudianls
	// 全名	: BeamLongitudinalSet::beamLongitudianls
	// 访问	: public 
	// 返回	: void
	// 参数	: data
	// 参数	: _spBeamLongitudinals
	// 说明	: 生成每个解析的纵梁配筋信息
	//************************************
	bool beamLongitudianls(std::shared_ptr<Data> data,
						   std::vector<std::shared_ptr<BeamLongitudinal>>& _spBeamLongitudinals,
						   std::vector<int>& temp_index,
						   std::vector<Line>& temp_lines);


	//************************************
	// 方法	: BeamLongitudinal
	// 全名	: BeamLongitudinalSet::beamLongitudinal
	// 访问	: public 
	// 返回	: std::shared_ptr<BeamLongitudinal>
	// 参数	: block
	// 参数	: data
	// 参数	: temp_index
	// 参数	: temp_line
	// 说明	: 生成框架梁信息
	//************************************
	std::shared_ptr<BeamLongitudinal> beamLongitudinal(std::shared_ptr<Block> block,
													   std::shared_ptr<Data> data,
													   std::vector<int>& temp_index,
													   std::vector<Line>& temp_line);

	//************************************
	// 方法	: ExcludeBlock
	// 全名	: BeamLongitudinalSet::excludeBlock
	// 访问	: public 
	// 返回	: bool
	// 参数	: _spBlockIdxVec
	// 参数	: data
	// 说明	: 获取需要的块
	//************************************
	bool getBlocks(std::vector<std::shared_ptr<Block>>& _blockVec);


	//************************************
	// 方法	: GetBlockDimensions
	// 全名	: BeamLongitudinalSet::getBlockDimensions
	// 访问	: public 
	// 返回	: bool
	// 参数	: _dimIdxVec
	// 参数	: box
	// 参数	: data
	// 说明	: 获取相应块中的标注
	//************************************
	bool getBlockDimensions(std::vector<unsigned int >& _dimIdxVec,
							const Box& box,
							std::shared_ptr<Data> data);

	//************************************
	// 方法	: FindLongitudianlLayerName
	// 全名	: BeamLongitudinalSet::findLongitudianlLayerName
	// 访问	: public 
	// 返回	: bool
	// 参数	: _strLayerName
	// 参数	: blockIdxVec
	// 参数	: data
	// 说明	: 查找纵梁配筋的图层
	//************************************
	bool findLongitudianlSpecialInfo(std::string& _strLayerName,
									 std::shared_ptr<Block> block,
									 Data& data);


public:
	std::vector<std::shared_ptr<Block>> _spBlockVec;//所有的区块
};

