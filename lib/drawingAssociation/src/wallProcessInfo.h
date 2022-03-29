#ifndef __DRAWINGRELATED_WALL_PROCESS_INFO_H__
#define __DRAWINGRELATED_WALL_PROCESS_INFO_H__
#include "line.h"
#include "json/json.h"

class WallLoc
{
public:
	//funcation interface
	/*初始化函数*/
	bool setName(const std::string& name);
	bool setRelativeElevation(const double& elevation);
	bool setThick(const double& thick);
	bool setStartReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis);
	bool setEndReferencePoint(const std::pair<std::string, double>& hAxis, const std::pair<std::string, double>& vAxis);
	
	/*返回函数*/

private:
	typedef struct _ReferencePoint
	{
		std::pair<std::string, double>_referenceHAxis;//参考轴，以及相对距离
		std::pair<std::string, double>_referenceVAxis;//参考轴，以及相对距离

	}ReferencePoint;
private:
	std::string _name;//墙名或墙编号
	double _relativeElevation;//标高
	double _thick;//厚度
	ReferencePoint _startPoint;
	ReferencePoint _endPont;

};

#endif