#include "blockSet.h"

bool BlockSet::setComponents(const std::vector<std::shared_ptr<Component>>& components, 
	const std::vector<std::shared_ptr<Component>>& borders, 
	const std::vector<std::shared_ptr<Axis>>& axises, 
	const std::vector<std::shared_ptr<Table>>& tables,
	const std::vector<std::shared_ptr<Dimension>> &dimensions,
	const std::vector<Axis::AxisLine> _axisHLines,
	const std::vector<Axis::AxisLine> _axisVLines)
{
	m_components = components;
	m_borders = borders;
	m_axises = axises;
	m_tables = tables;
	m_dimensions = dimensions;
	m_axisHLines = _axisHLines;
	m_axisVLines = _axisVLines;
	return true;
}

bool BlockSet::blocks(Data & data, std::vector<std::shared_ptr<Block>>& blocks, std::vector<std::shared_ptr<Axis>> &_axises)
{
	std::vector<bool> coms(m_components.size(), false);
	std::vector<bool>axisMap(m_axisHLines.size()+m_axisVLines.size(), false);
	double height = 0;
	//检测border
	checkBorder(coms);
	// find from tables.
	checkTables(coms, data, blocks);
	// find from components
	checkComponent(coms, data, blocks);

	//!集中处理块与轴网的关系，有的块内含有轴，但却不是轴网块
	for (auto it : blocks)
	{
		if (it->name == "table")
			continue;

		for (auto i = 0; i < m_axisHLines.size(); ++i)
		{
			
			if (it->box.cross(m_axisHLines[i].second) && 
				(it->box.right - it->box.left) * 0.5 < m_axisHLines[i].second.length())
			{
				it->m_axisHLines.push_back(m_axisHLines[i]);
					axisMap[i] = true;
			}
		}
		for (auto i = 0; i < m_axisVLines.size(); ++i)
		{
			
			if (it->box.cross(m_axisVLines[i].second) && 
				(it->box.top - it->box.bottom) * 0.5 < m_axisVLines[i].second.length())
			{
				it->m_axisVLines.push_back(m_axisVLines[i]);
				axisMap[m_axisHLines.size() + i] = true;
			}

		}
		//!设置块内轴信息
		if (it->m_axisHLines.size() != 0 && 
			it->m_axisVLines.size()!=0)
		{
			//初始化轴网信息
			AxisSet axisSet;
			std::vector<std::shared_ptr<Axis>> axises;
			axisSet.creatAxis(it->m_axisHLines, it->m_axisVLines, axises);
			axisSet.setAxis(axises);
			axisSet.optimizationAxis(axises);
			if (!axises.empty())
			{
				it->bAxis = true;
				it->axis = axises.front();
				//it->name = "axis";
				_axises = axises;
			}
			

		}
		else if(it->m_axisHLines.size() == 0 &&
			it->m_axisVLines.size() == 0)
		{
			it->bGrid = false;
			it->bAxis = false;
		}
		else
		{
			it->bGrid = true;
		}
		
	}

	return true;
}

bool BlockSet::checkBorder(std::vector<bool>& coms)
{
	for (auto border : m_borders)
	{
		for (int i = 0; i < m_components.size(); ++i)
		{
			auto com = m_components[i];
			if (border->box.cover(com->box) && com->box.area() > border->box.area() * 0.8)
			{
				coms[i] = true;
			}
		}
	}
	return true;
}

bool BlockSet::checkAxis(std::vector<bool>& coms, Data & data, std::vector<std::shared_ptr<Block>> &blocks)
{
	double height = 0.0;
	for (auto axis : m_axises)
	{
		height = axis->box.height() * 0.75;
		auto block = buildBlockFromBox(Block::AXIS, axis->box, "axis", coms, height, data);
		if (block.get())
		{
			block->axis = axis;
			blocks.push_back(block);
		}
	}
	return true;
}

bool BlockSet::checkTables(std::vector<bool>& coms, Data & data, std::vector<std::shared_ptr<Block>> &blocks)
{
	double height = 0.0;
	for (auto table : m_tables) {
		if (table->side)
			continue;
		height = table->height * 0.75;
		auto block = buildBlockFromBox(Block::TABLE, table->box, "table", coms, height, data);
		if (block.get()) {
			blocks.push_back(block);
		}
	}
	return true;
}

