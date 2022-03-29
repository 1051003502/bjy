#include "coordinateSystem.h"

//bool CoordinateSystem::iniAxisMap(const std::map<std::string, std::shared_ptr<Axis>>& _axisMap)
//{
//	this->axisMap = _axisMap;
//	return true;
//}

bool CoordinateSystem::findAxisPathJsonValue(const Json::Value& columnJV)
{
	//������Ϣ·������
	if (columnJV.isMember("floorPlan"))
	{
		_floorVec.push_back("floorPlan");
		Json::Value axisesValue = columnJV["floorPlan"];
		std::vector<std::pair<std::string, std::string>>drawingMap;

		if (axisesValue.isArray())
		{
			for (auto it : axisesValue)
			{
				Json::Value::Members drawingMembers = it.getMemberNames();
				std::string drawingName = drawingMembers.front();
				Json::Value tempValue = it[drawingName];
				drawingMap.push_back(std::pair<std::string, std::string>(drawingName, tempValue.asCString()));

			}
		}
		_drawingLocMap["floorPlan"] = drawingMap;
	}

#if 0
	Json::Value columnLocValue = columnJV["floorPlan"];
	if (columnLocValue.isObject())
	{
		auto columnInfoPlanVec = columnLocValue.getMemberNames();
		for (auto it : columnInfoPlanVec)
		{
			//��ʼ�����б�
			_floorVec.push_back(it);
			//���嵽ĳ�������
			Json::Value somePlanValue = columnLocValue[it];

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
						//tempDrawing[fileNameVec.front()] = itPlan[fileNameVec.front()].asCString();
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
#endif
	return true;
}

bool CoordinateSystem::readAxisCoordinateSystem()
{
	for (auto itPlan : _drawingLocMap)
	{
		std::vector<std::pair<std::string, std::vector<AxisProcessInfo>>>drawingAxisVec;

		for (auto drawing : itPlan.second)
		{

			auto path = setFloorPath(itPlan.first, drawing.first);
			auto field = drawing.second;

			auto aixsValue = _resultRead.returnFieldValue(path, field);

			auto AxisVec = setColumnLocJV(aixsValue);

			auto drawingAxisVecPair = std::pair<std::string, std::vector<AxisProcessInfo>>(drawing.first, AxisVec);
			drawingAxisVec.push_back(drawingAxisVecPair);
		}
		_drawingAxisVec[itPlan.first] = drawingAxisVec;
	}

	return true;
}

bool CoordinateSystem::outputAxisJsonData()
{

	//std::string basePath = constAxisPath();
	mergeAxis();//�ϲ���ͬ����

	checkSameName();//У���Ƿ����ͬ������

	checkAxisDistance();//�������С����ʮ���׵����

	outputAll();//������е���

	outputError();//����������

	return true;
}

std::string CoordinateSystem::constAxisPath()
{
	// TODO: �ڴ˴����� return ���
	std::string basePath = "../projectJsonFile/resultData/axis/";
	return basePath;
}

const std::string CoordinateSystem::setFloorPath(const std::string& type, const std::string& field)
{
	return std::string("../projectJsonFile/drawing/" + type + "/" + field);
}

std::vector<AxisProcessInfo> CoordinateSystem::setColumnLocJV(const Json::Value& axisValue)
{
	std::vector<AxisProcessInfo> axisVec = _resultRead.readAxisList(axisValue);

	return axisVec;
}

bool CoordinateSystem::mergeAxis()
{
	for (auto floor : _floorVec)//��ͬ�Ĳ�
	{
		auto fileMapIte = _drawingAxisVec.find(floor);
		if (fileMapIte != _drawingAxisVec.end())
		{
			for (auto drawing : fileMapIte->second)//ͬһ��Ĳ�ͬͼֽ
			{
				auto tempAxises = drawing.second;
				
				checkAxis(tempAxises);
			}
		}
	}
	
	return false;
}

bool CoordinateSystem::checkAxis(const std::vector<AxisProcessInfo>& drawingAxisVec)
{
	for (auto it : drawingAxisVec)
	{
		std::vector<SingleAxisInfo> haxises = _goalAxisSyetem.getHAxises();
		std::vector<SingleAxisInfo> vaxises = _goalAxisSyetem.getVAxises();

		checkHAxis(it.getHAxises(), haxises);
		checkVAxis(it.getVAxises(), vaxises);

		_goalAxisSyetem.setHAxises(haxises);
		_goalAxisSyetem.setVAxises(vaxises);
	}

	

	return true;
}

bool CoordinateSystem::checkHAxis(const std::vector<SingleAxisInfo>& hAxisVec, 
	std::vector<SingleAxisInfo>& goalHAxisVec)
{
	if (goalHAxisVec.empty())
	{
		goalHAxisVec.insert(goalHAxisVec.end(), hAxisVec.begin(), hAxisVec.end());
	}
	else
	{
		std::vector<SingleAxisInfo>nAddAxisMap;//��Ҫ��ӵ�����Ϣ
		std::pair<std::string, double >tempReferenceAxis;//��ʱ�洢��ǰͼֽ�еĲ������Լ�λ��

		for (auto hAxis : hAxisVec)
		{
			auto bHline = SingleAxisInfo::findAxis(goalHAxisVec, hAxis);
			if (tempReferenceAxis.first.empty() && bHline)
			{
				//referenceAxis[it.first].second = vAxis.getName();
				tempReferenceAxis.first = hAxis.getName();;
				tempReferenceAxis.second = hAxis.getDistance();
			}
			else if (!bHline)//!��ȫ�������в����ڵ�����ӽ���ҪnAddAxisMap��
			{
				nAddAxisMap.push_back(hAxis);
			}
		}
		if (tempReferenceAxis.first.empty())
		{
			;//TODO��Ҫ����д����־
		}
		for (auto vAxis : nAddAxisMap)
		{
			auto distance = vAxis.getDistance() - tempReferenceAxis.second;
			goalHAxisVec.push_back(vAxis);
		}
	}
	return true;
}

bool CoordinateSystem::checkVAxis(const std::vector<SingleAxisInfo>& vAxisVec, 
	std::vector<SingleAxisInfo>& goalVAxisVec)
{
	if (goalVAxisVec.empty())
	{
		goalVAxisVec.insert(goalVAxisVec.end(), vAxisVec.begin(), vAxisVec.end());
	}
	else
	{
		std::vector<SingleAxisInfo>nAddAxisMap;//��Ҫ��ӵ�����Ϣ
		std::pair<std::string, double >tempReferenceAxis;//��ʱ�洢��ǰͼֽ�еĲ������Լ�λ��

		for (auto vAxis : vAxisVec)
		{
			auto bVline = SingleAxisInfo::findAxis(goalVAxisVec, vAxis);
			if (tempReferenceAxis.first.empty() && bVline)
			{
				//referenceAxis[it.first].second = vAxis.getName();
				tempReferenceAxis.first = vAxis.getName();;
				tempReferenceAxis.second = vAxis.getDistance();
			}
			else if (!bVline)//!��ȫ�������в����ڵ�����ӽ���ҪnAddAxisMap��
			{
				nAddAxisMap.push_back(vAxis);
			}
		}
		if (tempReferenceAxis.first.empty())
		{
			;//TODO��Ҫ����д����־
		}
		for (auto vAxis : nAddAxisMap)
		{
			auto distance = vAxis.getDistance() - tempReferenceAxis.second;
			goalVAxisVec.push_back(vAxis);
		}
	}
	return true;
}

bool CoordinateSystem::checkSameName()
{
	auto hAxisVec = _goalAxisSyetem.getHAxises();
	auto vAxisVec = _goalAxisSyetem.getVAxises();
	std::map<std::string, int>haxisMap;
	std::map<std::string, int>vaxisMap;
	for (auto it : hAxisVec)
	{
		haxisMap[it.getName()]++;
	}
	for (auto it : vAxisVec)
	{
		vaxisMap[it.getName()]++;
	}

	for (auto it : haxisMap)
	{
		if (it.second > 1)
		{
			_sameNameError.push_back(it.first);
		}
	}
	for (auto it : vaxisMap)
	{
		if (it.second > 1)
		{
			_sameNameError.push_back(it.first);
		}
	}
	return true;
}

bool CoordinateSystem::checkAxisDistance()
{
	auto hAxisVec = _goalAxisSyetem.getHAxises();
	auto vAxisVec = _goalAxisSyetem.getVAxises();
	if (!hAxisVec.empty())
	{
		for (int i = 0; i < hAxisVec.size()-1; ++i)
		{
			for (int j = i + 1; j < hAxisVec.size(); ++j)
			{
				if (hAxisVec[j].getDistance() - hAxisVec[i].getDistance() < 50)
				{
					std::vector<std::string>errorName;
					errorName.push_back(hAxisVec[i].getName());
					errorName.push_back(hAxisVec[j].getName());
					_lessThanError.push_back(errorName);
				}
				else
				{
					break;
				}
			}
		}
	}
	if (!vAxisVec.empty())
	{
		for (int i = 0; i < vAxisVec.size() - 1; ++i)
		{
			for (int j = i + 1; j < vAxisVec.size(); ++j)
			{
				if (vAxisVec[j].getDistance() - vAxisVec[i].getDistance() < 50)
				{
					std::vector<std::string>errorName;
					errorName.push_back(vAxisVec[i].getName());
					errorName.push_back(vAxisVec[j].getName());
					_lessThanError.push_back(errorName);
				}
				else
				{
					break;
				}
			}
		}
	}
	return true;
}

bool CoordinateSystem::outputAll()
{
	
	auto axisValue = _resultWrite.AxisesW(_goalAxisSyetem);
	auto path = constAxisPath();
	_resultWrite.creatJsonResultFile(path, "coordinateSystem", axisValue);

	return true;
}

bool CoordinateSystem::outputError()
{
	Json::Value impeachAxis, sameName, lessThan;

	for (auto it : _sameNameError)
	{
		sameName.append(it);
	}
	for (auto it : _lessThanError)
	{
		Json::Value less;
		for (auto it1 : it)
		{
			less.append(it1);
		}
		lessThan.append(less);
	}
	impeachAxis["sameName"] = sameName;
	impeachAxis["lessThan"] = lessThan;

	auto path = constAxisPath();
	_resultWrite.creatJsonResultFile(path, "impeachAxis", impeachAxis);

	return true;
}

bool CoordinateSystem::iniGlobalCoordinateSystem()
{
	for (auto it : axisMap)
	{
		//!�����������ʼ��
		if (0 == vlines_cooMap.size())
		{
			for (auto vAixs : it.second->vlines_coo)
			{
				auto vAxisName = vAixs.first.first.second;
				//!��ʼ����������
				if (referenceAxisY.empty())
				{
					referenceAxisY = vAxisName;
					referenceAxis[it.first].second = vAxisName;

				}
				vlines_cooMap[vAxisName] = vAixs.second;
			}
		}
		else
		{
			std::map<std::string,double>nAddAxisMap;//need add axis name
			std::pair<std::string,double >tempReferenceAxis;//��ʱ�洢��ǰͼֽ�еĲ������Լ�λ��
			for (auto vAxis : it.second->vlines_coo)
			{
				auto vAxisName = vAxis.first.first.second;
				auto vline_cooIte = vlines_cooMap.find(vAxisName);
				if (tempReferenceAxis.first.empty() && vline_cooIte != vlines_cooMap.end())
				{
					referenceAxis[it.first].second = vAxisName;
					tempReferenceAxis.first = vAxisName;
					tempReferenceAxis.second = vAxis.second;
				}
				else if(vline_cooIte == vlines_cooMap.end())//!��ȫ�������в����ڵ�����ӽ���ҪnAddAxisMap��
				{
					nAddAxisMap[vAxisName] = vAxis.second;
				}
				
			}
			if (tempReferenceAxis.first.empty())
			{
				;//TODO��Ҫ����д����־
			}
			for (auto vAxis : nAddAxisMap)
			{
				auto distance = vAxis.second - tempReferenceAxis.second;
				vlines_cooMap[vAxis.first] = vlines_cooMap[tempReferenceAxis.first] + distance;
			}
		}
		//!�����ĺ����ʼ��
		if (0 == hlines_cooMap.size())
		{
			for (auto hAxis : it.second->hlines_coo)
			{
				auto hAxisName = hAxis.first.first.second;
				//!��ʼ�����պ���
				if (referenceAxisX.empty())
				{
					referenceAxisX = hAxisName;
					referenceAxis[it.first].first = hAxisName;
				}
				hlines_cooMap[hAxisName] = hAxis.second;
			}
		}
		else
		{
			std::map<std::string, double>nAddAxisMap;//need add axis name
			std::pair<std::string, double >tempReferenceAxis;//��ʱ�洢��ǰͼֽ�еĲ������Լ�λ��
			for (auto hAxis : it.second->hlines_coo)
			{
				auto hAxisName = hAxis.first.first.second;
				auto hline_cooIte = hlines_cooMap.find(hAxisName);
				if (tempReferenceAxis.first.empty() && hline_cooIte != hlines_cooMap.end())
				{
					referenceAxis[it.first].first = hAxisName;
					tempReferenceAxis.first = hAxisName;
					tempReferenceAxis.second = hAxis.second;
				}
				else if (hline_cooIte == hlines_cooMap.end())//!��ȫ�������в����ڵ�����ӽ���ҪnAddAxisMap��
				{
					nAddAxisMap[hAxisName] = hAxis.second;
				}

			}
			if (tempReferenceAxis.first.empty())
			{
				;//TODO��Ҫ����д����־
			}
			for (auto hAxis : nAddAxisMap)
			{
				auto distance = hAxis.second - tempReferenceAxis.second;
				hlines_cooMap[hAxis.first] = hlines_cooMap[tempReferenceAxis.first] + distance;
			}
		}
	}
	return true;
}


