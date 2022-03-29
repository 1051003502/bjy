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
		BottomBoard,//�Ӱ�ĵײ���ʼ
		TopBoard,   //�Ӱ�Ķ�����ʼ
		RebarBending,//�Ӹֽ�����۴�
		WallMargin,   //��ǽ�ı�Ե��
		none   //��
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
public://get set����
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
	int index;//�ֽ�����
	std::string rebarInfo;//�ֽ������ı�
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
	std::string _rebarInfo;//��Ҹ�������
	int index;
	Line line;
	double _haunchWidth;//Ҹ��
	double _hunchHeight;//Ҹ��
	double _insideWallLength;//����ǽ�ĳ���
	double _insideBoardLength;//�����ĳ���
};
//�����ֽ�


//����ǽ�ľ�����Ϣ
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
		

		//��ֱ�ǹ�ͨ
		Json::Value vNConnectReabarJsonData;
		for (auto& nConnectRebar :  _vNConnectRebarVec)
		{
			vNConnectReabarJsonData.append(nConnectRebar.getRebarInfo());
		}
		wallSectionJsonData["verticalNotConnectReabar"] = vNConnectReabarJsonData;

		//��ֱ��ͨ
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
		
		
		//��Ҹ��
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
	double  _height;//���
	double _thick;//���
	double _attitude;//���Ա��
	Box box;//ǽ��ɵ�box
	std::vector<StirBreakPoint> _hConnectRebarVec;//ˮƽ��
	std::vector<vNConnectRebar> _vNConnectRebarVec;//��ֱ�ǹ�ͨ��
	std::vector< vConnectRebar> _vConnectRebarVec;//��ֱ��ͨ��
	std::vector<Haunch>_haunchVec;//��Ҹ
	//std::pair<std::string, double>_startPointReferenceAxis;//���Ĳο��� ���� ����
	//std::pair<std::string, double>_endPointReferenceAxis;//�յ�Ĳο��� ���� ����
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

		//ǽ��
		for (auto& hole : _spHoleVec)
		{
			profileJson["hole"].append(hole->getHoleJsonData());
		}
		return profileJson;
	}
};
#endif