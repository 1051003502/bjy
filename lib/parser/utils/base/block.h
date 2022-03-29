#ifndef __PARSER_UTILS_BLOCK_H__
#define __PARSER_UTILS_BLOCK_H__

#include "box.h"
#include "line.h"
#include "axis.h"
#include "table.h"
#include "component.h"


class Block {
public:
	typedef enum {
		TABLE,
		AXIS,
		COMPONENT,
		TEXTS,
		UNKNOWN,
		DIMENSION,
		NUMBER
	}Type;

public:
	Block() 
	{
		bAxis = false;
		bGrid = false;
	}
	Block(Type type, const std::string& name, const Box& box):type(type), name(name),box(box) 
	{
		bAxis = false;
		bGrid = false;
	}
	void SetBlockInformation(/*const Point&point*/);			//��ʼ������Ϣ�ӿ�
	~Block() {}
	
public:
	Type type;//����������
	std::string name;//�������
	Box box;//�γ�������box

	//������
	std::vector<std::string>block_information;

	std::shared_ptr<Axis> axis;
	bool bAxis;//�жϴ˿��Ƿ�����
	bool bGrid;//�ж��Ƿ�Ϊ����
public:
	std::vector<Axis::AxisLine> m_axisHLines;//����
	std::vector<Axis::AxisLine> m_axisVLines;//����
};
//���ҿ���
std::shared_ptr<Block> buildBlockFromBox(Block::Type type, Box &box, const std::string &name, std::vector<bool> &coms, double distance,
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

//************************************
// ����	: SetBlockVHIndex
// ȫ��	: SetBlockVHIndex
// ����	: public 
// ����	: bool
// ����	: box
// ����	: lineData
// ˵��	: ��block�趨�߽��߷�Χ����ʼ�� ht,hf vt,vf
//************************************
bool SetBlockVHIndex(Box &box, LineData & lineData); 

//���border
bool checkBorder(std::vector<bool> &coms, std::vector<std::shared_ptr<Component>>& components, 
	const std::vector<std::shared_ptr<Component>> &borders);

//�������
bool checkAxis(std::vector<std::shared_ptr<Block>> &blocks,std::vector<bool> &coms, double &height, const std::vector<std::shared_ptr<Axis>>& _axises, 
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

//�����
bool checkTables(std::vector<std::shared_ptr<Block>> &blocks, std::vector<bool> &coms, double &height, const std::vector<std::shared_ptr<Table>>& _tables,
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);


//���ʣ������
bool checkComponent(std::vector<std::shared_ptr<Block>> &blocks, std::vector<bool> &coms, double &height, std::vector<std::shared_ptr<Component>>& components, 
	LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

bool findClosestVAxis(std::pair<std::string, double>& relativeDistance,
	std::vector<Axis::AxisLine>& axisVLines,
	double abscissa);

bool findClosestHAxis(std::pair<std::string, double>& relativeDistance,
	std::vector<Axis::AxisLine>& axisHLines,
	double coordinate);

#endif
