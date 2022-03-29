#ifndef __PARSER_UTILS_PILLARSET_H__
#define __PARSER_UTILS_PILLARSET_H__
#include <iostream>
#include "pillar.h"
#include "publicFunction.h"
class ColumnsTextPoint
{
public:
	ColumnsTextPoint(const Point&point, const std::string &name, bool bLeadLine = false);
public:
	Point m_point;//�ı���
	bool m_bLeadLine;//�ж��ı����Ƿ���������
	std::string m_name;//�ı�������
};
class ColumnsLeadGroup
{

public:

	ColumnsLeadGroup(const std::vector<int>&nowIndex,
		int textPointIndex, bool pair = false);
public:
	bool m_pair;//�Ƿ����
	int m_textPointIndex;//�ı��������
	//std::vector<int>m_primevalIndex;//ԭ����
	std::vector<int>m_nowIndex;//��ǰ����
	std::vector<int>m_cornerIndex;//����ͼ���ཻ������

};
class ColumnsBorderGroup
{
public:
	typedef enum _BorderGroupType
	{
		FLEAD,
		FGROP,
		FTEXTPOINT
	}BorderGroupType;
public:
	ColumnsBorderGroup(bool bTextPoint = false, bool bgrop = false, bool blead = false);
	friend bool operator==(const ColumnsBorderGroup& borderGrop1, const ColumnsBorderGroup &borderGrop2)
	{
		if (borderGrop1.m_blead != borderGrop2.m_blead||
			borderGrop1.m_bgrop != borderGrop2.m_bgrop||
			borderGrop1.m_btextPoint != borderGrop2.m_btextPoint)
		{
			return false;
		}
		if (borderGrop1.m_borders != borderGrop2.m_borders ||
			borderGrop1.m_leadGropsIndex != borderGrop2.m_leadGropsIndex ||
			borderGrop1.m_textPoints != borderGrop2.m_textPoints)
		{
			return false;
		}
		return true;
	}
public:
	bool m_blead;
	bool m_bgrop;
	bool m_btextPoint;
	std::vector<int>m_borders;
	std::vector<int>m_leadGropsIndex;//��һ����ʾleadGroping������
	std::vector<int>m_textPoints;

};
class PillarSet
{
public:
	bool setMember(const std::shared_ptr<Axis> &axises);
	bool columns(Data&data, std::vector<std::shared_ptr<Pillar>> &_pillars/*, std::vector<int> &temp_index,
		std::vector<Line>&temp_line, std::vector<Point>& corner*/);
public://��ʽ�㷨
	bool findColumns(Data&data, std::vector<std::shared_ptr<Pillar>> &_pillars);
	//���س�������ӱ߽�����������n�Գ���߽磬ÿ����n���߽�
	std::vector<std::pair<int, int>> returnColumnBorderGroupIndex();
private:
	/*�������߽�ͼ��*/
	bool findColumnLayer(LineData& lineData,CornerData& cornerData);
	/*�������е���ͼ����*/
	bool findColumnLines(LineData&lineData);
	/*���Ҷ�Ӧ���ı���*/
	bool findTextCandidatesPoints(TextPointData& textPointData);
	/*�ı����Ӧ��������*/
	bool findLeadLines(Data&data);
	bool findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, int &cornerIndex);//�����������������
	/*��ʼ����ͼ������*/
	bool initializationBeamLinesCorners();
	/*��ʼ���߽����*/
	bool findBorderGrop(Data&data);
	//�Ż�borderGrop����������
	bool optFBorderGrop(std::vector<int>&temp_index);//opt ָoptimization
	bool optFLTextPoint(std::vector<int>&test_index);//�Ż�û�������ߵ��ı���

	/*����ĳLeadLine�����ĸ���������*/
	int returnLeadGropIndex(const int &leadLineIndex);
	/*��Ϸ�ʽ*/
	//��ʼ��δ�������borderLine,leadLine,textPoint;
	bool initializationColumnType();
	/*�������ƽ����ߣ����ڹ����໥�����ӵ����*/
	bool returnColumnAverageWidthHeight();
	//�Ż����鵫ȴ�����������ε����

	/*�ı���Ϣ����*/
	//�Ƿ�������ᡱ��ʶ��
	bool checkAxisExist(const std::string& beamText, int& axisIdx);
	/*����Ϣ����*/
	bool columnInformation(std::vector<std::shared_ptr<Pillar>> &_pillars);
	//��ʼ��Rtree
	//bool pushRTreeColumnLines(const Line&line, const int &index);

private:
	
	bool initializationLeadLineMap();
	bool initializationBorderLineMap();


public://��ʽ�㷨
	

	//��ʼ����
	bool columnPush(std::vector<std::shared_ptr<Pillar>> &pillars, const std::string &str, const std::vector<int>& vl,
		const std::vector<int> &hl, const std::vector<int> &sl, const std::vector<int> &ls, LineData &lineData);
	//��û�н�������������ӳ�
	bool extendLeadLine(std::vector<Line> &lines, const std::vector<int> &ls, LineData &lineData, CornerData &cornerData);
	/*****************************/
	/*��ʼ����������֮��ľ���*/
	bool SetHaxis(AxisMark &axis_mark_h, const std::string &str, const double &length);
	bool SetVaxis(AxisMark &axis_mark_v, const std::string &str, const double &length);
	bool FindPillarNearAxis(Pillar &pill); //Ѱ�������������
    //���ҽ�С����
	bool findSmallHColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &haxis_mark);
	bool findBigHColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &haxis_mark);
	bool findSmallVColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &vaxis_mark);
	bool findBigVColumn(Pillar &pill, const std::shared_ptr<Axis> axise, std::vector<AxisMark> &vaxis_mark);


private://��ʽ�㷨
	std::shared_ptr<Axis> m_axises;
private://��ʽ�㷨
	
	std::string _columnLayer;//���߽�ͼ��
	LineData m_columnLines;//��ͼ��������߰���������
	CornerData m_colunmCorners;//��ͼ���ߵ����н���
	std::vector<ColumnsTextPoint>m_textPoint;//���ı���
	std::vector<ColumnsLeadGroup>m_leadGrops;//��������
	std::vector<ColumnsBorderGroup>m_BorderGrops;//��������
	std::vector<int>m_primevalIndex;//ԭʼborder����
	std::vector<int> m_borderLines;
	std::vector<int>m_LeadLines;
	double m_averageWidth;
	double m_averageHeight;
	
	//���ڿ��ٲ��ҵ������ֵ�
	/*leadLines��borderLines���ֵ����ڿ��ٲ���*/
	std::map<int, int>m_mapLeadLines;//ǰһ����һ������ʾleadLine�������ţ����ڿ����жϴ����Ƿ�Ϊ������
	std::map<int, int>m_mapBorderLines;//ǰһ����һ������ʾborderLine�������ţ����ڿ����жϴ����Ƿ�Ϊborderline
	std::map<bool, std::vector<int>>m_leadGropsType;
	std::map<bool, std::vector<int>>m_textPointType;
	std::map<ColumnsBorderGroup::BorderGroupType, std::vector<int>>m_borderGropsType;
	std::map<int, int>m_mapBorderGropIndex;//ǰһ����ʶcolumnLine������������һ����ʾ����������
	std::map<int, int>m_mapLeadGropIndex;//ǰһ����ʾleadLine����������һ����ʾ�����������
	//����Rtree
	RTreeData m_rTreeColumnLines;

	
};
#endif