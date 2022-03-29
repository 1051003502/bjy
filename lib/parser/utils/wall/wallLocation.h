#ifndef __PARSER_UTILS_WALLLOCATION_H__
#define __PARSER_UTILS_WALLLOCATION_H__
#include "publicFunction.h"
#include "data.h"
#include "sectionSymbolSet.h"
#include "json/json.h"

//相对坐标点
class ReferencePoint
{
private:
	Point _p;
	Point _engineeringPoint;
	std::pair<std::string, double> _referenceHAxis;//H???,??????
	std::pair<std::string, double> _referenceVAxis;//V???,??????
public:
	ReferencePoint() = default;
	ReferencePoint(const std::pair<std::string, double>& referenceHAxis,
		const std::pair<std::string, double>& referenceVAxis,
		Point p = Point())
	{
		this->_referenceHAxis = referenceHAxis;
		this->_referenceVAxis = referenceVAxis;
		this->_p = p;
	}
	ReferencePoint& operator=(const ReferencePoint& refP)
	{
		if (this != &refP)
		{
			_p = refP._p;
			_engineeringPoint = refP._engineeringPoint;
			_referenceHAxis = refP._referenceHAxis;
			_referenceVAxis = refP._referenceVAxis;
		}
		return *this;
	}
	void setEngineeringPoint(const Point& p)
	{
		this->_engineeringPoint = p;
	}
	Point getEngineeringPoint()
	{
		return _engineeringPoint;
	}
	Json::Value getJsonData()
	{
		Json::Value tempPoint;
		Json::Value referenceHAxis;
		Json::Value referenceVAxis;
		referenceHAxis["distance"] = round(_referenceHAxis.second);
		referenceHAxis["name"] = _referenceHAxis.first;
		referenceVAxis["distance"] = round(_referenceVAxis.second);
		referenceVAxis["name"] = _referenceVAxis.first;
		tempPoint["referenceHAxis"] = referenceHAxis;
		tempPoint["referenceVAxis"] = referenceVAxis;
		return tempPoint;
	}

};
//相对坐标点生成器
class ReferencePointGenerator
{
private:
	std::vector<Axis::AxisLine> _axisHLines;
	std::vector<Axis::AxisLine> _axisVLines;
public:
	//构造时传入轴网数据
	ReferencePointGenerator(std::vector<Axis::AxisLine> axisHLines,
		std::vector<Axis::AxisLine> axisVLines)
	{
		this->_axisHLines = axisHLines;
		this->_axisVLines = axisVLines;
	}
	//检查轴网信息是否完全
	bool checkGenerator()
	{
		if (_axisHLines.empty())return false;
		if (_axisVLines.empty())return false;
		return true;
	}
	//得到工程坐标点
	Point genEngineeringPoint(const Point& p)
	{
		if (_axisHLines.empty() || _axisVLines.empty())return emptyPoint;
		double y0 = _axisHLines[0].second.s.y;
		double x0 = _axisVLines[0].second.s.x;
		return Point(p.x - x0, p.y - y0);
	}
	//得到相对坐标点
	ReferencePoint generateReferencePoint(const Point& p)
	{
		std::pair<std::string, double> hPair;
		std::pair<std::string, double> vPair;

		//??????  ??publicFunction?
		findClosestHAxis(hPair, _axisHLines, p.y);
		findClosestVAxis(vPair, _axisVLines, p.x);
		ReferencePoint refP(hPair, vPair, p);
		refP.setEngineeringPoint(genEngineeringPoint(p));
		return refP;
	}
};
class WallReferencePoint
{
public:
	WallReferencePoint();
	WallReferencePoint(const std::pair<std::string, double>& referenceHAxis, 
		const std::pair<std::string, double>& referenceVAxis);
	std::pair<std::string, double>getReferenceHAxis();
	std::pair<std::string, double>getReferenceVAxis();
private:
	Point _referencePoint;//???
	std::pair<std::string, double>_referenceHAxis;//???,??????
	std::pair<std::string, double>_referenceVAxis;//???,??????
};
class WallOutline   //????
{
public:
	enum class OutlineType
	{
		HRectangle,
		VRectangle,
		Slant
	};
private:
	double _width;
public:
	WallOutline() { setWidth(-1); }
	void setWidth(double width)
	{
		_width = round( width);
	}
	double getWidth()
	{
		return _width;
	}
public:
	//????Json??
	virtual void generateReferenceData(ReferencePointGenerator& refG) = 0;
	virtual Json::Value getOutlineJsonData() = 0;
	//?????(?????)
	virtual std::vector<Line> getDrawingLineVec() = 0;
	virtual void calculateWidth() = 0;
	virtual OutlineType getOutlineType() = 0;
	virtual Line getOneMainLine() = 0;
	virtual Json::Value getNearestAxia() = 0;
};
class RectangleOutline :public WallOutline
{
public:
	Box box;
	Point p1;
	Point p2;
	ReferencePoint refP1;
	ReferencePoint refP2;
public:
	RectangleOutline() = default;
	RectangleOutline(Line& l1, Line& l2)
	{
		generateBox(l1, l2);
		
	}
public:
	Box& getBox() { return box; }
public:
	void generateBox(Line& l1, Line& l2)
	{
		box.expand(l1.s);
		box.expand(l1.e);
		box.expand(l2.s);
		box.expand(l2.e);
	}
	virtual void generateLocationPoint() = 0;
	virtual void generateReferenceData(ReferencePointGenerator& refG)
	{
		//ReferencePointGenerator refG(axisHLines, axisVLines);
		refP1 = refG.generateReferencePoint(p1);
	    refP2 = refG.generateReferencePoint(p2);
	}
	virtual Json::Value getOutlineJsonData()
	{
		Json::Value res;
		res["End1"] = refP1.getJsonData();
		res["End2"] = refP2.getJsonData();
		res["Width"] = getWidth();
		res["End1Engineering"] = refP1.getEngineeringPoint().getXYText();
		res["End2Engineering"] = refP2.getEngineeringPoint().getXYText();
		return res;
	}
	virtual std::vector<Line> getDrawingLineVec()
	{
		return publicFunction::generateBoxBorder(box);
	}
	
	
};
class HRectangleOutline :public RectangleOutline
{
public:
	HRectangleOutline() = default;
	HRectangleOutline(Line& l1, Line& l2) :RectangleOutline(l1, l2)
	{
		generateLocationPoint();
	
	}
	virtual Line getOneMainLine()
	{
		Point leftTop(box.left, box.top);
		Point rightTop(box.right, box.top);
		Line lineUp(leftTop, rightTop);
		return lineUp;
	}
	virtual Json::Value getNearestAxia()
	{
		Json::Value end1 = refP1.getJsonData();
		return end1["referenceHAxis"]["name"];
	}
private:
	virtual void generateLocationPoint()
	{
		p1.x = box.left;
		p1.y = (box.top + box.bottom) / 2;
		p2.x = box.right;
		p2.y = (box.top + box.bottom) / 2;
	}
	virtual void calculateWidth()
	{
		setWidth(box.top - box.bottom);
	}
	/*virtual Json::Value getOutlineJsonData()
	{
		Json::Value res;
		res["leftMiddle"] = refP1.getJsonData();
		res["rightMiddle"] = refP2.getJsonData();
		res["width"]=getWidth();
		return res;
	}*/
	virtual OutlineType getOutlineType()
	{
		return OutlineType::HRectangle;
	}
};
class VRectangleOutline :public RectangleOutline
{
public:
	VRectangleOutline() = default;
	VRectangleOutline(Line & l1, Line& l2) :RectangleOutline(l1, l2)
	{
		generateLocationPoint();
		
	}
	virtual Line getOneMainLine()
	{
		Point leftUp(box.left, box.top);
		Point leftDown(box.left, box.bottom);
		Line lineLeft(leftUp, leftDown);
		return lineLeft;
	}
	virtual Json::Value getNearestAxia()
	{
		Json::Value end1 = refP1.getJsonData();
		return end1["referenceVAxis"]["name"];
	}
private:
	virtual void generateLocationPoint()
	{
		p1.x = (box.left + box.right) / 2;
		p1.y = box.top;
		p2.x = (box.left + box.right) / 2;
		p2.y = box.bottom;
	}
	virtual void calculateWidth()
	{
		setWidth(box.right - box.left);
	}
	/*virtual Json::Value getOutlineJsonData()
	{
		Json::Value res;
		res["upMiddle"] = refP1.getJsonData();
		res["downMiddle"] = refP2.getJsonData();
		res["width"] = getWidth();
		return res;
	}*/
	virtual OutlineType getOutlineType()
	{
		return OutlineType::VRectangle;
	}
};
class SlantOutline:public WallOutline
{
private:
	std::vector<Line> _borderLineVec;
	Point p1;
	Point p2;
	ReferencePoint refP1;
	ReferencePoint refP2;
public:
	SlantOutline() = default;
	SlantOutline(Line& l1, Line& l2)
	{
		_borderLineVec.push_back(l1);
		_borderLineVec.push_back(l2);
		p1 = Point((l1.s.x + l2.s.x) / 2, (l1.s.y + l2.s.y) / 2);
		p2 = Point((l1.e.x + l2.e.x) / 2, (l1.e.y + l2.e.y) / 2);
	}
	virtual void generateReferenceData(ReferencePointGenerator& refG) 
	{
		refP1 = refG.generateReferencePoint(p1);
		refP2 = refG.generateReferencePoint(p2);
	}
	virtual Json::Value getOutlineJsonData() 
	{
		Json::Value res;
		res["End1"] = refP1.getJsonData();
		res["End2"] = refP2.getJsonData();
		res["Width"] = getWidth();
		res["End1Engineering"] = refP1.getEngineeringPoint().getXYText();
		res["End2Engineering"] = refP2.getEngineeringPoint().getXYText();
		return res;
	}
	//?????(?????)
	virtual std::vector<Line> getDrawingLineVec() 
	{
		std::vector<Line> res;
		//res .push_back( _borderLineVec[0]);
		res = _borderLineVec;
		return res;
	}
	virtual void calculateWidth()
	{
		double dis = slantingDistanceLine(_borderLineVec[0], _borderLineVec[1]);
		setWidth(dis);
	}
	virtual OutlineType getOutlineType()
	{
		return OutlineType::Slant;
	}
	virtual Line getOneMainLine()
	{
		if (_borderLineVec.empty())
			return Line();
		else
			return _borderLineVec[0];
	}
	virtual Json::Value getNearestAxia()
	{
		Json::Value end1 = refP1.getJsonData();
		return end1["referenceHAxis"]["name"];
	}
};
class WallBorderLine
{
public:
	WallBorderLine() {}
	bool hLines(const std::vector<Line>hlines);
	bool vLines(const std::vector<Line>vlines);
	bool sLines(const std::vector<Line>slines);
	std::vector<Line>getHLines();
	std::vector<Line>getVLines();
	std::vector<Line>getSLines();
	
private:
	/*???*/
	std::vector<Line>_hLines;
	std::vector<Line>_vLines;
	std::vector<Line>_sLines;
	
};

