#pragma once
#include "common.h"
#include "data.h"
#include "line.h"
#include "dimension.h"
#include "block.h"
#include "elevationSymbolSet.h"
#include <utils\beam\beam.h>

//todo
class LongitudinalBeamOutline
{
public:
	typedef enum
	{
		UNKNOWN,
		UP,
		DOWN
	}SettingType;

	LongitudinalBeamOutline() = default;

	

	int mainIdx;
	SettingType settingType;
	std::vector<int> boardIdxVec;
	RelativeDistance start; //梁轮廓线的起止水平位置
	RelativeDistance end;	//梁轮廓线的起止水平位置
};


class LongitudinalBoard
{
public:
	LongitudinalBoard() = default;

	Box box;				//板的box

	double thickness;		//板的厚度

	int boardTopIdx;		// 板顶线索引
	double ele1;			//板顶标高
	int boardBtmIdx;		//板底线索引
	double ele2;			//板底标高

	std::vector<Line> boardLineVec;//板的对角交叉线
};



class LongitudinalFloor
{
public:
	LongitudinalFloor() :mainIdx(-1)
	{}
	bool checkBoard(const std::shared_ptr<LongitudinalBoard>& board) const;
	void appendBoardIdx(const int boardIdx, const std::shared_ptr<LongitudinalBoard>& board);

	//对外查询的接口,返回对应得梁轮廓线
	std::shared_ptr<LongitudinalBeamOutline> findLongitudinalBeamOutline(std::shared_ptr<Beam> beam);
	

	Box box;							//楼层的box
	int mainIdx;						//该楼层的主板索引
	std::vector<int> boardIdxVec;		//该楼层的所有板索引
	std::vector<int> beamOutlineIdxVec;	//该楼层的梁轮廓线
	std::vector<std::shared_ptr<LongitudinalBeamOutline>> beamOutlines;//所有的轮廓线对象
};

class LongitudinalStructProfile
{
public:

	LongitudinalStructProfile() :
		pIdxVec(nullptr),
		pLineVec(nullptr),
		dynamicThickness(0)
	{};

	//************************************
	// 方法	: GetTheValidBlock
	// 全名	: LongitudinalStructProfile::getTheValidBlock
	// 访问	: public static 
	// 返回	: int
	// 参数	: blocks
	// 说明	: 获取有效的块
	//************************************
	static int getTheValidBlock(const std::vector<std::shared_ptr<Block>>& blocks);


	//************************************
	// 方法	: SetMFCPaintTestVec
	// 全名	: LongitudinalStructProfile::setMFCPaintTestVec
	// 访问	: public 
	// 返回	: void
	// 参数	: temp_idx
	// 参数	: temp_lines
	// 说明	: 设置MFC打印依赖
	//************************************
	void setMFCPaintTestVec(std::vector<int>& temp_idx,
							std::vector<Line>& temp_lines);

	//************************************
	// 方法	: SetAnalysisDependencise
	// 全名	: LongitudinalStructProfile::setAnalysisDependencise
	// 访问	: public 
	// 返回	: void
	// 参数	: prerequiseData
	// 参数	: _elevationSymbolSet
	// 参数	: _elevationSymbols
	// 说明	: 设置解析的依赖对象
	//************************************
	void setAnalysisDependencise(const std::shared_ptr<Data>& prerequiseData,
								 ElevationSymbolSet& _elevationSymbolSet,
								 std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols);


	//************************************
	// 方法	: IniSideVerticalDimVec
	// 全名	: LongitudinalStructProfile::iniSideVerticalDimVec
	// 访问	: public 
	// 返回	: void
	// 参数	: vAxisVec
	// 参数	: dimIdxVec
	// 说明	: 查找两侧的标注，并返回
	//************************************
	void iniSideVerticalDimVec(const std::vector<Axis::AxisLine>& vAxisVec,
							   const  std::vector<unsigned>& dimIdxVec);

	//************************************
	// 方法	: GetSideVerticalIdx
	// 全名	: LongitudinalStructProfile::getSideVerticalIdx
	// 访问	: public 
	// 返回	: std::vector<int>
	// 参数	: block
	// 说明	: 查找纵向剖面的最左右侧的板线索引
	//************************************
	std::vector<int> getSideVerticalIdx(const std::shared_ptr<Block>& block) const;

	//************************************
	// 方法	: FindBoardLineIdx
	// 全名	: LongitudinalStructProfile::findBoardLineIdx
	// 访问	: public 
	// 返回	: bool
	// 说明	: 根据图层搜索所有的板线
	//************************************
	bool findBoardLineIdx();

	//************************************
	// 方法	: FindBoardLayer
	// 全名	: LongitudinalStructProfile::findBoardLayer
	// 访问	: public 
	// 返回	: bool
	// 说明	: 查找到所有的板线的图层
	//************************************
	bool findBoardLayer();

	//************************************
	// 方法	: InitDimLayer
	// 全名	: LongitudinalStructProfile::initDimLayer
	// 访问	: public 
	// 返回	: bool
	// 说明	: 初始化尺寸所在的图层
	//************************************
	bool initDimLayer();



