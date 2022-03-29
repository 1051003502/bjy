#ifndef __DRAWINGRELATED_RWJSONFILE_H__
#define __DRAWINGRELATED_RWJSONFILE_H__
#include "json/json.h"
#include "data.h"
#include "axis.h"
#include "border.h"
#include "table.h"
#include "block.h"
#include "beamSet.h"
#include "beamSectionSet.h"
#include "pillarSet.h"
#include "ColumnSectionSet.h"
#include "transform.h"
#include "columnProcessInfo.h"
#include "wallLocation.h"
#include "wallProcessInfo.h"
#include "axisProcessInfo.h"
#if _WIN32
/*#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")*/
#include<io.h>
#else
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


/*读写基本的点数据*/
class RWBaseEntity
{
public:
	//写入点
	Json::Value pointValueW(const Point& point);
	//读入点
	Point pointValueR(const Json::Value& point);
	//写入点的相对坐标
	Json::Value pointValueW(const Point& referencePoint, const Point& point);
	//写入线
	Json::Value lineValueW(const Line& line);
	//写入线结果数据
	Json::Value lineResultValueW(const Line& line);
	//读入线
	Line lineValueR(const Json::Value& line);
	//写入线的相对坐标
	Json::Value lineValueW(const Point& referencePoint, const Line& line);

	//读入线列表
	std::vector<Line>readLineList(const Json::Value& lineVec);



};

/*读入分好类的参数文件*/
class ReadTypeFile
{
public:
	/*读入参数文件*/
	Json::Value readParameterFile(const std::string& parameterFile);
	//解析参数文件
	std::map<std::string,std::vector<std::string>>parserParameterFile(const Json::Value& parameterValue);
	//读出文件的板图纸
	Json::Value readFloorPlan(const Json::Value& parameterFile);
	//读出文件的剖面图纸
	Json::Value readProfileFile(const Json::Value& parameterFile);
	//读入文件的详图图纸
	Json::Value readDetailFile(const Json::Value& parameterFile);
public:

	
};

/*结果文件写入*/
class ResultWrite
{
public:
	//用于生成一个json文件
	bool creatJsonResultFile(const std::string& path, const std::string& fileName, const Json::Value& value);

	//写入轴网数据
	Json::Value axisesValueW(const Axis& _axis);
	//写入柱位置数据
	Json::Value columnLocationW(const std::shared_ptr<Pillar>& _spPillar);
	//写入轴网数据
	Json::Value AxisesW(AxisProcessInfo axises);
	//写入柱截面配筋数据
	Json::Value columnSectionStirrupW();
	//写入梁位置信息
	Json::Value beamLocationW(const std::shared_ptr<Beam>& _spBeamLocVec);
	//初始化梁跨信息
	Json::Value returnBeamSpanVec(const std::vector<std::shared_ptr<BeamSpan>>& spSpanVec);
	//写入墙位置信息
	Json::Value WallLocationW(const std::shared_ptr<WallLocation>& _spWallLoc);
	Json::Value referencePoint(WallReferencePoint referencePoint);


public:
	//柱最终结果写入
	Json::Value columnLocResultData(ColumnLoc columnLoc);

private:
	RWBaseEntity _rwBaseEntity;

};

class ResultRead
{
public:
	Json::Value readResultFile(const std::string& resultFilename);
	Json::Value returnFieldValue(const std::string& path, const std::string& field);
	
public:
	std::vector<ColumnLoc> readCoulmnLocList(const Json::Value& columnLocList);
	std::vector<WallLoc>readWallLocList(const Json::Value& wallLocList);
	std::vector<AxisProcessInfo> readAxisList(const Json::Value& axisList);

private:
	ColumnLoc readColumnLocValue(const Json::Value& columnLocValue);
	WallLoc readWallLocValue(const Json::Value& wallLocVale);
	SingleAxisInfo readAxisProcessInfo(const Json::Value& axisValue);

private:
	RWBaseEntity baseEntity;

};

#if 0
/*读入参数文件*/
Json::Value readParameterFile(const std::string& parameterFile);
//读出文件的板图纸
Json::Value readFloorPlan(const Json::Value& parameterFile);
//读出文件的剖面图纸
Json::Value readProfileFile(const Json::Value& parameterFile);
//读入文件的详图图纸
Json::Value readDetailFile(const Json::Value& parameterFile);

//写入点
Json::Value pointValueW(const Point& point);
//读入点
Point pointValueR(const Json::Value& point);
//写入点的相对坐标
Json::Value pointValueW(const Point& referencePoint, const Point& point);
//写入线
Json::Value lineValueW(const Line& line);
//读入线
Line lineValueR(const Json::Value& line);
//读入线列表
std::vector<Line>readLineList(const Json::Value& lineVec);



//写入线的相对坐标
Json::Value lineValueW(const Point& referencePoint, const Line& line);

//写入数字列表
Json::Value numberListW(const std::vector<int>& numberListW);
//写入圆
Json::Value circleValueW(const Circle& circle);


/************line数据************/
Json::Value lineDataValueW(LineData& lineData);

/***********circle数据***********/
Json::Value circleDataValueW(CirclesData& circleData);

/***********corner数据***********/
//写入corner数据
Json::Value cornerValueW(const Corner& corner);
//写入cornerData数据
Json::Value cornerDataValueW(const CornerData& cornerData);


/***********endPoint数据************/
//写入endPoint数据
Json::Value endPointValueW(const Endpoint& endPoint);
//写入endPoiintData数据
Json::Value endPointDataValueW(EndpointData& endPointData);


/**********写入textPoint数据***************/
//写入textPoint数据
Json::Value textPointValueW(const Point& point);
//写入textPointData数据
Json::Value textPointDataValueW(TextPointData& textPointData);

//写入图层线数据（将不同的图层线索引给出）
Json::Value lineLayerDataValueW(LineLayerData& lineLayerData);


/************写入AxisLine数据****************/
//写入axisLine数据
Json::Value axisLineValueW(const Axis::AxisLine& axisLine);
//写入axisLineData数据
Json::Value axisLineDataValueW(AxisLineData& axisLineData);
//写入轴网数据
Json::Value axisesValueW(const Axis& _axis);


/*************柱结果数据写入*****************/
Json::Value columnLocationW(const std::shared_ptr<Pillar>& _spPillar);
#endif











#endif