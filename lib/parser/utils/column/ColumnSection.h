#ifndef __PARSER_UTILS_SECTION_H__
#define __PARSER_UTILS_SECTION_H__
#include "line.h"
#include "dimension.h"
#include <iostream>
#include <fstream>
#include "block.h"
#include "json/json.h"
#include <stdlib.h>
#include <functional>
#include "data.h"
#include "publicFunction.h"



typedef enum ColumnDirection
{
	L1, //��
	R1, //��
	U1, //��
	D1, //��
	N1  //��
} ColumnDirection;

class ColumnBreakPoint
{
public:
	Point center_mind;
	double radius;
	int index;
};

class ColumnBPInfo
{
public:
	typedef enum _Direction
	{
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	}Direction;
public:
	std::vector<int>bpLeadVec;//�ݽ������������
	std::vector<int>bpVec;//�ݽ�
	std::vector<std::string>bpInfoVec;//�ݽ���Ϣ
	std::vector<int>bpInfoPointIndex;//��Ϣ��Ӧ���ı�������
	Direction dir;

};

class ColumnStirLeadInfo
{
public:
	std::vector<int>stirLeadVec;//��������
	//std::vector<int>stirVec;//�빿�����ߵ��ཻ�߹�����
	std::string stirInfo;//����������������Ϣ
	Point textPoint;//�ı���
	bool bSingle;
};


class ColumnStirrupHook
{
public:
	ColumnStirrupHook() 
	{
		hook_index1 = -1;
		hook_index2 = -1;
	}
	ColumnStirrupHook(const int& ide1, const int& ide2)
	{
		this->hook_index1 = ide1;
		this->hook_index2 = ide2;
	}
	ColumnStirrupHook& operator=(const ColumnStirrupHook&hook)
	{
		this->hook_index1 = hook.hook_index1;
		this->hook_index2 = hook.hook_index2;
		return *this;
	}
	bool operator==(const ColumnStirrupHook&hook)
	{
		if (((this->hook_index1 == hook.hook_index1&&
			this->hook_index2 == hook.hook_index2)||
			(this->hook_index1 == hook.hook_index2&&
				this->hook_index2 == hook.hook_index1)))
			return true;
		else
			return false;
	}
public:
	int hook_index1;
	int hook_index2;

};

class ColumnSStirrup
{
public:
	ColumnSStirrup() 
	{
		single = false;
	}
	ColumnSStirrup(const std::vector<Line>&c_l, const ColumnStirrupHook&hook,const bool &single)
	{
		this->circle_line = c_l;
		this->hook = hook;
		this->single = single;
	}
	ColumnSStirrup(const Line& line)
	{
		this->circle_line.push_back(line);
		this->single = true;
	}
	ColumnSStirrup&operator=(const ColumnSStirrup&s_l)
	{
		this->circle_line = s_l.circle_line;
		this->hook = s_l.hook;
		return *this;
	}
public:
	bool single;
	std::vector<Line> circle_line;	//����
	ColumnStirrupHook hook;
};

class ColumnSection
{
public:

public:
	typedef std::vector<int>Connected;
	typedef std::pair<std::string, Connected>Explanation;//�����ߵĽ���
	typedef std::pair<double, double>Dim_Information;//��һ��������ʾ���򣬵ڶ���������ʶ����
	typedef std::pair<std::string, int>Stirrup;//������ͺź͹������Ŀ

	typedef struct _BreakPoint
	{
		Point center_mind;
		double radius;
	}BreakPoint;

