#ifndef __DRAWINGRELATED_BEAMINFO_H__
#define __DRAWINGRELATED_BEAMINFO_H__
#include "beamSectionSet.h"
#include "beamSet.h"
#include "pillarSet.h"
#include "columnInfo.h"
#include "beamLongitudinalSet.h"

class BeamSectionInfo
{
public:
	BeamSectionInfo() {}

	//!分别用来描述梁的上下左右的箍筋类型
	typedef enum _LT//longitudinal tendons 纵筋的缩写
	{
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	}LT;
public:
	double _seale;//!梁断面比例信息
	double _width, _height;//!宽高
	std::vector<std::string> _singleStirrupInfoVec;//!单只箍
	std::vector<std::string> _doubleStirrupInfoVec;//!双支箍
	std::vector<std::vector<Line>>_singleStirrupVec;
	std::vector<std::vector<Line>>_doubleStirrupVec;
	//std::vector<std::pair<_LT, std::vector<std::string>>>_lonTen;//描述纵筋的类型
	std::map<LT, std::vector<std::string>>_lonTen;//描述纵筋的类型
	Point referencePoint;//!箍筋的参考点
	int _hsize, _vSize;//!横，纵支数
	std::string name;
	std::string style;//截面风格，矩形还是异形

};

class BeamInfo
{
public:
	//************************************
	// 方法	: iniBeamLoc
	// 全名	: BeamInfo::iniBeamLoc
	// 访问	: public 
	// 返回	: bool
	// 参数	: std::map<std::string
	// 参数	: _axisMap  梁的平面图
	// 参数	: _dataMap
	// 说明	: 初始化梁的位置信息
	//************************************
	bool iniAbBeamLoc(const std::map<std::string, std::shared_ptr<Axis>>& _axisMap,
					  std::map<std::string, std::shared_ptr<Data>>& _dataMap);

	//************************************
	// 方法	: iniBAssemblyAnnotation
	// 全名	: BeamInfo::iniBAssemblyAnnotation
	// 访问	: public 
	// 返回	: bool
	// 参数	: _bAA
	// 说明	: 初始化此套图纸梁是否为平法表示
	//************************************
	bool iniBAssemblyAnnotation(const bool& _bAA = false);



	//************************************
	// 方法	: iniAaBeam
	// 全名	: BeamInfo::iniAaBeam
	// 访问	: public 
	// 返回	: bool
	// 参数	: _BeamSectionMap
	// 说明	: 平法形式初始化梁信息
	//************************************
	bool iniAaBeam(const std::map<std::string, std::vector<std::shared_ptr<Beam>>>& _BeamMap);

	//************************************
	// 方法	: findBeamName
	// 全名	: BeamInfo::findBeamName
	// 访问	: public 
	// 返回	: bool
	// 说明	: 初始化不同图纸内的梁
	//************************************
	bool findBeamName();

	//************************************
	// 方法	: bBeamSectionBlock
	// 全名	: BeamInfo::bBeamSectionBlock
	// 访问	: public 
	// 返回	: bool
	// 参数	: _block
	// 说明	: 判断当前块是梁断面还是梁纵筋图
	//************************************
	bool bBeamSectionBlock(const std::shared_ptr<Block>& _block);

	//************************************
	// 方法	: findFrameBeamBlock
	// 全名	: BeamInfo::findFrameBeamBlock
	// 访问	: public 
	// 返回	: bool
	// 参数	: _dataMap
	// 参数	: _blocks
	// 参数	: _blocksMap
	// 参数	: _blockFileMap
	// 说明	: 查找相关框架梁
	//************************************
	bool findFrameBeamBlock(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							const std::vector<std::shared_ptr<Block>>& _blocks,
							const std::map<std::string, int>& _blocksMap,
							const std::map<std::string, std::vector<int>>& _blockFileMap);

	//!对梁纵筋进行解析
	bool parserFrameBeam(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
						 const std::vector<std::shared_ptr<Block>>& _blocks,
						 const std::map<std::string, int>& _blocksMap,
						 const std::map<std::string, std::vector<int>>& _blockFileMap);

	

	//************************************
	// 方法	: integrateBeamSectionName
	// 全名	: BeamInfo::integrateBeamSectionName
	// 访问	: public 
	// 返回	: bool
	// 说明	: 每条梁会有多个截面，那么多条梁之间他们的截面有可能相同有可能不同，因此归纳找出所有不同的截面名做后面的匹配截面匹配
	bool integrateBeamSectionName();

