#include "RWJsonFile.h"

#if 0
Json::Value readParameterFile(const std::string& parameterFile)
{
	std::ifstream of1;
	of1.open(parameterFile, std::ios::in);
	std::string strInfo;
	of1 >> strInfo;
	Json::Value body;
	Json::Reader reader;
	reader.parse(strInfo, body);

	return body;
}

Json::Value readFloorPlan(const Json::Value& parameterFile)
{
	Json::Value floorPlan;
	floorPlan = parameterFile["floorPlan"];
	return floorPlan;
}

Json::Value readProfileFile(const Json::Value& parameterFile)
{
	return parameterFile["profile"];
}

Json::Value readDetailFile(const Json::Value& parameterFile)
{
	return parameterFile["detail"];
}

Json::Value pointValueW(const Point& point)
{
	Json::Value _pointValue;
	_pointValue["X"] = point.x;
	_pointValue["Y"] = point.y;
	return _pointValue;
}

Point pointValueR(const Json::Value& point)
{
	Point tempPoint;
	tempPoint.x = point["X"].asDouble();
	tempPoint.y = point["Y"].asDouble();
	return tempPoint;
}

Json::Value pointValueW(const Point& referencePoint, const Point& point)
{
	Transform transform;
	transform.setTransform(referencePoint);
	auto tempPoint = transform.referencePoint(point);
	Json::Value _pointValue;
	_pointValue = pointValueW(tempPoint);
	return _pointValue;
}

Json::Value lineValueW(const Line& line)
{
	Json::Value _lineValue;
	_lineValue["startP"] = pointValueW(line.s);
	_lineValue["endP"] = pointValueW(line.e);
	_lineValue["side"] = line.isSide();
	_lineValue["layer"] = line.getLayer();

	return _lineValue;
}

Line lineValueR(const Json::Value& line)
{
	Line tempLine;
	tempLine.s = pointValueR(line["startP"]);
	tempLine.e = pointValueR(line["endP"]);
	tempLine.side = line["side"].asBool();
	tempLine.layer = line["layer"].asString();

	return tempLine;
}

std::vector<Line> readLineList(const Json::Value& lineVec)
{
	std::vector<Line>_lineVec;
	if (lineVec.isArray())
	{
		for (auto it : lineVec)
		{
			_lineVec.push_back(lineValueR(it));
		}
	}
	return _lineVec;
}

Json::Value lineValueW(const Point& referencePoint, const Line& line)
{
	
	Json::Value _lineValue;
	_lineValue["startP"] = pointValueW(referencePoint, line.s);
	_lineValue["endP"] = pointValueW(referencePoint, line.e);
	_lineValue["side"] = line.isSide();
	_lineValue["layer"] = line.getLayer();

	return _lineValue;
}

Json::Value numberListW(const std::vector<int>& numberListW)
{
	Json::Value numberListValue;
	for (auto it : numberListW)
	{
		numberListValue.append(it);
	}
	return numberListValue;
}

Json::Value circleValueW(const Circle& circle)
{
	Json::Value _circleValue;
	_circleValue["r"] = circle.r;
	_circleValue["center"] = pointValueW(circle);
	return _circleValue;
}

Json::Value lineDataValueW(LineData& lineData)
{
	Json::Value _lineData;
	Json::Value lines;
	Json::Value hLineIndexVec;
	Json::Value vLineIndexVec;
	Json::Value sLineIndexVec;
	for (auto it : lineData.lines())
	{
		lines.append(lineValueW(it));
	}
	//横线索引
	hLineIndexVec = numberListW(lineData.hLinesIndices());
	//纵线索引
	vLineIndexVec = numberListW(lineData.vLinesIndices());
	//斜线索引
	sLineIndexVec = numberListW(lineData.sLinesIndices());

	_lineData["lines"] = lines;
	_lineData["hLineIndexVec"] = hLineIndexVec;
	_lineData["vLineIndexVec"] = vLineIndexVec;
	_lineData["sLineIndexVec"] = sLineIndexVec;

	return _lineData;
}

Json::Value circleDataValueW(CirclesData& circleData)
{
	Json::Value _circleData;
	for (auto it : circleData.circles())
	{
		_circleData.append(circleValueW(it));
	}
	return _circleData;
}

