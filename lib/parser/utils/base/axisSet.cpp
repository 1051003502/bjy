#include "axisSet.h"

bool AxisSet::axis(Data& data/*, std::vector<Axis::AxisLine>& vAxisLine,
	std::vector<Axis::AxisLine>& hAxisLine*/
		/*std::vector<std::shared_ptr<Axis>> &axises*/)
{
	//����Բ������
	data.m_circleData.sortByRef(compareX);
	std::vector<Axis::AxisSymbol> pairs;

	//����һ�����⵱Ȧ����ı���ȡ������ʱ������Ϊ�ᴦ��1.���ı���������2.û���ı���
	buildAxisSymbol(pairs, data);

	//������ʼ��������������Ϣ
	buildAxisLines(pairs, data);
	//�Գ�����ʼ����������Ϣ��������
	sortAxisLines();

	// combineAxisHLines conbineAxisVLines ����������Ϊ�˽��ͬ�ߵ����
	m_axisHLines = combineAxisHLines(m_axisHLines);
	m_axisVLines = combineAxisVLines(m_axisVLines);

	//���ӶϿ�������
	linkAxisLines(data.m_lineData);
	//!��ʼ����������
	data.m_axisLineData.setAxisHLine(m_axisHLines);
	data.m_axisLineData.setAxisVLine(m_axisVLines);

	//��������
	//creatAxis(axises);

	//��ʼ������ϵ
	//setAxis(axises);
	//�����Ż�����
	//optimizationAxis(axises);
	return true;
}



bool AxisSet::buildAxisSymbol(std::vector<Axis::AxisSymbol>& pairs, Data& data)
{
	for (auto circle : data.m_circleData.circles())
	{
		auto res = data.m_kdtTreeData.kdtTexts().knnSearch(circle, 3);
		for (auto idx : res)
		{
			auto point = data.m_textPointData.textpoints()[idx];
			if (!circle.cover(point, 1.2))
				continue;

			std::shared_ptr<DRW_Text> tt = std::static_pointer_cast<DRW_Text>(point.entity);
			pairs.push_back(std::make_pair(circle, tt->text));
		}
	}
	return true;
}

bool AxisSet::buildAxisLines(std::vector<Axis::AxisSymbol>& pairs, Data& data)
{
	for (auto pair : pairs)
	{
		auto circle = pair.first;
		auto res = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, circle), 1); //��ѯԲ�Ĵ�����Ķ˵�����
		if (res.size() == 0)
			continue;

		auto point = data.m_endpointData.getEndpoint(res[0]);

		//����˶˵���Ȧ������
		if (!circle.cover(point, circle.r * 0.1))
			continue;
		//index������
		const Line& line = data.m_lineData.getLine(point.index);

		//����������Ȧ�ڣ������ߴ���Ȧ�����
		if ((TowPointsLength(line.s, circle) <= circle.r &&
			TowPointsLength(line.e, circle) <= circle.r) ||
			(TowPointsLength(line.s, circle) > circle.r * 1.1&&
				TowPointsLength(line.e, circle) > circle.r * 1.1))
		{
			continue;
		}

		if (line.horizontal())
		{
			m_axisHLines.push_back(std::make_pair(pair, line));
		}
		else if (line.vertical())
		{
			m_axisVLines.push_back(std::make_pair(pair, line));
		}
		else
		{
			if (line.length() > 2 * circle.r)
				continue;
			auto corners = data.m_cornerData.corners().find(point.index);
			if (corners == data.m_cornerData.corners().end())
				continue;
			//�˴��ж������⣬��Щͼֽ����б�ߵĶ˵��������ֱ�ߵ����
			if (corners->second.size() > 2)
				continue;
			auto corner = corners->second[0];
			auto l = corner.l1;
			if (l == point.index)
				l = corner.l2;
			const Line& ll = data.m_lineData.getLine(l);
			if (ll.horizontal())
			{
				m_axisHLines.push_back(std::make_pair(pair, ll));
			}
			else if (ll.vertical())
			{
				m_axisVLines.push_back(std::make_pair(pair, ll));
			}
		}
	}
	return true;
}

bool AxisSet::sortAxisLines()
{
	sort(m_axisHLines.begin(), m_axisHLines.end(), [](const std::pair<Axis::AxisSymbol, Line>& l1, const std::pair<Axis::AxisSymbol, Line>& l2)
	{
		return compareY(l1.first.first, l2.first.first);
	});

	sort(m_axisVLines.begin(), m_axisVLines.end(), [](const std::pair<Axis::AxisSymbol, Line>& l1, const std::pair<Axis::AxisSymbol, Line>& l2)
	{
		return compareX(l1.first.first, l2.first.first);
	});
	return true;
}

