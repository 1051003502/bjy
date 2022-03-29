#include "box.h"
#include <algorithm>



bool Box::cross(const Box& b) const
{
	if (!initv || !inith || !b.initv || !b.inith)
		return false;
	return (std::max(left, b.left) + Precision < std::min(right, b.right))
		&& (std::max(bottom, b.bottom) + Precision < std::min(top, b.top));
	//return std::max(left, b.left) < 0.0;
}

bool Box::cross(const Line& line) const
{
	if (line.horizontal())
	{
		if ((line.s.x > this->right) || (line.e.x < this->left))
		{
			return false;
		}
		else
		{
			if (line.s.y > this->bottom&& line.s.y < this->top)
			{
				return true;
			}
		}
		
	}
	else if (line.vertical())
	{
		if ((line.s.y > this->top) || (line.e.y < this->bottom))
		{
			return false;
		}
		else
		{
			if (line.s.x > this->left&& line.s.x < this->right)
			{
				return true;
			}
		}
	}
	else
	{
		;
	}
	return false;
}