Json::Value cornerValueW(const Corner& corner)
{
	Json::Value _cornerValue;
	_cornerValue["l1"] = corner.l1;
	_cornerValue["l2"] = corner.l2;
	_cornerValue["cornerPoint"] = pointValueW(corner);

	return _cornerValue;
}

Json::Value cornerDataValueW(const CornerData& cornerData)
{
	Json::Value _cornerData;
	
	for (auto it : cornerData.corners())
	{
		Json::Value _cornerValue;
		Json::Value corners;
		_cornerValue["key"] = it.first;
		for (auto corner : it.second)
		{
			corners.append(cornerValueW(corner));
		}
		_cornerValue["corners"] = corners;
		_cornerData.append(_cornerValue);
	}
	return _cornerData;
}

Json::Value endPointValueW(const Endpoint& endPoint)
{
	Json::Value _endPoint;
	_endPoint["lineIndex"] = endPoint.index;
	_endPoint["point"] = pointValueW(endPoint);

	return _endPoint;
}

Json::Value endPointDataValueW(EndpointData& endPointData)
{
	Json::Value _endPointData;
	for (auto it : endPointData.endpoints())
	{
		_endPointData.append(endPointValueW(it));
	}
	return _endPointData;
}

Json::Value textPointValueW(const Point& point)
{
	Json::Value textPoint;
	auto  textEntity = std::static_pointer_cast<DRW_Text>(point.entity);
	textPoint["text"] = textEntity->text;
	textPoint["angle"] = textEntity->angle;
	textPoint["point"] = pointValueW(point);
	return textPoint;
}

Json::Value textPointDataValueW(TextPointData& textPointData)
{
	Json::Value _textPointData;
	for (auto it : textPointData.textpoints())
	{
		Json::Value textPoint;
		textPoint = textPointValueW(it);
		_textPointData.append(textPoint);
	}
	return _textPointData;
}

Json::Value lineLayerDataValueW(LineLayerData& lineLayerData)
{
	Json::Value lineLayer;
	for (auto layer : lineLayerData.lineLayerMapVec())
	{
		Json::Value indexVec;
		for (auto it : layer.second)
		{
			indexVec.append(it);
		}
		lineLayer[layer.first] = indexVec;
	}
	return lineLayer;
}

Json::Value axisLineValueW(const Axis::AxisLine& axisLine)
{
	Json::Value _axisLine;

	Json::Value axisSymbol;

	axisSymbol["axisSymbol"]= circleValueW(axisLine.first.first);
	axisSymbol["name"] = axisLine.first.second;
	_axisLine["axisSymbol"] = axisSymbol;
	_axisLine["line"] = lineValueW(axisLine.second);
	_axisLine["angle"] = 0.0;
	return _axisLine;
}

Json::Value axisLineDataValueW(AxisLineData& axisLineData)
{
	Json::Value _axisLineData;
	//横轴
	auto axisHLines = axisLineData.axisHLines();
	if (axisHLines.empty())
	{
		_axisLineData["axisHLines"] = "";
	}
	else
	{
		for (auto it : axisLineData.axisHLines())
		{
			_axisLineData["axisHLines"].append(axisLineValueW(it));
		}
	}
	//纵轴
	auto axisVLines = axisLineData.axisVLines();
	if (axisVLines.empty())
	{
		_axisLineData["axisVLines"] = "";
	}
	else
	{
		for (auto it : axisLineData.axisVLines())
		{
			_axisLineData["axisVLines"].append(axisLineValueW(it));
		}
	}
	//斜轴
	auto axisSLines = axisLineData.axisSLines();
	if (axisVLines.empty())
	{
		_axisLineData["axisSLines"] = "";
	}
	else
	{
		for (auto it : axisLineData.axisSLines())
		{
			_axisLineData["axisSLines"].append(axisLineValueW(it));
		}
	}
	return _axisLineData;
}

