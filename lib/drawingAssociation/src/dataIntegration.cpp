#include "dataIntegration.h"

DrawingData::DrawingData(const std::shared_ptr<Transer>& _spDraTranser,
	const std::shared_ptr<Parser>& _spDarParser)
{
	this->spDraTranser = _spDraTranser;
	this->spDarParser = _spDarParser;
}

Drawing DataIntegration::returnDrawing(const std::shared_ptr<Transer>& _spDraTranser, 
	const std::shared_ptr<Parser>& _spDarParser, 
	const std::string &drawingParameter)
{
	return Drawing(_spDraTranser,_spDarParser, drawingParameter);
}

bool DataIntegration::parserDrawingWriteFile()
{
	for (auto it : _fileData)
	{
		Drawing drawing = returnDrawing(it.second.spDraTranser, it.second.spDarParser, it.first);
		drawing.parserDrawing();
		/*auto  tempThread = std::make_shared<std::thread>(std::bind(&DataIntegration::drawingParser, it.second, it.first));
		_drawingThread.push_back(tempThread);*/
	}
	return true;
}




bool DataIntegration::iniFileDataMap(const std::string &_fileName,
	const shared_ptr<Transer>& _spTranser,
	const shared_ptr<Parser>& _spParser)
{
	_fileData[_fileName] = DrawingData(_spTranser, _spParser);
	return true;
}

bool DataIntegration::processClueFile()
{
	std::string clueFilePath= "../projectJsonFile/clue.json";
	auto clueFileValue = _clueFileReadIn.readResultFile(clueFilePath);
	//ÖáÍø
	initegrationAxis(clueFileValue);
	//Öù
	integrationColumn(clueFileValue);
	//Áº

	//Ç½
	//integrationWall(clueFileValue);
	//°å
	return true;
}

bool DataIntegration::initegrationAxis(const Json::Value& axisData)
{
	//ÖáÍø
	if (axisData.isMember("axis"))
	{
		Json::Value axisClue = axisData["axis"];
		_coordinateSystem.findAxisPathJsonValue(axisClue);
		_coordinateSystem.readAxisCoordinateSystem();
		_coordinateSystem.outputAxisJsonData();
	}
	else
	{
		;//´íÎó´¦Àí
	}
	return true;
}

bool DataIntegration::integrationColumn(const Json::Value& columnData)
{
	//Öù
	if (columnData.isMember("column"))
	{
		Json::Value columnClue = columnData["column"];
		_columnInfo.findColumnPathJsonValue(columnClue);
		_columnInfo.readColumnLocInfo();
		_columnInfo.outputColumnJsonData();
	}
	else
	{
		;//´íÎó´¦Àí
	}
	return true;
}

bool DataIntegration::integrationBeam(const Json::Value& beamData)
{
	return true;
}

bool DataIntegration::integrationWall(const Json::Value& wallData)
{
	//Ç½
	if (wallData.isMember("wall"))
	{
		Json::Value columnClue = wallData["wall"];
		_wallInfo.findWallPathJsonValue(columnClue);
		_wallInfo.readWallLocInfor();
		//f();×öÕûºÏ
	}
	else
	{
		;//´íÎó´¦Àí
	}
	return true;
}

bool DataIntegration::integrationFloor(const Json::Value& floorData)
{
	return true;
}

DrawingData DataIntegration::returnFindFIle(const std::string & _fileName)
{
	return _fileData[_fileName];
}

int DataIntegration::filesSize()
{
	return _fileData.size();
}

bool DataIntegration::filesErase(const std::string & file)
{
	_fileData.erase(file);
	return true;
}

FileDataMap::iterator DataIntegration::filesBegin()
{
	return _fileData.begin();
}

FileDataMap::iterator DataIntegration::filesEnd()
{
	return _fileData.end();
}

bool DataIntegration::drawingParser(const std::string path,const DrawingData& fileData)
{

	Drawing drawing = returnDrawing(fileData.spDraTranser, fileData.spDarParser, path);
	//drawing.separationParameter();
	drawing.parserDrawing();
	return true;
}







