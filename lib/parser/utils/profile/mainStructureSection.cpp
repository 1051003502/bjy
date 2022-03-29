#include "mainStructureSection.h"

//MainStructureSection::MainStructureSection(const Block& block, Data& data)
//{
//	//parserMainStructureSectionInfo(block, data);
//}



bool MainStructureSection::setBaseDate(const std::shared_ptr<Data>& m_data, const std::shared_ptr<Block>& p_block)
{
	this->_spData = m_data;
	this->_spBlock = p_block;
	return true;
}

std::vector<int> MainStructureSection::BFSFindLine(const int& baseLineIndex, CornerData& cornerData, LineData& lineData, bool& baseLineSide)
{
	std::queue<int>queueIndex;//������̲�����ͬ��������
	std::map<int, int>lineIndexMap;//�����޳����Ѿ��������������
	std::vector<int>goalIndexVec;//���ڴ���Ѿ����ҵ�������
	bool baseStart = false;
	bool baseEnd = false;
	//��ʼ
	auto baseLineEntity = lineData.getLine(baseLineIndex);
	queueIndex.push(baseLineIndex);
	lineIndexMap[baseLineIndex] = baseLineIndex;
	goalIndexVec.push_back(baseLineIndex);
	while (!queueIndex.empty())
	{
		int index = queueIndex.front();
		queueIndex.pop();
		//�ཻ����
		auto corners = cornerData.corners().find(index);
		if (corners != cornerData.corners().end())
		{
			for (auto corner : corners->second)
			{
				auto l1 = corner.l1;
				if (l1 == index)
				{
					l1 = corner.l2;
				}

				auto bLineIndexMap = lineIndexMap.find(l1) != lineIndexMap.end() ? true : false;
				auto l1LineEntity = lineData.getLine(l1);
				if (!bLineIndexMap && l1LineEntity.getLayer() == baseLineEntity.getLayer())
				{
					queueIndex.push(l1);
					goalIndexVec.push_back(l1);
					lineIndexMap[l1] = l1;
					if (bLineConnect(baseLineEntity, l1LineEntity))
					{
						if (!baseStart && ConnectPoint(baseLineEntity.s, corner))
						{
							baseStart = true;
						}
						else if (!baseEnd && ConnectPoint(baseLineEntity.e, corner))
						{
							baseEnd = true;
						}
					}
				}
			}
		}
	}

	if (baseStart && baseEnd)
	{
		baseLineSide = true;
	}

	return goalIndexVec;
}

bool MainStructureSection::findLeadLine(const Block& block, Data& data, std::vector<int>& baseLineVec, std::vector<std::vector<int>>& leadLineGroup)
{
	bool mark;
	for (auto baseLineIndex : baseLineVec) {

		leadLineGroup.push_back(BFSFindLine(baseLineIndex, data.m_cornerData, data.m_lineData, mark));
	}
	return false;
}

bool MainStructureSection::checkRebarText(std::string text)
{
	if (text.find("@") == std::string::npos)return false;
	return true;
}