Json::Value axisesValueW(const Axis& _axis)
{
	Json::Value axisValue;
	if (!_axis.hlines_coo.empty() && !_axis.vlines_coo.empty())
	{
		Json::Value reference;
		reference["referenceH"] = _axis.referenceAxisX.front().first.first.second;
		reference["referenceV"] = _axis.referenceAxisY.front().first.first.second;
		axisValue["reference"] = reference;
		Json::Value axisH, axisV;
		for (auto it : _axis.hlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisH.append(axis);
		}
		for (auto it : _axis.vlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisV.append(axis);
		}
		axisValue["axisH"] = axisH;
		axisValue["axisV"] = axisV;
	}
	
	return axisValue;
}

Json::Value columnLocationW(const std::shared_ptr<Pillar>& _spPillar)
{
	Json::Value pillar;
	Json::Value border;
	Json::Value referenceAxis;
	Point referencePoint(_spPillar->vaxis.front().second, _spPillar->haxis.front().second);

	//name
	pillar["name"] = _spPillar->name;
	//border
	for (auto it : _spPillar->hlines)
	{
		border["hLines"].append(lineValueW(referencePoint, it));
	}
	for (auto it : _spPillar->vlines)
	{
		border["vLines"].append(lineValueW(referencePoint, it));
	}
	for (auto it : _spPillar->slines)
	{
		border["sLines"].append(lineValueW(referencePoint, it));
	}
	//reference axis
	Json::Value haxis, vaxis;
	haxis["name"] = _spPillar->haxis.front().first;
	haxis["distance"] = _spPillar->hdistance.second.first;

	vaxis["name"] = _spPillar->vaxis.front().first;
	vaxis["distance"] = _spPillar->vdistance.second.first;
	
	
	
	pillar["border"] = border;
	pillar["referenceHAxis"] = haxis;
	pillar["referenceVAxis"] = vaxis;
	pillar["angle"] = 0.0;
	return pillar;
}

std::shared_ptr<Columnloc> columnLocR(const Json::Value& column)
{
	std::shared_ptr<Columnloc>spColumn(new Columnloc);
	Json::Value border = column["border"];
	//读入柱边界
	spColumn->setHlines(readLineList(border["hLines"]));
	spColumn->setVlines(readLineList(border["vLines"]));
	spColumn->setSlines(readLineList(border["SLines"]));
	//读入柱的参照轴
	Json::Value referenceHAxis, referenceVAxis;
	referenceHAxis = column["referenceHAxis"];
	spColumn->setReferenceH(referenceHAxis["name"].asString(), referenceHAxis["distance"].asDouble());
	referenceVAxis = column["referenceVAxis"];
	spColumn->setReferenceV(referenceVAxis["name"].asString(), referenceVAxis["distance"].asDouble());

	return spColumn;
}

std::vector<std::shared_ptr<Columnloc>> columnLocationR(const Json::Value& columnVec)
{
	std::vector<std::shared_ptr<Columnloc>>spColumnVec;
	if (columnVec.isArray())
	{
		for (auto it : columnVec)
		{
			spColumnVec.push_back(columnLocR(it));
		}
	}
	return spColumnVec;
}
#endif


Json::Value RWBaseEntity::pointValueW(const Point& point)
{
	Json::Value _pointValue;
	_pointValue["X"] = point.x;
	_pointValue["Y"] = point.y;
	return _pointValue;
}

Point RWBaseEntity::pointValueR(const Json::Value& point)
{
	Point tempPoint;
	tempPoint.x = point["X"].asDouble();
	tempPoint.y = point["Y"].asDouble();
	return tempPoint;
}

Json::Value RWBaseEntity::pointValueW(const Point& referencePoint, const Point& point)
{
	Transform transform;
	transform.setTransform(referencePoint);
	auto tempPoint = transform.referencePoint(point);
	Json::Value _pointValue;
	_pointValue = pointValueW(tempPoint);
	return _pointValue;
}

Json::Value RWBaseEntity::lineValueW(const Line& line)
{
	Json::Value _lineValue;
	_lineValue["startP"] = pointValueW(line.s);
	_lineValue["endP"] = pointValueW(line.e);
	_lineValue["side"] = line.isSide();
	_lineValue["layer"] = line.getLayer();

	return _lineValue;
}

Json::Value RWBaseEntity::lineResultValueW(const Line& line)
{

	Json::Value _lineValue;
	_lineValue["startP"] = pointValueW(line.s);
	_lineValue["endP"] = pointValueW(line.e);
	//_lineValue["side"] = line.isSide();
	//_lineValue["layer"] = line.getLayer();

	return _lineValue;
}

