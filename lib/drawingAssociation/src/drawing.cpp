#include "drawing.h"



Drawing::Drawing(const std::shared_ptr<Transer>& _spDraTranser, 
	const std::shared_ptr<Parser>& _spDarParser, 
	const std::string& _drawingParameter)
{
	this->_spDraTranser = _spDraTranser;
	this->_spDarParser = _spDarParser;
	this->_drawingParameter = _drawingParameter;
}

bool Drawing::parserDrawing(/*const Drawing::DrawingType& drawingType*/)
{
	//��ʼ����������ͼֽ����
	_iniDrawingType.parserParameter(_drawingParameter);
	//��ʼ��ͼֽ����
	setAxis();
	//����ͼֽ���;������ͼֽ����
	specificParserDrawing();
	//д�뾭�����������õĽ������
	writeParserResultInfo();
	//д�������ļ�
	setClueFile();
	return true;
}

bool Drawing::setClueFile()
{
	
	std::string path = "../projectJsonFile/clue.json";
	Json::Value body = readClueFile(path);
	
	if (body.isObject())
	{
		body["axis"] = writeClueAxis(body);
		body["column"] = writeClueColumn(body);
		//body["beam"] = writeClueBeam(body);
		//body["wall"] = writeClueWall(body);
		//body["floor"] = writeClueFloor(body);
	}
	else
	{
		return true;
	}
	
	// ����޸�ʽjson�ַ���  
	writeClueFile(body, path);
	
	return true;
}

Json::Value Drawing::readClueFile(const std::string& fileName)
{
	std::fstream of;
	of.open(fileName, std::ios::in);
	std::string strInfo;
	of >> strInfo;
	of.close();
	Json::Value body;
	Json::Reader reader;
	if (reader.parse(strInfo, body))
	{
		return body;
	}
	else
	{
		//���������־
		return Json::Value();
	}

}

bool Drawing::writeClueFile(const Json::Value& rootValue, const std::string& fileName)
{
	// ����޸�ʽjson�ַ���  
	std::fstream of;
	Json::FastWriter writer;
	std::string writerInfoStr = writer.write(rootValue);
	of.open(fileName);
	of << writerInfoStr;
	of.close();
	return true;
}

Json::Value Drawing::writeClueAxis(const Json::Value& rootValue)
{
	if (rootValue.isMember("axis") && _iniDrawingType.getDirector().front() == "floorPlan")
	{
		Json::Value axises = rootValue["axis"];
		if (_resultMap.find(ResultValue::AXISES) != _resultMap.end() && !axises.isMember("floorPlan"))
		{
			Json::Value floorPlan;
			Json::Value axisValue;
			std::string drawingName = _iniDrawingType.getDrawingName();
			axisValue[drawingName] = "axises";
			floorPlan.append(axisValue);
			axises["floorPlan"] = floorPlan;
		}
		else
		{
			Json::Value floorPlan = axises["floorPlan"];
			Json::Value axisValue;
			std::string drawingName = _iniDrawingType.getDrawingName();
			axisValue[drawingName] = "axises";
			floorPlan.append(axisValue);
			axises["floorPlan"] = floorPlan;
		}
		
		return axises;
	}

#if 0
	if (rootValue.isMember("axis"))
	{
		Json::Value column = rootValue["axis"];
		if (_resultMap.find(ResultValue::AXISES) != _resultMap.end())
		{
			std::string directorFront, directorBack;
			auto directory = _iniDrawingType.getDirector();
			if (directory.size() > 1)
			{
				directorFront = directory.front();
				directorBack = directory.back();
			}
			else
			{
				directorFront = directory.front();
			}

			if (!directorFront.empty() && !directorBack.empty())
			{	//�������� ƽ�棬���棬��ͼ��
				Json::Value directorFrontValue;
				if (column[directorFront].isObject())
				{

					directorFrontValue = column[directorFront];
					Json::Value directorBackValue;
					if (directorFrontValue[directorBack].isArray())
					{
						directorBackValue = directorFrontValue[directorBack];
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingAddress()] = "axises";
						directorBackValue.append(fileField);
					}
					else
					{
						//Json::Value directorBackValue;
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingAddress()] = "axises";
						directorBackValue.append(fileField);


					}

					directorFrontValue[directorBack] = directorBackValue;
				}
				else//��û�����Լ�����
				{
					Json::Value directorBackValue;
					Json::Value fileField;
					fileField[_iniDrawingType.getDrawingAddress()] = "axises";
					directorBackValue.append(fileField);
					directorFrontValue[directorBack] = directorBackValue;
				}
				column[directorFront] = directorFrontValue;
			}
			else
			{
				;
			}
		}
		return column;
	}
