#ifndef __PARSER_UTILS_BEAMSET_H__
#define __PARSER_UTILS_BEAMSET_H__
#include "data.h"
#include "block.h"
#include "beam.h"
#include "pillar.h"
#include "axis.h"
#include <fstream>
#include "publicFunction.h"
#include <regex>



class BeamTextPoint
{
public:
	BeamTextPoint(const Point& point, const std::string& name, const bool& blead = false);
public:
	Point pt;//�ı�������
	bool bLeadLine;//�Ƿ�������
	std::string strText;//�ı�
};

/**
* Lead ��ʾ�����߽���ϵ�����
*/
class Lead
{
public:
	Lead() = default;
	friend bool operator==(const Lead& leadGrop, const Lead& leadGrop1);

public:
	std::vector<int> primevalIndexVec;			//ͼֽ�е������ߵ�����
	std::vector<int> nowIndexVec;				//������ߵ������ߵ�����
	std::vector<Point> insertPointVec;			// �����ߵĶ˵�
	std::vector<std::pair<int, int>> cornerIndex;//old,new:cross line indices
	bool bSide;			// �Ƿ��бպ��ߵ�
	bool bPair;			// �Ƿ�ͱ߽����
	double polyWidth;		// ����ߵĿ��
	double refCount;		//���������߱����õĴ��� ��ʱ��һ���������߻ᱻ��ε��� �˴�Ҫ��m_insertPoint.size()��bSide��bPair�������ʹ��
	int beamTextPointIndex;//�����������ŵ����ı����������BeamSet�У�

};

class BorderGroup
{
public:

	BorderGroup()
	{
		bGroup = false;
		bLead = false;
		bBeamTextPoint = false;
	}
public:
	std::vector<int> borderVec;		//��ͨ�������б߽�������
	std::vector<std::pair<int, bool>> leadGropsIndexVec;//��һ����ʾlead����������һ����ʾ��ʱ��״̬ ��ʱ��ͬһ�����߻����ζ����
	std::vector<int> beamTextPointVec; // �ͱ߽���ƥ��������ı��������
	bool bLead;// �Ƿ�������
	bool bGroup;//�Ƿ����
	bool bBeamTextPoint; // �Ƿ����ı���
};

class BeamSet
{
public:

	bool setMember(const std::shared_ptr<Axis>& axises,
				   const std::vector<std::shared_ptr<Pillar>>& pillars);
	bool findBeam(Data& data/*,
				  std::vector<int>& mark_index,
				  std::vector<Point>& corner,
				  std::vector<Line>& temp_line,
				  std::vector<std::vector<int>>& m_temp_index*/);

	//************************************
	// ����	: Beams
	// ȫ��	: BeamSet::beams
	// ����	: public 
	// ����	: bool ���󷵻�true������Ϊfalse
	// ����	: _beamVec ���ؽ����õ���
	// ����	: data 
	// ˵��	: ������õı߽��߼����߽�������
	//************************************
	bool beams(std::vector<std::shared_ptr<Beam>>& _beamVec, Data& data);
	
	std::shared_ptr<Beam> parseOneLeadBeamBorderGroup(Data& data,
													  BorderGroup& borderGrpItem,
													  Beam::Direction d);

	std::shared_ptr<Beam> parseNoLeadBeamBorderGroup(Data& data, BorderGroup& borderGrpItem,
													 Beam::Direction d);

public:
	//************************************
	// ����	: findBeamLayer
	// ȫ��	: BeamSet::findBeamLayer
	// ����	: private 
	// ����	: bool
	// ����	: data
	// ˵��	: �����й���ͼ�����
	//************************************
	bool findBeamLayer(const std::vector<std::pair<int, int>>&_columnHBorderIndex, LineData& lineData);
private:
	//************************************
	// ����	: findBeamLines
	// ȫ��	: BeamSet::findBeamLines
	// ����	: private 
	// ����	: bool
	// ����	: data
	// ˵��	: �����й���ͼ�����
	//************************************
	bool findBeamLines(Data& data);

