#ifndef __DRAWINGRELATED_COORDINATESYSTEM_H__
#define __DRAWINGRELATED_COORDINATESYSTEM_H__
#include "parser.h"
#include "spdlog/spdlog.h"
#include "RWJsonFile.h"
typedef std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>> ProjectFile;


/*!
 * \class CoordinateSystem
 *
 * \brief 全局坐标系的作用：
对于建筑图来说，一个建筑他的图纸，具体指坐标图纸。一般由多张侧重点不同的图相互补充构成，
现在的全局轴网就相当于给图纸中所有的构建，提供一个统一的平台，每张图纸将分析后的数据进行映射，
这样只需对构件进行添加补充，而不需要遍历查找。
 *
 * \author Xiong
 * \date 14/2/2020
 */
class CoordinateSystem
{

	/************************************新算法**********************************/
public://interface function
	//获得轴网信息存储字段的路径
	bool findAxisPathJsonValue(const Json::Value& columnJV);
	//从具体文件读出轴网的Json数据
	bool readAxisCoordinateSystem();
	//输出轴网的整体数据，
	bool outputAxisJsonData();
	//轴网信息结果文件路径
	std::string constAxisPath();

	
	

private:
	const std::string setFloorPath(const std::string& type, const std::string& field);//设置读取数据数据的路径
	std::vector<AxisProcessInfo> setColumnLocJV(const Json::Value& axisValue);//将柱位置Json数据解析为内存数据
	bool mergeAxis();//校验轴网
	bool checkAxis(const std::vector<AxisProcessInfo>& drawingAxisVec);//校验多个轴网
	bool checkHAxis(const std::vector<SingleAxisInfo>& hAxisVec, std::vector<SingleAxisInfo>& goalHAxisVec);//校验轴网的横轴
	bool checkVAxis(const std::vector<SingleAxisInfo>& vAxisVec, std::vector<SingleAxisInfo>& goalVAxisVec);//校验轴网的纵轴
	bool checkSameName();//检验同名轴的情况
	bool checkAxisDistance();//测试坐标系内，轴距小于五十的轴
	bool outputAll();//全部输出
	bool outputError();//错误信息输出
private:
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//第一个string表示某层，第二个表示某层的某张图纸，第三个表示某个字段
	std::map < std::string, std::vector<std::pair<std::string, std::vector<AxisProcessInfo>>>>_drawingAxisVec;//第一个string表示某层，第二个表示某图纸，vec表示柱列表
	std::vector<std::string>_floorVec;
	ResultWrite _resultWrite;//调用写入结果函数的对象
	ResultRead _resultRead;//调用读入过程结果函数的对象
	AxisProcessInfo _goalAxisSyetem;
	std::vector<std::vector<std::string>>_lessThanError;//轴距过小错误
	std::vector<std::string>_sameNameError;//轴距过小错误

	/***************************************************************************/
public:
	//************************************
	// 方法	: CoordinateSystem
	// 全名	: CoordinateSystem::CoordinateSystem
	// 访问	: public 
	// 返回	: 
	// 说明	: actor
	//************************************
	CoordinateSystem() {}
	//************************************
	// 方法	: iniAxisMap
	// 全名	: CoordinateSystem::iniAxisMap
	// 访问	: public 
	// 返回	: bool
	// 参数	: _axisMap
	//! 说明	: 用于初始化_axisMap
	//************************************
	//bool iniAxisMap(const std::map<std::string, std::shared_ptr<Axis>>&_axisMap);

	//************************************
	// 方法	: iniGlobalCoordinateSystem
	// 全名	: CoordinateSystem::iniGlobalCoordinateSystem
	// 访问	: public 
	// 返回	: bool 
	// 说明	: 将多个轴网相互关联，建立完整的全局坐标系,参数为多张图纸的
	//************************************
	bool iniGlobalCoordinateSystem();
	//!转换全局坐标系到某图纸坐标系
	bool fromGlobalToDrawing();
	//!转换图纸坐标系到全局
	bool fromDrawingToGlobal();
public:
	std::map<std::string,double> vlines_cooMap;//!第一个参数表示轴网的名字，第二个参数表示相对位置
	std::map<std::string,double> hlines_cooMap;//!同上，上一个表示纵轴，下一个表示横轴

	std::string referenceAxisX;//!参照轴x
	std::string referenceAxisY;//!参照轴y

	std::map<std::string, std::shared_ptr<Axis>>axisMap;//!分别为没张图纸对应的轴网
	
	std::map<std::string, std::pair<std::string, std::string>>referenceAxis;//!每张图纸轴网在全局轴网内的参照轴secont.first为横轴second.second为纵轴


};

#endif // !1

