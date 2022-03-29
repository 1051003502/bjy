#ifndef __PARSER_UTILS_SECTION_H__
#define __PARSER_UTILS_SECTION_H__
#include "line.h"
#include "dimension.h"
#include <iostream>
#include <fstream>
#include "block.h"
#include "json/json.h"
#include <stdlib.h>
#include <functional>
#include "data.h"
#include "publicFunction.h"



typedef enum ColumnDirection
{
	L1, //左
	R1, //右
	U1, //上
	D1, //下
	N1  //空
} ColumnDirection;

class ColumnBreakPoint
{
public:
	Point center_mind;
	double radius;
	int index;
};

class ColumnBPInfo
{
public:
	typedef enum _Direction
	{
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	}Direction;
public:
	std::vector<int>bpLeadVec;//纵筋的引出线索引
	std::vector<int>bpVec;//纵筋
	std::vector<std::string>bpInfoVec;//纵筋信息
	std::vector<int>bpInfoPointIndex;//信息对应的文本点索引
	Direction dir;

};

class ColumnStirLeadInfo
{
public:
	std::vector<int>stirLeadVec;//箍筋引线
	//std::vector<int>stirVec;//与箍筋引线的相交线箍筋线
	std::string stirInfo;//箍筋引线描述的信息
	Point textPoint;//文本点
	bool bSingle;
};


class ColumnStirrupHook
{
public:
	ColumnStirrupHook() 
	{
		hook_index1 = -1;
		hook_index2 = -1;
	}
	ColumnStirrupHook(const int& ide1, const int& ide2)
	{
		this->hook_index1 = ide1;
		this->hook_index2 = ide2;
	}
	ColumnStirrupHook& operator=(const ColumnStirrupHook&hook)
	{
		this->hook_index1 = hook.hook_index1;
		this->hook_index2 = hook.hook_index2;
		return *this;
	}
	bool operator==(const ColumnStirrupHook&hook)
	{
		if (((this->hook_index1 == hook.hook_index1&&
			this->hook_index2 == hook.hook_index2)||
			(this->hook_index1 == hook.hook_index2&&
				this->hook_index2 == hook.hook_index1)))
			return true;
		else
			return false;
	}
public:
	int hook_index1;
	int hook_index2;

};

class ColumnSStirrup
{
public:
	ColumnSStirrup() 
	{
		single = false;
	}
	ColumnSStirrup(const std::vector<Line>&c_l, const ColumnStirrupHook&hook,const bool &single)
	{
		this->circle_line = c_l;
		this->hook = hook;
		this->single = single;
	}
	ColumnSStirrup(const Line& line)
	{
		this->circle_line.push_back(line);
		this->single = true;
	}
	ColumnSStirrup&operator=(const ColumnSStirrup&s_l)
	{
		this->circle_line = s_l.circle_line;
		this->hook = s_l.hook;
		return *this;
	}
public:
	bool single;
	std::vector<Line> circle_line;	//匝线
	ColumnStirrupHook hook;
};

class ColumnSection
{
public:

public:
	typedef std::vector<int>Connected;
	typedef std::pair<std::string, Connected>Explanation;//引出线的解释
	typedef std::pair<double, double>Dim_Information;//第一个参数表示方向，第二个参数标识长度
	typedef std::pair<std::string, int>Stirrup;//箍筋的型号和箍筋的数目

	typedef struct _BreakPoint
	{
		Point center_mind;
		double radius;
	}BreakPoint;

	ColumnSection() {
		linear_ration = 1;
		section_id = -1;
		_bCircle = false;
		_rectangle = true;
		_maxHeight = 0;
		_maxHeight = 0;
	}
	ColumnSection(std::vector<BreakPoint>temp_bp, std::vector<Dimension>dimensions);
	bool SetBreakPoint(BreakPoint &bp, const Point& p, const double &r);
	double GetPillarWidth();
	bool SetSectionWidthHeight();
	bool SetSectionbox();
	bool SetSectionOverallWH();			//
										//bool SectionInformatioOutput();
										//
										//初始化截面名字
	bool SetSectionBlockName(const Block &section_block);
	bool SetSectionBlockBox(const Block&section_block);
	bool QuickSortLine(std::vector<Line>&lines, int left, int right, const char&ch);
	int DivisionLine(std::vector<Line>&lines, int left, int right, const char&ch);
	bool sortExplanation(std::vector<std::pair<Explanation, Point>>&explanation);
	long setSectionId(const std::string &sectionName);
	bool boolRectangle(bool *type_L);		//形状描述
	std::vector<Point> sortSurroundLine();
	bool DescribeShape(std::string &str, const bool &type_L); //异形柱形状分布
	bool setRectangleBH(Json::Value& Bars, const bool&type_l);	//设置断面的b,h 侧箍筋
	//************************************
	// 方法	: setRectangleBH
	// 全名	: ColumnSection::setRectangleBH
	// 访问	: public 
	// 返回	: bool
	// 参数	: bSide b 侧数据
	// 参数	: hSide h 测数据
	// 参数	: type_l
	// 说明	: 用于判断矩形柱，纵筋的h侧与b侧
	//************************************
	bool setRectangleBH(std::string&bSide, std::string&hSide, const bool &type_l);
	//************************************
	// 方法	: setStirrupHookCount
	// 全名	: ColumnSection::setStirrupHookCount
	// 访问	: public 
	// 返回	: bool
	// 参数	: hCount 横支
	// 参数	: vCount 纵支
	// 说明	: 用于初始化，箍筋的支数
	//************************************
	bool setStirrupHookCount(int &hCount, int &vCount);
	bool SetListStittups();//箍筋的排列信息
	bool del_char(std::string &str);//删除字符串中的特定字符
	//std::string numToString(const double&num);
	Json::Value OutPut();//导出柱信息
	Json::Value OutPutInSitu();//原位标注
	Json::Value OutPutCenter();//集中标注
public:

