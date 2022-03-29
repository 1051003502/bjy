#include "wallSectionSet.h"

bool WallSectionSet::paserSectionData()
{

	if (checkBlock(_spBlock))
	{
		parserSection();
		return true;
	}

	return false;
}
bool isEndprofile(const Block&  block/*std::string blockName*/){
	std::vector<std::string> strVec;
	std::string blockName = block.name;
	strVec.push_back("端");
	strVec.push_back("东");
	strVec.push_back("西");
	for (auto str : strVec) {
		std::string utf8Str = GbkToUtf8(str);
		if (blockName.find(utf8Str) != std::string::npos)
		{
			return true;
		}
	}
	return false;
};
bool isDetailBlock(Block& block)
{
	return false;//未做详图
	std::string blockName = block.name;
	if (blockName.find("-") != std::string::npos)
	{
		return true;
	}
	return false;
}
bool isHPROFILE(Block& block)
{
	if (block.m_axisHLines.size() == 0 && block.m_axisVLines.size() > 0)
	{
		return true;
		std::vector<std::string> strMustHaveVec;
		strMustHaveVec.push_back("-");
		strMustHaveVec.push_back("剖");
		//可用push_back扩充条件
		for (auto str : strMustHaveVec) {
			std::string utf8Str = GbkToUtf8(str);
			if (block.name.find(utf8Str) == std::string::npos)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}
bool isVPROFILE(Block& block)
{
	return false;
	if (block.m_axisHLines.size() > 0 && block.m_axisVLines.size() == 0)
	{
		return false;
	}
	else
	{
		return false;
	}
}
bool WallSectionSet::checkBlock(const std::shared_ptr<Block>& spBlock)
{
	if (isEndprofile(*spBlock))
	{
		_wallSectionType = WallSectionType::ENDPROFILE;
		return true;
	}
	else if (isHPROFILE(*spBlock))
	{
		_wallSectionType = WallSectionType::HPROFILE;
		return true;
	}
	else if (isVPROFILE(*spBlock))
	{
		_wallSectionType = WallSectionType::VPROFILE;
		return true;
	}
	else if (isDetailBlock(*spBlock))
	{
		_wallSectionType = WallSectionType::DETAILBLOCK;
		return false;
	}
	return false;
}

bool WallSectionSet::setBaseData(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block)
{
	this->_spData = spData;
	this->_spBlock = _block;
	this->_blockAxisHLines = _block->m_axisHLines;
	this->_blockAxisVLines = _block->m_axisVLines;
	return true;
}

bool WallSectionSet::parserSection()
{

	//校验快
	if (_wallSectionType == WallSectionType::HPROFILE)
	{
		HpProfile hpProfile;
		hpProfile.iniDrawing(pColorLineIndexVec, pColorLineVec);
		hpProfile.setDependency(_spData, _spBlock);
		hpProfile.parseProfileData();
		hpProfile.outputDataToFile("横剖配筋.json");
		/*this->_mainStructureSection.setBaseDate(_spData, _spBlock);
		this->_mainStructureSection.parserMainStructureSectionInfo(*_spData);
		this->parserHProfile();*/
		
		return true;
	}
	else if (_wallSectionType == WallSectionType::VPROFILE)
	{
		VpProfile vpf;
		vpf.iniData(_spData, _spBlock);
		vpf.iniDrawing(pColorLineIndexVec, pColorLineVec);
		vpf.parserWallSectionData();
		
		return true;
	}
	else if (_wallSectionType == WallSectionType::ENDPROFILE)//端墙
	{
		
		EndProfile endsec;
		endsec.iniData(_spData, _spBlock);
		endsec.iniDrawing(pColorLineIndexVec, pColorLineVec);
		endsec.parserWallSectionData();
		endsec.serializeEndPofile(endsec.getpWallSectionVec());
		return true;
	}
	else if (_wallSectionType == WallSectionType::DETAILBLOCK)
	{
		/*std::vector<Line> lineVec;
		this->_mainStructureSection.setBaseDate(_spData, _spBlock);
		this->_mainStructureSection.parserMainStructureSectionInfo(*_spData);
		auto lineIndexVec = _mainStructureSection.baseLineVec;
		Data& data = *_spData;
		pColorLineIndexVec->push_back(lineIndexVec);
		return true;*/
	}
	return false;
}

//void WallSectionSet::parserHProfile()
//{
//
//	generateProfileWall();
//	searchRebar();
//	//打印剖面墙
//#if 1
//	auto lineData = _spData->m_lineData.lines();
//	for (auto spwallSection : _spWallSectionVec)
//	{
//		Box box=spwallSection->getBox();
//		Line l1(Point(box.left, box.top), Point(box.right, box.top));
//		Line l2(Point(box.left, box.bottom), Point(box.right, box.bottom));
//		Line l3(Point(box.left, box.top), Point(box.left, box.bottom));
//		Line l4(Point(box.right, box.top), Point(box.right, box.bottom));
//		std::vector<Line> lineVec;
//		lineVec.push_back(l1);
//		lineVec.push_back(l2);
//		lineVec.push_back(l3);
//		lineVec.push_back(l4);
//		auto hunchVec=spwallSection->getHunchVec();
//		for (auto hunch : hunchVec) {
//			lineVec.push_back(lineData[hunch.getIndex()]);
//		}
//		pColorLineVec->push_back(lineVec);
//	}
//#endif
//}
//
//bool WallSectionSet::generateProfileWall()
//{
//	auto cuttingwallLineVec = this->_mainStructureSection._cuttingWallLineVec;
//	std::set<int> hasTraverse;
//	for (int i = 0;i < cuttingwallLineVec.size();i++) {
//		if (hasTraverse.count(i) > 0)continue;
//		hasTraverse.insert(i);
//		auto line1 = cuttingwallLineVec[i];
//		for (int j = i + 1;j < cuttingwallLineVec.size();j++) {
//			auto line2 = cuttingwallLineVec[j];
//			if (std::abs(line1.s.x - line2.s.x) < 2000 &&
//				std::abs(line1.s.x - line2.s.x) > 10) {
//				double crossLength = twoLineCrossLength(line1, line2);
//				if (crossLength / line1.length() > 0.5 && crossLength / line2.length() > 0.5) {
//					double leftX = std::min(line1.s.x, line2.s.x);
//					double rightX = std::max(line1.s.x, line2.s.x);
//					double upY = std::max(line1.e.y, line2.e.y);
//					double downY = std::min(line1.s.y, line2.s.y);
//					Point p1(leftX, upY);
//					Point p2(rightX, downY);
//
//					auto ptr = std::make_shared<WallSection>(p1, p2);
//					ptr->setAttitude(ptr->getBox().top - this->_mainStructureSection.bottomAttitude);//计算高度
//					
//					_spWallSectionVec.push_back(ptr);
//				}
//
//			}
//		}
//	}
//	return false;
//}
//
//bool WallSectionSet::searchRebar()
//{
//	searchBp();
//	searchConnectRebar();
//	searchHaunchRebar();//这里可能有问题
//	searchNConnectRebar();
//#if 0    //旧打印调试代码
//	auto lineData = _spData->m_lineData.lines();
//	for (auto spwallSection : _spWallSectionVec)
//	{
//		Box box = spwallSection->getBox();
//		Line l1(Point(box.left, box.top), Point(box.right, box.top));
//		Line l2(Point(box.left, box.bottom), Point(box.right, box.bottom));
//		Line l3(Point(box.left, box.top), Point(box.left, box.bottom));
//		Line l4(Point(box.right, box.top), Point(box.right, box.bottom));
//		std::vector<Line> lineVec;
//		lineVec.push_back(l1);
//		lineVec.push_back(l2);
//		lineVec.push_back(l3);
//		lineVec.push_back(l4);
//		auto hunchVec = spwallSection->getHunchVec();
//		for (auto hunch : hunchVec) {
//			lineVec.push_back(lineData[hunch.getIndex()]);
//		}
//		lineBlocks.push_back(lineVec);
//	}
//#endif
//	return false;
//}
//
//bool WallSectionSet::searchBp()
//{
//	for (auto& p_wall : _spWallSectionVec) {
//		for (auto bp : _mainStructureSection.getBpVec()) {
//			if (p_wall->getBox().cover(bp.center_mind)) {
//				p_wall->addHConnectRebar(bp);
//			}
//		}
//	}
//	return false;
//}
//
//bool WallSectionSet::searchConnectRebar()
//{
//	auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
//	auto& lineData = _spData->m_lineData.lines();
//	//遍历剖面墙
//	for (auto& p_wall : _spWallSectionVec) {
//		Box& box = p_wall->getBox();
//		Line boxLeft = Line(Point(box.left, box.bottom), Point(box.left, box.top));
//		for (auto stirPair : stirInfoMap) {
//			int stirIndex = stirPair.first;
//			Line line = lineData[stirIndex];
//			//钢筋线必竖直
//			if (line.vertical()) {
//				//钢筋线在box间
//				if (line.s.x > box.left&& line.s.x < box.right) {
//					double crossLength = twoLineCrossLength(line, boxLeft);
//					if (crossLength / boxLeft.length() > 0.8) {
//						//auto pointEntity = std::dynamic_pointer_cast<DRW_Text>(stirPoint.infoPoint.entity);
//						//导出文本点str
//						std::string pointText = stirPair.second;
//
//						//生成贯通筋在墙内的那一段 放入testConnectStir
//						double upY = std::min(box.top, line.e.y);
//						double downY = std::max(box.bottom, line.s.y);
//						Line newL(Point(line.s.x, upY), Point(line.s.x, downY));
//
//						p_wall->addVConnectReabar(stirIndex, newL, pointText);
//						//std::pair<int, std::string> pair1(stirIndex, pointText);
//						//p_wall->_wallConnectStirVec.push_back(pair1);
//
//						//p_wall->testConnectStir.push_back(newL);
//					}
//
//				}
//			}
//			
//		}
//	}
//	return false;
//}
//
//bool WallSectionSet::searchHaunchRebar()
//{
//	
//	auto stirInfoMap = _mainStructureSection.getStirInfoMap();
//	auto lineData = _spData->m_lineData.lines();
//	//遍历剖面墙
//	for (auto& p_wall : _spWallSectionVec) {
//		Box& box = p_wall->getBox();
//		//遍历钢筋线
//		for (auto stirPair : stirInfoMap) {
//			int stirIndex = stirPair.first;
//				Line& line = lineData[stirIndex];
//				//此线有一个端点在box范围内
//				if (box.cover(line.s) || box.cover(line.e)) {
//					auto cornerData = _spData->m_cornerData.corners();
//					auto cornerVec = cornerData[stirIndex];
//
//					if (cornerVec.size() > 0) {
//						//找和它端点相交的线
//						for (auto corner : cornerVec) {
//							if (!box.cover((Point)corner))continue;
//							int crossIndex = corner.l1 == stirIndex ? corner.l2 : corner.l1;
//							auto crossLine = lineData[crossIndex];
//							if (crossLine.getLayer() != _mainStructureSection._rebarLayer)continue;////////////mainLayer
//							//交点必是两线交点
//							if (ConnectPoint((Point)corner, line.s) || ConnectPoint((Point)corner, line.e)) {
//								if (ConnectPoint((Point)corner, crossLine.s) || ConnectPoint((Point)corner, crossLine.e)) {
//
//									if (returnLineType(line) == 3) {
//										
//										Haunch hunchrebar(stirIndex, stirPair.second, line);
//										p_wall->addHaunch(hunchrebar);
//										
//									}
//									else if (returnLineType(crossLine) == 3) {
//										Haunch hunchrebar(crossIndex, stirPair.second, crossLine);
//										p_wall->addHaunch(hunchrebar);
//										//testLineVec.push_back(crossLine);
//									}
//								}
//							}
//						}
//
//
//					}
//				}
//		}
//	}
//	return false;
//}
//
//bool WallSectionSet::searchNConnectRebar()
//{
//	auto& stirInfoMap = _mainStructureSection.getStirInfoMap();
//	const auto& lineData = _spData->m_lineData.lines();
//	//遍历剖面墙
//	for (auto& p_wall : _spWallSectionVec) {
//		Box& box = p_wall->getBox();
//		//遍历钢筋线
//		for (auto stirPair : stirInfoMap) {
//			int stirIndex = stirPair.first;
//			Line stirLine = lineData[stirIndex];
//			if (stirLine.vertical()) {
//				//钢筋线在box里面
//				if (box.cover(stirLine)){
//					std::string stirText = stirPair.second;
//					p_wall->addVNConnectRebar(stirIndex, stirLine, stirText);
//				}
//			}
//
//		}
//	}
//	return false;
//}