Line RWBaseEntity::lineValueR(const Json::Value& line)
{
	Line tempLine;
	tempLine.s = pointValueR(line["startP"]);
	tempLine.e = pointValueR(line["endP"]);
	tempLine.side = line["side"].asBool();
	tempLine.layer = line["layer"].asString();

	return tempLine;
}

Json::Value RWBaseEntity::lineValueW(const Point& referencePoint, const Line& line)
{
	Json::Value _lineValue;
	_lineValue["startP"] = pointValueW(referencePoint, line.s);
	_lineValue["endP"] = pointValueW(referencePoint, line.e);
	_lineValue["side"] = line.isSide();
	_lineValue["layer"] = line.getLayer();

	return _lineValue;
}

std::vector<Line> RWBaseEntity::readLineList(const Json::Value& lineVec)
{
	std::vector<Line>_lineVec;
	if (lineVec.isArray())
	{
		for (auto it : lineVec)
		{
			_lineVec.push_back(lineValueR(it));
		}
	}
	return _lineVec;
}



Json::Value ReadTypeFile::readParameterFile(const std::string& parameterFile)
{
	std::ifstream of1;
	of1.open(parameterFile, std::ios::in);
	std::string strInfo;
	while (!of1.eof()) {
		std::string str;
		std::getline(of1, str);
		strInfo = strInfo + str;
	}
	Json::Value body;
	Json::Reader reader;
	reader.parse(strInfo, body);

	return body;
}

std::map<std::string, std::vector<std::string>> ReadTypeFile::parserParameterFile(const Json::Value& parameterValue)
{
	std::map<std::string, std::vector<std::string>>drawingType;
	if (parameterValue.isObject())
	{
		//读出目录的图纸路径
		if (parameterValue.isMember("mulu"))
		{
			std::vector<std::string>muluVec;
			Json::Value tempValue = parameterValue["mulu"];
			if (tempValue.isArray())
			{
				for (auto it : tempValue)
				{
					muluVec.push_back(it.asString());
				}
			}
			drawingType["mulu"] = muluVec;
		}
		//读出详图类型以及路径
		if (parameterValue.isMember("detail"))
		{
			std::vector<std::string>detailVec;
			Json::Value detail = parameterValue["detail"];
			if (detail.isMember("detailType"))
			{
				Json::Value detailValue = detail["detailType"];
				Json::Value::Members detailType = detailValue.getMemberNames();
				for (auto it = detailType.begin(); it != detailType.end(); ++it)
				{
					std::string parameter;
					Json::Value tempValue = detailValue[*it];
					parameter = *it;
					if (tempValue.isArray())
					{
						for (auto it1 : tempValue)
						{
							std::string tempStr = parameter + "|" + it1.asString();;
							detailVec.push_back(tempStr);
						}
					}
				}
				drawingType["detail"] = detailVec;
			}
		}
		//读出板的类型已经图纸路径
		if (parameterValue.isMember("floorPlan"))
		{
			Json::Value floorValue = parameterValue["floorPlan"];
			Json::Value::Members floorMembers = floorValue.getMemberNames();
			std::vector<std::string> floorParameterVec;
			for (auto floorIte = floorMembers.begin(); floorIte != floorMembers.end(); ++floorIte)
			{
				std::string floorParameter1;
				std::string floorName = *floorIte;
				Json::Value planValue = floorValue[floorName];
				floorParameter1 = floorName;
				if (planValue.isObject())
				{
					Json::Value::Members planMembers = planValue.getMemberNames();
					for (auto plan = planMembers.begin(); plan != planMembers.end(); ++plan)
					{
						Json::Value tempValue = planValue[*plan];
						std::string floorParameter2 = floorParameter1 + "|" + *plan;
						if (tempValue.isArray())
						{
							for (auto it : tempValue)
							{
								std::string floorParameter3 = floorParameter2 + "|" + it.asString();
								floorParameterVec.push_back(floorParameter3);
							}
						}
					}
				}
			}
			drawingType["floorPlan"] = floorParameterVec;
		}
		//读出剖面图类型已经路径
		if (parameterValue.isMember("profile"))
		{
			Json::Value profileValue = parameterValue["profile"];
			std::vector<std::string>profileVec;
			//横剖
			if (profileValue.isMember("hProfile"))
			{
				Json::Value hProfile=profileValue["hProfile"];
				Json::Value::Members hProfileMembers = hProfile.getMemberNames();
				std::string parameter1 = "hProfile";
				for (auto hIte = hProfileMembers.begin(); hIte != hProfileMembers.end(); ++hIte)
				{
					std::string profileStr = *hIte;
					Json::Value profileValue = hProfile[profileStr];
					std::string parameter2 = parameter1 + "|" + profileStr;
					if (profileValue.isArray())
					{
						for (auto it : profileValue)
						{
							std::string parameter3 = parameter2 + "|" + it.asString();
							profileVec.push_back(parameter3);
						}
					}
				}
			}
			//纵剖
			if (profileValue.isMember("vProfile"))
			{
				Json::Value hProfile = profileValue["vProfile"];
				Json::Value::Members hProfileMembers = hProfile.getMemberNames();
				std::string parameter1 = "vProfile";
				for (auto hIte = hProfileMembers.begin(); hIte != hProfileMembers.end(); ++hIte)
				{
					std::string profileStr = *hIte;
					Json::Value profileValue = hProfile[profileStr];
					std::string parameter2 = parameter1 + "|" + profileStr;
					if (profileValue.isArray())
					{
						for (auto it : profileValue)
						{
							std::string parameter3 = parameter2 + "|" + it.asString();
							profileVec.push_back(parameter3);
						}
					}
				}
			}
			//不清楚如何剖
			if (profileValue.isMember("notClear"))
			{
				Json::Value hProfile = profileValue["notClear"];
				Json::Value::Members hProfileMembers = hProfile.getMemberNames();
				std::string parameter1 = "notClear";
				for (auto hIte = hProfileMembers.begin(); hIte != hProfileMembers.end(); ++hIte)
				{
					std::string profileStr = *hIte;
					Json::Value profileValue = hProfile[profileStr];
					std::string parameter2 = parameter1 + "|" + profileStr;
					if (profileValue.isArray())
					{
						for (auto it : profileValue)
						{
							std::string parameter3 = parameter2 + "|" + it.asString();
							profileVec.push_back(parameter3);
						}
					}
				}
			}
			drawingType["profile"] = profileVec;
		}
	}

	return drawingType;
}

