#ifndef __PARSER_UTILS_BEAM_H__
#define __PARSER_UTILS_BEAM_H__
#include <fstream>
#include <regex>
#include <chrono>
#include "block.h"
#include "data.h"
#include "publicFunction.h"
#include "ColumnSection.h"
#include "beamSection.h"
#include "pillar.h"


class Beam;

struct BeamPosZDelta
{
	BeamPosZDelta() = default;
	BeamPosZDelta(const BeamPosZDelta& o) = default;
	double rad;
	double length;
	Point pt;
	PillarDistance axisDistance;
};
using RelativeDistance = std::pair<std::string, double>;

struct Distance2AxisInCartesianCoordinates
{
	Point pt;
	RelativeDistance relativeHDistance;
	RelativeDistance relativeVDistance;
};

class BeamPosition
{
private:
	static const std::string LongitudianlPriorityReg;

public:
	//支撑类型
	enum class SupportType
	{
		Column,//柱
		Beam,//梁
		Wall,//墙
		Pending,//悬空
		Unknown//未知
	};

	//顶部和底部
	enum class LongitudinalPos
	{
		Up,
		Down
	};


	BeamPosition() = default;
	~BeamPosition() = default;

	//************************************
	// 方法	: ExtractPriorityAnnotation
	// 全名	: BeamPosition::extractPriorityAnnotation
	// 访问	: public 
	// 返回	: bool 解析成功返回true，否则false
	// 参数	: d 箍筋位置
	// 参数	: strText 识别文本
	// 说明	: 识别梁位置的纵筋箍筋信息
	//************************************
	bool extractPriorityAnnotation(LongitudinalPos d, const std::string& strText);

	bool extractMidPriortyAnnotation(const std::string& strText);
	//说明：对外接口
	std::string getSupportType();
	std::string getWaistReinforce();
	std::string getRelativeElevation();
	std::string getTopReinforce();
	std::string getBottomReinforce();
	

public:
	SupportType supportType;					//支撑类型
	double supportWidth;						//支撑宽度
	bool bMiddle;								//是否有跨中信息
	std::string waistLongitudinalReinforce;		//跨中腰纵筋
	std::string relativeElevation;				//相对标高
	std::string topLongitudinalReinforce;		//顶部纵筋
	std::string btmLongitudinalReinforce;		//底部纵筋

	//截断长度
	std::set<std::pair<double, double>, std::greater<std::pair<double, double>>> topTruncatedLength;// <pseudo coordinate y,length>
	std::set<std::pair<double, double>, std::greater<std::pair<double, double>>> btmTruncatedLength;
	//锚固长度
	std::set<std::pair<double, std::string>, std::greater<std::pair<double, std::string>>> topAnchorageLength;//<pseudo coordinate y,string>
	std::set<std::pair<double, std::string>, std::greater<std::pair<double, std::string>>> btmAnchorageLength;

	//跨位置的上下翻信息
	BeamPosZDelta topZDelta;
	BeamPosZDelta btmZDelta;

	//截面信息
	std::shared_ptr<BeamSection> spBeamSection;
};

class BeamSpan
{
public:
	//跨中的位置
	enum class SpanPosition
	{
		Left,
		Right,
		Mid
	};

	BeamSpan() = default;
	~BeamSpan() = default;
	//跨的方向
	enum class SpanDirection
	{
		Horz,
		Vert,
		Slash
	};

	//************************************
	// 方法	: SearchAnnotationPriority
	// 全名	: BeamSpan::SearchAnnotationPriority
	// 访问	: public 
	// 返回	: bool
	// 参数	: spBeam
	// 参数	: box
	// 参数	: data
	// 说明	: 根据梁的方向查找原位标注
	//************************************
	bool searchAnnotationPriority(std::set<int>& textPtSet, std::shared_ptr<Beam> spBeam, const Box& box, Data& data, SpanPosition spanPos);

	//************************************
	// 方法	: SearchMidPriority
	// 全名	: BeamSpan::SearchMidPriority
	// 访问	: public 
	// 返回	: bool
	// 参数	: spBeam
	// 参数	: box1
	// 参数	: box2
	// 参数	: data
	// 说明	: 找中间的原位标注
	//************************************
	bool searchMidPriority(std::set<int>& textPtSet, std::shared_ptr<Beam> spBeam, Box box1, Box box2, Data& data);

