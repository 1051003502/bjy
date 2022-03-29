#ifndef __DRAWINGRELATED_DATAINTEGRATION_H__
#define __DRAWINGRELATED_DATAINTEGRATION_H__
#include "coordinateSystem.h"
#include "columnInfo.h"
#include "beamInfo.h"
#include "spdlog/spdlog.h"
#include "drawing.h"
#include "wallInfo.h"

class DrawingData
{
public:
	DrawingData() {}
	DrawingData(const std::shared_ptr<Transer>& _spDraTranser,
				const std::shared_ptr<Parser>& _spDarParser);


public:
	std::shared_ptr<Transer> spDraTranser;//表示dra 表示图纸drawing 存储drawing的transer数据
	std::shared_ptr<Parser> spDarParser;//存储drawing的parser数据


};

//图纸文件数据，多张图纸已map存储
using FileDataMap = std::map<std::string, DrawingData>;

//数据整合
class DataIntegration
{

	//新结构
#if 1
/**********function interface*************/
public:
	// 说明	: 详细解析某类型的图纸，并将解析后的数据写入文件
	bool parserDrawingWriteFile();
	// 说明	: 初始化图纸数据_file
	bool iniFileDataMap(const std::string& _fileName,
		const shared_ptr<Transer>& _spTranser,
		const shared_ptr<Parser>& _spParser);
	// 说明	: 按照已划分柱，梁，墙，板分别处理其信息
	Drawing returnDrawing(const std::shared_ptr<Transer>& _spDraTranser,
		const std::shared_ptr<Parser>& _spDarParser,
		const std::string& drawingParameter);
	// 说明	: 返回当前图纸名称的图纸数据
	DrawingData	returnFindFIle(const std::string& _fileName);
	// 说明	: 返回传入的图纸数量
	int filesSize();
	// 说明	: 清除某图纸
	bool filesErase(const std::string& file);
	FileDataMap::iterator filesBegin();
	FileDataMap::iterator filesEnd();
	//根据基础数据解析在此解析图纸开启多线程
	bool drawingParser(const std::string path, const DrawingData& fileData);
/**************private function******************/
//private:
public:
	//说明：读入索引文件用于整合数据
	bool processClueFile();
	bool initegrationAxis(const Json::Value& axisData);
	bool integrationColumn(const Json::Value& columnData);
	bool integrationBeam(const Json::Value& beamData);
	bool integrationWall(const Json::Value&wallData);
	bool integrationFloor(const Json::Value& floorData);
private:
	FileDataMap _fileData;//图纸信息
	CoordinateSystem _coordinateSystem;//轴网坐标系
	ColumnInfo _columnInfo;//整理柱信息
	WallInfo _wallInfo;//整理墙信息
	ResultRead _clueFileReadIn;//读入索引文件
	std::vector<std::shared_ptr<thread>>_drawingThread;//由多个图纸开启多个线程

#endif



#if 1
private://!柱信息
	/*整理后的数据*/
	//ColumnInfo _columnInfo;//!所有柱信息
public://!梁信息
	BeamInfo _beamInfo;//!所有关于梁的信息
	std::map<std::string, std::vector<std::shared_ptr<BeamSection>>>_BeamSectionMap;//各图纸所对应的柱截面
	std::map<std::string, std::vector<std::shared_ptr<Beam>>>_AaBeams;//平法直接获取柱信息
#endif

};
#endif
