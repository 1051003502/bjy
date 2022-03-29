#include "wallInfo.h"

bool WallInfo::findWallPathJsonValue(const Json::Value& wallJVPath)
{
	
	Json::Value wallLocValue = wallJVPath["floorPlan"];
	if (wallLocValue.isObject())
	{
		auto wallInfoPlanVec = wallLocValue.getMemberNames();
		for (auto it : wallInfoPlanVec)
		{
			//��ʼ�����б�
			_floorVec.push_back(it);
			//���嵽ĳ�������
			Json::Value somePlanValue = wallLocValue[it];

			//ĳ����ܻ��ж��ͼֽ������Ҫ��������У��
			if (somePlanValue.isArray())
			{
				std::vector<std::pair<std::string, std::string>>planMapVec;
				for (auto itPlan : somePlanValue)
				{
					std::pair<std::string, std::string>tempDrawing;
					//ĳһ��ͼֽ����λ����Ϣ����
					auto fileNameVec = itPlan.getMemberNames();
					if (fileNameVec.size() == 1)
					{
						tempDrawing = std::pair<std::string, std::string>(fileNameVec.front(), itPlan[fileNameVec.front()].asCString());
						planMapVec.push_back(tempDrawing);
					}
					else
					{
						;//������
					}

				}
				_drawingLocMap[it] = planMapVec;
			}
			else
			{
				;//������
			}
		}
	}
	else
	{
		;//������
	}
	return true;
}

bool WallInfo::readWallLocInfor()
{
	for (auto itPlan : _drawingLocMap)
	{
		std::vector<std::pair<std::string, std::vector<WallLoc>>>drawingWallVec;

		for (auto drawing : itPlan.second)
		{
			auto path = setFloorPath(itPlan.first, drawing.first);
			auto field = drawing.second;

			auto wallLocValue = resultRead.returnFieldValue(path, field);


			auto wallLocVec = setWallLocJV(wallLocValue);

			auto drawingWallVecPair = std::pair<std::string, std::vector<WallLoc>>(drawing.first, wallLocVec);
			drawingWallVec.push_back(drawingWallVecPair);
		}
		_drawingWallLocVec[itPlan.first] = drawingWallVec;
	}

	return true;
}

const std::string WallInfo::setFloorPath(const std::string& type, const std::string& field)
{
	return std::string("../projectJsonFile/drawing/floorPlan/" + type + "/" + field);
}

std::vector<WallLoc> WallInfo::setWallLocJV(const Json::Value& wallLocValue)
{

	std::vector<WallLoc> columnVec = resultRead.readWallLocList(wallLocValue);
	return columnVec;
}
