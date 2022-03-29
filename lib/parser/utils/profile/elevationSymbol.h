#pragma once
#include "common.h"
#include "data.h"

class ElevationSymbol
{
public:
	ElevationSymbol() :
		pairSlash(std::make_pair<int, int>(-1, -1)),
		horizontalIdx(-1)
	{}
	bool updateElevationPoint(const std::shared_ptr<Data>& data);

	bool findElevationDescription(const std::shared_ptr<Data>& data);
	bool findHorizontalIdxImp(const std::vector<int>& horizontalIdxVec,
							  const std::shared_ptr<Data>& data,
							  const  double y1,
							  const double y2);
	bool updateSlashPriority(const std::shared_ptr<Data>& data);
	bool findHorizontalIdx(const std::vector<int>& horizontalIdxVec,
						   const std::shared_ptr<Data>& data);

	std::pair<int, int> pairSlash;			//����������б��
	int horizontalIdx;						//ˮƽ��
	Point pt;								//��ߵ�
	std::string elevationDescription;		//��߸߶ȱ���
};