	ColumnSection() {
		linear_ration = 1;
		section_id = -1;
		_bCircle = false;
		_rectangle = true;
		_maxHeight = 0;
		_maxHeight = 0;
	}
	ColumnSection(std::vector<BreakPoint>temp_bp, std::vector<Dimension>dimensions);
	bool SetBreakPoint(BreakPoint &bp, const Point& p, const double &r);
	double GetPillarWidth();
	bool SetSectionWidthHeight();
	bool SetSectionbox();
	bool SetSectionOverallWH();			//
										//bool SectionInformatioOutput();
										//
										//��ʼ����������
	bool SetSectionBlockName(const Block &section_block);
	bool SetSectionBlockBox(const Block&section_block);
	bool QuickSortLine(std::vector<Line>&lines, int left, int right, const char&ch);
	int DivisionLine(std::vector<Line>&lines, int left, int right, const char&ch);
	bool sortExplanation(std::vector<std::pair<Explanation, Point>>&explanation);
	long setSectionId(const std::string &sectionName);
	bool boolRectangle(bool *type_L);		//��״����
	std::vector<Point> sortSurroundLine();
	bool DescribeShape(std::string &str, const bool &type_L); //��������״�ֲ�
	bool setRectangleBH(Json::Value& Bars, const bool&type_l);	//���ö����b,h �๿��
	//************************************
	// ����	: setRectangleBH
	// ȫ��	: ColumnSection::setRectangleBH
	// ����	: public 
	// ����	: bool
	// ����	: bSide b ������
	// ����	: hSide h ������
	// ����	: type_l
	// ˵��	: �����жϾ��������ݽ��h����b��
	//************************************
	bool setRectangleBH(std::string&bSide, std::string&hSide, const bool &type_l);
	//************************************
	// ����	: setStirrupHookCount
	// ȫ��	: ColumnSection::setStirrupHookCount
	// ����	: public 
	// ����	: bool
	// ����	: hCount ��֧
	// ����	: vCount ��֧
	// ˵��	: ���ڳ�ʼ���������֧��
	//************************************
	bool setStirrupHookCount(int &hCount, int &vCount);
	bool SetListStittups();//�����������Ϣ
	bool del_char(std::string &str);//ɾ���ַ����е��ض��ַ�
	//std::string numToString(const double&num);
	Json::Value OutPut();//��������Ϣ
	Json::Value OutPutInSitu();//ԭλ��ע
	Json::Value OutPutCenter();//���б�ע
public:

	bool SetSectionScale(std::string &scale, const Box&section_box, Data& data);//��ʼ�������ߴ�

	bool SetSectionStirrup(std::vector<ColumnSection::Stirrup> &stirrup, const Box &section_box, Data& data);//��ʼ��������Ϣ
																									   //�ԶϿ��Ĺ����־������
	bool amendHooks(const int hook1, const int hook2, std::map<int, std::vector<Corner>>::const_iterator corners1,
		std::map<int, std::vector<Corner>>::const_iterator const corners2, std::vector<Corner>& corner1,
		std::vector<Corner> &corner2, Data& data);
	//��ʼ��section��breakPoint��߽�
	std::vector<ColumnSection::BreakPoint> ReturnSectionTopAngle(const ColumnSection &section);
	bool SetSectionBreakPointSurround(ColumnSection &section, Data &data);
	bool SetSectionHook(std::vector<ColumnStirrupHook> hooks, ColumnSection &section, Data &data);

	bool SetSectionBox(ColumnSection &section, Data &data);//���ù���İ�Χ��

	bool SetSectionStirrupReferencePoint(ColumnSection &section, Data &data);//���ù���Ĳο���
																	   //��ʼ��������Ϣ
	bool SetSectionStirrupInformation(ColumnSection &section, Data &data);
	/*�����ʶ��*/

	bool InTurnLinkStirrup(ColumnSection &section, Data &data);//������ͨ��ƥ�乿��

	bool DirecctionMatchingStirrup(ColumnSection &section, Data &data);//���ݷ���ƥ�乿��

																 //�����й���Ļ������ٴ�У�鵥֧����������Լ������־����׼�����
	bool CheckStirrup(ColumnSection &section, Data &data);

