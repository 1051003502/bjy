#include "data.h"
#include "publicFunction.h"

/* LineData */
bool LineData::clearLineData()
{
	_lines.clear();

	_vLinesIndices.clear();
	_hLinesIndices.clear();
	_sLinesIndices.clear();
	return true;
}

bool LineData::pushBackLines(const Line& line)
{
	this->_lines.push_back(line);
	return true;
}

bool LineData::pushBackVLinesIndices(const int& index)
{
	this->_vLinesIndices.push_back(index);
	return true;
}

bool LineData::pushBackHLinesIndices(const int& index)
{
	this->_hLinesIndices.push_back(index);
	return true;
}

bool LineData::pushBackSLinesIndices(const int& index)
{
	this->_sLinesIndices.push_back(index);
	return true;
}

bool LineData::setLines(const std::vector<Line>& line)
{
	this->_lines = line;
	return true;
}

bool LineData::setVLinesIndex(const std::vector<int>& vIndex)
{
	this->_vLinesIndices = vIndex;
	return true;
}

bool LineData::setHLinesIndex(const std::vector<int>& hIndex)
{
	this->_hLinesIndices = hIndex;
	return true;
}

bool LineData::setSLinesIndex(const std::vector<int>& sIndex)
{
	this->_sLinesIndices = sIndex;
	return true;
}



bool LineData::setLines(const int& index, const Point& start, const Point& end)
{
	_lines[index].s = start;
	_lines[index].e = end;
	return true;
}

int LineData::getLinesSize()
{
	return _lines.size();
}

int LineData::getVLinesSize()
{
	return _vLinesIndices.size();
}

int LineData::getHLinesSize()
{
	return _hLinesIndices.size();
}

int LineData::getSLinesSize()
{
	return _sLinesIndices.size();
}

Line LineData::getLine(const int& index)
{
	return _lines[index];
}

int LineData::getVLineIndex(const int& index)
{
	return _vLinesIndices[index];
}

int LineData::getHLineIndex(const int& index)
{
	return _hLinesIndices[index];
}

int LineData::getSLineIndex(const int& index)
{
	return _sLinesIndices[index];
}

std::vector<Line>& LineData::lines()
{
	return _lines;
}

const std::vector<int>& LineData::vLinesIndices()
{
	return _vLinesIndices;
}

const std::vector<int>& LineData::hLinesIndices()
{
	return _hLinesIndices;
}

const std::vector<int>& LineData::sLinesIndices()
{
	return _sLinesIndices;
}

void LineData::sortLinesByBindRef(enum cmLINEDATAMODE mode, Ref ref)
{
	if (mode == LINEDATAMODE_HINDEX)
		std::sort(_hLinesIndices.begin(), _hLinesIndices.end(), std::bind(ref, _lines, std::placeholders::_1, std::placeholders::_2));
	else if (mode == LINEDATAMODE_VINDEX)
		std::sort(_vLinesIndices.begin(), _vLinesIndices.end(), std::bind(ref, _lines, std::placeholders::_1, std::placeholders::_2));
}

int LineData::findLowerBound(enum cmLINEDATAMODE mode, double val, double precision, bool h)
{
	if (mode == LINEDATAMODE_HINDEX && !_hLinesIndices.empty())
		return ::findLowerBound(val, _hLinesIndices, _lines, precision, h);
	else if (mode == LINEDATAMODE_VINDEX && !_vLinesIndices.empty())
		return ::findLowerBound(val, _vLinesIndices, _lines, precision, h);
	else
		return -1;
}

int LineData::findUpperBound(enum cmLINEDATAMODE mode, double val, double precision, bool h)
{
	if (mode == LINEDATAMODE_HINDEX && !_hLinesIndices.empty())
		return	::findUpperBound(val, _hLinesIndices, _lines, precision, h);
	else if (mode == LINEDATAMODE_VINDEX && !_vLinesIndices.empty())
		return ::findUpperBound(val, _vLinesIndices, _lines, precision, h);
	else
		return -1;
}

