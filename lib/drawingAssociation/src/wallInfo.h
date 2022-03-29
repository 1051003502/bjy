#ifndef __DRAWINGRELATED_WALLINFO_H__
#define __DRAWINGRELATED_WALLINFO_H__
#include "RWJsonFile.h"
#include "wallProcessInfo.h"
class WallInfo
{
public:
	//interface funcation
	bool findWallPathJsonValue(const Json::Value& wallJVPath);
	bool readWallLocInfor();


private:
	const std::string setFloorPath(const std::string& type, const std::string& field);//���ö�ȡ�������ݵ�·��
	std::vector<WallLoc> setWallLocJV(const Json::Value& wallLocValue);//����λ��Json���ݽ���Ϊ�ڴ�����

private:
	Json::Value _wallLocJV;//ǽλ��
	Json::Value _wallStirrupJV;//ǽ���
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> _drawingLocMap;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳ���ĳ��ͼֽ����������ʾĳ���ֶ�
	std::map < std::string, std::vector<std::pair<std::string, std::vector<WallLoc>>>>_drawingWallLocVec;//��һ��string��ʾĳ�㣬�ڶ�����ʾĳͼֽ��vec��ʾ���б�
	ResultWrite resultWrite;//����д���������Ķ���
	ResultRead resultRead;//���ö�����̽�������Ķ���
	std::vector<std::string>_floorVec;//
};
#endif