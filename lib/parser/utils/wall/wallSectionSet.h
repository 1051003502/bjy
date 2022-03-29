#ifndef __PARSER_UTILS_WALLSECTIONSET_H__
#define __PARSER_UTILS_WALLSECTIONSET_H__
#include "wallSection.h"
#include "mainStructureSection.h"

class HpProfile
{
public:
	MainStructureSection _mainStructureSection;
	HpProfile() = default;
	void setDependency(const std::shared_ptr<Data> spData, const std::shared_ptr<Block> spBlock)
	{
		this->_spData = spData;
		this->_spBlock = spBlock;
	}
	
	//���������һ�׺������ͼ��������
	void parseProfileData()
	{
		this->_mainStructureSection.iniDrawing(pColorLineIndexVec, pColorLineVec);
		this->_mainStructureSection.setBaseDate(_spData, _spBlock);
		this->_mainStructureSection.parserMainStructureSectionInfo(*_spData);
		generateProfileWall();
		generateReference();
		generateProfileName();
		searchRebar();

		//����������ǽ
		drawWall();
	}
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {

		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	//��ǽ ����չʾ�����
	void drawWall()
	{
		auto lineData = _spData->m_lineData.lines();
		for (auto spwallSection : _spWallSectionVec)
		{
			Box box = spwallSection->getBox();
			Line l1(Point(box.left, box.top), Point(box.right, box.top));
			Line l2(Point(box.left, box.bottom), Point(box.right, box.bottom));
			Line l3(Point(box.left, box.top), Point(box.left, box.bottom));
			Line l4(Point(box.right, box.top), Point(box.right, box.bottom));
			std::vector<Line> lineVec;
			lineVec.push_back(l1);
			lineVec.push_back(l2);
			lineVec.push_back(l3);
			lineVec.push_back(l4);
			/*auto hunchVec = spwallSection->getHunchVec();
			for (auto hunch : hunchVec) {
				lineVec.push_back(lineData[hunch.getIndex()]);
			}*/
			for (auto vConnect : spwallSection->getVConnectReabarVec())
			{
				lineVec.push_back(vConnect.getLine());
			}
			pColorLineVec->push_back(lineVec);
		}
	}
	//�õ��������ͼ��Json���ݣ�����һ������ӿ�
	Json::Value getHprofileJsonData()
	{
		Json::Value body;
		for (auto spWallSection : _spWallSectionVec)
		{
			body.append(spWallSection->getWallSectionJsonData());
		}
		return body;
	}
	//������ļ�
	void outputDataToFile(std::string fileName)
	{
		std::fstream of(fileName, std::ios::out);
		Json::FastWriter fastWriter;
		auto jsonData = this->getHprofileJsonData();
		of << fastWriter.write(jsonData);
		of.close();
	}
private:
	void parserHProfile()
	{
		this->generateProfileWall();
		this->searchRebar();
	}
	//ͨ��������Ե�ǽ������һ��ǽ�Ļ�����Ϣ
	void twoLineBuildProfileWall(Line& line1, Line& line2)
	{
		double leftX = (std::min)(line1.s.x, line2.s.x);
		double rightX = (std::max)(line1.s.x, line2.s.x);
		double upY = (std::max)(line1.e.y, line2.e.y);
		double downY = (std::min)(line1.s.y, line2.s.y);
		Point p1(leftX, upY);
		Point p2(rightX, downY);

		auto ptr = std::make_shared<WallSection>(p1, p2);
		int a;
		ptr->setAttitude(this->_mainStructureSection.bottomAttitude);//����߶�
		_spWallSectionVec.push_back(ptr);
	}
	//��������ͼ�е�ǽ
	bool generateProfileWall()
	{
		auto cuttingWallLineVec = this->_mainStructureSection._cuttingWallLineVec;
		/*for (auto line : cuttingWallLineVec)
		{
			std::vector<Line> colorLine;
			colorLine.push_back(line);
			pColorLineVec->push_back(colorLine);
		}*/
		std::set<int> hasTraverse;
		for (int i = 0;i < cuttingWallLineVec.size();i++) {
			if (hasTraverse.count(i) > 0)continue;
			hasTraverse.insert(i);
			auto line1 = cuttingWallLineVec[i];
			for (int j = i + 1;j < cuttingWallLineVec.size();j++) {
				if (hasTraverse.count(j) > 0)continue;
				auto line2 = cuttingWallLineVec[j];
				if (std::abs(line1.s.x - line2.s.x) < 2000 &&
					std::abs(line1.s.x - line2.s.x) > 10) {
					double crossLength = publicFunction::twoLineCrossLength(line1, line2);
					if (crossLength / line1.length() > 0.5 && crossLength / line2.length() > 0.5) {
						hasTraverse.insert(j);
						twoLineBuildProfileWall(line1, line2);
						/*std::vector<Line> colorLine;
						colorLine.push_back(line1);
						colorLine.push_back(line2);
						pColorLineVec->push_back(colorLine);*/
					}

				}
			}
		}
		return false;
	}
	//���������������������
	void generateReference()
	{
		for (auto spWall : _spWallSectionVec)
		{
			spWall->findNearestAxis(this->_spBlock->m_axisVLines);
		}
	}
	//��ÿ��ǽ�������ֶΣ����ڵ����з���Ҳ����block������
	void generateProfileName()
	{
		std::string profileName;
		std::string blockName = _spBlock->name;
		std::size_t foundIndex=blockName.find_first_of("-");
		if (foundIndex != std::string::npos)
		{
			//profileName=blockName[foundIndex+1]+""  
			profileName=blockName.substr(foundIndex - 1, 3);
		}
		for (auto spWall : _spWallSectionVec)
		{
			spWall->setProfileName(profileName);
		}
	}
	//����ǽ�ڸֽ�
	bool searchRebar()
	{
		searchBp();//����ˮƽ��ͨ��
		searchVConnectRebar();//������ֱ��ͨ��
		searchHaunchRebar();//������Ҹ��
		searchVNConnectRebar();//������ֱ�ǹ�ͨ��
		generateRabarSide();//���ɸֽ���ڲ������Ϣ
		return true;
	}
	bool searchBp()//��ˮƽ��
	{
		for (auto& p_wall : _spWallSectionVec) {
			for (auto bp : _mainStructureSection.getBpVec()) {
				if (p_wall->getBox().cover(bp.center_mind)) {
					p_wall->addHConnectRebar(bp);
				}
			}
		}
		return false;
	}
	//˵�����ж����Ƿ����򴩹���box,������box�е��߶�ռ�Ƚϴ�
	bool vLineInBox(Data& data, const Box& box, int lineIndex)
	{
		const auto& lineData = data.m_lineData.lines();
		const Line& line = lineData[lineIndex];
		if (line.horizontal())return false;
		Line boxLeft = Line(Point(box.left, box.bottom), Point(box.left, box.top));
		double crossLength = publicFunction::twoLineCrossLength(line, boxLeft);
		if (crossLength / boxLeft.length() > 0.8  
			&& line.s.x > box.left&& line.s.x < box.right)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool searchVConnectRebar()//����ֱ��ͨ��
	{
		auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
		auto& lineData = _spData->m_lineData.lines();
		const auto& corners = _spData->m_cornerData.corners();
		std::string rebarLayer = _mainStructureSection.getRebarLayer();
		std::set<int> hasTranverse;
		//��������ǽ
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			Line boxLeft = Line(Point(box.left, box.bottom), Point(box.left, box.top));
			//�������иֽ�
			for (auto stirPair : stirInfoMap) {
				
				int stirIndex = stirPair.first;
				Line rebarLine = lineData[stirIndex];
				if (hasTranverse.count(stirIndex) != 0)continue;
				if(vLineInBox(*_spData,box,stirIndex))
				{
					std::string pointText = stirPair.second;
					
					//���ɹ�ͨ����ǽ�ڵ��Ƕ� ����testConnectStir
					double upY = (std::min)(box.top, rebarLine.e.y);
					double downY = (std::max)(box.bottom, rebarLine.s.y);
					Line newL(Point(rebarLine.s.x, upY), Point(rebarLine.s.x, downY));
					vConnectRebar vcRebar(stirIndex, newL, pointText);
					//p_wall->addVConnectReabar(stirIndex, newL, pointText);

					//Ѱ������
					
					const auto& cornerVec = corners.at(stirIndex);
					//�Ӱ嶥
					for (const auto& corner : cornerVec)
					{
						int crossLineIndex = getCrossLineIndexFromCorner(stirIndex, corner);
						auto crossLine = lineData[crossLineIndex];
						if (crossLine.getLayer() == rebarLayer
							&& crossLine.length() < 255
							&& returnLineType(crossLine) == 3
							&& box.cover(crossLine) == false
							)
						{

							std::vector<Line> lineVec;
							lineVec.push_back(crossLine);
							//pColorLineVec->push_back(lineVec);

							auto crossPoint = (Point)corner;
							std::vector<DimensionPair> dmPairVec;
							//crossPoint.y = -1678262.02;
							_spData->m_dimData.findHVDimension(dmPairVec, crossPoint, false, false);
							_spData->m_dimData.findHVDimension(dmPairVec, crossPoint, true, false);
							if (!dmPairVec.empty())

							{
								std::shared_ptr<Dimension> spDimension = _spData->m_dimData.dimensions()[int(dmPairVec[0].first)];
								//pColorLineVec->push_back(spDimension->lines);
								vcRebar.setExtendingType(Rebar::ExtendingType::TopBoard);
								vcRebar.setExtendingLength(spDimension->measurement);
							}
						}
					}
					//��ǽ��Ա�Ե��
					std::vector<Line> colorLineVec;
					for (const auto& corner : cornerVec)
					{
						if (!rebarLine.checkEndPoint(corner))continue;
						int crossLineIndex = getCrossLineIndexFromCorner(stirIndex, corner);
						auto crossLine = lineData[crossLineIndex];
						if (!crossLine.checkEndPoint(corner))continue;
						if (box.cover(corner)
							&& crossLine.horizontal()
							&& crossLine.getLayer() == rebarLayer
							)
						{
							std::vector<DimensionPair> dimPairVec;
							Point tick;
							tick = corner.positionEqual(crossLine.s) ? crossLine.e : crossLine.s;
							_spData->m_dimData.findHVDimension(dimPairVec, tick, false, true);
							_spData->m_dimData.findHVDimension(dimPairVec, tick, true, true);
							if (!dimPairVec.empty())
							{
								hasTranverse.insert(crossLineIndex);
								auto& dimensions = _spData->m_dimData.dimensions();
								auto spDimension = dimensions[int(dimPairVec.at(0).first)];
								//pColorLineVec->push_back(spDimension->lines);
								auto& lines = spDimension->lines;
								vcRebar.setExtendingType(Rebar::ExtendingType::WallMargin);
								vcRebar.setExtendingLength(spDimension->measurement);
								colorLineVec.insert(colorLineVec.end(), lines.begin(), lines.end());
								
							}
							colorLineVec.push_back(crossLine);
						}
					}
					//pColorLineVec->push_back(colorLineVec);
					p_wall->addVConnectReabar(vcRebar);
				}
				

					
				

			}
		}
		return false;
	}
	bool searchHaunchRebar()//�Ҽ�Ҹ��
	{
		auto stirInfoMap = _mainStructureSection.getStirInfoMap();
		auto lineData = _spData->m_lineData.lines();
		//��������ǽ
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			//�����ֽ���
			for (auto stirPair : stirInfoMap) {
				int stirIndex = stirPair.first;
				Line& line = lineData[stirIndex];
				//������һ���˵���box��Χ��
				if (box.cover(line.s) || box.cover(line.e)) {
					auto cornerData = _spData->m_cornerData.corners();
					auto cornerVec = cornerData[stirIndex];

					if (cornerVec.size() > 0) {
						//�Һ����˵��ཻ����
						for (auto corner : cornerVec) {
							if (!box.cover((Point)corner))continue;
							int crossIndex = corner.l1 == stirIndex ? corner.l2 : corner.l1;
							auto crossLine = lineData[crossIndex];
							if (crossLine.getLayer() != _mainStructureSection._rebarLayer)continue;////////////mainLayer
							//����������߽���
							if (ConnectPoint((Point)corner, line.s) || ConnectPoint((Point)corner, line.e)) {
								if (ConnectPoint((Point)corner, crossLine.s) || ConnectPoint((Point)corner, crossLine.e)) {

									if (returnLineType(line) == 3) {

										Haunch hunchrebar(stirIndex, stirPair.second, line);
										p_wall->addHaunch(hunchrebar);

									}
									else if (returnLineType(crossLine) == 3) {
										Haunch hunchrebar(crossIndex, stirPair.second, crossLine);
										p_wall->addHaunch(hunchrebar);
										//testLineVec.push_back(crossLine);
									}
								}
							}
						}


					}
				}
			}
		}
		return false;
	}
	//����ֱ�ǹ�ͨ��
	bool searchVNConnectRebar()
	{
		auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
		const auto& lineData = _spData->m_lineData.lines();
		//��������ǽ
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			//�����ֽ���
			for (auto stirPair : stirInfoMap) {
				int stirIndex = stirPair.first;
				Line stirLine = lineData[stirIndex];
				if (stirLine.vertical()) {
					//�ֽ�����box����
					if (box.cover(stirLine)) {
						//�޳����ߵ�������
						std::vector<int>  crossLineIndexVec3 = publicFunction::findCrossing3(*_spData, stirIndex);
						bool mark = false;
						for (auto index : crossLineIndexVec3)
						{
							if (lineData[index].length() > stirLine.length())//�ֽ��ڰ��е���һ�θ���
							{
								mark = true;
								break;
							}

						}
						if (mark)continue;

						//�޳����Ҹ��������С���ֽ���
						std::vector<int>  crossLineIndexVec2 = publicFunction::findCrossing2(*_spData, stirIndex);
						for (auto index : crossLineIndexVec2)
						{
							if (returnLineType( lineData[index])==3)
							{
								mark = true;
								break;
							}
						}
						if (mark)continue;
						std::string stirText = stirPair.second;
						p_wall->addVNConnectRebar(stirIndex, stirLine, stirText);
					}
				}

			}
		}
		return false;
	}
	//���ɸֽ���ڲ������Ϣs
	void generateRabarSide()
	{
		for (auto spWallSection : _spWallSectionVec)
		{
			spWallSection->generateRebarSide();
		}
	}