class WallLocation
{
public:
	enum  class WALLTYPE
	{
		UP,
		DOWN,
		LEFTENDPROFILE,
		RIGHTENDPROFILE,
		OBLIQUE,
		OTHER
	};
	WallLocation() 
	{
		this->_thick = 0.0;
		_relativeElevation = 0;
		wallType = WALLTYPE::OTHER;
	}
	//??????????
	WallLocation(Line& l1, Line& l2)
	{
		_spWallOutline = returnWallOutline(l1, l2);
		_spWallOutline->calculateWidth();//计算墙宽度
	}
	void generateReferenceData(ReferencePointGenerator& refG)
	{
		_spWallOutline->generateReferenceData(refG);
	}
	bool operator<(const WallLocation& wallLocation)const
	{
		return this->_spWallOutline->getOutlineJsonData() < wallLocation._spWallOutline->getOutlineJsonData();
	}
public:
	//得到描述墙的形状类 （简单工厂）
	std::shared_ptr<WallOutline> returnWallOutline(Line& l1, Line& l2)
	{
		if (l1.horizontal() && l2.horizontal())
		{
			return std::make_shared<HRectangleOutline>(l1, l2);
		}
		else if (l1.vertical() && l2.vertical())
		{
			return std::make_shared<VRectangleOutline>(l1, l2);
		}
		else
		{
			return std::make_shared<SlantOutline>(l1, l2);
		}
	}
public:
	//interface function

