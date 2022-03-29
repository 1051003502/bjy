#ifndef __PARSER_UTILS_LINE_H__
#define __PARSER_UTILS_LINE_H__

#include "point.h"



class Endpoint : public Point
{
public:
	Endpoint(int idx, const Point& p) : index(idx), Point(p)
	{}
	int index;
};

class Line
{
public:
	using Type = enum
	{
		CONTINUS,
		DOTS,
		DASHED,
		DIMENSIONS,
		EMPTY,
		UNKNOWN
	};

	Type getLineType(const std::shared_ptr<DRW_Entity> entity) const
	{
		if (entity)
		{
			if (entity->dwgType >= DRW::DIMENSION_ORDINATE &&
				entity->dwgType <= DRW::DIMENSION_DIAMETER)
				return DIMENSIONS;
		}
		return UNKNOWN;
	}

	Line() = default;


	Line(const Point& p1, const Point& p2) : entity(p1.entity), side(false)
	{
		type = getLineType(entity);
		setSE(p1, p2);
	}

	Line(const Point& p1, const Point& p2, bool side) : entity(p1.entity), side(side)
	{
		type = getLineType(entity);
		setSE(p1, p2);
	}

	Line(const Point& p1, const Point& p2, std::shared_ptr<DRW_Entity> entity) : entity(entity), side(false)
	{
		type = getLineType(entity);
		setSE(p1, p2);
	}
	Line(double x1, double y1, double x2, double y2)
	{
		type = Type::UNKNOWN;
		setSE(Point(x1, y1), Point(x2, y2));
	}
	Line(const Line& line) : s(line.s), e(line.e), entity(line.entity), side(line.side)
	{
		if (entity != nullptr)
			type = getLineType(entity);
	}
	
	Line& operator=(const Line& line)
	{
		if (this == &line)
			return *this;
		side = line.side;
		s = line.s;
		e = line.e;
		entity = line.entity;
		type = line.type;
		return *this;
	}
	bool operator==(const Line& line) const
	{
		if (side != line.side)
			return false;
		if (s != line.s)
			return false;
		if (e != line.e)
			return false;
		if (entity != line.entity)
			return false;
		if (type != line.type)
			return false;
		return true;
	}
	bool operator<(const Line& line)const
	{
		if (this->s < line.s)return true;
		if (line.s < this->s)return false;
		return this->e < line.e;
	}
	void setSE(const Point& p1, const Point& p2)
	{
		if (vertical(p1, p2))
		{
			if (p1.y < p2.y)
			{
				s = p1;
				e = p2;
			}
			else
			{
				s = p2;
				e = p1;
			}
		}
		else if (horizontal(p1, p2))
		{
			if (p1.x < p2.x)
			{
				s = p1;
				e = p2;
			}
			else
			{
				s = p2;
				e = p1;
			}
		}
		// K
		else if (p1.x + Precision < p2.x /* || (abs(p1.x - p2.x) < Precision && p1.y < p2.y)*/)
		{
			s = p1;
			e = p2;
		}
		else
		{
			s = p2;
			e = p1;
		}
	}

	const std::string& getLayer() const
	{
		if (entity.get())
		{
			return entity->layer;
		}
		return defaultLayer;
	}

	const std::string& getType()
	{
		if (entity.get())
		{
			//
		}
		return defaultLayer;
	}

	bool solidSide() const
	{
		if (entity.get())
		{
			return entity->dwgType == DRW::SOLID ||
				entity->dwgType == DRW::HATCH;
		}
		return false;
	}

	bool isSide() const
	{
		return side;
	}
	bool isDimension()const
	{
		if (getLineType(entity) == Type::DIMENSIONS)
			return true;
		else 
			return false;
	}
	bool isEmpty()const
	{
		if (type == Type::EMPTY)
			return true;
		return false;
	}
	static bool vertical(const Point& p1, const Point& p2)
	{
		if (abs(p1.y - p2.y) == 0)
		{
			return false;
		}
		double r = abs(p1.x - p2.x) / abs(p1.y - p2.y);
		return r < PrecisionAngle;
	}

	static bool horizontal(const Point& p1, const Point& p2)
	{
		if (abs(p1.x - p2.x) == 0)
		{
			return false;
		}
		double r = abs(p1.y - p2.y) / abs(p1.x - p2.x);
		return r < PrecisionAngle;
	}
	static Line getEmptyLine()
	{
		Line line;
		line.type = Line::Type::EMPTY;
		return line;
	}
	double length()
	{
		if (vertical())
		{
			return e.y - s.y;
		}
		else if (horizontal())
		{
			return e.x - s.x;
		}
		double y = e.y - s.y;
		double x = e.x - s.x;
		return sqrt(x * x + y * y);
	}

