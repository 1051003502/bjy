#ifndef __DRAWINGRELATED_COORDINATESYSTEM_H__
#define __DRAWINGRELATED_COORDINATESYSTEM_H__
#include "parser.h"
#include "spdlog/spdlog.h"
#include "RWJsonFile.h"
typedef std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>> ProjectFile;


/*!
 * \class CoordinateSystem
 *
 * \brief ȫ������ϵ�����ã�
���ڽ���ͼ��˵��һ����������ͼֽ������ָ����ͼֽ��һ���ɶ��Ų��ص㲻ͬ��ͼ�໥���乹�ɣ�
���ڵ�ȫ���������൱�ڸ�ͼֽ�����еĹ������ṩһ��ͳһ��ƽ̨��ÿ��ͼֽ������������ݽ���ӳ�䣬
����ֻ��Թ���������Ӳ��䣬������Ҫ�������ҡ�
 *
 * \author Xiong
 * \date 14/2/2020
 */
class CoordinateSystem
{

	/************************************���㷨**********************************/
public://interface function
	//���������Ϣ�洢�ֶε�·��
	bool findAxisPathJsonValue(const Json::Value& columnJV);
	//�Ӿ����ļ�����������Json����
	bool readAxisCoordinateSystem();
	//����������������ݣ�
	bool outputAxisJsonData();
	//������Ϣ����ļ�·��
	std::string constAxisPath();

	
	

private:
	const std::string setFloorPath(const std::string& type, const std::string& field);//���ö�ȡ�������ݵ�·��
	std::vector<AxisProcessInfo> setColumnLocJV(const Json::Value& axisValue);//����λ��Json���ݽ���Ϊ�ڴ�����
	bool mergeAxis();//У������
	bool checkAxis(const std::vector<AxisProcessInfo>& drawingAxisVec);//У��������
	bool checkHAxis(const std::vector<SingleAxisInfo>& hAxisVec, std::vector<SingleAxisInfo>& goalHAxisVec);//У�������ĺ���
	bool checkVAxis(const std::vector<SingleAxisInfo>& vAxisVec, std::vector<SingleAxisInfo>& goalVAxisVec);//У������������
	bool checkSameName();//����ͬ��������
	bool checkAxisDistance();//��������ϵ�ڣ����С����ʮ����
	bool outputAll();//ȫ�����
	bool outputError();//������Ϣ���
private:
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳ���ĳ��ͼֽ����������ʾĳ���ֶ�
	std::map < std::string, std::vector<std::pair<std::string, std::vector<AxisProcessInfo>>>>_drawingAxisVec;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳͼֽ��vec��ʾ���б�
	std::vector<std::string>_floorVec;
	ResultWrite _resultWrite;//����д���������Ķ���
	ResultRead _resultRead;//���ö�����̽�������Ķ���
	AxisProcessInfo _goalAxisSyetem;
	std::vector<std::vector<std::string>>_lessThanError;//����С����
	std::vector<std::string>_sameNameError;//����С����

	/***************************************************************************/
public:
	//************************************
	// ����	: CoordinateSystem
	// ȫ��	: CoordinateSystem::CoordinateSystem
	// ����	: public 
	// ����	: 
	// ˵��	: actor
	//************************************
	CoordinateSystem() {}
	//************************************
	// ����	: iniAxisMap
	// ȫ��	: CoordinateSystem::iniAxisMap
	// ����	: public 
	// ����	: bool
	// ����	: _axisMap
	//! ˵��	: ���ڳ�ʼ��_axisMap
	//************************************
	//bool iniAxisMap(const std::map<std::string, std::shared_ptr<Axis>>&_axisMap);

	//************************************
	// ����	: iniGlobalCoordinateSystem
	// ȫ��	: CoordinateSystem::iniGlobalCoordinateSystem
	// ����	: public 
	// ����	: bool 
	// ˵��	: ����������໥����������������ȫ������ϵ,����Ϊ����ͼֽ��
	//************************************
	bool iniGlobalCoordinateSystem();
	//!ת��ȫ������ϵ��ĳͼֽ����ϵ
	bool fromGlobalToDrawing();
	//!ת��ͼֽ����ϵ��ȫ��
	bool fromDrawingToGlobal();
public:
	std::map<std::string,double> vlines_cooMap;//!��һ��������ʾ���������֣��ڶ���������ʾ���λ��
	std::map<std::string,double> hlines_cooMap;//!ͬ�ϣ���һ����ʾ���ᣬ��һ����ʾ����

	std::string referenceAxisX;//!������x
	std::string referenceAxisY;//!������y

	std::map<std::string, std::shared_ptr<Axis>>axisMap;//!�ֱ�Ϊû��ͼֽ��Ӧ������
	
	std::map<std::string, std::pair<std::string, std::string>>referenceAxis;//!ÿ��ͼֽ������ȫ�������ڵĲ�����secont.firstΪ����second.secondΪ����


};

#endif // !1