private:
	std::shared_ptr<Data> _spData;//��������
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<Block> _spBlock;//����������
	std::vector<std::shared_ptr<WallSection>>_spWallSectionVec;
public:
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
};
class EndProfile
{
private:
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _spBlock;
	std::vector<int> textPointIndicesVec;
	std::vector<int> rebarIndicesVec;
	std::map<int, int> _rebarInfoMap;
	//��TextPointIndicesVec�����
	Point getTextPoint(int index)
	{
		auto& textpoints = _spData->m_textPointData.textpoints();
		if (textpoints.size() > index)
		{
			return textpoints[index];
		}
		else//��vector��Χ
		{
			return Point(0, 0);
		}
	}
	std::string _mainLayer;//����ͼ��
	std::vector<int> _mainLineIndices;//����ͼ���е���
	std::vector<Line> _boardLineVec;
	std::vector<std::shared_ptr<EndProfileSection> > _pWallSectionVec;
public:
	std::vector<std::shared_ptr<EndProfileSection> > getpWallSectionVec() { return _pWallSectionVec; }
public:
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
public:
	EndProfile() = default;
	bool iniData(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		this->_spData = spData;
		this->_spBlock = spBlock;
		return true;
	}
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {

		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	void parserWallSectionData()
	{
		this->findRebarTextPoint();
		this->findRebarLine();
		this->findMainLayer();
		this->generateWall();
		this->searchWallRebar();
		this->searchHole();
		this->draw();
	}
	//�ҿ����ı���
	void findRebarTextPoint()
	{
		Data& data = *_spData;
		auto& textpoints = data.m_textPointData.textpoints();
		for (int index = 0;index < textpoints.size();index++)
		{
			auto& textPoint = textpoints[index];
			auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(textPoint.entity);
			double autoHeight = dTPoint->autoHeight;
			Point textCenter(textPoint.x + autoHeight * textPoint.getText().length() / 3, textPoint.y);
			if (_spBlock->box.cover(textPoint)||_spBlock->box.cover(textCenter))
			{
				if (textPoint.getText().find('@') != std::string::npos)//�Ǹֽ�����
				{
					textPointIndicesVec.push_back(index);
				}
			}
		}
	}
	//�Ҹֽ���
	void findRebarLine()
	{
		std::vector<Line> lineVec;
		
		for (auto index : textPointIndicesVec)
		{
			
			Point p = this->getTextPoint(index);
			auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(p.entity);
			
			
			int baseLineIndex= publicFunction::findPointBaseLine(*_spData, p);//��������
			//std::cout << baseLineIndex << std::endl;
			if (baseLineIndex != -1)
			{
				
				rebarIndicesVec.push_back(baseLineIndex);//���ݴ� ��������
				_rebarInfoMap[baseLineIndex] = index;    //�ֽ�����-�ı�������MAP
			}
		}
		//pColorLineIndexVec->push_back(rebarIndicesVec);
		//pColorLineVec->push_back(lineVec);
	}
	//Ѱ������ͼ��
	void findMainLayer() {
		std::vector<int> axisIndexVec;//����������ߵ�����
		std::map<std::string, int> layerMap;
		std::vector<Axis::AxisLine> _blockAxisVLines = _spBlock->m_axisVLines;
		
		auto& m_lineData = _spData->m_lineData;
		auto& lineData = _spData->m_lineData.lines();
		for (auto axisVLine : _blockAxisVLines)
		{
			int index = -1;
			//auto index = findLineIndex(m_lineData, axisVLine.second, 'V');
			int i = m_lineData.findLowerBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, axisVLine.second.s.x- 3 * Precision, Precision, false);
			int j = m_lineData.findUpperBound(
				LineData::cmLINEDATAMODE::LINEDATAMODE_VINDEX, axisVLine.second.s.x+ 3 * Precision,Precision, false);

			if (i == -1 || j == -1)
			{
				return ;
			}
			int maxLineIndex=-1;
			for (int VI = i; VI <= j; ++VI)
			{
				if (maxLineIndex == -1)
				{
					maxLineIndex = VI;
					continue;
				}
				int lineIndex = m_lineData.vLinesIndices()[VI];
				if (lineData[lineIndex].length() > lineData[maxLineIndex].length())
				{
					maxLineIndex = lineIndex;
				}
				
			}
			if(maxLineIndex!=-1)
			axisIndexVec.push_back(maxLineIndex);//������ֱ��������
		}

		auto cornerdatas = _spData->m_cornerData.corners();
		//pColorLineIndexVec->push_back(axisIndexVec); ��ӡ�ҵ�������
		std::vector<Line> colorLineVec;
		std::vector<int> colorLineIndexVec;
		for (auto axisIndex : axisIndexVec)
		{
			auto cornerVec = cornerdatas[axisIndex];
			//ÿ��������������
			int upperIndex = -1;
			int lowerIndex = -1;

			for (auto corner : cornerVec)
			{
				int crossLineIndex = (axisIndex == corner.l1) ? corner.l2 : corner.l1;
				Line crossLine = lineData[crossLineIndex];
				if (crossLine.vertical())continue;//������ˮƽ�Ľ���
				/*if (crossLine.entity->dwgType == DRW::DIMENSION_DIAMETER||crossLine.vertical())
				{
					continue;
				}*/
				if ((crossLine.entity->dwgType >= DRW::DIMENSION_ORDINATE &&
					crossLine.entity->dwgType <= DRW::DIMENSION_DIAMETER))
				{
					continue;
				}
				if (upperIndex == -1)
				{
					upperIndex = crossLineIndex;
					lowerIndex = crossLineIndex;
				}
				else {
					if (crossLine.s.y > lineData[upperIndex].s.y)
					{
						upperIndex = crossLineIndex;
					}
					if (crossLine.s.y < lineData[lowerIndex].s.y)
					{
						lowerIndex = crossLineIndex;
					}
				}
			}
			//���if�Ǵ����������κν��ߵ������������ 
			//if (upperIndex != -1 && lowerIndex != -1)
			//{
			//	//��ӡ���߽�������������
			//	colorLineIndexVec.push_back(upperIndex);
			//	std::string layer1 = lineData[upperIndex].getLayer();
			//	std::string layer2 = lineData[lowerIndex].getLayer();
			//	layerMap[layer1]++;
			//	layerMap[layer2]++;
			//}
			if (upperIndex != -1)
			{
				//��ӡ���߽�������������
				/*colorLineIndexVec.push_back(upperIndex);*/
				std::string layer1 = lineData[upperIndex].getLayer();
				layerMap[layer1]++;
			}
			if (lowerIndex != -1)
			{
				//��ӡ���߽�������������
				/*colorLineIndexVec.push_back(lowerIndex);*/
				std::string layer2 = lineData[lowerIndex].getLayer();
				layerMap[layer2]++;
			}
			//pColorLineIndexVec->push_back(colorLineIndexVec);
		}
		
		std::string mainLayer;
		int maxCount = 0;
		for (auto pair : layerMap)
		{
			if (pair.second > maxCount)
			{
				maxCount = pair.second;
				mainLayer = pair.first;
			}
		}

		this->_mainLayer = mainLayer;

		for (int index = 0;index < lineData.size();index++)
		{
			const Line& line = lineData[index];
			if (line.getLayer() == _mainLayer)
			{
				if (_spBlock->box.cover(line))
				{
					_mainLineIndices.push_back(index);
				}
			}
		}
		/*pColorLineIndexVec->push_back(_mainLineIndices);*/
		
	}
	//����ǽ
	void generateWall()
	{
		findBoard();
		findWall();
	}
	//�Ұ���
	void findBoard()
	{
		std::vector<int> boardLineindices;
		std::vector<Line> boardLineVec;
		auto& lineData = _spData->m_lineData.lines();
		std::vector<Line> box_lineVec;
		/*for (auto index : _spData->m_lineData.hLinesIndices()) {
			if (lineData[index].getLayer() == _mainLayer) {
				boardLineindices.push_back(index);
			}
		}*/
		for (auto index : _mainLineIndices) {
			if (lineData[index].getLayer() == _mainLayer && lineData[index].horizontal()) {
				boardLineindices.push_back(index);
			}
		}
		std::set<int> hasTraverse;
		//���Ӱ���
		//hasTraverse.clear();
		for (int i = 0;i < boardLineindices.size();i++) {
			int lineIndex = boardLineindices[i];
			const Line& line = lineData[lineIndex];
			if (line.length() < _spBlock->box.width() * 0.1)continue;
			if (hasTraverse.count(lineIndex) != 0)continue;
			hasTraverse.insert(lineIndex);
			Point s = line.s;
			Point e = line.e;
			for (int j = i + 1;j < boardLineindices.size();j++) {
				int lineOtherIndex = boardLineindices[j];
				const Line& lineOther = lineData[lineOtherIndex];
				if (std::abs(line.s.y - lineOther.s.y) < Precision) {
					hasTraverse.insert(lineOtherIndex);
					if (lineOther.s.x < s.x)s = lineOther.s;
					if (lineOther.e.x > e.x)e = lineOther.e;
				}
			}
			Line newline = Line(s, e);
			if (newline.length() > _spBlock->box.width() * 0.5)
				boardLineVec.push_back(newline);
		}
		auto cmp = [](const Line& l1, const Line& l2){
			return l1.s.y < l2.s.y;
		};
		std::sort(boardLineVec.begin(), boardLineVec.end(), cmp);
		_boardLineVec.insert(_boardLineVec.end(), boardLineVec.begin(), boardLineVec.end());
		//��ӡ����
		//pColorLineVec->push_back(boardLineVec);
	}
	void findWall()
	{
		//#һ����ǽ��Ϊ��������ǽ
		//1������������������
		//2���ð�س�����
		//3����ǽ����
		//1
		auto& lineData = _spData->m_lineData.lines();
		std::vector<int> wallLineIndices;
		for (auto index : _mainLineIndices) {
			if (lineData[index].vertical()) {
				wallLineIndices.push_back(index);
			}
		}
		int mostLeftLineIndex= wallLineIndices[0];
		int mostRightLineIndex=wallLineIndices[0];
		for (auto index : wallLineIndices) {
			const Line& line = lineData[index];
			if (line.s.x < lineData[mostLeftLineIndex].s.x) {
				mostLeftLineIndex = index;
			}
			if (line.e.x > lineData[mostRightLineIndex].e.x) {
				mostRightLineIndex = index;
			}
		}

		//2
		if (_boardLineVec.size() != 6)return;
		Line leftLine = lineData[mostLeftLineIndex];
		Line rightLine = lineData[mostRightLineIndex];
		Line downBoard = _boardLineVec[0];
		Line upBoard = _boardLineVec[3];
		//��ӡǽ�� ��������
		/*std::vector<Line> lineBox;
		lineBox.push_back(leftLine);
		lineBox.push_back(rightLine);
		lineBox.push_back(downBoard);
		lineBox.push_back(upBoard);
		pColorLineVec->push_back(lineBox);*/
		//3
		Point p111(leftLine.s.x,leftLine.e.y);
		Point p112(rightLine.s.x,rightLine.e.y);
		Point p121(leftLine.s.x, upBoard.s.y);
		Point p122(rightLine.s.x, upBoard.s.y);
		std::shared_ptr<EndProfileSection> pWall1 = std::make_shared<EndProfileSection>(p111, p122);
		Point p211(leftLine.s.x, upBoard.e.y);
		Point p212(rightLine.s.x, upBoard.e.y);
		Point p221(leftLine.s.x, downBoard.s.y);
		Point p222(rightLine.s.x, downBoard.s.y);
		std::shared_ptr<EndProfileSection> pWall2 = std::make_shared<EndProfileSection>(p211, p222);
		_pWallSectionVec.push_back(pWall1);
		_pWallSectionVec.push_back(pWall2);
		
	}
	void searchWallRebar() 
	{
		searchHRebar();
		searchVRebar();
		showRebar();
	}
	void searchHRebar()//��ˮƽ�ǹ�ͨ��
	{
		
		auto& lineData = _spData->m_lineData.lines();
		for (auto& pWall : _pWallSectionVec)
		{
			Box& box = pWall->getBox();
			
			for (auto pair : _rebarInfoMap)
			{
				int lineIndex = pair.first;
				int pointIndex = pair.second;
				std::string rebarText = getTextPoint(pointIndex).getText();
				const Line& line = lineData[lineIndex];
				if (!box.cover(line))continue;
				if (line.horizontal())//ˮƽ��
				{
					if (line.length() / box.width() < 0.5)
					{
						//std::cout << "addHNC" << std::endl;
						pWall->addHNConnectRebar(lineIndex, line, rebarText);
					}
					else if (line.length() / box.width() >= 0.5)
					{
						//std::cout << "addHNC" << std::endl;
						pWall->addHConnectRebar(lineIndex, line, rebarText);
					}
				}
			}
		}
	}
	void searchVRebar()
	{
		auto& lineData = _spData->m_lineData.lines();
		for (auto& pWall : _pWallSectionVec)
		{
			Box& box = pWall->getBox();
			for (auto pair : _rebarInfoMap)
			{
				int lineIndex = pair.first;
				int pointIndex = pair.second;
				std::string rebarText = getTextPoint(pointIndex).getText();
				const Line& line = lineData[lineIndex];
				if (line.vertical())
				{
					int maxSY = (std::max) (box.bottom,line.s.y);
					int minEY = (std::min) (box.top, line.e.y);
					int crossLength = minEY - maxSY;
					if (crossLength < 0)continue;
					if (crossLength > box.height()*0.7)
					{
						pWall->addVConnectRebar(lineIndex,line,rebarText);
					}
					else if (crossLength > box.height() * 0.2)
					{
						pWall->addVNConnectRebar(lineIndex, line, rebarText);
					}
				}
			}
		}
	}
	void showRebar()
	{
		//auto& lineData = _spData->m_lineData.lines();
		//for (auto& pWall : _pWallSectionVec)
		//{
		//	std::vector<Line> colorLineVec;
		//	for (auto hn : pWall->_vNConnectRebarVec)//��ֱ�ǹ�ͨ
		//	{
		//		 colorLineVec.push_back(hn.getLine());
		//	}
		//	for (auto hn : pWall->_hNConnectRebarVec)//ˮƽ�ǹ�ͨ
		//	{
		//		colorLineVec.push_back(hn.getLine());
		//	}
		//	for (auto hn : pWall->_hConnectRebarVec)//ˮ ��ͨ
		//	{
		//		colorLineVec.push_back(hn.getLine());
		//	}
		//	plusBoxToLineVec(pWall->getBox(), colorLineVec);
		//	pColorLineVec->push_back(colorLineVec);
		//}
		/*std::vector<Line> colorLineVec;
		for (auto& pair : _rebarInfoMap)
		{
			int index = pair.first;
			Line& line = lineData[index];
			colorLineVec.push_back(line);
		}
		pColorLineVec->push_back(colorLineVec);*/
	}
	
