#include "columnProcessInfo.h"

#if 1
bool ColumnLoc::setHlines(const Line& line)
{
	this->_hlines.push_back(line);
	return true;
}

bool ColumnLoc::setHlines(const std::vector<Line>& lineVec)
{
	if (!lineVec.empty())
	{
		this->_hlines.insert(this->_hlines.end(), lineVec.begin(), lineVec.end());
	}
	return true;
}

bool ColumnLoc::setVlines(const Line& line)
{
	this->_vlines.push_back(line);
	return true;
}

bool ColumnLoc::setVlines(const std::vector<Line>& lineVec)
{
	if (!lineVec.empty())
	{
		this->_vlines.insert(this->_vlines.end(), lineVec.begin(), lineVec.end());
	}
	return true;
}

bool ColumnLoc::setSlines(const Line& line)
{
	this->_slines.push_back(line);
	return true;
}

bool ColumnLoc::setSlines(const std::vector<Line>& lineVec)
{
	if (!lineVec.empty())
	{
		this->_slines.insert(this->_slines.end(), lineVec.begin(), lineVec.end());
	}
	return true;
}
bool ColumnLoc::setReferenceH(const std::string& name, const double& distance)
{
	this->_referenceH = std::pair<std::string, double>(name, distance);
	return true;
}
bool ColumnLoc::setReferenceV(const std::string& name, const double& distance)
{
	this->_referenceV = std::pair<std::string, double>(name, distance);
	return true;
}
bool ColumnLoc::setName(const std::string& name)
{
	this->_name = name;
	return true;
}
bool ColumnLoc::setReferenceHAxisValue(const Json::Value& hAxis)
{
	this->_referenceHAxisValue = hAxis;
	return true;
}
bool ColumnLoc::setReferenceVAxisValue(const Json::Value& vAxis)
{
	this->_referenceVAxisValue = vAxis;
	return true;
}

bool ColumnLoc::setAngle(const double& angle)
{
	this->_angle = angle;
	return true;
}
std::vector<Line> ColumnLoc::getHlines()
{
	return _hlines;
}
std::vector<Line> ColumnLoc::getVlines()
{
	return _vlines;
}
std::vector<Line> ColumnLoc::getSlines()
{
	return _slines;
}
std::string ColumnLoc::getName()
{
	return _name;
}
Json::Value ColumnLoc::getReferenceHAxisValue()
{
	return _referenceHAxisValue;
}
Json::Value ColumnLoc::getReferenceVAxisValue()
{
	return _referenceVAxisValue;
}
double ColumnLoc::getAngle()
{
	return _angle;
}
#endif