bool MainStructureSection::findBaseLine(const Block& block, Data& data, std::vector<int>& baseLineVec, std::vector<Point>& pointVec)
{
	
	auto& textpoints = data.m_textPointData.textpoints();
	auto& lineData = data.m_lineData.lines();
	//std::fstream cout("findGoalLayerAndIndex.txt", std::ios::out);

	auto cornerdatas = data.m_cornerData.corners();//ȫ��corner
	for (int textPointIndex = 0; textPointIndex < textpoints.size(); textPointIndex++) {
		std::vector<int> leadIndexVec;//���ı�����ص��������ߣ���׼��+�����ߣ�

		auto textpoint = textpoints[textPointIndex];
		if (!block.box.cover(textpoint))continue;//������ڵ�
		auto t_point = std::dynamic_pointer_cast<DRW_Text>(textpoint.entity);
		std::string& point_text = t_point->text;//��ע������
		double point_angle = t_point->angle;//��ע���ֵĽǶ� 0�ȴ���������ˮƽ��
		//���ֱ����Ǹֽ��ע
		if (!checkRebarText(point_text))continue;
		//���ı�����Χ��������
		auto ends = data.m_kdtTreeData.kdtEndpoints().knnSearch(Endpoint(0, textpoint), 4);

		for (auto endpointIndex : ends) {//������4����
			auto endpoint = data.m_endpointData.getEndpoint(endpointIndex);

			//�ж���������ڵ����Ƿ��������ߵ�����
			auto realbaselineIndex = endpoint.index;//�õ��˵��Ӧ���ߵ�����
			auto baseline = lineData[realbaselineIndex];
			//�жϻ�׼������
			if (!checkBaseLine(baseline))continue;
			//��׼�����ı�����Ҫ����һЩ��ϵ
			if (!pointCloseLine(textpoint, baseline))continue;
			auto cornervec = cornerdatas[realbaselineIndex];//�õ�����߶�Ӧ��cornervec

			if (cornervec.size() == 0) {//û�н���Ĳ�������
				continue;
			}
			baseLineVec.push_back(realbaselineIndex);
			pointVec.push_back(textpoint);
			break;
			
			
		}
	}
	return false;
}
bool MainStructureSection::pointCloseLine(const Point& point, const Line& line)
{
	if (!isparallel(point, line))return false;
	auto textPoint = std::dynamic_pointer_cast<DRW_Text>(point.entity);
	std::string str = point.getText();
	//�ı�ˮƽ����
	if (point.getDirection() == 1) {
		//�õ��ı����ĵ�
		double X = point.x + double(textPoint->autoHeight) *(point.getText().length() / 6);
		if (X > line.s.x&& X<line.e.x )return true;
		else return false;
	}
	else if (point.getDirection() == 2) {
		double Y = point.y + double(textPoint->autoHeight) * (point.getText().length() / 6);
		if (Y > line.s.y&& Y<line.e.y )return true;
		else return false;
	}
}
bool MainStructureSection::checkBaseLine(Line line)
{
	double L = line.length();
	if (line.length() < 2000&& line.length() >450)return true;
	return false;
}
bool MainStructureSection::isparallel(const Point& pt, const Line& line)
{
	double point_angle = std::dynamic_pointer_cast<DRW_Text>(pt.entity)->angle;
	if (getPointDirection(pt) == 1) {
		//if (abs(point_angle) < Precision || abs(point_angle - M_PI) < Precision) {
		if (line.horizontal())return true;
	}
	if (getPointDirection(pt) == 0) {
		if (line.vertical())return true;
	}
	return false;
}
int MainStructureSection::getPointDirection(const Point& p)
{
	double point_angle = std::dynamic_pointer_cast<DRW_Text>(p.entity)->angle;
	//k��PIˮƽ
	double value1 = point_angle / M_PI;
	double value2 = (int)(point_angle / M_PI);
	double value3 = value1 - value2;
	point_angle += PrecisionAngle;
	if (abs(point_angle / M_PI - (int)(point_angle / M_PI)) < PrecisionAngle) {
		return 1;
	}
	//k�� 0.5PI ��ֱ
	if (abs(point_angle / M_PI_2 - (int)(point_angle / M_PI_2)) < PrecisionAngle && abs(point_angle) > PrecisionAngle) {
		return 0;
	}
	return -1;
}
//��һ��������С���߽߱�L   Ҳ����length<L��ΪС����
double MainStructureSection::findSmallLine(std::vector<Line>& lineData, std::vector<int> lineIndexVec)
{
	double maxLength = 0;
	for (auto lineIndex : lineIndexVec) {
		if (lineData[lineIndex].length() > maxLength)maxLength = lineData[lineIndex].length();
	}
	return maxLength / 4;
}
bool MainStructureSection::parserMainStructureSectionInfo(Data& data)
{

	
	std::vector<std::vector<int> > leadLineGroup;
	std::vector<Point> pointVec;
	//��ʼ�����ϵ����ݽṹ ���ͼ��
	iniStirBreakPoint(data);
	//���ı��� �Լ�����      ������Ż�Ϊ��������  
	findBaseLine(*_spBlock, data, this->baseLineVec, pointVec);
	
	findLeadLine(*_spBlock, data, this->baseLineVec, leadLineGroup);
	//�Ҹֽ�ͼ��
	findRebarLayer(data, leadLineGroup);
	//�ҳ����еĸֽ�������
	findRebarLineIndex(data);
	
	mapStrAndRebar(data, leadLineGroup, pointVec);
	cleanRebarData(*_spBlock, data);
	findMainLine();
#if 0
	for (auto spi : _stirPointInfoVec) {
		std::vector<int> colorBlock;
		for (auto index : spi.leadIndexVec) {
			colorBlock.push_back(index);
		}
		for (auto index : spi.stirIndexVec) {
			colorBlock.push_back(index);
		}
		colorBlocks.push_back(colorBlock);

	}
#endif
	return false;

}