Json::Value ReadTypeFile::readFloorPlan(const Json::Value& parameterFile)
{
	return parameterFile["floorPlan"];
}

Json::Value ReadTypeFile::readProfileFile(const Json::Value& parameterFile)
{
	return parameterFile["profile"];
}

Json::Value ReadTypeFile::readDetailFile(const Json::Value& parameterFile)
{
	return parameterFile["detail"];
}

bool ResultWrite::creatJsonResultFile(const std::string& path, const std::string& fileName, const Json::Value& value)
{
	std::string writePath = path;
	bool mark = true;
#if _WIN32

	
	if (access(writePath.c_str(),0) == -1)
	{
		CreateDirectory(path.c_str(), NULL);
	}
#else

	if (access(writePath.c_str(), 0) == -1)
	{
		mkdir(path.c_str(), S_IRWXU);
	}
#endif

	auto jsonPath = path + fileName + ".json";
	Json::FastWriter writer;
	std::fstream of(jsonPath, std::ios::out);
	of << writer.write(value);
	of.close();

	return true;
}

Json::Value ResultWrite::axisesValueW(const Axis& _axis)
{
	Json::Value axisValue;
	if (!_axis.hlines_coo.empty() && !_axis.vlines_coo.empty())
	{
		Json::Value reference;
		reference["referenceH"] = _axis.referenceAxisX.front().first.first.second;
		reference["referenceV"] = _axis.referenceAxisY.front().first.first.second;
		axisValue["reference"] = reference;
		Json::Value axisH, axisV;
		for (auto it : _axis.hlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisH.append(axis);
		}
		for (auto it : _axis.vlines_coo)
		{
			Json::Value axis;
			axis["name"] = it.first.first.second;
			axis["distance"] = it.second;
			axisV.append(axis);
		}
		axisValue["axisH"] = axisH;
		axisValue["axisV"] = axisV;
	}

	return axisValue;
}