	void  searchBorders(const std::shared_ptr<Pillar>& col2,
						const std::shared_ptr<Pillar>& col1,
						std::shared_ptr<Beam> _spBeam);

	bool updateWithAndDirection();

	bool updateNetLength(const std::shared_ptr<Pillar>& col2,
						 const std::shared_ptr<Pillar>& col1);

	bool updateSpanMidlinePosition(const std::shared_ptr<Pillar>& col1,
								   const std::shared_ptr<Pillar>& col2,
								   std::shared_ptr<Axis>& axis,
								   std::vector<int>& idxVec,
								   std::vector<Line>& lineVec);

	void getLeftStartOfMidline(Line& l1,
							   Line& l2,
							   Line& tpLine);

	Point getLeftStartInMidlineOfBtmLine(Line& btmLine,
										 Line& topLine,
										 Line midline);


	Point getRightEndInMidlineOfBtmLine(Line& btmLine,
										Line& topLine,
										Line midline);

	Point getLeftStartInMidlineOfTopLine(Line& topLine,
										 Line& btmLine,
										 Line midline);

	Line getMidline(Line& l1, Line& l2);

	Line getVLineBasedOnStartOfLine2(Line& l1,
									 Line l2,
									 double offset = 0.0);

	Line getVLineBasedOnEndOfLine2(Line& l1,
								   Line l2,
								   double offset = 0.0);

	bool checkRightBorderBound(Point& pt,
							   const std::shared_ptr<Pillar>& pillar2,
							   Line& l1);

	bool checkLeftBorderBound(Point& pt,
							  const std::shared_ptr<Pillar>& pillar1,
							  Line& l1);

	//说明：对外接口
	unsigned char getSpanId();//返回梁跨序号
	std::pair<std::string, std::string>getSize();//返回梁的尺寸
	double getSpanNetLength();//返回梁跨净长
	double getRelativeElevation();//返回梁的相对标高
	std::vector<Line>getBorders();
	Distance2AxisInCartesianCoordinates beamLocStart();
	Distance2AxisInCartesianCoordinates beamLocEnd();
	std::vector<BeamPosition> getBeamPosition();//肯定会有三个，第一个做，二中，三右





public:
	unsigned char spanId;				//梁跨序号
	std::string strSecHeight;			//断面高
	std::string strSecWidth;			//断面宽
	double spanNetLength;				//净长
	double relativeElevation;			//相对标高
	BeamPosition left;					//左端支撑
	BeamPosition right;					//右端支撑
	BeamPosition mid;					//跨中
	std::pair<int, int> supportPair; //支撑类型的索引对

	// 跨的位置信息
	Distance2AxisInCartesianCoordinates midlineStart;
	Distance2AxisInCartesianCoordinates midlineEnd;
	SpanDirection spanDirection;		// 跨的朝向
	double redundantWidth;				//冗余的宽度
	std::vector<Line> borders;

	//! test 
	std::vector<int>* pIdxVec;
	std::vector<Line>* pLineVec;
private:
	void getRightEndOfMidline(Line& l1, Line& l2, Line& tpLine);
	Point getRightEndInMidlineOfTopLine(Line& topLine, Line& btmLine, Line& midline);
};


class Beam
{

public:
	static const std::string BeamReg;
	static const std::string BeamReg2;
	static const std::string SpanAxisSecReg;
	static const std::string SpanReg;
	static const std::string SecSizeReg;
	static const std::string StirrupReg;
	static const std::string LongitudinalReforceReg;
	static const std::string LongitudinalReforceReg2;
	static const std::string WasitLongitudinalReforceReg;
	static const std::string ElevationReg;
	static const std::string PriorityReg;
	//梁信息文本匹配模式
	static const std::string BeamSecSizeReg;
	static const std::string BeamBracketReg;//匹配括号内的信息
	static const std::string SpanAxisReg;//匹配跨或轴信息


public:
	//梁的朝向
	enum class Direction
	{
		H,
		V,
		S,
	};

	Beam() = default;
	~Beam() = default;

	bool getUUID(std::string strInfo);

	//************************************
	// 方法	: ExtractBeamSecSize
	// 全名	: Beam::extractBeamSecSize
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取尺寸
	//************************************
	bool extractBeamSecSize(const std::string& strText);

