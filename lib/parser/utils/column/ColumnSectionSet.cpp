#include "ColumnSectionSet.h"

bool ColumnSectionSet::setMember(const std::vector<std::shared_ptr<Block>>& blocks,
	std::vector<std::shared_ptr<Dimension>> dimensions)
{
	m_blocks = blocks;
	//m_dimensions = dimensions;
	return true;
}

std::shared_ptr<ColumnSection> ColumnSectionSet::columnSection(Data& data, const std::shared_ptr<Block>&_block)
{
	std::shared_ptr<ColumnSection>tempSection = nullptr;
	if (bColumnSection(data, _block))
	{
		auto columnSection = ColumnSection::retrunColumnSection(*_block,data);
		tempSection = std::shared_ptr<ColumnSection>(new ColumnSection(columnSection));
		testSection.append(tempSection->outputColumnSection());

		//���Գ���
		std::vector<int>testIndexVec1;
		std::vector<Point>testPoint1;
		columnSection.testFuction(testIndexVec1, testPoint1);
		testIndexVec.insert(testIndexVec.begin(), testIndexVec1.begin(), testIndexVec1.end());
		//testIndexVec.insert(testIndexVec.begin(), columnSection.testIndexVec.begin(), columnSection.testIndexVec.end());
		testPoint.insert(testPoint.end(), testPoint1.begin(), testPoint1.end());
		testBox = tempSection->testBox;
		return tempSection;
	}
	return tempSection;
}
#if 0
bool ColumnSectionSet::bColumnSection(Data&data,const std::shared_ptr<Block>& _block)
{
	auto it = _block;
	if (it->type == Block::AXIS || it->type == Block::TABLE)
	{
		return false;
	}
	if (it->name.find("L") != std::string::npos ||
		it->name.find("l") != std::string::npos ||
		it->name.find("Z") == std::string::npos)
	{
		return false;
	}
	int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.bottom);
	int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.top);
	int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.left, Precision, false);
	int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.right, Precision, false);

	//����������
	for (ht; ht >= hf; --ht)
	{
		int idx = data.m_lineData.hLinesIndices()[ht];
		auto &line = data.m_lineData.lines()[idx];
		if (!it->box.cover(line.s) || !it->box.cover(line.e))
		{
			continue;
		}

		if (BoolBreakpoint(line, data))
		{
			return true;
		}
	}
	//�����������
	for (vt; vt >= vf; --vt)
	{
		int idx = data.m_lineData.vLinesIndices()[vt];
		auto &line = data.m_lineData.lines()[idx];
		if (!it->box.cover(line.s) || !it->box.cover(line.e))
		{
			continue;
		}
		if (BoolBreakpoint(line, data))
		{
			return true;
		}
	}
	//���������Ȧ������ͼ�ĸֽ�������һ������Բ���ƣ�
	return false;
}
#endif