	//************************************
	// ����	: findLeadLinesAndTextPoint
	// ȫ��	: BeamSet::findLeadLinesAndTextPoint
	// ����	: private 
	// ����	: bool
	// ����	: data
	// ˵��	: �������е��ı����Ӧ����������
	//************************************
	bool findLeadLinesAndTextPoint(Data& data);

	//�����ı���Ӧ��λ�õ�
	bool findTextCandidatesPoints(TextPointData& textPointData);

	//���������߲�����������
	//************************************
	// ����	: findLeadLineIndex
	// ȫ��	: BeamSet::findLeadLineIndex
	// ����	: private 
	// ����	: int ����-1Ϊ�ҵ�������-2��ʾ�������㹫�õ�������
	// ����	: goal_index��Ŀ��������
	// ����	: point_index ����ѡ�������
	// ����	: textPointIndex������־�ı���Ӧ�ĵ�����
	// ����	: data
	// ˵��	: ������־�ı���Ӧ�ĺ�ѡ�߶˵��ڣ��ҳ�����ϵ���������������������Ϊ�����ı����Ӧ��������
	//************************************
	bool findLeadLineIndex(int& goal_index, const std::vector<int>& point_index, const int& textPointIndex, Data& data);
	//���������ߵ��������
	bool findRelatedLineIndex(const int& relateLine, Data& data, std::vector<int>& dline, std::vector<int>& leadCorner);
	//�ж��������Ƿ��в���㣬����ʼ����������
	bool addLeadGroupInsertPoint(Lead& leadGrop, Data& data, std::vector<int>& dlines);
	//��ʼ�������Լ�֮��Ľ���
	bool initializationBeamLinesCorners();
	//��ʼ������
	bool setBeamData();
	/*�����ǳ���������Ҫ�ĺ���*/
	//����ƽ�����
	bool  retrunBorderAverageWidth(std::vector<int>& goal_index);
	double borderWidthAverage(const int& conut, std::vector<int>& borderMap, std::vector<int>& goal_index);
	//!�������һ����Ժõı߽���
	bool findPairedBorder(std::pair<int, int>& pairedBorder);
	//!����goal_index������Ӧ�ı߽��ߵĶԱߣ�����ֵδ��Աߵ�����
	int findPairedBorder(const int& goal_index, const double& relative_value, LineData& beamLines,
						 const std::vector<Lead>& leadGrops);//��Ŀ�ĵ�����

	int findGropPairedBorder(const int& original_index, const double& relative_value, LineData& beamLines,
							 const std::vector<int>& borderGrop);//��Ŀ���з�Χ������
																 //!goal_index��Ҫ���ҶԱߵı߽���������������ֵ��goal_index��ͬʱ��ʾ����ʧ�ܣ�������ֵ��ͬʱwidthΪ����֮��Ŀ��
	int returnGroupPairIndex(const int& goal_index, double& width);

	/*��������*/
	//!Ϊ���߳������з���
	bool findBeamBorderGrops();

	//************************************
	// ����	: slashCornerJudgment
	// ȫ��	: BeamSet::slashCornerJudgment
	// ����	: private 
	// ����	: bool
	// ����	: sLineIndex �߽�������
	// ����	: lineIndex �߽�������
	// ����	: point ����
	// ˵��	: �ж�б�ߵ��ཻ��������ڶԱ߽���з���
	//************************************
	bool slashCornerJudgment(const int& sLineIndex, const int& lineIndex, const Point& point);

	//************************************
	// ����	: bLeadLineBeamLayer ��ʾ����ͼ������ϲ�ѯ
	// ȫ��	: BeamSet::bLeadLine
	// ����	: private 
	// ����	: bool ����true����ʾΪ���Ѿ����ڵ����ߣ�false��������
	// ����	: leadGrops�����е�������
	// ����	: index ����Ҫ���ҵ���������
	// ����	: LeadGropIndex�����������ߵ�����·��������������������
	// ˵��	: �ж�index�Ƿ�Ϊ������
	//************************************
	bool bLeadLineBeamLayer(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex);

