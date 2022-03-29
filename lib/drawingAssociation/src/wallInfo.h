#ifndef __DRAWINGRELATED_WALLINFO_H__
#define __DRAWINGRELATED_WALLINFO_H__
#include "RWJsonFile.h"
#include "wallProcessInfo.h"
class WallInfo
{
public:
	//interface funcation
	bool findWallPathJsonValue(const Json::Value& wallJVPath);
	bool readWallLocInfor();


private:
	const std::string setFloorPath(const std::string& type, const std::string& field);//设置读取数据数据的路径
	std::vector<WallLoc> setWallLocJV(const Json::Value& wallLocValue);//将柱位置Json数据解析为内存数据

private:
	Json::Value _wallLocJV;//墙位置
	Json::Value _wallStirrupJV;//墙配筋
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//第一个string表示某层，第二个表示某层的某张图纸，第三个表示某个字段
	std::map < std::string, std::vector<std::pair<std::string, std::vector<WallLoc>>>>_drawingWallLocVec;//第一个string表示某层，第二个表示某图纸，vec表示柱列表
	ResultWrite resultWrite;//调用写入结果函数的对象
	ResultRead resultRead;//调用读入过程结果函数的对象
	std::vector<std::string>_floorVec;//
};
#endif