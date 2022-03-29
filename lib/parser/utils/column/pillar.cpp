#include "pillar.h"



bool IsItTheSamePillar(const Pillar &pillar1, const Pillar &pillar2)
{

	//bool mark = false;
	bool mark_h = false;
	bool mark_v = false;
	/*if (pillar1.name == pillar2.name)
	{
		mark = true;
	}
	else
	{
		return false;
	}*/
	//ºáÖá
	for (auto it1 : pillar1._haxis)
	{
		for (auto it2 : pillar2._haxis)
		{
			if (it1.first == it2.first)
			{
				mark_h = true;
				break;
			}
			else
			{
				mark_h = false;
			}
		}
		if (mark_h == true)
		{
			break;
		}
	}
	/*if (!mark)
		return false;*/
	//×ÝÖá
	for (auto it1 : pillar1._vaxis)
	{
		for (auto it2 : pillar2._vaxis)
		{
			if (it1.first == it2.first)
			{
				mark_v = true;
				break;
			}
			else
			{
				mark_v = false;
			}
		}
		if (mark_v == true)
		{
			break;
		}
	}

	if (/*mark && */mark_h && mark_v)
	{
		return true;
	}
	else
	{
		return false;
	}
}



void Pillar::SetHsize()
{
	AxisMark temp;
	for (auto it : _haxis)
	{
		if (temp.first == "")
		{
			temp = it;
		}
		else
		{
			temp = temp.second < it.second ? temp : it;
		}
	}
	_hdistance.first = temp.first;
	_hdistance.second.first = ROUND(_box.bottom) - ROUND(temp.second);
	
}

void Pillar::SetVsize()
{
	AxisMark temp;
	for (auto it : _vaxis)
	{
		if (temp.first == "")
		{
			temp = it;
		}
		else
		{
			temp = temp.second < it.second ? temp : it;
		}
	}
	_vdistance.first = temp.first;
	/*vdistance.second.first = ROUND(box.left) - ROUND(temp.second);
	vdistance.second.second = ROUND(box.right) - ROUND(temp.second);*/
	_vdistance.second.first = ROUND(_box.left) - ROUND(temp.second);
}

void Pillar::SetPillarBox()
{
	for (auto it : _vlines)
	{
		_box.expand(it.s);
		_box.expand(it.e);
	}
	for (auto it : _hlines)
	{
		_box.expand(it.s);
		_box.expand(it.e);
	}
	for (auto it : _slines)
	{
		_box.expand(it.s);
		_box.expand(it.e);
	}
}

void Pillar::RestPillarAxis()
{
	_haxis.clear();
	_vaxis.clear();
}

bool Pillar::setVlines(const std::vector<Line>& lineVec)
{
	this->_vlines = lineVec;
	return true;
}

bool Pillar::setHlines(const std::vector<Line>& lineVec)
{
	this->_hlines = lineVec;
	return true;
}

bool Pillar::setSlines(const std::vector<Line>& lineVec)
{
	this->_slines = lineVec;
	return true;
}

bool Pillar::setDLines(const std::vector<Line>& lineVec)
{
	this->_dlines = lineVec;
	return true;
}

bool Pillar::setHAxis(const std::vector<AxisMark>& hAxis)
{
	this->_haxis = hAxis;
	return true;
}

bool Pillar::setVAxis(const std::vector<AxisMark>& vAxis)
{
	this->_vaxis = vAxis;
	return true;
}

bool Pillar::setHAxis(const AxisMark& hAxis)
{
	this->_haxis.push_back(hAxis);
	return true;
}

bool Pillar::setVAxis(const AxisMark& vAxis)
{
	this->_vaxis.push_back(vAxis);
	return true;
}

std::vector<Line> Pillar::getVlines()
{
	return _vlines;
}

std::vector<Line> Pillar::getHlines()
{
	return _hlines;
}

std::vector<Line> Pillar::getSlines()
{
	return _slines;
}

std::vector<Line> Pillar::getDLines()
{
	return this->_dlines;
}

bool Pillar::setError(const bool& error)
{
	return error;
}

bool Pillar::setAngle()
{
	if (_hlines.empty() && _vlines.empty() && !_slines.empty())
	{
		auto lowestPoint = returnLowestPoint();
		auto lines = retrunConnecLines(_slines, lowestPoint);
		this->_angle = returnAngle(lines);
	}
	return true;
}



std::vector<AxisMark> Pillar::getHAxis()
{
	return this->_haxis;
}

std::vector<AxisMark> Pillar::getVAxis()
{
	return this->_vaxis;
}

std::string Pillar::getName()
{
	return this->_name;
}

Point Pillar::getTestPoint()
{
	return this->_point;
}

bool Pillar::getError()
{
	return _error;
}

double Pillar::getAngle()
{
	return this->_angle;
}

Box Pillar::getBox()
{
	return this->_box;
}

PillarDistance Pillar::getHPDistance()
{
	return _hdistance;
}

PillarDistance Pillar::getVPDistance()
{
	return _vdistance;
}


Point Pillar::returnLowestPoint()
{
	Point tempPoint(-1, -1);
	if (!_slines.empty())
	{
		std::vector<Point>pointVec;
		for (auto sl : _slines)
		{
			pointVec.push_back(sl.s);
			pointVec.push_back(sl.e);
		}
		for (auto p : pointVec)
		{
			if (tempPoint.y == -1)
			{
				tempPoint = p;
			}
			else
			{
				if (tempPoint.y > p.y)
				{
					tempPoint = p;
				}
			}
		}
	}
	return tempPoint;
}

std::vector<Line> Pillar::retrunConnecLines(const std::vector<Line>& lines, const Point& point)
{
	std::vector<Line>goalLines;
	for (auto it : lines)
	{
		if (ConnectPoint(it.s, point) || ConnectPoint(it.e, point))
		{
			goalLines.push_back(it);
		}
		
	}
	return goalLines;
}

double Pillar::returnAngle(const std::vector<Line>& lines)
{
	Line tempLine(Point(0, 0), Point(100, 0));
	std::vector<double>angleVec;
	double goalAngle = -1;
	for (auto itLine : lines)
	{
		angleVec.push_back(itLine.rad());
	}

	if (angleVec.front() > 0 && angleVec.back() < 0)
	{
		return  deg2Ang(angleVec.front());
	}
	else if (angleVec.front() < 0 && angleVec.back() > 0)
	{
		return deg2Ang(angleVec.back());
	}
	else
	{
		return -1;//表示有错
	}
	
}