	bool SetSectionScale(std::string &scale, const Box&section_box, Data& data);//初始化比例尺寸

	bool SetSectionStirrup(std::vector<ColumnSection::Stirrup> &stirrup, const Box &section_box, Data& data);//初始化箍筋信息
																									   //对断开的箍筋标志做连接
	bool amendHooks(const int hook1, const int hook2, std::map<int, std::vector<Corner>>::const_iterator corners1,
		std::map<int, std::vector<Corner>>::const_iterator const corners2, std::vector<Corner>& corner1,
		std::vector<Corner> &corner2, Data& data);
	//初始化section的breakPoint外边界
	std::vector<ColumnSection::BreakPoint> ReturnSectionTopAngle(const ColumnSection &section);
	bool SetSectionBreakPointSurround(ColumnSection &section, Data &data);
	bool SetSectionHook(std::vector<ColumnStirrupHook> hooks, ColumnSection &section, Data &data);

	bool SetSectionBox(ColumnSection &section, Data &data);//设置箍筋的包围线

	bool SetSectionStirrupReferencePoint(ColumnSection &section, Data &data);//设置箍筋的参考点
																	   //初始化箍筋信息
	bool SetSectionStirrupInformation(ColumnSection &section, Data &data);
	/*箍筋的识别*/

	bool InTurnLinkStirrup(ColumnSection &section, Data &data);//根据连通性匹配箍筋

	bool DirecctionMatchingStirrup(ColumnSection &section, Data &data);//根据方向匹配箍筋

																 //在已有箍筋的基础上再次校验单支箍的情况，以及箍筋标志不标准的情况
	bool CheckStirrup(ColumnSection &section, Data &data);

	bool FindLongitudinalTendonCollection(const std::vector<Point> &center_mind,
		std::vector<std::vector<int>> &connect_line, Data &data);//纵筋的集合线
	bool SetLTC(std::vector<std::vector<int>> &l_t_c, const std::vector<std::vector<int>> &l_t_c1);

	bool FindExplanation(const std::vector<std::vector<int>>&lines_index, const Box&temp_box,
		const Box&block_box, std::vector<std::pair<ColumnSection::Explanation, Point>>&explanation, Data &data);//查找箍筋断面关联信息

	bool RetrunLeadLineType(const std::vector<int> &index, const Box &temp_box, Point &goal_point, Data &data);//判断引出线的类型，返回true为闭合型，否则发散型
	bool SetSectionExplanation(std::vector<std::pair<ColumnSection::Explanation, Point>>&explanation,
		const std::vector<std::pair<ColumnSection::Explanation, Point>>&lead_line_explanation);
	bool CenterNote(ColumnSection &section, Data &data);

public:
	std::vector<BreakPoint>break_point;		//所有箍筋的断点信息
	std::vector<Dimension>dimensions;		//标注信息
	std::vector<int>lead_line;				//引出线
	std::vector<std::vector<int>>l_t_c;		//归类引出线
	Box section_block_box;					//柱断面的块范围 ---整个详图的obx
	Box section_box;						//柱详图的块范围 ---箍筋的box
	std::vector<int> pillar_surround;		//柱子的包围盒 -----柱子断面的box
	Point stirrup_refer_point;				//箍筋的参照点
	std::vector<ColumnStirrupHook>hooks;			//箍筋的勾
	std::vector<int>bp_surround;			//纵筋包围盒
	std::vector<int>stirrup_lines;          //候选箍筋线
	LineData line_data;
	CornerData corner_data;
public:
	//原位注写
	std::string name;
	std::string scale;						//比例
	std::vector<ColumnSStirrup>stittups;			//箍筋 宽*高 相对位置 支数
	std::vector<std::string>list_stittups;	//箍筋的排列信息
	double all_section_width;				//整体的宽
	double all_section_height;				//整体的高
											//Box stirrup_refer_box;				//箍筋参照盒
	std::vector<std::pair<Explanation, Point>>explanation;	//引出线解释
	std::vector<Stirrup>stirrup_model;		//箍筋型号信息加密区间等信息
	std::vector<Dim_Information>u_width;	//上部断面宽标注
	std::vector<Dim_Information>d_width;	//下部断面宽标注
	std::vector<Dim_Information>l_height;	//左部断面的高标注
	std::vector<Dim_Information>r_height;	//右部断面的高标注
	unsigned int section_id;						//截面Id
public:
	//集中注写
	typedef enum {
		L,//左
		R,//右
		U,//上
		D, //下
	}Direcction;