	double length() const
	{
		return const_cast<Line*>(this)->length();
	}

	bool vertical() const
	{
		return Line::vertical(s, e);
	}
	bool horizontal() const
	{
		return Line::horizontal(s, e);
	}
	//-1.57...~1.57...(-90°~90°)
	static double rad(const Line& line)
	{
		double a = std::atan2(line.e.y - line.s.y, line.e.x - line.s.x);
		return /*a < 0 ? a + M_PIx2 : */a;
	}

	double rad() const
	{
		return rad(*this);
	}
	
	bool perpendicularTo(const Line& line) const
	{
		if (this->horizontal() && line.vertical())
			return true;
		if (this->vertical() && line.horizontal())
			return true;
		//todo pending to verify
		auto a = std::tan(this->rad());
		auto b = std::tan(line.rad());
		auto c = a * b;
		if (abs(c + 1) < 0.1)
			return true;
		return false;
	}
	bool parallelTo(const Line& line)const
	{
		if (this->horizontal() && line.horizontal())
			return true;
		if (this->vertical() && line.vertical())
			return true;
		auto a = std::tan(this->rad());
		auto b = std::tan(line.rad());
		if (abs(a - b) < 0.01)
			return true;
		return false;
	}
	//说明：判断点是否是一根线的端点(只考虑位置)
	//返回：true 是端点     false  不是此线端点
	bool checkEndPoint(const Point& p)const
	{
		if (this->s.positionEqual(p))return true;
		if (this->e.positionEqual(p))return true;
		return false;
	}
	//判断两条线是否重复
	bool bRepeatLine(const int& index1, const int& index2);
	static bool bRepeatLine(const Line& line1, const Line& line2);
	const static std::string defaultLayer;
	Point s;
	Point e;
	Type type;
	std::shared_ptr<DRW_Entity> entity;
	bool side; /* 是否闭合 */
	std::string layer;
};


//由线依次围成的闭合区
class ClosedZone
{
public:
	typedef enum
	{
		U,//上
		D,//下
		L,//左
		R//右

	}Scope;//闭合线的每条线段都有自己闭合区域的作用域
		   //ClosedZone() {}


public:
	std::vector<std::pair<int, Scope>>borderline;

};

inline bool compareRefH(const std::vector<Line>& lines, int l1, int l2)
{
	const Line& p1 = lines[l1];
	const Line& p2 = lines[l2];
	if (p1.s.y > p2.s.y + Precision)
		return false;
	else if (p1.s.y < p2.s.y - Precision)
		return true;
	return p1.s.x < p2.s.x;
}

inline bool compareRefV(const std::vector<Line>& lines, int l1, int l2)
{
	const Line& p1 = lines[l1];
	const Line& p2 = lines[l2];
	if (p1.s.x > p2.s.x + Precision)
		return false;
	else if (p1.s.x < p2.s.x - Precision)
		return true;
	return p1.s.y < p2.s.y;
}

inline bool comparev(const Line& p1, const Line& p2)
{
	if (p1.s.x > p2.s.x + Precision)
		return false;
	else if (p1.s.x < p2.s.x - Precision)
		return true;
	return p1.s.y < p2.s.y;
}

inline bool compareh(const Line& p1, const Line& p2)
{
	if (p1.s.y > p2.s.y + Precision)
		return false;
	else if (p1.s.y < p2.s.y - Precision)
		return true;
	return p1.s.x < p2.s.x;
}

int findLowerBound(double y, const std::vector<int>& list,
				   const std::vector<Line>& lines,
				   double precision = Precision, bool h = true);
int findUpperBound(double y, const std::vector<int>& list,
				   const std::vector<Line>& lines,
				   double precision = Precision, bool h = true);

//两点式直线方程，已知x求y
double knowXToY(const Line& line, const double& x);
//已知y求x
double knowYToX(const Line& line, const double& y);
// 得到直线的tan斜率
double GetTanOfLine(const Line& line);
// 判断两直线是否平行
bool IsParallelLines(const Line& line0, const Line& line1);
//求斜平行线之间的距离
double slantingDistanceLine(const Line& line0, const Line& line1);

//判断线1是否在线2的上面
bool isAbove(const Line& l1, const Line& l2);

//判断线1是否和线2在同一延长线
bool bAtExtendedLine(const Line& l1, const Line& l2);


#endif