#include "columnInfo.h"

/*******************************/
columnLocationW::columnLocationW(const std::string& name,
							   const PillarDistance& hDistance,
							   const PillarDistance& vDistance, unsigned int Id)
{
	this->name = name;
	this->hDistance = hDistance;
	this->vDistance = vDistance;
	this->columnSectionId = Id;
}
bool ColumnInfo::findColumnPathJsonValue(const Json::Value& columnJV)
{
	//柱位置信息路径解析
	Json::Value columnLocValue = columnJV["floorPlan"];
	if (columnLocValue.isObject())
	{
		auto columnInfoPlanVec = columnLocValue.getMemberNames();
		for (auto it : columnInfoPlanVec)
		{
			//初始化板列表
			_floorVec.push_back(it);
			//具体到某层板数据
			Json::Value somePlanValue = columnLocValue[it];
				
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
						//tempDrawing[fileNameVec.front()] = itPlan[fileNameVec.front()].asCString();
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
	//柱截面信息
	//if (columnInfoClue["section"].isObject())
	//{
	//}
	//else
	//{
	//	;//错误处理
	//}
	//readColumnInfo();
	return true;
}
bool ColumnInfo::outputColumnJsonData()
{
	std::string basePath = "../projectJsonFile/resultData/column/";//柱信息结果文件
	checkSameFloorColumnLoc();//先校验柱位置信息
	outputErrorLocData(basePath);//生成有问题的柱信息文件
	outputColumnLocData(basePath);//柱没有问题的结果文件
	outputColumnSectionData(basePath);//输出截面信息

	return true;
}
bool ColumnInfo::readColumnLocInfo()
{
	for (auto itPlan : _drawingLocMap)
	{
		std::vector<std::pair<std::string, std::vector<ColumnLoc>>>drawingColumnVec;

		for (auto drawing : itPlan.second)
		{
			
			
			auto path = setFloorPath(itPlan.first, drawing.first);
			auto field = drawing.second;
			
			auto columnLocValue = resultRead.returnFieldValue(path, field);

			auto columnLocVec = setColumnLocJV(columnLocValue);
			
			auto drawingColumnVecPair = std::pair<std::string, std::vector<ColumnLoc>>(drawing.first, columnLocVec);
			drawingColumnVec.push_back(drawingColumnVecPair);
		}
		_drawingColumnLocVec[itPlan.first] = drawingColumnVec;
	}

	return true;
}

const std::string ColumnInfo::setFloorPath(const std::string& type, const std::string& field)
{
	return std::string("../projectJsonFile/drawing/floorPlan/" + field);
}

std::vector<ColumnLoc> ColumnInfo::setColumnLocJV(const Json::Value& columnLocValue)
{
	
	std::vector<ColumnLoc> columnVec = resultRead.readCoulmnLocList(columnLocValue);

	return columnVec;
}

bool ColumnInfo::checkSameFloorColumnLoc()
{
	for (auto floorPlan : _drawingColumnLocVec)
	{
		auto num = floorPlan.second.size();
		if (num != 1 && num > 1)
		{
			;
		}
		else
		{
			auto floor = floorPlan.first;
			for (auto column : floorPlan.second.front().second)
			{
				if (column.getName() != "")
				{
					_outputColumnLocVec[floorPlan.first].push_back(column);
				}
				else
				{
					_unkonwNameColumnVec[floorPlan.first].push_back(column);
				}
			}
		}

	}
	return true;
}

bool ColumnInfo::outputErrorLocData(const std::string& basePath)
{
	
	
	for (auto it : _floorVec)
	{
		Json::Value unKonwNameValue, locErrorValue;
		auto unknowName = _unkonwNameColumnVec.find(it);
		auto locError = _notSameColumnLocVec.find(it);
		if (unknowName != _unkonwNameColumnVec.end())
		{
			auto floor = unknowName->first;
			for (auto column : unknowName->second)
			{
				
				unKonwNameValue.append(resultWrite.columnLocResultData(column));
			}
		}
		if (locError != _notSameColumnLocVec.end())
		{
			auto floor = locError->first;
			for (auto column : locError->second)
			{

				locErrorValue.append(resultWrite.columnLocResultData(column));
			}
		}

		auto path = setWriteFile(it);
		Json::Value impeachColumn;
		impeachColumn["unkonwName"] = unKonwNameValue;
		impeachColumn["locError"] = locErrorValue;
		resultWrite.creatJsonResultFile(path, "impeachColumn", impeachColumn);
	}
	return true;
}

bool ColumnInfo::outputColumnLocData(const std::string& basePath)
{

	for (auto it : _outputColumnLocVec)
	{
		Json::Value columnLocVec;
		
		auto path = setWriteFile(it.first);
		for (auto column : it.second)
		{
			columnLocVec.append(resultWrite.columnLocResultData(column));
		}
		//产生结果文件
		resultWrite.creatJsonResultFile(path, "columnLocation", columnLocVec);
	}
	return true;
}

bool ColumnInfo::outputColumnSectionData(const std::string& basePath)
{

	return true;
}

bool ColumnInfo::singleLayerFloor(const std::string& floorPlan, const std::vector<ColumnLoc>& columnLocVec)
{
	for (auto column : columnLocVec)
	{
		if (column.getName() != "")
		{
			_outputColumnLocVec[floorPlan].push_back(column);
		}
		else
		{
			_unkonwNameColumnVec[floorPlan].push_back(column);
		}
	}
	return true;
}

std::string ColumnInfo::setWriteFile(const std::string& floorFile)
{
	std::string writePath = "../projectJsonFile/resultData/column/" + floorFile + "/";
#if _WIN32
	
	if (access(writePath.c_str(),0)==-1)
	{
		CreateDirectory(writePath.c_str(), NULL);
	}
	//CreateDirectory(path.c_str(), NULL);
#else

#endif
	return writePath;
}



bool operator==(const columnLocationW& _column1, const columnLocationW& _column2)
{
	//TODO:需要重载PillarDistance，他们之间的距离应该给一个误差值
	if (_column1.hDistance.first == _column2.hDistance.first &&
		_column1.vDistance.first == _column2.vDistance.first)
	{
		if ((std::abs(_column1.hDistance.second.first - _column2.hDistance.second.first) < 1) &&
			(std::abs(_column1.vDistance.second.first - _column2.vDistance.second.first) < 1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}


