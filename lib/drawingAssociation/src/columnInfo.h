#ifndef __DRAWINGRELATED_COLUMNINFO_H__
#define __DRAWINGRELATED_COLUMNINFO_H__
#include "json/json.h"
//#include "pillarSet.h"
//#include "ColumnSectionSet.h"
//#include "columnProcessInfo.h"
#include "RWJsonFile.h"



/*!
* \class columnLocationW
*
* \brief 此类用于集中整理柱的位置信息
*
* \author Xiong
* \date 15/2/2020
*/
class columnLocationW
{
public:
	columnLocationW(const std::string& name,
				   const PillarDistance& hDistance,
				   const PillarDistance& vDistance, unsigned int Id = 0);
	friend bool operator==(const columnLocationW& _column1, const columnLocationW& _column2);
public:
	unsigned int columnSectionId;
	std::string name;//柱名称
	PillarDistance hDistance;//距离最近的横轴，PillarDistance::second.first 为下横轴 PillarDistance::second.second上横轴
	PillarDistance vDistance;//距离最近的左纵轴，PillarDistance::second.first 为下横轴 PillarDistance::second.second右纵轴



};
/*!
* \class ColumnSection
*
* \brief
*
* \author Xiong
* \date 15/2/2020
*/
class ColumnSectionInfo
{
	struct SideBar
	{
		std::string aSideBar;//所有纵筋信息
		std::string bSideBar;//b 侧纵筋
		std::string hSideBar;//h 侧纵筋
		std::string cornerBars;//脚筋
	};
	struct ColumnStirrup
	{
		int hCount;//横向支数
		int vCount;//纵向支数
		std::string aStirrupInfo;//柱断面所有标识箍筋信息的所有信息
		std::vector<std::string>configuration;//箍筋信息排列信息
	};
public:
	std::string name;//柱截面名字
	std::string scal;//柱截面比例
	unsigned int section_id;//柱截面ID
	bool bRectangle;//判断柱截面是否为矩形柱
	double width;//柱截面的宽
	double height;//柱截面高

	SideBar bars;//纵筋信息
	ColumnStirrup stirrup;//箍筋信息
						  //std::string stirrups;//箍筋型号
	std::string others;//额外的信息


};

class ColumnInfo
{
/************新结构读入数据**************/
#if 1
/************interface function*****************/
public:

	bool findColumnPathJsonValue(const Json::Value& columnJV);
	//从具体文件读出具体的柱Json数据
	bool readColumnLocInfo();
	//输出柱子的整体数据，分别由位置数据，钢筋数据，校验后数据
	bool outputColumnJsonData();
/****************private function*******************/
private:
	
	const std::string setFloorPath(const std::string& file, const std::string& field);//设置读取数据数据的路径

	std::vector<ColumnLoc> setColumnLocJV(const Json::Value& columnLocValue);//将柱位置Json数据解析为内存数据
	//bool setColumnSectionJV(const Json::Value& columnSectionValue);
	bool checkSameFloorColumnLoc();//校验同一层，不同图纸间柱位置问题，用于校验错误

	bool outputErrorLocData(const std::string& basePath);
	bool outputColumnLocData(const std::string& basePath);
	bool outputColumnSectionData(const std::string& basePath);

	bool singleLayerFloor(const std::string& floorPlan, const std::vector<ColumnLoc>& columnLocVec);//当某层只有一张图纸时
	std::string setWriteFile(const std::string& floorFile);//用于判断某个层文件加是否存在，返回具体路径
	
private:
	Json::Value _columnLocJV;
	Json::Value _columnSectionJV;
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//第一个string表示某层，第二个表示某层的某张图纸，第三个表示某个字段
	std::map < std::string, std::vector<std::pair<std::string, std::vector<ColumnLoc>>>>_drawingColumnLocVec;//第一个string表示某层，第二个表示某图纸，vec表示柱列表
	std::map < std::string, std::vector<ColumnLoc>>_unkonwNameColumnVec;//不知道名字的柱子位置信息
	std::map < std::string, std::vector<ColumnLoc>>_notSameColumnLocVec;//本该相同的柱不相同
	std::map < std::string, std::vector<ColumnLoc>>_outputColumnLocVec;//没有问题可以正确输出的位置信息
	ResultWrite resultWrite;//调用写入结果函数的对象
	ResultRead resultRead;//调用读入过程结果函数的对象
	std::vector<std::string>_floorVec;
public:
	std::map<std::string, std::vector<std::shared_ptr<Pillar>>>_pillarVecMap;//记录每张图纸中的柱位置信息


#endif

};
#endif