std::vector<Axis::AxisLine> AxisSet::combineAxisHLines(const std::vector<Axis::AxisLine>& hlines)
{
	if (hlines.size() <= 0)
		return hlines;
	std::vector<std::pair<Axis::AxisSymbol, Line>> hl;
	std::pair<Axis::AxisSymbol, Line> pre = hlines[0];
	if (hlines.size() == 1)
	{
		hl.push_back(pre);
	}
	else
	{
		for (int i = 1; i < hlines.size(); ++i)
		{
			if (hlines[i].second.s.y - hlines[(long long)i - 1].second.s.y > Precision)
			{
				hl.push_back(pre);
				pre = hlines[i];
				continue;
			}
			if (hlines[i].second.s.x - hlines[(long long)i - 1].second.e.x > Precision)
			{
				hl.push_back(pre);
				pre = hlines[i];
				continue;
			}
			if (pre.second.e.x < hlines[i].second.e.x)
			{
				pre.second.e = hlines[i].second.e;
			}
		}
		hl.push_back(pre);
	}
	return hl;
}

std::vector<Axis::AxisLine> AxisSet::combineAxisVLines(const std::vector<Axis::AxisLine>& vlines)
{
	if (vlines.size() <= 0)
		return vlines;
	std::vector<std::pair<Axis::AxisSymbol, Line>> vl;
	std::pair<Axis::AxisSymbol, Line> pre = vlines[0];
	if (vlines.size() == 1)
	{
		vl.push_back(pre);
	}
	else
	{
		for (int i = 1; i < vlines.size(); ++i)
		{
			if (vlines[i].second.s.x - vlines[(long long)i - 1].second.s.x > Precision)
			{
				vl.push_back(pre);
				pre = vlines[i];
				continue;
			}

			if (vlines[i].second.s.y - vlines[(long long)i - 1].second.e.y > Precision)
			{
				vl.push_back(pre);
				pre = vlines[i];
				continue;
			}
			if (pre.second.e.y < vlines[i].second.e.y)
			{
				pre.second.e = vlines[i].second.e;
			}
		}
		vl.push_back(pre);
	}

	return vl;
}

bool AxisSet::linkAxisLines(LineData& lineData)
{
	for (auto& line : m_axisHLines)
	{
		double yb = line.second.s.y - Precision;
		double yu = line.second.s.y + Precision;

		int i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		int j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		double temp_length = line.second.length();
		for (i; i <= j; ++i)
		{
			const Line& l = lineData.getLine(lineData.hLinesIndices()[i]);
			// ray
			// dot line

			if (line.first.first.x < line.second.s.x)
			{
				//���l.e.x > line.second.e.x�жϣ�����Ϊ���ڽ϶̱�ע���������ظ������߼�����©��
				if (l.s.x > line.second.s.x&& l.e.x > line.second.e.x && (l.s.x - line.second.e.x) < temp_length)
				{
					line.second.e = l.e;
				}
			}
			else
			{
				//���l.s.x < line.second.s.x�жϣ�����Ϊ���ڽ϶̱�ע���������ظ������߼�����©��
				if (l.e.x < line.second.e.x && l.s.x < line.second.s.x && (line.second.s.x - l.e.x) < temp_length)
				{
					line.second.s = l.s;
				}
			}
		}
	}

	for (auto& line : m_axisVLines)
	{
		double yl = line.second.s.x - Precision;
		double yr = line.second.s.x + Precision;

		int i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yl, Precision, false);
		int j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yr, Precision, false);

		double temp_length = line.second.length();
		for (i; i <= j; ++i)
		{
			const Line& l = lineData.getLine(lineData.vLinesIndices()[i]);
			// ray
			// dot line
			if (line.first.first.y < line.second.s.y)
			{
				//���l.e.y > line.second.e.y�ж������ͬ��
				if (l.s.y > line.second.s.y&& l.e.y > line.second.e.y && (l.s.y - line.second.e.y) < temp_length)
				{
					line.second.e = l.e;
				}
			}
			else
			{
				//���l.s.y < line.second.s.y�ж������ͬ��
				if (l.e.y < line.second.e.y && l.s.y < line.second.s.y && (line.second.s.y - l.e.y) < temp_length)
				{
					line.second.s = l.s;
				}
			}
		}
	}
	return true;
}

bool AxisSet::creatAxis(const std::vector<Axis::AxisLine>& _axisHLines, 
	const std::vector<Axis::AxisLine>& _axisVLines,
	std::vector<std::shared_ptr<Axis>>& axises)
{
	std::vector<bool> vh(_axisHLines.size(), false);
	std::vector<bool> vv(_axisVLines.size(), false);
	std::vector<bool> cs(_axisVLines.size() * _axisHLines.size(), false);

	int cnt = 0;
	for (int i = 0; i < _axisVLines.size(); ++i)
	{
		for (int j = 0; j < _axisHLines.size(); ++j)
		{
			Corner corner;
			auto cross = crossLineVH(_axisVLines[i].second, _axisHLines[j].second, corner);
			cs[i * _axisHLines.size() + j] = cross;
			cnt += cross ? 1 : 0;
		}
	}

	for (int i = 0; i < _axisHLines.size(); ++i)
	{
		if (vh[i])
			continue;
		auto axis = buildAxisFrom(_axisHLines,_axisVLines,i, cs, false, vv, vh);

		axises.push_back(axis);
	}

	for (int i = 0; i < _axisVLines.size(); ++i)
	{
		if (vv[i])
			continue;
		auto axis = buildAxisFrom(_axisHLines,_axisVLines,i, cs, true, vv, vh);
		axises.push_back(axis);
	}
	return true;
}

