#ifndef __DRAWINGRELATED_AXIS_PROCESS_INFO_H__
#define __DRAWINGRELATED_AXIS_PROCESS_INFO_H__
#include "line.h"
#include "json/json.h"

class SingleAxisInfo
{
public:
	SingleAxisInfo(const double& distance = 0, const std::string& name = "", const double& angle = 0)
	{
		this->_distance = distance;
		this->_name = name;
		this->_angle = angle;
	}
public:
	bool setDistance(const double& distance);
	bool setName(const std::string& name);
	bool setAngle(const double& angle);
	std::string getName();
	double getDistance();
	double getAngle();
	static bool findAxis(const std::vector<SingleAxisInfo>& axisVec, SingleAxisInfo axis);

	
	
private:
	double _distance;
	std::string _name;
	double _angle;
	
};

class AxisProcessInfo
{
public:
	bool setHAxises(const std::vector<SingleAxisInfo>& hAxises);
	bool setVAxises(const std::vector<SingleAxisInfo>& vAxises);
	std::vector<SingleAxisInfo> getHAxises();
	std::vector<SingleAxisInfo> getVAxises();

private:
	std::vector<SingleAxisInfo> _hAxises;
	std::vector<SingleAxisInfo>_vAxises;
};


#endif