	//����ǽ��
	void searchHole()
	{
		auto& circles = _spData->m_circleData.circles();
		
		for (auto pWall : _pWallSectionVec)
		{
			Box& box = pWall->getBox();
			for (auto circle : circles)
			{
				if (box.cover(circle))
				{
					if (circle.r > box.height() * 0.2 || circle.r > box.width() * 0.2)
					{
						std::shared_ptr<EndProfileSection::Hole> sp = 
							std::make_shared<EndProfileSection::CircleHole>(circle.x,circle.y,circle.r);
						pWall->addHole(sp);
					}
				}
			}
		}
	}
	//������ ����
	void draw() {
		/*std::vector<Line> lineVec;
		std::vector<int> lineIndexVec;
		auto& lineData = _spData->m_lineData.lines();
		for (auto& pWall : _pWallSectionVec)
		{
			std::vector<Line> colorLineVec;
			for (auto sp : pWall->_spHoleVec)
			{
				auto it=sp->getHoleData().begin();
				auto lineVec = sp->getHoleData();
				colorLineVec.insert(colorLineVec.end(), lineVec.begin(), lineVec.end());
			}
			pColorLineVec->push_back(colorLineVec);
		}*/
	}

	//������_pWallSectionVec  ����֮���ع�
	Json::Value serializeEndPofile(std::vector<std::shared_ptr<EndProfileSection> > pWallSectionVec)
	{
		Json::Value res;
		Json::FastWriter fastWriter;
		std::fstream of("wallEndProfile.json", std::ios::out);
		for (auto spEndProfile : pWallSectionVec)
		{
			res.append(spEndProfile->getWallProfileJsonData());
		}
		
		of << fastWriter.write(res);
		of.close();
		return res;
	}
};
class VpProfile
{
public:
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
public:
	VpProfile() = default;
	bool iniData(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		this->_spData = spData;
		this->_spBlock = spBlock;
		return true;
	}
	void iniDrawing(std::vector<std::vector<int> > * temp_index_colorBlocks,
		std::vector<std::vector<Line> > * temp_line_colorBlocks) {

		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	void parserWallSectionData()
	{
		this->findRebarTextPoint();
		this->findRebarLine();
	}
private:
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _spBlock;
	std::vector<int> textPointIndicesVec;
	std::vector<int> rebarIndicesVec;
	std::map<int, int> _rebarInfoMap;
	//��TextPointIndicesVec�����
	Point getTextPoint(int index)
	{
		auto& textpoints = _spData->m_textPointData.textpoints();
		if (textpoints.size() > index)
		{
			return textpoints[index];
		}
		else//��vector��Χ
		{
			return Point(0, 0);
		}
	}
	//�ҿ����ı���
	void findRebarTextPoint()
	{
		Data& data = *_spData;
		auto& textpoints = data.m_textPointData.textpoints();
		for (int index = 0;index < textpoints.size();index++)
		{
			auto& textPoint = textpoints[index];
			auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(textPoint.entity);
			double autoHeight = dTPoint->autoHeight;
			Point textCenter(textPoint.x + autoHeight * textPoint.getText().length() / 3, textPoint.y);
			if (_spBlock->box.cover(textPoint) || _spBlock->box.cover(textCenter))
			{
				if (textPoint.getText().find('@') != std::string::npos)//�Ǹֽ�����
				{
					textPointIndicesVec.push_back(index);
				}
			}
		}
	}
	void findRebarLine()
	{
		//auto lineData = _spData->m_lineData.lines();
		std::vector<Line> lineVec;
		for (auto index : textPointIndicesVec)
		{

			Point& p = this->getTextPoint(index);
			/*if (p.getDirection() == 1) //��ӡ��
			{
				auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(p.entity);
				double autoHeight = dTPoint->autoHeight;
				Line line(p, Point(p.x + autoHeight * p.getText().length() / 3, p.y ));
				lineVec.push_back(line);
			}*/
			int baseLineIndex = publicFunction::findPointBaseLine(*_spData, p);
			if (baseLineIndex != -1)
			{
				rebarIndicesVec.push_back(baseLineIndex);//���ݴ� ��������
				_rebarInfoMap[baseLineIndex] = index;    //�ֽ�����-�ı�������MAP
			}
		}
		pColorLineIndexVec->push_back(rebarIndicesVec);
		//pColorLineVec->push_back(lineVec);
	}
	Json::Value getWallProfileJsonData()
	{
		Json::Value profileJson;
		
		return profileJson;
	}
};
class WallSectionSet
{
public:
	//reference function
	WallSectionSet(){}
	WallSectionSet(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block) {
		setBaseData(spData, _block);
	}
	bool paserSectionData();
	std::vector<std::shared_ptr<WallSection>> getWallSection() {
		return _spWallSectionVec;
	}
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}


private:
	typedef enum _WallSectionType
	{
		ENDPROFILE,//��ǽ���棬һ�㶼�Ǻ���
		VPROFILE,//����ǽ
		HPROFILE,//����ǽ
		DETAILBLOCK,//С������ͼ
		UNKNOW

	}WallSectionType;

private://process function
	
