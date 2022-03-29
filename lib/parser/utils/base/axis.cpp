#include "axis.h"


Axis::AxisLineCoordinate Axis::findAxisLine(std::string name, const char& type) {
	//此处需要优化，优化方式快速查找
	if (type == 'H') {
		for (auto it : hlines_coo) {
			if (it.first.first.second == name) {
				return it;
			}
		}
		//return *hlines_coo.end();
	} else if (type == 'V') {
		for (auto it : vlines_coo) {
			if (it.first.first.second == name) {
				return it;
			}
		}
		//return *vlines_coo.end();
	}
}
