#ifndef __PARSER_UTILS_BEAMSET_H__
#define __PARSER_UTILS_BEAMSET_H__
#include "data.h"
#include "block.h"
#include "beam.h"
#include "pillar.h"
#include "axis.h"
#include <fstream>
#include "publicFunction.h"
#include <regex>



class BeamTextPoint
{
public:
	BeamTextPoint(const Point& point, const std::string& name, const bool& blead = false);
public:
	Point pt;//文本点坐标
	bool bLeadLine;//是否有引线
	std::string strText;//文本
};

/**
* Lead 表示和梁边界相较的引线
*/
class Lead
{
public:
	Lead() = default;
	friend bool operator==(const Lead& leadGrop, const Lead& leadGrop1);

public:
	std::vector<int> primevalIndexVec;			//图纸中的所有线的索引
	std::vector<int> nowIndexVec;				//组成引线的所有线的索引
	std::vector<Point> insertPointVec;			// 梁引线的端点
	std::vector<std::pair<int, int>> cornerIndex;//old,new:cross line indices
	bool bSide;			// 是否有闭合线点
	bool bPair;			// 是否和边界配对
	double polyWidth;		// 多段线的宽度
	double refCount;		//定义引出线被调用的次数 有时候一条组引出线会被多次调用 此处要与m_insertPoint.size()，bSide，bPair结合起来使用
	int beamTextPointIndex;//该引线所附着的梁文本点的索引（BeamSet中）

};

class BorderGroup
{
public:

	BorderGroup()
	{
		bGroup = false;
		bLead = false;
		bBeamTextPoint = false;
	}
public:
	std::vector<int> borderVec;		//连通梁的所有边界线索引
	std::vector<std::pair<int, bool>> leadGropsIndexVec;//第一个表示lead的索引，后一个表示此时的状态 有时候同一引出线会修饰多个梁
	std::vector<int> beamTextPointVec; // 和边界线匹配的所有文本点的索引
	bool bLead;// 是否有引线
	bool bGroup;//是否成组
	bool bBeamTextPoint; // 是否有文本点
};

class BeamSet
{
public:

	bool setMember(const std::shared_ptr<Axis>& axises,
				   const std::vector<std::shared_ptr<Pillar>>& pillars);
	bool findBeam(Data& data/*,
				  std::vector<int>& mark_index,
				  std::vector<Point>& corner,
				  std::vector<Line>& temp_line,
				  std::vector<std::vector<int>>& m_temp_index*/);

	//************************************
	// 方法	: Beams
	// 全名	: BeamSet::beams
	// 访问	: public 
	// 返回	: bool 无误返回true，否则为false
	// 参数	: _beamVec 返回解析好的梁
	// 参数	: data 
	// 说明	: 将分组好的边界线及引线解析成梁
	//************************************
	bool beams(std::vector<std::shared_ptr<Beam>>& _beamVec, Data& data);
	
	std::shared_ptr<Beam> parseOneLeadBeamBorderGroup(Data& data,
													  BorderGroup& borderGrpItem,
													  Beam::Direction d);

	std::shared_ptr<Beam> parseNoLeadBeamBorderGroup(Data& data, BorderGroup& borderGrpItem,
													 Beam::Direction d);

public:
	//************************************
	// 方法	: findBeamLayer
	// 全名	: BeamSet::findBeamLayer
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 查找有关梁图层的线
	//************************************
	bool findBeamLayer(const std::vector<std::pair<int, int>>&_columnHBorderIndex, LineData& lineData);
private:
	//************************************
	// 方法	: findBeamLines
	// 全名	: BeamSet::findBeamLines
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 查找有关梁图层的线
	//************************************
	bool findBeamLines(Data& data);

	//************************************
	// 方法	: findLeadLinesAndTextPoint
	// 全名	: BeamSet::findLeadLinesAndTextPoint
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 查找所有的文本点对应的引出线组
	//************************************
	bool findLeadLinesAndTextPoint(Data& data);

