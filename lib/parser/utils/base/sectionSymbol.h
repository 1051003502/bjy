#pragma once
#include"publicFunction.h"
#include "pillar.h"

class SectionLineGroup
{
public:
	enum class SectionDirection
	{
		Unknown = 0,
		Left = 1,
		Right = 2,
		Up = 4,
		Down = 8,
		/*Slash = 16*/
	};

	SectionLineGroup(int baseLineIdx = -1, int directionLineIdx = -1,
					 SectionDirection d = SectionDirection::Unknown) :
		sectionBaseLineIdx(baseLineIdx),
		sectionDirectionLineIdx(directionLineIdx),
		direction(d)
	{}




	inline bool isExistSecondDirectionLineIdx() const
	{
		if (this->sectionDirectionLineIdx == -1)
			return false;
		return true;
	}

	int sectionBaseLineIdx;				//剖切的基线索引
	int sectionDirectionLineIdx;		//剖切的方向线索引
	SectionDirection direction;			//剖视方向
};


struct SymbolTextInfo
{
	std::string text;
	int pointIdx;
};

struct SectionSymbol
{
	SectionLineGroup sectionLineGroup;								//剖切线组
	std::pair<std::string, double> axisVDistance;					//剖切线的位置
	std::pair<std::string, double> axisHDistance;					//剖切线的水平位置
	SymbolTextInfo symbolTextInfo;									//剖切符号
};

class SectionSymbolGroup
{
public:

	bool checkAndAdd(std::shared_ptr<Data>data, std::shared_ptr<SectionSymbol> other);
	bool searchText(std::shared_ptr<Data>data, SectionSymbol& s1, SectionSymbol& s2);
	bool updateChainBaseIdx(std::shared_ptr<Data> data);
	std::vector<std::shared_ptr<SectionSymbol>> sectionSymbolVec; //包含的剖切符
};