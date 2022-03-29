#ifndef __PARSER_UTILS_DATA_H__
#define __PARSER_UTILS_DATA_H__
#include <iostream>
#include "line.h"
#include "corner.h"
#include "circle.h"
#include "dimension.h"
#include "RTree.h"
#include "kdt.h"
#include <queue>
#include "axis.h"


typedef RTree<int, double, 2, float> RLine;
using compRef = bool (*)(const Point&, const Point&);



class LineData
{
public:
	using Ref = bool (*)(const std::vector<Line>&, int, int);
	enum cmLINEDATAMODE
	{
		LINEDATAMODE_LINES,  /* lines */
		LINEDATAMODE_VINDEX, /* vertical index */
		LINEDATAMODE_HINDEX, /* horizontal index */
		LINEDATAMODE_SINDEX  /* slash index */
	};

	bool clearLineData();
	bool pushBackLines(const Line& line);
	bool pushBackVLinesIndices(const int& index);
	bool pushBackHLinesIndices(const int& index);
	bool pushBackSLinesIndices(const int& index);
	bool setLines(const std::vector<Line>& line);
	bool setVLinesIndex(const std::vector<int>& vIndex);
	bool setHLinesIndex(const std::vector<int>& hIndex);
	bool setSLinesIndex(const std::vector<int>& sIndex);
	/*set lines start and end*/
	bool setLines(const int& index, const Point& start, const Point& end);

	/* get the size of vars */
	int getLinesSize();
	int getVLinesSize();
	int getHLinesSize();
	int getSLinesSize();

	Line getLine(const int& index);
	int getVLineIndex(const int& index);
	int getHLineIndex(const int& index);
	int getSLineIndex(const int& index);

	/* access to vars */
	std::vector<Line>& lines();
	const std::vector<int>& vLinesIndices();
	const std::vector<int>& hLinesIndices();
	const std::vector<int>& sLinesIndices();

	void sortLinesByBindRef(enum cmLINEDATAMODE mode, LineData::Ref ref);

	/* find the min and the max boundary */
	int findLowerBound(enum cmLINEDATAMODE mode, double val, double precision = Precision, bool h = true);
	int findUpperBound(enum cmLINEDATAMODE mode, double val, double precision = Precision, bool h = true);

	/* lines sort */
	bool linesSort(std::vector<Line>& lines, enum cmLINEDATAMODE mode);
	bool quickSortLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode);
	int divisionLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode);

	/* lines sort index */
	bool linesSortIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode);
	bool quickSortLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode);
	int divisionLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode);

	/* merge lines */
	//一较长线为准，合并同一位置处的线
	bool mergeLines(std::vector<Line>& lines, enum cmLINEDATAMODE mode); //合并线条
	/* merge lines index */
	bool mergeLinesIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode);
	//功能函数
	bool lineTypeVHS(const std::vector<int>& index, std::vector<int>& v_lines,
					 std::vector<int>& h_lines, std::vector<int>& s_lines);





private:
	std::vector<Line> _lines;		 /* lines */
	std::vector<int> _vLinesIndices; /* vertical lines indices */
	std::vector<int> _hLinesIndices; /* horizontal lines indices */
	std::vector<int> _sLinesIndices; /* slash lines indices */
};




//圈，椭圆
class CirclesData
{
public:
	void clearCircles();
	bool pushBackCircles(const Circle&);
	const std::vector<Circle>& circles();
	//根据圆心排序
	bool sortByRef(compRef ref);

private:
	std::vector<Circle> _circles;
};

class CornerData
{
	/*理论上_corners初始化后是不可以被更改的，因为 图纸从读入那一刻就是不变的，我们不可以写入数据
	因此只设置，1.初始化接口2.查找接口，3.末尾接口*/
public:
	bool clearCorners(); //清除函数

	const std::map<int, std::vector<Corner>>& corners() const
	{
		return _corners;
	};

	bool crossLinesS(const std::vector<Line>& lines, int idxs, int idx, double xp = Precision, double yp = Precision);
	bool crossLinesVH(const std::vector<Line>& lines, int idxv, int idxh, double xp = Precision, double yp = Precision);
	bool crossLinesSS(const std::vector<Line>& lines, int idxs1, int idxs2);
	bool setCorners(const std::map<int, std::vector<Corner>>& corner);