	//查找文本对应的位置点
	bool findTextCandidatesPoints(TextPointData& textPointData);

	//查找引出线并返回线索引
	//************************************
	// 方法	: findLeadLineIndex
	// 全名	: BeamSet::findLeadLineIndex
	// 访问	: private 
	// 返回	: int 返回-1为找到，返回-2表示与其它点公用的引出线
	// 参数	: goal_index，目标线索引
	// 参数	: point_index ，候选点的索引
	// 参数	: textPointIndex，梁标志文本对应的点索引
	// 参数	: data
	// 说明	: 在梁标志文本对应的候选线端点内，找出最符合的引出线条件的线索引作为，此文本点对应的引出线
	//************************************
	bool findLeadLineIndex(int& goal_index, const std::vector<int>& point_index, const int& textPointIndex, Data& data);
	//查找引出线的相关引线
	bool findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, std::vector<int>& leadCorner);
	//判断引出线是否有插入点，并初始化整组引线
	bool addLeadGroupInsertPoint(Lead& leadGrop, Data& data, std::vector<int>& dlines);
	//初始化梁线以及之间的交点
	bool initializationBeamLinesCorners();
	//初始化数据
	bool setBeamData();
	/*以下是初步分组重要的函数*/
	//返回平均宽度
	bool  retrunBorderAverageWidth(std::vector<int>& goal_index);
	double borderWidthAverage(const int& conut, std::vector<int>& borderMap, std::vector<int>& goal_index);
	//!随机产生一对配对好的边界线
	bool findPairedBorder(std::pair<int, int>& pairedBorder);
	//!查找goal_index索引对应的边界线的对边，返回值未其对边的索引
	int findPairedBorder(const int& goal_index, const double& relative_value, LineData& beamLines,
						 const std::vector<Lead>& leadGrops);//有目的的搜索

	int findGropPairedBorder(const int& original_index, const double& relative_value, LineData& beamLines,
							 const std::vector<int>& borderGrop);//有目的有范围的搜索
																 //!goal_index需要查找对边的边界线索引，当返回值于goal_index相同时表示查找失败，当返回值不同时width为他们之间的宽度
	int returnGroupPairIndex(const int& goal_index, double& width);

	/*分组梁线*/
	//!为梁线初步进行分组
	bool findBeamBorderGrops();

	//************************************
	// 方法	: slashCornerJudgment
	// 全名	: BeamSet::slashCornerJudgment
	// 访问	: private 
	// 返回	: bool
	// 参数	: sLineIndex 边界线索引
	// 参数	: lineIndex 边界线索引
	// 参数	: point 交点
	// 说明	: 判断斜线的相交情况，用于对边界进行分组
	//************************************
	bool slashCornerJudgment(const int& sLineIndex, const int& lineIndex, const Point& point);

	//************************************
	// 方法	: bLeadLineBeamLayer 表示在梁图层基础上查询
	// 全名	: BeamSet::bLeadLine
	// 访问	: private 
	// 返回	: bool 返回true，表示为是已经存在的引线，false非引出线
	// 参数	: leadGrops，所有的引出线
	// 参数	: index ，所要查找的引线索引
	// 参数	: LeadGropIndex，在是引出线的情况下返回引出线所在组的索引
	// 说明	: 判断index是否为引出线
	//************************************
	bool bLeadLineBeamLayer(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex);

	//************************************
	// 方法	: bLeadLineAllLines 在整张图纸的层面上查找
	// 全名	: BeamSet::bLeadLineAllLines
	// 访问	: private 
	// 返回	: bool
	// 参数	: leadGrops
	// 参数	: index
	// 参数	: LeadGropIndex
	// 说明	: 此函数与前一个函数的作用相同，只不过所参照的索引不同，
	//上衣个函数在梁的图层线索引内判断，如今在所有线索引范围内判断
	//************************************
	bool bLeadLineAllLines(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex);
	//剔除分组中误识别的引出线
	bool removeExcessLeadInBorderGrop(std::vector<BorderGroup>& borderGrops, LineData& beamLines, const double& relative_value,
									  CornerData& borderCorners, const double& average_distance, std::vector<Lead>& leadGrops);
	//!判断引出线是否描述当前边界
	bool bLeadlineBorderPaired(LineData& beamLines, const double& relative_value, CornerData& borderCorners,
							   BorderGroup& BeamGrop, std::vector<Lead>& leadgrops);
	//!查找符合条件的引线组和边界线组，borderGrops表示所有引线组，bLead表示是否有引线，bGrop表示是否成组，beadGrop表示所有的引线组，bside表示是否有断点，bPair表示是否配对，后面两项为返回结果
	bool byConditionExtractBorderGropLeadGrop(const std::vector<BorderGroup>& borderGrops, const bool& bLead, const bool& bGrop,
											  const std::vector<Lead>& leadGrop, const bool& bSide, const bool& bPair, std::vector<int>& conditationBorder, std::vector<int>& conditationLead);
	//!将未与边界线配对的引线再次配对，用于优化没有产生相交关系但却相互成对的情况
	bool borderLeadPair1(Data& beamData, const double& averageWidth, std::vector<Lead>& leadGrops, std::vector<BorderGroup>& borderGrops,
						 std::vector<int>& conditationBorder, std::vector<int>& conditationLead);
	//!判断borderPair是否同为一组，如果时borderGroupsIndex返回边线组索引，并返回true,如果不为同一组则返回false,borderGroupsIndex=-1
	bool bBorderGrop(std::vector<BorderGroup>& borderGrops, const std::pair<int, int>& borderPair, int& borderGroupsIndex);
	//！
	bool byConditionExtractBorderGropTextPoints(const std::vector<BorderGroup>& borderGrops, const bool& bLead1, const bool& bGrop,
												const std::vector<BeamTextPoint>& textPoints, const bool& bLead2, std::vector<int>& conditationBorder, std::vector<int>& conditationPoint);
	//!优化没有引线的文本点与成对边界线之间的配对关系
	bool borderPointPair(Data& beamData, const double& averageWidth, std::vector<BeamTextPoint>& textPoints, std::vector<BorderGroup>& borderGrops,
						 std::vector<int>& conditationBorder, std::vector<int>& conditationTextPint);

	/*返回是否包含跨数信息，以及包含时跨数，以及是否有包含位置信息*/
	bool returnBeamSpanNumber(const std::string& beam_name, bool& b_axis, int& number);
	//在包含有跨数信息的基础上，且有位置信息的情况下找出梁的位置范围
	bool returnLocationRange(const std::string& beam_name, std::string& begin, std::string& end);
	//对柱按照轴进行排序 （由左到右，由上到下）
	bool setPillarsRtree();
	//返回某所在的轴网区域
	bool returnPointInAxisRegion(const Point& point, const Axis& axise,
								 std::vector<Axis::AxisLine>& point_mark);


	//************************************
	// 方法	: findCrossLinesExceptItself
	// 全名	: BeamSet::findCrossLinesExceptItself
	// 访问	: private 
	// 返回	: bool
	// 参数	: _crossLineVec 返回的交线索引
	// 参数	: excludeLineVec 需要排除的索引
	// 参数	: targetIdx 需要查询线的索引
	// 说明	: 查找除自身外与指定线相加的索引
	//************************************
	bool findCrossLinesExceptItself(std::set<int>& _crossLineVec,
									std::vector<int>& fromLineVec,
									const std::vector<int>& excludeLineVec,
									int targetIdx);

	//************************************
	// 方法: returnPiarIdxInLines
	// 全名: BeamSet::returnPiarIdxInLines
	// 访问: private 
	// 返回: bool 找到返回true，否则false
	// 参数: _pairIdx 返回线的索引
	// 参数: fromLineVec 需要查询的线
	// 参数: excludeVec 需要排除的线
	// 参数: targetIdx 需要查询的线
	// 说明: 返回与指定索引的线中点距离最小的线的索引
	//************************************
	bool matchPairIdx(int& _pairIdx,
					  std::vector<int>& fromLineVec,
					  std::vector<int>& excludeVec,
					  int targetIdx);

	//************************************
	// 方法	: findBeamBorders
	// 全名	: BeamSet::findBeamBorders
	// 访问	: private 
	// 返回	: bool 存在边线相接为true，否则false
	// 参数	: _queue 返回的队列
	// 参数	: fromLineVec 搜索的边线集合
	// 参数	: excludeVec 需要排除的边线
	// 参数	: targetIdx 目标边线
	// 说明	: 从fromLineVec中查找与target线索引相接或相交的边线对加入到_queue中
	//************************************
	bool findBeamBorders(std::queue<std::pair<int, int>>& _queue,
						 std::vector<int>& fromLineVec,
						 std::vector<int>& excludeVec,
						 int targetIdx);

	//************************************
	// 方法	: excludeLeadIdx
	// 全名	: BeamSet::excludeLeadIdx
	// 访问	: private 
	// 返回	: bool
	// 参数	: boundIdx 判断的边界索引
	// 参数	: fromLineVec 搜索的引线交线索引
	// 参数	: allLeadIdxVec 搜索的引线
	// 说明	: 判断boundIdx边界线的交线是否存在与给定的allLeadIdxVec引线中的线相交
	//************************************
	bool excludeLeadIdx(int boundIdx,
						std::vector<int>& fromLineVec,
						std::vector<std::pair<int, bool>>& allLeadIdxVec
	);

	//************************************
	// 方法	: searchTextPoint
	// 全名	: BeamSet::searchTextPoint
	// 访问	: private 
	// 返回	: bool
	// 参数	: _textPointVec
	// 参数	: leadIdx
	// 说明	: 搜索指定引线索引附近的所有的文本点
	//************************************
	bool extractTextAnnotationInfo(std::shared_ptr<Beam> _spBeam,
								   Data& data,
								   int leadIdx);

	//************************************
	// 方法	: GetClosestLineIdxFrom
	// 全名	: BeamSet::getClosestLineIdxFrom
	// 访问	: private 
	// 返回	: bool
	// 参数	: _idx 返回最近的线的索引
	// 参数	: fromLineVec 搜索的范围
	// 参数	: pt 文本点
	// 说明	: 从fromLineVec中获取离带L梁文本点的最近的线
	//************************************
	bool getClosestLineIdxFrom(int& _idx,
							   const std::vector<int>& fromLineVec, 
							   const Point& pt);

	//************************************
	// 方法	: SearchText
	// 全名	: BeamSet::searchText
	// 访问	: private 
	// 返回	: bool
	// 参数	: height 返回的文本点的高
	// 参数	: _spBeam
	// 参数	: data
	// 参数	: pt 搜索圆心
	// 参数	: radius 搜索半径
	// 说明	: 在给定的点pt，以radius为半径查找文本点，并解析到spBeam
	//************************************
	bool searchText(std::shared_ptr<Beam> _spBeam,
					Data& data,
					Point& pt,
					double radius);

	//************************************
	// 方法	: CheckDirectionLines
	// 全名	: BeamSet::checkDirectionLines
	// 访问	: private 
	// 返回	: bool
	// 参数	: d
	// 参数	: lineIdxVec
	// 说明	: 判断连通梁的方向
	//************************************
	bool checkDirectionLines(Beam::Direction& d,
							 const std::vector<int>& lineIdxVec);



	//************************************
	// 方法	: iniColumnRtree
	// 全名	: BeamSet::iniColumnRtree
	// 访问	: private 
	// 返回	: bool
	// 说明	: 初始化，柱子的Rtree，用于查找柱子的位置信息
	//************************************
	bool iniColumnRtree();

	//************************************
	// 方法	: returnRtreeColumnIndex
	// 全名	: BeamSet::returnRtreeColumnIndex
	// 访问	: private 
	// 返回	: std::vector<int>
	// 参数	: min 是一个一维数量为二数组，第一个存放left,第二个存放bottom
	// 参数	: max 是一个一维数量为二数组，第一个存放right,第二个存放top
	// 说明	: 返回所给范围在所给区域内的所有柱索引
	//************************************
	std::vector<int> returnRtreeColumnIndex(const double min[],
											const double max[]);

	//************************************
	// 方法	: UpdateMinMax
	// 全名	: BeamSet::updateMinMax
	// 访问	: private 
	// 返回	: bool
	// 参数	: min 最小的x，y
	// 参数	: max 最大的x，y
	// 参数	: line 需要取的用于更新的两个点
	// 说明	: 从line的两个点更新最小的x，y和最大的x，y分别到min，max
	//************************************
	bool updateMinMax(double* min,
					  double* max,
					  const Line line);

	bool updateColChain(std::shared_ptr<Beam> spBeam,
						const Line& line);

	//************************************
	// 方法	: ParseSpan
	// 全名	: BeamSet::parseSpan
	// 访问	: private 
	// 返回	: bool
	// 参数	: _spBeam 要解析的梁
	// 参数	: min 梁的左下点
	// 参数	: max 梁的右上点
	// 说明	: 解析跨，净长等
	//************************************
	bool parseSpan(std::shared_ptr<Beam> _spBeam,
				   Data& data);



	//************************************
	// 方法	: ParseBeamInfo
	// 全名	: BeamSet::parseBeamInfo
	// 访问	: private 
	// 返回	: bool
	// 参数	: _spBeam
	// 参数	: data
	// 参数	: leadIdx
	// 说明	: 解析引线上的梁信息
	//************************************
	bool parseBeamInfo(std::shared_ptr<Beam> _spBeam, Data& data, int leadIdx);

