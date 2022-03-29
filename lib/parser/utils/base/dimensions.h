#ifndef __PARSER_UTILS_DIM_H__
#define __PARSER_UTILS_DIM_H__
#include"line.h"
#include "box.h"
#include "common.h"
#include <math.h>

class Dimensions
{
public:
	typedef enum {
		DIMLINEAR,
		DIMALIGNED,
		DIMANGULAR,
		DIMANGULAR3P,
		DIMRADIUS,
		DIMDIAMETER,
		DIMORDINATE
	}EmDimType;
public:
	
	Dimensions();
	Dimensions(const EmDimType& type, const double &size, const double& length, std::vector<Line> lines,std::vector<int> index);
	Dimensions(const EmDimType& type, const double &size, const double& length, std::vector<Line> lines, std::vector<int> index,
		const double&angle_rotated, const double&oblique_angle);
	bool SetDirection();

	~Dimensions();


public:
	EmDimType type;
	//std::string name;
	double measurement;//尺寸
	double length;
	std::vector<Line> lines;
	std::vector<int>lineIndex;
	double angle_rotated;	//旋转角度，水平或垂直尺寸
	double oblique_angle;	//斜角度
	double direction;
	Box box;
	double scale;//!比例信息
};

//idx, 是否增
using  DimensionPair = std::pair<unsigned, bool>;

#endif