#ifndef __PARSER_UTILS_FLOORPLAN_H__
#define __PARSER_UTILS_FLOORPLAN_H__
#include "publicFunction.h"
#include "data.h"
#include "block.h"
#include "json/json.h"
#include "wallLocationSet.h"

//解决多线段的相关问题
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
	//简介：将多条线段转化成顺时针的一串点
	//前置数据：需要通过构造函数传入lineVec  或者通过setLine传入lineVec
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
	//简介:返回需要画出的内容，每个点用两段正交的线段表示，交点是这个点，前置函数是transeToPoint
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
	//简介：判断多线段是否是闭合的，后面用来检验数据，可以根据transeToPoint函数的思路来实现
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
	//简介：将一个ReferencePointGenerator传进来，就可以得到需要输出的边界Json数据
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
//板钢筋的一个基类，是一个数据类
class BoardRebar
{
public:
	std::string text;//钢筋标注
	Line line;       //钢筋线
	int lineIndex;   //钢筋线索引
	BoardRebar() = default;
	BoardRebar(std::string text = "", Line line = Line(),int lineIndex = -1)
	{
		this->text = text;
		this->line = line;
		this->lineIndex = lineIndex;
	}
};
//板水平通长筋（水平贯通筋），继承了板钢筋基类，针对其特点可以增加独有的数据，方便扩展
class BoardHConnectRebar:public BoardRebar
{
public:
	BoardHConnectRebar(std::string text = "", Line line = Line(), int lineIndex = -1) :BoardRebar(text, line, lineIndex)
	{

	}
};
//板竖直通长筋（竖直贯通筋），继承了板钢筋基类，针对其特点可以增加独有的数据，方便扩展
class BoardVConnectRebar:public BoardRebar
{
public:
	BoardVConnectRebar(std::string text = "", Line line = Line(), int lineIndex = -1) :BoardRebar(text, line, lineIndex)
	{

	}
};
//存放板数据
class FloorData
{
public:
	Box box;//板的一个大致轮廓  可用其来板内钢筋
	std::vector<Line> border; //板的边界线  之所以保存下来是因为之后可能用于校验板边界  增加鲁棒性
	MyPolyline polyline;      //边界线的进一步解析，主要用来校对，以及输出Json数据
	std::vector< BoardHConnectRebar > hcRebarVec;//水平通长筋
	std::vector< BoardVConnectRebar > vcRebarVec;//数值通长筋
public:
	
};
//钢筋解析者，负责从板平面配筋图初步解析钢筋，以及二度重组钢筋
class RebarParser
{
public:
	//关于primaryParseRebar的函数测试
	void test_parseRebar()
	{
		primaryParseRebar();
		pColorLineIndexVec->push_back(_rebarIndicesOrigin);
	}
	//关于test_cleanRebar的函数测试
	void test_cleanRebar()
	{
		primaryParseRebar();
		findRebarLayer();
		cleanRebar();
		drawRebar();
	}
	//整个的解析过程，也是对外提供的主要接口，外部将基础数据传入后，就调用此函数
	void parse()
	{
		primaryParseRebar();
		findRebarLayer();
		cleanRebar();
		//drawRebar();
	}
	//对钢筋进行聚类，一个板的钢筋聚合到一个vector中，最后生成了多个vector
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
	//在有板的基本结构后进一步地增加板的钢筋数据，时间顺序是   初步解析钢筋->生成板->填充钢筋信息
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
	//构造
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
	//设置data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//初始化画图构件
	void initDrawing(std::vector<std::vector<int> > * temp_index_colorBlocks,
		std::vector<std::vector<Line> > * temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
private:
	//检查一个字符串是否是一个钢筋标注，这里可能存在无法识别钢筋的问题，所以需要后续完善这个函数
	bool checkRebarText(std::string text)
	{
		if (text.find("@") == std::string::npos)return false;
		return true;
	}
	//初步解析出钢筋，遍历所有文本点，然后再根据findPointBaseLine函数找到钢筋文本点所对应到的钢筋线条
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
	//通过初步解析出来的钢筋线提取钢筋图层
	void findRebarLayer()
	{
		_rebarLayer = publicFunction::findMaxAmountLayer(_rebarIndicesOrigin, *_spData);
	}
	//在这里进行钢筋的数据清洗，尽可能地剔除掉不是钢筋的线，以及重复的钢筋线
	void cleanRebar()
	{
		//剔除_rebarOrigin中不是钢筋图层的线
		//XXXXXX
		//XXXXXX

		auto& lineData = _spData->m_lineData.lines();
		//合并一根线被多个文本标注的情况
		for (auto& rebarPair : _rebarPairOrigin)
		{
			int index = rebarPair.first;
			if (lineData[index].length() < 100)continue;//太短过滤
			if (lineData[index].getLayer() != _rebarLayer)continue;//不是钢筋图层过滤
			Point& p = rebarPair.second;
			_rebarMap[index] = p.getText();
		}
	}

private:
	//从钢筋数据中得到水平钢筋
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
	//将一根钢筋插入到rebarcClusterh中，这里总体存在2种情况
	//1rebarCluster中已经有了与新插入的钢筋在同一个板的钢筋
	//2rebarCluster的钢筋和新插入的钢筋都不在同一个板中
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
	//水平方向的筋聚集成集合
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
	//利用已有的水平方向的筋，通过相交性找到竖直方向的钢筋
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
private: //对外提供的钢筋数据
	std::map<int, std::string> _rebarMap;
	std::vector<std::vector<int> > _horizontalCluster;
	std::vector<std::vector<int> > _verticalCluster;
private://data
	std::shared_ptr<Data> _spData;
	std::shared_ptr<Block> _block;
private://画图
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
//板边界解析
class FloorOutline
{
public:
	class Shape;//声明一下 后续调整结构可以省去这个
	//根据聚类后的钢筋生成板的位置信息，这里是总调用接口
	void generateOutLineByRebar(std::shared_ptr<RebarParser> spRebarParser)
	{
		
		_spRectangleVec = generateRectangleByRebarClusters(spRebarParser->getHorizontalCluster(),spRebarParser->getVerticalCluster());
		extendOutline();
	}
	//画板边界点
	void drawPolylinePoint()
	{
		for (auto data : floorDataVec)
		{
			pColorLineVec->push_back(data.polyline.getDrawingData());
		}
	}
	//根据板的box轮廓去搜索精确的板线
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
			//pColorLineVec->push_back(polyline.getDrawingData());//打印边界点

		}
	}
	std::vector<FloorData> getFloorDataVec()
	{
		return floorDataVec;
	}
public:
	//构造
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
	//设置data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//初始化画图构件
	void initDrawing(std::vector<std::vector<int> > * temp_index_colorBlocks,
		std::vector<std::vector<Line> > * temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	//找box横向范围内的最上面、最下面的水平墙线，这些线是板的上下边界
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
	//根据上下板线生成 完整的板边界
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
		//for (auto pair : lonelyPair)//打印lonelypair
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
	//板的上下边界也可能有竖线，这些竖线的特点是连通了板线，所以通过搜索算法去寻找这些线，
	//这里暂时只搜索一次，有待改进成持续搜索型
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
	//从板线中找到有孤儿线，孤儿线是少了一个连接或者两个连接的线，也就是板线上面线的最左边的、最右边的线，
	//找这种线的目的是找到竖直方向的板线
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
	//孤儿线的配对，配对后的孤儿线可用于寻找板的左右边界
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
	//通过成对的孤儿线生成三条线段，分别是孤儿线（可能有一定的延长）、连接孤儿线的竖直板边界
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
				//leftBoardLine.s.x += 200;//调试
				//leftBoardLine.e.x += 200;//调试
				border.push_back(leftBoardLine);
			}
			if (abs(line1.e.x - boardBox.right) < 1000 ||
				abs(line2.e.x - boardBox.right) < 1000)
			{
				Line rightBoardLine = box.getRightLine();
				//rightBoardLine.s.x -= 200;//调试
				//rightBoardLine.e.x -= 200;//调试
				border.push_back(rightBoardLine);
			}
		}
		
		return border;
	}
	//生成所有的
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
		//用一堆线条来扩展box
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
	//遍历各个集合中的线，利用竖直交线扩充box
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
	
private://画图
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
};
//解析板平面图数据
class FloorPlanParser
{
public:
	//一整套的解析流程
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
	//构造
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
	//设置data
	void setSpData(std::shared_ptr<Data> spData)
	{
		this->_spData = spData;
	}
	void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> block)
	{
		this->_spData = spData;
		this->_block = block;
	}
	//初始化画图构件
	void initDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks)
	{
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
private:
	//解析钢筋
	void parseRebar()
	{
		std::shared_ptr<RebarParser> spRebarParser = std::make_shared<RebarParser>(_spData, _block);
		spRebarParser->initDrawing(pColorLineIndexVec, pColorLineVec);
		spRebarParser->parse();
		spRebarParser->gather();
		_spRebarParser = spRebarParser;
	}
	//解析板边界
	void parseFloorOutline()
	{
		_spFloorOutLine=std::make_shared<FloorOutline>(_spData, _block);
		_spFloorOutLine->initDrawing(pColorLineIndexVec, pColorLineVec);
		_spFloorOutLine->generateOutLineByRebar(_spRebarParser);
	}
	//组合起来板位置数据以及板钢筋数据  形成一个完整的板数据
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

private://画图
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
};
 
#endif 