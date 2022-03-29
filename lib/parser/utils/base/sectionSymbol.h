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

	int sectionBaseLineIdx;				//���еĻ�������
	int sectionDirectionLineIdx;		//���еķ���������
	SectionDirection direction;			//���ӷ���
};


struct SymbolTextInfo
{
	std::string text;
	int pointIdx;
};

struct SectionSymbol
{
	SectionLineGroup sectionLineGroup;								//��������
	std::pair<std::string, double> axisVDistance;					//�����ߵ�λ��
	std::pair<std::string, double> axisHDistance;					//�����ߵ�ˮƽλ��
	SymbolTextInfo symbolTextInfo;									//���з���
};

class SectionSymbolGroup
{
public:

	bool checkAndAdd(std::shared_ptr<Data>data, std::shared_ptr<SectionSymbol> other);
	bool searchText(std::shared_ptr<Data>data, SectionSymbol& s1, SectionSymbol& s2);
	bool updateChainBaseIdx(std::shared_ptr<Data> data);
	std::vector<std::shared_ptr<SectionSymbol>> sectionSymbolVec; //���������з�
};