#endif;
	return Json::Value();
}

Json::Value Drawing::writeClueColumn(const Json::Value& rootValue)
{
	if (rootValue.isMember("column"))
	{
		Json::Value column = rootValue["column"];
		if (_resultMap.find(ResultValue::COLUMNLOC) != _resultMap.end()&& 
			_iniDrawingType.getDirector().front() == "floorPlan")
		{
			std::string directorFront, directorBack;
			auto directory = _iniDrawingType.getDirector();
			if (directory.size() > 1)
			{
				directorFront = directory.front();
				directorBack = directory[1];
			}
			else
			{
				directorFront = directory.front();
			}

			if (!directorFront.empty() && !directorBack.empty())
			{	//�������� ƽ�棬���棬��ͼ��
				Json::Value directorFrontValue;
				if (column[directorFront].isObject())
				{

					directorFrontValue = column[directorFront];
					Json::Value directorBackValue;
					if (directorFrontValue[directorBack].isArray())
					{
						directorBackValue = directorFrontValue[directorBack];
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingName()] = "columnLoc";
						directorBackValue.append(fileField);
					}
					else
					{
						//Json::Value directorBackValue;
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingName()] = "columnLoc";
						directorBackValue.append(fileField);


					}

					directorFrontValue[directorBack] = directorBackValue;
				}
				else//��û�����Լ�����
				{
					Json::Value directorBackValue;
					Json::Value fileField;
					fileField[_iniDrawingType.getDrawingName()] = "columnLoc";
					directorBackValue.append(fileField);

					//Json::Value directorFrontValue;
					directorFrontValue[directorBack] = directorBackValue;

					//column[directorFront]=directorFrontValue;
				}
				column[directorFront] = directorFrontValue;
			}
			else
			{
				;
			}
		}
		return column;
	}
	return Json::Value();
	
}

Json::Value Drawing::writeClueBeam(const Json::Value& rootValue)
{
	return true;
}

Json::Value Drawing::writeClueWall(const Json::Value& rootValue)
{
	if (rootValue.isMember("wall"))
	{
		Json::Value wall = rootValue["wallLoc"];
		if (_resultMap.find(ResultValue::WALLLOC) != _resultMap.end())
		{
			std::string directorFront, directorBack;
			auto directory = _iniDrawingType.getDirector();

			if (directory.size() > 1)
			{
				directorFront = directory.front();
				directorBack = directory.back();
			}
			else
			{
				directorFront = directory.front();
			}
			if (!directorFront.empty() && !directorBack.empty())
			{	//�������� ƽ�棬���棬��ͼ��
				Json::Value directorFrontValue;
				if (wall[directorFront].isObject())
				{

					directorFrontValue = wall[directorFront];
					Json::Value directorBackValue;
					if (directorFrontValue[directorBack].isArray())
					{
						directorBackValue = directorFrontValue[directorBack];
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingAddress()] = "wallLoc";
						directorBackValue.append(fileField);
					}
					else
					{
						//Json::Value directorBackValue;
						Json::Value fileField;
						fileField[_iniDrawingType.getDrawingAddress()] = "wallLoc";
						directorBackValue.append(fileField);


					}

					directorFrontValue[directorBack] = directorBackValue;
				}
				else//��û�����Լ�����
				{
					Json::Value directorBackValue;
					Json::Value fileField;
					fileField[_iniDrawingType.getDrawingAddress()] = "wallLoc";
					directorBackValue.append(fileField);
					directorFrontValue[directorBack] = directorBackValue;
				}
				wall[directorFront] = directorFrontValue;
			}
			else
			{
				;
			}
		}
		return wall;
	}
	return Json::Value();
}