	//************************************
	// 方法	: ParseBoard
	// 全名	: LongitudinalStructProfile::parseBoard
	// 访问	: public 
	// 返回	: bool
	// 说明	: 解析得到所有的板
	//************************************
	bool parseBoard();


	//************************************
	// 方法	: BuildBoard
	// 全名	: LongitudinalStructProfile::buildBoard
	// 访问	: public 
	// 返回	: bool
	// 参数	: condidateIdx
	// 参数	: refLine
	// 参数	: board
	// 说明	: 构建板
	//************************************
	bool buildBoard(const std::vector<int>& condidateIdx,
					const Line& refLine,
					std::shared_ptr<LongitudinalBoard>& board);

	//************************************
	// 方法	: ParseBeamOutline
	// 全名	: LongitudinalStructProfile::parseBeamOutline
	// 访问	: public 
	// 返回	: bool
	// 说明	: 解析梁的轮廓线
	//************************************
	bool parseBeamOutline();

	//************************************
	// 方法	: GetMinHorizontalDist
	// 全名	: LongitudinalStructProfile::getMinHorizontalDist
	// 访问	: public 
	// 返回	: long long
	// 参数	: condidateIdx
	// 参数	: refLine
	// 说明	: 计算候选线中与指定线最近的距离
	//************************************
	long long getMinVerticalDist(const std::vector<int>& condidateIdx,
								 const Line& refLine);

	//************************************
	// 方法	: HorizontalCross
	// 全名	: LongitudinalStructProfile::bHorizontalCross
	// 访问	: public 
	// 返回	: bool
	// 参数	: refLine
	// 参数	: checkLine
	// 说明	: 两平行线是否有平行交错部分
	//************************************
	bool bHorizontalCross(const Line& refLine, const Line& checkLine);

	//************************************
	// 方法	: AdjustDynamicThickness
	// 全名	: LongitudinalStructProfile::adjustDynamicThickness
	// 访问	: public 
	// 返回	: void
	// 参数	: minThickness
	// 说明	: 根据传入的值调整允许的最大板的厚度
	//************************************
	void adjustDynamicThickness(long long minThickness);