std::shared_ptr<Axis> AxisSet::buildAxisFrom(const std::vector<Axis::AxisLine>& _axisHLines,
	const std::vector<Axis::AxisLine>& _axisVLines,
	int idx, const std::vector<bool>& map, bool v,
	std::vector<bool>& mapv, std::vector<bool>& maph)
{
	std::vector<int> vls;
	std::vector<int> hls;
	findCrossedAxis(idx, _axisVLines.size(), _axisHLines.size(), vls, hls, map, v, mapv, maph);

	std::vector<Axis::AxisLine> vs;
	std::vector<Axis::AxisLine> hs;

	//sort result
	sort(vls.begin(), vls.end());
	sort(hls.begin(), hls.end());

	for (auto i : vls)
	{
		vs.push_back(_axisVLines[i]);
	}

	for (auto i : hls)
	{
		hs.push_back(_axisHLines[i]);
	}

	std::shared_ptr<Axis> axis(new Axis(vs, hs));
	return axis;
}

bool AxisSet::findCrossedAxis(int idx, int vl, int hl, std::vector<int>& vls, std::vector<int>& hls, const std::vector<bool>& map, bool v, std::vector<bool>& mv, std::vector<bool>& mh)
{
	if (v)
	{
		mv[idx] = true;
		vls.push_back(idx);
		for (int i = 0; i < hl; ++i)
		{
			if (mh[i])
				continue;
			if (map[(long long)idx * hl + i])
			{
				findCrossedAxis(i, vl, hl, vls, hls, map, false, mv, mh);
			}
		}
	}
	else
	{
		mh[idx] = true;
		hls.push_back(idx);
		for (int i = 0; i < vl; ++i)
		{
			if (mv[i])
				continue;
			if (map[(long long)hl * i + idx])
			{
				findCrossedAxis(i, vl, hl, vls, hls, map, true, mv, mh);
			}
		}
	}
	return true;
}

bool AxisSet::setAxis(std::vector<std::shared_ptr<Axis>>& axises)
{
	for (int i = 0; i < axises.size(); ++i)
	{

		//�ϲ�ͬһ��������ͬ����
		axises[i]->hlines = combineAxisHLines(axises[i]->hlines);
		axises[i]->vlines = combineAxisVLines(axises[i]->vlines);

		int H_lines_size = axises[i]->hlines.size();
		int V_lines_size = axises[i]->vlines.size();

		//��ʼ��ԭʼ����ϵ
		if (H_lines_size >= 1)
		{
			axises[i]->referenceAxisX.push_back(std::pair<Axis::AxisLine, double>(axises[i]->hlines[0], axises[i]->hlines[0].second.s.y));
		}
		if (V_lines_size >= 1)
		{
			axises[i]->referenceAxisY.push_back(std::pair<Axis::AxisLine, double>(axises[i]->vlines[0], axises[i]->vlines[0].second.s.x));
		}
		//��ʼ�������Լ�����
		for (int h1 = 0; h1 < H_lines_size; ++h1)
		{
			axises[i]->hlines_coo.push_back(std::pair<Axis::AxisLine, double>(axises[i]->hlines[h1],
																			  ROUND(axises[i]->hlines[h1].second.s.y - axises[i]->referenceAxisX[0].second)));
		}
		for (int v1 = 0; v1 < V_lines_size; ++v1)
		{

			axises[i]->vlines_coo.push_back(std::pair<Axis::AxisLine, double>(axises[i]->vlines[v1],
																			  ROUND(axises[i]->vlines[v1].second.s.x - axises[i]->referenceAxisY[0].second)));
		}


	}
	return true;
}

bool AxisSet::optimizationAxis(std::vector<std::shared_ptr<Axis>>& axisVec)
{
	//TODO:�˴���Ҫ���ƣ�������ͼֽ�еĶ���������кϲ����ڴ�ֻ������򵥵��Ż��������ҳ�����������֮����������
	int num = 0;
	std::shared_ptr<Axis> axis;
	for (auto it : axisVec)
	{
		//���ɵ�����ɵ��������˵�
		if (it->vlines.size() == 0 || it->hlines.size()==0)
		{
			continue;
		}

		auto vhSum = it->hlines.size() + it->vlines.size();
		if (vhSum > num)
		{
			num = vhSum;
			axis = it;
		}
	}
	axisVec.clear();
	if (axis.get())
		axisVec.push_back(axis);
	return true;
}
