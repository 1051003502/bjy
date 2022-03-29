#ifndef __PARSER_UTILS_SECTIONSET_H__
#define __PARSER_UTILS_SECTIONSET_H__
#include "ColumnSection.h"
class ColumnSectionSet
{
public:
	bool setMember(const std::vector<std::shared_ptr<Block>> &blocks, 
		std::vector<std::shared_ptr<Dimension>> dimensions);
	//!�ж�block�Ƿ�û������
	//bool bColumnSection(Data&data, const std::shared_ptr<Block>&_block);
	
	std::shared_ptr<ColumnSection> columnSections(Data & data, const std::shared_ptr<Block>&_block);
	//�ж��Ƿ�Ϊ������
	bool SetSectionsMap(std::vector<bool> &map, Data& data);
	//�ж��Ƿ�Ϊ�ݽ����
	//bool BoolBreakpoint(const Line &line, Data& data);
	bool FindBP(const Box &box, std::vector<Point> &mid_point, std::vector<ColumnSection::BreakPoint> &temp_bp, 
		Data& data);
	//���ҹ���ϵ�
	bool findBreakPoint(const std::vector<ColumnSection::BreakPoint> &bk, const Point &p);
	//����������ͼ�еı�ע��Ϣ
	bool FindSectionDIm(Data &data, const Box &box, std::vector<Dimension> &temp_dim);
public:
	std::vector<std::shared_ptr<Block>> m_blocks;
/**************************************���㷨***************************************************/
public:
	std::shared_ptr<ColumnSection>columnSection(Data& data, const std::shared_ptr<Block>& _block);
private:
	//!�ж�block�Ƿ�Ϊ������
	bool bColumnSection(Data& data, const std::shared_ptr<Block>& _block);

public:
	std::vector<int>testIndexVec;
	std::vector<Point>testPoint;
	Box testBox;
	Json::Value testSection;
	

};
#endif