bool LineData::mergeLinesIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode)
{
	std::vector<int> lines_map(lines.size());
	for (int i = 0; i < lines.size() - 1; ++i)
	{
		if (lines.size() < 2)
			break;
		if (lines_map[i] == -1)
			continue;
		for (int j = i + 1; j < lines.size(); ++j)
		{
			if (lines_map[i] == -1 || lines_map[j] == -1)
				continue;
			if (mode == LINEDATAMODE_VINDEX)
			{
				if (_lines[lines[i]].s.x + Precision > _lines[lines[j]].s.x &&
					_lines[lines[i]].s.x - Precision < _lines[lines[j]].s.x)
				{
					int num = _lines[lines[i]].length() < _lines[lines[j]].length() ? i : j;
					lines_map[num] = -1;
				}
				else
				{
					break;
				}
			}
			else if (mode == LINEDATAMODE_HINDEX)
			{
				if (_lines[lines[i]].s.y + Precision > _lines[lines[j]].s.y &&
					_lines[lines[i]].s.y - Precision < _lines[lines[j]].s.y)
				{
					int num = _lines[lines[i]].length() < _lines[lines[j]].length() ? i : j;
					lines_map[num] = -1;
				}
				else
				{
					break;
				}
			}
			else if (mode == LINEDATAMODE_SINDEX)
			{
				; //后续补全 TODO
			}
		}
	}
	std::vector<int> temp_line;
	for (int i = 0; i < lines.size(); ++i)
	{
		if (lines_map[i] != -1)
		{
			temp_line.push_back(lines[i]);
		}
	}
	lines.clear();
	lines = temp_line;

	return true;
}

bool LineData::lineTypeVHS(const std::vector<int>& index, std::vector<int>& v_lines, std::vector<int>& h_lines, std::vector<int>& s_lines)
{
	for (auto it : index)
	{
		if (_lines[it].vertical())
		{
			v_lines.push_back(it);
		}
		else if (_lines[it].horizontal())
		{
			h_lines.push_back(it);
		}
		else
		{
			s_lines.push_back(it);
		}
	}
	return false;
}




bool LineData::mergeLines(std::vector<Line>& lines, enum cmLINEDATAMODE mode)
{
	std::vector<int> lines_map(lines.size());
	for (size_t i = 0; i < lines.size() - 1; ++i)
	{
		if (lines.empty())
			break;
		if (lines_map[i] == -1)
			continue;
		for (int j = i + 1; j < lines.size(); ++j)
		{
			if (lines_map[i] == -1 || lines_map[j] == -1)
				continue;
			if (mode == LINEDATAMODE_VINDEX)
			{
				if (lines[i].s.x + Precision > lines[j].s.x &&
					lines[i].s.x - Precision < lines[j].s.x)
				{
					int num = lines[i].length() < lines[j].length() ? i : j;
					lines_map[num] = -1;
				}
				else
				{
					break;
				}
			}
			else if (mode == LINEDATAMODE_HINDEX)
			{
				if (lines[i].s.y + Precision > lines[j].s.y &&
					lines[i].s.y - Precision < lines[j].s.y)
				{
					int num = lines[i].length() < lines[j].length() ? i : j;
					lines_map[num] = -1;
				}
				else
				{
					break;
				}
			}
			else if (mode == LINEDATAMODE_SINDEX)
			{
				; //后续补全
			}
		}
	}
	std::vector<Line> temp_line;
	for (int i = 0; i < lines.size(); ++i)
	{
		if (lines_map[i] != -1)
		{
			temp_line.push_back(lines[i]);
		}
	}
	lines.clear();
	lines = temp_line;

	return true;
}

bool LineData::linesSortIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode)
{
	int left = 0;
	int right = lines.size() - 1;
	//if()
	quickSortLineIndex(lines, left, right, mode);
	return true;
}

bool LineData::quickSortLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode)
{
	if (left < right)
	{
		int base = divisionLineIndex(lines, left, right, mode);
		quickSortLineIndex(lines, left, base - 1, mode);
		quickSortLineIndex(lines, base + 1, right, mode);
	}
	return true;
}

int LineData::divisionLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode)
{
	int base = lines[left];
	while (left < right)
	{
		if (mode == LINEDATAMODE_VINDEX)
		{
			while (left < right && _lines[lines[right]].s.x >= _lines[base].s.x)
				right--;
			lines[left] = lines[right];
			while (left < right && _lines[lines[left]].s.x <= _lines[base].s.x)
				left++;
			lines[right] = lines[left];
		}
		else if (mode == LINEDATAMODE_HINDEX)
		{
			while (left < right && _lines[lines[right]].s.y >= _lines[base].s.y)
				right--;
			lines[left] = lines[right];
			while (left < right && _lines[lines[left]].s.y <= _lines[base].s.y)
				left++;
			lines[right] = lines[left];
		}
	}
	lines[left] = base;
	return left;
	//return 0;
}

