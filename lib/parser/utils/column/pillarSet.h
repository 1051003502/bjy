#ifndef __PARSER_UTILS_PILLARSET_H__
#define __PARSER_UTILS_PILLARSET_H__
#include <iostream>
#include "pillar.h"
#include "publicFunction.h"
class ColumnsTextPoint
{
public:
	ColumnsTextPoint(const Point&point, const std::string &name, bool bLeadLine = false);
public:
	Point m_point;//文本点
	bool m_bLeadLine;//判断文本点是否有引出线
	std::string m_name;//文本的名字
};
class ColumnsLeadGroup
{

public:

	ColumnsLeadGroup(const std::vector<int>&nowIndex,
		int textPointIndex, bool pair = false);
public:
	bool m_pair;//是否组合
	int m_textPointIndex;//文本点的索引
	//std::vector<int>m_primevalIndex;//原索引
	std::vector<int>m_nowIndex;//当前索引
	std::vector<int>m_cornerIndex;//与柱图层相交的引线

};
class ColumnsBorderGroup
{
public:
	typedef enum _BorderGroupType
	{
		FLEAD,
		FGROP,
		FTEXTPOINT
	}BorderGroupType;
public:
	ColumnsBorderGroup(bool bTextPoint = false, bool bgrop = false, bool blead = false);
	friend bool operator==(const ColumnsBorderGroup& borderGrop1, const ColumnsBorderGroup &borderGrop2)
	{
		if (borderGrop1.m_blead != borderGrop2.m_blead||
			borderGrop1.m_bgrop != borderGrop2.m_bgrop||
			borderGrop1.m_btextPoint != borderGrop2.m_btextPoint)
		{
			return false;
		}
		if (borderGrop1.m_borders != borderGrop2.m_borders ||
			borderGrop1.m_leadGropsIndex != borderGrop2.m_leadGropsIndex ||
			borderGrop1.m_textPoints != borderGrop2.m_textPoints)
		{
			return false;
		}
		return true;
	}
public:
	bool m_blead;
	bool m_bgrop;
	bool m_btextPoint;
	std::vector<int>m_borders;
	std::vector<int>m_leadGropsIndex;//第一个表示leadGroping的索引
	std::vector<int>m_textPoints;

};
class PillarSet
{
public:
	bool setMember(const std::shared_ptr<Axis> &axises);
	bool columns(Data&data, std::vector<std::shared_ptr<Pillar>> &_pillars/*, std::vector<int> &temp_index,
		std::vector<Line>&temp_line, std::vector<Point>& corner*/);
public://新式算法
	bool findColumns(Data&data, std::vector<std::shared_ptr<Pillar>> &_pillars);
	//返回成组的柱子边界线索引，有n对成组边界，每对有n条边界
	std::vector<std::pair<int, int>> returnColumnBorderGroupIndex();
private:
	/*计算柱边界图层*/
	bool findColumnLayer(LineData& lineData,CornerData& cornerData);
	/*查找所有的柱图层线*/
	bool findColumnLines(LineData&lineData);
	/*查找对应的文本点*/
	bool findTextCandidatesPoints(TextPointData& textPointData);
	/*文本点对应的引出线*/
	bool findLeadLines(Data&data);
	bool findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, int &cornerIndex);//查找相关联的引出线
	/*初始化柱图层数据*/
	bool initializationBeamLinesCorners();
	/*初始柱边界的组*/
	bool findBorderGrop(Data&data);
	//优化borderGrop不成组的情况
	bool optFBorderGrop(std::vector<int>&temp_index);//opt 指optimization
	bool optFLTextPoint(std::vector<int>&test_index);//优化没有引出线的文本点

	/*查找某LeadLine属于哪个引出线组*/
	int returnLeadGropIndex(const int &leadLineIndex);
	/*组合方式*/
	//初始化未经处理的borderLine,leadLine,textPoint;
	bool initializationColumnType();
	/*求出柱的平均宽高，用于归类相互不连接的情况*/
	bool returnColumnAverageWidthHeight();
	//优化成组但却无引出线修饰的情况

	/*文本信息分析*/
	//是否包含“轴”标识符
	bool checkAxisExist(const std::string& beamText, int& axisIdx);
	/*柱信息整理*/
	bool columnInformation(std::vector<std::shared_ptr<Pillar>> &_pillars);
	//初始化Rtree
	//bool pushRTreeColumnLines(const Line&line, const int &index);

private:
	
	bool initializationLeadLineMap();
	bool initializationBorderLineMap();


public://老式算法
	

	//初始化柱
	bool columnPush(std::vector<std::shared_ptr<Pillar>> &pillars, const std::string &str, const std::vector<int>& vl,
		const std::vector<int> &hl, const std::vector<int> &sl, const std::vector<int> &ls, LineData &lineData);
	//对没有交点的引出线做延长
	bool extendLeadLine(std::vector<Line> &lines, const std::vector<int> &ls, LineData &lineData, CornerData &cornerData);
	/*****************************/
	/*初始化柱与轴网之间的距离*/
	bool SetHaxis(AxisMark &axis_mark_h, const std::string &str, const double &length);
	bool SetVaxis(AxisMark &axis_mark_v, const std::string &str, const double &length);
	bool FindPillarNearAxis(Pillar &pill); //寻找柱子最近的轴
    //查找较小横轴
	bool findSmallHColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &haxis_mark);
	bool findBigHColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &haxis_mark);
	bool findSmallVColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &vaxis_mark);
	bool findBigVColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &vaxis_mark);


private://老式算法
	std::shared_ptr<Axis> m_axises;
private://新式算法
	
	std::string _columnLayer;//柱边界图层
	LineData m_columnLines;//柱图层的所有线包括引出线
	CornerData m_colunmCorners;//柱图层线的所有交点
	std::vector<ColumnsTextPoint>m_textPoint;//柱文本点
	std::vector<ColumnsLeadGroup>m_leadGrops;//引出线组
	std::vector<ColumnsBorderGroup>m_BorderGrops;//柱的索引
	std::vector<int>m_primevalIndex;//原始border索引
	std::vector<int> m_borderLines;
	std::vector<int>m_LeadLines;
	double m_averageWidth;
	double m_averageHeight;
	
	//用于快速查找的所有字典
	/*leadLines与borderLines的字典用于快速查找*/
	std::map<int, int>m_mapLeadLines;//前一个后一个都表示leadLine的索引号，用于快速判断此线是否为引出线
	std::map<int, int>m_mapBorderLines;//前一个后一个都表示borderLine的索引号，用于快速判断此线是否为borderline
	std::map<bool, std::vector<int>>m_leadGropsType;
	std::map<bool, std::vector<int>>m_textPointType;
	std::map<ColumnsBorderGroup::BorderGroupType, std::vector<int>>m_borderGropsType;
	std::map<int, int>m_mapBorderGropIndex;//前一个标识columnLine的线索引，后一个表示所在组索引
	std::map<int, int>m_mapLeadGropIndex;//前一个表示leadLine的索引，后一个表示所在组的索引
	//创建Rtree
	RTreeData m_rTreeColumnLines;

	
};
#endif