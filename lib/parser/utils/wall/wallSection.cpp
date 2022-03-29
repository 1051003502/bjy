#include "wallSection.h"

Haunch::Haunch()
{
}

Haunch::Haunch(const std::string& info,
	const double& width,
	const double& height,
	const double& insideWall,
	const double& insideBoard)
{
	this->_rebarInfo = info;
	this->_haunchWidth = width;
	this->_hunchHeight = height;
	this->_insideBoardLength = insideBoard;
	this->_insideWallLength = insideWall;
}


