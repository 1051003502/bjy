#ifndef __DRAWINGRELATED_COLUMNPROCESSINFO_H__
#define __DRAWINGRELATED_COLUMNPROCESSINFO_H__
#include "line.h"
#include "json/json.h"

class ColumnLoc
{
#if 1
public:
	//function interface

	/*初始化函数*/
	bool setHlines(const Line& line);
	bool setHlines(const std::vector<Line>& lineVec);
	bool setVlines(const Line& line);
	bool setVlines(const std::vector<Line>& lineVec);
	bool setSlines(const Line& line);
	bool setSlines(const std::vector<Line>& lineVec);
	bool setReferenceH(const std::string& name, const double& distance);
	bool setReferenceV(const std::string& name, const double& distance);
	bool setName(const std::string& name);
	bool setReferenceHAxisValue(const Json::Value& hAxis);
	bool setReferenceVAxisValue(const Json::Value& vAxis);
	bool setAngle(const double& angle);
	/*返回函数*/
	std::vector<Line> getHlines();
	std::vector<Line> getVlines();
	std::vector<Line> getSlines();
	std::string getName();
	Json::Value getReferenceHAxisValue();
	Json::Value getReferenceVAxisValue();
	double getAngle();
	
private:

private:
	double _angle;//角度
	std::string _name;//名字
	std::pair<std::string, double>_referenceH;
	std::pair<std::string, double>_referenceV;
	Json::Value _referenceHAxisValue;//柱参考轴
	Json::Value _referenceVAxisValue;//柱参考轴
	
	std::vector<Line>_hlines;
	std::vector<Line>_vlines;
	std::vector<Line>_slines;

#endif

#if 0
public:
	

private:
	std::vector<Line>_hlines;
	std::vector<Line>_vlines;
	std::vector<Line>_slines;

#endif
};

#endif