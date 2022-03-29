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
	double measurement;//�ߴ�
	double length;
	std::vector<Line> lines;
	std::vector<int>lineIndex;
	double angle_rotated;	//��ת�Ƕȣ�ˮƽ��ֱ�ߴ�
	double oblique_angle;	//б�Ƕ�
	double direction;
	Box box;
	double scale;//!������Ϣ
};

//idx, �Ƿ���
using  DimensionPair = std::pair<unsigned, bool>;

#endif