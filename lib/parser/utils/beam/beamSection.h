#ifndef __PARSER_UTILS_BEAMSECTION_H__
#define __PARSER_UTILS_BEAMSECTION_H__
#include "block.h"
#include "data.h"
#include "publicFunction.h"
#include "dimension.h"
#include <stack>

/*!
* \file beamSection.h
*
* \author Xiong
* \date ����
*为箍筋线条生成方向，用于配对箍筋
*
*/

typedef enum
{
	L, //�?
	R, //�?
	U, //�?
	D, //�?
	N  //�?
} Direction;

//!梁断面的纵筋断点结构
typedef struct _BraeakPoint
{
	Point center_mind;
	double radius;
	int index;
}BreakPoint;
/*!
* \file beamSection.h
*
* \author Xiong
* \date ����
* 用于详细描述纵筋的类�?
*
*/
class BeamBPInfo
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

class BeamStirLeadInfo
{
public:
	std::vector<int>stirLeadVec;//箍筋引线
	//std::vector<int>stirVec;//与箍筋引线的相交线箍筋线
	std::string stirInfo;//箍筋引线描述的信�?
	Point textPoint;//文本�?
	bool bSingle;
};

/*!
* \file beamSection.h
*
* \author Xiong
* \date ����
*此类是箍筋的标志“勾”的索引
*
*/

