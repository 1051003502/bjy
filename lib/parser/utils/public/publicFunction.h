#ifndef __PARSER_UTILS_PUBLICFUNCTION_H__
#define __PARSER_UTILS_PUBLICFUNCTION_H__

#include "common.h"
#include "data.h"
#include "intern/drw_textcodec.h"
#include "drw_color.h"


// TODO ʵ�ִ�ӡ������Ϣ�Ĺ���
#pragma region Log Print

void logLine(const Line& line);


#pragma endregion Log Print

//************************************
// ����	: GetRandom16Chars
// ȫ��	: getRandom16Chars
// ����	: public 
// ����	: std::string ���ص��ַ���
// ˵��	: ����ַ���16���ַ�
//************************************
std::string getRandom16Chars();

//************************************
// ����	: StrimDouble
// ȫ��	: strimDouble
// ����	: public 
// ����	: double
// ����	: d Ҫ��������ĸ�����
// ����	: bit ������λ��
// ˵��	: �������뱣��С�����λ��
//************************************
double strimDouble(double d, unsigned char bit = 2);


/*������غ���*/
bool b_closedzone(const std::vector<int>& borderline, ClosedZone& temp_borderline, Data& data);
bool b_InClosedZone(int& candidate, const std::vector<int>& line__index, const ClosedZone& temp_borderline, Data& data);

/*�ߵ���غ���*/
//���ߵ����ͽ��з��࣬���ߣ����ߣ�б������
bool lineTypeVHS(Data& data,
				 const std::vector<int>& index,
				 std::vector<int>& v_lines,
				 std::vector<int>& h_lines,
				 std::vector<int>& s_lines);

bool lineTypeVHSIndex(const std::vector<Line>& lines,
					  std::vector<int>& v_lines,
					  std::vector<int>& h_lines,
					  std::vector<int>& s_lines);

bool lineTypeVHSEntity(const std::vector<Line>& lines,
					   std::vector<Line>& v_lines,
					   std::vector<Line>& h_lines,
					   std::vector<Line>& s_lines);

bool lineTypeVHS(const std::vector<Line>& lines,
				 const std::vector<int>& index,
				 std::vector<int>& v_lines,
				 std::vector<int>& h_lines,
				 std::vector<int>& s_lines);

//��ʼ��һ���ߵĽ���
bool bulidLinesCorner(const std::vector<Line>& lines,
					  const std::vector<int>& vIndex,
					  const std::vector<int>& hIndex,
					  const std::vector<int>& sIndex,
					  std::map<int, std::vector<Corner>>& corners);

//��ȡ����ĳ������������
bool nearestLine(const std::vector<Line>& lines,
				 const std::vector<int>& linesIndex,
				 const Point& point,
				 std::pair<int, int>goal_index);


//ɾ���ض��ַ�
bool del_char(std::string& str);

bool pushRTreeLines(const Line& line, const int& index, RTreeData& rTree);

//************************************
// ����	: returnLineType
// ȫ��	: returnLineType
// ����	: public 
// ����	: int
// ����	: line
// ˵��	: �����ߵ����ͣ�����1��Ϊ���ߣ�����2Ϊ���ߣ�����3Ϊб��
//************************************
int returnLineType(const Line& line);

//************************************
// ����	: bLineConnect
// ȫ��	: bLineConnect
// ����	: public 
// ����	: bool
// ����	: line1
// ����	: line2
// ˵��	: �ж������Ƿ���β���
//************************************
bool bLineConnect(const Line& line1, const Line& line2);

//************************************
// ����	: BoolBreakpoint
// ȫ��	: BoolBreakpoint
// ����	: public 
// ����	: bool
// ����	: line
// ����	: data
// ˵��	: �ж϶�����Ƿ�Ϊ�ݽ����
//************************************
bool BoolBreakpoint(const Line& line, Data& data);

//************************************
// ����	: SetBoxVHIndex
// ȫ��	: SetBoxVHIndex
// ����	: public 
// ����	: bool
// ����	: box
// ����	: lineData
// ˵��	: ��ʼ��box�ı߽緶Χ����
//************************************
bool SetBoxVHIndex(Box& box, LineData& lineData);

//************************************
// ����	: returnRtreeIndex
// ȫ��	: returnRtreeIndex
// ����	: public 
// ����	: std::vector<int>
// ����	: min
// ����	: max
// ����	: goalRTree
// ˵��	: ����Rtree���������
//************************************
std::vector<int> returnRtreeIndex(const double min[], const double max[], RTreeData& goalRTree);

/*Ϊ��������������(������ʵ������)*/
//�����������Ϊ��������
//************************************
// ����	: LinesSort
// ȫ��	: LinesSort
// ����	: public 
// ����	: bool
// ����	: lines Ҫ����������߶���
// ����	: ch ѡ��Ժ�������������
// ˵��	: ����������������
//************************************
bool LinesSort(std::vector<Line>& lines, const char& ch);//����֪���ߣ����߽�������
 //************************************
 // ����	: QuickSortLine
 // ȫ��	: QuickSortLine
 // ����	: public 
 // ����	: bool
 // ����	: lines
 // ����	: left ��ʼ
 // ����	: right ��ֹ
 // ����	: ch ����ʽ
 // ˵��	: ��������ݹ�Ĺ���
 //************************************
