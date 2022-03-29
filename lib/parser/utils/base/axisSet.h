#ifndef __PARSER_UTILS_AXISSET_H__
#define __PARSER_UTILS_AXISSET_H__

#include "axis.h"
#include "data.h"
class AxisSet
{
public:
	AxisSet() {}
	bool axis(Data&data/*, std::vector<Axis::AxisLine>&vAxisLine,
		std::vector<Axis::AxisLine>&hAxisLine*/
		/*std::vector<std::shared_ptr<Axis>> &axises*/);
	//获取轴网的注释
	bool buildAxisSymbol(std::vector<Axis::AxisSymbol> &pairs, Data &data);
	//初步初始化轴网的轴线信息
	bool buildAxisLines(std::vector<Axis::AxisSymbol> &pairs, Data&data);
	//对初步初始化的轴线信息进行排序
	bool sortAxisLines();
	//合并相同的轴线
	std::vector<Axis::AxisLine> combineAxisHLines(const std::vector<Axis::AxisLine> &hlines);
	std::vector<Axis::AxisLine> combineAxisVLines(const std::vector<Axis::AxisLine> &vlines);
	//连接断开的轴线
	bool linkAxisLines(LineData &lineData);
	//创建轴网
	bool creatAxis(const std::vector<Axis::AxisLine>& _axisHLines,
		const std::vector<Axis::AxisLine>& _axisVLines,
		std::vector<std::shared_ptr<Axis>>& axises);
	std::shared_ptr<Axis> buildAxisFrom(const std::vector<Axis::AxisLine>& _axisHLines,
		const std::vector<Axis::AxisLine>& _axisVLines,
		int idx, const std::vector<bool> &map, bool v, 
		std::vector<bool> &mapv, std::vector<bool> &maph);
	bool findCrossedAxis(int idx, int vl, int hl, std::vector<int> &vls, std::vector<int> &hls,
		const std::vector<bool> &map, bool v,
		std::vector<bool> &mv, std::vector<bool> &mh);
	//初始化轴网
	bool setAxis(std::vector<std::shared_ptr<Axis>> &axises);
	bool optimizationAxis(std::vector<std::shared_ptr<Axis>>&axisVec);
public:
	std::vector<Axis::AxisLine> m_axisHLines;
	std::vector<Axis::AxisLine> m_axisVLines;

};
#endif