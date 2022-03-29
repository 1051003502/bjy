#ifndef __DRAWINGRELATED_DRAWINGDATA_H__
#define __DRAWINGRELATED_DRAWINGDATA_H__
#include "data.h"
#include "parser.h"
#include "transer.h"
#include "pillarSet.h"
#include "ColumnSectionSet.h"
#include "beamSet.h"
#include "beamLongitudinalSet.h"
#include "beamSectionSet.h"
#include "RWJsonFile.h"
#include "columnInfo.h"
#if _WIN32
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")
#else

#endif

class IniDrawingType
{

public:
	typedef enum
	{
		/*平面图*/
		PLAN,//平面图
		MSP,//main structure plan 主体结构平面图
		LBSP,//Longitudinal beam stirrup plan 纵梁配筋平面图
		BSP,//board stirrup plan 板配筋平面图
		WSP,//wall stirrup plan 墙配筋平面图

		/*剖面图*/
		MSPP,//主体结构剖面图
		MSSP,//main struct stirrup profile
		MSHP,//main stirrup profile 主体结构横剖
		MSVP,//main structure longitudinal profile 主体结构纵剖 
		MSSHP,//main structure stirrup profile 主体结构配筋横剖
		MSSVP,//main structure stirrup profile 主体结构配筋纵剖
		PROV,//纵剖不知配筋还是结构
		PROH,//横剖布置配筋还是结构
		PRO,//剖面图profile
		/*详图*/
		DETAIL,//表示详图 详图按照具体的块细分每个块所表达的内容
		CSSD,//表示柱截面配筋详图column section stirrup detail
		BSD,//beam stirrup detail 梁详图
		WSD,//wall stirrup detail 墙详图
		CWSD,//column beam stirrup detail 主梁详图
		CBSSD,//柱梁截面
		UNKONW
	}DrawingType;

public:
	bool parserParameter(const std::string& parameter);
	bool setDaringType(const IniDrawingType::DrawingType& type);
	const std::vector<std::string>& getDirector();
	const IniDrawingType::DrawingType& returnDrawingType();
	const int& directorSize();
	std::string getDrawingAddress();
	std::string getDrawingName();

private:
	std::vector<int>findCharLocNum(const std::string& str, const char& ch);//查找字符串中某字符出现的个数以及位置
	bool setAddress(const std::string& str, const int& loc);//初始化图纸地址
	bool setFileName();//初始化图纸名字
	bool setDrawingDirectory(const std::string& str, const std::vector<int>& chLocVec);
	bool setDrawingType();//初始化图纸类型


private:

	//std::string _drawingParameter;//图纸参数
	std::vector<std::string>_directory;//图纸目录 有时候目录会有多层，最多不会超过三层
	std::string _address;//图纸的地址
	std::string _fileName;
	DrawingType _drawingType;


};

class Drawing
{
public:

	typedef enum
	{
		COLUMNLOC,//柱位置
		BEAMLOC,//梁位置
		WALLLOC,//墙位置
		FLOORLOC,//板位置
		ELEVATION,//标高
		BEAMFLAT,//梁平法
		AXISES,//轴网
		SCD,//section column detail 柱详图
		SBD,//section beam detail 梁详图
		UNKNOW
	}ResultValue;

public:
	//对外接口
	Drawing()
	{}
	Drawing(const std::shared_ptr<Transer>& _spDraTranser,
			const std::shared_ptr<Parser>& _spDarParser,
			const std::string& _drawingParameter);

	//************************************
	// 说明	: 按照已划分柱，梁，墙，板分别处理其信息
	//************************************
	bool parserDrawing(/*const Drawing::DrawingType& drawingType*/);
	//************************************
	// 说明	: 返回当前图纸过程文件ID
	//************************************
	//std::string returnFileID();