#if 0
bool ColumnSectionSet::columnSections(Data & data, std::vector<std::shared_ptr<ColumnSection>>& _section)
{
	std::vector<bool> map(m_blocks.size(), 0);
	SetSectionsMap(map, data);
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		if (map[i])
		{
			//�ڴ���Ҫ����жϣ��Ƿ��Ǽ��б�ע
			std::shared_ptr<ColumnSection> temp_section;
			std::vector<ColumnSection::BreakPoint> temp_bp; //�ݽ�Ķ���
			std::vector<Point> center_mind;           //�ݽ�����Բ��
			std::vector<Dimensions> temp_dim;         //��ע
			std::vector<std::vector<int>>
				l_t_c; //�ݽ�ļ����� Longitudinal Tendon Collection
			std::vector<std::pair<ColumnSection::Explanation, Point>>
				lead_line_explanation; //�����߽�����Ϣ

									   //����ϵ�
			FindBP(m_blocks[i]->box, center_mind, temp_bp, data);
			//��Щ�ֽ�Ķ���ͼ����Բ���ɵģ��������

			//�Ѽ���ע��Ϣ(�������жϣ��е������߲�ֹ�������ж�������)
			FindSectionDIm(data, m_blocks[i]->box, temp_dim);
			// temp_section->FindSectionDIm(_blocks[i]->box, temp_dim, _dimensions);
			//Ϊsection�����ڴ�
			//��ʼ������Ϣ
			temp_section.reset(new ColumnSection(temp_bp, temp_dim));
			//��ʼ����������
			temp_section->line_data = data.m_lineData;
			temp_section->corner_data = data.m_cornerData;

			temp_section->SetSectionBlockBox(*m_blocks[i]);
			temp_section->SetSectionScale(temp_section->scale, m_blocks[i]->box,
				data);

			temp_section->SetSectionStirrup(temp_section->stirrup_model,
				m_blocks[i]->box, data);

			temp_section->SetSectionBlockName(*m_blocks[i]);

			//������Ϣ
			temp_section->SetSectionBreakPointSurround(*temp_section, data);
			temp_section->SetSectionBox(*temp_section, data);
			//
			temp_section->SetSectionHook(temp_section->hooks, *temp_section, data);
			if (temp_section->hooks.size() == 0)
				continue;
			temp_section->SetSectionStirrupReferencePoint(*temp_section, data);

			temp_section->SetSectionStirrupInformation(*temp_section, data);

			//������ϵ���ص�������
			temp_section->FindLongitudinalTendonCollection(center_mind, l_t_c,
				data);
			temp_section->SetLTC(temp_section->l_t_c, l_t_c);
			//���û�������߰����б�ע������
			if (!l_t_c.empty())
			{
				// temp_section->SetSectionbox();

				temp_section->FindExplanation(l_t_c, temp_section->section_box,
					m_blocks[i]->box, lead_line_explanation,
					data);
				temp_section->SetSectionExplanation(temp_section->explanation,
					lead_line_explanation);
			}
			else
			{
				temp_section->CenterNote(*temp_section, data);
			}
			//��ע�Ŀ��
			temp_section->SetSectionWidthHeight();

			//��ʼ��������Ϣ
			_section.push_back(temp_section);
		}
	}

	return true;
}

#endif

std::shared_ptr<ColumnSection> ColumnSectionSet::columnSections(Data & data,const std::shared_ptr<Block>&_block)
{
	//�ڴ���Ҫ����жϣ��Ƿ��Ǽ��б�ע
	std::shared_ptr<ColumnSection> temp_section = nullptr;
	if (bColumnSection(data,_block))
	{
		std::vector<ColumnSection::BreakPoint> temp_bp; //�ݽ�Ķ���
		std::vector<Point> center_mind;           //�ݽ�����Բ��
		std::vector<Dimension> temp_dim;         //��ע
		std::vector<std::vector<int>>
			l_t_c; //�ݽ�ļ����� Longitudinal Tendon Collection
		std::vector<std::pair<ColumnSection::Explanation, Point>>
			lead_line_explanation; //�����߽�����Ϣ

								   //����ϵ�
		FindBP(_block->box, center_mind, temp_bp, data);
		//��Щ�ֽ�Ķ���ͼ����Բ���ɵģ��������

		//�Ѽ���ע��Ϣ(�������жϣ��е������߲�ֹ�������ж�������)
		FindSectionDIm(data, _block->box, temp_dim);
		// temp_section->FindSectionDIm(_blocks[i]->box, temp_dim, _dimensions);
		//Ϊsection�����ڴ�
		//��ʼ������Ϣ
		temp_section.reset(new ColumnSection(temp_bp, temp_dim));
		//��ʼ����������
		temp_section->line_data = data.m_lineData;
		temp_section->corner_data = data.m_cornerData;

		temp_section->SetSectionBlockBox(*_block);
		temp_section->SetSectionScale(temp_section->scale, _block->box,
			data);

		temp_section->SetSectionStirrup(temp_section->stirrup_model,
			_block->box, data);

		temp_section->SetSectionBlockName(*_block);

		//������Ϣ
		temp_section->SetSectionBreakPointSurround(*temp_section, data);
		temp_section->SetSectionBox(*temp_section, data);
		//
		temp_section->SetSectionHook(temp_section->hooks, *temp_section, data);
		if (temp_section->hooks.size() == 0)
			return nullptr;
		temp_section->SetSectionStirrupReferencePoint(*temp_section, data);

		temp_section->SetSectionStirrupInformation(*temp_section, data);

		//������ϵ���ص�������
		temp_section->FindLongitudinalTendonCollection(center_mind, l_t_c,
			data);
		temp_section->SetLTC(temp_section->l_t_c, l_t_c);
		//���û�������߰����б�ע������
		if (!l_t_c.empty())
		{
			// temp_section->SetSectionbox();

			temp_section->FindExplanation(l_t_c, temp_section->section_box,
				_block->box, lead_line_explanation,
				data);
			temp_section->SetSectionExplanation(temp_section->explanation,
				lead_line_explanation);
		}
		else
		{
			temp_section->CenterNote(*temp_section, data);
		}
		//��ע�Ŀ��
		temp_section->SetSectionWidthHeight();
	}
	
	return temp_section;
}