class BeamStirrupHook
{
public:
	BeamStirrupHook()
	{
		int hook_index1 = -1;
		int hook_index2 = -1;
	}
	BeamStirrupHook(int hook1, int hook2)
	{
		this->hook_index1 = hook1;
		this->hook_index2 = hook2;
	}
	friend bool operator==(const BeamStirrupHook &hook1, const BeamStirrupHook &hook2)
	{
		if ((hook1.hook_index1 == hook2.hook_index1 &&
			hook1.hook_index2 == hook2.hook_index2) ||
			(hook1.hook_index1 == hook2.hook_index2 &&
				hook1.hook_index2 == hook2.hook_index1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
public:
	int hook_index1;
	int hook_index2;

};

/*!
* \file beamSection.h
*
* \author Xiong
* \date ����
* 箍筋匝属�?
*
*/
class BeamSStirrup
{
public:
	BeamSStirrup()
	{
		single = false;
	}
public:
	bool single;//!判断是否为单支箍
	BeamStirrupHook hook;//箍筋标志
	std::vector<Line> circle_line;	//匝线

};


class BeamSection
{
public:
	typedef std::vector<int>ConnectedVec;
	typedef std::pair<std::string, ConnectedVec>Explanation;//!纵筋断面的引出线集合线，以及其说�?



	BeamSection()
	{
		this->seale = -1;
		this->maxHeight = 0.0;
		this->maxWidth = 0.0;
		this->sectionType = "rectangle";
	}
public:
	//************************************
	// 方法	: iniBeamBlockVec
	// 全名	: BeamSection::iniBeamBlockVec
	// 访问	: public 
	// 返回	: bool
	// 参数	: _beamBlockVec
	// 说明	: 初始化梁断面的block信息，以及梁断面的名�?
	//************************************
	bool iniBeamBlock(const Block&_beamBlockVec);
	//************************************
	// 方法	: retrunBeamSection
	// 全名	: BeamSection::retrunBeamSection
	// 访问	: public 
	// 返回	: BeamSection
	// 说明	: 返回梁断面信�?
	//************************************
	static BeamSection retrunBeamSection(Block &block, Data&data);

private:

	//************************************
	// 方法	: iniBreakPoint
	// 全名	: BeamSection::iniBreakPoint
	// 访问	: private 
	// 返回	: bool
	// 参数	: block  梁断面的�?在块
	// 说明	: 初始化梁断面的纵筋断�?
	//************************************
	bool iniBreakPoint(Block &block, Data&data);
	//************************************
	// 方法	: findBreakPoint
	// 全名	: BeamSection::findBreakPoint
	// 访问	: private 
	// 返回	: bool
	// 参数	: p 断点中心
	// 说明	: 查找是否存在相同的断点，因为纵筋的断点一般由两条重复的多段线组成
	//************************************
	bool findBreakPoint(const Point&p);
	//************************************
	// 方法	: iniBeamDImensions
	// 全名	: BeamSection::iniBeamDImensions
	// 访问	: private 
	// 返回	: bool
	// 参数	: _spDimensions
	// 说明	: 初始化梁断面的标注信�?
	//************************************
	bool iniBeamDImensions(Data &data, const std::vector<std::shared_ptr<Dimension>> &_spDimensions);

	//************************************
	// 方法	: iniStirrupLayer
	// 全名	: BeamSection::iniStirrupLayer
	// 访问	: private 
	// 返回	: bool
	// 参数	: _lineData
	// 说明	: 返回箍筋的图层信�?
	//************************************
	bool iniStirrupLayer(LineData&_lineData);

	//************************************
	// 方法	: bpSurroundBox
	// 全名	: BeamSection::bpSurroundBox
	// 访问	: private 
	// 返回	: Box
	// 参数	: _lineData
	// 说明	: 返回�?个由�?有纵筋形成的包围�?
	//************************************
	Box bpSurroundBox(LineData&_lineData);

	//************************************
	// 方法	: iniStirrupIndex
	// 全名	: BeamSection::iniStirrupIndex
	// 访问	: private 
	// 返回	: bool
	// 参数	: _lineData
	// 说明	: 初始化所有箍筋线索引
	//************************************
	bool iniStirrupIndex(LineData&_lineData);


	//************************************
	// 方法	: iniBeamHook
	// 全名	: BeamSection::iniBeamHook
	// 访问	: private 
	// 返回	: bool
	// 参数	: goalRTree
	// 说明	: 初始化梁断面内所有箍筋的标志�?
	//************************************
	bool iniBeamHook(LineData &lineData, RTreeData &goalRTree);

	//************************************
	// 方法	: iniStriiup
	// 全名	: BeamSection::iniStriiup
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 初始化箍筋信�?
	//************************************
	bool iniStriiup(Data&data);

	//************************************
	// 方法	: setStirrupDirection
	// 全名	: BeamSection::setStirrupDirection
	// 访问	: private 
	// 返回	: bool
	// 说明	: 初始化箍筋方�?
	//************************************
	bool setStirrupDirection(std::vector<int> &_vLines,
		std::vector<int> &_hLines,
		std::vector<int> &_sLines,
		Data&_data);

	//************************************
	// 方法	: divideStirrupDir
	// 全名	: BeamSection::divideStirrupDir
	// 访问	: private 
	// 返回	: bool
	// 参数	: lineIndex �?要区分的箍筋索引

	// 参数	: goalIndex 返回的direction �? N 的索�? 
	// 参数	: ch 区分的类�?
	// 说明	: 进一步区分箍筋的方向�?
	//************************************
	bool divideStirrupDir(const std::vector<int>&lineIndex,
		std::map<int, Direction>&goalIndex,
		const char&ch);


	//************************************
	// 方法	: divideStirLineType
	// 全名	: BeamSection::divideStirLineType
	// 访问	: private 
	// 返回	: bool
	// 参数	: v_lines  箍筋线内竖线的索�?
	// 参数	: h_lines  箍筋线内横线的索�?
	// 参数	: ss_lines 箍筋线内斜线的索�?
	// 参数	: data
	// 说明	: 对已经识别出的箍筋线，再次细化，细化为横线，竖线，斜线，并且对此进行排序合并
	//************************************
	bool divideStirLineType(std::vector<int> &v_lines,
		std::vector<int> &h_lines,
		std::vector<int> &ss_lines,
		Data&data);

	//************************************
	// 方法	: makePairStir
	// 全名	: BeamSection::makePairStir
	// 访问	: private 
	// 返回	: bool
	// 参数	: hStirPair 横向箍筋配对后的返回结果
	// 参数	: vStirPair 纵向箍筋配对后的返回结果
	// 参数	: v_lines 横向箍筋索引
	// 参数	: h_lines 纵向箍筋索引
	// 说明	: 将带有方向的箍筋与，未带方向的箍筋进行配�?
	//************************************
	bool makePairStir(std::vector<std::pair<int, int>>&hStirPair,
		std::vector<std::pair<int, int>>&vStirPair,
		const std::vector<int> &v_lines,
		const std::vector<int> &h_lines,
		Data &data);


	//************************************
	// 方法	: checkStir
	// 全名	: BeamSection::checkStir
	// 访问	: private 
	// 返回	: bool
	// 说明	: 校验箍筋
	//************************************
	bool checkStir(Data&data);
	//************************************
	// 方法	: checkStirPair1
	// 全名	: BeamSection::checkStirPair1
	// 访问	: private 
	// 返回	: BeamSStirrup
	// 参数	: stirDirIndex 表示某箍筋标志对，找到的箍筋索引
	// 参数	: _lineData �?有线条的对象�?
	// 说明	: 对已经配对好的箍筋做补充
	//************************************
	BeamSStirrup checkStirPair1(std::vector<int>stirDirIndex, LineData&_lineData);

	//************************************
	// 方法	: checkStirPair2
	// 全名	: BeamSection::checkStirPair2
	// 访问	: private 
	// 返回	: BeamSStirrup
	// 参数	: 
	// 参数	: _lineData
	// 说明	: 对已经配对，且两个箍筋标志都已配对，做正确�?�校�?
	//************************************
	BeamSStirrup checkStirPair2(std::vector<int>stirDirIndex, Data& data);

	//************************************
	// 方法	: findBeamSitrrupSignal
	// 全名	: BeamSection::findBeamSitrrupSignal
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 将梁断面内的单支箍，用排除的方式从所有箍筋内找到找到
	//************************************
	bool findBeamSitrrupSignal(Data&data);

	//************************************
	// 方法	: findSameStir
	// 全名	: BeamSection::findSameStir
	// 访问	: private 
	// 返回	: bool 返回true表示，匹配到，false 表示为匹配到
	// 参数	: _line �?要匹配的直线
	// 参数	: lineIndex 在匹配到的情况下，返回箍筋的索引
	// 参数	: _lineData
	// 说明	: 为自己计算出的箍筋，匹配对应的箍筋索引，目的用于后续单支箍的判断
	//************************************
	bool findSameStir(const Line &_line, int&lineIndex, LineData & _lineData);

	//************************************
	// 方法	: findBpLeadLine
	// 全名	: BeamSection::findBpLeadLine
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 查找�?有用于描述梁纵筋的引�?
	//************************************
	bool findBpLeadLine(Data&data);

	//************************************
	// 方法	: iniBeamStirLead
	// 全名	: BeamSection::iniBeamStirLeadInfo
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 初始化梁断面的箍筋标注信�?
	//************************************
	bool iniBeamStirLead(Data&data);

	//************************************
	// 方法	: findStirLead
	// 全名	: BeamSection::findStirLead
	// 访问	: private 
	// 返回	: std::vector<int>
	// 参数	: _lineData
	// 说明	: 分离箍筋的引出线
	//************************************
	std::vector<int> divideStirLead(LineData&_lineData);

	//************************************
	// 方法	: iniBeamLeadInfo
	// 全名	: BeamSection::iniBeamLeadInfo
	// 访问	: private 
	// 返回	: bool
	// 参数	: point 文本�?
	// 参数	: refPoint 查找文本的参照点
	// 参数	：_bpLead给定查找的是否为纵筋的文本信息，否则就是箍筋的信�?
	// 参数	: _lineIndex 引出线索�?
	// 参数	: data
	// 说明	: 初始化引出线�?对应的信息，以及引出线对应的文本�?
	//************************************
	bool iniBeamLeadInfo(std::vector<int> & textPointIndex, Point & refPoint, const std::vector<int>& _lineIndex, const bool & _bpLead, Data & data);

	//************************************
	// 方法	: findBeamLeadInfoPoint
	// 全名	: BeamSection::findBeamLeadInfoPoint
	// 访问	: private 
	// 返回	: bool
	// 参数	: point 搜索�?
	// 参数  ：searchR 搜索半径
	// 参数	: _lineIndex
	// 参数	: data
	// 说明	: 在几条引出线中找到合适的文本搜索�?
	//************************************
	bool findBeamLeadInfoPoint(Point&point, double &searchR, const std::vector<int>&_lineIndex, const bool &_bpBox, Data&data);

	//************************************
	// 方法	: iniRebarInfo
	// 全名	: BeamSection::iniRebarInfo
	// 访问	: private 
	// 返回	: bool
	// 参数	: data
	// 说明	: 初始化梁断面箍筋信息
	//************************************
	bool iniRebarInfo(Data & data);

	//************************************
	// 方法	: returnLeadLineDir
	// 全名	: BeamSection::returnLeadLineDir
	// 访问	: private 
	// 返回	: bool
	// 参数	: ch
	// 参数	: std::vector<int>_leadLineIndex
	// 参数	: data
	// 说明	: 判断引出线的方向，用于校验文本与引出线的映射是否正确，经过对比文字一般会与一出线平行
	//************************************
	bool returnLeadLineDir(char &ch, const bool &_bpBox, const std::vector<int>_leadLineIndex, Data&data);

	//************************************
	// 方法	: iniMaxSize
	// 全名	: BeamSection::iniMaxSize
	// 访问	: private 
	// 返回	: bool
	// 说明	: 初始化梁断面的尺�?
	//************************************
	bool iniMaxSize();

	//************************************
	// 方法	: iniReferencePoint
	// 全名	: BeamSection::iniReferencePoint
	// 访问	: private 
	// 返回	: bool
	// 说明	: 设置参照�?
	//************************************
	bool iniReferencePoint(Data&_data);

	//************************************
	// 方法	: setStirInfoType
	// 全名	: BeamSection::setStirInfoType
	// 访问	: private 
	// 返回	: bool
	// 说明	: 设置箍筋标注描述信息�?描述的是否为单只�?
	//************************************
	bool setStirInfoType(Data& data);
	//************************************
	// 方法	: setBPLeadInfoDirection
	// 全名	: BeamSection::setBPLeadInfoDirection
	// 访问	: private 
	// 返回	: bool
	// 说明	: 设置箍筋标注描述信息�?描述的是否为单只�?
	//************************************
	bool setBPLeadInfoDirection(LineData& _lineData);
	//TODO 后续完善梁截面类型，现在默认矩形
	bool bSectionType();


public:
	Block beamBlock;//!梁断面的块信�?

	std::string bpLayer;//!纵筋断点�?在图�?

	std::string stirrupLayer;//!梁断面箍筋所在图�?

	std::vector<Dimension>dimensionsVec;		//!梁断面的标注信息

	std::vector<int>dimLineIndex;//!标注线的索引

	double seale;//!梁断面的比例信息

	std::string sealeStr;//!用于记录断面给出的尺寸信�?

	std::string name;//!梁断面的名字

	std::string sectionType;

public:

	std::vector<int>stirrupIndexVec;//!�?有箍筋的索引

	std::vector<BeamStirrupHook>beamHookVec;//!箍筋的标志，即箍筋标志勾，默认箍筋的画法肯定会绘制标志勾

	std::vector<BeamSStirrup>beamStirrupVec;//!梁箍�?

	std::map<int, Direction>stirrupDirMap;//!箍筋方向 int表示箍筋线条的索引，Direction表示此线条的方向

	std::vector<BeamStirLeadInfo>beamStirInfo;//!梁断面的箍筋信息,指的是描述字符串

	//!单双之箍索引是为了判断箍筋描述信息所描述的是单支箍，还是多支�?
	std::vector<int>singleStirIndex;//!单支箍索�?
	std::vector<int>doubleStirIndex;//!双支箍索�?

	Box stirBox;//由箍筋组成的box


public://分析纵筋信息
	std::vector<BreakPoint>break_pointVec;		//!�?有箍筋的断点信息

	std::vector<int>bpLeadLineVec;//!纵筋断点的所有引出线

	std::vector<BeamBPInfo>beamBPinfoVec;//!纵筋信息

	Box bpBox;//!纵筋的包围盒
public:
	double maxHeight;
	double maxWidth;
	Point referencePoint;//!箍筋的参照点

	//!test leads
	int testLeft;
	int testBottom;
	//std::vector<int>test_hookIndexVec;



private:
	std::map<int, int>_bpIndexMap;//!创建�?有断点的索引字典由于快�?�的排除非箍筋线
	std::map<int, bool>_bStirPair;//!用于判断箍筋是否配对，从而排查出单支�?
	std::map<int, int>stirDirHookIndexMap;//!用于映射箍筋对应的hook，第�?个int表示箍线的索引，?��二个int表示beamHookVec的索�?
	std::map<int, std::vector<int>>_hookStirrupIndexMap;//!用于映射，标志随配对后的箍筋线索引，用于快�?�查�?
};
#endif