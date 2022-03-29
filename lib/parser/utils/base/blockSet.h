#ifndef __PARSER_UTILS_BLOCKSET_H__
#define __PARSER_UTILS_BLOCKSET_H__
#include "block.h"
#include "dimension.h"
#include "axisSet.h"
class BlockSet
{
public:
	BlockSet() {}
	bool setComponents(const std::vector<std::shared_ptr<Component>> &components,
		const std::vector<std::shared_ptr<Component>> &borders,
		const std::vector<std::shared_ptr<Axis>> &axises,
		const std::vector<std::shared_ptr<Table>> &tables,
		const std::vector<std::shared_ptr<Dimension>> &dimensions,
		std::vector<Axis::AxisLine> m_axisHLines,
	std::vector<Axis::AxisLine> m_axisVLines);
	bool blocks(Data &data,	std::vector<std::shared_ptr<Block>> &blocks, std::vector<std::shared_ptr<Axis>> &_axises);
	//���border
	bool checkBorder(std::vector<bool> &coms);
	//�������
	bool checkAxis(std::vector<bool> &coms, Data&data, std::vector<std::shared_ptr<Block>> &blocks);
	//�����
	bool checkTables(std::vector<bool> &coms, Data & data, std::vector<std::shared_ptr<Block>> &blocks);
	//������עʵ��
	bool connectDimensions(std::vector<bool>& coms);
	//���ҿ���
	std::shared_ptr<Block> buildBlockFromBox(Block::Type type, Box &bbox, const std::string &n, std::vector<bool> &coms, double distance, Data & data);
	//���ʣ������
	bool checkComponent(std::vector<bool> &coms, Data & data, std::vector<std::shared_ptr<Block>>& blocks);

public:
	std::vector<std::shared_ptr<Component>> m_components;
	std::vector<std::shared_ptr<Component>> m_borders;
	std::vector<std::shared_ptr<Axis>> m_axises;
	std::vector<std::shared_ptr<Table>> m_tables;
	std::vector<std::shared_ptr<Dimension>> m_dimensions;
	//!����
	std::vector<Axis::AxisLine> m_axisHLines;
	std::vector<Axis::AxisLine> m_axisVLines;


};
#endif