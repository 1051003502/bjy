#pragma once
#include "publicFunction.h"
#include "block.h"
#include "dimension.h"
#include "pillar.h"
#include "beam.h"

class BeamLongitudinalProfile
{
public:
	BeamLongitudinalProfile() = default;
	~BeamLongitudinalProfile() = default;

	//************************************
	// ����	: FindTextAttribute
	// ȫ��	: BeamProfile::findTextAttribute
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ˵��	: �ҵ����ߵ��ı���
	//************************************
	bool findTextAttribute(Data& data);

private:
	//************************************
	// ����	: FindText
	// ȫ��	: BeamProfile::findText
	// ����	: public 
	// ����	: bool
	// ����	: _textPtIdxVec
	// ����	: data
	// ����	: idx
	// ˵��	: �����������������ߵĳ���Ϊ�뾶���������˷ֱ�����ı��㷵��
	//************************************
	bool findText(std::vector<int>& _textPtIdxVec, Data& data, const int idx);

public:
	int tagPointIdx{};// ����λ�����ڵ��ı�������
	std::pair<int, int> profileLineIdxPair;//����λ�����ڵ���������

};

class BeamPosition;


class BeamLongitudinalRein
{
public:
	enum ReinType
	{
		UNKNOWN = 0,
		BOUNDARY,
		INNER
	};



	class ReinCompnent
	{
	public:
		enum class ReinCompnentType
		{
			Unknown,
			Left,
			Longitudinal,
			Right
		};

		ReinCompnent() = default;
		ReinCompnentType compnentType{};						//�������
		std::shared_ptr<std::vector<DimensionPair>> dims;	//��ע�ߴ�
		int lineIdx{};										//�ӹ��ߵ�����
		int reinIdx{};										//���ڼӹ̵�����
	};

	using ReinCompType = ReinCompnent::ReinCompnentType;


	BeamLongitudinalRein() = default;

	~BeamLongitudinalRein() = default;

	//************************************
	// ����	: AppendH
	// ȫ��	: BeamLongitudinalRein::appendH
	// ����	: private 
	// ����	: bool
	// ����	: line
	// ����	: idx
	// ˵��	: ������ߣ�������������չ�߽�
	//************************************
	bool updateHBound(const Line& line);


	//************************************
	// ����	: UpdateVBound
	// ȫ��	: BeamLongitudinalRein::updateVBound
	// ����	: public 
	// ����	: bool
	// ����	: line
	// ˵��	: ���¿�ĸ߶ȱ߽�
	//************************************
	bool updateVBound(const Line& line);

	//************************************
	// ����	: AddComp
	// ȫ��	: BeamLongitudinalRein::addComp
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ����	: comp
	// ˵��	: �������
	//************************************
	bool addComp(const std::shared_ptr<Data>& data,
				 std::shared_ptr<BeamLongitudinalRein::ReinCompnent>& comp);

	//************************************
	// ����	: UpdateBound
	// ȫ��	: BeamLongitudinalRein::updateBound
	// ����	: public 
	// ����	: bool
	// ����	: targetIdx
	// ˵��	: ���±߽�
	//************************************
	bool updateHBound(double l, double r);

	//************************************
	// ����	: GetBoundLength
	// ȫ��	: BeamLongitudinalRein::getBoundLength
	// ����	: public 
	// ����	: double
	// ˵��	: ��ȡ�߽�Ŀ��
	//************************************
	double longitudinalLength() const { return right - left; }


	//************************************
	// ����	: SpanHeight
	// ȫ��	: BeamLongitudinalRein::spanHeight
	// ����	: public 
	// ����	: double
	// ˵��	: ��ĸ߶ȱ߽�
	//************************************
	double spanHeight() const { return top - bottom; }


	//************************************
	// ����	: FindOuterLineIdx
	// ȫ��	: BeamLongitudinalRein::findOuterLineIdx
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ����	: block
	// ˵��	: �������������
	//************************************
	bool findOuterLineIdx(std::shared_ptr<Data>& data,
						  std::shared_ptr<Block>& block,
						  std::vector<Line>* pTestLineVec);

	//************************************
	// ����	: ExistOtherComponent
	// ȫ��	: BeamLongitudinalRein::bExistOtherComponent
	// ����	: public 
	// ����	: bool
	// ˵��	: �Ƿ��������component
	//************************************
	bool bExistOtherComponent() const {
		for (const auto& com : this->components)
		{
			if (com->compnentType == ReinCompType::Left ||
				com->compnentType == ReinCompType::Right)
				return  true;
		}
		return false;
	}


public:
	std::vector<std::shared_ptr<ReinCompnent>> components;	//�ӹ̵����
	int outerIdx{};											// ���Χ����������������componentʱ��
	BeamPosZDelta leftZDelta{};								//������ӹ̣�����ڿ��ܴ���Z���������
	BeamPosZDelta rightZDelta{};
	double left{};								//ˮƽ�ӹ���߽�
	double right{};								//ˮƽ�ӹ��ұ߽�
	double top{};								//�ӹ̵��ϱ߽�
	double bottom{};							//�ӹ̵��±߽�
	int next{};									//��һ�����Ӽӹ�
	int pre{};									//��һ�����Ӽӹ�
	enum ReinType reinType {};					//�ӹ̵�����
	std::vector<std::pair<DimensionPair, double>> truncatedLenDimDes;//�ضϱ�ע
};



