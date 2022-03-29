#ifndef __PARSER_UTILS_DIM_H__
#define __PARSER_UTILS_DIM_H__
#include"line.h"
#include "box.h"
#include "common.h"
#include <math.h>

class Dimension
{
public:
	typedef enum
	{
		DIMLINEAR,	//线性标注
		DIMALIGNED,	//对角标注
		DIMANGULAR,	//转角标注（2线）
		DIMANGULAR3P,//转角标注（3点）
		DIMRADIUS,	//半径标注
		DIMDIAMETER,	//直径标注
		DIMORDINATE		//坐标标注
	}EmDimType;
public:

	Dimension() = default;

	Dimension(const EmDimType& type,
			  const double& size,
			  const Point&pt);

	Dimension(const EmDimType& type,
			   const double& size,
			   std::vector<Line> lines,
			   std::vector<int> index);

	Dimension(const EmDimType& type,
			   const double& size,
			   std::vector<Line> lines,
			   std::vector<int> index,
			   const double& angle_rotated,
			   const double& oblique_angle);

	bool setDirection();

	~Dimension();

	double getActualMeasurement();


public:
	EmDimType emDimType;
	double measurement;				//尺寸
	std::vector<Line> lines;
	std::vector<int> lineIndex;
	double angle_rotated;			//旋转角度，水平或垂直尺寸
	double oblique_angle;			//斜角度
	double direction;
	Box box;
	double scale;					//!比例信息
	Point pt;						//!半径标注或直径标注所描述的圆心
};

//idx, 是否增
using  DimensionPair = std::pair<unsigned, bool>;

#endif