	typedef enum {
		LEFT,//左
		RIGHT,//右
		UP,//上
		DOWN, //下
	}Direction1;

	std::vector<std::pair<std::string, Direcction>>bp_string;
	std::vector<std::string>all_txt;


protected:
	double linear_ration;//线性比例

/************************************新算法******************************************/
public:
	//public function 
	static ColumnSection retrunColumnSection(Block& block, Data& data);
	bool testFuction(std::vector<int>&test_indexVec,std::vector<Point>&cornerVec);
	Json::Value outputColumnSection();
private:
	//private function
	bool iniColumnBlock(const Block& _columnBlock);
	//初始化柱截面内的所有断点
	bool iniBreakPoint(const Block& block, Data& data);
	//查找是否存在相同的断点，因为当纵筋的断点由多段线组成是，它会是两条重复的线条
	bool findBreakPoint(const Point& p);
	//初始化箍筋信息
	bool iniStirrup(Data& data);
	//初始话所有箍筋的索引
	bool iniStirrupIndex(LineData& _lineData);
	//返回箍筋的图层信息
	bool iniStirrupLayer(LineData& _lineData);
	//返回一个由所有纵筋形成的包围盒
	Box bpSurroundBox(LineData& _lineData);
	//初始化梁断面内所有箍筋的标志勾
	bool iniColumnHook(LineData& lineData, RTreeData& goalRTree);
	// 说明	: 对已经识别出的箍筋线，再次细化，细化为横线，竖线，斜线，并且对此进行排序合并
	bool divideStirLineType(std::vector<int>& v_lines,
		std::vector<int>& h_lines,
		std::vector<int>& ss_lines,
		Data& data);