	bool FindLongitudinalTendonCollection(const std::vector<Point> &center_mind,
		std::vector<std::vector<int>> &connect_line, Data &data);//�ݽ�ļ�����
	bool SetLTC(std::vector<std::vector<int>> &l_t_c, const std::vector<std::vector<int>> &l_t_c1);

	bool FindExplanation(const std::vector<std::vector<int>>&lines_index, const Box&temp_box,
		const Box&block_box, std::vector<std::pair<ColumnSection::Explanation, Point>>&explanation, Data &data);//���ҹ�����������Ϣ

	bool RetrunLeadLineType(const std::vector<int> &index, const Box &temp_box, Point &goal_point, Data &data);//�ж������ߵ����ͣ�����trueΪ�պ��ͣ�����ɢ��
	bool SetSectionExplanation(std::vector<std::pair<ColumnSection::Explanation, Point>>&explanation,
		const std::vector<std::pair<ColumnSection::Explanation, Point>>&lead_line_explanation);
	bool CenterNote(ColumnSection &section, Data &data);

public:
	std::vector<BreakPoint>break_point;		//���й���Ķϵ���Ϣ
	std::vector<Dimension>dimensions;		//��ע��Ϣ
	std::vector<int>lead_line;				//������
	std::vector<std::vector<int>>l_t_c;		//����������
	Box section_block_box;					//������Ŀ鷶Χ ---������ͼ��obx
	Box section_box;						//����ͼ�Ŀ鷶Χ ---�����box
	std::vector<int> pillar_surround;		//���ӵİ�Χ�� -----���Ӷ����box
	Point stirrup_refer_point;				//����Ĳ��յ�
	std::vector<ColumnStirrupHook>hooks;			//����Ĺ�
	std::vector<int>bp_surround;			//�ݽ��Χ��
	std::vector<int>stirrup_lines;          //��ѡ������
	LineData line_data;
	CornerData corner_data;
public:
	//ԭλעд
	std::string name;
	std::string scale;						//����
	std::vector<ColumnSStirrup>stittups;			//���� ��*�� ���λ�� ֧��
	std::vector<std::string>list_stittups;	//�����������Ϣ
	double all_section_width;				//����Ŀ�
	double all_section_height;				//����ĸ�
											//Box stirrup_refer_box;				//������պ�
	std::vector<std::pair<Explanation, Point>>explanation;	//�����߽���
	std::vector<Stirrup>stirrup_model;		//�����ͺ���Ϣ�����������Ϣ
	std::vector<Dim_Information>u_width;	//�ϲ�������ע
	std::vector<Dim_Information>d_width;	//�²�������ע
	std::vector<Dim_Information>l_height;	//�󲿶���ĸ߱�ע
	std::vector<Dim_Information>r_height;	//�Ҳ�����ĸ߱�ע
	unsigned int section_id;						//����Id
public:
	//����עд
	typedef enum {
		L,//��
		R,//��
		U,//��
		D, //��
	}Direcction;

	typedef enum {
		LEFT,//��
		RIGHT,//��
		UP,//��
		DOWN, //��
	}Direction1;

	std::vector<std::pair<std::string, Direcction>>bp_string;
	std::vector<std::string>all_txt;


protected:
	double linear_ration;//���Ա���

/************************************���㷨******************************************/
public:
	//public function 
	static ColumnSection retrunColumnSection(Block& block, Data& data);
	bool testFuction(std::vector<int>&test_indexVec,std::vector<Point>&cornerVec);
	Json::Value outputColumnSection();
private:
	//private function
	bool iniColumnBlock(const Block& _columnBlock);
	//��ʼ���������ڵ����жϵ�
	bool iniBreakPoint(const Block& block, Data& data);
	//�����Ƿ������ͬ�Ķϵ㣬��Ϊ���ݽ�Ķϵ��ɶ��������ǣ������������ظ�������
	bool findBreakPoint(const Point& p);
	//��ʼ��������Ϣ
	bool iniStirrup(Data& data);
	//��ʼ�����й��������
	bool iniStirrupIndex(LineData& _lineData);
	//���ع����ͼ����Ϣ
	bool iniStirrupLayer(LineData& _lineData);
	//����һ���������ݽ��γɵİ�Χ��
	Box bpSurroundBox(LineData& _lineData);
	//��ʼ�������������й���ı�־��
	bool iniColumnHook(LineData& lineData, RTreeData& goalRTree);
	// ˵��	: ���Ѿ�ʶ����Ĺ����ߣ��ٴ�ϸ����ϸ��Ϊ���ߣ����ߣ�б�ߣ����ҶԴ˽�������ϲ�
	bool divideStirLineType(std::vector<int>& v_lines,
		std::vector<int>& h_lines,
		std::vector<int>& ss_lines,
		Data& data);

