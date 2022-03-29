#ifndef __DRAWINGRELATED_WALL_PROCESS_INFO_H__
#define __DRAWINGRELATED_WALL_PROCESS_INFO_H__
#include "line.h"
#include "json/json.h"

class WallLoc
{
public:
	//funcation interface
	/*��ʼ������*/
	bool setName(const std::string& name);
	bool setRelativeElevation(const double& elevation);
	bool setThick(const double& thick);
	bool setStartReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis);
	bool setEndReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis);
	
	/*���غ���*/

private:
	typedef struct _ReferencePoint
	{
		std::pair<std::string, double>_referenceHAxis;//�ο��ᣬ�Լ���Ծ���
		std::pair<std::string, double>_referenceVAxis;//�ο��ᣬ�Լ���Ծ���

	}ReferencePoint;
private:
	std::string _name;//ǽ����ǽ���
	double _relativeElevation;//���
	double _thick;//���
	ReferencePoint _startPoint;
	ReferencePoint _endPont;

};

#endif