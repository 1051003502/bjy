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
	//��ȡ������ע��
	bool buildAxisSymbol(std::vector<Axis::AxisSymbol> &pairs, Data &data);
	//������ʼ��������������Ϣ
	bool buildAxisLines(std::vector<Axis::AxisSymbol> &pairs, Data&data);
	//�Գ�����ʼ����������Ϣ��������
	bool sortAxisLines();
	//�ϲ���ͬ������
	std::vector<Axis::AxisLine> combineAxisHLines(const std::vector<Axis::AxisLine> &hlines);
	std::vector<Axis::AxisLine> combineAxisVLines(const std::vector<Axis::AxisLine> &vlines);
	//���ӶϿ�������
	bool linkAxisLines(LineData &lineData);
	//��������
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
	//��ʼ������
	bool setAxis(std::vector<std::shared_ptr<Axis>> &axises);
	bool optimizationAxis(std::vector<std::shared_ptr<Axis>>&axisVec);
public:
	std::vector<Axis::AxisLine> m_axisHLines;
	std::vector<Axis::AxisLine> m_axisVLines;

};
#endif