Json::Value ResultWrite::columnLocationW(const std::shared_ptr<Pillar>& _spPillar)
{
	Json::Value pillar;
	Json::Value border;
	Json::Value referenceAxis;
	Point referencePoint(_spPillar->getVAxis().front().second, _spPillar->getHAxis().front().second);

	//name
	pillar["name"] = _spPillar->getName();
	//border
	for (auto it : _spPillar->getHlines())
	{
		border["hLines"].append(_rwBaseEntity.lineValueW(referencePoint, it));
	}
	for (auto it : _spPillar->getVlines())
	{
		border["vLines"].append(_rwBaseEntity.lineValueW(referencePoint, it));
	}
	for (auto it : _spPillar->getSlines())
	{
		border["sLines"].append(_rwBaseEntity.lineValueW(referencePoint, it));
	}
	//reference axis
	Json::Value haxis, vaxis;
	haxis["name"] = _spPillar->getHAxis().front().first;
	haxis["distance"] = _spPillar->getHPDistance().second.first;

	vaxis["name"] = _spPillar->getVAxis().front().first;
	vaxis["distance"] = _spPillar->getVPDistance().second.first;



	pillar["border"] = border;
	pillar["referenceHAxis"] = haxis;
	pillar["referenceVAxis"] = vaxis;
	pillar["angle"] = 0.0;
	return pillar;
}

Json::Value ResultWrite::AxisesW(AxisProcessInfo axises)
{
	Json::Value axisValue;
	Json::Value hAxises, vAxises;
	for (auto hAxis : axises.getHAxises())
	{
		Json::Value haxisValue;
		haxisValue["name"] = hAxis.getName();
		haxisValue["angle"] = hAxis.getAngle();
		haxisValue["distance"] = hAxis.getDistance();
		hAxises.append(haxisValue);
	}
	for (auto vAxis : axises.getVAxises())
	{
		Json::Value vaxisValue;
		vaxisValue["name"] = vAxis.getName();
		vaxisValue["angle"] = vAxis.getAngle();
		vaxisValue["distance"] = vAxis.getDistance();
		vAxises.append(vaxisValue);
	}

	axisValue["hAxis"] = hAxises;
	axisValue["vAxis"] = vAxises;

	return axisValue;
}

Json::Value ResultWrite::beamLocationW(const std::shared_ptr<Beam>& _spBeamLocVec)
{
	Json::Value beamLoc;
	beamLoc["name"] = _spBeamLocVec->getName();
	beamLoc["flat"] = _spBeamLocVec->getbAssemblyAnnotation();
	beamLoc["spanNum"] = _spBeamLocVec->getSpanNum();
	
	//初始化梁跨信息
	for (auto it : _spBeamLocVec->spSpanVec)
	{

	}


	return Json::Value();
}

Json::Value ResultWrite::returnBeamSpanVec(const std::vector<std::shared_ptr<BeamSpan>>& spSpanVec)
{
	Json::Value beamSpanVec;
	if (spSpanVec.empty())
	{
		;//错误
	}
	else
	{
		for (auto it : spSpanVec)
		{
			Json::Value beamSpan;



			beamSpanVec.append(beamSpan);
		}
	}
	return beamSpanVec;
}

Json::Value ResultWrite::WallLocationW(const std::shared_ptr<WallLocation>& _spWallLoc)
{
	Json::Value wallLoc;
	wallLoc["name"] = _spWallLoc->getName();
	wallLoc["thick"] = _spWallLoc->getThick();
	wallLoc["sectionSymbolText"] = _spWallLoc->sectionSymbolText;
	//初始化参考点
	Json::Value tempReferencePoint;
	tempReferencePoint["startReferencePoint"] = referencePoint(_spWallLoc->getReferenceStartPoint());
	tempReferencePoint["endReferencePoint"] = referencePoint(_spWallLoc->getReferenceEndPoint());
	wallLoc["referencePoint"] = tempReferencePoint;

	//所在楼层
	//tempReferencePoint["floor"] = "";
	return wallLoc;
}