//�ж������Ƿ�λ����ͬ
auto pointPositionEqual = [](Point& p1, Point& p2)->bool {
	return (std::abs(p1.x - p2.x) < Precision && std::abs(p1.y - p2.y) < Precision);
};

//ĳ���Ƿ��ڶϵ㷶Χ�� 
auto StirBreakPointsurround = [](std::vector<StirBreakPoint>& bpVec, const Point& p, int& StirBreakPointindex)->bool {
	for (int bpIndex = 0; bpIndex < bpVec.size(); bpIndex++) {
		if (bpVec.at(bpIndex).surround(p)) {
			StirBreakPointindex = bpIndex;
			return true;
		}
	}
	StirBreakPointindex = -1;
	return false;
};



bool MainStructureSection::findRebarLayer(Data& data
	, std::vector<std::vector<int> >& leadLineGroup)
{
	std::map<std::string, int> layerMap;
	int needAmount = 50;//ͨ�������50��������ı���Ѱ�Ҹֽ�ͼ�� �Ƚ����� Ч��Ҳ������
	auto cornerData = data.m_cornerData.corners();
	auto lineData = data.m_lineData.lines();
	for (auto leadVec : leadLineGroup) {
		std::string leadLineLayer = lineData[leadVec[0]].getLayer();
		for (auto leadLineIndex : leadVec) {
			auto corners = cornerData[leadLineIndex];
			for (auto corner : corners) {
				auto line = lineData[leadLineIndex];
				int crossLineIndex = corner.l1 == leadLineIndex ? corner.l2 : corner.l1;
				if (std::find(_bplineIndexVec.begin(), _bplineIndexVec.end(), crossLineIndex) != _bplineIndexVec.end())continue;
				std::string crossLineLayer = lineData[crossLineIndex].getLayer();
				//���˱��ͼ�����(��ˮƽ��ͼ��)
				if (crossLineLayer != leadLineLayer) {
					//�����Ƕ˵�
					if (pointPositionEqual((Point)corner, line.s) || pointPositionEqual((Point)corner, line.e))
					{
						layerMap[crossLineLayer]++;
						--needAmount;
					}
				}

			}
		}
		if (needAmount < 0)break;
	}
	std::string rebarLayer;
	for (auto pair : layerMap) {
		if (rebarLayer == "")rebarLayer = pair.first;
		else if (pair.second > layerMap[rebarLayer]) {
			rebarLayer = pair.first;
		}
	}
	this->_rebarLayer = rebarLayer;

	return false;
}
bool MainStructureSection::findRebarLineIndex(Data& data)
{
	auto lineData = data.m_lineData.lines();
	for (auto lineIndex : data.m_lineData.hLinesIndices()) {
		if (lineData[lineIndex].getLayer() == _rebarLayer) {
			_hLineIndexVec.push_back(lineIndex);
		}
	}
	for (auto lineIndex : data.m_lineData.vLinesIndices()) {
		if (lineData[lineIndex].getLayer() == _rebarLayer) {
			_vLineIndexVec.push_back(lineIndex);
		}
	}
	for (auto lineIndex : data.m_lineData.sLinesIndices()) {
		if (lineData[lineIndex].getLayer() == _rebarLayer) {
			_sLineIndexVec.push_back(lineIndex);
		}
	}
	//��ӡб��
	/*for (auto lineIndex : _sLineIndexVec) {
		writeIndexVec.push_back(lineIndex);
	}
	colorBlocks.push_back(writeIndexVec);*/
	return true;
}