Json::Value Drawing::writeClueFloor(const Json::Value& rootValue)
{
	return true;
}

bool Drawing::setResultMap(const ResultValue& valueType, const std::string& fieldName)
{
	_resultMap[valueType] = fieldName;
	return true;
}

bool Drawing::writeParserResultInfo()
{
	Json::Value resuleData;
	switch (_iniDrawingType.returnDrawingType())
	{
	case IniDrawingType::DrawingType::MSP://����ṹƽ��ͼ
		writeSMP(resuleData);
		break;
	case IniDrawingType::DrawingType::LBSP://�������ͼ

		break;
	case IniDrawingType::DrawingType::BSP://�����ƽ��ͼ
		break;
	case IniDrawingType::DrawingType::WSP://ǽ���ƽ��ͼ
		break;
	case IniDrawingType::DrawingType::MSHP://����ṹ����ͼ
		break;
	case IniDrawingType::DrawingType::MSVP://����ṹ����ͼ
		break;
	case IniDrawingType::DrawingType::MSSHP://����ṹ������ͼ
		break;
	case IniDrawingType::DrawingType::MSSVP://����ṹ�������ͼ
		break;
	case IniDrawingType::DrawingType::CSSD://��������ͼ
		writeCSSD(resuleData);
		break;
	default:
		//ͼֽ���δ֪������֤�����
		break;

	}
	
	Json::FastWriter writer;
	std::string path = "../projectJsonFile/drawing/";
	//setFileID();
	auto typeFirst = _iniDrawingType.getDirector().front();
	path = path + typeFirst;

	
#if _WIN32
	//std::string fileId = _iniDrawingType.getDrawingAddress();
 	if (!PathIsDirectory(path.c_str()))
	{
		CreateDirectory(path.c_str(), NULL);
	}
	//CreateDirectory(path.c_str(), NULL);
#else

#endif
	std::string resultFileName;
	resultFileName = _iniDrawingType.getDrawingName() + ".json";
	path = path + "/" + resultFileName;
	std::fstream of(path, std::ios::out);
	of << writer.write(resuleData);
	of.close();
	
	return true;
}


//bool Drawing::separationParameter()
//{
//#if 0
//	int directoryLocFirst = static_cast<int>(_drawingParameter.find_first_of("|"));
//	int directorLocLast = static_cast<int>(_drawingParameter.find_last_of("|"));
//	if (directoryLocFirst == directorLocLast)
//	{
//		
//		auto directory = _drawingParameter.substr((long long)0, directoryLocFirst);
//		_directory.push_back(directory);
//		_address = _drawingParameter.substr((long long)directoryLocFirst + 1);
//		creatFileDirectory(_directory);
//
//	}
//	else
//	{
//		
//		auto directory1 = _drawingParameter.substr((long long)0, directoryLocFirst);
//		auto directory2 = _drawingParameter.substr((long long)directoryLocFirst + 1, (long long)directorLocLast - directoryLocFirst - 1);
//		_directory.push_back(directory1);
//		_directory.push_back(directory2);
//		_address = _drawingParameter.substr((long long)directorLocLast + 1);
//		creatFileDirectory(_directory);
//	}
//#endif
//	return true;
//}

bool Drawing::setAxis()
{
	if (!_spDarParser->_axises.empty())
		_axises = _spDarParser->_axises;
	return true;
}

bool Drawing::creatFileDirectory(const std::vector<std::string>& directorVec)
{
	std::string director = "../projectJsonFile/drawing/";

#if _WIN32
	for (auto it : directorVec)
	{
		director = director + it + "/";
		if (!PathIsDirectory(director.c_str()))
		{
			CreateDirectory(director.c_str(), NULL);
		}
	}
#else

#endif

	return true;
}

//bool Drawing::setDrawingType()
//{
//#if 0
//	auto firstType = _directory.front();
//	//�����������Ҫ
//	if (firstType == "floorPlan")
//	{
//		_drawingType = DrawingType::MSP;
//	}
//	else if (firstType == "profile")
//	{
//		auto secondType = _directory.back();
//		if (secondType == "hProfile")
//		{
//			_drawingType = DrawingType::MSHP;
//		}
//		else if (secondType == "vProfile")
//		{
//			_drawingType = DrawingType::MSVP;
//		}
//	}
//	else if (firstType =="detail")
//	{
//		_drawingType = DrawingType::CSSD;
//	}
//#endif
//	return true;
//}