	// 初始化箍筋方向
	bool setStirrupDirection(std::vector<int>& _vLines,
		std::vector<int>& _hLines,
		std::vector<int>& _sLines,
		Data& _data);
	// 说明	: 将带有方向的箍筋与，未带方向的箍筋进行配对
	bool makePairStir(std::vector<std::pair<int, int>>& hStirPair,
		std::vector<std::pair<int, int>>& vStirPair,
		const std::vector<int>& v_lines,
		const std::vector<int>& h_lines,
		Data& data);
	// 说明	: 进一步区分箍筋的方向性
	bool divideStirrupDir(const std::vector<int>& lineIndex,
		std::map<int, ColumnDirection>& goalIndex,
		const char& ch);
	// 说明	: 校验箍筋
	bool checkStir(Data& data);
	// 说明	: 对已经配对好的箍筋做补充
	ColumnSStirrup checkStirPair1(std::vector<int>stirDirIndex, LineData& _lineData);
	// 说明	: 对已经配对，且两个箍筋标志都已配对，做正确性校验
	ColumnSStirrup checkStirPair2(std::vector<int>stirDirIndex, Data& data);
	// 说明	: 为自己计算出的箍筋，匹配对应的箍筋索引，目的用于后续单支箍的判断
	bool findSameStir(const Line& _line, int& lineIndex, LineData& _lineData);
	// 说明	: 将柱断面内的单支箍，用排除的方式从所有箍筋内找到找到
	bool findColumnSitrrupSignal(Data& data);
	// 说明	: 查找所有用于描述柱纵筋的引线
	bool findBpLeadLine(Data& data);
	// 说明	: 初始化柱断面的箍筋标注信息
	bool iniColumnStirLead(Data& data);
	// 说明	: 分离箍筋的引出线,当箍筋存在多个引线时
	std::vector<int> divideStirLead(LineData& _lineData);
	// 说明	: 初始化柱断面箍筋信息
	bool iniRebarInfo(Data& data);
	// 说明	: 初始化引出线所对应的信息，以及引出线对应的文本点
	bool iniBeamLeadInfo(std::vector<int>& textPointIndex, Point& refPoint, const std::vector<int>& _lineIndex, const bool& _bpLead, Data& data);
	// 说明	: 在几条引出线中找到合适的文本搜索点
	bool findColumnLeadInfoPoint(Point& point, double& searchR, const std::vector<int>& _lineIndex, const bool& _bpBox, Data& data);
	// 说明	: 判断引出线的方向，用于校验文本与引出线的映射是否正确，经过对比文字一般会与一出线平行
	bool returnLeadLineDir(char& ch, const bool& _bpBox, const std::vector<int>_leadLineIndex, Data& data);
	// 说明	: 设置箍筋标注描述信息所描述的是否为单只箍
	bool setStirInfoType(Data& data);
	// 说明	: 设置箍筋标注描述信息所描述的是否为单只箍
	bool setBPLeadInfoDirection(LineData& _lineData);
	// 说明 : 为纵筋进行排序
	bool sortBreakPointToLine();
	// 说明 : 截面标注
	bool iniColumnDImensions(Data& data, const std::vector<std::shared_ptr<Dimension>>& _spDimensions);
	// 说明 : 初始化截面的尺寸
	bool iniMaxSize();
	// 说明 : 初始化横向纵筋截面
	std::vector<std::vector<int>>setHBreakPointLine();
	// 说明 : 初始纵筋纵筋截面
	std::vector<std::vector<int>>setVBreakPointLine();
	// 说明 : 初始化一列纵筋端点为一条线
	std::vector<Line> setBreakPointVecToLine(const std::vector<std::vector<int>>& BreakPointVec, const char& ch = 'H');
	// 说明 : 设置箍筋的Json数据
	Json::Value stirrupVaule();
	// 说明 : 初始化箍筋位置
	Json::Value stirrupLoc();
	// 说明 : 确定某线相对纵筋的位置
	Json::Value lineRelativeBPLoc(const Line& line, const char& ch = 'H');
	// 说明 : 设置纵筋的json数据
	Json::Value breakPointInfo();




private:
	/********process data***/
	bool _bCircle;
	Box _bpBox;
	Box _stirBox;//由箍筋组成的box
	Block _columnBlock;//柱截面块信息
	std::string _bpLayer;//断点所在图层
	std::string _stirrupLayer;//!梁截面箍筋所在图层
	std::vector<ColumnBreakPoint>break_pointVec;//!所有箍筋的断点信息
	std::vector<int>_stirrupIndexVec;//!所有箍筋的索引
	
	std::vector<ColumnSStirrup>_columnStirrupVec;//!柱箍筋

	//!单双之箍索引是为了判断箍筋描述信息所描述的是单支箍，还是多支箍
	std::vector<int>_singleStirIndex;//!单支箍索引
	std::vector<int>_doubleStirIndex;//!双支箍索引

	std::vector<ColumnBPInfo>_columnBPinfoVec;//!纵筋信息
	std::vector<int>_bpLeadLineVec;//!纵筋断点的所有引出线
	std::vector<Dimension>_dimensionsVec;//!梁断面标注信息
	std::vector<ColumnStirLeadInfo>_columnStirInfo;//!梁断面的箍筋信息,指的是描述字符串

	std::vector<std::pair<Line, std::vector<int>>>_bpHLine;//在同一行的纵筋
	std::vector<std::pair<Line, std::vector<int>>>_bpVLine;//在同一列的纵筋

	
private:
	//字典
	std::map<int, int>_bpIndexMap;//!创建所有断点的索引字典由于快速的排除非箍筋线
	std::map<int, bool>_bStirPair;//!用于判断箍筋是否配对，从而排查出单支箍
	std::map<int, ColumnDirection>_stirrupDirMap;//!箍筋方向 int表示箍筋线条的索引，Direction表示此线条的方向
	std::map<int, int>_stirDirHookIndexMap;//!用于映射箍筋对应的hook，第一个int表示箍线的索引，第二个int表示beamHookVec的索引
	std::map<int, std::vector<int>>_hookStirrupIndexMap;//!用于映射，标志随配对后的箍筋线索引，用于快速查找

private:
	/*****result data*******/
	std::string _name;
	double _seale;
	std::string _stirrup;
	bool _rectangle;
	double _maxWidth;
	double _maxHeight;
	double _scale;
	int _countH;
	int _countV;

public:
	//测试参数
	std::vector<int>testIndexVec;
	//std::vector<Point>cornerVec;
	Box testBox;
	std::vector<ColumnStirrupHook>_columnHookVec;
	std::vector<Line>testLine;

};






#endif