#if 0
bool ColumnSectionSet::SetSectionsMap(std::vector<bool>& map, Data & data)
{

	/*
	- �ж�ĳһ�����ǲ��������棬���õĹ�Լ����-
	-1.���Ҵ˿����Ƿ���ڣ��պϵ�ȴ�ǵ�����ֱ��-
	-(�����Աȷ��ֹ���ͼ���ݽ�ģ����Ʒ�ʽΪ�պϵĶ����)-
	-2.�е���������Ҳ���ڹ���Ķ��棬�����������Ϣ��Լ-
	-(�����Աȷ������Ķ�����Ȼ��д��XXXL����Ϣ)-
	-3.˵������Բ����Ϊ�������ͼ�Ļ��Ʒ�ʽ��Ŀǰδ���ֺ������-
	*/
	for (auto i = 0; i < m_blocks.size(); ++i)
	{
		auto it = m_blocks[i];
		if (it->type == Block::AXIS || it->type == Block::TABLE)
		{
			continue;
		}
		if (it->name.find("L") != std::string::npos ||
			it->name.find("l") != std::string::npos ||
			it->name.find("Z") == std::string::npos)
		{
			continue;
		}
		int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.bottom);
		int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.top);
		int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.left, Precision, false);
		int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.right, Precision, false);

		//����������
		for (ht; ht >= hf; --ht)
		{
			int idx = data.m_lineData.hLinesIndices()[ht];
			auto &line = data.m_lineData.lines()[idx];
			if (!it->box.cover(line.s) || !it->box.cover(line.e))
			{
				continue;
			}

			if (BoolBreakpoint(line, data))
			{
				map[i] = true;
			}
		}
		//�����������
		for (vt; vt >= vf && !map[i]; --vt)
		{
			int idx = data.m_lineData.vLinesIndices()[vt];
			auto &line = data.m_lineData.lines()[idx];
			if (!it->box.cover(line.s) || !it->box.cover(line.e))
			{
				continue;
			}
			if (BoolBreakpoint(line, data))
			{
				map[i] = true;
			}
		}
		//���������Ȧ������ͼ�ĸֽ�������һ������Բ���ƣ�
	}
	return true;
}
#endif

#if 0
bool ColumnSectionSet::BoolBreakpoint(const Line & line, Data & data)
{
	//�Թ��������ж�
	if (line.side)
	{
		auto ends1 = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, line.s), 1);
		auto e = data.m_endpointData.getEndpoint(ends1[0]);
		auto corners = data.m_cornerData.corners().find(e.index);
		bool mark = false;
		if (corners != data.m_cornerData.corners().end())
		{
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				auto l2 = corner.l2;
				int index = 0;
				if (e.index == l1)
					index = l2;
				else
					index = l1;
				if (data.m_lineData.lines()[index].isSide())
				{
					mark = true;
					break;
				}
			}
		}
		if (mark)
			return false;
		else
			return true;
	}
	return false;
}
#endif

