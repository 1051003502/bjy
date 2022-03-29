#ifndef __DRAWINGRELATED_DRAWINGDATA_H__
#define __DRAWINGRELATED_DRAWINGDATA_H__
#include "data.h"
#include "parser.h"
#include "transer.h"
#include "pillarSet.h"
#include "ColumnSectionSet.h"
#include "beamSet.h"
#include "beamLongitudinalSet.h"
#include "beamSectionSet.h"
#include "RWJsonFile.h"
#include "columnInfo.h"
#if _WIN32
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")
#else

#endif

class IniDrawingType
{

public:
	typedef enum
	{
		/*ƽ��ͼ*/
		PLAN,//ƽ��ͼ
		MSP,//main structure plan ����ṹƽ��ͼ
		LBSP,//Longitudinal beam stirrup plan �������ƽ��ͼ
		BSP,//board stirrup plan �����ƽ��ͼ
		WSP,//wall stirrup plan ǽ���ƽ��ͼ

		/*����ͼ*/
		MSPP,//����ṹ����ͼ
		MSSP,//main struct stirrup profile
		MSHP,//main stirrup profile ����ṹ����
		MSVP,//main structure longitudinal profile ����ṹ���� 
		MSSHP,//main structure stirrup profile ����ṹ������
		MSSVP,//main structure stirrup profile ����ṹ�������
		PROV,//���ʲ�֪���ǽṹ
		PROH,//���ʲ������ǽṹ
		PRO,//����ͼprofile
		/*��ͼ*/
		DETAIL,//��ʾ��ͼ ��ͼ���վ���Ŀ�ϸ��ÿ��������������
		CSSD,//��ʾ�����������ͼcolumn section stirrup detail
		BSD,//beam stirrup detail ����ͼ
		WSD,//wall stirrup detail ǽ��ͼ
		CWSD,//column beam stirrup detail ������ͼ
		CBSSD,//��������
		UNKONW
	}DrawingType;

public:
	bool parserParameter(const std::string& parameter);
	bool setDaringType(const IniDrawingType::DrawingType& type);
	const std::vector<std::string>& getDirector();
	const IniDrawingType::DrawingType& returnDrawingType();
	const int& directorSize();
	std::string getDrawingAddress();
	std::string getDrawingName();

private:
	std::vector<int>findCharLocNum(const std::string& str, const char& ch);//�����ַ�����ĳ�ַ����ֵĸ����Լ�λ��
	bool setAddress(const std::string& str, const int& loc);//��ʼ��ͼֽ��ַ
	bool setFileName();//��ʼ��ͼֽ����
	bool setDrawingDirectory(const std::string& str, const std::vector<int>& chLocVec);
	bool setDrawingType();//��ʼ��ͼֽ����


private:

	//std::string _drawingParameter;//ͼֽ����
	std::vector<std::string>_directory;//ͼֽĿ¼ ��ʱ��Ŀ¼���ж�㣬��಻�ᳬ������
	std::string _address;//ͼֽ�ĵ�ַ
	std::string _fileName;
	DrawingType _drawingType;


};

class Drawing
{
public:

	typedef enum
	{
		COLUMNLOC,//��λ��
		BEAMLOC,//��λ��
		WALLLOC,//ǽλ��
		FLOORLOC,//��λ��
		ELEVATION,//���
		BEAMFLAT,//��ƽ��
		AXISES,//����
		SCD,//section column detail ����ͼ
		SBD,//section beam detail ����ͼ
		UNKNOW
	}ResultValue;

public:
	//����ӿ�
	Drawing()
	{}
	Drawing(const std::shared_ptr<Transer>& _spDraTranser,
			const std::shared_ptr<Parser>& _spDarParser,
			const std::string& _drawingParameter);

	//************************************
	// ˵��	: �����ѻ�����������ǽ����ֱ�������Ϣ
	//************************************
	bool parserDrawing(/*const Drawing::DrawingType& drawingType*/);
	//************************************
	// ˵��	: ���ص�ǰͼֽ�����ļ�ID
	//************************************
	//std::string returnFileID();

