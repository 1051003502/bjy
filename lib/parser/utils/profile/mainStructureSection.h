#ifndef __PARSER_UTILS_MAINSTRUCTURESECTION_H__
#define __PARSER_UTILS_MAINSTRUCTURESECTION_H__
#include "data.h"
#include "blockSet.h"
#include "publicFunction.h"

#define smallLineRatio 0.2

class StirPointInfo
{
public:
	StirPointInfo() {}
	StirPointInfo(Point& infoPoint, std::vector<int>& leadIndexVec, std::vector<int>& stirIndexVec);

	Point infoPoint;//�ı���
	int baseLineIndex;//��������
	std::vector<int> leadIndexVec;//�ı��������������
	std::vector<int> stirIndexVec;//�ı��������εĸֽ�������
public:
	Point& getInfoPoint() { return infoPoint; }
	std::vector<int>& getStirIndexVec() { return stirIndexVec; }
	std::vector<int>& getLeadIndexVec() { return leadIndexVec; }
	bool setBaseLineIndex(int index) { return this->baseLineIndex = index; }
	int getBaseLineIndex() { return baseLineIndex; }
};
class StirBreakPoint {
public:
	StirBreakPoint() = default;
	StirBreakPoint(Line& line) {
		auto e = line.e;
		auto s = line.s;
		Point center((e.x + s.x) / 2, (e.y + s.y) / 2);
		center_mind = center;
		radius = line.length() / 2;//���뾶
	}
	StirBreakPoint& operator=(const StirBreakPoint& p)
	{
		center_mind = p.center_mind;
		radius = p.radius;
		return *this;
	}