bool ColumnSectionSet::FindBP(const Box & box, std::vector<Point>& mid_point, std::vector<ColumnSection::BreakPoint>& temp_bp, Data & data)
{
	//����ϵ�
	auto it = box;
	int hf = box.hf;
	int ht = box.ht;
	int vf = box.vf;
	int vt = box.vt;
	//С�̺�
	for (int ht1 = ht; ht1 >= hf; --ht1)
	{
		int idx = data.m_lineData.hLinesIndices()[ht1];
		auto &line = data.m_lineData.lines()[idx];
		if (!box.cover(line.s) || !box.cover(line.e))
		{
			continue;
		}

		if (BoolBreakpoint(line, data))
		{

			ColumnSection::BreakPoint bp;
			Point temp_center_mid = Point((line.e.x + line.s.x) / 2, line.s.y);
			if (findBreakPoint(temp_bp, temp_center_mid))
				continue;
			mid_point.push_back(temp_center_mid); //Բ��
			bp.center_mind = temp_center_mid;
			bp.radius = (ROUND(line.e.x) - ROUND(line.s.x)) / 2.0; //�뾶
			temp_bp.push_back(bp);
		}
	}
	//С����
	for (int vt1 = vt; vt1 >= vf; --vt1)
	{
		int idx = data.m_lineData.vLinesIndices()[vt1];
		auto &line = data.m_lineData.lines()[idx];
		if (!box.cover(line.s) || !box.cover(line.e))
		{
			continue;
		}

		if (BoolBreakpoint(line, data))
		{
			//map[i] = true;
			ColumnSection::BreakPoint bp;
			Point temp_center_mid = Point(line.s.x, (line.e.y + line.s.y) / 2);

			if (findBreakPoint(temp_bp, temp_center_mid))
			{
				continue;
			}

			mid_point.push_back(temp_center_mid); //Բ��
			bp.center_mind = temp_center_mid;
			bp.radius = (ROUND(line.e.y) - ROUND(line.s.y)) / 2.0; //45 �뾶
			temp_bp.push_back(bp);
		}
	}
	return true;
}

bool ColumnSectionSet::findBreakPoint(const std::vector<ColumnSection::BreakPoint>& bk, const Point & p)
{
	for (auto it : bk)
	{
		if (ConnectPoint(it.center_mind, p))
		{
			return true;
		}
	}
	return false;
}

bool ColumnSectionSet::FindSectionDIm(Data &data,const Box & box, std::vector<Dimension>& temp_dim)
{
	int l_idx = data.m_lineData.vLinesIndices()[box.vf];
	int r_idx = data.m_lineData.vLinesIndices()[box.vt];
	int t_idx = data.m_lineData.hLinesIndices()[box.ht];
	int b_idx = data.m_lineData.hLinesIndices()[box.hf];
	for (auto it : data.m_dimensions)
	{
		if (it->box.left + Precision > data.m_lineData.lines()[l_idx].s.x &&
			it->box.right - Precision < data.m_lineData.lines()[r_idx].s.x &&
			it->box.bottom + Precision > data.m_lineData.lines()[b_idx].s.y &&
			it->box.top - Precision < data.m_lineData.lines()[t_idx].s.y)
		{
			temp_dim.push_back(*it);
		}
	}
	return true;
}

bool ColumnSectionSet::bColumnSection(Data& data, const std::shared_ptr<Block>& _block)
{
	auto it = _block;
	if (it->type == Block::AXIS || it->type == Block::TABLE)
	{
		return false;
	}
	if (it->name.find("L") != std::string::npos ||
		/*it->name.find("l") != std::string::npos ||*/
		it->name.find("Z") == std::string::npos)
	{
		return false;
	}
	int hf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.bottom);
	int ht = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, it->box.top);
	int vf = data.m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.left, Precision, false);
	int vt = data.m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, it->box.right, Precision, false);

	//����������
	for (ht; ht >= hf; --ht)
	{
		int idx = data.m_lineData.hLinesIndices()[ht];
		auto& line = data.m_lineData.lines()[idx];
		if (!it->box.cover(line.s) || !it->box.cover(line.e))
		{
			continue;
		}

		if (BoolBreakpoint(line, data))
		{
			return true;
		}
	}
	//�����������
	for (vt; vt >= vf; --vt)
	{
		int idx = data.m_lineData.vLinesIndices()[vt];
		auto& line = data.m_lineData.lines()[idx];
		if (!it->box.cover(line.s) || !it->box.cover(line.e))
		{
			continue;
		}
		if (BoolBreakpoint(line, data))
		{
			return true;
		}
	}
	//���������Ȧ������ͼ�ĸֽ�������һ������Բ���ƣ�

	return false;
}