	//У��˿��Ƿ�������ȡ��Ϣ�Ļ�������
	bool checkBlock(const std::shared_ptr<Block>& _block);
	//��ʼ����������
	bool setBaseData(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block);
	//���ݲ�ͬ�����͵��ö�Ӧ�Ľ�������
	bool parserSection();
//private:
//
//	//�����������ͼ  //��Щ���̿ɿ��Ƿ�װ��һ������������  
//	void parserHProfile();
//	bool generateProfileWall();
//	bool searchRebar();
//	bool searchBp();//��ˮƽ��
//	bool searchConnectRebar();//����ֱ��ͨ��
//	bool searchHaunchRebar();//�Ҽ�Ҹ��
//	//����ֱ�ǹ�ͨ��
//	bool searchNConnectRebar();
	
private:
	//������ǽ���ͼ
private:
	//�����������ͼ

private://Member
	WallSectionType _wallSectionType;
	//MainStructureSection _mainStructureSection;
	std::shared_ptr<Data> _spData;//��������
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<Block> _spBlock;//����������
private://result
	std::vector<std::shared_ptr<WallSection>>_spWallSectionVec;
	std::vector<std::shared_ptr<EndProfile>>_spWallEndProfileVec;
public:
	std::vector<std::vector<int> >*  pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
	//std::vector<int> writeLineIndexVec;
	//std::vector<int> greenLineIndexVec;
	//std::vector<std::vector<int> > colorBlocks;
	//std::vector<std::vector<Line>> lineBlocks;
};

#endif