public:
	/*判断点所在区域*/
	bool findSmallHAxis(const Point& point, const Axis& axise, Axis::AxisLine& haxis_mark);
	bool findBigHAxis(const Point& point, const Axis& axise, Axis::AxisLine& haxis_mark);
	bool findSmallVAxis(const Point& point, const Axis& axise, Axis::AxisLine& vaxis_mark);
	bool findBigVAxis(const Point& point, const Axis& axise, Axis::AxisLine& vaxis_mark);

	void setMFCPaintTestVec(std::vector<int>& testIdxVec, std::vector<Line>& testLineVec);

public:
	std::string beamLayer;
	std::vector<Beam> beamVec;		//所有梁
	Data beamData;				// 梁的所有数据
	std::vector<int> primevalIndexVec;//原始border索引
	LineData beamLines;//梁图层的所有线包括引出线
	std::vector<BeamTextPoint>	beamTextPointVec;//梁号所对应的文本点位置以及是否含有引出线
	std::vector<Lead> leadGroupVec;//所有引出线
	CornerData beamCornerData;//梁图层内的所有交点
	double avgWidth;//平均宽度
	std::vector<BorderGroup> borderGroupVec;//所有的成组的边界组
	std::vector<int> condidateBorderIdxVec;
	std::vector<int> condidateLeadIdxVec;
	std::vector<int> condidateBeamTextPointIdxVec;
public:
	//必须已知的变量
	std::shared_ptr<Axis> spAxis;
	std::vector<std::shared_ptr<Pillar>> spPillarVec;
	RTreeData columnRTree;              // aka pillarTRee 对pillars建立Rtree用于快速搜索
	RTreeData borderLineRTree;
	std::vector<std::string> strBorderLayerVec;

	//测试 
	std::vector<int>* pIdxVec;
	std::vector<Line>* pLineVec;
};

#endif