Json::Value ResultWrite::referencePoint(WallReferencePoint referencePoint)
{
	Json::Value tempPoint;
	Json::Value referenceHAxis;
	Json::Value referenceVAxis;
	auto hAxis = referencePoint.getReferenceHAxis();
	referenceHAxis["distance"] = hAxis.second;
	referenceHAxis["name"] = hAxis.first;
	auto vAxis = referencePoint.getReferenceVAxis();
	referenceVAxis["distance"] = vAxis.second;
	referenceVAxis["name"] = vAxis.first;
	
	tempPoint["referenceHAxis"] = referenceHAxis;
	tempPoint["referenceVAxis"] = referenceVAxis;


	return tempPoint;
}

Json::Value ResultWrite::columnLocResultData(ColumnLoc columnLoc)
{
	Json::Value columnLocValue;
	columnLocValue["name"] = columnLoc.getName();
	columnLocValue["angle"] = columnLoc.getAngle();
	columnLocValue["referenceHAxis"] = columnLoc.getReferenceHAxisValue();
	columnLocValue["referenceVAxis"] = columnLoc.getReferenceVAxisValue();
	Json::Value borders, hLines, vLines, sLines;
	
	for (auto it : columnLoc.getHlines())
	{
		hLines.append(_rwBaseEntity.lineResultValueW(it));
	}

	for (auto it : columnLoc.getVlines())
	{
		vLines.append(_rwBaseEntity.lineResultValueW(it));
	}

	for (auto it : columnLoc.getSlines())
	{
		sLines.append(_rwBaseEntity.lineResultValueW(it));
	}

	borders["hLines"] = hLines;
	borders["vLines"] = vLines;
	borders["sLines"] = sLines;
	columnLocValue["borders"] = borders;

	return columnLocValue;
}

Json::Value ResultRead::readResultFile(const std::string& resultFilename)
{
	std::ifstream of1;
	of1.open(resultFilename, std::ios::in);
	std::string strInfo;
	of1 >> strInfo;
	Json::Value body;
	Json::Reader reader;
	reader.parse(strInfo, body);

	if (!body.isObject())
	{
		;//错误处理
	}
	return body;
}

Json::Value ResultRead::returnFieldValue(const std::string& path, const std::string& field)
{
	Json::Value jsonData;
	//bool mark = false;
	bool mark = true;
#if 0
#if _WIN32
	std::string fileId = path;
	//path = path + fileId;
	if (!PathIsDirectory(path.c_str()))
	{
		;//输出错误数据
	}
	else
	{
		mark = true;
	}
	//CreateDirectory(path.c_str(), NULL);
#elif
#endif

#endif
	if (mark)
	{
		auto tempPath = path + ".json";
		auto tempValue = readResultFile(tempPath);
		

		if (tempValue[field].isArray() || tempValue[field].isObject())
		{
			jsonData = tempValue[field];
		}
		else
		{
			;//错误处理
		}

	}
	return jsonData;
}

std::vector<ColumnLoc> ResultRead::readCoulmnLocList(const Json::Value& columnLocList)
{
	std::vector<ColumnLoc> tempColumnLocVec;
	/*std::fstream of("testJson.txt", std::ios::out);
	of << columnLocList;
	of.close();*/

	if (columnLocList.isArray())
	{
		for (auto it : columnLocList)
		{
			tempColumnLocVec.push_back(readColumnLocValue(it));
		}
	}
	else
	{
		;//错误处理
	}
	return tempColumnLocVec;
}

std::vector<WallLoc> ResultRead::readWallLocList(const Json::Value& wallLocList)
{
	std::vector<WallLoc> tempWallVec;

	if (wallLocList.isArray())
	{
		for (auto it : wallLocList)
		{
			tempWallVec.push_back(readWallLocValue(it));
		}
	}
	else
	{
		;//错误处理
	}
	return tempWallVec;
}

