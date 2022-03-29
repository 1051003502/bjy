#ifndef __PARSER_UTILS_MAINSTRUCTURESECTION_H__
#define __PARSER_UTILS_MAINSTRUCTURESECTION_H__
#include "data.h"
#include "blockSet.h"
#include "publicFunction.h"

#define smallLineRatio 0.2

class StirPointInfo
{
public:
	StirPointInfo() {}
	StirPointInfo(Point& infoPoint, std::vector<int>& leadIndexVec, std::vector<int>& stirIndexVec);

	Point infoPoint;//文本点
	int baseLineIndex;//基线索引
	std::vector<int> leadIndexVec;//文本点关联的线索引
	std::vector<int> stirIndexVec;//文本点所修饰的钢筋线索引
public:
	Point& getInfoPoint() { return infoPoint; }
	std::vector<int>& getStirIndexVec() { return stirIndexVec; }
	std::vector<int>& getLeadIndexVec() { return leadIndexVec; }
	bool setBaseLineIndex(int index) { return this->baseLineIndex = index; }
	int getBaseLineIndex() { return baseLineIndex; }
};
class StirBreakPoint {
public:
	StirBreakPoint() = default;
	StirBreakPoint(Line& line) {
		auto e = line.e;
		auto s = line.s;
		Point center((e.x + s.x) / 2, (e.y + s.y) / 2);
		center_mind = center;
		radius = line.length() / 2;//横筋半径
	}
	StirBreakPoint& operator=(const StirBreakPoint& p)
	{
		center_mind = p.center_mind;
		radius = p.radius;
		return *this;
	}