bool LineData::linesSort(std::vector<Line>& lines, enum cmLINEDATAMODE mode)
{
	int left = 0;
	int right = lines.size() - 1;

	quickSortLine(lines, left, right, mode);
	return true;
}

bool LineData::quickSortLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode)
{
	if (left < right)
	{
		int base = divisionLine(lines, left, right, mode);
		quickSortLine(lines, left, base - 1, mode);
		quickSortLine(lines, base + 1, right, mode);
	}
	return true;
}

int LineData::divisionLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode)
{
	Line base = lines[left];
	while (left < right)
	{
		if (mode == LINEDATAMODE_VINDEX)
		{
			while (left < right && lines[right].s.x >= base.s.x)
				right--;
			lines[left] = lines[right];
			while (left < right && lines[left].s.x <= base.s.x)
				left++;
			lines[right] = lines[left];
		}
		else if (mode == LINEDATAMODE_HINDEX)
		{
			while (left < right && lines[right].s.y >= base.s.y)
				right--;
			lines[left] = lines[right];
			while (left < right && lines[left].s.y <= base.s.y)
				left++;
			lines[right] = lines[left];
		}
	}
	lines[left] = base;
	return left;
}

/* CornerData */
bool CornerData::clearCorners()
{
	_corners.clear();
	return true;
}

bool CornerData::crossLinesS(const std::vector<Line>& lines, int idxs, int idx, double xp, double yp)
{
	const Line& s = lines[idxs];
	const Line& l = lines[idx];
	Corner corner;

	if (!crossLineS(s, l, corner, xp, yp))
		return false;
	corner.l1 = idxs;
	corner.l2 = idx;

	if (_corners.find(idxs) == _corners.end())
	{
		_corners[idxs] = std::vector<Corner>();
	}
	if (_corners.find(idx) == _corners.end())
	{
		_corners[idx] = std::vector<Corner>();
	}

	_corners[idxs].push_back(corner);
	_corners[idx].push_back(corner);

	return true;
	return true;
}

bool CornerData::crossLinesVH(const std::vector<Line>& lines, int idxv, int idxh, double xp, double yp)
{
	const Line& v = lines[idxv];
	const Line& h = lines[idxh];

	Corner corner;

	if (!crossLineVH(v, h, corner, xp, yp))
		return false;

	corner.l1 = idxv;
	corner.l2 = idxh;

	if (_corners.find(idxv) == _corners.end())
	{
		_corners[idxv] = std::vector<Corner>();
	}
	if (_corners.find(idxh) == _corners.end())
	{
		_corners[idxh] = std::vector<Corner>();
	}
	_corners[idxv].push_back(corner);
	_corners[idxh].push_back(corner);
	return true;
}

bool CornerData::crossLinesSS(const std::vector<Line>& lines, int idxs1, int idxs2)
{
	const Line& s1 = lines[idxs1];
	const Line& s2 = lines[idxs2];
	Corner corner;
	if (!crossLineSS(s1, s2, corner))
		return false;
	corner.l1 = idxs1;
	corner.l2 = idxs2;
	if (_corners.find(idxs1) == _corners.end())
	{
		_corners[idxs1] = std::vector<Corner>();
	}
	if (_corners.find(idxs2) == _corners.end())
	{
		_corners[idxs2] = std::vector<Corner>();
	}

	_corners[idxs1].push_back(corner);
	_corners[idxs2].push_back(corner);

	return true;
}

bool CornerData::setCorners(const std::map<int, std::vector<Corner>>& corner)
{
	this->_corners = corner;
	return true;
}

/* CircleData */
bool CirclesData::pushBackCircles(const Circle& c)
{
	_circles.push_back(c);
	return true;
}

const std::vector<Circle>& CirclesData::circles()
{
	return _circles;
}

void CirclesData::clearCircles()
{
	_circles.clear();
}

bool CirclesData::sortByRef(compRef ref)
{
	std::sort(_circles.begin(), _circles.end(), ref);
	return true;
}

