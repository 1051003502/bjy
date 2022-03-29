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
		DIMLINEAR,	//���Ա�ע
		DIMALIGNED,	//�ԽǱ�ע
		DIMANGULAR,	//ת�Ǳ�ע��2�ߣ�
		DIMANGULAR3P,//ת�Ǳ�ע��3�㣩
		DIMRADIUS,	//�뾶��ע
		DIMDIAMETER,	//ֱ����ע
		DIMORDINATE		//�����ע
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
	double measurement;				//�ߴ�
	std::vector<Line> lines;
	std::vector<int> lineIndex;
	double angle_rotated;			//��ת�Ƕȣ�ˮƽ��ֱ�ߴ�
	double oblique_angle;			//б�Ƕ�
	double direction;
	Box box;
	double scale;					//!������Ϣ
	Point pt;						//!�뾶��ע��ֱ����ע��������Բ��
};

//idx, �Ƿ���
using  DimensionPair = std::pair<unsigned, bool>;

#endif