	//************************************
	// ����	: bLeadLineAllLines ������ͼֽ�Ĳ����ϲ���
	// ȫ��	: BeamSet::bLeadLineAllLines
	// ����	: private 
	// ����	: bool
	// ����	: leadGrops
	// ����	: index
	// ����	: LeadGropIndex
	// ˵��	: �˺�����ǰһ��������������ͬ��ֻ���������յ�������ͬ��
	//���¸�����������ͼ�����������жϣ������������������Χ���ж�
	//************************************
	bool bLeadLineAllLines(const std::vector<Lead>& leadGrops, const int& index, int& LeadGropIndex);
	//�޳���������ʶ���������
	bool removeExcessLeadInBorderGrop(std::vector<BorderGroup>& borderGrops, LineData& beamLines, const double& relative_value,
									  CornerData& borderCorners, const double& average_distance, std::vector<Lead>& leadGrops);
	//!�ж��������Ƿ�������ǰ�߽�
	bool bLeadlineBorderPaired(LineData& beamLines, const double& relative_value, CornerData& borderCorners,
							   BorderGroup& BeamGrop, std::vector<Lead>& leadgrops);
	//!���ҷ���������������ͱ߽����飬borderGrops��ʾ���������飬bLead��ʾ�Ƿ������ߣ�bGrop��ʾ�Ƿ���飬beadGrop��ʾ���е������飬bside��ʾ�Ƿ��жϵ㣬bPair��ʾ�Ƿ���ԣ���������Ϊ���ؽ��
	bool byConditionExtractBorderGropLeadGrop(const std::vector<BorderGroup>& borderGrops, const bool& bLead, const bool& bGrop,
											  const std::vector<Lead>& leadGrop, const bool& bSide, const bool& bPair, std::vector<int>& conditationBorder, std::vector<int>& conditationLead);
	//!��δ��߽�����Ե������ٴ���ԣ������Ż�û�в����ཻ��ϵ��ȴ�໥�ɶԵ����
	bool borderLeadPair1(Data& beamData, const double& averageWidth, std::vector<Lead>& leadGrops, std::vector<BorderGroup>& borderGrops,
						 std::vector<int>& conditationBorder, std::vector<int>& conditationLead);
	//!�ж�borderPair�Ƿ�ͬΪһ�飬���ʱborderGroupsIndex���ر�����������������true,�����Ϊͬһ���򷵻�false,borderGroupsIndex=-1
	bool bBorderGrop(std::vector<BorderGroup>& borderGrops, const std::pair<int, int>& borderPair, int& borderGroupsIndex);
	//��
	bool byConditionExtractBorderGropTextPoints(const std::vector<BorderGroup>& borderGrops, const bool& bLead1, const bool& bGrop,
												const std::vector<BeamTextPoint>& textPoints, const bool& bLead2, std::vector<int>& conditationBorder, std::vector<int>& conditationPoint);
	//!�Ż�û�����ߵ��ı�����ɶԱ߽���֮�����Թ�ϵ
	bool borderPointPair(Data& beamData, const double& averageWidth, std::vector<BeamTextPoint>& textPoints, std::vector<BorderGroup>& borderGrops,
						 std::vector<int>& conditationBorder, std::vector<int>& conditationTextPint);

	/*�����Ƿ����������Ϣ���Լ�����ʱ�������Լ��Ƿ��а���λ����Ϣ*/
	bool returnBeamSpanNumber(const std::string& beam_name, bool& b_axis, int& number);
	//�ڰ����п�����Ϣ�Ļ����ϣ�����λ����Ϣ��������ҳ�����λ�÷�Χ
	bool returnLocationRange(const std::string& beam_name, std::string& begin, std::string& end);
	//����������������� �������ң����ϵ��£�
	bool setPillarsRtree();
	//����ĳ���ڵ���������
	bool returnPointInAxisRegion(const Point& point, const Axis& axise,
								 std::vector<Axis::AxisLine>& point_mark);


	//************************************
	// ����	: findCrossLinesExceptItself
	// ȫ��	: BeamSet::findCrossLinesExceptItself
	// ����	: private 
	// ����	: bool
	// ����	: _crossLineVec ���صĽ�������
	// ����	: excludeLineVec ��Ҫ�ų�������
	// ����	: targetIdx ��Ҫ��ѯ�ߵ�����
	// ˵��	: ���ҳ���������ָ������ӵ�����
	//************************************
	bool findCrossLinesExceptItself(std::set<int>& _crossLineVec,
									std::vector<int>& fromLineVec,
									const std::vector<int>& excludeLineVec,
									int targetIdx);