	//�˴���2������Ŀ�ģ�std::set֧��StirBreakPoint�����
	bool operator==(const StirBreakPoint& p) const {
		return std::abs(this->center_mind.x - p.center_mind.x) < Precision &&
			std::abs(this->center_mind.y - p.center_mind.y) < Precision;
	}
	bool operator<(const StirBreakPoint& p) const {
		if (this->center_mind.x + Precision < p.center_mind.x)return true;
		if (this->center_mind.x - Precision > p.center_mind.x)return false;
		if (this->center_mind.y + Precision < p.center_mind.y)return true;
		return false;
	}
	//�жϴ˶ϵ��Ƿ����p�� �����ж����߶˵��Ƿ�һ���ϵ�Ȧ����
	bool surround(const Point& p) {
		return this->center_mind.distance(p) < radius;
	}
	bool addTextPoint(Point& p) {
		this->textPoint = p;
		return true;
	};
public:
	Point center_mind;//Բ��
	double radius;//�뾶
	Point textPoint;

};
class MainStructureSection
{
public:
	MainStructureSection() = default;
	//����
	//MainStructureSection(const Block& block, Data& data);
	//!���ظ�ֵ����
	//const MainStructureSection operator=(const MainStructureSection& mss);
	//!��������
	bool parserMainStructureSectionInfo(Data& data);
	//��ʼ����������
	bool setBaseDate(const std::shared_ptr<Data>& m_data, const std::shared_ptr<Block>& p_block);
	std::vector<StirBreakPoint>& getBpVec() { return _bpVec; }
	std::vector<StirPointInfo>& getStirPointInfoVec() {
		return _stirPointInfoVec;
	}
	std::vector<int>& getBaseLineIndexVec() { return baseLineVec; }
	std::map<int, std::string>& getStirInfoMap() { return _stirInfoMap; }
public:
	std::string getRebarLayer() { return _rebarLayer; }
public:
	//ͨ���ı��Ļ����߲�����֮�����������
	std::vector<int> BFSFindLine(const int& baseLineIndex, CornerData& cornerData, LineData& lineData, bool& baseLineSide);
	//ͨ����׼���Ҷ�Ӧ��һ������
	bool findLeadLine(const Block& block, Data& data, std::vector<int>& baseLineVec
		, std::vector<std::vector<int> >& leadLineGroup);
	//���text�Ƿ��Ǹֽ��ע
	bool checkRebarText(std::string text);
	//�Ҹֽ��ע��Ӧ�Ļ�׼�߷���baseLineVec��
	bool findBaseLine(const Block& block, Data& data, std::vector<int>& baseLineVec, std::vector<Point>& pointVec);
	//�жϵ��Ƿ�������
	bool pointCloseLine(const Point& point, const Line& line);
	//���line�Ƿ������������ ��ʱֻͨ���������ж�
	bool checkBaseLine(Line line);
	//�жϵ������Ƿ�ƽ��
	bool isparallel(const Point& pt, const Line& line);
	//�ж��ı���ķ��� //1ˮƽ 0��ֱ  -1б�ŵ�
	int getPointDirection(const Point& p);
	//��һ�����߼����У��ҳ��б�ʾ���õ�С����
	double findSmallLine(std::vector<Line>& lineData, std::vector<int>lineIndexVec);
	//�����������ͼ�ľ���ֽ���Ϣ������ȡ��_bpLayerͼ�㣬_rebarLayerͼ�㣬_bpVec,v h IndexVec
	//��ʼ��ˮƽ��ϵ�
	bool iniStirBreakPoint(Data& data);
	//�ҷ�ˮƽ��ͼ��
	bool findRebarLayer(Data& data, std::vector<std::vector<int> >& leadLineIndex);
	//�÷�ˮƽ���ͼ���ʼ����ˮƽ�����
	bool findRebarLineIndex(Data& data);
	//���ı�����Ӧ����
	bool mapStrAndRebar(Data& data, std::vector<std::vector<int> >& leadLineGroup, std::vector<Point>& pointVec);
	bool cleanRebarData(Block& block, Data& data);
	//���ӶϿ��İ��� ��Ҳ����Ѱ�Ұ��ߣ���������ܶ���һ������������д��Ż���
	bool connectLine();
	//������ṹͼ�㣨�ж���������ͨ���Աߵ��ߣ�
	bool findMainLayer();

public://����ͼ��������
	std::shared_ptr<Block> _spBlock;
	std::shared_ptr<Data> _spData;
	std::string _bpLayer;//ˮƽ������ͼ��
	std::vector<int> baseLineVec;//����
	std::vector<int> _bplineIndexVec;//ˮƽ�����
	std::string _rebarLayer;//��ˮƽ������ͼ��
	std::vector<int> _hLineIndexVec;//!��ˮƽ��ĺ�������
	std::vector<int> _vLineIndexVec;//!��ˮƽ�����������
	std::vector<int> _sLineIndexVec;//!��ˮƽ���б������
	std::vector<StirBreakPoint> _bpVec;//!ˮƽ������
	std::vector<StirPointInfo>_stirPointInfoVec;//��ˮƽ������
	std::map<int, std::string> _stirInfoMap;//������ϴ��ķ�ˮƽ������
	std::string _mainLayer;

public:/*ǽ*/
	std::vector<int> _wallLineIndexVec;//��ֱǽ��
	std::vector<Line> _wallLineVec;//��ֱ���Ӻ��ǽ��
	std::vector<Line> _cuttingWallLineVec;//������ض�֮���ǽ��
	//std::vector<std::shared_ptr<ProfileWall>> _profileWallVec;
public:
	//��������ͼ���һЩ����
	bool findMainLine();
	//��������ͼ����ǽ��
	bool findWallLineIndex();
	bool findWallConnectRebar();
	//���ð��߽ضϳ��µ�ǽ��
	bool cutWall();
	//���������
	void normalCutting();
	//�Ĳ������
	void specialCutting();

	//����_mainstru�е�_cuttingwallLineVec����ǽ������profileWallVec
public:/*��*/
	std::vector<int> _boardLineIndexVec;//ˮƽ����
	std::vector<Line> _boardLineVec;//ˮƽ���Ӻ�İ���
	double bottomAttitude;//�װ嶥���������꣨yֵ��
	bool findBoardLineIndex();
	double getBottomAttitude() {
		if (_boardLineVec.size() >= 2)return _boardLineVec[1].s.y;
		else return 0;
	}
	//���ڴ�ӡ����
public:
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
	//��ʼ����ͼ����
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	/*std::vector<int> writeIndexVec;
	std::vector<int> greenIndexVec;
	std::vector<std::vector<int> > colorBlocks;
	std::vector<Line> writeLineVec;*/
};




#endif