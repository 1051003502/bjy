
#ifndef __PARSER_UTILS_TABLE_H__
#define __PARSER_UTILS_TABLE_H__

#include "component.h"
#include "kdt.h"
#include "data.h"
#include <assert.h>
#define COLUMN_LEAD_PROERTY_LEFT 0 //���Դ�����״̬
#define COLUMN_LEAD_PROERTY_RIGHT 1
#define HORIZONTAL_LEAD_PROERTY_TOP 2	//���Դ�����״̬���ڶ���
#define HORIZONTAL_LEAD_PROERTY_BOTTOM 3 //���Դ�����״̬���ڵײ�
//#define PARSER_TEST_TABLE

// TODO add table lines.
class Table : public Component
{

public:
	class cell
	{
	public:
		cell();
		void setCell(const std::map<int, int> &t_left, const std::map<int, int> &t_right, const std::map<int, int> &t_bottom,
					 const std::map<int, int> &t_top, const std::string t_text);

	public:
		/*double width;
		double height;*/
		std::string c_text;
		double row_rate;
		double col_rate;
		int row_count;
		int col_count;

		//map<int,int>��һ��������ʾ�߽磬�ڶ���������ʾ��٣�1Ϊ�棬0Ϊ��
		std::map<int, int> c_left;
		std::map<int, int> c_right;
		std::map<int, int> c_bottom;
		std::map<int, int> c_top;
	};
	Table(const std::vector<Line> &vl, const std::vector<Line> &hl, int left, int top, int right, int bottom)
		: Component(vl, hl, std::vector<Line>(), false), left(left), top(top), right(right), bottom(bottom), side(false)
	{

		b_attr_repeat.insert({false, 0});
		attr_repeat = -1;
		exist_text = false;
	}
	~Table() {}

	static std::shared_ptr<Table> buildFromComponent(const Component &component);
	static std::shared_ptr<Table> buildFromCrossLines(std::vector<Line> &vls, std::vector<Line> &hls,
													  double px, double py, const Box &box);

	static void findBorder(std::vector<Line> &vlines, std::vector<Line> &hlines,
						   int &top, int &bottom, int &left, int &right, std::map<int, std::vector<Corner>> corners);
	static void findBorder(std::vector<Line> &vlines, std::vector<Line> &hlines, int &top, int &bottom,
						   int &left, int &right, double px, double py, const Box &box, std::map<int, std::vector<Corner>> corners);

	std::map<int, std::map<int, std::pair<std::string, Point>/*std::shared_ptr<DRW_Text>*/>> &extract(const kdt::KDTree<Point> &kdt, const std::vector<Point> &texts);
	int minValue(const int &a, const int &b, const int &c);
	double calculateStringDistance(std::string strA, std::string strB);
	int returnAttributeArrangementPlus();
	bool tableExclude();
	bool steTextPoind(const TextPointData &textPointData);
	//bool clearCellText();//
private:
	void setTableCell();
	void splitCell();
	void calculationRowColRate(double &average_col, double &average_row);
	void findReadyRowAttribute(int &row_bottom, int &row_top);
	void dealRowAttributeRrepeat(bool &temp_step, std::vector<int> same_interval, const int &row);
	void setCellBorder(std::map<int, int> &t_left, std::map<int, int> &t_right, std::map<int, int> &t_bottom, std::map<int, int> &t_top,
					   const int &l, const int &r, const int &b, const int &t);
	void findReadyColAttribute(int &col_left, int &col_right);
	void dealColAttributeRrepeat(bool &temp_step, std::vector<int> same_interval, const int &col);
	void setColAttributeCorrectionInformation(std::vector<int> &potential_col, std::vector<int> &potential_col_step, bool &b_potential_col);
	void setrowAttributeCorrectionInformation(std::vector<int> &potential_col, std::vector<int> &potential_col_step, bool &b_potential_col);
	void iterationCorrectionAttributeRow(const int &num, std::vector<int> &potential_row,
										 std::vector<int> &potential_row_step, bool &b_potential_row);
	void iterationCorrectionAttributeCol(const int &num, std::vector<int> &potential_col,
										 std::vector<int> &potential_col_step, bool &b_potential_col);

public:
	int left;
	int top;
	int right;
	int bottom;
	int cells;
	double height;
	bool side;
	bool exist_text; //�����ڱ����������Ƶ��������磨�塢������ͼ��
	//std::map<int, std::map<int, std::shared_ptr<DRW_Text>>> table;
	std::map<int, std::map<int, std::pair<std::string,Point>>> table;
	//std::vector<std::shared_ptr<DRW_Text>> cell_text;

	int row;
	int col;
	TextPointData textPointData;
	//��ʼ��table_cell
	std::vector<std::vector<cell>> table_cell; //(row, std::vector<cell>(col));

	std::map<bool, int> b_attr_repeat; //�����Ƿ��ظ�
	int attr_repeat;				   //�����ظ�����λ��
									   //�����������У�������Ϊ�����ԣ�����Ϊ�㣬������Ϊ��
	std::map<std::string, int> property_row;
	std::map<std::string, int> property_col;
};

#endif