bool MainStructureSection::mapStrAndRebar(Data& data, std::vector<std::vector<int>>& leadLineGroup, std::vector<Point>& pointVec)
{
	auto cornerData = data.m_cornerData.corners();
	auto lineData = data.m_lineData.lines();
	for (int index = 0; index < leadLineGroup.size(); index++) {
		auto leadVec = leadLineGroup[index];
		Point& point = pointVec[index];
		
		double smallLineLength = findSmallLine(lineData, leadVec);
		std::vector<int> stirIndexVec;
		std::set<int> stirIndexSet;
		for (auto leadLineIndex : leadVec) {
			auto line = lineData[leadLineIndex];
			//�ȴ����������κ��ϵ�����
			int mark = 0;
			for (auto& bp : _bpVec) {
				if (bp.surround(line.s) || bp.surround(line.e)) {
					bp.addTextPoint(point);
					mark = 1;
					break;
				}
			}
			if (mark == 1)continue;
			//�������θֽ��ߵ����
			auto corners = cornerData[leadLineIndex];
			for (auto corner : corners) {

				int crossLineIndex = corner.l1 == leadLineIndex ? corner.l2 : corner.l1;
				//if (std::find(_bplineIndexVec.begin(), _bplineIndexVec.end(), crossLineIndex) != _bplineIndexVec.end())continue;
				std::string crossLineLayer = lineData[crossLineIndex].getLayer();
				//���˸ֽ�ͼ�����
				if (crossLineLayer == _rebarLayer) {
					//line��С����
					if (line.length() < smallLineLength) {
						stirIndexSet.insert(crossLineIndex);
					}
					//�����Ƕ˵�
					if (line.checkEndPoint(corner) )
					{
						stirIndexSet.insert(crossLineIndex);
					}
				}

			}
		}

		if (stirIndexSet.size() > 0) {
			stirIndexVec.insert(stirIndexVec.end(), stirIndexSet.begin(), stirIndexSet.end());
			StirPointInfo spi(point, leadVec, stirIndexVec);
			spi.setBaseLineIndex(getBaseLineIndexVec()[index]);
			_stirPointInfoVec.push_back(spi);
		}
	}
	return false;
}

