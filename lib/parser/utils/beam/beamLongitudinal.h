#pragma once
#include "publicFunction.h"
#include "block.h"
#include "dimension.h"
#include "pillar.h"
#include "beam.h"

class BeamLongitudinalProfile
{
public:
	BeamLongitudinalProfile() = default;
	~BeamLongitudinalProfile() = default;

	//************************************
	// 方法	: FindTextAttribute
	// 全名	: BeamProfile::findTextAttribute
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 说明	: 找到剖线的文本点
	//************************************
	bool findTextAttribute(Data& data);

private:
	//************************************
	// 方法	: FindText
	// 全名	: BeamProfile::findText
	// 访问	: public 
	// 返回	: bool
	// 参数	: _textPtIdxVec
	// 参数	: data
	// 参数	: idx
	// 说明	: 对所给线索引，以线的长度为半径，在线两端分别查找文本点返回
	//************************************
	bool findText(std::vector<int>& _textPtIdxVec, Data& data, const int idx);

public:
	int tagPointIdx{};// 剖面位置所在的文本点索引
	std::pair<int, int> profileLineIdxPair;//剖面位置所在的竖线索引

};

class BeamPosition;


class BeamLongitudinalRein
{
public:
	enum ReinType
	{
		UNKNOWN = 0,
		BOUNDARY,
		INNER
	};



	class ReinCompnent
	{
	public:
		enum class ReinCompnentType
		{
			Unknown,
			Left,
			Longitudinal,
			Right
		};

		ReinCompnent() = default;
		ReinCompnentType compnentType{};						//组件类型
		std::shared_ptr<std::vector<DimensionPair>> dims;	//标注尺寸
		int lineIdx{};										//加固线的索引
		int reinIdx{};										//所在加固的索引
	};

	using ReinCompType = ReinCompnent::ReinCompnentType;


	BeamLongitudinalRein() = default;

	~BeamLongitudinalRein() = default;

	//************************************
	// 方法	: AppendH
	// 全名	: BeamLongitudinalRein::appendH
	// 访问	: private 
	// 返回	: bool
	// 参数	: line
	// 参数	: idx
	// 说明	: 加入横线，及索引，并扩展边界
	//************************************
	bool updateHBound(const Line& line);


	//************************************
	// 方法	: UpdateVBound
	// 全名	: BeamLongitudinalRein::updateVBound
	// 访问	: public 
	// 返回	: bool
	// 参数	: line
	// 说明	: 更新跨的高度边界
	//************************************
	bool updateVBound(const Line& line);

	//************************************
	// 方法	: AddComp
	// 全名	: BeamLongitudinalRein::addComp
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 参数	: comp
	// 说明	: 加入组件
	//************************************
	bool addComp(const std::shared_ptr<Data>& data,
				 std::shared_ptr<BeamLongitudinalRein::ReinCompnent>& comp);

	//************************************
	// 方法	: UpdateBound
	// 全名	: BeamLongitudinalRein::updateBound
	// 访问	: public 
	// 返回	: bool
	// 参数	: targetIdx
	// 说明	: 更新边界
	//************************************
	bool updateHBound(double l, double r);

	//************************************
	// 方法	: GetBoundLength
	// 全名	: BeamLongitudinalRein::getBoundLength
	// 访问	: public 
	// 返回	: double
	// 说明	: 获取边界的跨度
	//************************************
	double longitudinalLength() const { return right - left; }


	//************************************
	// 方法	: SpanHeight
	// 全名	: BeamLongitudinalRein::spanHeight
	// 访问	: public 
	// 返回	: double
	// 说明	: 跨的高度边界
	//************************************
	double spanHeight() const { return top - bottom; }


	//************************************
	// 方法	: FindOuterLineIdx
	// 全名	: BeamLongitudinalRein::findOuterLineIdx
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 参数	: block
	// 说明	: 查找外边线索引
	//************************************
	bool findOuterLineIdx(std::shared_ptr<Data>& data,
						  std::shared_ptr<Block>& block,
						  std::vector<Line>* pTestLineVec);

	//************************************
	// 方法	: ExistOtherComponent
	// 全名	: BeamLongitudinalRein::bExistOtherComponent
	// 访问	: public 
	// 返回	: bool
	// 说明	: 是否存在左右component
	//************************************
	bool bExistOtherComponent() const {
		for (const auto& com : this->components)
		{
			if (com->compnentType == ReinCompType::Left ||
				com->compnentType == ReinCompType::Right)
				return  true;
		}
		return false;
	}


public:
	std::vector<std::shared_ptr<ReinCompnent>> components;	//加固的组件
	int outerIdx{};											// 外包围线索引（存在左右component时）
	BeamPosZDelta leftZDelta{};								//若是外加固，则存在可能存在Z方向的数据
	BeamPosZDelta rightZDelta{};
	double left{};								//水平加固左边界
	double right{};								//水平加固右边界
	double top{};								//加固的上边界
	double bottom{};							//加固的下边界
	int next{};									//下一个链接加固
	int pre{};									//上一个链接加固
	enum ReinType reinType {};					//加固的类型
	std::vector<std::pair<DimensionPair, double>> truncatedLenDimDes;//截断标注
};



