#ifndef __PARSER_UTILS_BEAM_H__
#define __PARSER_UTILS_BEAM_H__
#include <fstream>
#include <regex>
#include <chrono>
#include "block.h"
#include "data.h"
#include "publicFunction.h"
#include "ColumnSection.h"
#include "beamSection.h"
#include "pillar.h"


class Beam;

struct BeamPosZDelta
{
	BeamPosZDelta() = default;
	BeamPosZDelta(const BeamPosZDelta& o) = default;
	double rad;
	double length;
	Point pt;
	PillarDistance axisDistance;
};
using RelativeDistance = std::pair<std::string, double>;

struct Distance2AxisInCartesianCoordinates
{
	Point pt;
	RelativeDistance relativeHDistance;
	RelativeDistance relativeVDistance;
};

class BeamPosition
{
private:
	static const std::string LongitudianlPriorityReg;

public:
	//֧������
	enum class SupportType
	{
		Column,//��
		Beam,//��
		Wall,//ǽ
		Pending,//����
		Unknown//δ֪
	};

	//�����͵ײ�
	enum class LongitudinalPos
	{
		Up,
		Down
	};


	BeamPosition() = default;
	~BeamPosition() = default;

	//************************************
	// ����	: ExtractPriorityAnnotation
	// ȫ��	: BeamPosition::extractPriorityAnnotation
	// ����	: public 
	// ����	: bool �����ɹ�����true������false
	// ����	: d ����λ��
	// ����	: strText ʶ���ı�
	// ˵��	: ʶ����λ�õ��ݽ����Ϣ
	//************************************
	bool extractPriorityAnnotation(LongitudinalPos d, const std::string& strText);

	bool extractMidPriortyAnnotation(const std::string& strText);
	//˵��������ӿ�
	std::string getSupportType();
	std::string getWaistReinforce();
	std::string getRelativeElevation();
	std::string getTopReinforce();
	std::string getBottomReinforce();
	

public:
	SupportType supportType;					//֧������
	double supportWidth;						//֧�ſ��
	bool bMiddle;								//�Ƿ��п�����Ϣ
	std::string waistLongitudinalReinforce;		//�������ݽ�
	std::string relativeElevation;				//��Ա��
	std::string topLongitudinalReinforce;		//�����ݽ�
	std::string btmLongitudinalReinforce;		//�ײ��ݽ�

	//�ضϳ���
	std::set<std::pair<double, double>, std::greater<std::pair<double, double>>> topTruncatedLength;// <pseudo coordinate y,length>
	std::set<std::pair<double, double>, std::greater<std::pair<double, double>>> btmTruncatedLength;
	//ê�̳���
	std::set<std::pair<double, std::string>, std::greater<std::pair<double, std::string>>> topAnchorageLength;//<pseudo coordinate y,string>
	std::set<std::pair<double, std::string>, std::greater<std::pair<double, std::string>>> btmAnchorageLength;

	//��λ�õ����·���Ϣ
	BeamPosZDelta topZDelta;
	BeamPosZDelta btmZDelta;

	//������Ϣ
	std::shared_ptr<BeamSection> spBeamSection;
};

class BeamSpan
{
public:
	//���е�λ��
	enum class SpanPosition
	{
		Left,
		Right,
		Mid
	};

	BeamSpan() = default;
	~BeamSpan() = default;
	//��ķ���
	enum class SpanDirection
	{
		Horz,
		Vert,
		Slash
	};

	//************************************
	// ����	: SearchAnnotationPriority
	// ȫ��	: BeamSpan::SearchAnnotationPriority
	// ����	: public 
	// ����	: bool
	// ����	: spBeam
	// ����	: box
	// ����	: data
	// ˵��	: �������ķ������ԭλ��ע
	//************************************
	bool searchAnnotationPriority(std::set<int>& textPtSet, std::shared_ptr<Beam> spBeam, const Box& box, Data& data, SpanPosition spanPos);

	//************************************
	// ����	: SearchMidPriority
	// ȫ��	: BeamSpan::SearchMidPriority
	// ����	: public 
	// ����	: bool
	// ����	: spBeam
	// ����	: box1
	// ����	: box2
	// ����	: data
	// ˵��	: ���м��ԭλ��ע
	//************************************
	bool searchMidPriority(std::set<int>& textPtSet, std::shared_ptr<Beam> spBeam, Box box1, Box box2, Data& data);

