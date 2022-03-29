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
	void SetBlockInformation(/*const Point&point*/);			//初始化块信息接口
	~Block() {}
	
public:
	Type type;//这个块的类型
	std::string name;//块的名字
	Box box;//形成这个块的box

	//块内容
	std::vector<std::string>block_information;

	std::shared_ptr<Axis> axis;
	bool bAxis;//判断此块是否有轴
	bool bGrid;//判断是否为轴网
public:
	std::vector<Axis::AxisLine> m_axisHLines;//横轴
	std::vector<Axis::AxisLine> m_axisVLines;//纵轴
};
//查找块名
std::shared_ptr<Block> buildBlockFromBox(Block::Type type, Box &box, const std::string &name, std::vector<bool> &coms, double distance,
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

//************************************
// 方法	: SetBlockVHIndex
// 全名	: SetBlockVHIndex
// 访问	: public 
// 返回	: bool
// 参数	: box
// 参数	: lineData
// 说明	: 给block设定边界线范围，初始化 ht,hf vt,vf
//************************************
bool SetBlockVHIndex(Box &box, LineData & lineData); 

//检测border
bool checkBorder(std::vector<bool> &coms, std::vector<std::shared_ptr<Component>>& components, 
	const std::vector<std::shared_ptr<Component>> &borders);

//检测轴网
bool checkAxis(std::vector<std::shared_ptr<Block>> &blocks,std::vector<bool> &coms, double &height, const std::vector<std::shared_ptr<Axis>>& _axises, 
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

//检测表格
bool checkTables(std::vector<std::shared_ptr<Block>> &blocks, std::vector<bool> &coms, double &height, const std::vector<std::shared_ptr<Table>>& _tables,
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);


//检测剩余的组件
bool checkComponent(std::vector<std::shared_ptr<Block>> &blocks, std::vector<bool> &coms, double &height, std::vector<std::shared_ptr<Component>>& components, 
	LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData);

bool findClosestVAxis(std::pair<std::string, double>& relativeDistance,
	std::vector<Axis::AxisLine>& axisVLines,
	double abscissa);

bool findClosestHAxis(std::pair<std::string, double>& relativeDistance,
	std::vector<Axis::AxisLine>& axisHLines,
	double coordinate);

#endif