bool BlockSet::connectDimensions(std::vector<bool>& coms)
{
	for (auto dim : m_dimensions)
	{
		Box old_box = dim->box;
		Box new_box = dim->box;
		double length = 65535;
		int temp_index = -1;
		for (int i = m_components.size() - 1; i >= 0; --i)
		{
			if (m_components[i]->box == dim->box)
			{
				coms[i] = 1;
			}
			if (coms[i])
				continue;
			auto component = m_components[i];

			if ((dim->box.cover(component->box) || dim->box.cross(component->box) ||
				component->box.cover(dim->box) || component->box.cross(dim->box)))
			{
				component->box.expand(dim->box);
				new_box = component->box;
			}
		}
		if (old_box == new_box)
		{
			for (int i = 0; i < m_components.size(); --i)
			{
				if (coms[i])
					continue;
				auto it = m_components[i];
				if (dim->direction == 0 && it->box.left > dim->box.right)
				{
					auto temp_length = it->box.left - dim->box.right;
					length = length < temp_length ? length : temp_length;
					temp_index = i;
				}
				else if (dim->direction == 90 && it->box.bottom > dim->box.top)
				{
					auto temp_length = it->box.bottom - dim->box.top;
					length = length < temp_length ? length : temp_length;
					temp_index = i;
				}
				else if (dim->direction == 180 && it->box.right < dim->box.left)
				{
					auto temp_length = dim->box.left - it->box.right;
					length = length < temp_length ? length : temp_length;
					temp_index = i;
				}
				else if (dim->direction == 270 && it->box.top < dim->box.bottom)
				{
					auto temp_length = dim->box.bottom - it->box.top;
					length = length < temp_length ? length : temp_length;
					temp_index = i;
				}
			}
			if (temp_index != -1)
				m_components[temp_index]->box.expand(dim->box);
		}
	}
	return true;
}

std::shared_ptr<Block> BlockSet::buildBlockFromBox(Block::Type type, Box & bbox, const std::string & n, 
	std::vector<bool>& coms, double distance, Data & data)
{
	Box box = bbox;
	std::string name = n;
	std::shared_ptr<Block> block;

	// expand

	for (int i = m_components.size() - 1; i >= 0; --i)
	{

		if (coms[i])
			continue;
		auto component = m_components[i];

		if ((box.cover(component->box) || box.cross(component->box) ||
			component->box.cover(box) || component->box.cross(box)))
		{
			box.expand(component->box);
			coms[i] = true;
		}
	}

	int f = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom - distance);
	int t = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom);

	std::vector<int> candidates;
	//pickup candidates title lines
	for (t; t >= f; --t)
	{
		int idx = data.m_lineData.hLinesIndices()[t];
		auto &line = data.m_lineData.lines()[idx];
		if (box.cover(line.s) || box.cover(line.e))
		{
			continue;
		}
		if (line.e.x + Precision < box.left || line.s.x - Precision > box.right)
			continue;

		double len = line.length();

		double len2 = box.right - box.left;
		if (line.e.x < box.right)
			len2 -= box.right - line.e.x;
		if (line.s.x > box.left)
			len2 -= line.s.x - box.left;

		if (len2 < 0.9 * len)
			continue;

		auto it = data.m_cornerData.corners().find(idx);
		if (it != data.m_cornerData.corners().end() && it->second.size() > 0)
			continue;

		candidates.push_back(idx);
	}

	if (candidates.size() > 0)
	{
		int tline = candidates[0];
		auto &tl = data.m_lineData.lines()[tline];
		auto& bl = data.m_lineData.lines()[tline];
		double r = (tl.e.x - tl.s.x) / 2;
		Point tempPoint = Point((tl.e.x + tl.s.x) / 2, tl.s.y);
		candidates = data.m_kdtTreeData.kdtTexts().radiusSearch(tempPoint, r);
		for (auto candidate : candidates)
		{
			auto &point = data.m_textPointData.textpoints()[candidate];
			if (point.y > box.bottom)
				continue;
			if (point.x > tl.e.x)
				continue;
			if (tl.s.x - point.x > tl.length() * 0.1)
				continue;

			std::shared_ptr<DRW_Text> tt = std::static_pointer_cast<DRW_Text>(point.entity);
			if (!name.empty())
			{
				name += "|";
			}
			name += tt->text;
		}
		box.expand(bl.s);
		box.expand(bl.e);
	}

	for (int i = m_components.size() - 1; i >= 0; --i)
	{
		if (coms[i])
			continue;
		auto component = m_components[i];
		if (box.cover(component->box) || box.cross(component->box) ||
			component->box.cover(box) || component->box.cross(box))
		{
			box.expand(component->box);
			coms[i] = true;
		}
	}
	SetBlockVHIndex(box, data.m_lineData);
	block = std::make_shared<Block>(type, name, box);

	return block;
}

bool BlockSet::checkComponent(std::vector<bool>& coms, Data & data, std::vector<std::shared_ptr<Block>>& blocks)
{
	double height = 0.0;
	for (int i = 0; i < m_components.size(); ++i)
	{
		
		if (coms[i])
			continue;
		auto com = m_components[i];

		height = m_components[i]->box.height() * 0.75 + m_components[i]->box.width() * 0.3;
		auto block = buildBlockFromBox(Block::COMPONENT, com->box, "", coms, height, data);

		if (block.get())
		{
			//在此函数中分析断面图或刨面图的详情
			blocks.push_back(block);
		}
	}
	return true;
}
