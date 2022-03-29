#ifndef __DRAWINGRELATED_COLUMNINFO_H__
#define __DRAWINGRELATED_COLUMNINFO_H__
#include "json/json.h"
//#include "pillarSet.h"
//#include "ColumnSectionSet.h"
//#include "columnProcessInfo.h"
#include "RWJsonFile.h"



/*!
* \class columnLocationW
*
* \brief �������ڼ�����������λ����Ϣ
*
* \author Xiong
* \date 15/2/2020
*/
class columnLocationW
{
public:
	columnLocationW(const std::string& name,
				   const PillarDistance& hDistance,
				   const PillarDistance& vDistance, unsigned int Id = 0);
	friend bool operator==(const columnLocationW& _column1, const columnLocationW& _column2);
public:
	unsigned int columnSectionId;
	std::string name;//������
	PillarDistance hDistance;//��������ĺ��ᣬPillarDistance::second.first Ϊ�º��� PillarDistance::second.second�Ϻ���
	PillarDistance vDistance;//��������������ᣬPillarDistance::second.first Ϊ�º��� PillarDistance::second.second������



};
/*!
* \class ColumnSection
*
* \brief
*
* \author Xiong
* \date 15/2/2020
*/
class ColumnSectionInfo
{
	struct SideBar
	{
		std::string aSideBar;//�����ݽ���Ϣ
		std::string bSideBar;//b ���ݽ�
		std::string hSideBar;//h ���ݽ�
		std::string cornerBars;//�Ž�
	};
	struct ColumnStirrup
	{
		int hCount;//����֧��
		int vCount;//����֧��
		std::string aStirrupInfo;//���������б�ʶ������Ϣ��������Ϣ
		std::vector<std::string>configuration;//������Ϣ������Ϣ
	};
public:
	std::string name;//����������
	std::string scal;//���������
	unsigned int section_id;//������ID
	bool bRectangle;//�ж��������Ƿ�Ϊ������
	double width;//������Ŀ�
	double height;//�������

	SideBar bars;//�ݽ���Ϣ
	ColumnStirrup stirrup;//������Ϣ
						  //std::string stirrups;//�����ͺ�
	std::string others;//�������Ϣ


};

class ColumnInfo
{
/************�½ṹ��������**************/
#if 1
/************interface function*****************/
public:

	bool findColumnPathJsonValue(const Json::Value& columnJV);
	//�Ӿ����ļ������������Json����
	bool readColumnLocInfo();
	//������ӵ��������ݣ��ֱ���λ�����ݣ��ֽ����ݣ�У�������
	bool outputColumnJsonData();
/****************private function*******************/
private:
	
	const std::string setFloorPath(const std::string& file, const std::string& field);//���ö�ȡ�������ݵ�·��

	std::vector<ColumnLoc> setColumnLocJV(const Json::Value& columnLocValue);//����λ��Json���ݽ���Ϊ�ڴ�����
	//bool setColumnSectionJV(const Json::Value& columnSectionValue);
	bool checkSameFloorColumnLoc();//У��ͬһ�㣬��ͬͼֽ����λ�����⣬����У�����

	bool outputErrorLocData(const std::string& basePath);
	bool outputColumnLocData(const std::string& basePath);
	bool outputColumnSectionData(const std::string& basePath);

	bool singleLayerFloor(const std::string& floorPlan, const std::vector<ColumnLoc>& columnLocVec);//��ĳ��ֻ��һ��ͼֽʱ
	std::string setWriteFile(const std::string& floorFile);//�����ж�ĳ�����ļ����Ƿ���ڣ����ؾ���·��
	
private:
	Json::Value _columnLocJV;
	Json::Value _columnSectionJV;
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳ���ĳ��ͼֽ����������ʾĳ���ֶ�
	std::map < std::string, std::vector<std::pair<std::string, std::vector<ColumnLoc>>>>_drawingColumnLocVec;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳͼֽ��vec��ʾ���б�
	std::map < std::string, std::vector<ColumnLoc>>_unkonwNameColumnVec;//��֪�����ֵ�����λ����Ϣ
	std::map < std::string, std::vector<ColumnLoc>>_notSameColumnLocVec;//������ͬ��������ͬ
	std::map < std::string, std::vector<ColumnLoc>>_outputColumnLocVec;//û�����������ȷ�����λ����Ϣ
	ResultWrite resultWrite;//����д���������Ķ���
	ResultRead resultRead;//���ö�����̽�������Ķ���
	std::vector<std::string>_floorVec;
public:
	std::map<std::string, std::vector<std::shared_ptr<Pillar>>>_pillarVecMap;//��¼ÿ��ͼֽ�е���λ����Ϣ


#endif

};
#endif