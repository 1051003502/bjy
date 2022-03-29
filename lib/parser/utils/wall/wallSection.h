#ifndef __PARSER_UTILS_WALLSECTION_H__
#define __PARSER_UTILS_WALLSECTION_H__
#include "data.h"
#include "block.h"
#include "mainStructureSection.h"
class Rebar
{
public:
	enum class ExtendingType
	{
		BottomBoard,//从板的底部开始
		TopBoard,   //从板的顶部开始
		RebarBending,//从钢筋的弯折处
		WallMargin,   //从墙的边缘处
		none   //无
	};
	enum class SideType
	{
		Unknow,
		Inside,
		Outside,
	};
public:
	Rebar() = default;
	Rebar(int index, Line& line, std::string rebarInfo) {
		this->index = index;
		this->line = line;
		this->rebarInfo = rebarInfo;
		this->extendingLength = 0;
		this->extendingType = ExtendingType::none;
	};
	Rebar(const Rebar& rebar)
	{
		this->index = rebar.index;
		this->line = rebar.line;
		this->rebarInfo = rebar.rebarInfo;
		this->extendingType = rebar.extendingType;
		this->extendingLength = rebar.extendingLength;
	}
	int getIndex() { return index; }
	std::string getRebarInfo() { return rebarInfo; }
	Line& getLine() { return line; }
public://get set函数
	void setExtendingType(ExtendingType type) { this->extendingType = type; }
	ExtendingType getExtendingType() { return extendingType; }
	void setExtendingLength(double length) { this->extendingLength = length; }
	double getExtendingLength() { return this->extendingLength; }
	void setSideType(SideType sideType) { this->sideType = sideType; }
	SideType getSideType() { return sideType; }
	std::string getSideTypeStr()
	{
		if (sideType == SideType::Inside)return "Inside";
		if (sideType == SideType::Outside)return "Outside";
		return "UnknowSide";
	}
private:
	int index;//钢筋索引
	std::string rebarInfo;//钢筋描述文本
	Line line;
	SideType sideType;
private:
	double extendingLength;
	ExtendingType extendingType;
};
class hNConnectRebar :public Rebar
{
public:
	hNConnectRebar() { ; }
	hNConnectRebar(int index, Line& newL, std::string pointStr) :Rebar(index, newL, pointStr) { ; }
};
class hConnectRebar :public Rebar
{

public:
	hConnectRebar() { ; }
	hConnectRebar(int index, Line& newL, std::string pointStr) :Rebar(index, newL, pointStr) { ; }

};
class vNConnectRebar :public Rebar
{
public:
	vNConnectRebar() { ; }
	vNConnectRebar(int index, Line& newL, std::string pointStr) :Rebar(index, newL, pointStr) { ; }
};
class vConnectRebar :public Rebar
{

public:
	vConnectRebar() { ; }
	vConnectRebar(int index, Line& newL, std::string pointStr) :Rebar(index, newL, pointStr) { ; }
	vConnectRebar(const vConnectRebar& rebar):Rebar(rebar)
	{
		
	}

};
class Haunch
{
public:
	Haunch();
	Haunch(const std::string& info,
		const double& width,
		const double& height,
		const double& insideWall,
		const double& insideBoard);
	//Haunch(const Haunch& haunch);
	Haunch(int stirIndex, std::string text,Line& line) {
		_rebarInfo = text;
		this->index = stirIndex;
		this->line = line;
	}
	int getIndex() { return index; }
	std::string getRebarInfo() { return _rebarInfo; }
private:
	std::string _rebarInfo;//加腋筋的描述
	int index;
	Line line;
	double _haunchWidth;//腋宽
	double _hunchHeight;//腋高
	double _insideWallLength;//深入墙的长度
	double _insideBoardLength;//深入板的长度
};
//描述钢筋