bool Drawing::specificParserDrawing()
{
	switch (_iniDrawingType.returnDrawingType())
	{
	case IniDrawingType::DrawingType::MSP://����ṹƽ��ͼ
		parserSMP();
		break;
	case IniDrawingType::DrawingType::LBSP://�������ͼ

		break;
	case IniDrawingType::DrawingType::BSP://�����ƽ��ͼ
		break;
	case IniDrawingType::DrawingType::WSP://ǽ���ƽ��ͼ

		break;
	case IniDrawingType::DrawingType::MSHP://����ṹ����ͼ
		break;
	case IniDrawingType::DrawingType::MSVP://����ṹ����ͼ
		break;
	case IniDrawingType::DrawingType::MSSHP://����ṹ������ͼ
		//bool parserMSSHP();
		break;
	case IniDrawingType::DrawingType::MSSVP://����ṹ�������ͼ
		break;
	case IniDrawingType::DrawingType::CSSD://��������ͼ
		parserCSSD();
		break;
	default:
		//ͼֽ���δ֪������֤�����
		break;

	}
	return true;
}

bool Drawing::parserSMP()
{
	
	//��
	parserColumnLoc();
	//��
	//parserBeamLoc();
	//ǽ
	//parserWallLoc();
	//��
	return true;
}

bool Drawing::parserCSSD()
{
	for (auto it : _spDarParser->_blocks)
	{
		auto ite = _columnSectionSet.columnSection(*_spDarParser->m_data, it);
		if (ite != nullptr)
			_spColumnSectionVec.push_back(ite);
	}
	return true;
}

bool Drawing::parserMSSHP()
{

	return true;
}

bool Drawing::parserColumnLoc()
{
	if (!_axises.empty())
	{
		_columnLocSet.setMember(_axises.front());
		_columnLocSet.columns(*this->_spDarParser->m_data, _spColumnVec/*, indexVec, lineVec, cornerVec*/);
	}
	else
	{
		;//�д���
	}
	
	return true;
}

bool Drawing::parserBeamLoc()
{
	if (!_axises.empty())
	{
		//!��λ��
		_beamLocSet.setMember(_axises.front(), _spColumnVec);
		_beamLocSet.findBeamLayer(_columnLocSet.returnColumnBorderGroupIndex(), this->_spDarParser->m_data->m_lineData);
		_beamLocSet.findBeam(*this->_spDarParser->m_data);
		_beamLocSet.beams(_spBeamLocVec, *this->_spDarParser->m_data);
	}
	
	return true;
}

bool Drawing::parserWallLoc()
{
	if (_axises.empty())
	{
		;//����
	}
	else
	{
		//ǽλ��

		for (auto it : _spDarParser->_blocks)
		{
			if (it->bAxis)
			{
				auto tempWallVec = _wallLocSet.returnWallLocation(_spDarParser->m_data, it,_spDarParser->_sectionSybmols);
				
				_spWallLocVec.insert(_spWallLocVec.end(), tempWallVec.begin(), tempWallVec.end());
			}
		}
	}
	return true;
}

bool Drawing::parserFloorLoc()
{
	return true;
}

bool Drawing::parserColumnSection()
{
	for (auto block : this->_spDarParser->_blocks)
	{
		auto columnSection = _columnSectionSet.columnSection(*_spDarParser->m_data, block);
		if (columnSection != nullptr)
		{
			_spColumnSectionVec.push_back(columnSection);
		}
	}
	return true;
}

bool Drawing::writeSMP(Json::Value& resultData)
{
	if (_axises.empty())
	{
		;//�������Json��Ϣ
		return true;
	}
	else
	{
		
		//����
		resultData["axises"] = axisesLoc();
		//��
		resultData["columnLoc"] = columnLoc();
		
		//��
		//resultData["beamLoc"] = beamLoc();

		//ǽ
		//resultData["wallLoc"] = wallLoc();


		//��
		return true;
	}
	
}

