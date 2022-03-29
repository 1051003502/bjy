#include "wallProcessInfo.h"

bool WallLoc::setName(const std::string& name)
{
	this->_name = name;

	return true;
}

bool WallLoc::setRelativeElevation(const double& elevation)
{
	this->_relativeElevation = elevation;
	return true;
}

bool WallLoc::setThick(const double& thick)
{
	this->_thick = thick;
	return true;
}

bool WallLoc::setStartReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis)
{
	this->_startPoint._referenceHAxis = hAxis;
	this->_startPoint._referenceVAxis = vAxis;
	return true;
}

bool WallLoc::setEndReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis)
{
	this->_endPont._referenceHAxis = hAxis;
	this->_endPont._referenceVAxis = vAxis;
	return true;
}

