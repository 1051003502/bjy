#include "wallLocation.h"

WallReferencePoint::WallReferencePoint()
{
}

WallReferencePoint::WallReferencePoint(const std::pair<std::string, double>& referenceHAxis,
	const std::pair<std::string, double>& referenceVAxis)
{
	auto tempHName = referenceHAxis.first;
	auto tempVName = referenceVAxis.first;
	double hDistance = referenceHAxis.second;
	double vDistance = referenceVAxis.second;

	this->_referenceHAxis = std::pair<std::string, double>(tempHName, strimDouble(hDistance));
	this->_referenceVAxis = std::pair<std::string, double>(tempVName, strimDouble(vDistance));


}

std::pair<std::string, double> WallReferencePoint::getReferenceHAxis()
{
	return this->_referenceHAxis;
}

std::pair<std::string, double> WallReferencePoint::getReferenceVAxis()
{
	return this->_referenceVAxis;
}

bool WallLocation::setName(const std::string& name)
{
	this->_name = name;
	return true;
}

bool WallLocation::setRelativeElevation(const double& re)
{
	this->_relativeElevation = re;
	return true;
}

bool WallLocation::setThick(const double& thick)
{
	this->_thick = thick;
	return true;
}

bool WallLocation::setReferencePoint(const std::vector<std::pair<std::string, double>>& startPoint, 
	const std::vector<std::pair<std::string, double>>& endPoint)
{
	this->_startPoint = WallReferencePoint(startPoint.front(), startPoint.back());
	this->_endPoint = WallReferencePoint(endPoint.front(), endPoint.back());
	return true;
}

bool WallLocation::setBorders(const std::vector<Line>& lines)
{
	std::vector<Line>hLines, vLines, sLines;
	for (auto it : lines)
	{
		if (it.horizontal())
		{
			hLines.push_back(it);
		}
		else if (it.vertical())
		{
			vLines.push_back(it);
		}
		else
		{
			sLines.push_back(it);
		}
	}

	_wallBorderLine.hLines(hLines);
	_wallBorderLine.vLines(vLines);
	_wallBorderLine.sLines(sLines);


	if (hLines.empty() && vLines.empty()&&!sLines.empty())
	{
		this->wallType = WALLTYPE::OBLIQUE;
	}
	return true;
}

std::string WallLocation::getName()
{
	return this->_name;
}

double WallLocation::getRelativeElevation()
{
	return this->_relativeElevation;
}

double WallLocation::getThick()
{
	return this->_thick;
}

WallBorderLine WallLocation::getWallBorderLine()
{
	return _wallBorderLine;
}



WallReferencePoint WallLocation::getReferenceStartPoint()
{
	return this->_startPoint;
}

WallReferencePoint WallLocation::getReferenceEndPoint()
{
	return this->_endPoint;
}

std::vector<Line> WallLocation::getBorder()
{
	std::vector<Line>tempLines;
	auto temp = _wallBorderLine.getHLines();
	tempLines.insert(tempLines.end(), temp.begin(), temp.end());
	temp = _wallBorderLine.getVLines();
	tempLines.insert(tempLines.end(), temp.begin(), temp.end());
	temp = _wallBorderLine.getSLines();
	tempLines.insert(tempLines.end(), temp.begin(), temp.end());

	return tempLines;
}

bool WallBorderLine::hLines(const std::vector<Line> hlines)
{
	this->_hLines = hlines;
	return true;
}

bool WallBorderLine::vLines(const std::vector<Line> vlines)
{
	this->_vLines = vlines;
	return true;
}

bool WallBorderLine::sLines(const std::vector<Line> slines)
{
	this->_sLines = slines;
	return true;
}

std::vector<Line> WallBorderLine::getHLines()
{
	return this->_hLines;
}

std::vector<Line> WallBorderLine::getVLines()
{
	return this->_vLines;
}

std::vector<Line> WallBorderLine::getSLines()
{
	return this->_sLines;
}
