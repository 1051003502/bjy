#ifndef __PARSER_UTILS_PUBLICFUNCTION_H__
#define __PARSER_UTILS_PUBLICFUNCTION_H__

#include "common.h"
#include "data.h"
#include "intern/drw_textcodec.h"
#include "drw_color.h"


// TODO 实现打印基础信息的功能
#pragma region Log Print

void logLine(const Line& line);


#pragma endregion Log Print

//************************************
// 方法	: GetRandom16Chars
// 全名	: getRandom16Chars
// 访问	: public 
// 返回	: std::string 返回的字符串
// 说明	: 随机字符的16个字符
//************************************
std::string getRandom16Chars();

//************************************
// 方法	: StrimDouble
// 全名	: strimDouble
// 访问	: public 
// 返回	: double
// 参数	: d 要四舍五入的浮点数
// 参数	: bit 保留的位数
// 说明	: 四舍五入保留小数点的位数
//************************************
double strimDouble(double d, unsigned char bit = 2);


/*区域相关函数*/
bool b_closedzone(const std::vector<int>& borderline, ClosedZone& temp_borderline, Data& data);
bool b_InClosedZone(int& candidate, const std::vector<int>& line__index, const ClosedZone& temp_borderline, Data& data);

/*线的相关函数*/
//对线的类型进行分类，横线，竖线，斜线三类
bool lineTypeVHS(Data& data,
				 const std::vector<int>& index,
				 std::vector<int>& v_lines,
				 std::vector<int>& h_lines,
				 std::vector<int>& s_lines);

bool lineTypeVHSIndex(const std::vector<Line>& lines,
					  std::vector<int>& v_lines,
					  std::vector<int>& h_lines,
					  std::vector<int>& s_lines);

bool lineTypeVHSEntity(const std::vector<Line>& lines,
					   std::vector<Line>& v_lines,
					   std::vector<Line>& h_lines,
					   std::vector<Line>& s_lines);

bool lineTypeVHS(const std::vector<Line>& lines,
				 const std::vector<int>& index,
				 std::vector<int>& v_lines,
				 std::vector<int>& h_lines,
				 std::vector<int>& s_lines);

//初始化一堆线的交点
bool bulidLinesCorner(const std::vector<Line>& lines,
					  const std::vector<int>& vIndex,
					  const std::vector<int>& hIndex,
					  const std::vector<int>& sIndex,
					  std::map<int, std::vector<Corner>>& corners);

//获取距离某点相对最近的线
bool nearestLine(const std::vector<Line>& lines,
				 const std::vector<int>& linesIndex,
				 const Point& point,
				 std::pair<int, int>goal_index);


//删除特定字符
bool del_char(std::string& str);

bool pushRTreeLines(const Line& line, const int& index, RTreeData& rTree);

//************************************
// 方法	: returnLineType
// 全名	: returnLineType
// 访问	: public 
// 返回	: int
// 参数	: line
// 说明	: 返回线的类型，返回1，为横线，返回2为竖线，返回3为斜线
//************************************
int returnLineType(const Line& line);

//************************************
// 方法	: bLineConnect
// 全名	: bLineConnect
// 访问	: public 
// 返回	: bool
// 参数	: line1
// 参数	: line2
// 说明	: 判断线条是否首尾相接
//************************************
bool bLineConnect(const Line& line1, const Line& line2);

//************************************
// 方法	: BoolBreakpoint
// 全名	: BoolBreakpoint
// 访问	: public 
// 返回	: bool
// 参数	: line
// 参数	: data
// 说明	: 判断多段线是否为纵筋断面
//************************************
bool BoolBreakpoint(const Line& line, Data& data);

//************************************
// 方法	: SetBoxVHIndex
// 全名	: SetBoxVHIndex
// 访问	: public 
// 返回	: bool
// 参数	: box
// 参数	: lineData
// 说明	: 初始化box的边界范围索引
//************************************
bool SetBoxVHIndex(Box& box, LineData& lineData);

//************************************
// 方法	: returnRtreeIndex
// 全名	: returnRtreeIndex
// 访问	: public 
// 返回	: std::vector<int>
// 参数	: min
// 参数	: max
// 参数	: goalRTree
// 说明	: 返回Rtree的搜索结果
//************************************
std::vector<int> returnRtreeIndex(const double min[], const double max[], RTreeData& goalRTree);

/*为线条设置排序函数(按照线实体排序)*/
//这里的排序函数为快速排序
//************************************
// 方法	: LinesSort
// 全名	: LinesSort
// 访问	: public 
// 返回	: bool
// 参数	: lines 要排序的所有线对象
// 参数	: ch 选择对横线排序还是竖线
// 说明	: 线条快速排序的入口
//************************************
bool LinesSort(std::vector<Line>& lines, const char& ch);//对已知横线，竖线进行排序
 //************************************
 // 方法	: QuickSortLine
 // 全名	: QuickSortLine
 // 访问	: public 
 // 返回	: bool
 // 参数	: lines
 // 参数	: left 起始
 // 参数	: right 终止
 // 参数	: ch 排序方式
 // 说明	: 快速排序递归的过程
 //************************************