class BeamLongitudinal
{
public:
	BeamLongitudinal() = default;
	~BeamLongitudinal() = default;

public:


	//************************************
	// 方法	: ReturnBeamLongitudinal
	// 全名	: BeamLongitudinal::returnBeamLongitudinal
	// 访问	: public static 
	// 返回	: bool
	// 参数	: _block
	// 参数	: _strReinLayer
	// 参数	: data
	// 参数	: pTestVec
	// 说明	: 解析包含框架梁信息的块
	//************************************
	bool parseBlock(std::shared_ptr<Block> _spBlock,
					std::string& _strReinLayer,
					std::shared_ptr<Data> data,
					std::vector<unsigned int>& _blockDimIdxVec,
					std::vector<int>& pTestVec,
					std::vector<Line>& pTestLineVec);


	//************************************
	// 方法	: InitReqiuredInfo
	// 全名	: BeamLongitudinal::initReqiuredInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: _block
	// 参数	: _strReinLayer
	// 参数	: pTestVec
	// 参数	: pTestLinesVec
	// 说明	: 初始化分析的需求数据
	//************************************
	bool initReqiuredInfo(std::shared_ptr<Block> _block,
						  std::string& _strReinLayer,
						  std::vector<int>& pTestVec,
						  std::vector<Line>& pTestLinesVec);


	//************************************
	// 方法	: InitDimMap
	// 全名	: BeamLongitudinal::initDimMap
	// 访问	: public 
	// 返回	: bool
	// 参数	: _blockDimensionVec
	// 说明	: 初始化块里面的标注信息映射
	//************************************
	void initDimMap(std::vector<unsigned int>& _blockDimIdxVec,
					std::shared_ptr<Data> data);



	//************************************
	// 方法	: ParseRein
	// 全名	: BeamLongitudinal::parseRein
	// 访问	: public 
	// 返回	: bool
	// 参数	: _hLineIdxVec
	// 参数	: _vLineIdxVec
	// 参数	: data
	// 说明	: 找到所有的加固线
	//************************************
	bool findRein(std::vector<int>& _hLineIdxVec,
				  const std::shared_ptr<Data>& data);

	//************************************
	// 方法	: ParseRein
	// 全名	: BeamLongitudinal::parseRein
	// 访问	: public 
	// 返回	: bool
	// 参数	: hBound
	// 参数	: vBound
	// 参数	: data
	// 说明	: 解析加固线
	//************************************
	bool parseRein(std::vector<int>& _hLineIdxVec,
				   std::shared_ptr<Data>& data);

	//************************************
	// 方法	: ParseZData
	// 全名	: BeamLongitudinal::parseZData
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 参数	: rein
	// 说明	: 获取Z轴数据
	//************************************
	bool parseZData(const std::shared_ptr<Data>& data,
					std::shared_ptr<BeamLongitudinalRein>& rein);



	//************************************
	// 方法	: ReturnSRtreeDimIndex
	// 全名	: BeamLongitudinal::returnSRtreeDimIndex
	// 访问	: public 
	// 返回	: std::vector<int>
	// 参数	: min
	// 参数	: max
	// 说明	: 返回搜索到的斜的标注
	//************************************
	std::vector<int> returnSRtreeDimIndex(const double min[], const double max[]);

	//************************************
	// 方法	: FindHDimension
	// 全名	: BeamLongitudinal::findHDimension
	// 访问	: public 
	// 返回	: bool
	// 参数	: _spDimension
	// 参数	: tick
	// 参数	: incrementDirection
	// 参数	: data
	// 说明	: 以高度差和方向搜索尺寸标注
	//************************************
	bool findHVDimension(std::vector<DimensionPair>& _dimPairVec,
						 const Point& tick,
						 bool bIncrement,
						 bool bHorizontal,
						 const std::shared_ptr<Data>& data);


	//************************************
	// 方法	: FindTruncatedDimPair
	// 全名	: BeamLongitudinal::findTruncatedDimPair
	// 访问	: public 
	// 返回	: bool
	// 参数	: rein
	// 参数	: dimPairVec
	// 参数	: accessPtF
	// 参数	: accessCheckPtF
	// 参数	: data
	// 说明	: 分析标注加入到加固
	//************************************
	bool findTruncatedDimPair(std::shared_ptr<BeamLongitudinalRein> rein,
							  std::vector<DimensionPair>& dimPairVec,
							  const std::shared_ptr<Data>& data);