	//************************************
	// 方法	: GetBlock
	// 全名	: LongitudinalStructProfile::getBlock
	// 访问	: public 
	// 返回	: bool
	// 参数	: blocks
	// 说明	: 从给块中找到合法的唯一块
	//************************************
	bool getBlock(const std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// 方法	: Parse
	// 全名	: LongitudinalStructProfile::parse
	// 访问	: public 
	// 返回	: bool
	// 参数	: blocks
	// 说明	: 解析纵向剖面的所有数据
	//************************************
	bool parse(const std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// 方法	: GetBlockDimensions
	// 全名	: LongitudinalStructProfile::getBlockDimensions
	// 访问	: public 
	// 返回	: bool
	// 参数	: _dimIdxVec
	// 参数	: box
	// 说明	: 得到块中所有的尺寸标注
	//************************************
	bool getBlockDimensions(std::vector<unsigned int>& _dimIdxVec,
							const Box& box) const;
	//************************************
	// 方法	: BuildDimDesMap
	// 全名	: LongitudinalStructProfile::buildDimDesMap
	// 访问	: public 
	// 返回	: bool
	// 参数	: dimIdxVec
	// 说明	: 建立竖直标注尺寸的映射
	//************************************
	bool buildDimDesMap(const std::vector<unsigned>& dimIdxVec);

	//************************************
	// 方法	: IniDimMap
	// 全名	: LongitudinalStructProfile::iniDimMap
	// 访问	: public 
	// 返回	: bool
	// 说明	: 建立必要的尺寸依赖及映射
	//************************************
	bool iniDimMap();

	//************************************
	// 方法	: FindBeamOutlineLayer
	// 全名	: LongitudinalStructProfile::findBeamOutlineLayer
	// 访问	: public 
	// 返回	: bool
	// 说明	: 查找梁轮廓线的图层
	//************************************
	bool findBeamOutlineLayer();

	// todo replace with non-recursive codes
	bool traverseNonBoardConnectedLineChain(const int idx,
											bool firstLevel,
											std::map<std::string, int>& layerCounterMap,
											std::set<int>& excludedIdxSet);

	//************************************
	// 方法	: FindBeamOutlineIdx
	// 全名	: LongitudinalStructProfile::findBeamOutlineIdx
	// 访问	: public 
	// 返回	: bool
	// 参数	: beamOutlineIdxVec
	// 说明	: 查找所有的梁轮廓线索引
	//************************************
	bool findBeamOutlineIdx(std::vector<int>& beamOutlineIdxVec);

	bool installElevationForBoards();


	double getEle(const int& lineIdx);

	//************************************
	// 方法	: ParseFloor
	// 全名	: LongitudinalStructProfile::parseFloor
	// 访问	: private 
	// 返回	: bool
	// 说明	: 解析得到楼层
	//************************************
	bool parseFloor();

	//************************************
	// 方法	: BuildFloors
	// 全名	: LongitudinalStructProfile::buildFloors
	// 访问	: private 
	// 返回	: bool
	// 参数	: 
	// 参数	: idxChain
	// 说明	: 构建楼层
	//************************************
	bool buildFloors(std::map<int, std::list<int>>& idxChain);

	//************************************
	// 方法	: InitFloor
	// 全名	: LongitudinalStructProfile::initFloor
	// 访问	: private 
	// 返回	: void
	// 参数	: flr
	// 参数	: second
	// 说明	: 初始化楼层
	//************************************
	void initFloor(std::shared_ptr<LongitudinalFloor>& flr, std::list<int>& second);

	//************************************
	// 方法	: AppendBoardIdxVec2Floor
	// 全名	: LongitudinalStructProfile::appendBoardIdxVec2Floor
	// 访问	: private 
	// 返回	: bool
	// 参数	: floor
	// 参数	: condVec
	// 说明	: 添加板索引到楼层
	//************************************
	bool appendBoardIdxVec2Floor(std::shared_ptr<LongitudinalFloor>& floor, const std::list<int>& condVec);

	//************************************
	// 方法	: CheckBoardIdxForFloor
	// 全名	: LongitudinalStructProfile::checkBoardIdxForFloor
	// 访问	: private 
	// 返回	: bool
	// 参数	: condVec
	// 参数	: floor
	// 说明	: 检测候选的板索引是否在该楼层
	//************************************
	bool checkBoardIdxForFloor(const std::list<int>& condVec, std::shared_ptr<LongitudinalFloor>& floor);

	//************************************
	// 方法	: FindBoardMainIdx
	// 全名	: LongitudinalStructProfile::findBoardMainIdx
	// 访问	: private 
	// 返回	: void
	// 参数	: flrItm
	// 说明	: 查找到该楼层的主板索引
	//************************************
	void findBoardMainIdx(std::shared_ptr<LongitudinalFloor>& flrItm);

	//************************************
	// 方法	: AppendBeamOutlineIdxVec2Floor
	// 全名	: LongitudinalStructProfile::appendBeamOutlineIdxVec2Floor
	// 访问	: public 
	// 返回	: void
	// 参数	: beamOutlineIdxItm
	// 说明	: 添加梁轮廓线到相应的楼层中
	//************************************
	void appendBeamOutlineIdxVec2Floor(const  int beamOutlineIdxItm);


	//************************************
	// 方法	: SameFloorCheck
	// 全名	: LongitudinalStructProfile::bSameFloorCheck
	// 访问	: public 
	// 返回	: bool
	// 参数	: pre
	// 参数	: ne
	// 说明	: 判断是否两个板是在同一楼层
	//************************************
	bool bSameFloorCheck(std::shared_ptr<LongitudinalBoard>& pre, std::shared_ptr<LongitudinalBoard>& ne);

	//************************************
	// 方法	: FindConnectedBoardsMap
	// 全名	: LongitudinalStructProfile::findConnectedBoardsMap
	// 访问	: public 
	// 返回	: bool
	// 参数	: 
	// 参数	: idxChain
	// 说明	: 返回前后相接的板映射
	//************************************
	bool findConnectedBoardsMap(std::map<int, std::list<int>>& idxChain);


	//************************************
	// 方法	: InstallRelativeElvationForBeam
	// 全名	: LongitudinalStructProfile::installRelativeElvationForBeam
	// 访问	: public 
	// 返回	: bool
	// 参数	: beam
	// 参数	: beamOutline
	// 说明	: 设置相对标高到每一跨
	//************************************
	bool installRelativeElvationForBeam(std::shared_ptr<Beam> beam,
										std::shared_ptr<LongitudinalBeamOutline> beamOutline);


	std::vector<std::shared_ptr<LongitudinalBoard>> longitudianlBoards;	//所有的纵向楼板
	std::vector<std::shared_ptr<LongitudinalFloor>> longitudinalFloors;	//所有的纵向楼层

private:
	std::shared_ptr<Block> block;	//所在块
	std::shared_ptr<Data> data;		//所在data
	std::vector<int>* pIdxVec;		// 测试线索引
	std::vector<Line>* pLineVec;	// 测试线

	ElevationSymbolSet* elevationSymbolSet;
	std::vector<std::shared_ptr<ElevationSymbol>>* elevationSymbols;
private:

	std::map<long long, std::vector<DimensionPair>> vDimDesMap;			//标注y映射
	std::vector<unsigned> leftDimIdxVec;								//竖轴左侧外的标注索引
	std::vector<unsigned> rightDimIdxVec;								//竖轴右侧外的标注索引
	std::string dimLayer;												//尺寸的图层

	std::vector<int> beamOutlineIdxVec;				//所有的梁轮廓线
	std::vector<int> boardLineIdxVec;				//所有板线索引
	long long dynamicThickness;
	std::string boardLayer;			//板所在图层
	std::string beamOutlineLayer;	//梁轮廓线图层
};