	// ��ʼ�������
	bool setStirrupDirection(std::vector<int>& _vLines,
		std::vector<int>& _hLines,
		std::vector<int>& _sLines,
		Data& _data);
	// ˵��	: �����з���Ĺ����룬δ������Ĺ���������
	bool makePairStir(std::vector<std::pair<int, int>>& hStirPair,
		std::vector<std::pair<int, int>>& vStirPair,
		const std::vector<int>& v_lines,
		const std::vector<int>& h_lines,
		Data& data);
	// ˵��	: ��һ�����ֹ���ķ�����
	bool divideStirrupDir(const std::vector<int>& lineIndex,
		std::map<int, ColumnDirection>& goalIndex,
		const char& ch);
	// ˵��	: У�鹿��
	bool checkStir(Data& data);
	// ˵��	: ���Ѿ���ԺõĹ���������
	ColumnSStirrup checkStirPair1(std::vector<int>stirDirIndex, LineData& _lineData);
	// ˵��	: ���Ѿ���ԣ������������־������ԣ�����ȷ��У��
	ColumnSStirrup checkStirPair2(std::vector<int>stirDirIndex, Data& data);
	// ˵��	: Ϊ�Լ�������Ĺ��ƥ���Ӧ�Ĺ���������Ŀ�����ں�����֧�����ж�
	bool findSameStir(const Line& _line, int& lineIndex, LineData& _lineData);
	// ˵��	: ���������ڵĵ�֧�������ų��ķ�ʽ�����й������ҵ��ҵ�
	bool findColumnSitrrupSignal(Data& data);
	// ˵��	: �������������������ݽ������
	bool findBpLeadLine(Data& data);
	// ˵��	: ��ʼ��������Ĺ����ע��Ϣ
	bool iniColumnStirLead(Data& data);
	// ˵��	: ���빿���������,��������ڶ������ʱ
	std::vector<int> divideStirLead(LineData& _lineData);
	// ˵��	: ��ʼ�������湿����Ϣ
	bool iniRebarInfo(Data& data);
	// ˵��	: ��ʼ������������Ӧ����Ϣ���Լ������߶�Ӧ���ı���
	bool iniBeamLeadInfo(std::vector<int>& textPointIndex, Point& refPoint, const std::vector<int>& _lineIndex, const bool& _bpLead, Data& data);
	// ˵��	: �ڼ������������ҵ����ʵ��ı�������
	bool findColumnLeadInfoPoint(Point& point, double& searchR, const std::vector<int>& _lineIndex, const bool& _bpBox, Data& data);
	// ˵��	: �ж������ߵķ�������У���ı��������ߵ�ӳ���Ƿ���ȷ�������Ա�����һ�����һ����ƽ��
	bool returnLeadLineDir(char& ch, const bool& _bpBox, const std::vector<int>_leadLineIndex, Data& data);
	// ˵��	: ���ù����ע������Ϣ���������Ƿ�Ϊ��ֻ��
	bool setStirInfoType(Data& data);
	// ˵��	: ���ù����ע������Ϣ���������Ƿ�Ϊ��ֻ��
	bool setBPLeadInfoDirection(LineData& _lineData);
	// ˵�� : Ϊ�ݽ��������
	bool sortBreakPointToLine();
	// ˵�� : �����ע
	bool iniColumnDImensions(Data& data, const std::vector<std::shared_ptr<Dimension>>& _spDimensions);
	// ˵�� : ��ʼ������ĳߴ�
	bool iniMaxSize();
	// ˵�� : ��ʼ�������ݽ����
	std::vector<std::vector<int>>setHBreakPointLine();
	// ˵�� : ��ʼ�ݽ��ݽ����
	std::vector<std::vector<int>>setVBreakPointLine();
	// ˵�� : ��ʼ��һ���ݽ�˵�Ϊһ����
	std::vector<Line> setBreakPointVecToLine(const std::vector<std::vector<int>>& BreakPointVec, const char& ch = 'H');
	// ˵�� : ���ù����Json����
	Json::Value stirrupVaule();
	// ˵�� : ��ʼ������λ��
	Json::Value stirrupLoc();
	// ˵�� : ȷ��ĳ������ݽ��λ��
	Json::Value lineRelativeBPLoc(const Line& line, const char& ch = 'H');
	// ˵�� : �����ݽ��json����
	Json::Value breakPointInfo();




private:
	/********process data***/
	bool _bCircle;
	Box _bpBox;
	Box _stirBox;//�ɹ�����ɵ�box
	Block _columnBlock;//���������Ϣ
	std::string _bpLayer;//�ϵ�����ͼ��
	std::string _stirrupLayer;//!�����湿������ͼ��
	std::vector<ColumnBreakPoint>break_pointVec;//!���й���Ķϵ���Ϣ
	std::vector<int>_stirrupIndexVec;//!���й��������
	
