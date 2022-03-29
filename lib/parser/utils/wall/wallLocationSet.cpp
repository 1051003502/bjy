#include "wallLocationSet.h"

std::vector<std::shared_ptr<WallLocation>> WallLocationSet::returnWallLocation(const std::shared_ptr<Data>& spData, 
	const std::shared_ptr<Block>& _block,std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec)
{
	if (isPlanBlock(_block))
	{
		//初始化基础数据
		setBaseData(spData,_block,ssgVec);
		//查找墙图层
		findWallLayer();
		//初始化墙图层线
		findWallIndex();
		//构建墙
		buildWall();
		//
		cleanRepeat();
		//输出墙数据
		printWallLocationData();
		//画墙
		drawWall();
	}
	return _spWallLocVec;
}

bool WallLocationSet::isPlanBlock(const std::shared_ptr<Block>& _block)
{
	if (_block->axis)
		return true;
	else 
		return false;
}

bool WallLocationSet::setBaseData(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block,std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec)
{
	this->_spData = spData;
	this->_spBlock = _block;
	this->_blockAxisHLines = _block->m_axisHLines;
	this->_blockAxisVLines = _block->m_axisVLines;
	this->_spRefG = std::make_shared<ReferencePointGenerator>(this->_blockAxisHLines, this->_blockAxisVLines);
	this->_ssgVec = ssgVec;
	return true;
}

bool WallLocationSet::findWallLayer()
{
	std::vector<int> axisIndexVec;//获得纵向轴线的索引
	std::map<std::string, int> layerMap;
	axisIndexVec = returnAxisLineVIndex();
	findCornerLineLayerMap(layerMap, axisIndexVec);
	this->_wallLayer = returnWallLayer(layerMap);
	if (this->_wallLayer.empty())
		return false;
	else
		return true;
}

std::vector<int> WallLocationSet::returnAxisLineVIndex(/*LineData& lineData*/)
{
	std::vector<int> axisIndexVec;//获得纵向轴线的索引
	auto lineData = _spData->m_lineData;

	//std::vector<Line> colorLineVec;//打印轴线
	for (auto axisVLine : _blockAxisVLines)
	{
		//colorLineVec.push_back(axisVLine.second);
		auto index = findLineIndex(lineData, axisVLine.second, 'V');
		if(index!=-1)
		axisIndexVec.push_back(index);//保存竖直轴线索引
	}
	
	//pColorLineVec->push_back(colorLineVec);
	return axisIndexVec;
}

bool WallLocationSet::findCornerLineLayerMap(std::map<std::string, int>& layerWeight, 
	const std::vector<int>& axisIndexVec)
{
	auto cornerdatas = _spData->m_cornerData.corners();
	auto lineData = _spData->m_lineData;

	for (auto axisIndex : axisIndexVec)
	{
		auto cornerVec = cornerdatas[axisIndex];
		//每条线找最上最下
		int upperIndex = -1;
		int lowerIndex = -1;

		for (auto corner : cornerVec)
		{
			int crossLineIndex = (axisIndex == corner.l1) ? corner.l2 : corner.l1;
			auto crossLine = lineData.lines()[crossLineIndex];

			if (crossLine.isDimension())continue;
			if (crossLine.vertical())continue;
			/*if ((crossLine.entity->dwgType >= DRW::DIMENSION_ORDINATE &&
				crossLine.entity->dwgType <= DRW::DIMENSION_DIAMETER) ||
				!crossLine.horizontal())
			{
				continue;
			}*/


			if (upperIndex == -1)
			{
				upperIndex = crossLineIndex;
				lowerIndex = crossLineIndex;
			}
			else {
				if (crossLine.s.y > lineData.lines()[upperIndex].s.y)
				{
					upperIndex = crossLineIndex;
				}
				if (crossLine.s.y < lineData.lines()[lowerIndex].s.y)
				{
					lowerIndex = crossLineIndex;
				}
			}

		}
		//这个if是处理轴网无任何交线的特殊情况！！ 
		if (upperIndex != -1 && lowerIndex != -1)
		{
			//打印轴线交到的最上最下
			std::string layer1 = lineData.lines()[upperIndex].getLayer();
			std::string layer2 = lineData.lines()[lowerIndex].getLayer();
			layerWeight[layer1]++;
			layerWeight[layer2]++;
		}
	}
	return true;
}

std::string WallLocationSet::returnWallLayer(std::map<std::string, int>& layerWeight)
{
	std::string wallLayer;
	int maxCount = 0;
	for (auto pair : layerWeight) 
	{
		if (pair.second > maxCount) 
		{
			maxCount = pair.second;
			wallLayer = pair.first;
		}
	}
	
	return wallLayer;
}