bool Drawing::writeCSSD(Json::Value& resuleData)
{
	/*for (auto it : _spColumnSectionVec)
	{

	}*/

	Json::Value columnSection;
	setResultMap(ResultValue::SCD, "columnSection");
	for (auto it : _spColumnSectionVec)
	{
		columnSection.append(it->outputColumnSection());
	}
	resuleData["columnSection"] = columnSection;

	/*Json::FastWriter writer;
	std::fstream of("columnSection1.json", std::ios::out);
	of << writer.write(_columnSectionSet.testSection);
	of.close();*/
	return true;
}

Json::Value Drawing::axisesLoc()
{
	Json::Value axises;
	if (!_axises.empty())
	{
		//�����������Ľ��д��result map��
		setResultMap(ResultValue::AXISES, "axises");
		for (auto it : _axises)
		{
			axises.append(_resultWrite.axisesValueW(*it));
		}
	}
	return axises;
}

Json::Value Drawing::columnLoc()
{
	//!��
	Json::Value columnLoc;
	if (_spColumnVec.empty())
	{
		;//���������Ϣ
		return columnLoc;
	}
	else
	{
		setResultMap(ResultValue::COLUMNLOC, "columnLoc");

		for (auto it : _spColumnVec)
		{
			columnLoc.append(_resultWrite.columnLocationW(it));
		}
	}
	std::fstream of("�����ĵ�.txt", std::ios::out);
	of << columnLoc;
	of.close();

	return columnLoc;
}

Json::Value Drawing::beamLoc()
{
	//��
	Json::Value beamLoc;
	if (_spBeamLocVec.empty())
	{
		;//���������Ϣ
		return beamLoc;
	}
	else
	{
		setResultMap(ResultValue::BEAMLOC, "beamLoc");
		for (auto it : _spBeamLocVec)
		{
			beamLoc.append(_resultWrite.beamLocationW(it));
		}
	}

	return beamLoc;
}

Json::Value Drawing::wallLoc()
{
	//��
	Json::Value wallLoc;
	if (_spWallLocVec.empty())
	{
		;//���������Ϣ
		return wallLoc;
	}
	else
	{
		setResultMap(ResultValue::WALLLOC, "wallLoc");
		for (auto it : _spWallLocVec)
		{
			wallLoc.append(_resultWrite.WallLocationW(it));
		}
	}
	/*std::fstream of("wallLoc.txt", std::ios::out);
	of << wallLoc << std::endl;
	of.close();*/
	return wallLoc;
}

Json::Value Drawing::floorLoc()
{
	return Json::Value();
}

bool IniDrawingType::parserParameter(const std::string& parameter)
{
	auto locVec = findCharLocNum(parameter, '|');
	int lastLoc = locVec.back();
	setDrawingDirectory(parameter, locVec);
	setDrawingType();//��ʼ��ͼֽ����
	setAddress(parameter, lastLoc);//��ʼ��ͼֽ��ַ
	setFileName();//��ʼ��ͼֽ����
	return true;
}

bool IniDrawingType::setDaringType(const IniDrawingType::DrawingType& type)
{
	_drawingType = type;
	return true;
}

const std::vector<std::string>& IniDrawingType::getDirector()
{
	return _directory;
}

const IniDrawingType::DrawingType& IniDrawingType::returnDrawingType()
{
	return _drawingType;
}

const int& IniDrawingType::directorSize()
{
	return _directory.size();
}

std::string IniDrawingType::getDrawingAddress()
{
	std::string fileId;
	auto num = _address.find_last_of("/");
	if (num != std::string::npos)
	{
		//fileId = GbkToUtf8(_address.substr(num + 1));
		fileId = _address.substr(num + 1);
	}

	return fileId;
}

std::string IniDrawingType::getDrawingName()
{
	std::string fileId;
	std::string fileName;
	auto num = _address.find_last_of("/");
	if (num != std::string::npos)
	{
		//fileId = GbkToUtf8(_address.substr(num + 1));
		fileId = _address.substr(num + 1);
		fileName = fileId.substr(0, fileId.find_last_of("."));
	}

	return fileName;
}



std::vector<int> IniDrawingType::findCharLocNum(const std::string& str, const char& ch)
{
	std::vector<int>tempReturnLoc;
	for (int i = 0; i < str.size(); ++i)
	{
		if (str[i] == ch)
		{
			tempReturnLoc.push_back(i);
		}
	}
	return tempReturnLoc;
}

