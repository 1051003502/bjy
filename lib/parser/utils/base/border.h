#ifndef __PARSER_UTILS_BORDER_H__
#define __PARSER_UTILS_BORDER_H__
#include "component.h"

class MyClass
{
public:
	MyClass();
	//~MyClass();
	

private:
	std::shared_ptr<Component> component;
};
//bool buildBordersMap(std::vector<char> &map, std::vector<char> &map2, const std::vector<std::shared_ptr<Component>>&components);
//bool buildBorders(double &area, std::vector<std::shared_ptr<Component>> &borders, const std::vector<std::shared_ptr<Component>>&components, const std::vector<char>& map);

#endif