bool WallLocationSet::buildWall()
{
	std::vector<std::vector<int> > wallLineVec1;//第一阶段发现的墙线
	std::vector<std::vector<int> > wallLineVec2;//第二阶段发现的墙线
	std::set<int> hasDeal;//已经处理过的墙线放在这里
	const auto& lineData = _spData->m_lineData.lines();
	for (auto index1 : _wallLineIndex)
	{
		if (lineData[index1].length() < 6000)continue;
		if (hasDeal.count(index1) > 0)
		{
			continue;
		}
		int index2 = findNearestLine(index1, hasDeal);
		if (index2 != -1)
		{
			std::vector<int> twoLineVec;
			twoLineVec.push_back(index1);
			twoLineVec.push_back(index2);
			wallLineVec1.push_back(twoLineVec);
			
			hasDeal.insert(index1);
			hasDeal.insert(index2);
		}
	}
	for (auto vec : wallLineVec1)
	{
		extendWallLine(vec.at(0), vec.at(1), hasDeal, wallLineVec2);
	}
	for (auto vec : wallLineVec1)
	{
		twoLineBuildWall(vec.at(0), vec.at(1));
	}

	for (auto vec : wallLineVec2)
	{
		twoLineBuildWall(vec.at(0), vec.at(1));
	}
	
	

	return true;
}

void WallLocationSet::cleanRepeat()
{
	std::set<Json::Value> wallOutlineSet;
	std::vector<std::shared_ptr<WallLocation> > newSpWallLocVec;//z 墙vector
	for (auto spWall : _spWallLocVec)
	{
		Json::Value outLine = spWall->getWallLocationJsonData();
		if (wallOutlineSet.count(outLine) == 0)
		{
			wallOutlineSet.insert(outLine);
			newSpWallLocVec.push_back(spWall);
		}
	}
	_spWallLocVec = newSpWallLocVec;
}

int WallLocationSet::findNearestLine(int lineIndex, std::set<int>& hasDeal)
{
	
	auto lineData = _spData->m_lineData.lines();
	double ratio_short = 0.4;


	if (lineData[lineIndex].horizontal()) 
	{
		//处理水平墙线
		std::vector<int> probablyIndexVec;
		double left = lineData[lineIndex].s.x;
		double right = lineData[lineIndex].e.x;

		//上下找线（两线在x方向的投影要有较大的相交）
		for (auto index : _wallHLineIndex) 
		{
			if (index == lineIndex)continue;
			if (hasDeal.count(index) != 0)continue;
			auto line = lineData[index];

			double length = publicFunction::twoLineCrossLength(lineData[lineIndex], line);
			double r1 = length / line.length();
			double r2 = length / lineData[lineIndex].length();
			if (r1 > ratio_short&& r2 > ratio_short) 
			{
				probablyIndexVec.push_back(index);
			}
		}
		//找一条距离最近的线
		if (probablyIndexVec.size() == 0)
		{
			return -1;
		}
			
		int nearestLineIndex = probablyIndexVec[0];
		for (auto index : probablyIndexVec) 
		{
			if (publicFunction::calculateParallelDistance(lineData[index], lineData[lineIndex]) < publicFunction::calculateParallelDistance(lineData[nearestLineIndex], lineData[lineIndex])&&
				publicFunction::calculateParallelDistance(lineData[index], lineData[lineIndex])>10)
			{
				nearestLineIndex = index;
			}
		}

		if (publicFunction::calculateParallelDistance(lineData[nearestLineIndex], lineData[lineIndex]) < 2000)
		{
			return nearestLineIndex;
		}	
		else
		{
			return -1;
		}
	}
	else if (lineData[lineIndex].vertical()) 
	{
		//处理竖直墙线

		std::vector<int> probablyIndexVec;
		double down = lineData[lineIndex].s.y;
		double upper = lineData[lineIndex].e.y;

		//范围  [left,right]
		for (auto index : _wallVLineIndex) 
		{
			if (index == lineIndex)
				continue;
			if (hasDeal.count(index) != 0)
				continue;
			auto line = lineData[index];
			double length = publicFunction::twoLineCrossLength(lineData[lineIndex], line);
			double r1 = length / line.length();
			double r2 = length / lineData[lineIndex].length();
			if (r1 > ratio_short&& r2 > ratio_short)
			{
				probablyIndexVec.push_back(index);
			}
		}
		//找一条距离最近的线
		if (probablyIndexVec.size() == 0)
		{
			return -1;
		}
		int nearestLineIndex = probablyIndexVec[0];
		for (auto index : probablyIndexVec) 
		{
			if (publicFunction::calculateParallelDistance(lineData[index], lineData[lineIndex]) <
				publicFunction::calculateParallelDistance(lineData[nearestLineIndex], lineData[lineIndex])&&
				publicFunction::calculateParallelDistance(lineData[index], lineData[lineIndex]) > 10)
			{
				nearestLineIndex = index;
			}
		}
		if (publicFunction::calculateParallelDistance(lineData[nearestLineIndex], lineData[lineIndex]) < 2000)
		{
			return nearestLineIndex;
		}
		else
		{
			return -1;
		}

	}
	else 
	{
		//处理斜墙线
	}
	return -1;
}