std::vector<AxisProcessInfo> ResultRead::readAxisList(const Json::Value& axisList)
{
	std::vector<AxisProcessInfo> tempAxisVec;

	for (auto it : axisList)
	{
		AxisProcessInfo tempAxisProcessInfo;
		Json::Value tempAxisList = it;
		if (tempAxisList.isMember("axisH"))
		{
			Json::Value hAxis = tempAxisList["axisH"];
			std::vector<SingleAxisInfo>tempHAxis;
			for (auto it : hAxis)
			{
				tempHAxis.push_back(readAxisProcessInfo(it));
			}
			tempAxisProcessInfo.setHAxises(tempHAxis);
		}
		else
		{
			;//错误
		}
		if (tempAxisList.isMember("axisV"))
		{
			Json::Value vAxis = tempAxisList["axisV"];
			std::vector<SingleAxisInfo>tempVAxis;
			for (auto it : vAxis)
			{
				tempVAxis.push_back(readAxisProcessInfo(it));
			}
			tempAxisProcessInfo.setVAxises(tempVAxis);
		}
		else
		{
			;//错误
		}

		tempAxisVec.push_back(tempAxisProcessInfo);
	}
	
	
	return tempAxisVec;
}

ColumnLoc ResultRead::readColumnLocValue(const Json::Value& columnLocValue)
{
	ColumnLoc tempColumnLoc;
	tempColumnLoc.setName(columnLocValue["name"].asCString());
	tempColumnLoc.setAngle(columnLocValue["angle"].asDouble());
	tempColumnLoc.setReferenceHAxisValue(columnLocValue["referenceHAxis"]);
	tempColumnLoc.setReferenceVAxisValue(columnLocValue["referenceVAxis"]);

	//读入线数据(斜线待补充)
	auto border = columnLocValue["border"];
	auto borderHLines = border["hLines"];
	auto borderVLines = border["vLines"];
	//auto borderSLines = border["sLines"];
	tempColumnLoc.setHlines(baseEntity.readLineList(borderHLines));
	tempColumnLoc.setVlines(baseEntity.readLineList(borderVLines));
	//tempColumnLoc.setSlines(readLineList(borderSLines));
	
	//读入参考轴以及参考轴的相对距离
	auto referencehAxis = columnLocValue["referenceHAxis"];
	auto referencevAxis = columnLocValue["referenceVAxis"];
	tempColumnLoc.setReferenceH(referencehAxis["name"].asCString(), referencehAxis["distance"].asDouble());
	tempColumnLoc.setReferenceV(referencevAxis["name"].asCString(), referencevAxis["distance"].asDouble());

	return tempColumnLoc;
}

WallLoc ResultRead::readWallLocValue(const Json::Value& wallLocValue)
{

	WallLoc tempWallLoc;
	tempWallLoc.setName(wallLocValue["name"].asString());
	tempWallLoc.setThick(wallLocValue["thick"].asDouble());
	//初始化参考点
	Json::Value referencePoint = wallLocValue["referencePoint"];
	Json::Value startPoint = referencePoint["startReferencePoint"];
	Json::Value endPoint = referencePoint["endReferencePoint"];
	Json::Value tempHAxis = startPoint["referenceHAxis"];
	Json::Value tempVAxis = startPoint["referenceVAxis"];
	tempWallLoc.setStartReferencePoint(std::pair<std::string, double>(tempHAxis["name"].asCString(), tempHAxis["distance"].asDouble()),
		std::pair<std::string, double>(tempVAxis["name"].asCString(), tempVAxis["distance"].asDouble()));

	tempHAxis = endPoint["referenceHAxis"];
	tempVAxis = endPoint["referenceVAxis"];
	tempWallLoc.setEndReferencePoint(std::pair<std::string, double>(tempHAxis["name"].asCString(), tempHAxis["distance"].asDouble()),
		std::pair<std::string, double>(tempVAxis["name"].asCString(), tempVAxis["distance"].asDouble()));

	return tempWallLoc;
}

SingleAxisInfo ResultRead::readAxisProcessInfo(const Json::Value& axisValue)
{
	SingleAxisInfo tempAxis;
	if (axisValue.isMember("distance"))
	{
		tempAxis.setDistance(axisValue["distance"].asDouble());
	}
	else
	{
		;//错误
	}
	if (axisValue.isMember("name"))
	{
		tempAxis.setName(axisValue["name"].asString());
	}
	else
	{
		;//错误
	}
	if (axisValue.isMember("angle"))
	{
		tempAxis.setAngle(axisValue["angle"].asDouble());
	}
	else
	{
		;//错误
	}

	return tempAxis;
}