bool MainStructureSection::cleanRebarData(Block& block, Data& data)
{
	/*1������ͬ�������ֽ��ߵ�stirpoint
	2ɾ��stirpoint�в��Եĸֽ�����*/
	auto& spiVec=getStirPointInfoVec();
	auto lineData = this->_spData->m_lineData.lines();
	for (int i = 0;i < spiVec.size();i++)
	{
		auto& spi1 = spiVec[i];
		auto& stirIndexVec1 = spi1.getStirIndexVec();
		if (stirIndexVec1.size() != 2)continue;
		for (int j = i+1;j < spiVec.size();j++)
		{
			auto& spi2 = spiVec[j];
			auto& stirIndexVec2 = spi2.getStirIndexVec();
			if (stirIndexVec2.size() != 2)continue;
			std::set<int> boxSet;
			boxSet.insert(stirIndexVec1.begin(), stirIndexVec1.end());
			int amount1 = boxSet.size();
			boxSet.insert(stirIndexVec2.begin(), stirIndexVec2.end());
			int amount2 = boxSet.size();
			boxSet.clear();
			boxSet.insert(spi1.getLeadIndexVec().begin(), spi1.getLeadIndexVec().end());
			int amount3 = boxSet.size();
			boxSet.insert(spi2.getLeadIndexVec().begin(), spi2.getLeadIndexVec().end());
			int amount4 = boxSet.size();
			if ( amount1==amount2&&amount3==amount4) {//����stirpoint��������ȫ��ͬ�����εĸֽ���Ҳ��ȫ��ͬ
				//������ֱ����  �ı����Ӧ��ֽ� �Ҷ���
				Line& line1 = lineData[spi1.getBaseLineIndex()];
				Line& line2 = lineData[spi2.getBaseLineIndex()];
				if (line1.vertical() && line2.vertical()) {
					if (line1.s.x < line2.s.x) {
						Line& stirLine1 = lineData[stirIndexVec1[0]];
						Line& stirLine2= lineData[stirIndexVec1[1]];
						if (stirLine1.s.x < stirLine2.s.x) {
							//ɾ����stirLine2
							stirIndexVec1.erase(stirIndexVec1.begin() + 1);
						}
						else {
							//ɾ����stirLine1
							stirIndexVec1.erase(stirIndexVec1.begin());
						}
					}
					else {
						Line& stirLine1 = lineData[stirIndexVec1[0]];
						Line& stirLine2 = lineData[stirIndexVec1[1]];
						if (stirLine1.s.x > stirLine2.s.x) {
							//ɾ����stirLine2
							stirIndexVec1.erase(stirIndexVec1.begin() + 1);
						}
						else {
							//ɾ����stirLine1
							stirIndexVec1.erase(stirIndexVec1.begin());
						}
					}
					if (stirIndexVec2[0] == stirIndexVec1[0]) {
						stirIndexVec2.erase(stirIndexVec2.begin());
					}
					else {
						stirIndexVec2.erase(stirIndexVec2.begin()+1);
					}
					
				}
			}
		}
	}

	for (int i = 0;i < spiVec.size();i++)
	{
		auto stirIndexVec = spiVec[i].getStirIndexVec();
		std::string stirStr = spiVec[i].getInfoPoint().getText();
		for (int index : stirIndexVec)
		{
			if (_stirInfoMap.count(index) != 0) {
				std::string str1 = _stirInfoMap[index];
				std::string str2 = stirStr;
				if (str1 != str2) {
					_stirInfoMap[index] = "specialStir";//����������⴦�� һ���߱�������ע����
				}
			}
			else {
				_stirInfoMap[index] = stirStr;
			}

		}
	}
	return false;
}

bool MainStructureSection::connectLine()
{
	auto lineData = _spData->m_lineData.lines();
	std::set<int> hasTraverse;
	//���Ӱ���
	hasTraverse.clear();
	for (int i = 0;i < _boardLineIndexVec.size();i++) {
		int lineIndex = _boardLineIndexVec[i];
		Line& line = lineData[lineIndex];
		if (hasTraverse.count(lineIndex) != 0)continue;
		hasTraverse.insert(lineIndex);
		Point s = line.s;
		Point e = line.e;
		for (int j = i + 1;j < _boardLineIndexVec.size();j++) {
			int lineOtherIndex = _boardLineIndexVec[j];
			Line& lineOther = lineData[lineOtherIndex];
			if (std::abs(line.s.y - lineOther.s.y) < Precision) {
				hasTraverse.insert(lineOtherIndex);
				if (lineOther.s.x < s.x)s = lineOther.s;
				if (lineOther.e.x > e.x)e = lineOther.e;
			}
		}
		Line newline = Line(s, e);
		if (newline.length() > _spBlock->box.width() * 0.4)
			_boardLineVec.push_back(newline);
	}
	auto cmp = [](const Line& l1, const Line& l2) {
		return l1.s.y < l2.s.y;
	};
	std::sort(_boardLineVec.begin(), _boardLineVec.end(), cmp);
	if (_boardLineVec.size() >= 2)
		this->bottomAttitude = _boardLineVec[0].s.y;
	return false;
}



bool MainStructureSection::findMainLine()
{
	//������ͼ��
	findMainLayer();
	//��ǽ��
	findWallLineIndex();
	//�Ұ���
	findBoardLineIndex();
	//���Ӱ��ߡ�ǽ��
	connectLine();
	//�ð�ض�ǽ
	cutWall();
	//pColorLineVec->push_back(_cuttingWallLineVec);
	return false;
}