//体现墙的具体信息
class WallSection
{
public:
	WallSection()
	{
		_height = 0;
		_thick = 0;
	}
	WallSection(Point& p1, Point& p2) {
		_height = 0;
		_thick = 0;
		box.iniBox(p1, p2);
	}
	void findNearestAxis(std::vector<Axis::AxisLine> axisVLines)
	{
		double centerX = box.getCenterX();
		std::pair<std::string, double> refCenterX;
		findClosestVAxis(refCenterX,axisVLines, centerX);
		nearestAxisName = refCenterX.first;
	}
public://reference 
	std::string getProfileName()
	{
		return profileName;
	}
	void setProfileName(std::string profileName)
	{
		this->profileName = profileName;
	}
	bool setAttitude(double attitude) {
		_attitude = attitude;
		return false;
	}
	double getAttitude() { return _attitude; }
	double getTopLevel()
	{
		double topLevel = box.top - _attitude;
		int topLevelInt = publicFunction::roundToInt(topLevel);
		return  topLevelInt;
	}
	double getBottomLevel()
	{
		double bottomLevel = box.bottom - _attitude;
		int bottomLevelInt = publicFunction::roundToInt(bottomLevel);
		return  bottomLevelInt;
	}
	double getWidth()
	{
		return publicFunction::roundToInt(box.width());
	}
	std::string getNearestAxisName()
	{
		return nearestAxisName;
	}
	Box& getBox() { return box; }
	std::vector<Haunch>& getHunchVec() { return _haunchVec; }
	std::vector<vConnectRebar>& getVConnectReabarVec() { return _vConnectRebarVec; }
	bool addHConnectRebar(StirBreakPoint& sp) {
		_hConnectRebarVec.push_back(sp);
		return false;
	}
	bool addVConnectReabar(int index,Line& newL,std::string stirText) {
		_vConnectRebarVec.push_back(vConnectRebar(index, newL, stirText));
		return false;
	}
	bool addVConnectReabar(vConnectRebar rebar) {
		_vConnectRebarVec.push_back(rebar);
		return false;
	}
	bool addHaunch(Haunch h) {
		this->_haunchVec.push_back(h);
		return false;
	}
	bool addVNConnectRebar(int stirLineindex, Line& stirLine, std::string stirText) {
		_vNConnectRebarVec.push_back(vNConnectRebar(stirLineindex, stirLine, stirText));
		return false;
	}
	void generateRebarSide()
	{
		if (this->getNearestAxisName() == "A")
		{
			for (auto& vCRebar : _vConnectRebarVec)
			{
				Line rebarLine = vCRebar.getLine();
				if (rebarLine.s.x > this->box.getCenterX())
				{
					vCRebar.setSideType(Rebar::SideType::Inside);
				}
				else
				{
					vCRebar.setSideType(Rebar::SideType::Outside);
				}
			}

		}
		if (this->getNearestAxisName() == "D")
		{
			for (auto& vCRebar : _vConnectRebarVec)
			{
				Line rebarLine = vCRebar.getLine();
				if (rebarLine.s.x > this->box.getCenterX())
				{
					vCRebar.setSideType(Rebar::SideType::Outside);
				}
				else
				{
					vCRebar.setSideType(Rebar::SideType::Inside);
				}
			}
		}
	}
	Json::Value getWallSectionJsonData()
	{
		Json::Value wallSectionJsonData;
		//wallSectionJsonData["attitude"] = this->getAttitude();
		wallSectionJsonData["TopLevel"] = this->getTopLevel();
		wallSectionJsonData["BtmLevel"] = this->getBottomLevel();
		wallSectionJsonData["Width"] = this->getWidth();
		wallSectionJsonData["NearestAxis"] = this->getNearestAxisName();
		wallSectionJsonData["ProfileName"] = this->getProfileName();
		wallSectionJsonData["location"]["x1"] = box.left;
		wallSectionJsonData["location"]["y1"] = box.top;
		wallSectionJsonData["location"]["x2"] = box.right;
		wallSectionJsonData["location"]["y2"] = box.bottom;
		

		wallSectionJsonData["horizontalRebar"] = "";
		for (auto& bp : _hConnectRebarVec)
		{
			if (bp.textPoint.entity != NULL)
			{
				wallSectionJsonData["horizontalRebar"] = bp.textPoint.getText();
				break;
			}
		}
		

		//竖直非贯通
		Json::Value vNConnectReabarJsonData;
		for (auto& nConnectRebar :  _vNConnectRebarVec)
		{
			vNConnectReabarJsonData.append(nConnectRebar.getRebarInfo());
		}
		wallSectionJsonData["verticalNotConnectReabar"] = vNConnectReabarJsonData;

		//竖直贯通
		Json::Value vConnectReabarJsonDataList;
		for (auto& connectRebar : _vConnectRebarVec)
		{
			Json::Value vConnectReabarJsonData;
			vConnectReabarJsonData["rebarText"] = connectRebar.getRebarInfo();
			vConnectReabarJsonData["Side"] = connectRebar.getSideTypeStr();
			if (connectRebar.getExtendingType() == Rebar::ExtendingType::WallMargin)
			{
				vConnectReabarJsonData["ExtendingType"] = "WallMargin";
				vConnectReabarJsonData["ExtendingLength"] = connectRebar.getExtendingLength();
			}
			vConnectReabarJsonDataList.append(vConnectReabarJsonData);
		}
		wallSectionJsonData["verticalConnectRebar"] = vConnectReabarJsonDataList;
		
		
		//加腋筋
		Json::Value haunchJsonData;
		for (auto& haunch : _haunchVec)
		{
			haunchJsonData.append(haunch.getRebarInfo());
		}
		wallSectionJsonData["haunch"] = haunchJsonData;
		return wallSectionJsonData;
	}

private:
	std::string _name;
	double  _height;//标高
	double _thick;//厚度
	double _attitude;//绝对标高
	Box box;//墙组成的box
	std::vector<StirBreakPoint> _hConnectRebarVec;//水平筋
	std::vector<vNConnectRebar> _vNConnectRebarVec;//竖直非贯通筋
	std::vector< vConnectRebar> _vConnectRebarVec;//竖直贯通筋
	std::vector<Haunch>_haunchVec;//加腋
	//std::pair<std::string, double>_startPointReferenceAxis;//起点的参考轴 名字 距离
	//std::pair<std::string, double>_endPointReferenceAxis;//终点的参考轴 名字 距离
	std::string nearestAxisName;
	std::string profileName;
};
class EndProfileSection
{
public:
	