class BeamLongitudinal
{
public:
	BeamLongitudinal() = default;
	~BeamLongitudinal() = default;

public:


	//************************************
	// ����	: ReturnBeamLongitudinal
	// ȫ��	: BeamLongitudinal::returnBeamLongitudinal
	// ����	: public static 
	// ����	: bool
	// ����	: _block
	// ����	: _strReinLayer
	// ����	: data
	// ����	: pTestVec
	// ˵��	: ���������������Ϣ�Ŀ�
	//************************************
	bool parseBlock(std::shared_ptr<Block> _spBlock,
					std::string& _strReinLayer,
					std::shared_ptr<Data> data,
					std::vector<unsigned int>& _blockDimIdxVec,
					std::vector<int>& pTestVec,
					std::vector<Line>& pTestLineVec);


	//************************************
	// ����	: InitReqiuredInfo
	// ȫ��	: BeamLongitudinal::initReqiuredInfo
	// ����	: public 
	// ����	: bool
	// ����	: _block
	// ����	: _strReinLayer
	// ����	: pTestVec
	// ����	: pTestLinesVec
	// ˵��	: ��ʼ����������������
	//************************************
	bool initReqiuredInfo(std::shared_ptr<Block> _block,
						  std::string& _strReinLayer,
						  std::vector<int>& pTestVec,
						  std::vector<Line>& pTestLinesVec);


	//************************************
	// ����	: InitDimMap
	// ȫ��	: BeamLongitudinal::initDimMap
	// ����	: public 
	// ����	: bool
	// ����	: _blockDimensionVec
	// ˵��	: ��ʼ��������ı�ע��Ϣӳ��
	//************************************
	void initDimMap(std::vector<unsigned int>& _blockDimIdxVec,
					std::shared_ptr<Data> data);



	//************************************
	// ����	: ParseRein
	// ȫ��	: BeamLongitudinal::parseRein
	// ����	: public 
	// ����	: bool
	// ����	: _hLineIdxVec
	// ����	: _vLineIdxVec
	// ����	: data
	// ˵��	: �ҵ����еļӹ���
	//************************************
	bool findRein(std::vector<int>& _hLineIdxVec,
				  const std::shared_ptr<Data>& data);

	//************************************
	// ����	: ParseRein
	// ȫ��	: BeamLongitudinal::parseRein
	// ����	: public 
	// ����	: bool
	// ����	: hBound
	// ����	: vBound
	// ����	: data
	// ˵��	: �����ӹ���
	//************************************
	bool parseRein(std::vector<int>& _hLineIdxVec,
				   std::shared_ptr<Data>& data);

	//************************************
	// ����	: ParseZData
	// ȫ��	: BeamLongitudinal::parseZData
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ����	: rein
	// ˵��	: ��ȡZ������
	//************************************
	bool parseZData(const std::shared_ptr<Data>& data,
					std::shared_ptr<BeamLongitudinalRein>& rein);



	//************************************
	// ����	: ReturnSRtreeDimIndex
	// ȫ��	: BeamLongitudinal::returnSRtreeDimIndex
	// ����	: public 
	// ����	: std::vector<int>
	// ����	: min
	// ����	: max
	// ˵��	: ������������б�ı�ע
	//************************************
	std::vector<int> returnSRtreeDimIndex(const double min[], const double max[]);

	//************************************
	// ����	: FindHDimension
	// ȫ��	: BeamLongitudinal::findHDimension
	// ����	: public 
	// ����	: bool
	// ����	: _spDimension
	// ����	: tick
	// ����	: incrementDirection
	// ����	: data
	// ˵��	: �Ը߶Ȳ�ͷ��������ߴ��ע
	//************************************
	bool findHVDimension(std::vector<DimensionPair>& _dimPairVec,
						 const Point& tick,
						 bool bIncrement,
						 bool bHorizontal,
						 const std::shared_ptr<Data>& data);


	//************************************
	// ����	: FindTruncatedDimPair
	// ȫ��	: BeamLongitudinal::findTruncatedDimPair
	// ����	: public 
	// ����	: bool
	// ����	: rein
	// ����	: dimPairVec
	// ����	: accessPtF
	// ����	: accessCheckPtF
	// ����	: data
	// ˵��	: ������ע���뵽�ӹ�
	//************************************
	bool findTruncatedDimPair(std::shared_ptr<BeamLongitudinalRein> rein,
							  std::vector<DimensionPair>& dimPairVec,
							  const std::shared_ptr<Data>& data);



	//************************************
	// ����	: FindProfile
	// ȫ��	: BeamLongitudinal::findProfile
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ˵��	: �������е�����
	//************************************
	bool findProfile(Data& data);



