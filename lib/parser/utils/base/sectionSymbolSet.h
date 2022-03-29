#pragma once
#include "sectionSymbol.h"
#include "axis.h"
#include "block.h"


class SectionSymbolSet
{
public:
	//************************************
	// ����	: SectionSymbols
	// ȫ��	: SectionSymbolSet::sectionSymbols
	// ����	: public 
	// ����	: bool
	// ����	: sectionSymbols
	// ˵��	: ���ؽ���������
	//************************************
	bool parse(std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols,
						std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// ����	: ParseBlock
	// ȫ��	: SectionSymbolSet::parseBlock
	// ����	: public 
	// ����	: void
	// ����	: block
	// ˵��	: ����ָ���Ŀ�
	//************************************
	bool parseBlock(std::shared_ptr<Block> block, std::vector<std::shared_ptr<SectionSymbolGroup>>& sectionSymbols);

	//************************************
	// ����	: FindSectionLineIdx
	// ȫ��	: SectionSymbolSet::findSectionLineIdx
	// ����	: public 
	// ����	: std::vector<int>
	// ����	: block
	// ˵��	: �ҵ����е�����������
	//************************************
	std::vector<int> findSectionLineIdx(std::shared_ptr<Block> block);

	//************************************
	// ����	: FindSectionSymbolLayer
	// ȫ��	: SectionSymbolSet::findSectionSymbolLayer
	// ����	: public 
	// ����	: bool
	// ����	: block
	// ˵��	: ���������ߵ�ͼ��
	//************************************
	bool findSectionSymbolLayer(std::shared_ptr<Block> block);

	//************************************
	// ����	: ValidateLine
	// ȫ��	: SectionSymbolSet::validateLine
	// ����	: public 
	// ����	: bool
	// ����	: block
	// ����	: vLine
	// ˵��	: �Ƿ�����������ߵ�����
	//************************************
	bool bVerticalSectionLine(std::shared_ptr<Block> block, const Line& vLine);


	//************************************
	// ����	: ValidSectionGroupPair
	// ȫ��	: SectionSymbolSet::bValidSectionGroupPair
	// ����	: public 
	// ����	: bool
	// ����	: first
	// ����	: second
	// ˵��	: ���������Ƿ�Ϊ���
	//************************************
	bool bValidSectionGroupPair(const std::shared_ptr<SectionLineGroup>& first, const std::shared_ptr<SectionLineGroup>& second);

	//************************************
	// ����	: CheckExistsSameText
	// ȫ��	: SectionSymbolSet::checkExistsSameText
	// ����	: public 
	// ����	: bool
	// ����	: p1
	// ����	: radius1
	// ����	: p2
	// ����	: radius2
	// ˵��	: �ٸ������������Ը����뾶�����ı��Ƿ����
	//************************************
	bool checkExistsSameText(const Point& p1, const double radius1, const Point& p2, const double radius2);

	//************************************
	// ����	: NoOhterCrossLineAtAnotherDirection
	// ȫ��	: SectionSymbolSet::bNoOhterCrossLineAtAnotherDirection
	// ����	: public 
	// ����	: bool
	// ����	: d
	// ����	: hIdx
	// ����	: vIdx
	// ˵��	: �ڽ��ߵ���һ��û�н���
	//************************************
	bool bNoOhterCrossLineAtAnotherDirection(SectionLineGroup::SectionDirection d, int hIdx, int vIdx);

	//************************************
	// ����	: SetAnalysisDependencise
	// ȫ��	: SectionSymbolSet::setAnalysisDependencise
	// ����	: public 
	// ����	: void
	// ����	: data
	// ����	: blocks
	// ˵��	: ���ý������ߵ�����
	//************************************
	void setAnalysisDependencise(std::shared_ptr<Data> data);

	//************************************
	// ����	: SetMFCPaintTestVec
	// ȫ��	: SectionSymbolSet::setMFCPaintTestVec
	// ����	: public 
	// ����	: void
	// ����	: IdxVec
	// ����	: lineVec
	// ˵��	: ����MFC���Գ�Ա
	//************************************
	void setMFCPaintTestVec(std::vector<int>& IdxVec, std::vector<Line>& lineVec);

	bool validateBlock(const std::shared_ptr<Block>& blockItm);


	int getSectionSymbolGroup(const Line& l,const std::vector<std::shared_ptr<SectionSymbolGroup>>& ssgVec);

	std::shared_ptr<Data>  data;				//����
	std::vector<std::shared_ptr<Block>> blocks;//���еĿ�

	std::string sectionSymbolLayer;				//�����ߵ�ͼ��

	// ����
	std::vector<int>* pIdxVec{};
	std::vector<Line>* pLineVec{};
};
