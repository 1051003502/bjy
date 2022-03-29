#ifndef __DRAWINGRELATED_DATAINTEGRATION_H__
#define __DRAWINGRELATED_DATAINTEGRATION_H__
#include "coordinateSystem.h"
#include "columnInfo.h"
#include "beamInfo.h"
#include "spdlog/spdlog.h"
#include "drawing.h"
#include "wallInfo.h"

class DrawingData
{
public:
	DrawingData() {}
	DrawingData(const std::shared_ptr<Transer>& _spDraTranser,
				const std::shared_ptr<Parser>& _spDarParser);


public:
	std::shared_ptr<Transer> spDraTranser;//��ʾdra ��ʾͼֽdrawing �洢drawing��transer����
	std::shared_ptr<Parser> spDarParser;//�洢drawing��parser����


};

//ͼֽ�ļ����ݣ�����ͼֽ��map�洢
using FileDataMap = std::map<std::string, DrawingData>;

//��������
class DataIntegration
{

	//�½ṹ
#if 1
/**********function interface*************/
public:
	// ˵��	: ��ϸ����ĳ���͵�ͼֽ�����������������д���ļ�
	bool parserDrawingWriteFile();
	// ˵��	: ��ʼ��ͼֽ����_file
	bool iniFileDataMap(const std::string& _fileName,
		const shared_ptr<Transer>& _spTranser,
		const shared_ptr<Parser>& _spParser);
	// ˵��	: �����ѻ�����������ǽ����ֱ�������Ϣ
	Drawing returnDrawing(const std::shared_ptr<Transer>& _spDraTranser,
		const std::shared_ptr<Parser>& _spDarParser,
		const std::string& drawingParameter);
	// ˵��	: ���ص�ǰͼֽ���Ƶ�ͼֽ����
	DrawingData	returnFindFIle(const std::string& _fileName);
	// ˵��	: ���ش����ͼֽ����
	int filesSize();
	// ˵��	: ���ĳͼֽ
	bool filesErase(const std::string& file);
	FileDataMap::iterator filesBegin();
	FileDataMap::iterator filesEnd();
	//���ݻ������ݽ����ڴ˽���ͼֽ�������߳�
	bool drawingParser(const std::string path, const DrawingData& fileData);
/**************private function******************/
//private:
public:
	//˵�������������ļ�������������
	bool processClueFile();
	bool initegrationAxis(const Json::Value& axisData);
	bool integrationColumn(const Json::Value& columnData);
	bool integrationBeam(const Json::Value& beamData);
	bool integrationWall(const Json::Value&wallData);
	bool integrationFloor(const Json::Value& floorData);
private:
	FileDataMap _fileData;//ͼֽ��Ϣ
	CoordinateSystem _coordinateSystem;//��������ϵ
	ColumnInfo _columnInfo;//��������Ϣ
	WallInfo _wallInfo;//����ǽ��Ϣ
	ResultRead _clueFileReadIn;//���������ļ�
	std::vector<std::shared_ptr<thread>>_drawingThread;//�ɶ��ͼֽ��������߳�

#endif



#if 1
private://!����Ϣ
	/*����������*/
	//ColumnInfo _columnInfo;//!��������Ϣ
public://!����Ϣ
	BeamInfo _beamInfo;//!���й���������Ϣ
	std::map<std::string, std::vector<std::shared_ptr<BeamSection>>>_BeamSectionMap;//��ͼֽ����Ӧ��������
	std::map<std::string, std::vector<std::shared_ptr<Beam>>>_AaBeams;//ƽ��ֱ�ӻ�ȡ����Ϣ
#endif

};
#endif