	bool setName(const std::string& name);
	bool setRelativeElevation(const double& re);
	bool setThick(const double& thick);
	bool setReferencePoint(const std::vector<std::pair<std::string, double>>& startPoint,
		const std::vector<std::pair<std::string, double>>& endPoint);
	bool setBorders(const std::vector<Line>& lines);
	std::string getName();
	double getRelativeElevation();
	double getThick();
	std::string getWallType()
	{
		std::string res;
		switch (wallType)
		{
		case WALLTYPE::UP:
			res = "UP";
			break;
		case WALLTYPE::DOWN:
			res = "DOWN";
			break;
		case WALLTYPE::LEFTENDPROFILE:
			res = "LEFTENDPROFILE";
			break;
		case WALLTYPE::RIGHTENDPROFILE:
			res = "RIGHTENDPROFILE";
			break;
		case WALLTYPE::OBLIQUE:
			res = "OBLIQUE";
			break;
		default:
			res = "OTHER";
		}
		return res;
	}
	WallReferencePoint getReferenceStartPoint();
	WallReferencePoint getReferenceEndPoint();
	std::vector<Line>getBorder();//??????
	WallBorderLine getWallBorderLine();
	//????? ???_wallborderLine??
	void setWallType(Box& box)
	{
		std::shared_ptr<RectangleOutline> spRecOutline;
		double hMiddle = (box.top + box.bottom) / 2;
		double vMiddle = (box.left + box.right) / 2;
		WallOutline::OutlineType outlineType = _spWallOutline->getOutlineType();
		if (outlineType == WallOutline::OutlineType::Slant)
		{
			this->wallType = WALLTYPE::OBLIQUE;
		}
		else if (outlineType == WallOutline::OutlineType::HRectangle)
		{
			spRecOutline = std::dynamic_pointer_cast<RectangleOutline>( _spWallOutline);
				Box& wallBox=spRecOutline->getBox();
				if (wallBox.bottom > hMiddle)
				{
					this->wallType = WALLTYPE::UP;
				}
				else if (wallBox.top < hMiddle)
				{
					this->wallType = WALLTYPE::DOWN;
				}
				else
				{
					this->wallType = WALLTYPE::OTHER;
				}
		}
		else if (outlineType == WallOutline::OutlineType::VRectangle)
		{
			spRecOutline = std::dynamic_pointer_cast<RectangleOutline>(_spWallOutline);
			Box& wallBox = spRecOutline->getBox();
			if (wallBox.left > vMiddle)
			{
				this->wallType = WALLTYPE::RIGHTENDPROFILE;
			}
			else if (wallBox.right < hMiddle)
			{
				this->wallType = WALLTYPE::LEFTENDPROFILE;
			}
			else
			{
				this->wallType = WALLTYPE::OTHER;
			}
		}
		else
		{
			this->wallType = WALLTYPE::OTHER;
		}
	}
	void setWallSectionSymbol(std::shared_ptr<Data> spData, std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec)
	{
		SectionSymbolSet sss;
		sss.setAnalysisDependencise(spData);
		Line mainLine = this->_spWallOutline->getOneMainLine();
		int sgIndex = sss.getSectionSymbolGroup(mainLine, ssgVec);
		if (sgIndex != -1)
		{
			auto ssg = ssgVec[sgIndex];
			auto ssVec = ssg->sectionSymbolVec;//ssgVec[sgIndex]->sectionSymbolVec;
			this->sectionSymbolVec = ssVec;//ssgVec[sgIndex]->sectionSymbolVec;
			std::string symbolText= this->sectionSymbolVec[0]->symbolTextInfo.text;
			if(symbolText.length()<5)
			this->sectionSymbolText = this->sectionSymbolVec[0]->symbolTextInfo.text;
			
		}
		
		//if (wallType == WALLTYPE::UP || wallType == WALLTYPE::DOWN)
		//{
		//	auto wallLineVec = this->getWallBorderLine().getHLines();

		//	if (wallLineVec.size() != 0)
		//	{
		//		int sgIndex = sss.getSectionSymbolGroup(wallLineVec[0], ssgVec);
		//		if (sgIndex != -1)
		//		{
		//			auto ssg = ssgVec[sgIndex];
		//			auto ssVec = ssg->sectionSymbolVec;//ssgVec[sgIndex]->sectionSymbolVec;
		//			this->sectionSymbolVec = ssVec;//ssgVec[sgIndex]->sectionSymbolVec;
		//			this->sectionSymbolText = this->sectionSymbolVec[0]->symbolTextInfo.text;
		//		}
		//	}
		//}
		////???
		//if (wallType == WALLTYPE::LEFTENDPROFILE || wallType == WALLTYPE::RIGHTENDPROFILE)
		//{
		//	auto wallLineVec = this->getWallBorderLine().getVLines();

		//	if (wallLineVec.size() != 0)
		//	{
		//		int sgIndex = sss.getSectionSymbolGroup(wallLineVec[0], ssgVec);
		//		if (sgIndex != -1)
		//		{
		//			auto ssg = ssgVec[sgIndex];
		//			auto ssVec = ssg->sectionSymbolVec;
		//			this->sectionSymbolVec = ssVec;
		//			this->sectionSymbolText = this->sectionSymbolVec[0]->symbolTextInfo.text;
		//		}
		//	}
		//}
		//else
		//{
		//	//????
		//}
		
	}
	//得到墙Json数据
	Json::Value getWallLocationJsonData()
	{

		Json::Value wallLoc;
		Json::Value wallOutLine= _spWallOutline->getOutlineJsonData();
		Json::Value End1EngineeringJson = wallOutLine["End1Engineering"];
		Json::Value End2EngineeringJson = wallOutLine["End2Engineering"];
		std::string End1Engineering = End1EngineeringJson.asString();
		std::string End2Engineering = End2EngineeringJson.asString();
		wallLoc["IDCode"] = "Wall-(" +End1Engineering+End2Engineering+")";
		wallLoc["End1"] = wallOutLine["End1"];
		wallLoc["End2"] = wallOutLine["End2"];
		wallLoc["Width"] = wallOutLine["Width"];
		//wallLoc["wallType"] = this->getWallType();
		wallLoc["ReferenceProfile"] = this->sectionSymbolText + "-" + this->sectionSymbolText;
		wallLoc["NearestAxia"] = _spWallOutline->getNearestAxia();
		return wallLoc;
	}
	std::vector<Line> getDrawingLineVec()
	{
		return _spWallOutline->getDrawingLineVec();
	}
private:
	std::string _name;//名字
	double _relativeElevation;//??
	double _thick;//厚度
	WallReferencePoint _startPoint;//起始点
	WallReferencePoint _endPoint;//终止点
	std::string _floorLayer;//板图层
	WallBorderLine _wallBorderLine;//墙边界（原来的表达方法）
	std::shared_ptr<WallOutline> _spWallOutline;//墙形状数据
	WALLTYPE wallType;//墙类型
	std::vector<std::shared_ptr<SectionSymbol>> sectionSymbolVec; //剖切符数据
public:
	std::string sectionSymbolText; //被哪个所剖切
};

#endif