bool MainStructureSection::findWallLineIndex()
{
	auto lineData = _spData->m_lineData.lines();
	for (auto index : _spData->m_lineData.vLinesIndices()) {
		auto line = lineData[index];
		if (line.getLayer() == _mainLayer) {
			if (line.length() > 4000)
			{
				_wallLineIndexVec.push_back(index);
			}
		}
	}

	return false;
}

bool MainStructureSection::findMainLayer()
{
	auto lineData = _spData->m_lineData.lines();
	std::map < std::string, int > wallLayerMap;
	auto& vLinesIndices = _spData->m_lineData.vLinesIndices();
	//pColorLineIndexVec->push_back(_vLineIndexVec);
	for (auto index : _vLineIndexVec) {
		auto line = lineData[index];
		if (line.length() > 10000/*(_spBlock->box.height()) * 0.3*/) {

			int lowerIdx = _spData->m_lineData.findLowerBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, line.s.x - line.length() * 0.05, Precision, false);
			int upperIdx = _spData->m_lineData.findUpperBound(LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, line.s.x + line.length() * 0.05, Precision, false);
			for (int i = lowerIdx; i <= upperIdx; i++) {
				int idx = vLinesIndices[i];
				if (lineData[idx].getLayer() != _rebarLayer) {
					if (lineData[idx].length() > line.length() * 0.4) {
						wallLayerMap[lineData[idx].getLayer()]++;

					}
				}
			}
			//writeIndexVec.push_back(index);
		}
	}
	std::string mainLayerStr;
	for (auto pair : wallLayerMap) {
		if (mainLayerStr == "") {
			mainLayerStr = pair.first;
		}
		else if (wallLayerMap[mainLayerStr] < pair.second) {
			mainLayerStr = pair.first;
		}
	}
	this->_mainLayer = mainLayerStr;


	//��ӡ����ṹͼ���ߣ�Ҳ�����˺��ߣ��壩
#if 0
	for (int index = 0; index < lineData.size(); index++) {
		if (lineData[index].getLayer() == mainLayerStr) {
			writeIndexVec.push_back(index);
		}
	}
#endif
	return false;
}

bool MainStructureSection::findWallConnectRebar()
{

	return false;
}

bool MainStructureSection::cutWall()
{
	if (_boardLineVec.size() == 6)
	{
		normalCutting();
		return true;
	}
	else if (_boardLineVec.size() == 8)
	{
		specialCutting();
		return true;
	}
	return false;
}

void MainStructureSection::normalCutting()
{
	Line& middleBoardUp = _boardLineVec[3];//�а������һ����
	Line& bottomBoardUp = _boardLineVec[0];//�װ������һ����
	double yUp = middleBoardUp.s.y;
	double yDown = bottomBoardUp.s.y;
	const auto& lineData = _spData->m_lineData.lines();
	for (auto lineIndex : _wallLineIndexVec) {
		Line line = lineData[lineIndex];
		//��yUp�ض�
		if (line.s.y < yUp && line.e.y>yUp) {
			Line wallLineUp(Point(line.s.x, yUp), line.e);
			_cuttingWallLineVec.push_back(wallLineUp);
			//��yDown�ض�
			if (line.s.y<yDown && line.e.y>yDown) {
				Line wallLineDown(Point(line.s.x, yDown), Point(line.s.x, yUp));
				_cuttingWallLineVec.push_back(wallLineDown);
			}
			else {
				Line wallLineDown(Point(line.s), Point(line.s.x, yUp));
				_cuttingWallLineVec.push_back(wallLineDown);
			}
		}
		//����yDown�ض�
		else if (line.s.y < yDown && line.e.y>yDown) {
			Line wallLineDown(Point(line.s.x, yDown), Point(line.e));
			_cuttingWallLineVec.push_back(wallLineDown);
		}
		else {
			_cuttingWallLineVec.push_back(lineData[lineIndex]);
		}
	}
}