	class Hole
	{
	public:
		virtual std::vector<Line> getHoleData() = 0;
		virtual Json::Value getHoleJsonData() = 0;
	};
	class CircleHole:public Hole
	{
	public:
		double X;
		double Y;
		double radius;
		CircleHole() = default;
		CircleHole(double X, double Y, double radius)
		{
			this->X = X;
			this->Y = Y;
			this->radius = radius;
		}
		virtual std::vector<Line> getHoleData() {
			std::vector<Line> lineVec;
			Line line1(Point(X - radius, Y), Point(X + radius, Y));
			Line line2(Point(X, Y - radius), Point(X, Y + radius));
			lineVec.push_back(line1);
			lineVec.push_back(line2);
			return lineVec;
		}
		virtual Json::Value getHoleJsonData()
		{
			Json::Value res;
			res["holeType"] = "circle";
			res["center"]["X"] = X;
			res["center"]["Y"] = Y;
			res["radius"] = radius;
			return res;
		}
	};
	Box box;
public:
	std::vector<hNConnectRebar> _hNConnectRebarVec;
	std::vector<hConnectRebar> _hConnectRebarVec;
	std::vector<vNConnectRebar> _vNConnectRebarVec;
	std::vector<vConnectRebar> _vConnectRebarVec;
	//std::shared_ptr<Hole> _spHole;
	std::vector<std::shared_ptr<Hole> > _spHoleVec;
public:
	EndProfileSection(Point& p1, Point& p2) {
		
		box.iniBox(p1, p2);
	}
	Box& getBox() { return box; }
	bool addHNConnectRebar(int index, Line line, std::string rebarText) {
		_hNConnectRebarVec.push_back(hNConnectRebar(index, line, rebarText));
		return false;
	}
	bool addHConnectRebar(int index,Line line,std::string rebarText) {
		_hConnectRebarVec.push_back(hConnectRebar(index,line,rebarText));
		return false;
	}
	bool addVNConnectRebar(int index, Line line, std::string rebarText) {
		_vNConnectRebarVec.push_back(vNConnectRebar(index, line, rebarText));
		return false;
	}
	bool addVConnectRebar(int index, Line line, std::string rebarText) {
		_vConnectRebarVec.push_back(vConnectRebar(index, line, rebarText));
		return false;
	}
	bool addHole(std::shared_ptr<Hole> sp)
	{
		this->_spHoleVec.push_back(sp);
		return true;
	}
	Json::Value getWallProfileJsonData()
	{
		Json::Value profileJson;
		profileJson["location"]["x1"] = box.left;
		profileJson["location"]["y1"] = box.top;
		profileJson["location"]["x2"] = box.right;
		profileJson["location"]["y2"] = box.bottom;

		Json::Value hConnectRebarJsonData;
		Json::Value hNConnectRebarJsonData;
		Json::Value vConnectRebarJsonData;
		Json::Value vNConnectRebarJsonData;
		
		for (auto& rebar : _hConnectRebarVec)
		{
			hConnectRebarJsonData.append(rebar.getRebarInfo());
		}
		for (auto& rebar : _hNConnectRebarVec)
		{
			hNConnectRebarJsonData.append(rebar.getRebarInfo());
		}
		for (auto& rebar : _vConnectRebarVec)
		{
			vConnectRebarJsonData.append(rebar.getRebarInfo());
		}
		for (auto& rebar : _vNConnectRebarVec)
		{
			vNConnectRebarJsonData.append(rebar.getRebarInfo());
		}
		profileJson["hConnectReabar"] = hConnectRebarJsonData;
		profileJson["hNConnectReabar"] = hNConnectRebarJsonData;
		profileJson["vConnectReabar"] = vConnectRebarJsonData;
		profileJson["vNConnectRebar"] = vNConnectRebarJsonData;

		//墙洞
		for (auto& hole : _spHoleVec)
		{
			profileJson["hole"].append(hole->getHoleJsonData());
		}
		return profileJson;
	}
};
#endif