#ifndef __DRAWINGRELATED_RWJSONFILE_H__
#define __DRAWINGRELATED_RWJSONFILE_H__
#include "json/json.h"
#include "data.h"
#include "axis.h"
#include "border.h"
#include "table.h"
#include "block.h"
#include "beamSet.h"
#include "beamSectionSet.h"
#include "pillarSet.h"
#include "ColumnSectionSet.h"
#include "transform.h"
#include "columnProcessInfo.h"
#include "wallLocation.h"
#include "wallProcessInfo.h"
#include "axisProcessInfo.h"
#if _WIN32
/*#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")*/
#include<io.h>
#else
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


/*��д�����ĵ�����*/
class RWBaseEntity
{
public:
	//д���
	Json::Value pointValueW(const Point& point);
	//�����
	Point pointValueR(const Json::Value& point);
	//д�����������
	Json::Value pointValueW(const Point& referencePoint, const Point& point);
	//д����
	Json::Value lineValueW(const Line& line);
	//д���߽������
	Json::Value lineResultValueW(const Line& line);
	//������
	Line lineValueR(const Json::Value& line);
	//д���ߵ��������
	Json::Value lineValueW(const Point& referencePoint, const Line& line);

	//�������б�
	std::vector<Line>readLineList(const Json::Value& lineVec);



};

/*����ֺ���Ĳ����ļ�*/
class ReadTypeFile
{
public:
	/*��������ļ�*/
	Json::Value readParameterFile(const std::string& parameterFile);
	//���������ļ�
	std::map<std::string,std::vector<std::string>>parserParameterFile(const Json::Value& parameterValue);
	//�����ļ��İ�ͼֽ
	Json::Value readFloorPlan(const Json::Value& parameterFile);
	//�����ļ�������ͼֽ
	Json::Value readProfileFile(const Json::Value& parameterFile);
	//�����ļ�����ͼͼֽ
	Json::Value readDetailFile(const Json::Value& parameterFile);
public:

	
};

/*����ļ�д��*/
class ResultWrite
{
public:
	//��������һ��json�ļ�
	bool creatJsonResultFile(const std::string& path, const std::string& fileName, const Json::Value& value);

	//д����������
	Json::Value axisesValueW(const Axis& _axis);
	//д����λ������
	Json::Value columnLocationW(const std::shared_ptr<Pillar>& _spPillar);
	//д����������
	Json::Value AxisesW(AxisProcessInfo axises);
	//д���������������
	Json::Value columnSectionStirrupW();
	//д����λ����Ϣ
	Json::Value beamLocationW(const std::shared_ptr<Beam>& _spBeamLocVec);
	//��ʼ��������Ϣ
	Json::Value returnBeamSpanVec(const std::vector<std::shared_ptr<BeamSpan>>& spSpanVec);
	//д��ǽλ����Ϣ
	Json::Value WallLocationW(const std::shared_ptr<WallLocation>& _spWallLoc);
	Json::Value referencePoint(WallReferencePoint referencePoint);


public:
	//�����ս��д��
	Json::Value columnLocResultData(ColumnLoc columnLoc);

private:
	RWBaseEntity _rwBaseEntity;

};

class ResultRead
{
public:
	Json::Value readResultFile(const std::string& resultFilename);
	Json::Value returnFieldValue(const std::string& path, const std::string& field);
	
public:
	std::vector<ColumnLoc> readCoulmnLocList(const Json::Value& columnLocList);
	std::vector<WallLoc>readWallLocList(const Json::Value& wallLocList);
	std::vector<AxisProcessInfo> readAxisList(const Json::Value& axisList);

private:
	ColumnLoc readColumnLocValue(const Json::Value& columnLocValue);
	WallLoc readWallLocValue(const Json::Value& wallLocVale);
	SingleAxisInfo readAxisProcessInfo(const Json::Value& axisValue);

private:
	RWBaseEntity baseEntity;

};

#if 0
/*��������ļ�*/
Json::Value readParameterFile(const std::string& parameterFile);
//�����ļ��İ�ͼֽ
Json::Value readFloorPlan(const Json::Value& parameterFile);
//�����ļ�������ͼֽ
Json::Value readProfileFile(const Json::Value& parameterFile);
//�����ļ�����ͼͼֽ
Json::Value readDetailFile(const Json::Value& parameterFile);

//д���
Json::Value pointValueW(const Point& point);
//�����
Point pointValueR(const Json::Value& point);
//д�����������
Json::Value pointValueW(const Point& referencePoint, const Point& point);
//д����
Json::Value lineValueW(const Line& line);
//������
Line lineValueR(const Json::Value& line);
//�������б�
std::vector<Line>readLineList(const Json::Value& lineVec);



//д���ߵ��������
Json::Value lineValueW(const Point& referencePoint, const Line& line);

//д�������б�
Json::Value numberListW(const std::vector<int>& numberListW);
//д��Բ
Json::Value circleValueW(const Circle& circle);


/************line����************/
Json::Value lineDataValueW(LineData& lineData);

/***********circle����***********/
Json::Value circleDataValueW(CirclesData& circleData);

/***********corner����***********/
//д��corner����
Json::Value cornerValueW(const Corner& corner);
//д��cornerData����
Json::Value cornerDataValueW(const CornerData& cornerData);


/***********endPoint����************/
//д��endPoint����
Json::Value endPointValueW(const Endpoint& endPoint);
//д��endPoiintData����
Json::Value endPointDataValueW(EndpointData& endPointData);


/**********д��textPoint����***************/
//д��textPoint����
Json::Value textPointValueW(const Point& point);
//д��textPointData����
Json::Value textPointDataValueW(TextPointData& textPointData);

//д��ͼ�������ݣ�����ͬ��ͼ��������������
Json::Value lineLayerDataValueW(LineLayerData& lineLayerData);


/************д��AxisLine����****************/
//д��axisLine����
Json::Value axisLineValueW(const Axis::AxisLine& axisLine);
//д��axisLineData����
Json::Value axisLineDataValueW(AxisLineData& axisLineData);
//д����������
Json::Value axisesValueW(const Axis& _axis);


/*************���������д��*****************/
Json::Value columnLocationW(const std::shared_ptr<Pillar>& _spPillar);
#endif











#endif