void MainStructureSection::specialCutting()
{
	
	

	const auto& lineData = _spData->m_lineData.lines();
	double boardBottom = _boardLineVec[0].s.y;
	double boardTop = _boardLineVec.back().s.y;
	double middleBottom = _boardLineVec[3].s.y;
	double middleTop = _boardLineVec[5].s.y;
	for (auto lineIndex : _wallLineIndexVec)
	{
		Line oneWallLine = lineData[lineIndex];
		if (oneWallLine.s.y<boardBottom && oneWallLine.e.y>boardTop)
		{
			//��Ҫ�ضϵİ���
			double wallX = oneWallLine.s.x;
			Line btmLine = Line(wallX, boardBottom, wallX, middleBottom);
			Line middleLine= Line(wallX, middleBottom, wallX, middleTop);
			Line topLine = Line(wallX, middleTop, wallX, boardTop);
			_cuttingWallLineVec.push_back(btmLine);
			_cuttingWallLineVec.push_back(middleLine);
			_cuttingWallLineVec.push_back(topLine);
			
		}
		else
		{
			_cuttingWallLineVec.push_back(oneWallLine);
		}
	}
	
	//pColorLineVec->push_back(colorLine);//////////
}





bool MainStructureSection::findBoardLineIndex()
{
	auto lineData = _spData->m_lineData.lines();
	double maxBoardLength = 0;
	int maxLineIndex;
	for (auto index : _spData->m_lineData.hLinesIndices()) {
		auto line = lineData[index];
		//����������ͼ��ˮƽ��  ���Ȳ���̫�̣������д���һ�����ǣ�
		if (line.getLayer() == _mainLayer) {
			if (line.length() > maxBoardLength) {
				maxBoardLength = line.length();
				maxLineIndex = index;

			}
			//maxBoardLength = std::max(maxBoardLength, line.length());
		}
	}
	//writeIndexVec.push_back(maxLineIndex);
	for (auto index : _spData->m_lineData.hLinesIndices()) {
		auto line = lineData[index];
		//����������ͼ��ˮƽ��  ���Ȳ���̫�̣������д���һ�����ǣ�
		if (line.getLayer() == _mainLayer) {
			if (line.length() > maxBoardLength * 0.1)
			{
				//�������
				_boardLineIndexVec.push_back(index);
				//��ӡ�ҵ��İ���
				//writeIndexVec.push_back(index);
			}
		}
	}
	return false;
}








bool MainStructureSection::iniStirBreakPoint(Data& data)
{
	auto lineData = data.m_lineData.lines();
	std::set<StirBreakPoint/*, std::less<StirBreakPoint>*/> StirBreakPointSet;//��set����StirBreakPoint���� Ŀ����ȥ��
	//�����ҵ����еĺ�ˮƽ��ϵ�
	std::map<std::string, int> bpLayerMap;
	for (int lineIndex = 0; lineIndex < lineData.size(); lineIndex++) {
		if (_spBlock->box.cover(lineData[lineIndex]) == false)continue;
		if (BoolBreakpoint(lineData[lineIndex], data)) {
			//��ӡ���ϵ���Ϣ
			//writeIndexVec.push_back(lineIndex);
			//��ȡ�����ϵ��ͼ����Ϣ
			bpLayerMap[lineData[lineIndex].getLayer()]++;//��¼���ϵ�ͼ����Ϣ
			StirBreakPointSet.insert(StirBreakPoint(lineData[lineIndex]));//һ�����ϵ��������ߣ�setʵ�����Զ�ȥ��
			_bplineIndexVec.push_back(lineIndex);
		}
	}
	//set->vector
	std::string bpLayer;
	for (auto pair : bpLayerMap) {
		if (bpLayer == "")bpLayer = pair.first;
		else if (pair.second > bpLayerMap[bpLayer]) {
			bpLayer = pair.first;
		}
	}
	this->_bpLayer = bpLayer;
	for (auto p : StirBreakPointSet) {
		_bpVec.push_back(p);
	}
	return true;
}



StirPointInfo::StirPointInfo(Point& infoPoint, std::vector<int>& leadIndexVec, std::vector<int>& stirIndexVec)
{
	this->infoPoint = infoPoint;
	this->leadIndexVec = leadIndexVec;
	this->stirIndexVec = stirIndexVec;

}