	//************************************
	// 方法	: ExtractBeamInfo
	// 全名	: Beam::extractBeamInfo
	// 访问	: public 
	// 返回	: bool 提取成功true，否则false
	// 参数	: strText 分析的文本
	// 说明	: 尽可能提取梁的信息提取
	//************************************
	bool extractBeamInfo(const std::string& strText);

	//************************************
	// 方法	: ExtractBeamInfo1
	// 全名	: Beam::extractBeamInfo1
	// 访问	: public 
	// 返回	: bool 提取无误true，否则false
	// 参数	: strText 所要提取的文本
	// 说明	: 提取梁号
	//************************************
	bool extractBeamInfo1(const std::string& strText);

	//************************************
	// 方法	: ExtractBracketInfo
	// 全名	: Beam::extractBracketInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取Info1中括号内的信息
	//************************************
	bool extractBracketInfo(const std::string& strText);


	//************************************
	// 方法	: ExtractSpanAxis
	// 全名	: Beam::extractSpanAxis
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取跨和轴放在一起的信息
	//************************************
	bool extractSpanAxis(const std::string& strText);

	//************************************
	// 方法	: ExtractBeamInfo2
	// 全名	: Beam::extractBeamInfo2
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取箍筋
	//************************************
	bool extractBeamInfo2(const std::string& strText);


	//************************************
	// 方法	: ExtractBeamInfo3
	// 全名	: Beam::extractBeamInfo3
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取顶部底部纵筋信息
	//************************************
	bool extractBeamInfo3(const std::string& strText);


	//************************************
	// 方法	: ExtractBeamInfo4
	// 全名	: Beam::extractBeamInfo4
	// 访问	: public 
	// 返回	: bool
	// 参数	: strText
	// 说明	: 提取腰筋
	//************************************
	bool extractBeamInfo4(const std::string& strText);

	//************************************
	// 方法	: ExtractBeamElevation
	// 全名	: Beam::extractBeamElevation
	// 访问	: public 
	// 返回	: bool
	// 说明	: 提取相对标高
	//************************************
	bool extractBeamElevation(const std::string& strText);



	//************************************
	// 方法	: UpdateDirectionWidth
	// 全名	: Beam::updateDirectionWidth
	// 访问	: public 
	// 返回	: bool
	// 说明	: 更新梁架设方向以及梁的宽度
	//************************************
	bool updateDirectionWidth();

	//************************************
	// 方法	: operator==
	// 全名	: operator==
	// 访问	: public 
	// 返回	: friend bool
	// 参数	: beam1
	// 参数	: beam2
	// 说明	: 判断两个梁基本信息是否相等
	//************************************
	friend bool operator==(std::shared_ptr<Beam>& beam1, std::shared_ptr<Beam>& beam2);

	//************************************
	// 方法	: AppendSpan
	// 全名	: Beam::appendSpan
	// 访问	: public 
	// 返回	: bool
	// 参数	: span
	// 说明	: 将该跨加入到该梁中
	//************************************
	bool appendSpan(std::shared_ptr<BeamSpan> span);
	
	//说明：对外接口
	std::string getName();//返回梁名
	unsigned char getSpanNum();//返回梁跨数
	bool getbAssemblyAnnotation();//返回是否为平法
	std::vector<std::shared_ptr<BeamSpan>>getBeamSpanVec();





public:
	uuid beamId;
	std::vector<std::shared_ptr<BeamSpan>> spSpanVec;
	unsigned char spanNum;  //跨数
	std::vector<Line> borderLineVec;//梁的边界线
	//ColumnSection section;
	int leadIdx;//描述该梁的引线索引
	std::vector<Axis::AxisLine> beamRegion;
	//用于存取串连梁的柱支撑
	std::vector<int> colChain;
	//文本的高度（作为参考高度）
	double refHeight;

	//该梁所持有的集中标注的文本点的索引
	std::set<int> textPtIdxSet;

	bool bAssemblyAnnotation;				//是否是平法标注
	std::string sAxis;						//结束轴
	std::string eAxis;						//开始轴
	std::string strBeamName;				//梁的名字
	std::string strSecSize;					//断面尺寸
	std::string strStirrup;					//箍筋
	std::string topLongitudinalReinforce;//顶部纵筋
	std::string btmLongitudinalReinforce;//底部纵筋
	std::string waistLongitudinalReinforce;//腰纵筋

	std::string relativeElevation;//标高
	Direction direction;//是否是水平的梁
	double widSize;//梁的宽度

public:
	//测试
	std::vector<int> colIdxVec;
};

#endif