	//************************************
	// 方法	: findBeamSpanSection
	// 全名	: BeamInfo::findBeamSpanSection
	// 访问	: public 
	// 返回	: bool
	// 参数	: _dataMap
	// 参数	: _blocks
	// 参数	: _blocksMap
	// 参数	: _blockFileMap
	// 说明	: 查找每跨的相关截面
	bool findBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							 const std::vector<std::shared_ptr<Block>>& _blocks,
							 const std::map<std::string, int>& _blocksMap,
							 const std::map<std::string, std::vector<int>>& _blockFileMap);
	//************************************
	// 方法	: iniBeamSpanSection
	// 全名	: BeamInfo::iniBeamSpanSection
	// 访问	: public 
	// 返回	: bool
	// 说明	: 对梁跨截面进行解析
	//************************************
	bool iniBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							const std::vector<std::shared_ptr<Block>>& _blocks, const std::map<std::string, int>& _blocksMap,
							const std::map<std::string, std::vector<int>>& _blockFileMap);

	//************************************
	// 方法	: supplementaryBeamSectionInfo
	// 全名	: BeamInfo::supplementaryBeamSectionInfo
	// 访问	: public 
	// 返回	: bool
	// 说明	: 将已经解析好的梁截面信息，补充进梁的截面信息内
	bool supplementaryBeamSectionInfo();
	
	//************************************
	// 方法	: returnBeamSectionInfo
	// 全名	: BeamInfo::returnBeamSectionInfo
	// 访问	: public 
	// 返回	: bool
	// 参数	: _beamSectionInfo 需要整理截面信息
	// 说明	: 将解析过的梁截面信息进行整理，然后格式化输出
	BeamSectionInfo returnBeamSectionInfo(const std::shared_ptr<BeamSection>& _beamSectionInfo);
	//************************************
	// 方法	: outputBeamSection
	// 全名	: BeamInfo::outputBeamSection
	// 访问	: public 
	// 返回	: bool
	// 说明	: 将梁截面信息输出为json文件
	Json::Value outputBeamSection(const std::shared_ptr<BeamSection>& _beamSection);

	//************************************
	// 方法	: outputBeams
	// 全名	: BeamInfo::outputBeams
	// 访问	: public 
	// 返回	: bool
	// 说明	: 以Json格式文件输出
	//************************************
	bool outputBeams();

	//************************************
	// 方法	: ClearBeamVec
	// 全名	: BeamInfo::clearBeamVec
	// 访问	: public 
	// 返回	: bool
	// 说明	: 清空BeamVec
	//************************************
	bool clearBeamVec();

	//************************************
	// 方法	: SerializeAaBeamInfo
	// 全名	: BeamInfo::serializeAaBeamInfo
	// 访问	: public 
	// 返回	: bool
	// 说明	: 输出平法梁信息
	//************************************
	bool serializeAaBeamInfo();

	//************************************
	// 方法	: serializeElevationProfileBeamInfo
	// 全名	: BeamInfo::serializeElevationProfileBeamInfo
	// 访问	: public 
	// 返回	: bool
	// 说明	: 输出立剖面梁的信息
	//************************************
	bool serializeElevationProfileBeamInfo();

	/*测试程序，用于测试梁截面信息整合*/
	bool testBeamSectionParser(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							   const std::vector<std::shared_ptr<Block>>& _blocks, const std::map<std::string, int>& _blocksMap,
							   const std::map<std::string, std::vector<int>>& _blockFileMap);

public:
	bool bAssemblyAnnotation;//判断梁是否为平法标注
	std::map<std::string,std::vector<std::shared_ptr<Beam>>> spBeamVec;
	ColumnInfo _columnInfo;//!所有柱信息
private:
	BeamSectionSet _beamSectionSet;                                               //!立剖面时会使用
	BeamLongitudinalSet _beamLongitudinalSet;
	std::map<std::string, std::shared_ptr<BeamSection>> _beamSectionMap;            //梁截面名与截面信息的映射
	std::map<std::string, std::vector<std::shared_ptr<Beam>>> _drawing2BeamVecMap; //各图纸所对应的梁位置
	std::vector<std::string> _beamNameVec;                                         //梁名
	std::map<std::string, std::vector<std::string>> _beamNameVecMap;               //记录不同图内的名称
	std::vector<std::shared_ptr<BeamSection>> _spBeamSectionVec;                   //查找梁截面

	std::vector<std::pair<std::shared_ptr<Block>, std::shared_ptr<Data>>> _BeamFrameBlockVec;   //梁的通长筋块信息
	std::vector<std::pair<std::shared_ptr<Block>, std::shared_ptr<Data>>> _beamSectionBlockVec;//梁截面块信息
	std::vector<std::string> _beamSectionNameVec;                                //梁截面名字
	std::map<std::string, std::map<std::string, std::vector<std::string>>>_beamSpanSectionInfoMap;//某图纸中，某梁的截面

public:
	//测试
	std::vector<int> temp_index;
	std::vector<Line> temp_line;
	std::vector<Point> _corners;
	std::vector<std::vector<int>> m_temp_index;
};
#endif
