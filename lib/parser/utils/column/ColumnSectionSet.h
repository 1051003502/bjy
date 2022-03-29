#ifndef __PARSER_UTILS_SECTIONSET_H__
#define __PARSER_UTILS_SECTIONSET_H__
#include "ColumnSection.h"
class ColumnSectionSet
{
public:
	bool setMember(const std::vector<std::shared_ptr<Block>> &blocks, 
		std::vector<std::shared_ptr<Dimension>> dimensions);
	//!判断block是否没柱截面
	//bool bColumnSection(Data&data, const std::shared_ptr<Block>&_block);
	
	std::shared_ptr<ColumnSection> columnSections(Data & data, const std::shared_ptr<Block>&_block);
	//判断是否为柱断面
	bool SetSectionsMap(std::vector<bool> &map, Data& data);
	//判断是否为纵筋断面
	//bool BoolBreakpoint(const Line &line, Data& data);
	bool FindBP(const Box &box, std::vector<Point> &mid_point, std::vector<ColumnSection::BreakPoint> &temp_bp, 
		Data& data);
	//查找箍筋断点
	bool findBreakPoint(const std::vector<ColumnSection::BreakPoint> &bk, const Point &p);
	//查找柱断面图中的标注信息
	bool FindSectionDIm(Data &data, const Box &box, std::vector<Dimension> &temp_dim);
public:
	std::vector<std::shared_ptr<Block>> m_blocks;
/**************************************新算法***************************************************/
public:
	std::shared_ptr<ColumnSection>columnSection(Data& data, const std::shared_ptr<Block>& _block);
private:
	//!判断block是否为柱截面
	bool bColumnSection(Data& data, const std::shared_ptr<Block>& _block);

public:
	std::vector<int>testIndexVec;
	std::vector<Point>testPoint;
	Box testBox;
	Json::Value testSection;
	

};
#endif
