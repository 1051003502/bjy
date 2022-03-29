#include "sectionSymbol.h"



bool SectionSymbolGroup::checkAndAdd(std::shared_ptr<Data> data, std::shared_ptr<SectionSymbol> other)
{
	if (searchText(data, *sectionSymbolVec.back(), *other))
	{
		this->sectionSymbolVec.push_back(other);
		return true;
	}
	return false;
}

bool SectionSymbolGroup::searchText(std::shared_ptr<Data>data, SectionSymbol& s1, SectionSymbol& s2)
{
	if (s1.sectionLineGroup.isExistSecondDirectionLineIdx() !=
		s2.sectionLineGroup.isExistSecondDirectionLineIdx())
		return false;

	if (s2.sectionLineGroup.isExistSecondDirectionLineIdx())
	{
		const auto& l1 = data->m_lineData.lines().at(s1.sectionLineGroup.sectionDirectionLineIdx);
		const auto& l2 = data->m_lineData.lines().at(s2.sectionLineGroup.sectionDirectionLineIdx);
		auto vec1 = data->m_kdtTreeData.kdtTexts().radiusSearch(l1.s, l1.length() * 2);
		auto vec2 = data->m_kdtTreeData.kdtTexts().radiusSearch(l2.s, l2.length() * 2);
		for (auto i = 0; i < vec1.size(); ++i)
		{
			const auto textIdx1 = vec1.at(i);
			const auto& te1 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx1).entity);
			const auto& t1 = te1->text;
			for (auto j = 0; j < vec2.size(); ++j)
			{
				const auto textIdx2 = vec2.at(j);
				/*if (textIdx1 == textIdx2)
					continue;*/
				const auto& te2 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx2).entity);
				const auto& t2 = te2->text;
				if (t1 == t2)
				{
					s1.symbolTextInfo.pointIdx = textIdx1;
					s1.symbolTextInfo.text = t1;
					s2.symbolTextInfo.pointIdx = textIdx2;
					s2.symbolTextInfo.text = t2;
					return true;
				}
			}
		}
		vec1 = data->m_kdtTreeData.kdtTexts().radiusSearch(l1.e, l1.length() * 2);
		vec2 = data->m_kdtTreeData.kdtTexts().radiusSearch(l2.e, l2.length() * 2);
		for (auto i = 0; i < vec1.size(); ++i)
		{
			const auto textIdx1 = vec1.at(i);
			auto te1 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx1).entity);
			const auto& t1 = te1->text;
			for (auto j = 0; j < vec2.size(); ++j)
			{
				const auto textIdx2 = vec2.at(j);
				/*if (textIdx1 == textIdx2)
					continue;*/
				auto te2 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx2).entity);
				const auto& t2 = te2->text;
				if (t1 == t2)
				{
					s1.symbolTextInfo.pointIdx = textIdx1;
					s1.symbolTextInfo.text = t1;
					s2.symbolTextInfo.pointIdx = textIdx2;
					s2.symbolTextInfo.text = t2;
					return true;
				}
			}
		}
	}
	else
	{
		const auto& l1 = data->m_lineData.lines().at(s1.sectionLineGroup.sectionBaseLineIdx);
		const auto& l2 = data->m_lineData.lines().at(s2.sectionLineGroup.sectionBaseLineIdx);

		auto vec1 = data->m_kdtTreeData.kdtTexts().radiusSearch(l1.s, l1.length() * 2);
		auto vec2 = data->m_kdtTreeData.kdtTexts().radiusSearch(l2.s, l2.length() * 2);
		for (auto i = 0; i < vec1.size(); ++i)
		{
			const auto textIdx1 = vec1.at(i);
			auto te1 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx1).entity);
			const auto& t1 = te1->text;
			for (auto j = 0; j < vec2.size(); ++j)
			{
				const auto textIdx2 = vec2.at(j);
				/*if (textIdx1 == textIdx2)
					continue;*/
				auto te2 = std::dynamic_pointer_cast<DRW_Text>(data->m_textPointData.textpoints().at(textIdx2).entity);
				const auto& t2 = te2->text;
				if (t1 == t2)
				{
					s1.symbolTextInfo.pointIdx = textIdx1;
					s1.symbolTextInfo.text = t1;
					s2.symbolTextInfo.pointIdx = textIdx2;
					s2.symbolTextInfo.text = t2;
					return true;
				}
			}
		}

	}
	return false;
}

bool SectionSymbolGroup::updateChainBaseIdx(std::shared_ptr<Data> data)
{
	if (!sectionSymbolVec.back()->sectionLineGroup.isExistSecondDirectionLineIdx() ||
		sectionSymbolVec.size() < 2)
		return false;

	auto j = 1;
	auto ss1 = sectionSymbolVec.at(static_cast<long>(j - 1));
	while (j != sectionSymbolVec.size())
	{
		auto ss2 = sectionSymbolVec.at(j);
		const auto& l1 = data->m_lineData.lines().at(ss1->sectionLineGroup.sectionDirectionLineIdx);
		const auto& l2 = data->m_lineData.lines().at(ss2->sectionLineGroup.sectionDirectionLineIdx);
		if (bAtExtendedLine(l1, l2))
		{
			std::swap(ss1->sectionLineGroup.sectionBaseLineIdx, ss1->sectionLineGroup.sectionDirectionLineIdx);
			std::swap(ss2->sectionLineGroup.sectionBaseLineIdx, ss2->sectionLineGroup.sectionDirectionLineIdx);
		}
		j++;
		ss1 = ss2;
	}
	return true;
}