void WallLocationSet::extendWallLine(int lineIndex1, int lineIndex2, 
	std::set<int>& hasDealWallLineSet, 
	std::vector<std::vector<int>>& wallLineVec2)
{
	auto cornerData = _spData->m_cornerData.corners();//交点信息
	auto lineData = _spData->m_lineData.lines();//线数据
	auto cornerVec1 = cornerData[lineIndex1];//线一的cornerVec
	auto cornerVec2 = cornerData[lineIndex2];//线2的cornerVec
	
	for (auto corner : cornerVec1) {//遍历线1的交点
		auto crossLineIndex1 = corner.l1 == lineIndex1 ? corner.l2 : corner.l1;//得到线1的交线
		if (lineData[crossLineIndex1].getLayer() != _wallLayer)continue;//此交线必须在wallLayer图层
		if (hasDealWallLineSet.count(crossLineIndex1) != 0)continue;//此交线不能再hasDealWallLineSet中
		
		for (auto corner2 : cornerVec2) {//遍历线2的交线
			auto crossLineIndex2 = corner2.l1 == lineIndex2 ? corner2.l2 : corner2.l1;//得到线2的交线

			//GreenIndexVec.push_back(lineIndex2);
			if (lineData[crossLineIndex2].getLayer() != _wallLayer)continue;//交线必须在wallLayer图层中

			if (hasDealWallLineSet.count(crossLineIndex2) != 0)continue;//此交线不能在hasDealWallLineSet中
			if (!IsParallelLines(lineData[crossLineIndex1], lineData[crossLineIndex2]))continue;//两线需要平行
			if (publicFunction::calculateParallelDistance(lineData[crossLineIndex1], lineData[crossLineIndex2]) > 2000)continue;//两条线不能太远
			if (publicFunction::calculateParallelDistance(lineData[crossLineIndex1], lineData[crossLineIndex2]) < 10)continue;//也不能太近
			hasDealWallLineSet.insert(crossLineIndex1);
			hasDealWallLineSet.insert(crossLineIndex2);
			std::vector<int> boxVec;
			boxVec.push_back(crossLineIndex1);
			boxVec.push_back(crossLineIndex2);
			wallLineVec2.push_back(boxVec);
			extendWallLine(crossLineIndex1, crossLineIndex2, hasDealWallLineSet, wallLineVec2);

		}
	}
}

bool WallLocationSet::twoLineBuildWall(int lineIndex1, int lineIndex2)
{
	//std::fstream cout("findNearestLine.txt", std::ios::out);
	auto lineData = _spData->m_lineData.lines();
	std::shared_ptr<WallLocation>tempWallLoc = std::make_shared<WallLocation>(lineData[lineIndex1],lineData[lineIndex2]);
	tempWallLoc->generateReferenceData(*_spRefG);
	tempWallLoc->setWallType(_spBlock->box);
	tempWallLoc->setWallSectionSymbol(_spData, _ssgVec);
	_spWallLocVec.push_back(tempWallLoc);

	
	return true;
	/*double thick = 0.0;
	Point startPoint, endPoint;
	auto borders = setBorderLine(lineData[lineIndex1], lineData[lineIndex2], startPoint, endPoint, thick);
	if (!borders.empty())
	{
		tempWallLoc->setThick(thick);
		tempWallLoc->setBorders(borders);
		tempWallLoc->setWallType(_spBlock->box);
		tempWallLoc->setReferencePoint(findPointReferenceAxis(startPoint),
			findPointReferenceAxis(endPoint));
		tempWallLoc->setWallSectionSymbol(_spData, _ssgVec);
		if(thick!=0)
		{
			_spWallLocVec.push_back(tempWallLoc);
		}
	}

	return true;*/
}