	//此处的2个重载目的：std::set支持StirBreakPoint类对象
	bool operator==(const StirBreakPoint& p) const {
		return std::abs(this->center_mind.x - p.center_mind.x) < Precision &&
			std::abs(this->center_mind.y - p.center_mind.y) < Precision;
	}
	bool operator<(const StirBreakPoint& p) const {
		if (this->center_mind.x + Precision < p.center_mind.x)return true;
		if (this->center_mind.x - Precision > p.center_mind.x)return false;
		if (this->center_mind.y + Precision < p.center_mind.y)return true;
		return false;
	}
	//判断此断点是否包含p点 用于判断引线端点是否被一个断点圈包含
	bool surround(const Point& p) {
		return this->center_mind.distance(p) < radius;
	}
	bool addTextPoint(Point& p) {
		this->textPoint = p;
		return true;
	};
public:
	Point center_mind;//圆心
	double radius;//半径
	Point textPoint;

};
class MainStructureSection
{
public:
	MainStructureSection() = default;
	//构造
	//MainStructureSection(const Block& block, Data& data);
	//!重载赋值函数
	//const MainStructureSection operator=(const MainStructureSection& mss);
	//!解析剖面
	bool parserMainStructureSectionInfo(Data& data);
	//初始化基础数据
	bool setBaseDate(const std::shared_ptr<Data>& m_data, const std::shared_ptr<Block>& p_block);
	std::vector<StirBreakPoint>& getBpVec() { return _bpVec; }
	std::vector<StirPointInfo>& getStirPointInfoVec() {
		return _stirPointInfoVec;
	}
	std::vector<int>& getBaseLineIndexVec() { return baseLineVec; }
	std::map<int, std::string>& getStirInfoMap() { return _stirInfoMap; }
public:
	std::string getRebarLayer() { return _rebarLayer; }
public:
	//通过文本的基础线查找与之相关联的线条
	std::vector<int> BFSFindLine(const int& baseLineIndex, CornerData& cornerData, LineData& lineData, bool& baseLineSide);
	//通过基准线找对应的一撮引线
	bool findLeadLine(const Block& block, Data& data, std::vector<int>& baseLineVec
		, std::vector<std::vector<int> >& leadLineGroup);
	//检查text是否是钢筋标注
	bool checkRebarText(std::string text);
	//找钢筋标注对应的基准线放入baseLineVec中
	bool findBaseLine(const Block& block, Data& data, std::vector<int>& baseLineVec, std::vector<Point>& pointVec);
	//判断点是否在线内
	bool pointCloseLine(const Point& point, const Line& line);
	//检查line是否满足基线条件 暂时只通过长度来判断
	bool checkBaseLine(Line line);
	//判断点与线是否平行
	bool isparallel(const Point& pt, const Line& line);
	//判断文本点的方向 //1水平 0垂直  -1斜着的
	int getPointDirection(const Point& p);
	//从一个引线集合中，找出有表示作用的小短线
	double findSmallLine(std::vector<Line>& lineData, std::vector<int>lineIndexVec);
	//解析剖面配筋图的具体钢筋信息，即获取，_bpLayer图层，_rebarLayer图层，_bpVec,v h IndexVec
	//初始化水平筋断点
	bool iniStirBreakPoint(Data& data);
	//找非水平筋图层
	bool findRebarLayer(Data& data, std::vector<std::vector<int> >& leadLineIndex);
	//用非水平筋的图层初始化非水平筋的线
	bool findRebarLineIndex(Data& data);
	//将文本与筋对应起来
	bool mapStrAndRebar(Data& data, std::vector<std::vector<int> >& leadLineGroup, std::vector<Point>& pointVec);
	bool cleanRebarData(Block& block, Data& data);
	//连接断开的板线 （也就是寻找板线，板线如果很短是一个例外情况，有待优化）
	bool connectLine();
	//找主体结构图层（判定条件：贯通筋旁边的线）
	bool findMainLayer();

public://剖面图基础数据
	std::shared_ptr<Block> _spBlock;
	std::shared_ptr<Data> _spData;
	std::string _bpLayer;//水平筋所在图层
	std::vector<int> baseLineVec;//基线
	std::vector<int> _bplineIndexVec;//水平筋的线
	std::string _rebarLayer;//非水平筋所在图层
	std::vector<int> _hLineIndexVec;//!非水平筋的横向索引
	std::vector<int> _vLineIndexVec;//!非水平筋的纵向索引
	std::vector<int> _sLineIndexVec;//!非水平筋的斜线索引
	std::vector<StirBreakPoint> _bpVec;//!水平筋数据
	std::vector<StirPointInfo>_stirPointInfoVec;//非水平筋数据
	std::map<int, std::string> _stirInfoMap;//经过清洗后的非水平筋数据
	std::string _mainLayer;

public:/*墙*/
	std::vector<int> _wallLineIndexVec;//竖直墙线
	std::vector<Line> _wallLineVec;//竖直连接后的墙线
	std::vector<Line> _cuttingWallLineVec;//经过板截断之后的墙线
	//std::vector<std::shared_ptr<ProfileWall>> _profileWallVec;
public:
	//处理主体图层的一些数据
	bool findMainLine();
	//根据主体图层找墙线
	bool findWallLineIndex();
	bool findWallConnectRebar();
	//利用板线截断出新的墙线
	bool cutWall();
	//三层板的情况
	void normalCutting();
	//四层板的情况
	void specialCutting();

	//根据_mainstru中的_cuttingwallLineVec生成墙，放入profileWallVec
public:/*板*/
	std::vector<int> _boardLineIndexVec;//水平板线
	std::vector<Line> _boardLineVec;//水平连接后的板线
	double bottomAttitude;//底板顶部绝对坐标（y值）
	bool findBoardLineIndex();
	double getBottomAttitude() {
		if (_boardLineVec.size() >= 2)return _boardLineVec[1].s.y;
		else return 0;
	}
	//用于打印线条
public:
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
	//初始化画图构件
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	/*std::vector<int> writeIndexVec;
	std::vector<int> greenIndexVec;
	std::vector<std::vector<int> > colorBlocks;
	std::vector<Line> writeLineVec;*/
};




#endif