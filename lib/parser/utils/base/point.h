#ifndef __PARSER_UTILS_POINT_H__
#define __PARSER_UTILS_POINT_H__

#include "common.h"
#include "drw_interface.h"
#include <array>
class Point : public std::array<double, 2>
{
public:
	static const int DIM = 2;
	Point() : x(0), y(0) 
	{
		entity = nullptr;
	}
	Point(double x, double y, std::shared_ptr<DRW_Entity> ent = nullptr) : x(x), y(y), entity(ent)
	{
		(*this)[0] = x;
		(*this)[1] = y;
	}
	Point(const Point& p) : x(p.x), y(p.y)/*, entity(p.entity==nullptr?nullptr:p.entity)*/
	{
		(*this)[0] = x;
		(*this)[1] = y;
		if (p.entity.get())
			entity = p.entity;
	}

	const std::string& getLayer() const
	{
		if (entity.get())
			return entity->layer;
		return emptyStr;
	}
	const std::string getText()const 
	{
		if (this->entity != NULL)
		{
			auto pEntity=std::dynamic_pointer_cast<DRW_Text>(this->entity);
			return pEntity->text;
		}
		return emptyStr;
	}
	std::string getShowString()const {
		std::string str="("+std::to_string(x) + "," + std::to_string(y) + ")";
		return str;
	}
	void round()
	{
		x = std::round(x);
		y = std::round(y);
	}
	Json::Value getXYText()
	{
		int newX = int(std::round(x));
		int newY = int(std::round(y));
		return "(" + std::to_string(newX) + "," + std::to_string(newY) + ")";
	}
	Point& operator=(const Point& p)
	{
		if (this != &p)
		{
			x = p.x;
			y = p.y;
			entity = p.entity;
			(*this)[0] = x;
			(*this)[1] = y;
		}
		return *this;
	}
	bool operator<(const Point& p)const {
		if (this->x + Precision < p.x)return true;
		if (this->x - Precision > p.x)return false;
		if (this->y + Precision < p.y)return true;
		return false;
	}
	double distance(const Point& p) const
	{
		auto dx = p.x - x;
		auto dy = p.y - y;
		auto quare = dx * dx + dy * dy;
		if (quare < Precision)
			return 0.0;
		return sqrt(quare);
	}
	bool operator==(const Point& point)
	{
		if (this->x == point.x &&
			this->y == point.y &&
			this->entity == point.entity)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	//得到文本点文本的方向  1水平 2垂直  0其他
	int getDirection() const{
		double point_angle = std::dynamic_pointer_cast<DRW_Text>(this->entity)->angle;
		//k倍PI水平
		double value1 = point_angle / M_PI;
		double value2 = (int)(point_angle / M_PI);
		double value3 = value1 - value2;
		point_angle += PrecisionAngle;
		if (abs(point_angle / M_PI - (int)(point_angle / M_PI)) < PrecisionAngle) {
			return 1;
		}
		//k倍 0.5PI 垂直
		if (abs(point_angle / M_PI_2 - (int)(point_angle / M_PI_2)) < PrecisionAngle && abs(point_angle) > PrecisionAngle) {
			return 2;
		}
		return 0;
	}
	//说明：判断与两个点是否位置相同
	//返回：true 位置相同    false 位置不同
	bool positionEqual(const Point& p)const
	{
		return std::abs(this->x - p.x)<Precision && std::abs(this->y - p.y)<Precision;
	}
	
public:
	double x;
	double y;
	std::shared_ptr<DRW_Entity> entity;
};

inline bool compareX(const Point& p1, const Point& p2)
{
	if (p1.x > p2.x + Precision)
		return false;
	else if (p1.x < p2.x - Precision)
		return true;
	return p1.y < p2.y;
}

inline bool compareY(const Point& p1, const Point& p2)
{
	if (p1.y > p2.y + Precision)
		return false;
	if (p1.y < p2.y - Precision)
		return true;
	return p1.x < p2.x;
}
const Point emptyPoint(0, 0);
int findLowerBound(double y, const std::vector<Point>& points, double precision = Precision);

int findUpperBound(double y, const std::vector<Point>& points, double precision = Precision);
bool ConnectPoint(const Point& p1, const Point& p2);//两点足够接近返回TRUE

double TowPointsLength(const Point& p1, const Point& p2);

#endif