#pragma once
#include "sectionSymbol.h"
#include "axis.h"
#include "block.h"


class SectionSymbolSet
{
public:
	//************************************
	// 方法	: SectionSymbols
	// 全名	: SectionSymbolSet::sectionSymbols
	// 访问	: public 
	// 返回	: bool
	// 参数	: sectionSymbols
	// 说明	: 返回解析的剖线
	//************************************
	bool parse(std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols,
						std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// 方法	: ParseBlock
	// 全名	: SectionSymbolSet::parseBlock
	// 访问	: public 
	// 返回	: void
	// 参数	: block
	// 说明	: 解析指定的块
	//************************************
	bool parseBlock(std::shared_ptr<Block> block, std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols);

	//************************************
	// 方法	: FindSectionLineIdx
	// 全名	: SectionSymbolSet::findSectionLineIdx
	// 访问	: public 
	// 返回	: std::vector<int>
	// 参数	: block
	// 说明	: 找到块中的所有剖切线
	//************************************
	std::vector<int> findSectionLineIdx(std::shared_ptr<Block> block);

	//************************************
	// 方法	: FindSectionSymbolLayer
	// 全名	: SectionSymbolSet::findSectionSymbolLayer
	// 访问	: public 
	// 返回	: bool
	// 参数	: block
	// 说明	: 查找剖切线的图层
	//************************************
	bool findSectionSymbolLayer(std::shared_ptr<Block> block);

	//************************************
	// 方法	: ValidateLine
	// 全名	: SectionSymbolSet::validateLine
	// 访问	: public 
	// 返回	: bool
	// 参数	: block
	// 参数	: vLine
	// 说明	: 是否符合剖切竖线的属性
	//************************************
	bool bVerticalSectionLine(std::shared_ptr<Block> block, const Line& vLine);


	//************************************
	// 方法	: ValidSectionGroupPair
	// 全名	: SectionSymbolSet::bValidSectionGroupPair
	// 访问	: public 
	// 返回	: bool
	// 参数	: first
	// 参数	: second
	// 说明	: 两个剖切是否为配对
	//************************************
	bool bValidSectionGroupPair(const std::shared_ptr<SectionLineGroup>& first, const std::shared_ptr<SectionLineGroup>& second);

	//************************************
	// 方法	: CheckExistsSameText
	// 全名	: SectionSymbolSet::checkExistsSameText
	// 访问	: public 
	// 返回	: bool
	// 参数	: p1
	// 参数	: radius1
	// 参数	: p2
	// 参数	: radius2
	// 说明	: 再给定的两个点以给定半径查找文本是否相等
	//************************************
	bool checkExistsSameText(const Point& p1, const double radius1, const Point& p2, const double radius2);

	//************************************
	// 方法	: NoOhterCrossLineAtAnotherDirection
	// 全名	: SectionSymbolSet::bNoOhterCrossLineAtAnotherDirection
	// 访问	: public 
	// 返回	: bool
	// 参数	: d
	// 参数	: hIdx
	// 参数	: vIdx
	// 说明	: 在交线的另一端没有交线
	//************************************
	bool bNoOhterCrossLineAtAnotherDirection(SectionLineGroup::SectionDirection d, int hIdx, int vIdx);

	//************************************
	// 方法	: SetAnalysisDependencise
	// 全名	: SectionSymbolSet::setAnalysisDependencise
	// 访问	: public 
	// 返回	: void
	// 参数	: data
	// 参数	: blocks
	// 说明	: 设置解析剖线的依赖
	//************************************
	void setAnalysisDependencise(std::shared_ptr<Data> data);

	//************************************
	// 方法	: SetMFCPaintTestVec
	// 全名	: SectionSymbolSet::setMFCPaintTestVec
	// 访问	: public 
	// 返回	: void
	// 参数	: IdxVec
	// 参数	: lineVec
	// 说明	: 设置MFC测试成员
	//************************************
	void setMFCPaintTestVec(std::vector<int>& IdxVec, std::vector<Line>& lineVec);

	bool validateBlock(const std::shared_ptr<Block>& blockItm);


	int getSectionSymbolGroup(const Line& l,const std::vector<std::shared_ptr<SectionSymbolGroup>>& ssgVec);

	std::shared_ptr<Data>  data;				//数据
	std::vector<std::shared_ptr<Block>> blocks;//所有的块

	std::string sectionSymbolLayer;				//剖切线的图层

	// 测试
	std::vector<int>* pIdxVec{};
	std::vector<Line>* pLineVec{};
};