	void  searchBorders(const std::shared_ptr<Pillar>& col2,
						const std::shared_ptr<Pillar>& col1,
						std::shared_ptr<Beam> _spBeam);

	bool updateWithAndDirection();

	bool updateNetLength(const std::shared_ptr<Pillar>& col2,
						 const std::shared_ptr<Pillar>& col1);

	bool updateSpanMidlinePosition(const std::shared_ptr<Pillar>& col1,
								   const std::shared_ptr<Pillar>& col2,
								   std::shared_ptr<Axis>& axis,
								   std::vector<int>& idxVec,
								   std::vector<Line>& lineVec);

	void getLeftStartOfMidline(Line& l1,
							   Line& l2,
							   Line& tpLine);

	Point getLeftStartInMidlineOfBtmLine(Line& btmLine,
										 Line& topLine,
										 Line midline);


	Point getRightEndInMidlineOfBtmLine(Line& btmLine,
										Line& topLine,
										Line midline);

	Point getLeftStartInMidlineOfTopLine(Line& topLine,
										 Line& btmLine,
										 Line midline);

	Line getMidline(Line& l1, Line& l2);

	Line getVLineBasedOnStartOfLine2(Line& l1,
									 Line l2,
									 double offset = 0.0);

	Line getVLineBasedOnEndOfLine2(Line& l1,
								   Line l2,
								   double offset = 0.0);

	bool checkRightBorderBound(Point& pt,
							   const std::shared_ptr<Pillar>& pillar2,
							   Line& l1);

	bool checkLeftBorderBound(Point& pt,
							  const std::shared_ptr<Pillar>& pillar1,
							  Line& l1);

	//˵��������ӿ�
	unsigned char getSpanId();//�����������
	std::pair<std::string, std::string>getSize();//�������ĳߴ�
	double getSpanNetLength();//�������羻��
	double getRelativeElevation();//����������Ա��
	std::vector<Line>getBorders();
	Distance2AxisInCartesianCoordinates beamLocStart();
	Distance2AxisInCartesianCoordinates beamLocEnd();
	std::vector<BeamPosition> getBeamPosition();//�϶�������������һ���������У�����





public:
	unsigned char spanId;				//�������
	std::string strSecHeight;			//�����
	std::string strSecWidth;			//�����
	double spanNetLength;				//����
	double relativeElevation;			//��Ա��
	BeamPosition left;					//���֧��
	BeamPosition right;					//�Ҷ�֧��
	BeamPosition mid;					//����
	std::pair<int, int> supportPair; //֧�����͵�������

	// ���λ����Ϣ
	Distance2AxisInCartesianCoordinates midlineStart;
	Distance2AxisInCartesianCoordinates midlineEnd;
	SpanDirection spanDirection;		// ��ĳ���
	double redundantWidth;				//����Ŀ��
	std::vector<Line> borders;

	//! test 
	std::vector<int>* pIdxVec;
	std::vector<Line>* pLineVec;
private:
	void getRightEndOfMidline(Line& l1, Line& l2, Line& tpLine);
	Point getRightEndInMidlineOfTopLine(Line& topLine, Line& btmLine, Line& midline);
};


class Beam
{

public:
	static const std::string BeamReg;
	static const std::string BeamReg2;
	static const std::string SpanAxisSecReg;
	static const std::string SpanReg;
	static const std::string SecSizeReg;
	static const std::string StirrupReg;
	static const std::string LongitudinalReforceReg;
	static const std::string LongitudinalReforceReg2;
	static const std::string WasitLongitudinalReforceReg;
	static const std::string ElevationReg;
	static const std::string PriorityReg;
	//����Ϣ�ı�ƥ��ģʽ
	static const std::string BeamSecSizeReg;
	static const std::string BeamBracketReg;//ƥ�������ڵ���Ϣ
	static const std::string SpanAxisReg;//ƥ��������Ϣ


public:
	//���ĳ���
	enum class Direction
	{
		H,
		V,
		S,
	};

	Beam() = default;
	~Beam() = default;

	bool getUUID(std::string strInfo);

	//************************************
	// ����	: ExtractBeamSecSize
	// ȫ��	: Beam::extractBeamSecSize
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡ�ߴ�
	//************************************
	bool extractBeamSecSize(const std::string& strText);

	//************************************
	// ����	: ExtractBeamInfo
	// ȫ��	: Beam::extractBeamInfo
	// ����	: public 
	// ����	: bool ��ȡ�ɹ�true������false
	// ����	: strText �������ı�
	// ˵��	: ��������ȡ������Ϣ��ȡ
	//************************************
	bool extractBeamInfo(const std::string& strText);

