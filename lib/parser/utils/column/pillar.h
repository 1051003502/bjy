#ifndef __PARSER_UTILS_PILLAR_H__
#define __PARSER_UTILS_PILLAR_H__

#include <string>
#include "point.h"
#include "line.h"
#include "box.h"
#include "data.h"
#include "block.h"
#include "axis.h"
#include "data.h"
typedef std::pair<std::string, double> AxisMark;
typedef std::pair<double, double> Distance;		//��һ������Ϊ��||�£��ڶ�����������||��

typedef std::pair<std::string, Distance> PillarDistance;
class Pillar {
public:
	Pillar() 
	{
		this->_angle = 0.0;
	}
	
public:
	Pillar(const std::string& name, const Point& point):_name(name),_point(point),_error(true)
	{
		this->_angle = 0.0;
	}
	Pillar(const std::string& name, const std::vector<Line>& vl, 
		const std::vector<Line>& hl, const std::vector<Line>&sl,
		const std::vector<Line>& dl):
			_name(name), _vlines(vl), _hlines(hl), _slines(sl), _dlines(dl),_error(false)
	{
		this->_angle = 0.0;
		
	}
	
	friend bool IsItTheSamePillar(const Pillar& pillar1, const Pillar& pillar2);
	~Pillar() {}
public:
	
	void SetHsize();
	void SetVsize();
	void SetPillarBox();
	void RestPillarAxis();
	//��ʼ���ӿ�
	bool setVlines(const std::vector<Line>& lineVec);
	bool setHlines(const std::vector<Line>& lineVec);
	bool setSlines(const std::vector<Line>& lineVec);
	bool setDLines(const std::vector<Line>& lineVec);
	bool setHAxis(const std::vector<AxisMark>& hAxis);
	bool setVAxis(const std::vector<AxisMark>& vAxis);
	bool setHAxis(const AxisMark& hAxis);
	bool setVAxis(const AxisMark& vAxis);
	bool setError(const bool& error);
	bool setAngle();
	

	std::vector<Line> getVlines();
	std::vector<Line> getHlines();
	std::vector<Line> getSlines();
	std::vector<Line> getDLines();
	std::vector<AxisMark>getHAxis();
	std::vector<AxisMark>getVAxis();
	std::string getName();
	Point getTestPoint();
	bool getError();
	double getAngle();
	Box getBox();
	PillarDistance getHPDistance();
	PillarDistance getVPDistance();


private://process 
	Point returnLowestPoint();
	std::vector<Line>retrunConnecLines(const std::vector<Line>& lines, const Point& point);
	double returnAngle(const std::vector<Line>& lines);

private:
	std::string _name;
	Point _point;
	std::vector<Line> _vlines;	//���ӵ�����߽�
	std::vector<Line> _hlines;	//���ӵĺ���߽�
	std::vector<Line> _slines;	//����б��߽�
	std::vector<Line> _dlines;	//��ע��
	std::vector<AxisMark>_haxis;	//��������ĺ���
	std::vector<AxisMark>_vaxis;	//�������������
	PillarDistance _hdistance;//�������������
	PillarDistance _vdistance;//��������������
	double _angle;//��ת��
	unsigned int _section_id;
	std::string _columnLayer;
	Box _box;
	bool _error;
};
#endif