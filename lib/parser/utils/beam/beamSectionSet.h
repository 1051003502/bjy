#ifndef __PARSER_UTILS_SET1_BEAMSECTIONSET_H__
#define __PARSER_UTILS_SET1_BEAMSECTIONSET_H__
#include<iostream>
#include"block.h"
#include"beamSection.h"

#include<vector>

class BeamSectionSet
{
public:
	
	BeamSectionSet() = default;

	//************************************
	// 方法	: iniBlockNameVec
	// 全名	: BeamSectionSet::iniBlockNameVec
	// 访问	: public 
	// 返回	: bool
	// 参数	: _blockNmaeVec
	//TODO:在多图纸结合的时候会用到，目前只是一个接口方便后面调用
	// 说明	: 初始化梁断面，图块的名字从而查找图内的所有断面并做分析
	//************************************
	bool findBeamSectionBlock(const std::vector<std::string>&_blockNmaeVec);

	//************************************
	// 方法	: setMember
	// 全名	: BeamSectionSet::setMember
	// 访问	: public 
	// 返回	: bool
	// 参数	: _spBlocksVec
	// 说明	: 初始话单张图纸内的所有块信息
	//************************************
	bool setMember(const std::vector<std::shared_ptr<Block>>& _spBlocksVec,
		const std::vector<std::shared_ptr<Dimension>> &_spDimensions);

	//************************************
	// 方法	: returnBeamSection
	// 全名	: BeamSectionSet::returnBeamSection
	// 访问	: public 
	// 返回	: std::vector<BeamSection>
	// 说明	: 此函数用于返回解析后的，梁断面信息，给以后统信息使用的接口
	//************************************
	std::vector<BeamSection> beamSections(Data &data, std::vector<std::shared_ptr<BeamSection>>&_beamSections);

private:

	//************************************
	// 方法	: iniGoalBlockVec
	// 全名	: BeamSectionSet::iniGoalBlockVec
	// 访问	: private 
	// 返回	: bool
	// 参数	: spBlocksVec
	// 说明	: 临时测试用，用于寻找梁截面的图块，后续会删掉
	//************************************
	bool iniGoalBlockVec(std::vector<bool>&mapVec, Data&data);
public:
	//************************************
	// 方法	: bBeamSectionBlock
	// 全名	: BeamSectionSet::bBeamSectionBlock
	// 访问	: private 
	// 返回	: bool
	// 参数	: _block
	// 说明	: 用于判断此块是否为梁截面
	bool bBeamSectionBlock(const std::shared_ptr<Block>& _block,Data&data);



public:
	std::vector<std::shared_ptr<BeamSection>>spBeamSectionVec;//!解析最后的梁断面信息
	std::vector<std::string>blockNameVec;//!目标块名字，即梁断面的名称 此处可能不需要后续优化
	std::vector<std::shared_ptr<Dimension>> spDimensions;//!所有标注信息
private:
	std::vector<std::shared_ptr<Block>> spBlocksVec;//!单张图纸内的块信息
	std::vector<Block>goalBlockVec;//!确定当前图纸被筛选出来的块信息






};
#endif // !1