bool EndpointData::pushBackEndpoint(const Endpoint& endpoint)
{
	_endpoints.push_back(endpoint);
	return true;
}

Endpoint EndpointData::getEndpoint(const int& index)
{
	return _endpoints[index];
}

bool EndpointData::clearEndpoints()
{
	_endpoints.clear();
	return true;
}

const std::vector<Endpoint>& EndpointData::endpoints()
{
	return this->_endpoints;
}

bool KDTreeData::kdttextsClear()
{
	_kdttexts.clear();
	return true;
}

bool KDTreeData::kdtendsClear()
{

	_kdtends.clear();
	return true;
}

bool KDTreeData::buildKDTreeData(const std::vector<Endpoint>& _endpoints, const std::vector<Point>& _textPoints)
{
	_kdttexts.build(_textPoints);
	_kdtends.build(_endpoints);
	return true;
}

bool RTreeData::clearSLine()
{
	s_lines.RemoveAll();
	return true;
}

bool RTreeData::clearVLine()
{
	v_lines.RemoveAll();
	return true;
}

bool RTreeData::clearHLine()
{
	h_lines.RemoveAll();
	return true;
}

bool RTreeData::clearSBackIndex()
{
	s_back_index.clear();
	return true;
}

bool RTreeData::insertSLines(const double min[], const double max[], const int id)
{
	s_lines.Insert(min, max, id);
	return true;
}

bool RTreeData::pushBackSBackIndex(int id)
{
	s_back_index.push_back(id);
	return true;
}

const RLine& RTreeData::sLines()
{
	return s_lines;
}

const std::vector<int>& RTreeData::SBackIndex()
{
	return s_back_index;
}


bool TextPointData::pushBackPoint(const Point& point)
{
	_textpoints.push_back(point);
	return true;
}

bool TextPointData::clearPoints()
{
	_textpoints.clear();
	return true;
}

bool TextPointData::sortByRef(compRef ref)
{
	std::sort(_textpoints.begin(), _textpoints.end(), ref);
	return true;
}

const std::vector<Point>& TextPointData::textpoints()
{
	return _textpoints;
}

int TextPointData::findLowerBound(double val, double precision)
{
	return ::findLowerBound(val, _textpoints, precision);
}

int TextPointData::findUpperBound(double val, double precision)
{
	return ::findUpperBound(val, _textpoints, precision);
}

bool lineTypeVHS(const std::vector<int>& index, std::vector<int>& v_lines,
				 std::vector<int>& h_lines, std::vector<int>& s_lines, LineData& lineData)
{
	for (auto it : index)
	{
		if (lineData.lines()[it].vertical())
		{
			v_lines.push_back(it);
		}
		else if (lineData.lines()[it].horizontal())
		{
			h_lines.push_back(it);
		}
		else
		{
			s_lines.push_back(it);
		}
	}
	return true;
}

bool extendCorner(std::vector<Corner>& corners, const Line& line, LineData& lineData)
{
	int i, j;
	if (line.vertical())
	{
		double yb = line.s.y;
		double yu = line.e.y;
		i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		Corner c;
		for (i; i <= j; ++i)
		{
			auto h = lineData.lines()[lineData.hLinesIndices()[i]];
			if (crossLineVH(line, h, c))
			{
				c.l1 = lineData.hLinesIndices()[i];
				c.l2 = -1;
				corners.push_back(c);
			}
		}
	}
	else if (line.horizontal())
	{
		double yl = line.s.x;
		double yr = line.e.x;
		i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yl, Precision, false);
		j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yr, Precision, false);
		Corner c;
		for (i; i <= j; ++i)
		{
			auto v = lineData.lines()[lineData.vLinesIndices()[i]];
			if (crossLineVH(v, line, c))
			{
				c.l1 = lineData.vLinesIndices()[i];
				c.l2 = -1;
				corners.push_back(c);

			}
		}
	}
	else
	{
		double yb = std::min(line.s.y, line.e.y);
		double yu = std::max(line.s.y, line.e.y);
		i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yb);
		j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_HINDEX, yu);
		Corner c;
		for (i; i <= j; ++i)
		{
			auto h = lineData.lines()[lineData.hLinesIndices()[i]];
			if (crossLineS(line, h, c))
			{
				c.l1 = lineData.hLinesIndices()[i];
				c.l2 = -1;
				corners.push_back(c);
			}
		}
		double yl = std::min(line.s.x, line.e.x);
		double yr = std::max(line.s.x, line.e.x);
		i = lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yl, Precision, false);
		j = lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, yr, Precision, false);
		for (i; i <= j; ++i)
		{
			auto v = lineData.lines()[lineData.vLinesIndices()[i]];
			if (crossLineS(line, v, c))
			{
				c.l1 = lineData.vLinesIndices()[i];
				c.l2 = -1;
				corners.push_back(c);
			}
		}
	}
	return true;
}