bool QuickSortLine(std::vector<Line>& lines, int left, int right, const char& ch);
//************************************
// 方法	: DivisionLine
// 全名	: DivisionLine
// 访问	: public 
// 返回	: int
// 参数	: lines
// 参数	: left 起始
// 参数	: right 终止
// 参数	: ch 排序方式
// 说明	: 快速排序比较的过程
//************************************
int DivisionLine(std::vector<Line>& lines, int left, int right, const char& ch);
/*对线进行排序，按照索引排序*/
bool LinesSort(std::vector<int>& linesIndex, const char& ch, LineData& _linedata);
bool QuickSortLine(std::vector<int>& linesIndex, int left, int right, const char& ch, LineData& _linedata);
int DivisionLine(std::vector<int>& linesIndex, int left, int right, const char& ch, LineData& _linedata);

//************************************
// 方法	: MergeLines
// 全名	: MergeLines
// 访问	: public 
// 返回	: bool
// 参数	: lines
// 参数	: ch 合并类型
// 说明	: 对处在同一位置的线条进行合并
//************************************
bool MergeLines(std::vector<Line>& lines, const char& ch);

//************************************
// 方法	: MergeLines
// 全名	: MergeLines
// 访问	: public 
// 返回	: bool
// 参数	: linesIndexVec 需要处理的线条索引
// 参数	: ch 需要合并直线类型
// 参数	: _lineData 索引对应的实体
// 说明	: 合并重复的直线,即两条线条平行重合，并且相互包含
//************************************
bool MergeLines(std::vector<int>& linesIndexVec, const char& ch, LineData& _lineData);

//************************************
// 方法	: bPointBelongLineEnd
// 全名	: bPointBelongLineEnd
// 访问	: public 
// 返回	: bool
// 参数	: _line
// 参数	: point
// 说明	: 判断点是否属于一条线的端点
//************************************
bool bPointBelongLineEnd(const Line& _line, const Point& point);

//************************************
// 方法	: bConnectTowLine
// 全名	: bConnectTowLine
// 访问	: public 
// 返回	: bool 相接返回true ，否则返回 false
// 参数	: _line1
// 参数	: _line2
// 说明	: 判断两条直线是否相接
//************************************
bool bConnectTowLine(const Line& _line1, const Line& _line2);
//************************************
// 方法	: numToString
// 全名	: numToString
// 访问	: public 
// 参数	: num 需要转换的数字
// 说明	: 将字符转换为数字
//************************************
std::string numToString(const double& num);
//************************************
// 方法	: fineLineIndex
// 全名	: fineLineIndex
// 访问	: public 
// 参数	: line 需要查找的线实体
// 参数 : ch 为所要查找线的类型
// 说明	: 查找线实体的索引 （有些线条只知道位置信息，而不知索引信息需另外查找）
//************************************
int findLineIndex(LineData &lineData, const Line& line, const char& ch = 'H');
//************************************
// 方法	: findLineAdjacentIndex
// 全名	: findLineAdjacentIndex
// 访问	: public 
// 参数	: line 需要查找的线实体
// 参数 : size 索要查找的范围
// 参数 : ch 为所要查找线的类型
// 参数 : loc表示索要查找的范围 T上 B下 L左 R右
// 说明	: 查找线实体的索引 （有些线条只知道位置信息，而不知索引信息需另外查找）
//************************************
int findLineAdjacentIndex(LineData& lineData, const Line&line,const double& size, const char& type= 'H',const char& loc = 'T');



//gbk转UTF-8  
std::string GbkToUtf8(const std::string& __strGbk);// 传入的strGbk是GBK编码 
//UTF-8转gbk
std::string Utf8ToGbk(const std::string& __strUtf8);

namespace publicFunction
{
	//说明：找最靠近点p的一根线
	int findPointBaseLine(Data& data, const Point& p);
	//说明：返回一根线的所有交线(不包括它自己)
	std::vector<int> findAllCrossingLine(Data& data, int lineIndex);
	//说明：返回一根线的所有交线，需满足：交点是两条线的端点
	std::vector<int> findCrossing2(Data& data, int lineIndex);
	//说明：返回一跟线的交线，需满足1交点是两线端点 2两线分别是水平、竖直线
	std::vector<int> findCrossing3(Data& data, int lineIndex1);
	//说明：找这些线最多是聚集在哪个图层
	std::string findMaxAmountLayer(std::vector<int> lineIndices, Data& data);
	//说明：返回Box的四条线，可用于打印
	std::vector<Line> generateBoxBorder(const Box& box);
	//两根直线是否相交 并且得到交点
	bool checkCrossingAndGetCorner(const Line& line1, const Line& line2, Corner& corner);
	double calculateParallelDistance(Line& l1, Line& l2);
	// 说明	: 判断两个平行线之间的交叉长度
	double twoLineCrossLength(const Line& l1, const Line& l2);
	// 说明	: 两平行线交叉长度/较长的线的长度
	double calculateTwoLineCrossRatio(const Line& l1, const Line& l2);
	
	int roundToInt(double data);
	
	Line findMatchLine(std::vector<Line> lineFamily, Line lonelyLine);
	int findMatchLineIndex(Data& data, std::vector<int> indices, int index);
}

#endif