	//************************************
	// ����: returnPiarIdxInLines
	// ȫ��: BeamSet::returnPiarIdxInLines
	// ����: private 
	// ����: bool �ҵ�����true������false
	// ����: _pairIdx �����ߵ�����
	// ����: fromLineVec ��Ҫ��ѯ����
	// ����: excludeVec ��Ҫ�ų�����
	// ����: targetIdx ��Ҫ��ѯ����
	// ˵��: ������ָ�����������е������С���ߵ�����
	//************************************
	bool matchPairIdx(int& _pairIdx,
					  std::vector<int>& fromLineVec,
					  std::vector<int>& excludeVec,
					  int targetIdx);

	//************************************
	// ����	: findBeamBorders
	// ȫ��	: BeamSet::findBeamBorders
	// ����	: private 
	// ����	: bool ���ڱ������Ϊtrue������false
	// ����	: _queue ���صĶ���
	// ����	: fromLineVec �����ı��߼���
	// ����	: excludeVec ��Ҫ�ų��ı���
	// ����	: targetIdx Ŀ�����
	// ˵��	: ��fromLineVec�в�����target��������ӻ��ཻ�ı��߶Լ��뵽_queue��
	//************************************
	bool findBeamBorders(std::queue<std::pair<int, int>>& _queue,
						 std::vector<int>& fromLineVec,
						 std::vector<int>& excludeVec,
						 int targetIdx);

	//************************************
	// ����	: excludeLeadIdx
	// ȫ��	: BeamSet::excludeLeadIdx
	// ����	: private 
	// ����	: bool
	// ����	: boundIdx �жϵı߽�����
	// ����	: fromLineVec ���������߽�������
	// ����	: allLeadIdxVec ����������
	// ˵��	: �ж�boundIdx�߽��ߵĽ����Ƿ�����������allLeadIdxVec�����е����ཻ
	//************************************
	bool excludeLeadIdx(int boundIdx,
						std::vector<int>& fromLineVec,
						std::vector<std::pair<int, bool>>& allLeadIdxVec
	);

	//************************************
	// ����	: searchTextPoint
	// ȫ��	: BeamSet::searchTextPoint
	// ����	: private 
	// ����	: bool
	// ����	: _textPointVec
	// ����	: leadIdx
	// ˵��	: ����ָ�������������������е��ı���
	//************************************
	bool extractTextAnnotationInfo(std::shared_ptr<Beam> _spBeam,
								   Data& data,
								   int leadIdx);

	//************************************
	// ����	: GetClosestLineIdxFrom
	// ȫ��	: BeamSet::getClosestLineIdxFrom
	// ����	: private 
	// ����	: bool
	// ����	: _idx ����������ߵ�����
	// ����	: fromLineVec �����ķ�Χ
	// ����	: pt �ı���
	// ˵��	: ��fromLineVec�л�ȡ���L���ı�����������
	//************************************
	bool getClosestLineIdxFrom(int& _idx,
							   const std::vector<int>& fromLineVec, 
							   const Point& pt);

	//************************************
	// ����	: SearchText
	// ȫ��	: BeamSet::searchText
	// ����	: private 
	// ����	: bool
	// ����	: height ���ص��ı���ĸ�
	// ����	: _spBeam
	// ����	: data
	// ����	: pt ����Բ��
	// ����	: radius �����뾶
	// ˵��	: �ڸ����ĵ�pt����radiusΪ�뾶�����ı��㣬��������spBeam
	//************************************
	bool searchText(std::shared_ptr<Beam> _spBeam,
					Data& data,
					Point& pt,
					double radius);

	//************************************
	// ����	: CheckDirectionLines
	// ȫ��	: BeamSet::checkDirectionLines
	// ����	: private 
	// ����	: bool
	// ����	: d
	// ����	: lineIdxVec
	// ˵��	: �ж���ͨ���ķ���
	//************************************
	bool checkDirectionLines(Beam::Direction& d,
							 const std::vector<int>& lineIdxVec);