bool QuickSortLine(std::vector<Line>& lines, int left, int right, const char& ch);
//************************************
// ����	: DivisionLine
// ȫ��	: DivisionLine
// ����	: public 
// ����	: int
// ����	: lines
// ����	: left ��ʼ
// ����	: right ��ֹ
// ����	: ch ����ʽ
// ˵��	: ��������ȽϵĹ���
//************************************
int DivisionLine(std::vector<Line>& lines, int left, int right, const char& ch);
/*���߽������򣬰�����������*/
bool LinesSort(std::vector<int>& linesIndex, const char& ch, LineData& _linedata);
bool QuickSortLine(std::vector<int>& linesIndex, int left, int right, const char& ch, LineData& _linedata);
int DivisionLine(std::vector<int>& linesIndex, int left, int right, const char& ch, LineData& _linedata);

//************************************
// ����	: MergeLines
// ȫ��	: MergeLines
// ����	: public 
// ����	: bool
// ����	: lines
// ����	: ch �ϲ�����
// ˵��	: �Դ���ͬһλ�õ��������кϲ�
//************************************
bool MergeLines(std::vector<Line>& lines, const char& ch);

//************************************
// ����	: MergeLines
// ȫ��	: MergeLines
// ����	: public 
// ����	: bool
// ����	: linesIndexVec ��Ҫ�������������
// ����	: ch ��Ҫ�ϲ�ֱ������
// ����	: _lineData ������Ӧ��ʵ��
// ˵��	: �ϲ��ظ���ֱ��,����������ƽ���غϣ������໥����
//************************************
bool MergeLines(std::vector<int>& linesIndexVec, const char& ch, LineData& _lineData);

//************************************
// ����	: bPointBelongLineEnd
// ȫ��	: bPointBelongLineEnd
// ����	: public 
// ����	: bool
// ����	: _line
// ����	: point
// ˵��	: �жϵ��Ƿ�����һ���ߵĶ˵�
//************************************
bool bPointBelongLineEnd(const Line& _line, const Point& point);

//************************************
// ����	: bConnectTowLine
// ȫ��	: bConnectTowLine
// ����	: public 
// ����	: bool ��ӷ���true �����򷵻� false
// ����	: _line1
// ����	: _line2
// ˵��	: �ж�����ֱ���Ƿ����
//************************************
bool bConnectTowLine(const Line& _line1, const Line& _line2);
//************************************
// ����	: numToString
// ȫ��	: numToString
// ����	: public 
// ����	: num ��Ҫת��������
// ˵��	: ���ַ�ת��Ϊ����
//************************************
std::string numToString(const double& num);
//************************************
// ����	: fineLineIndex
// ȫ��	: fineLineIndex
// ����	: public 
// ����	: line ��Ҫ���ҵ���ʵ��
// ���� : ch Ϊ��Ҫ�����ߵ�����
// ˵��	: ������ʵ������� ����Щ����ֻ֪��λ����Ϣ������֪������Ϣ��������ң�
//************************************
int findLineIndex(LineData &lineData, const Line& line, const char& ch = 'H');
//************************************
// ����	: findLineAdjacentIndex
// ȫ��	: findLineAdjacentIndex
// ����	: public 
// ����	: line ��Ҫ���ҵ���ʵ��
// ���� : size ��Ҫ���ҵķ�Χ
// ���� : ch Ϊ��Ҫ�����ߵ�����
// ���� : loc��ʾ��Ҫ���ҵķ�Χ T�� B�� L�� R��
// ˵��	: ������ʵ������� ����Щ����ֻ֪��λ����Ϣ������֪������Ϣ��������ң�
//************************************
int findLineAdjacentIndex(LineData& lineData, const Line&line,const double& size, const char& type= 'H',const char& loc = 'T');



//gbkתUTF-8  
std::string GbkToUtf8(const std::string& __strGbk);// �����strGbk��GBK���� 
//UTF-8תgbk
std::string Utf8ToGbk(const std::string& __strUtf8);

namespace publicFunction
{
	//˵�����������p��һ����
	int findPointBaseLine(Data& data, const Point& p);
	//˵��������һ���ߵ����н���(���������Լ�)
	std::vector<int> findAllCrossingLine(Data& data, int lineIndex);
	//˵��������һ���ߵ����н��ߣ������㣺�����������ߵĶ˵�
	std::vector<int> findCrossing2(Data& data, int lineIndex);
	//˵��������һ���ߵĽ��ߣ�������1���������߶˵� 2���߷ֱ���ˮƽ����ֱ��
	std::vector<int> findCrossing3(Data& data, int lineIndex1);
	//˵��������Щ������Ǿۼ����ĸ�ͼ��
	std::string findMaxAmountLayer(std::vector<int> lineIndices, Data& data);
	//˵��������Box�������ߣ������ڴ�ӡ
	std::vector<Line> generateBoxBorder(const Box& box);
	//����ֱ���Ƿ��ཻ ���ҵõ�����
	bool checkCrossingAndGetCorner(const Line& line1, const Line& line2, Corner& corner);
	double calculateParallelDistance(Line& l1, Line& l2);
	// ˵��	: �ж�����ƽ����֮��Ľ��泤��
	double twoLineCrossLength(const Line& l1, const Line& l2);
	// ˵��	: ��ƽ���߽��泤��/�ϳ����ߵĳ���
	double calculateTwoLineCrossRatio(const Line& l1, const Line& l2);
	
	int roundToInt(double data);
	
	Line findMatchLine(std::vector<Line> lineFamily, Line lonelyLine);
	int findMatchLineIndex(Data& data, std::vector<int> indices, int index);
}

#endif
