#include"dimension.h"


Dimension::Dimension(const EmDimType& type, const double& size, std::vector<Line> lines, std::vector<int> index)
{
	this->emDimType = type;
	this->measurement = size;
	this->lines = lines;
	this->angle_rotated = 0;
	this->oblique_angle = 0;
	this->lineIndex = index;
	setDirection();
	for (const auto& it : this->lines)
	{
		box.expand(it.s);
		box.expand(it.e);
	}
}

Dimension::Dimension(const EmDimType& type, const double& size, std::vector<Line> lines, std::vector<int> index,
					 const double& angle_rotated, const double& oblique_angle)
{
	this->emDimType = type;
	this->measurement = size;
	this->lines = lines;
	this->lineIndex = index;
	this->angle_rotated = angle_rotated;
	this->oblique_angle = oblique_angle;
	setDirection();
	for (const auto& it : this->lines)
	{
		box.expand(it.s);
		box.expand(it.e);
	}
}

Dimension::Dimension(const EmDimType& type, const double& size,
					 const Point& pt)
{
	this->emDimType = type;
	this->measurement = size;
	this->pt = pt;
}

bool Dimension::setDirection()
{
	auto it1 = this->lines.back();
	auto it2 = this->lines.front();
	double temp = 0;
	double length = 0;
	//�˴���Ӧ����ô�ɣ�Ӧ����������ߣ�������x������н�
	if (it1.s.y + PrecisionAngle > it1.e.y && it1.s.y - PrecisionAngle < it1.e.y)
	{
		if (it1.s.y - PrecisionAngle > it2.s.y)
		{

			this->direction = 270;//��

		}
		else
		{
			this->direction = 90;//��

		}
	}
	else if (it1.s.x + PrecisionAngle > it1.e.x && it1.s.x - PrecisionAngle < it1.e.x)
	{
		if (it1.s.x - PrecisionAngle > it2.s.x)
		{
			this->direction = 180;//��
		}
		else
		{
			this->direction = 0;//��
		}
	}
	return true;
}

Dimension::~Dimension()
{}

double Dimension::getActualMeasurement()
{
	return measurement;
}