	//************************************
	// ����	: iniColumnRtree
	// ȫ��	: BeamSet::iniColumnRtree
	// ����	: private 
	// ����	: bool
	// ˵��	: ��ʼ�������ӵ�Rtree�����ڲ������ӵ�λ����Ϣ
	//************************************
	bool iniColumnRtree();

	//************************************
	// ����	: returnRtreeColumnIndex
	// ȫ��	: BeamSet::returnRtreeColumnIndex
	// ����	: private 
	// ����	: std::vector<int>
	// ����	: min ��һ��һά����Ϊ�����飬��һ�����left,�ڶ������bottom
	// ����	: max ��һ��һά����Ϊ�����飬��һ�����right,�ڶ������top
	// ˵��	: ����������Χ�����������ڵ�����������
	//************************************
	std::vector<int> returnRtreeColumnIndex(const double min[],
											const double max[]);

	//************************************
	// ����	: UpdateMinMax
	// ȫ��	: BeamSet::updateMinMax
	// ����	: private 
	// ����	: bool
	// ����	: min ��С��x��y
	// ����	: max ����x��y
	// ����	: line ��Ҫȡ�����ڸ��µ�������
	// ˵��	: ��line�������������С��x��y������x��y�ֱ�min��max
	//************************************
	bool updateMinMax(double* min,
					  double* max,
					  const Line line);

	bool updateColChain(std::shared_ptr<Beam> spBeam,
						const Line& line);

	//************************************
	// ����	: ParseSpan
	// ȫ��	: BeamSet::parseSpan
	// ����	: private 
	// ����	: bool
	// ����	: _spBeam Ҫ��������
	// ����	: min �������µ�
	// ����	: max �������ϵ�
	// ˵��	: �����磬������
	//************************************
	bool parseSpan(std::shared_ptr<Beam> _spBeam,
				   Data& data);



	//************************************
	// ����	: ParseBeamInfo
	// ȫ��	: BeamSet::parseBeamInfo
	// ����	: private 
	// ����	: bool
	// ����	: _spBeam
	// ����	: data
	// ����	: leadIdx
	// ˵��	: ���������ϵ�����Ϣ
	//************************************
	bool parseBeamInfo(std::shared_ptr<Beam> _spBeam, Data& data, int leadIdx);

public:
	/*�жϵ���������*/
	bool findSmallHAxis(const Point& point, const Axis& axise, Axis::AxisLine& haxis_mark);
	bool findBigHAxis(const Point& point, const Axis& axise, Axis::AxisLine& haxis_mark);
	bool findSmallVAxis(const Point& point, const Axis& axise, Axis::AxisLine& vaxis_mark);
	bool findBigVAxis(const Point& point, const Axis& axise, Axis::AxisLine& vaxis_mark);

	void setMFCPaintTestVec(std::vector<int>& testIdxVec, std::vector<Line>& testLineVec);

public:
	std::string beamLayer;
	std::vector<Beam> beamVec;		//������
	Data beamData;				// ������������
	std::vector<int> primevalIndexVec;//ԭʼborder����
	LineData beamLines;//��ͼ��������߰���������
	std::vector<BeamTextPoint>	beamTextPointVec;//��������Ӧ���ı���λ���Լ��Ƿ���������
	std::vector<Lead> leadGroupVec;//����������
	CornerData beamCornerData;//��ͼ���ڵ����н���
	double avgWidth;//ƽ�����
	std::vector<BorderGroup> borderGroupVec;//���еĳ���ı߽���
	std::vector<int> condidateBorderIdxVec;
	std::vector<int> condidateLeadIdxVec;
	std::vector<int> condidateBeamTextPointIdxVec;
public:
	//������֪�ı���
	std::shared_ptr<Axis> spAxis;
	std::vector<std::shared_ptr<Pillar>> spPillarVec;
	RTreeData columnRTree;              // aka pillarTRee ��pillars����Rtree���ڿ�������
	RTreeData borderLineRTree;
	std::vector<std::string> strBorderLayerVec;

	//���� 
	std::vector<int>* pIdxVec;
	std::vector<Line>* pLineVec;
};

#endif