	//************************************
	// ����	: InitFirstSupportRef
	// ȫ��	: BeamLongitudinal::initFirstSupportRef
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ˵��	: ��ʼ��������һ����֧�ŵĲο���ע
	//************************************
	bool initFirstSupportRef(std::shared_ptr<Data> data);

	//************************************
	// ����	: FindFrameInfo
	// ȫ��	: BeamLongitudinal::findFrameInfo
	// ����	: public 
	// ����	: bool
	// ����	: spBeam
	// ����	: data
	// ����	: _pillarVec
	// ˵��	: �����������Ϣ,��װ��spBeam��
	//************************************
	bool installFrameInfo(std::shared_ptr<Beam> spBeam,
						  std::shared_ptr<Data> data,
						  std::vector<std::shared_ptr<Pillar>>& _pillarVec);

	//************************************
	// ����	: InstallZDeltaInfoo
	// ȫ��	: BeamLongitudinal::installZDeltaInfoo
	// ����	: public 
	// ����	: void
	// ����	: reinItm
	// ����	: data
	// ����	: col1
	// ����	: col2
	// ����	: col1RightInFrame
	// ����	: col2LeftInFrame
	// ����	: spanItm
	// ˵��	: �������·����ݣ���װ��spBeam�и�����
	//************************************
	void installZDeltaInfoo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
							std::shared_ptr<Data> data,
							const std::shared_ptr<Pillar>& col1,
							const std::shared_ptr<Pillar>& col2,
							double col1RightInFrame,
							double col2LeftInFrame,
							std::shared_ptr<BeamSpan>& spanItm);


	//************************************
	// ����	: InstallAnchorageInfo
	// ȫ��	: BeamLongitudinal::installAnchorageInfo
	// ����	: public 
	// ����	: void
	// ����	: reinItm
	// ����	: data
	// ����	: col1RightInFrame
	// ����	: col2LeftInFrame
	// ����	: col1
	// ����	: col2
	// ����	: spanItm
	// ˵��	: ��װê�̳���
	//************************************
	void installAnchorageInfo(const std::shared_ptr<BeamLongitudinalRein>& reinItm,
							  std::shared_ptr<Data> data,
							  double col1RightInFrame,
							  double col2LeftInFrame,
							  const std::shared_ptr<Pillar>& col1,
							  const std::shared_ptr<Pillar>& col2,
							  std::shared_ptr<BeamSpan>& spanItm);


	//************************************
	// ����	: GetDimInfo
	// ȫ��	: BeamLongitudinal::getDimInfo
	// ����	: public 
	// ����	: bool
	// ����	: dim
	// ˵��	: ��ȡ��ע����Ϣ
	//************************************
	std::string getDimInfo(const std::shared_ptr<Dimension>& dim);

	//************************************
	// ����	: InstallProfileInfo
	// ȫ��	: BeamLongitudinal::installProfileInfo
	// ����	: public 
	// ����	: bool
	// ����	: data
	// ����	: col1RightInFrame
	// ����	: col2RightInFrame
	// ����	: spanItm
	// ˵��	: Ϊָ���Ŀ�ͷ�Χ�������ߣ�����װ
	//************************************
	bool installProfileInfo(std::shared_ptr<Data> data,
							double col1RightInFrame,
							double col2RightInFrame,
							std::shared_ptr<BeamSpan>& spanItm);



	//************************************
	// ����	: InstallTruncatedInfo
	// ȫ��	: BeamLongitudinal::installTruncatedInfo
	// ˵��	: ��װ�ضϳ��ȵ���
	//************************************
	void installTruncatedInfo(const std::shared_ptr<BeamLongitudinalRein>& rein,
							  const std::shared_ptr<Data>& data,
							  const double col1RightInFrame,
							  const double col2LeftInFrame,
							  std::shared_ptr<BeamSpan>& spanItm);



	std::shared_ptr<Block> spBlock;   //��ǰ�Ŀ�
	// ! <α���,<�ߴ��ע,��ע�Ƿ���������>>
	std::map<long long, std::vector<DimensionPair>> hDimDesMap;	//��עxӳ��
	std::map<long long, std::vector<DimensionPair>> vDimDesMap;	//��עyӳ��
	std::map<long long, std::vector<unsigned int>> lenDimDesMap;//��ע����ӳ��

	RTreeData sDimRTree;//б��עRtree
	std::vector<int> reinLineIdxVec;			//���еļӹ��ߵ�����
	std::map<int, int> reinLineIdxToReinIdxMap;	//ÿ���ӹ��߶�Ӧ�ļӹ�����
	std::string strReinLayer;					//����ͼ��
	std::string strTagName;						//��ʶ�������
	std::shared_ptr<Dimension> supportDim;		//��������������ұ߽�ı�ע 

	std::vector<std::shared_ptr<BeamLongitudinalProfile>> longitudinalProfileVec;	//���е�����
	std::vector<std::shared_ptr<BeamLongitudinalRein>> spReinVec;					//���мӹ�

	//����
	std::vector<int>* pTestIdxVec{};
	std::vector<Line>* pTestLineVec{};
};
