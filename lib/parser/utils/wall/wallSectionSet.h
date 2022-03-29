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
	
	//完成完整的一套横剖配筋图解析流程
	void parseProfileData()
	{
		this->_mainStructureSection.iniDrawing(pColorLineIndexVec, pColorLineVec);
		this->_mainStructureSection.setBaseDate(_spData, _spBlock);
		this->_mainStructureSection.parserMainStructureSectionInfo(*_spData);
		generateProfileWall();
		generateReference();
		generateProfileName();
		searchRebar();

		//画出来剖面墙
		drawWall();
	}
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {

		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	//画墙 便于展示与调试
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
	//得到横剖配筋图的Json数据，这是一个对外接口
	Json::Value getHprofileJsonData()
	{
		Json::Value body;
		for (auto spWallSection : _spWallSectionVec)
		{
			body.append(spWallSection->getWallSectionJsonData());
		}
		return body;
	}
	//输出到文件
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
	//通过两根配对的墙线生成一个墙的基本信息
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
		ptr->setAttitude(this->_mainStructureSection.bottomAttitude);//计算高度
		_spWallSectionVec.push_back(ptr);
	}
	//生成剖面图中的墙
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
	//生成相对轴网的坐标数据
	void generateReference()
	{
		for (auto spWall : _spWallSectionVec)
		{
			spWall->findNearestAxis(this->_spBlock->m_axisVLines);
		}
	}
	//给每个墙体新增字段：所在的剖切符，也就是block的名字
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
	//搜索墙内钢筋
	bool searchRebar()
	{
		searchBp();//搜索水平贯通筋
		searchVConnectRebar();//搜索竖直贯通筋
		searchHaunchRebar();//搜索加腋筋
		searchVNConnectRebar();//搜索竖直非贯通筋
		generateRabarSide();//生成钢筋的内测外侧信息
		return true;
	}
	bool searchBp()//找水平筋
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
	//说明：判断线是否竖向穿过了box,并且在box中的线段占比较大
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
	bool searchVConnectRebar()//找竖直贯通筋
	{
		auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
		auto& lineData = _spData->m_lineData.lines();
		const auto& corners = _spData->m_cornerData.corners();
		std::string rebarLayer = _mainStructureSection.getRebarLayer();
		std::set<int> hasTranverse;
		//遍历剖面墙
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			Line boxLeft = Line(Point(box.left, box.bottom), Point(box.left, box.top));
			//遍历所有钢筋
			for (auto stirPair : stirInfoMap) {
				
				int stirIndex = stirPair.first;
				Line rebarLine = lineData[stirIndex];
				if (hasTranverse.count(stirIndex) != 0)continue;
				if(vLineInBox(*_spData,box,stirIndex))
				{
					std::string pointText = stirPair.second;
					
					//生成贯通筋在墙内的那段 放入testConnectStir
					double upY = (std::min)(box.top, rebarLine.e.y);
					double downY = (std::max)(box.bottom, rebarLine.s.y);
					Line newL(Point(rebarLine.s.x, upY), Point(rebarLine.s.x, downY));
					vConnectRebar vcRebar(stirIndex, newL, pointText);
					//p_wall->addVConnectReabar(stirIndex, newL, pointText);

					//寻找延申
					
					const auto& cornerVec = corners.at(stirIndex);
					//从板顶
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
					//从墙相对边缘处
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
	bool searchHaunchRebar()//找加腋筋
	{
		auto stirInfoMap = _mainStructureSection.getStirInfoMap();
		auto lineData = _spData->m_lineData.lines();
		//遍历剖面墙
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			//遍历钢筋线
			for (auto stirPair : stirInfoMap) {
				int stirIndex = stirPair.first;
				Line& line = lineData[stirIndex];
				//此线有一个端点在box范围内
				if (box.cover(line.s) || box.cover(line.e)) {
					auto cornerData = _spData->m_cornerData.corners();
					auto cornerVec = cornerData[stirIndex];

					if (cornerVec.size() > 0) {
						//找和它端点相交的线
						for (auto corner : cornerVec) {
							if (!box.cover((Point)corner))continue;
							int crossIndex = corner.l1 == stirIndex ? corner.l2 : corner.l1;
							auto crossLine = lineData[crossIndex];
							if (crossLine.getLayer() != _mainStructureSection._rebarLayer)continue;////////////mainLayer
							//交点必是两线交点
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
	//找竖直非贯通筋
	bool searchVNConnectRebar()
	{
		auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
		const auto& lineData = _spData->m_lineData.lines();
		//遍历剖面墙
		for (auto& p_wall : _spWallSectionVec) {
			Box& box = p_wall->getBox();
			//遍历钢筋线
			for (auto stirPair : stirInfoMap) {
				int stirIndex = stirPair.first;
				Line stirLine = lineData[stirIndex];
				if (stirLine.vertical()) {
					//钢筋线在box里面
					if (box.cover(stirLine)) {
						//剔除板线的延申线
						std::vector<int>  crossLineIndexVec3 = publicFunction::findCrossing3(*_spData, stirIndex);
						bool mark = false;
						for (auto index : crossLineIndexVec3)
						{
							if (lineData[index].length() > stirLine.length())//钢筋在板中的那一段更长
							{
								mark = true;
								break;
							}

						}
						if (mark)continue;

						//剔除与加腋筋相连的小竖钢筋线
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
	//生成钢筋的内测外侧信息s
	void generateRabarSide()
	{
		for (auto spWallSection : _spWallSectionVec)
		{
			spWallSection->generateRebarSide();
		}
	}
private:
	std::shared_ptr<Data> _spData;//基础数据
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<Block> _spBlock;//解析的主块
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
	//与TextPointIndicesVec相配合
	Point getTextPoint(int index)
	{
		auto& textpoints = _spData->m_textPointData.textpoints();
		if (textpoints.size() > index)
		{
			return textpoints[index];
		}
		else//超vector范围
		{
			return Point(0, 0);
		}
	}
	std::string _mainLayer;//主体图层
	std::vector<int> _mainLineIndices;//主体图层中的线
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
	//找块中文本点
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
				if (textPoint.getText().find('@') != std::string::npos)//是钢筋描述
				{
					textPointIndicesVec.push_back(index);
				}
			}
		}
	}
	//找钢筋线
	void findRebarLine()
	{
		std::vector<Line> lineVec;
		
		for (auto index : textPointIndicesVec)
		{
			
			Point p = this->getTextPoint(index);
			auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(p.entity);
			
			
			int baseLineIndex= publicFunction::findPointBaseLine(*_spData, p);//问题所在
			//std::cout << baseLineIndex << std::endl;
			if (baseLineIndex != -1)
			{
				
				rebarIndicesVec.push_back(baseLineIndex);//先暂存 可能有用
				_rebarInfoMap[baseLineIndex] = index;    //钢筋索引-文本点索引MAP
			}
		}
		//pColorLineIndexVec->push_back(rebarIndicesVec);
		//pColorLineVec->push_back(lineVec);
	}
	//寻找主体图层
	void findMainLayer() {
		std::vector<int> axisIndexVec;//获得纵向轴线的索引
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
			axisIndexVec.push_back(maxLineIndex);//保存竖直轴线索引
		}

		auto cornerdatas = _spData->m_cornerData.corners();
		//pColorLineIndexVec->push_back(axisIndexVec); 打印找到的轴线
		std::vector<Line> colorLineVec;
		std::vector<int> colorLineIndexVec;
		for (auto axisIndex : axisIndexVec)
		{
			auto cornerVec = cornerdatas[axisIndex];
			//每条线找最上最下
			int upperIndex = -1;
			int lowerIndex = -1;

			for (auto corner : cornerVec)
			{
				int crossLineIndex = (axisIndex == corner.l1) ? corner.l2 : corner.l1;
				Line crossLine = lineData[crossLineIndex];
				if (crossLine.vertical())continue;//必须找水平的交线
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
			//这个if是处理轴网无任何交线的特殊情况！！ 
			//if (upperIndex != -1 && lowerIndex != -1)
			//{
			//	//打印轴线交到的最上最下
			//	colorLineIndexVec.push_back(upperIndex);
			//	std::string layer1 = lineData[upperIndex].getLayer();
			//	std::string layer2 = lineData[lowerIndex].getLayer();
			//	layerMap[layer1]++;
			//	layerMap[layer2]++;
			//}
			if (upperIndex != -1)
			{
				//打印轴线交到的最上最下
				/*colorLineIndexVec.push_back(upperIndex);*/
				std::string layer1 = lineData[upperIndex].getLayer();
				layerMap[layer1]++;
			}
			if (lowerIndex != -1)
			{
				//打印轴线交到的最上最下
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
	//生成墙
	void generateWall()
	{
		findBoard();
		findWall();
	}
	//找板线
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
		//连接板线
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
		//打印板线
		//pColorLineVec->push_back(boardLineVec);
	}
	void findWall()
	{
		//#一个端墙分为上下两个墙
		//1先找最左、最右主体线
		//2再用板截成两半
		//3生成墙对象
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
		//打印墙线 两根板线
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
	void searchHRebar()//找水平非贯通筋
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
				if (line.horizontal())//水平筋
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
		//	for (auto hn : pWall->_vNConnectRebarVec)//竖直非贯通
		//	{
		//		 colorLineVec.push_back(hn.getLine());
		//	}
		//	for (auto hn : pWall->_hNConnectRebarVec)//水平非贯通
		//	{
		//		colorLineVec.push_back(hn.getLine());
		//	}
		//	for (auto hn : pWall->_hConnectRebarVec)//水 贯通
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
	
	//搜索墙洞
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
	//测试用 画线
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

	//参数是_pWallSectionVec  方便之后重构
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
	//与TextPointIndicesVec相配合
	Point getTextPoint(int index)
	{
		auto& textpoints = _spData->m_textPointData.textpoints();
		if (textpoints.size() > index)
		{
			return textpoints[index];
		}
		else//超vector范围
		{
			return Point(0, 0);
		}
	}
	//找块中文本点
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
				if (textPoint.getText().find('@') != std::string::npos)//是钢筋描述
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
			/*if (p.getDirection() == 1) //打印字
			{
				auto dTPoint = std::dynamic_pointer_cast<DRW_Text>(p.entity);
				double autoHeight = dTPoint->autoHeight;
				Line line(p, Point(p.x + autoHeight * p.getText().length() / 3, p.y ));
				lineVec.push_back(line);
			}*/
			int baseLineIndex = publicFunction::findPointBaseLine(*_spData, p);
			if (baseLineIndex != -1)
			{
				rebarIndicesVec.push_back(baseLineIndex);//先暂存 可能有用
				_rebarInfoMap[baseLineIndex] = index;    //钢筋索引-文本点索引MAP
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
		ENDPROFILE,//端墙剖面，一般都是横剖
		VPROFILE,//纵剖墙
		HPROFILE,//横剖墙
		DETAILBLOCK,//小剖面详图
		UNKNOW

	}WallSectionType;

private://process function
	
	//校验此块是否满足提取信息的基本条件
	bool checkBlock(const std::shared_ptr<Block>& _block);
	//初始化基础数据
	bool setBaseData(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block);
	//根据不同的类型调用对应的解析函数
	bool parserSection();
//private:
//
//	//解析横剖配筋图  //这些过程可考虑封装到一个独立的类中  
//	void parserHProfile();
//	bool generateProfileWall();
//	bool searchRebar();
//	bool searchBp();//找水平筋
//	bool searchConnectRebar();//找竖直贯通筋
//	bool searchHaunchRebar();//找加腋筋
//	//找竖直非贯通筋
//	bool searchNConnectRebar();
	
private:
	//解析端墙配筋图
private:
	//解析纵剖配筋图

private://Member
	WallSectionType _wallSectionType;
	//MainStructureSection _mainStructureSection;
	std::shared_ptr<Data> _spData;//基础数据
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<Block> _spBlock;//解析的主块
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