	//************************************
	// ˵��	: �����ļ��������ݵ�����
	//************************************


private:
	//************************************
	// ˵��	: �����ѻ�����������ǽ����ֱ�������Ϣ
	//************************************
	//bool setDrawingType(const Drawing::DrawingType& drawingType);
	//************************************
	// ˵��	: ���ɵ�ǰͼֽ����ļ�ID
	//************************************
	//bool setFileID();
	//************************************
	// ˵��	: ��������json�ļ�
	//************************************
	bool setClueFile();
	Json::Value readClueFile(const std::string& fileName);
	bool writeClueFile(const Json::Value& rootValue, const std::string& fileName);
	Json::Value writeClueAxis(const Json::Value& rootValue);
	Json::Value writeClueColumn(const Json::Value& rootValue);
	Json::Value writeClueBeam(const Json::Value& rootValue);
	Json::Value writeClueWall(const Json::Value& rootValue);
	Json::Value writeClueFloor(const Json::Value& rootValue);
	//************************************
	// ˵��	: ���ͼֽʶ�������Ϣö��
	//************************************
	bool setResultMap(const ResultValue& valueType, const std::string& fieldName);

private:
	/*������������Ϣ���Լ������Ϣд���ļ�*/
	//************************************
	// ˵��	: д�������Ϣ
	//************************************
	bool writeParserProcessInfo();
	//************************************
	// ˵��	: д������Ϣ
	//************************************
	bool writeParserResultInfo();
	//************************************
	// ˵��	: ��ִ�������������Ŀ¼���ַ�������ݲ�������ͼֽ���ͽ��н���
	//************************************
	//bool separationParameter();
	//************************************
	// ˵��	: ��ʼ������
	//************************************
	bool setAxis();
	//************************************
	// ˵��	: �ļ�����Ŀ¼
	//************************************
	bool creatFileDirectory(const std::vector<std::string>& directorVec);
	//************************************
	// ˵��	: ��ʼ��ͼֽ����
	//************************************
	//bool setDrawingType();//�����ʼ��
	//bool chengeDrawingType();//���ݾ�������仯ͼֽ����

	//************************************
	// ˵��	: ����ͼֽ���;������ͼֽ������
	//************************************
	bool specificParserDrawing();
private:
	//�������ĳ��ͼֽ

	//************************************
	// ˵��	: �������ṹƽ��ͼ
	//************************************
	bool parserSMP();
	//************************************
	// ˵��	: ��������ͼ
	//************************************
	bool parserCSSD();
	//************************************
	// ˵��	: �����������ͼ
	//************************************
	bool parserMSSHP();

private:

	/*��λ��*/
	bool parserColumnLoc();
	/*��λ��*/
	bool parserBeamLoc();
	/*ǽλ��*/
	bool parserWallLoc();
	/*��λ��*/
	bool parserFloorLoc();
	/*��������ͼ*/
	bool parserColumnSection();

private:
	//������ͼֽ���õ�����д���Ӧ���ļ�

	//************************************
	// ˵��	: д������ṹƽ��ͼ�Ľ������
	//************************************
	bool writeSMP(Json::Value& resuleData);
	// ˵�� : д����������ͼ�������
	bool writeCSSD(Json::Value& resuleData);
private:
	//��ӦJson����

	/*����*/
	Json::Value axisesLoc();
	/*��λ��*/
	Json::Value columnLoc();
	/*��λ��*/
	Json::Value beamLoc();
	/*ǽλ��*/
	Json::Value wallLoc();
	/*��λ��*/
	Json::Value floorLoc();

private:
	/*json reader writer*/
	ResultWrite _resultWrite;



private:

	std::shared_ptr<Transer> _spDraTranser;//��ʾdra ��ʾͼֽdrawing �洢drawing��transer����
	std::shared_ptr<Parser> _spDarParser;//�洢drawing��parser����
	std::string _drawingParameter;//ͼֽ����
	IniDrawingType _iniDrawingType;//ͼֽ������Ŀ¼

	std::map<ResultValue, std::string>_resultMap;//��ʶ����Ľ����Ϣö��д���ֵ����ʽ������д�������ļ�
private:
	//!Axis
	std::vector<std::shared_ptr<Axis>> _axises;//����

/*����������*/
	PillarSet _columnLocSet;//column location��������λ����Ϣ
	std::vector<std::shared_ptr<Pillar>> _spColumnVec;//���е���λ����Ϣ

	ColumnSectionSet _columnSectionSet;//������������Ϣ
	std::vector<std::shared_ptr<ColumnSection>>_spColumnSectionVec;//����������Ϣ

/*����������*/
	BeamSet _beamLocSet;//��λ����Ϣ
	BeamLongitudinalSet _beamLongitudinalSet;//������ֽ�����
	BeamSectionSet _beamSectionSet;//���������
	std::vector<std::shared_ptr<Beam>> _spBeamLocVec;//������λ����Ϣ

/*ǽ��������*/
	WallLocationSet _wallLocSet;//ǽλ��
	std::vector<std::shared_ptr<WallLocation>>_spWallLocVec;//���е�ǽ


/*���������*/

/*��ÿ��ͼֽ���ɶ�Ӧ*/

};


#endif