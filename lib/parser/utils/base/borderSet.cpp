#include "borderSet.h"

bool BorderSet::borders(std::vector<std::shared_ptr<Component>>& _borders, const std::vector<std::shared_ptr<Component>>& components)
{
	setBorderMap(components);
	buildBordersMap(components);
	double max_area = 0;
	std::vector<std::shared_ptr<Component>> borders;
	buildBorders(max_area, borders, components);
	// last border
	for (auto it : borders)
	{
		if (it->box.area() > max_area * 0.1)
		{
			_borders.push_back(it);
		}
	}
	borders.clear();
	return true;
}

bool BorderSet::setBorderMap(const std::vector<std::shared_ptr<Component>>& components)
{
	std::vector<char> map1(components.size());
	std::vector<char> map2(components.size());
	this->m_map = map1;
	this->m_map2 = map2;
	return true;
}

bool BorderSet::buildBordersMap(const std::vector<std::shared_ptr<Component>>& components)
{
	for (auto it = components.begin(); it != components.end(); ++it)
	{
		if (!(*it)->hasBorder())
		{
			m_map[it - components.begin()] = 100;
			continue;
		}
		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			if (it != jt)
			{
				if ((*it)->cover(*(*jt)))
				{
					m_map[jt - components.begin()] += 1;
					double area1 = (*it)->box.area();
					double area2 = (*jt)->box.area();
					//nearest border&border
					if (area1 > 1.01 * area2 && area1 < 1.3 * area2)
					{
						m_map2[it - components.begin()] = 1;
						m_map[it - components.begin()] -= 1;
					}
				}
			}
		}
	}
	return true;
}

bool BorderSet::buildBorders(double &area, std::vector<std::shared_ptr<Component>> &borders, const std::vector<std::shared_ptr<Component>>&components)
{
	//double max_area = 0;
	for (int i = 0; i < components.size(); ++i)
	{
		bool mark = false;
		if (m_map[i] < 2 /*&& map2[i]*/)
		{
			//borders.push_back(_components[i]);
			for (int j = 0; j < components.size(); ++j)
			{
				if (i != j && m_map[j] >= 2 && components[i]->box.area() < components[j]->box.area())
				{
					mark = true;
					break;
				}
			}

			if (!mark)
			{
				if (components[i]->box.area() > area)
				{
					area = components[i]->box.area();
				}
				//_borders.push_back(_components[i]);
				borders.push_back(components[i]);
			}
		}
	}
	return true;
}