	//************************************
	// ����	: ExtractBeamInfo1
	// ȫ��	: Beam::extractBeamInfo1
	// ����	: public 
	// ����	: bool ��ȡ����true������false
	// ����	: strText ��Ҫ��ȡ���ı�
	// ˵��	: ��ȡ����
	//************************************
	bool extractBeamInfo1(const std::string& strText);

	//************************************
	// ����	: ExtractBracketInfo
	// ȫ��	: Beam::extractBracketInfo
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡInfo1�������ڵ���Ϣ
	//************************************
	bool extractBracketInfo(const std::string& strText);


	//************************************
	// ����	: ExtractSpanAxis
	// ȫ��	: Beam::extractSpanAxis
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡ��������һ�����Ϣ
	//************************************
	bool extractSpanAxis(const std::string& strText);

	//************************************
	// ����	: ExtractBeamInfo2
	// ȫ��	: Beam::extractBeamInfo2
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡ����
	//************************************
	bool extractBeamInfo2(const std::string& strText);


	//************************************
	// ����	: ExtractBeamInfo3
	// ȫ��	: Beam::extractBeamInfo3
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡ�����ײ��ݽ���Ϣ
	//************************************
	bool extractBeamInfo3(const std::string& strText);


	//************************************
	// ����	: ExtractBeamInfo4
	// ȫ��	: Beam::extractBeamInfo4
	// ����	: public 
	// ����	: bool
	// ����	: strText
	// ˵��	: ��ȡ����
	//************************************
	bool extractBeamInfo4(const std::string& strText);

	//************************************
	// ����	: ExtractBeamElevation
	// ȫ��	: Beam::extractBeamElevation
	// ����	: public 
	// ����	: bool
	// ˵��	: ��ȡ��Ա��
	//************************************
	bool extractBeamElevation(const std::string& strText);



	//************************************
	// ����	: UpdateDirectionWidth
	// ȫ��	: Beam::updateDirectionWidth
	// ����	: public 
	// ����	: bool
	// ˵��	: ���������跽���Լ����Ŀ��
	//************************************
	bool updateDirectionWidth();

	//************************************
	// ����	: operator==
	// ȫ��	: operator==
	// ����	: public 
	// ����	: friend bool
	// ����	: beam1
	// ����	: beam2
	// ˵��	: �ж�������������Ϣ�Ƿ����
	//************************************
	friend bool operator==(std::shared_ptr<Beam>& beam1, std::shared_ptr<Beam>& beam2);

	//************************************
	// ����	: AppendSpan
	// ȫ��	: Beam::appendSpan
	// ����	: public 
	// ����	: bool
	// ����	: span
	// ˵��	: ���ÿ���뵽������
	//************************************
	bool appendSpan(std::shared_ptr<BeamSpan> span);
	
	//˵��������ӿ�
	std::string getName();//��������
	unsigned char getSpanNum();//����������
	bool getbAssemblyAnnotation();//�����Ƿ�Ϊƽ��
	std::vector<std::shared_ptr<BeamSpan>>getBeamSpanVec();





public:
	uuid beamId;
	std::vector<std::shared_ptr<BeamSpan>> spSpanVec;
	unsigned char spanNum;  //����
	std::vector<Line> borderLineVec;//���ı߽���
	//ColumnSection section;
	int leadIdx;//������������������
	std::vector<Axis::AxisLine> beamRegion;
	//���ڴ�ȡ����������֧��
	std::vector<int> colChain;
	//�ı��ĸ߶ȣ���Ϊ�ο��߶ȣ�
	double refHeight;

	//���������еļ��б�ע���ı��������
	std::set<int> textPtIdxSet;

	bool bAssemblyAnnotation;				//�Ƿ���ƽ����ע
	std::string sAxis;						//������
	std::string eAxis;						//��ʼ��
	std::string strBeamName;				//��������
	std::string strSecSize;					//����ߴ�
	std::string strStirrup;					//����
	std::string topLongitudinalReinforce;//�����ݽ�
	std::string btmLongitudinalReinforce;//�ײ��ݽ�
	std::string waistLongitudinalReinforce;//���ݽ�

	std::string relativeElevation;//���
	Direction direction;//�Ƿ���ˮƽ����
	double widSize;//���Ŀ��

public:
	//����
	std::vector<int> colIdxVec;
};

#endif