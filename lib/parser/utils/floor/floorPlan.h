#ifndef __PARSER_UTILS_FLOORPLAN_H__
#define __PARSER_UTILS_FLOORPLAN_H__
#include "publicFunction.h"
#include "data.h"
#include "block.h"
#include "json/json.h"
#include "wallLocationSet.h"

//������߶ε��������
class MyPolyline
{
private:
	std::list<Line> lineList;
	std::set<Line> lineSet;
	std::vector<Line> lineVec;
	std::vector<Point> pointVec;
public:
	MyPolyline() = default;
	MyPolyline(std::vector<Line> lineVec)
	{
		this->lineVec = lineVec;
	}
	void setLine(std::vector<Line> lineVec)
	{
		this->lineVec = lineVec;
	}
	//��飺�������߶�ת����˳ʱ���һ����
	//ǰ�����ݣ���Ҫͨ�����캯������lineVec  ����ͨ��setLine����lineVec
	void transeToPoint()
	{
		pointVec.clear();
		if (lineVec.empty())return;
		std::set<Line> notUsingLine;
		for (auto line : lineVec)
		{
			notUsingLine.insert(line);
		}
		Line firstLine = lineVec[0];
		pointVec.push_back(firstLine.s);
		pointVec.push_back(firstLine.e);
		notUsingLine.erase(firstLine);
		while (true)
		{
			Point lastPoint = pointVec.back();
			bool mark = false;
			for (auto line : notUsingLine)
			{
				if (lastPoint.positionEqual(line.s) || lastPoint.positionEqual(line.e))
				{
					Point newPoint = lastPoint.positionEqual(line.s) ? line.e : line.s;
					pointVec.push_back(newPoint);
					notUsingLine.erase(line);
					mark = true;
					break;
				}
			}
			if (notUsingLine.size() == 0)
				break;
			if (!mark)
				break;
		}

	}
	//���:������Ҫ���������ݣ�ÿ�����������������߶α�ʾ������������㣬ǰ�ú�����transeToPoint
	std::vector<Line> getDrawingData()
	{
		std::vector<Line> drawing;
		for (auto point : this->pointVec)
		{
			Point left = Point(point.x - 200, point.y);
			Point right = Point(point.x + 200, point.y);
			Point up = Point(point.x , point.y+200);
			Point down = Point(point.x, point.y - 200);
			Line hline(left, right);
			Line vline(down, up);
			drawing.push_back(hline);
			drawing.push_back(vline);
		}
		return drawing;
	}
	//��飺�ж϶��߶��Ƿ��Ǳպϵģ����������������ݣ����Ը���transeToPoint������˼·��ʵ��
	bool checkClosed()
	{}
	bool insertIntoLineSet(Line line)
	{

		if (lineSet.count(line)!=0)
		{
			lineSet.insert(line);
			return true;
		}
		return false;
	}
	std::vector<Line> getBorderLine()
	{
		std::vector<Line> border;
		border.insert(border.end(), lineSet.begin(), lineSet.end());
		return border;
	}
	std::vector<Point> getPointVec()
	{
		return pointVec;
	}
	//��飺��һ��ReferencePointGenerator���������Ϳ��Եõ���Ҫ����ı߽�Json����
	Json::Value getReferenceData(ReferencePointGenerator& ref)
	{
		if (!ref.checkGenerator())return Json::Value();
		Json::Value result;
		for (auto& point : pointVec)
		{
			auto refPoint= ref.generateReferencePoint(point);
			auto pointJson=refPoint.getJsonData();
			result.append(pointJson);
		}
		return result;
	}
};
//��ֽ��һ�����࣬��һ��������
class BoardRebar
{
public:
	std::string text;//�ֽ��ע
	Line line;       //�ֽ���
	int lineIndex;   //�ֽ�������
	BoardRebar() = default;
	BoardRebar(std::string text = "", Line line = Line(),int lineIndex = -1)
	{
		this->text = text;
		this->line = line;
		this->lineIndex = lineIndex;
	}
};
//��ˮƽͨ���ˮƽ��ͨ����̳��˰�ֽ���࣬������ص�������Ӷ��е����ݣ�������չ
class BoardHConnectRebar:public BoardRebar
{
public:
	BoardHConnectRebar(std::string text = "", Line line = Line(), int lineIndex = -1) :BoardRebar(text, line, lineIndex)
	{

	}
};
//����ֱͨ�����ֱ��ͨ����̳��˰�ֽ���࣬������ص�������Ӷ��е����ݣ�������չ
class BoardVConnectRebar:public BoardRebar
{
public:
	BoardVConnectRebar(std::string text = "", Line line = Line(), int lineIndex = -1) :BoardRebar(text, line, lineIndex)
	{

	}
};
//��Ű�����
class FloorData
{
public:
	Box box;//���һ����������  �����������ڸֽ�
	std::vector<Line> border; //��ı߽���  ֮���Ա�����������Ϊ֮���������У���߽�  ����³����
	MyPolyline polyline;      //�߽��ߵĽ�һ����������Ҫ����У�ԣ��Լ����Json����
	std::vector< BoardHConnectRebar > hcRebarVec;//ˮƽͨ����
	std::vector< BoardVConnectRebar > vcRebarVec;//��ֵͨ����
public:
	
};
//�ֽ�����ߣ�����Ӱ�ƽ�����ͼ���������ֽ�Լ���������ֽ�
class RebarParser
{
public:
	//����primaryParseRebar�ĺ�������
	void test_parseRebar()
	{
		primaryParseRebar();
		pColorLineIndexVec->push_back(_rebarIndicesOrigin);
	}
	//����test_cleanRebar�ĺ�������
	void test_cleanRebar()
	{
		primaryParseRebar();
		findRebarLayer();
		cleanRebar();
		drawRebar();
	}
	//�����Ľ������̣�Ҳ�Ƕ����ṩ����Ҫ�ӿڣ��ⲿ���������ݴ���󣬾͵��ô˺���
	void parse()
	{
		primaryParseRebar();
		findRebarLayer();
		cleanRebar();
		//drawRebar();
	}
	//�Ըֽ���о��࣬һ����ĸֽ�ۺϵ�һ��vector�У���������˶��vector
	void gather()
	{
		_horizontalCluster = gatherHorizontalRebar(_rebarMap);
		_verticalCluster = gatherVerticalRebar(_horizontalCluster);
		/*for (int index = 0;index < _horizontalCluster.size();index++)
		{
			std::vector<int> colorIndexVec;
			std::vector<int> horizontalVec = _horizontalCluster[index];
			std::vector<int> verticalVec = _verticalCluster[index];
			colorIndexVec.insert(colorIndexVec.end(), horizontalVec.begin(), horizontalVec.end());
			colorIndexVec.insert(colorIndexVec.end(), verticalVec.begin(), verticalVec.end());
			pColorLineIndexVec->push_back(colorIndexVec);
		}*/
	}
	std::map<int, std::string> getRebar()
	{
		return _rebarMap;
	}
	std::vector<std::vector<int> >& getHorizontalCluster() { return _horizontalCluster; }
	std::vector<std::vector<int> >& getVerticalCluster() { return  _verticalCluster; }
	//���а�Ļ����ṹ���һ�������Ӱ�ĸֽ����ݣ�ʱ��˳����   ���������ֽ�->���ɰ�->���ֽ���Ϣ
	void fillInRebar(std::vector<FloorData>& floorDataVec)
	{
		//_horizontalCluster
		auto& lineData = _spData->m_lineData.lines();
		for (int i = 0;i < floorDataVec.size();i++)
		{
			//floorDataVec[i].hcRebarVec
			for (auto index : _horizontalCluster[i])
			{
				auto& floorData = floorDataVec[i];
				auto rebarLine = lineData[index];
				if (rebarLine.length() / floorData.box.width() > 0.7)
				{
					BoardHConnectRebar hcRebar(_rebarMap[index]);
					floorData.hcRebarVec.push_back(hcRebar);
				}
			}
			for (auto index : _verticalCluster[i])
			{
				auto& floorData = floorDataVec[i];
				auto rebarLine = lineData[index];
				if (rebarLine.length() / floorData.box.height() > 0.7)
				{
					BoardVConnectRebar vcRebar(_rebarMap[index]);
					floorData.vcRebarVec.push_back(vcRebar);
				}
			}
		}
	}
public:
	//����
	RebarParser() = default;
	RebarParser(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	RebarParser(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//����data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//��ʼ����ͼ����
	void initDrawing(std::vector<std::vector<int> > * temp_index_colorBlocks,
		std::vector<std::vector<Line> > * temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
private:
	//���һ���ַ����Ƿ���һ���ֽ��ע��������ܴ����޷�ʶ��ֽ�����⣬������Ҫ���������������
	bool checkRebarText(std::string text)
	{
		if (text.find("@") == std::string::npos)return false;
		return true;
	}
	//�����������ֽ���������ı��㣬Ȼ���ٸ���findPointBaseLine�����ҵ��ֽ��ı�������Ӧ���ĸֽ�����
	void primaryParseRebar()
	{
		const auto& textpoints = _spData->m_textPointData.textpoints();
		for (int textPointIndex = 0;textPointIndex < textpoints.size(); textPointIndex++)
		{
			const auto& textPoint = textpoints[textPointIndex];
			if (!this->_block->box.cover(textPoint))continue;
			if (checkRebarText(textPoint.getText()))
			{
				int rebarLineIndex = publicFunction::findPointBaseLine(*_spData, textPoint);
				if (rebarLineIndex != -1)
				{
					_rebarPairOrigin.push_back(std::pair<int, Point>(rebarLineIndex, textPoint));
					_rebarIndicesOrigin.push_back(rebarLineIndex);
				}
			}
		}
		//pColorLineIndexVec->push_back(_rebarIndicesOrigin);
	}
	//ͨ���������������ĸֽ�����ȡ�ֽ�ͼ��
	void findRebarLayer()
	{
		_rebarLayer = publicFunction::findMaxAmountLayer(_rebarIndicesOrigin, *_spData);
	}
	//��������иֽ��������ϴ�������ܵ��޳������Ǹֽ���ߣ��Լ��ظ��ĸֽ���
	void cleanRebar()
	{
		//�޳�_rebarOrigin�в��Ǹֽ�ͼ�����
		//XXXXXX
		//XXXXXX

		auto& lineData = _spData->m_lineData.lines();
		//�ϲ�һ���߱�����ı���ע�����
		for (auto& rebarPair : _rebarPairOrigin)
		{
			int index = rebarPair.first;
			if (lineData[index].length() < 100)continue;//̫�̹���
			if (lineData[index].getLayer() != _rebarLayer)continue;//���Ǹֽ�ͼ�����
			Point& p = rebarPair.second;
			_rebarMap[index] = p.getText();
		}
	}

private:
	//�Ӹֽ������еõ�ˮƽ�ֽ�
	std::vector<int> findHorizontal(const std::map<int, std::string>& rebarMap)
	{
		std::vector<int> horizontalVec;
		if (_spData == NULL)return horizontalVec;
		auto& lineData = _spData->m_lineData.lines();
		for (auto& pair : rebarMap)
		{
			int rebarIndex = pair.first;
			Line& rebarLine = lineData.at(rebarIndex);
			if (rebarLine.horizontal())
			{
				horizontalVec.push_back(rebarIndex);
			}
		}
		//pColorLineIndexVec->push_back(horizontalVec);
		return horizontalVec;
	}
	//��һ���ֽ���뵽rebarcClusterh�У������������2�����
	//1rebarCluster���Ѿ��������²���ĸֽ���ͬһ����ĸֽ�
	//2rebarCluster�ĸֽ���²���ĸֽ����ͬһ������
	void insertIndexIntoCluster(int newIndex, std::vector<std::vector<int> >& rebarCluster)
	{
		if (_spData == NULL)return;
		auto& lineData = _spData->m_lineData.lines();
		Line& newLine = lineData[newIndex];
		if (newLine.length() < 6000)return;
		for (auto& cluster : rebarCluster)
		{
			for (auto& index : cluster)
			{
				Line& line = lineData[index];
				if (publicFunction::calculateTwoLineCrossRatio(line, newLine) > 0.8)
				{
					cluster.push_back(newIndex);
					return;
				}
			}
		}
		std::vector<int> cluster;
		cluster.push_back(newIndex);
		rebarCluster.push_back(cluster);
	}
	//ˮƽ����Ľ�ۼ��ɼ���
	std::vector<std::vector<int> > gatherHorizontalRebar(std::map<int, std::string> rebarMap)
	{
		std::vector<int> horizontalVec;
		horizontalVec = findHorizontal(rebarMap);
		std::vector<std::vector<int> > rebarCluster;
		if (horizontalVec.empty())return rebarCluster;
		for (int& index : horizontalVec)
		{
			insertIndexIntoCluster(index, rebarCluster);
		}
		return rebarCluster;
	}
	//�������е�ˮƽ����Ľͨ���ཻ���ҵ���ֱ����ĸֽ�
 	std::vector<std::vector<int> > gatherVerticalRebar(std::vector<std::vector<int> > rebarClusterHorizontal)
	{
		std::vector<std::vector<int> > verticalCluster;
		auto& lineData = _spData->m_lineData.lines();
		//auto corners = _spData->m_cornerData.corners();
		for (auto& horizontalVec : rebarClusterHorizontal)
		{
			std::vector<int> verticalVec;
			std::set<int> verticalSet;
			for (int hIndex : horizontalVec)
			{
				Line& hLine = lineData[hIndex];
				std::vector<int> crosses = publicFunction::findAllCrossingLine(*_spData, hIndex);
				for (int crossIndex : crosses)
				{
					Line& crossLine = lineData[crossIndex];
					if (crossLine.getLayer() == _rebarLayer && crossLine.vertical())
					{
						//verticalVec.push_back(crossIndex);
						verticalSet.insert(crossIndex);
					}
				}
			}
			verticalVec.insert(verticalVec.end(), verticalSet.begin(), verticalSet.end());
			verticalCluster.push_back(verticalVec);
		}
		return verticalCluster;
	}
private:
	std::vector<std::pair<int, Point>> _rebarPairOrigin;
	std::vector<int>_rebarIndicesOrigin;
	std::string _rebarLayer;
private: //�����ṩ�ĸֽ�����
	std::map<int, std::string> _rebarMap;
	std::vector<std::vector<int> > _horizontalCluster;
	std::vector<std::vector<int> > _verticalCluster;
private://data
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _block;
private://��ͼ
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
public:
	void drawRebar()
	{
		std::vector<int> colorIndices;
		for (auto& pair : _rebarMap)
		{
			colorIndices.push_back(pair.first);
		}
		pColorLineIndexVec->push_back(colorIndices);
	}
};
//��߽����
class FloorOutline
{
public:
	class Shape;//����һ�� ���������ṹ����ʡȥ���
	//���ݾ����ĸֽ����ɰ��λ����Ϣ���������ܵ��ýӿ�
	void generateOutLineByRebar(std::shared_ptr<RebarParser> spRebarParser)
	{
		
		_spRectangleVec = generateRectangleByRebarClusters(spRebarParser->getHorizontalCluster(),spRebarParser->getVerticalCluster());
		extendOutline();
	}
	//����߽��
	void drawPolylinePoint()
	{
		for (auto data : floorDataVec)
		{
			pColorLineVec->push_back(data.polyline.getDrawingData());
		}
	}
	//���ݰ��box����ȥ������ȷ�İ���
	void extendOutline()
	{
		WallLocationSetAdapter wallAdapter;
		auto wallHIndices = wallAdapter.getHAndSLineIndices(_spData,_block);
		_wallLayer = wallAdapter.getWallLayer();
		for (auto& spRectangle : _spRectangleVec)
		{
			Box boardBox= spRectangle->getBox();
			std::vector<int> upAndDownIndices=findUpAndDownWallLineIndices(wallHIndices, boardBox);
			std::vector<Line> floorBorder=generateFloorBorder(upAndDownIndices, boardBox);
			MyPolyline polyline(floorBorder);
			polyline.transeToPoint();
			FloorData floorData;
			floorData.box = boardBox;
			floorData.border = floorBorder;
			floorData.polyline = polyline;
			floorDataVec.push_back(floorData);
			//pColorLineVec->push_back(polyline.getDrawingData());//��ӡ�߽��

		}
	}
	std::vector<FloorData> getFloorDataVec()
	{
		return floorDataVec;
	}
public:
	//����
	FloorOutline() = default;
	FloorOutline(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	FloorOutline(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//����data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//��ʼ����ͼ����
	void initDrawing(std::vector<std::vector<int> > * temp_index_colorBlocks,
		std::vector<std::vector<Line> > * temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	//��box����Χ�ڵ������桢�������ˮƽǽ�ߣ���Щ���ǰ�����±߽�
	std::vector<int> findUpAndDownWallLineIndices(std::vector<int> wallHindices, Box oneFloorbox)
	{
		
		auto corners = _spData->m_cornerData.corners();
		auto& lineData = _spData->m_lineData.lines();
		std::set<int> wallLineIndicesSet;
		double bottom = oneFloorbox.bottom - 6000;
		double top = oneFloorbox.top + 6000;
		for (double x = oneFloorbox.left+3500;x < oneFloorbox.right-3000;x += 4000)
		{
			Point bottomPoint(x, bottom);
			Point topPoint(x, top);
			Line hLine(bottomPoint, topPoint);
			int bottomWallLineIndex=-1;
			Corner bottomCorner;
			int topWallLineIndex=-1;
			Corner topCorner;
			for (int wallIndex : wallHindices)
			{
				Corner corner;
				Line wallLine = lineData[wallIndex];
				if (publicFunction::twoLineCrossLength(wallLine, oneFloorbox.getTopLine()) < 3500&&
					publicFunction::twoLineCrossLength(wallLine, oneFloorbox.getTopLine())/wallLine.length()<0.8
					)continue;
				bool crossMark=publicFunction::checkCrossingAndGetCorner(hLine, wallLine, corner);
				
				if (crossMark)
				{
					if (bottomWallLineIndex == -1||corner.y < bottomCorner.y&& corner.y < topCorner.y)
					{
						bottomWallLineIndex = wallIndex;
						bottomCorner = corner;
					}
					

					if (topWallLineIndex == -1||corner.y>topCorner.y&& corner.y > bottomCorner.y)
					{
						topWallLineIndex = wallIndex;
						topCorner = corner;
					}
				}
			}
			if(bottomWallLineIndex!=-1)
				wallLineIndicesSet.insert(bottomWallLineIndex);
			if(topWallLineIndex!=-1)
				wallLineIndicesSet.insert(topWallLineIndex);
		}
		std::vector<int> wallLineIndicesVec;
		wallLineIndicesVec.insert(wallLineIndicesVec.end(), wallLineIndicesSet.begin(), wallLineIndicesSet.end());
		return wallLineIndicesVec;
	}
	//�������°������� �����İ�߽�
	std::vector<Line> generateFloorBorder(std::vector<int> upAndDownIndices,Box boardBox)
	{
		std::set<int> upAndDownIndicesSet;
		std::vector<Line> border;
		upAndDownIndicesSet.insert(upAndDownIndices.begin(), upAndDownIndices.end());
		for (int index : upAndDownIndices)
		{
			dfsExtendLine(index, upAndDownIndicesSet);
		}
		std::vector<int> boardIndices;
		boardIndices.insert(boardIndices.end(), upAndDownIndicesSet.begin(), upAndDownIndicesSet.end());
		auto lonelyIndices=findLonelyIndices(upAndDownIndicesSet);
		std::vector<std::pair<int, int> > lonelyPair= generateLonelyPair(lonelyIndices);
		//for (auto pair : lonelyPair)//��ӡlonelypair
		//{
		//	std::vector<int> colorIndices;
		//	colorIndices.push_back(pair.first);
		//	colorIndices.push_back(pair.second);
		//	pColorLineIndexVec->push_back(colorIndices);
		//}
		std::vector<Line> someBorder= generateLineByLonelyPair(lonelyPair, boardBox);


		border=generateAllborder(boardIndices,lonelyIndices ,someBorder);
		
		return border;
	}
	//������±߽�Ҳ���������ߣ���Щ���ߵ��ص�����ͨ�˰��ߣ�����ͨ�������㷨ȥѰ����Щ�ߣ�
	//������ʱֻ����һ�Σ��д��Ľ��ɳ���������
	void dfsExtendLine(int index, std::set<int>& hasTranverse)
	{
		auto& lineData = _spData->m_lineData.lines();
		std::vector<int> crossIndices=publicFunction::findAllCrossingLine(*_spData, index);
		for (int crossIndex : crossIndices)
		{
			if (hasTranverse.count(crossIndex) != 0)
				continue;
			Line crossLine = lineData[crossIndex];
			if (crossLine.length() > 15000)continue;
			if (crossLine.getLayer() == _wallLayer)
			{
				std::vector<int> crossIndicess = publicFunction::findAllCrossingLine(*_spData, crossIndex);
				int countCross=0;
				for (auto indexs : crossIndicess)
				{
					if (hasTranverse.count(indexs) != 0)
						countCross++;

				}
				if (countCross >= 2)
				{
					hasTranverse.insert(crossIndex);
				}
			}
			
		}
	}
	//�Ӱ������ҵ��й¶��ߣ��¶���������һ�����ӻ����������ӵ��ߣ�Ҳ���ǰ��������ߵ�����ߵġ����ұߵ��ߣ�
	//�������ߵ�Ŀ�����ҵ���ֱ����İ���
	std::set<int> findLonelyIndices(std::set<int> boardIndices)
	{
		std::set<int> lonelyIndices;
		//auto corners = _spData->m_cornerData.corners();
		for (int index : boardIndices)
		{
			std::vector<int> crossIndices = publicFunction::findCrossing2(*_spData, index);
			int crossCount = 0;
			for (int crossIndex : crossIndices)
			{
				if (boardIndices.count(crossIndex) > 0)
					crossCount++;
			}
			if (crossCount < 2)
			{
				lonelyIndices.insert(index);
			}
		}
		return lonelyIndices;
	}
	//�¶��ߵ���ԣ���Ժ�Ĺ¶��߿�����Ѱ�Ұ�����ұ߽�
	std::vector<std::pair<int,int> >generateLonelyPair(std::set<int> lonelyIndices)
	{
		std::vector<int> indicesVec;
		indicesVec.insert(indicesVec.end(), lonelyIndices.begin(), lonelyIndices.end());
		//pColorLineIndexVec->push_back(indicesVec);
		std::vector<std::pair<int, int> > lonelyPair;
		std::set<int> hasDeal;
		for (int index : lonelyIndices)
		{
			if (hasDeal.count(index) != 0)continue;
			int matchIndex=publicFunction::findMatchLineIndex(*_spData, indicesVec, index);
			if (matchIndex != -1)
			{
				std::pair<int, int> pair = std::make_pair(index, matchIndex);
				lonelyPair.push_back(pair);
				hasDeal.insert(index);
				hasDeal.insert(matchIndex);
			}
		}
		return lonelyPair;
	}
	//ͨ���ɶԵĹ¶������������߶Σ��ֱ��ǹ¶��ߣ�������һ�����ӳ��������ӹ¶��ߵ���ֱ��߽�
	std::vector<Line> generateLineByLonelyPair(std::vector<std::pair<int, int> > lonelyPair,Box boardBox)
	{
		std::vector<Line> border;
		auto& lineData = _spData->m_lineData.lines();
		for (auto pair : lonelyPair)
		{
			Line line1 = lineData[pair.first];
			Line line2 = lineData[pair.second];
			Box box;
			box.expand(line1);
			box.expand(line2);
			border.push_back(box.getTopLine());
			border.push_back(box.getBottomLine());
			if (abs(line1.s.x - boardBox.left) < 1000||
				abs(line2.s.x - boardBox.left) < 1000)
			{
				Line leftBoardLine = box.getLeftLine();
				//leftBoardLine.s.x += 200;//����
				//leftBoardLine.e.x += 200;//����
				border.push_back(leftBoardLine);
			}
			if (abs(line1.e.x - boardBox.right) < 1000 ||
				abs(line2.e.x - boardBox.right) < 1000)
			{
				Line rightBoardLine = box.getRightLine();
				//rightBoardLine.s.x -= 200;//����
				//rightBoardLine.e.x -= 200;//����
				border.push_back(rightBoardLine);
			}
		}
		
		return border;
	}
	//�������е�
	std::vector<Line> generateAllborder(std::vector<int> boardIndices, std::set<int> lonelyIndices,
		std::vector<Line> someBorder)
	{
		std::vector<Line> border;
		auto& lineData = _spData->m_lineData.lines();
		border.insert(border.end(), someBorder.begin(), someBorder.end());
		for (int index : boardIndices)
		{
			if (lonelyIndices.count(index) == 0)
			{
				border.push_back(lineData[index]);
			}
		}
		return border;
	}
private://data
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _block;
public://shape
	class Shape
	{
	public:
		//virtual Json::Value getJson() = 0;
		virtual std::vector<Line> getBorderLine() = 0;
	};
	class Rectangle:public Shape
	{
	public:
		/*virtual Json::Value getJson()
		{
			Json::Value root;
			return root;
		}*/
		Box getBox()const
		{
			return this->box;
		}
	public:
		Rectangle() = default;
		Rectangle(Box& box)
		{
			this->box = box;
		}
		Rectangle(const std::vector<Line>& lineVec)
		{
			box.expand(lineVec);
		}
		Rectangle(const std::vector<Line>& horizontalLineVec,const std::vector<Line>& verticalLineVec)
		{
			box.expand(horizontalLineVec);
			box.expand(verticalLineVec);
		}
		void expand(const std::vector<Line>& lineVec)
		{
			box.expand(lineVec);
		}
		//��һ����������չbox
		void expand(const std::vector<Line>& horizontalLineVec, const std::vector<Line>& verticalLineVec)
		{
			box.expand(horizontalLineVec);
			box.expand(verticalLineVec);
		}
		virtual std::vector<Line> getBorderLine()
		{
			return this->box.getBorder();
		}
	private:
		Box box;
	}; 
private:
	std::vector<FloorData> floorDataVec;
	std::vector<std::shared_ptr<Rectangle> > _spRectangleVec;
	std::string _wallLayer;
private:
	//�������������е��ߣ�������ֱ��������box
	std::vector<std::shared_ptr<Rectangle> > generateRectangleByRebarClusters(std::vector<std::vector<int> > horizontalClusters,
		std::vector<std::vector<int> > verticalClusters)
	{
		std::vector<std::shared_ptr<Rectangle> > spRectangleVec;
		auto& lineData = _spData->m_lineData.lines();
		for (int index = 0;index < horizontalClusters.size();index++)
		{
			std::vector<int> horizontalVec = horizontalClusters[index];
			std::vector<int> verticalVec = verticalClusters[index];
			std::vector<Line> horizontalLineVec;
			std::vector<Line> verticalLineVec;
			for (int index : horizontalVec)
			{
				horizontalLineVec.push_back(lineData[index]);
			}
			for (int index : verticalVec)
			{
				verticalLineVec.push_back(lineData[index]);
			}
			std::shared_ptr<Rectangle> spRectangle = std::make_shared<Rectangle>(horizontalLineVec, verticalLineVec);
			spRectangleVec.push_back(spRectangle);
			/*std::vector<int> colorIndexVec;
			colorIndexVec.insert(colorIndexVec.end(), horizontalVec.begin(), horizontalVec.end());
			colorIndexVec.insert(colorIndexVec.end(), verticalVec.begin(), verticalVec.end());
			pColorLineIndexVec->push_back(colorIndexVec);*/
		}
		return spRectangleVec;
	}
	
private://��ͼ
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
};
//������ƽ��ͼ����
class FloorPlanParser
{
public:
	//һ���׵Ľ�������
	void parse()
	{
		parseRebar();
		parseFloorOutline();
		combine();
		printData();
	}
	void test_RebarParser()
	{
		RebarParser rebarParser(_spData, _block);
		rebarParser.initDrawing(pColorLineIndexVec, pColorLineVec);
		rebarParser.test_cleanRebar();
	}
	
public:
	//����
	FloorPlanParser() = default;
	FloorPlanParser(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	FloorPlanParser(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//����data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//��ʼ����ͼ����
	void initDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
private:
	//�����ֽ�
	void parseRebar()
	{
		std::shared_ptr<RebarParser> spRebarParser = std::make_shared<RebarParser>(_spData, _block);
		spRebarParser->initDrawing(pColorLineIndexVec, pColorLineVec);
		spRebarParser->parse();
		spRebarParser->gather();
		_spRebarParser = spRebarParser;
	}
	//������߽�
	void parseFloorOutline()
	{
		_spFloorOutLine=std::make_shared<FloorOutline>(_spData, _block);
		_spFloorOutLine->initDrawing(pColorLineIndexVec, pColorLineVec);
		_spFloorOutLine->generateOutLineByRebar(_spRebarParser);
	}
	//���������λ�������Լ���ֽ�����  �γ�һ�������İ�����
	void combine()
	{
		_floorDataVec=_spFloorOutLine->getFloorDataVec();
		_spRebarParser->fillInRebar(_floorDataVec);
	}
	void generateJsonData()
	{
		

	}
	Json::Value getJsonData()
	{
		ReferencePointGenerator ref(_block->m_axisHLines, _block->m_axisVLines);
		Json::Value result;
		for (auto& floor : _floorDataVec)
		{
			Json::Value oneFloor;
			auto polyline = floor.polyline;
			auto polyJson = polyline.getReferenceData(ref);
			oneFloor["Border"] = polyJson;
			for (auto hcRebar : floor.hcRebarVec)
			{
				oneFloor["HorizontalConnectRebar"].append(hcRebar.text);
			}
			for (auto vcRebar : floor.vcRebarVec)
			{
				oneFloor["VerticalConnectRebar"].append(vcRebar.text);
			}
			//oneFloor["HorizontalConnectRebar"]
			result.append(oneFloor);
		}
		return result;
	}
	void printData()
	{
		_spFloorOutLine->drawPolylinePoint();
		std::fstream cout("floor.json",std::ios::out);
		cout << getJsonData();
		cout.close();
	}
private://data
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _block;
private:
	std::shared_ptr<RebarParser> _spRebarParser;
	std::shared_ptr<FloorOutline> _spFloorOutLine;
	std::vector<FloorData> _floorDataVec;

private://��ͼ
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
};
 
#endif 