std::vector<int> LineLayerData::getGoalLayerLineVec(const std::string& goalLayer)
{
	if (_lineLayerMapVec.find(goalLayer) == _lineLayerMapVec.end())
	{

		return std::vector<int>();
	}
	else
	{
		return _lineLayerMapVec[goalLayer];
	}

}

bool LineLayerData::setLineLayerMapVec(const int& lineIndex, const Line& line)
{
	if (line.getLayer() != "")
	{
		_lineLayerMapVec[line.getLayer()].push_back(lineIndex);
	}

	return true;
}

const std::map<std::string, std::vector<int>>& LineLayerData::lineLayerMapVec()
{
	return this->_lineLayerMapVec;
}

std::map<std::string, std::vector<int>>::const_iterator LineLayerData::lineLayerEndP()
{
	return this->_lineLayerMapVec.end();
}



bool AxisLineData::setAxisHLine(const std::vector<Axis::AxisLine>& axisHLines)
{
	this->_axisHLines = axisHLines;
	return true;
}

bool AxisLineData::setAxisVLine(const std::vector<Axis::AxisLine>& axisVLines)
{
	this->_axisVLines = axisVLines;
	return true;
}

bool AxisLineData::setAxisSLine(const std::vector<Axis::AxisLine>& axisSLines)
{
	this->_axisSLines = axisSLines;
	return true;
}

const std::vector<Axis::AxisLine>& AxisLineData::axisHLines()
{
	// TODO: 在此处插入 return 语句
	return this->_axisHLines;
}

const std::vector<Axis::AxisLine>& AxisLineData::axisVLines()
{
	// TODO: 在此处插入 return 语句
	return this->_axisVLines;
}

const std::vector<Axis::AxisLine>& AxisLineData::axisSLines()
{
	return this->_axisSLines;
}

void DimData::setDim(const std::shared_ptr<Dimension> dim)
{
	if (dim->emDimType == Dimension::DIMLINEAR ||
		dim->emDimType == Dimension::DIMALIGNED)
		setDimLinearAligned(dim);
	else if (dim->emDimType == Dimension::DIMANGULAR ||
			 dim->emDimType == Dimension::DIMANGULAR3P)
		setDimAngular(dim);
	else if (dim->emDimType == Dimension::DIMRADIUS ||
			 dim->emDimType == Dimension::DIMDIAMETER)
		setDimRadius(dim);
}

bool DimData::findHVDimension(std::vector<DimensionPair>& _dimPairVec,
							  const Point& tick,
							  bool bIncrement,
							  bool bHorizontal)
{
	long long tickKey = 0;
	if (bHorizontal)
	{
		tickKey = pseudoDouble(tick.x);
		for (const auto& dimDesItm : this->hDimDesMap[tickKey])
		{
			const auto& dim = _dimensions.at(dimDesItm.first);
			if (dimDesItm.second == bIncrement)
			{
				_dimPairVec.push_back(dimDesItm);
			}
		}
	}
	else
	{
		tickKey = pseudoDouble(tick.y);
		for (const auto& dimDesItm : this->vDimDesMap[tickKey])
		{
			const auto& dim = _dimensions.at(dimDesItm.first);
			if (dimDesItm.second == bIncrement)
			{
				_dimPairVec.push_back(dimDesItm);
			}
		}
	}

	return true;
}

bool DimData::findHDimension(std::vector<DimensionPair>& _dimPairVec,
							 const double xTick,
							 const double offset/*=1.0*/)
{
	auto lowerBoundIt = this->hDimDesMap.lower_bound(pseudoDouble(xTick - offset));
	auto upperBoundIt = this->hDimDesMap.upper_bound(pseudoDouble(xTick + offset));

	for (auto it = lowerBoundIt; it != upperBoundIt; it++)
	{
		for (const auto& dimDesItm : it->second)
		{
			_dimPairVec.push_back(dimDesItm);
		}
	}
	return true;
}

