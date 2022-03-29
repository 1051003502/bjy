#include "block.h"

void Block::SetBlockInformation(/*const Point&point*/)
{
	
}
//std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData

std::shared_ptr<Block> buildBlockFromBox(Block::Type type, Box & bbox, const std::string & n, std::vector<bool>& coms, double distance, 
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData)
{
	Box box = bbox;
	std::string name = n;
	std::shared_ptr<Block> block;

	// expand

	for (int i = components.size() - 1; i >= 0; --i)
	{

		if (coms[i])
			continue;
		auto component = components[i];

		if ((box.cover(component->box) || box.cross(component->box) ||
			component->box.cover(box) || component->box.cross(box)))
		{
			box.expand(component->box);
			coms[i] = true;
		}
	}

	int f = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom - distance);
	int t = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom);

	std::vector<int> candidates;
	//pickup candidates title lines
	for (t; t >= f; --t)
	{
		int idx = lineData.hLinesIndices()[t];
		auto &line = lineData.lines()[idx];
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

		auto it = cornerData.corners().find(idx);
		if (it != cornerData.corners().end() && it->second.size() > 0)
			continue;

		candidates.push_back(idx);
	}

	if (candidates.size() > 0)
	{
		int tline = candidates[0];
		int bline = candidates[0];
		auto &tl = lineData.lines()[tline];
		auto &bl = lineData.lines()[bline];
		candidates = kdtTreeData.kdtTexts().radiusSearch(tl.s, box.bottom - tl.s.y);
		for (auto candidate : candidates)
		{
			auto &point = textPointData.textpoints()[candidate];
			if (point.y > box.bottom)
				continue;
			/*if (point.y < tl.s.y)
			continue;*/
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

	for (int i = components.size() - 1; i >= 0; --i)
	{
		if (coms[i])
			continue;
		auto component = components[i];
		if (box.cover(component->box) || box.cross(component->box) ||
			component->box.cover(box) || component->box.cross(box))
		{
			box.expand(component->box);
			coms[i] = true;
		}
	}
	SetBlockVHIndex(box, lineData);
	block = std::make_shared<Block>(type, name, box);

	return block;
}

bool SetBlockVHIndex(Box &box, LineData & lineData)
{
	box.hf = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.bottom);
	box.ht = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, box.top);
	box.vf = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, box.left, Precision, false);
	box.vt = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, box.right, Precision, false);
	return true;
}

bool checkBorder(std::vector<bool>& coms, std::vector<std::shared_ptr<Component>>& components, const std::vector<std::shared_ptr<Component>>& borders)
{
	for (auto border : borders)
	{
		for (int i = 0; i < components.size(); ++i)
		{
			auto com = components[i];
			if (border->box.cover(com->box) && com->box.area() > border->box.area() * 0.8)
			{
				coms[i] = true;
			}
		}
	}
	return true;
}

bool checkAxis(std::vector<std::shared_ptr<Block>> &blocks, std::vector<bool> &coms, double &height, const std::vector<std::shared_ptr<Axis>>& axises, 
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData &textPointData)
{
	for (auto axis : axises)
	{
		height = axis->box.height() * 0.75;
		auto block = buildBlockFromBox(Block::AXIS, axis->box, "axis", coms, height,
			components, lineData, cornerData, kdtTreeData, textPointData);
		if (block.get())
		{
			block->axis = axis;
			blocks.push_back(block);
		}
	}
	return true;
}

bool checkTables(std::vector<std::shared_ptr<Block>>& blocks, std::vector<bool>& coms, double & height, const std::vector<std::shared_ptr<Table>>& _tables, std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData & textPointData)
{
	for (auto table : _tables) {
		if (table->side)
			continue;
		height = table->height * 0.75;
		auto block = buildBlockFromBox(Block::TABLE, table->box, "table", coms, height,
			components, lineData, cornerData, kdtTreeData, textPointData);
		if (block.get()) {
			blocks.push_back(block);
		}
	}
	return true;
}

bool checkComponent(std::vector<std::shared_ptr<Block>>& blocks, std::vector<bool>& coms, double & height, 
	std::vector<std::shared_ptr<Component>>& components, LineData & lineData, CornerData & cornerData, KDTreeData & kdtTreeData, TextPointData & textPointData)
{
	for (int i = 0; i < components.size(); ++i)
	{

		if (coms[i])
			continue;
		auto com = components[i];

		height = components[i]->box.height() * 0.75 + components[i]->box.width() * 0.3;
		auto block = buildBlockFromBox(Block::COMPONENT, com->box, "", coms, height,
			components, lineData, cornerData, kdtTreeData, textPointData);

		/*if (block->name == "")
		{
			unknown_block.push_back(block);
		}*/
		if (block.get())
		{
			//在此函数中分析断面图或刨面图的详情
			blocks.push_back(block);
		}
	}
	return true;
}

bool findClosestVAxis(std::pair<std::string, double>& relativeDistance, std::vector<Axis::AxisLine>& axisVLines, double abscissa)
{
	auto frontAxis = axisVLines.begin();
	auto tailAxis = axisVLines.end() - 1;
	double lowAbscissa = frontAxis->second.s.x;
	double highAbscissa = tailAxis->second.s.x;
	if (abscissa <= lowAbscissa)
		relativeDistance = std::pair<std::string, double>(frontAxis->first.second, abscissa - lowAbscissa);
	else if (abscissa >= highAbscissa)
		relativeDistance = std::pair<std::string, double>(tailAxis->first.second, abscissa - highAbscissa);
	else
	{
		auto midAxis = axisVLines.begin() + axisVLines.size() / 2;
		while (tailAxis - frontAxis > 1)
		{
			if (abscissa < midAxis->second.s.x)
				tailAxis = midAxis;
			else
				frontAxis = midAxis;
			midAxis = frontAxis + (tailAxis - frontAxis) / 2;
		}
		if (abs(frontAxis->second.s.x - abscissa) < abs(tailAxis->second.s.x - abscissa))
			relativeDistance = std::pair<std::string, double>(frontAxis->first.second, abscissa - frontAxis->second.s.x);
		else
			relativeDistance = std::pair<std::string, double>(tailAxis->first.second, abscissa - tailAxis->second.s.x);
	}
	return true;
}

bool findClosestHAxis(std::pair<std::string, double>& relativeDistance, std::vector<Axis::AxisLine>& axisHLines, double coordinate)
{
	auto frontAxis = axisHLines.begin();
	auto tailAxis = axisHLines.end() - 1;
	double lowCoordinate = frontAxis->second.s.y;
	double hightCoordinate = tailAxis->second.s.y;
	if (coordinate <= lowCoordinate)
		relativeDistance = std::pair<std::string, double>(frontAxis->first.second, coordinate - lowCoordinate);
	else if (coordinate >= hightCoordinate)
		relativeDistance = std::pair<std::string, double>(tailAxis->first.second, coordinate - hightCoordinate);
	else
	{
		auto midAxis = axisHLines.begin() + axisHLines.size() / 2;
		while (tailAxis - frontAxis > 1)
		{
			if (coordinate < midAxis->second.s.y)
				tailAxis = midAxis;
			else
				frontAxis = midAxis;
			midAxis = frontAxis + (tailAxis - frontAxis) / 2;
		}
		if (abs(frontAxis->second.s.y - coordinate) < abs(tailAxis->second.s.y - coordinate))
			relativeDistance = std::pair<std::string, double>(frontAxis->first.second, coordinate - frontAxis->second.s.y);
		else
			relativeDistance = std::pair<std::string, double>(tailAxis->first.second, coordinate - tailAxis->second.s.y);
	}
	return true;
}