	std::vector<ColumnSStirrup>_columnStirrupVec;//!������

	//!��˫֮��������Ϊ���жϹ���������Ϣ���������ǵ�֧�������Ƕ�֧��
	std::vector<int>_singleStirIndex;//!��֧������
	std::vector<int>_doubleStirIndex;//!˫֧������

	std::vector<ColumnBPInfo>_columnBPinfoVec;//!�ݽ���Ϣ
	std::vector<int>_bpLeadLineVec;//!�ݽ�ϵ������������
	std::vector<Dimension>_dimensionsVec;//!�������ע��Ϣ
	std::vector<ColumnStirLeadInfo>_columnStirInfo;//!������Ĺ�����Ϣ,ָ���������ַ���

	std::vector<std::pair<Line, std::vector<int>>>_bpHLine;//��ͬһ�е��ݽ�
	std::vector<std::pair<Line, std::vector<int>>>_bpVLine;//��ͬһ�е��ݽ�

	
private:
	//�ֵ�
	std::map<int, int>_bpIndexMap;//!�������жϵ�������ֵ����ڿ��ٵ��ų��ǹ�����
	std::map<int, bool>_bStirPair;//!�����жϹ����Ƿ���ԣ��Ӷ��Ų����֧��
	std::map<int, ColumnDirection>_stirrupDirMap;//!����� int��ʾ����������������Direction��ʾ�������ķ���
	std::map<int, int>_stirDirHookIndexMap;//!����ӳ�乿���Ӧ��hook����һ��int��ʾ���ߵ��������ڶ���int��ʾbeamHookVec������
	std::map<int, std::vector<int>>_hookStirrupIndexMap;//!����ӳ�䣬��־����Ժ�Ĺ��������������ڿ��ٲ���

private:
	/*****result data*******/
	std::string _name;
	double _seale;
	std::string _stirrup;
	bool _rectangle;
	double _maxWidth;
	double _maxHeight;
	double _scale;
	int _countH;
	int _countV;

public:
	//���Բ���
	std::vector<int>testIndexVec;
	//std::vector<Point>cornerVec;
	Box testBox;
	std::vector<ColumnStirrupHook>_columnHookVec;
	std::vector<Line>testLine;

};






#endif