bool DimData::findVDimension(std::vector<DimensionPair>& _dimPairVec,
							 const double yTick,
							 const double offset /*= 1.0*/)
{
	auto lowerBoundIt = this->vDimDesMap.lower_bound(pseudoDouble(yTick - offset));
	auto upperBoundIt = this->vDimDesMap.upper_bound(pseudoDouble(yTick + offset));
	for (auto it = lowerBoundIt; it != upperBoundIt; it++)
	{
		for (const auto& dimDesItm : it->second)
		{
			_dimPairVec.push_back(dimDesItm);
		}
	}
	return true;
}

std::vector<int> DimData::findAngularDimension(const std::vector<unsigned>& lineIdxVec,
											   const std::shared_ptr<Data>& data) const
{
	std::vector<int> rets;
	for (const auto& refLineIdxItm : lineIdxVec)
	{
		const auto& refLine = data->m_lineData.lines().at(refLineIdxItm);
		for (const auto& angularLineItm : this->angularLineVec)
		{
			if (bAtExtendedLine(refLine, angularLineItm.first))
			{
				rets.push_back(angularLineItm.second);
			}
		}
	}
	return rets;
}

unsigned int DimData::findRadialAndDiameterDimension(const Point& pt) const
{
	for (const auto& ptItm : this->circlePtVec)
	{
		if (ConnectPoint(pt, ptItm.first))
			return ptItm.second;
	}
	return -1;
}

void DimData::setDimLinearAligned(const std::shared_ptr<Dimension> dim)
{
	auto dimIdxItm = this->_dimensions.size();
	const auto& refLine1 = dim->lines.at(0);
	const auto& refLIne2 = dim->lines.at(1);
	const auto& refLIne3 = dim->lines.at(2);
	const Line refLine(refLine1.s, refLIne2.s);
	//水平标注端点的x映射
	if (refLIne3.horizontal())
	{
		auto tickS = pseudoDouble(refLine.s.x);
		auto tickE = pseudoDouble(refLine.e.x);
		//右增
		if (tickS < tickE)
		{
			this->hDimDesMap[tickS].emplace_back(dimIdxItm, true);
			this->hDimDesMap[tickE].emplace_back(dimIdxItm, false);
		}
		//左增
		else
		{
			this->hDimDesMap[tickS].emplace_back(dimIdxItm, false);
			this->hDimDesMap[tickE].emplace_back(dimIdxItm, true);
		}
	}
	//竖直标注y轴映射
	else if (refLIne3.vertical())
	{
		auto tickS = pseudoDouble(refLine.s.y);
		auto tickE = pseudoDouble(refLine.e.y);
		//上增
		if (tickS < tickE)
		{
			this->vDimDesMap[tickS].emplace_back(dimIdxItm, true);
			this->vDimDesMap[tickE].emplace_back(dimIdxItm, false);
		}
		//左增
		else
		{
			this->vDimDesMap[tickS].emplace_back(dimIdxItm, false);
			this->vDimDesMap[tickE].emplace_back(dimIdxItm, true);
		}
	}
	//斜的标注
	else
	{
		Line tpLine(Point(dim->box.left, dim->box.top), Point(dim->box.right, dim->box.bottom));
		pushRTreeLines(tpLine, dimIdxItm, this->sDimRTree);
	}

	//标注的长度映射
	auto lenTick = pseudoDouble(refLine.length());
	lenDimDesMap[lenTick].push_back(dimIdxItm);
	this->_dimensions.push_back(dim);
}

void DimData::setDimRadius(const std::shared_ptr<Dimension> dim)
{
	auto dimIdxItm = this->_dimensions.size();
	this->circlePtVec.emplace_back(dim->pt, dimIdxItm);
	this->_dimensions.push_back(dim);
}

void DimData::setDimAngular(const std::shared_ptr<Dimension> dim)
{
	auto dimIdxItm = this->_dimensions.size();
	this->angularLineVec.emplace_back(dim->lines.front(), dimIdxItm);
	this->angularLineVec.emplace_back(dim->lines.back(), dimIdxItm);
	this->_dimensions.push_back(dim);
}
