#include "wallInfo.h"

bool WallInfo::findWallPathJsonValue(const Json::Value& wallJVPath)
{
	
	Json::Value wallLocValue = wallJVPath["floorPlan"];
	if (wallLocValue.isObject())
	{
		auto wallInfoPlanVec = wallLocValue.getMemberNames();
		for (auto it : wallInfoPlanVec)
		{
			//初始化板列表
			_floorVec.push_back(it);
			//具体到某层板数据
			Json::Value somePlanValue = wallLocValue[it];

			//某层可能会有多个图纸描述需要进行数据校验
			if (somePlanValue.isArray())
			{
				std::vector<std::pair<std::string, std::string>>planMapVec;
				for (auto itPlan : somePlanValue)
				{
					std::pair<std::string, std::string>tempDrawing;
					//某一张图纸的柱位置信息内容
					auto fileNameVec = itPlan.getMemberNames();
					if (fileNameVec.size() == 1)
					{
						tempDrawing = std::pair<std::string, std::string>(fileNameVec.front(), itPlan[fileNameVec.front()].asCString());
						planMapVec.push_back(tempDrawing);
					}
					else
					{
						;//错误处理
					}

				}
				_drawingLocMap[it] = planMapVec;
			}
			else
			{
				;//错误处理
			}
		}
	}
	else
	{
		;//错误处理
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