	//************************************
	// 方法	: FindProfile
	// 全名	: BeamLongitudinal::findProfile
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 说明	: 查找所有的剖线
	//************************************
	bool findProfile(Data& data);



	//************************************
	// 方法	: InitFirstSupportRef
	// 全名	: BeamLongitudinal::initFirstSupportRef
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 说明	: 初始化靠近第一个柱支撑的参考标注
	//************************************
	bool initFirstSupportRef(std::shared_ptr<Data> data);

	//************************************
	// 方法	: FindFrameInfo
	// 全名	: BeamLongitudinal::findFrameInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: spBeam
	// 参数	: data
	// 参数	: _pillarVec
	// 说明	: 查找梁框架信息,安装到spBeam中
	//************************************
	bool installFrameInfo(std::shared_ptr<Beam> spBeam,
						  std::shared_ptr<Data> data,
						  std::vector<std::shared_ptr<Pillar>>& _pillarVec);

	//************************************
	// 方法	: InstallZDeltaInfoo
	// 全名	: BeamLongitudinal::installZDeltaInfoo
	// 访问	: public 
	// 返回	: void
	// 参数	: reinItm
	// 参数	: data
	// 参数	: col1
	// 参数	: col2
	// 参数	: col1RightInFrame
	// 参数	: col2LeftInFrame
	// 参数	: spanItm
	// 说明	: 查找上下翻数据，安装到spBeam中各跨中
	//************************************
	void installZDeltaInfoo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
							std::shared_ptr<Data> data,
							const std::shared_ptr<Pillar>& col1,
							const std::shared_ptr<Pillar>& col2,
							double col1RightInFrame,
							double col2LeftInFrame,
							std::shared_ptr<BeamSpan>& spanItm);


	//************************************
	// 方法	: InstallAnchorageInfo
	// 全名	: BeamLongitudinal::installAnchorageInfo
	// 访问	: public 
	// 返回	: void
	// 参数	: reinItm
	// 参数	: data
	// 参数	: col1RightInFrame
	// 参数	: col2LeftInFrame
	// 参数	: col1
	// 参数	: col2
	// 参数	: spanItm
	// 说明	: 安装锚固长度
	//************************************
	void installAnchorageInfo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
							  std::shared_ptr<Data> data,
							  double col1RightInFrame,
							  double col2LeftInFrame,
							  const std::shared_ptr<Pillar>& col1,
							  const std::shared_ptr<Pillar>& col2,
							  std::shared_ptr<BeamSpan>& spanItm);


	//************************************
	// 方法	: GetDimInfo
	// 全名	: BeamLongitudinal::getDimInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: dim
	// 说明	: 获取标注的信息
	//************************************
	std::string getDimInfo(const std::shared_ptr<Dimension>& dim);

	//************************************
	// 方法	: InstallProfileInfo
	// 全名	: BeamLongitudinal::installProfileInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: data
	// 参数	: col1RightInFrame
	// 参数	: col2RightInFrame
	// 参数	: spanItm
	// 说明	: 为指定的跨和范围搜索剖线，并安装
	//************************************
	bool installProfileInfo(std::shared_ptr<Data> data,
							double col1RightInFrame,
							double col2RightInFrame,
							std::shared_ptr<BeamSpan>& spanItm);



	//************************************
	// 方法	: InstallTruncatedInfo
	// 全名	: BeamLongitudinal::installTruncatedInfo
	// 说明	: 安装截断长度到跨
	//************************************
	void installTruncatedInfo(const std::shared_ptr<BeamLongitudinalRein>& rein,
							  const std::shared_ptr<Data>& data,
							  const double col1RightInFrame,
							  const double col2LeftInFrame,
							  std::shared_ptr<BeamSpan>& spanItm);



	std::shared_ptr<Block> spBlock;   //当前的块
	// ! <伪相关,<尺寸标注,标注是否增量方向>>
	std::map<long long, std::vector<DimensionPair>> hDimDesMap;	//标注x映射
	std::map<long long, std::vector<DimensionPair>> vDimDesMap;	//标注y映射
	std::map<long long, std::vector<unsigned int>> lenDimDesMap;//标注长度映射

	RTreeData sDimRTree;//斜标注Rtree
	std::vector<int> reinLineIdxVec;			//所有的加固线的索引
	std::map<int, int> reinLineIdxToReinIdxMap;	//每个加固线对应的加固索引
	std::string strReinLayer;					//箍筋图层
	std::string strTagName;						//辨识标记名称
	std::shared_ptr<Dimension> supportDim;		//最左边贴近柱的右边界的标注 

	std::vector<std::shared_ptr<BeamLongitudinalProfile>> longitudinalProfileVec;	//所有的剖面
	std::vector<std::shared_ptr<BeamLongitudinalRein>> spReinVec;					//所有加固

	//测试
	std::vector<int>* pTestIdxVec{};
	std::vector<Line>* pTestLineVec{};
};
