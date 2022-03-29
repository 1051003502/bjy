#ifndef __PARSER_UTILS_BORDERSET_H__
#define __PARSER_UTILS_BORDERSET_H__
#include "border.h"

class BorderSet 
{
public:
	BorderSet() {
		/*m_area = 0;*/
	}
	~BorderSet() {}
	bool borders(std::vector<std::shared_ptr<Component>>& _borders, const std::vector<std::shared_ptr<Component>>& components);
	bool setBorderMap(const std::vector<std::shared_ptr<Component>>&components);
	bool buildBordersMap(const std::vector<std::shared_ptr<Component>>&components);
	bool buildBorders(double &area, std::vector<std::shared_ptr<Component>> &borders, const std::vector<std::shared_ptr<Component>>&components);
public:
	std::vector<char> m_map;
	std::vector<char> m_map2;
	


};
#endif
