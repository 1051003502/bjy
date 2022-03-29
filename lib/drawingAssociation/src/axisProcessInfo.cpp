#include "axisProcessInfo.h"

bool SingleAxisInfo::setDistance(const double& distance)
{
	this->_distance = distance;
	return true;
}

bool SingleAxisInfo::setName(const std::string& name)
{
	this->_name = name;
	return true;
}

bool SingleAxisInfo::setAngle(const double& angle)
{
	this->_angle = angle;
	return true;
}

std::string SingleAxisInfo::getName()
{
	return this->_name;
}

double SingleAxisInfo::getDistance()
{
	return this->_distance;
}

double SingleAxisInfo::getAngle()
{
	return this->_angle;
}

bool SingleAxisInfo::findAxis(const std::vector<SingleAxisInfo>& axisVec, SingleAxisInfo axis)
{
	for (auto it : axisVec)
	{
		if (it.getName() == axis.getName() &&
			it.getAngle() == axis.getAngle() &&
			it.getDistance() == axis.getDistance())
		{
			return true;
		}
	}

	return false;
}




bool AxisProcessInfo::setHAxises(const std::vector<SingleAxisInfo>& hAxises)
{
	this->_hAxises = hAxises;
	return true;
}

bool AxisProcessInfo::setVAxises(const std::vector<SingleAxisInfo>& vAxises)
{
	this->_vAxises = vAxises;
	return true;
}

std::vector<SingleAxisInfo> AxisProcessInfo::getHAxises()
{
	return this->_hAxises;
}

std::vector<SingleAxisInfo> AxisProcessInfo::getVAxises()
{
	return this->_vAxises;
}