std::vector<Line> WallLocationSet::setBorderLine(const Line& line1, const Line& line2,
	Point& startPoint, Point& endpoint, double& thick)
{
	std::vector<Line>borderLineVec;


	if (line1.horizontal() && line2.horizontal())
	{
		borderLineVec = creatRectangular(line1, line2, startPoint, endpoint, thick, 'H');
	}
	else if (line1.vertical()&&line2.vertical())
	{
		borderLineVec = creatRectangular(line1, line2, startPoint, endpoint, thick, 'V');
	}
	else
	{
		return borderLineVec;//斜边暂时不处理
	}
	return borderLineVec;
}

std::vector<Line> WallLocationSet::creatRectangular(const Line& lien1, const Line& line2,
	Point& startPoint, Point& endpoint, double& thick, const char& ch)
{
	std::vector<Line>borders;
	double leftX = 0, rightX = 0, upY = 0, downY = 0;
	if (ch == 'H')
	{
		leftX = std::min(lien1.s.x, line2.s.x);
		rightX = std::max(lien1.e.x, line2.e.x);
		upY = std::max(lien1.s.y, line2.s.y);
		downY = std::min(lien1.s.y, line2.s.y);

		thick = upY - downY;

		startPoint = Point(leftX, (upY + downY) / 2);//未验证  左中
		endpoint = Point(rightX, (upY + downY) / 2);//未验证    有中
	}
	else if (ch == 'V')
	{
		//double leftX = 0, rightX = 0, upY = 0, downY = 0;
		leftX = std::min(lien1.s.x, line2.s.x);
		rightX = std::max(lien1.s.x, line2.s.x);
		upY = std::max(lien1.e.y, line2.e.y);
		downY = std::min(lien1.s.y, line2.s.y);

		thick = rightX - leftX;

		startPoint = Point((leftX + rightX) / 2, upY);//未验证  上中
		endpoint = Point((leftX + rightX) / 2, downY);//未验证   下中 
	}
	else if (ch == 'S')
	{
		return borders;//斜边暂时不处理
	}

	borders.push_back(Line(Point(leftX, upY), Point(rightX, upY)));//上
	borders.push_back(Line(Point(leftX, downY), Point(rightX, downY)));//下
	borders.push_back(Line(Point(leftX, upY), Point(leftX, downY)));//左
	borders.push_back(Line(Point(rightX, upY), Point(rightX, downY)));//右

	return borders;
}

std::vector<std::pair<std::string, double>> WallLocationSet::findPointReferenceAxis(const Point& point)
{
	std::vector<std::pair<std::string, double>>temp;
	std::pair<std::string, double> Hpair;
	std::pair<std::string, double> Vpair;

	findClosestHAxis(Hpair, _blockAxisHLines, point.y);
	findClosestVAxis(Vpair, _blockAxisVLines, point.x);
	temp.push_back(Hpair);
	temp.push_back(Vpair);
	return temp;
}
//int WallLocationSet::getTwoLineDistance(Line& l1, Line& l2)
//{
//	if (l1.horizontal() && l2.horizontal()) {
//		return std::abs(l1.s.y - l2.s.y);
//	}
//	else if (l1.vertical() && l2.vertical()) {
//		return std::abs(l1.s.x - l2.s.x);
//	}
//	else {
//		return slantingDistanceLine(l1, l2);
//	}
//}

bool WallLocationSet::findWallIndex()
{
	auto lineData = _spData->m_lineData.lines();
	auto vLineIndices = _spData->m_lineData.vLinesIndices();
	auto hLineIndices = _spData->m_lineData.hLinesIndices();
	auto sLineIndices = _spData->m_lineData.sLinesIndices();
	Box mainBox = _spBlock->box;
	mainBox.bottom -= 1000;
	for (int lineIndex = 0; lineIndex < lineData.size(); lineIndex++)
	{
		auto line = lineData.at(lineIndex);
		if(!mainBox.cover(line))continue;
		if (line.getLayer() == _wallLayer)
		{
			_wallLineIndex.push_back(lineIndex);
		}
	}
	for (auto vindice : vLineIndices)
	{
		auto line = lineData[vindice];
		if (!mainBox.cover(line))continue;
		if (line.getLayer() == _wallLayer)
		{
			_wallVLineIndex.push_back(vindice);
		}
	}
	for (auto hindice : hLineIndices)
	{
		auto line = lineData[hindice];
		if (!mainBox.cover(line))continue;
		if (line.getLayer() == _wallLayer)
		{
			_wallHLineIndex.push_back(hindice);
		}
	}
	for (auto sindice : sLineIndices)
	{
		auto line = lineData[sindice];
		if (!mainBox.cover(line))continue;
		if (line.getLayer() == _wallLayer)
		{
			_wallSLineIndex.push_back(sindice);
		}
	}
	return true;
}




