#include "border.h"

MyClass::MyClass()
{
}

//bool buildBordersMap(std::vector<char>& map, std::vector<char>& map2, const std::vector<std::shared_ptr<Component>>& components)
//{
//	for (auto it = components.begin(); it != components.end(); ++it)
//	{
//		if (!(*it)->hasBorder())
//		{
//			map[it - components.begin()] = 100;
//			continue;
//		}
//		for (auto jt = components.begin(); jt != components.end(); ++jt)
//		{
//			if (it != jt)
//			{
//				if ((*it)->cover(*(*jt)))
//				{
//					map[jt - components.begin()] += 1;
//					double area1 = (*it)->box.area();
//					double area2 = (*jt)->box.area();
//					//nearest border&border
//					if (area1 > 1.01 * area2 && area1 < 1.3 * area2)
//					{
//						map2[it - components.begin()] = 1;
//						map[it - components.begin()] -= 1;
//					}
//				}
//			}
//		}
//	}
//	return true;
//}
//
//bool buildBorders(double &area,std::vector<std::shared_ptr<Component>>& borders, 
//	const std::vector<std::shared_ptr<Component>>& components,const std::vector<char>& map)
//{
//	//double max_area = 0;
//	for (int i = 0; i < components.size(); ++i)
//	{
//		bool mark = false;
//		if (map[i] < 2 /*&& map2[i]*/)
//		{
//			//borders.push_back(_components[i]);
//			for (int j = 0; j < components.size(); ++j)
//			{
//				if (i != j && map[j] >= 2 && components[i]->box.area() < components[j]->box.area())
//				{
//					mark = true;
//					break;
//				}
//			}
//
//			if (!mark)
//			{
//				if (components[i]->box.area() > area)
//				{
//					area = components[i]->box.area();
//				}
//				//_borders.push_back(_components[i]);
//				borders.push_back(components[i]);
//			}
//		}
//	}
//	return true;
//}