bool IniDrawingType::setAddress(const std::string& str, const int& loc)
{

	_address = str.substr(loc + 1);
	return true;
}

bool IniDrawingType::setFileName()
{
	int drawingFileNameLoc = static_cast<int>(_address.find_last_of("/"));
	_fileName = _address.substr(drawingFileNameLoc + 1);
	return true;
}

bool IniDrawingType::setDrawingDirectory(const std::string& str, const std::vector<int>& chLocVec)
{
	int lastLoc = chLocVec.back();
	int firstLoc = chLocVec.front();
	if (lastLoc == firstLoc)
	{
		_directory.push_back(str.substr(0, firstLoc));
	}
	else
	{
		for (int i = 0; i < chLocVec.size(); ++i)
		{

			int nowLoc = -1;//�ض��ַ�
			if (i > 0)
			{
				nowLoc = chLocVec[i - 1];
			}
			_directory.push_back(str.substr(nowLoc + 1, chLocVec[i] - nowLoc - 1));
		}
	}
	
	return true;
}

bool IniDrawingType::setDrawingType()
{
	//�˺����ɱ���ݾ�������޸�
	if (!_directory.empty())
	{
		auto tempStrType = _directory.front();
		if (tempStrType=="floorPlan")
		{
			auto tempStirBack = _directory.back();
			if (tempStirBack == "struct")
			{
				_drawingType = IniDrawingType::DrawingType::MSP;
			}
			else if (tempStirBack == "beamFlat")
			{
				_drawingType = IniDrawingType::DrawingType::LBSP;
			}
			else if (tempStirBack == "floorStirrup")
			{
				_drawingType = IniDrawingType::DrawingType::BSP;
			}
			else if (tempStirBack == "notClear")
			{
				_drawingType = IniDrawingType::DrawingType::PLAN;
			}
		}
		else if (tempStrType == "detail")
		{
			auto tempStirBack = _directory.back();
			if (tempStirBack == "beamDetail")
			{
				_drawingType = IniDrawingType::DrawingType::BSD;
			}
			else if (tempStirBack == "columnDetail")
			{
				_drawingType = IniDrawingType::DrawingType::CSSD;
			}
			else if (tempStirBack == "columnAndBeam")
			{
				_drawingType = IniDrawingType::DrawingType::CBSSD;
			}
			else if (tempStirBack == "wallDetail")
			{
				_drawingType = IniDrawingType::DrawingType::WSD;
			}
			else if (tempStirBack == "xiangtu")
			{
				_drawingType = IniDrawingType::DrawingType::DETAIL;
			}

		}
		else if (tempStrType == "profile")
		{
			auto tempStirSecond = _directory[1];
			auto tempStirBack = _directory.back();
			if (tempStirSecond == "hProfile")
			{
				if (tempStirBack == "struct")
				{
					_drawingType = IniDrawingType::DrawingType::MSHP;
				}
				else if (tempStirBack == "stirrup")
				{
					_drawingType = IniDrawingType::DrawingType::MSSHP;
				}
				else if (tempStirBack == "notCLear")
				{
					_drawingType = IniDrawingType::DrawingType::PROH;
				}
			}
			else if (tempStirSecond == "vProfile")
			{
				if (tempStirBack == "struct")
				{
					_drawingType = IniDrawingType::DrawingType::MSVP;
				}
				else if (tempStirBack == "stirrup")
				{
					_drawingType = IniDrawingType::DrawingType::MSSVP;
				}
				else if (tempStirBack == "notCLear")
				{
					_drawingType = IniDrawingType::DrawingType::PROV;
				}
			}
			else if (tempStirSecond == "notClear")
			{
				if (tempStirBack == "struct")
				{
					_drawingType = IniDrawingType::DrawingType::MSPP;
				}
				else if (tempStirBack == "stirrup")
				{
					_drawingType = IniDrawingType::DrawingType::MSSP;
				}
				else if (tempStirBack == "notCLear")
				{
					_drawingType = IniDrawingType::DrawingType::PRO;
				}
			}
		}

	}
		

	

	return true;
}