	//************************************
	// 说明	: 给出文件包含内容的索引
	//************************************


private:
	//************************************
	// 说明	: 按照已划分柱，梁，墙，板分别处理其信息
	//************************************
	//bool setDrawingType(const Drawing::DrawingType& drawingType);
	//************************************
	// 说明	: 生成当前图纸结果文件ID
	//************************************
	//bool setFileID();
	//************************************
	// 说明	: 完善索引json文件
	//************************************
	bool setClueFile();
	Json::Value readClueFile(const std::string& fileName);
	bool writeClueFile(const Json::Value& rootValue, const std::string& fileName);
	Json::Value writeClueAxis(const Json::Value& rootValue);
	Json::Value writeClueColumn(const Json::Value& rootValue);
	Json::Value writeClueBeam(const Json::Value& rootValue);
	Json::Value writeClueWall(const Json::Value& rootValue);
	Json::Value writeClueFloor(const Json::Value& rootValue);
	//************************************
	// 说明	: 添加图纸识别出的信息枚举
	//************************************
	bool setResultMap(const ResultValue& valueType, const std::string& fieldName);

private:
	/*将解析过程信息，以及结果信息写入文件*/
	//************************************
	// 说明	: 写入过程信息
	//************************************
	bool writeParserProcessInfo();
	//************************************
	// 说明	: 写入结果信息
	//************************************
	bool writeParserResultInfo();
	//************************************
	// 说明	: 拆分传入参数，分离出目录与地址，并根据参数区分图纸类型进行解析
	//************************************
	//bool separationParameter();
	//************************************
	// 说明	: 初始化轴网
	//************************************
	bool setAxis();
	//************************************
	// 说明	: 文件创建目录
	//************************************
	bool creatFileDirectory(const std::vector<std::string>& directorVec);
	//************************************
	// 说明	: 初始化图纸类型
	//************************************
	//bool setDrawingType();//最初初始化
	//bool chengeDrawingType();//根据具体情况变化图纸类型

	//************************************
	// 说明	: 根据图纸类型具体分析图纸内数据
	//************************************
	bool specificParserDrawing();
private:
	//具体解析某类图纸

	//************************************
	// 说明	: 板的主体结构平面图
	//************************************
	bool parserSMP();
	//************************************
	// 说明	: 柱截面详图
	//************************************
	bool parserCSSD();
	//************************************
	// 说明	: 解析横剖配筋图
	//************************************
	bool parserMSSHP();

private:

	/*柱位置*/
	bool parserColumnLoc();
	/*梁位置*/
	bool parserBeamLoc();
	/*墙位置*/
	bool parserWallLoc();
	/*板位置*/
	bool parserFloorLoc();
	/*柱截面详图*/
	bool parserColumnSection();

private:
	//将解析图纸后获得的数据写入对应的文件

	//************************************
	// 说明	: 写入主体结构平面图的结果数据
	//************************************
	bool writeSMP(Json::Value& resuleData);
	// 说名 : 写入柱截面详图结果数据
	bool writeCSSD(Json::Value& resuleData);
private:
	//对应Json数据

	/*轴网*/
	Json::Value axisesLoc();
	/*柱位置*/
	Json::Value columnLoc();
	/*梁位置*/
	Json::Value beamLoc();
	/*墙位置*/
	Json::Value wallLoc();
	/*板位置*/
	Json::Value floorLoc();

private:
	/*json reader writer*/
	ResultWrite _resultWrite;



private:

	std::shared_ptr<Transer> _spDraTranser;//表示dra 表示图纸drawing 存储drawing的transer数据
	std::shared_ptr<Parser> _spDarParser;//存储drawing的parser数据
	std::string _drawingParameter;//图纸参数
	IniDrawingType _iniDrawingType;//图纸的类型目录

	std::map<ResultValue, std::string>_resultMap;//将识别出的结果信息枚举写成字典的形式，用于写入索引文件
private:
	//!Axis
	std::vector<std::shared_ptr<Axis>> _axises;//轴网

/*柱对象数据*/
	PillarSet _columnLocSet;//column location分析柱柱位置信息
	std::vector<std::shared_ptr<Pillar>> _spColumnVec;//所有的柱位置信息

	ColumnSectionSet _columnSectionSet;//分析柱截面信息
	std::vector<std::shared_ptr<ColumnSection>>_spColumnSectionVec;//柱截面结果信息

/*梁对象数据*/
	BeamSet _beamLocSet;//梁位置信息
	BeamLongitudinalSet _beamLongitudinalSet;//梁纵向钢筋描述
	BeamSectionSet _beamSectionSet;//梁截面分析
	std::vector<std::shared_ptr<Beam>> _spBeamLocVec;//所有梁位置信息

/*墙对象数据*/
	WallLocationSet _wallLocSet;//墙位置
	std::vector<std::shared_ptr<WallLocation>>_spWallLocVec;//所有的墙


/*板对象数据*/

/*对每张图纸生成对应*/

};


#endif