	//************************************
	// 方法	: FindCross
	// 全名	: CornerData::findCross
	// 访问	: public 
	// 返回	: bool
	// 参数	: _fromLineIdxVec
	// 参数	: _excludeLineIdxVec
	// 参数	: target
	// 参数	: _pred
	// 说明	: 查找相交线，并执行_pred
	//************************************
	template<typename _Pr>
	bool findCross(std::vector<int>& _fromLineIdxVec,
				   std::vector<int>& _excludeLineIdxVec,
				   int targetIdx,
				   _Pr _pred)
	{
		int idx = -1;
		bool bRet = false;
		const auto& cornerVecIt = _corners.find(targetIdx);
		if (cornerVecIt != _corners.end())
		{
			bool bRet2;
			//遍历交点
			for (const auto& cornerItm : cornerVecIt->second)
			{
				bRet2 = false;
				//过滤自身
				idx = (cornerItm.l2 == targetIdx) ? cornerItm.l1 : cornerItm.l2;
				//过滤需要排除的线
				if (std::find(_excludeLineIdxVec.begin(), _excludeLineIdxVec.end(), idx) == _excludeLineIdxVec.end() &&
					std::find(_fromLineIdxVec.begin(), _fromLineIdxVec.end(), idx) != _fromLineIdxVec.end())
				{
					bRet2 = _pred(idx, cornerItm);
					if (!bRet)
						bRet = bRet2;
				}
			}
		}
		return bRet;
	}


private:
	std::map<int, std::vector<Corner>> _corners;
};
//扩展交点
bool extendCorner(std::vector<Corner>& corners, const Line& line, LineData& lineData);






class EndpointData
{
public:
	bool clearEndpoints(); //清除函数
	/*同理，endpoint表示，线的端点，以及线的索引，在图纸读入那一刻将不会放生改变
	，一般与CornereData联合使用快速定位交点，以及相关的线（endpoint可以以索引查找交点，从而找到与之相关线
	，corner也可以通过索引去找到线相关的端点），
	封装1.初始化 2.通过索引查找端点，及索引*/
	bool pushBackEndpoint(const Endpoint& endpoint); //初始化

	Endpoint getEndpoint(const int& index);			 //返回值 通过索引查找

	const std::vector<Endpoint>& endpoints();

private:
	std::vector<Endpoint> _endpoints;
};

//KDTree搜索树---->此树用于一维数据点的搜索，会有很高的效率
/*KDTree 的三个函数
*/
class KDTreeData
{
public:
	bool kdttextsClear();
	bool kdtendsClear();
	bool buildKDTreeData(const std::vector<Endpoint>& _endpoints, const std::vector<Point>& _textPoints);
	const kdt::KDTree<Point>& kdtTexts() const
	{
		return _kdttexts;
	};
	const kdt::KDTree<Endpoint>& kdtEndpoints() const
	{
		return _kdtends;
	}

private:
	kdt::KDTree<Point> _kdttexts;
	kdt::KDTree<Endpoint> _kdtends;
};

//RTree 空间搜索树 目前我只是将此树应用到了斜线上面，理论上应该横线竖线也需要，这样搜索速度会大幅度提升
class RTreeData
{
public:


	bool clearSLine();
	bool clearVLine();
	bool clearHLine();
	bool clearSBackIndex();
	bool insertSLines(const double min[], const double max[], const int id);
	bool pushBackSBackIndex(int id);

	const RLine& sLines();
	const std::vector<int>& SBackIndex();


private:
	RLine s_lines;
	RLine v_lines;
	RLine h_lines;
	std::vector<int> s_back_index;
};

class TextPointData
{
public:
	bool pushBackPoint(const Point& point);
	bool clearPoints();
	bool sortByRef(compRef ref);
	const std::vector<Point>& textpoints();

	int findLowerBound(double val, double precision = Precision);
	int findUpperBound(double val, double precision = Precision);

private:
	std::vector<Point> _textpoints;
};

//将不同的图层写成map，将相同图层的线索引存在在一起
class LineLayerData
{
public:
	//************************************
	// 方法	: bInLineLayerMap
	// 全名	: LineLayerData::getGoalLayerLineVec
	// 访问	: private 
	// 返回	: std::vector<int>
	// 参数	: goalLayer 需要判断的图层
	// 说明	: 查看是否存在此图层
	//************************************
	std::vector<int> getGoalLayerLineVec(const std::string& goalLayer);
	//************************************
	// 方法	: setLineLayerMapVec
	// 全名	: LineLayerData::setLineLayerMapVec
	// 访问	: private 
	// 返回	: bool
	// 参数	: lineIndex 当前线条的索引
	// 参数 : line  线条实体
	// 说明	: 查看是否存在此图层
	//************************************
	bool setLineLayerMapVec(const int& lineIndex, const Line& line);
	//************************************
	// 方法	: lineLayerMapVec
	// 全名	: LineLayerData::lineLayerMapVec
	// 访问	: private 
	// 返回	: std::map<std::string, std::vector<int>>
	// 说明	: 返回相关图层的线信息
	//************************************
	const std::map<std::string, std::vector<int>>& lineLayerMapVec();
	//end指针
	std::map<std::string, std::vector<int>>::const_iterator lineLayerEndP();
private:

	std::map<std::string, std::vector<int>> _lineLayerMapVec;//将不同图层的线条进行分类存储
};

//将图纸内地轴数据存放成公用的data结构数据
class AxisLineData
{
public:

	bool setAxisHLine(const std::vector<Axis::AxisLine>& axisHLines);
	bool setAxisVLine(const std::vector<Axis::AxisLine>& axisVLines);
	bool setAxisSLine(const std::vector<Axis::AxisLine>& axisSLines);
	const std::vector<Axis::AxisLine>& axisHLines();
	const std::vector<Axis::AxisLine>& axisVLines();
	const std::vector<Axis::AxisLine>& axisSLines();
private:
	std::vector<Axis::AxisLine> _axisHLines;
	std::vector<Axis::AxisLine> _axisVLines;
	std::vector<Axis::AxisLine> _axisSLines;
};
class Data;

class DimData
{
public:

	void setDim(const std::shared_ptr<Dimension> dim);

	//************************************
	// 方法	: FindHVDimension
	// 全名	: DimData::findHVDimension
	// 访问	: public 
	// 返回	: bool
	// 参数	: _dimPairVec
	// 参数	: tick
	// 参数	: bIncrement
	// 参数	: bHorizontal
	// 参数	: data
	// 说明	: 查找水平和竖直标注
	//************************************
	bool findHVDimension(std::vector<DimensionPair>& _dimPairVec,
						 const Point& tick,
						 bool bIncrement,
						 bool bHorizontal);

	bool findHDimension(std::vector<DimensionPair>& _dimPairVec,
						const double xTick,
						const double offset = Precision);

	bool findVDimension(std::vector<DimensionPair>& _dimPairVec,
						const double yTick,
						const double offset = Precision);


	//************************************
	// 方法	: FindAngularDimension
	// 全名	: DimData::findAngularDimension
	// 访问	: public 
	// 返回	: std::vector<int> 标注的索引向量
	// 参数	: lineIdxVec	
	// 参数	: data
	// 说明	: 查找角度标注
	//************************************
	std::vector<int> findAngularDimension(const std::vector<unsigned>& lineIdxVec,
										  const std::shared_ptr<Data>& data)const;

	//************************************
	// 方法	: FindRadialAndDiameterDimension
	// 全名	: DimData::findRadialAndDiameterDimension
	// 访问	: public 
	// 返回	: unsigned int
	// 参数	: pt		圆心
	// 说明	: 查找半径标注及直径标注
	//************************************
	unsigned int findRadialAndDiameterDimension(const Point& pt) const;

	//************************************
	// 方法	: GetSlashDimRTree
	// 全名	: DimData::getSlashDimRTree
	// 访问	: public 
	// 返回	: const RTreeData&
	// 说明	: 返回 用于查找倾斜标注的RTree
	//************************************
	const RTreeData& getSlashDimRTree() const
	{
		return sDimRTree;
	}

	//************************************
	// 方法	: Dimensions
	// 全名	: DimData::dimensions
	// 访问	: public 
	// 返回	: const std::vector<std::shared_ptr<Dimension>>
	// 说明	: 标注的集合
	//************************************
	const std::vector<std::shared_ptr<Dimension>> dimensions()const
	{
		return _dimensions;
	}

private:
	void setDimRadius(const std::shared_ptr<Dimension> dim);
	void setDimAngular(const std::shared_ptr<Dimension> dim);
	void setDimLinearAligned(const std::shared_ptr<Dimension> dim);

	std::vector<std::pair<Point, unsigned>> circlePtVec;
	std::vector<std::pair<Line, unsigned>> angularLineVec;
	std::map<long long, std::vector<DimensionPair>> hDimDesMap;	//水平标注x映射
	std::map<long long, std::vector<DimensionPair>> vDimDesMap;	//竖直标注y映射
	std::map<long long, std::vector<unsigned>> lenDimDesMap;	//标注长度映射
	RTreeData sDimRTree;										//斜标注Rtree
	std::vector<std::shared_ptr<Dimension>> _dimensions;		//所有的尺寸
};

//数据集
class Data
{

public:
	LineData m_lineData;
	CornerData m_cornerData;
	TextPointData m_textPointData;
	RTreeData m_rtreeData;
	KDTreeData m_kdtTreeData;
	EndpointData m_endpointData;
	CirclesData m_circleData;
	LineLayerData m_lineLayerData;
	AxisLineData m_axisLineData;
	DimData m_dimData;
	std::vector<std::shared_ptr<Dimension>> m_dimensions;
};
#endif