#pragma once
#include "common.h"
#include "data.h"
#include "line.h"
#include "dimension.h"
#include "block.h"
#include "elevationSymbolSet.h"
#include <utils\beam\beam.h>

//todo
class LongitudinalBeamOutline
{
public:
	typedef enum
	{
		UNKNOWN,
		UP,
		DOWN
	}SettingType;

	LongitudinalBeamOutline() = default;

	

	int mainIdx;
	SettingType settingType;
	std::vector<int> boardIdxVec;
	RelativeDistance start; //�������ߵ���ֹˮƽλ��
	RelativeDistance end;	//�������ߵ���ֹˮƽλ��
};


class LongitudinalBoard
{
public:
	LongitudinalBoard() = default;

	Box box;				//���box

	double thickness;		//��ĺ��

	int boardTopIdx;		// �嶥������
	double ele1;			//�嶥���
	int boardBtmIdx;		//���������
	double ele2;			//��ױ��

	std::vector<Line> boardLineVec;//��ĶԽǽ�����
};



class LongitudinalFloor
{
public:
	LongitudinalFloor() :mainIdx(-1)
	{}
	bool checkBoard(const std::shared_ptr<LongitudinalBoard>& board) const;
	void appendBoardIdx(const int boardIdx, const std::shared_ptr<LongitudinalBoard>& board);

	//�����ѯ�Ľӿ�,���ض�Ӧ����������
	std::shared_ptr<LongitudinalBeamOutline> findLongitudinalBeamOutline(std::shared_ptr<Beam> beam);
	

	Box box;							//¥���box
	int mainIdx;						//��¥�����������
	std::vector<int> boardIdxVec;		//��¥������а�����
	std::vector<int> beamOutlineIdxVec;	//��¥�����������
	std::vector<std::shared_ptr<LongitudinalBeamOutline>> beamOutlines;//���е������߶���
};

class LongitudinalStructProfile
{
public:

	LongitudinalStructProfile() :
		pIdxVec(nullptr),
		pLineVec(nullptr),
		dynamicThickness(0)
	{};

	//************************************
	// ����	: GetTheValidBlock
	// ȫ��	: LongitudinalStructProfile::getTheValidBlock
	// ����	: public static 
	// ����	: int
	// ����	: blocks
	// ˵��	: ��ȡ��Ч�Ŀ�
	//************************************
	static int getTheValidBlock(const std::vector<std::shared_ptr<Block>>& blocks);


	//************************************
	// ����	: SetMFCPaintTestVec
	// ȫ��	: LongitudinalStructProfile::setMFCPaintTestVec
	// ����	: public 
	// ����	: void
	// ����	: temp_idx
	// ����	: temp_lines
	// ˵��	: ����MFC��ӡ����
	//************************************
	void setMFCPaintTestVec(std::vector<int>& temp_idx,
							std::vector<Line>& temp_lines);

	//************************************
	// ����	: SetAnalysisDependencise
	// ȫ��	: LongitudinalStructProfile::setAnalysisDependencise
	// ����	: public 
	// ����	: void
	// ����	: prerequiseData
	// ����	: _elevationSymbolSet
	// ����	: _elevationSymbols
	// ˵��	: ���ý�������������
	//************************************
	void setAnalysisDependencise(const std::shared_ptr<Data>& prerequiseData,
								 ElevationSymbolSet& _elevationSymbolSet,
								 std::vector<std::shared_ptr<ElevationSymbol>>& _elevationSymbols);


	//************************************
	// ����	: IniSideVerticalDimVec
	// ȫ��	: LongitudinalStructProfile::iniSideVerticalDimVec
	// ����	: public 
	// ����	: void
	// ����	: vAxisVec
	// ����	: dimIdxVec
	// ˵��	: ��������ı�ע��������
	//************************************
	void iniSideVerticalDimVec(const std::vector<Axis::AxisLine>& vAxisVec,
							   const  std::vector<unsigned>& dimIdxVec);

	//************************************
	// ����	: GetSideVerticalIdx
	// ȫ��	: LongitudinalStructProfile::getSideVerticalIdx
	// ����	: public 
	// ����	: std::vector<int>
	// ����	: block
	// ˵��	: ������������������Ҳ�İ�������
	//************************************
	std::vector<int> getSideVerticalIdx(const std::shared_ptr<Block>& block) const;

	//************************************
	// ����	: FindBoardLineIdx
	// ȫ��	: LongitudinalStructProfile::findBoardLineIdx
	// ����	: public 
	// ����	: bool
	// ˵��	: ����ͼ���������еİ���
	//************************************
	bool findBoardLineIdx();

	//************************************
	// ����	: FindBoardLayer
	// ȫ��	: LongitudinalStructProfile::findBoardLayer
	// ����	: public 
	// ����	: bool
	// ˵��	: ���ҵ����еİ��ߵ�ͼ��
	//************************************
	bool findBoardLayer();

	//************************************
	// ����	: InitDimLayer
	// ȫ��	: LongitudinalStructProfile::initDimLayer
	// ����	: public 
	// ����	: bool
	// ˵��	: ��ʼ���ߴ����ڵ�ͼ��
	//************************************
	bool initDimLayer();



	//************************************
	// ����	: ParseBoard
	// ȫ��	: LongitudinalStructProfile::parseBoard
	// ����	: public 
	// ����	: bool
	// ˵��	: �����õ����еİ�
	//************************************
	bool parseBoard();


	//************************************
	// ����	: BuildBoard
	// ȫ��	: LongitudinalStructProfile::buildBoard
	// ����	: public 
	// ����	: bool
	// ����	: condidateIdx
	// ����	: refLine
	// ����	: board
	// ˵��	: ������
	//************************************
	bool buildBoard(const std::vector<int>& condidateIdx,
					const Line& refLine,
					std::shared_ptr<LongitudinalBoard>& board);

	//************************************
	// ����	: ParseBeamOutline
	// ȫ��	: LongitudinalStructProfile::parseBeamOutline
	// ����	: public 
	// ����	: bool
	// ˵��	: ��������������
	//************************************
	bool parseBeamOutline();

	//************************************
	// ����	: GetMinHorizontalDist
	// ȫ��	: LongitudinalStructProfile::getMinHorizontalDist
	// ����	: public 
	// ����	: long long
	// ����	: condidateIdx
	// ����	: refLine
	// ˵��	: �����ѡ������ָ��������ľ���
	//************************************
	long long getMinVerticalDist(const std::vector<int>& condidateIdx,
								 const Line& refLine);

	//************************************
	// ����	: HorizontalCross
	// ȫ��	: LongitudinalStructProfile::bHorizontalCross
	// ����	: public 
	// ����	: bool
	// ����	: refLine
	// ����	: checkLine
	// ˵��	: ��ƽ�����Ƿ���ƽ�н�����
	//************************************
	bool bHorizontalCross(const Line& refLine, const Line& checkLine);

	//************************************
	// ����	: AdjustDynamicThickness
	// ȫ��	: LongitudinalStructProfile::adjustDynamicThickness
	// ����	: public 
	// ����	: void
	// ����	: minThickness
	// ˵��	: ���ݴ����ֵ�������������ĺ��
	//************************************
	void adjustDynamicThickness(long long minThickness);

	//************************************
	// ����	: GetBlock
	// ȫ��	: LongitudinalStructProfile::getBlock
	// ����	: public 
	// ����	: bool
	// ����	: blocks
	// ˵��	: �Ӹ������ҵ��Ϸ���Ψһ��
	//************************************
	bool getBlock(const std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// ����	: Parse
	// ȫ��	: LongitudinalStructProfile::parse
	// ����	: public 
	// ����	: bool
	// ����	: blocks
	// ˵��	: ���������������������
	//************************************
	bool parse(const std::vector<std::shared_ptr<Block>>& blocks);

	//************************************
	// ����	: GetBlockDimensions
	// ȫ��	: LongitudinalStructProfile::getBlockDimensions
	// ����	: public 
	// ����	: bool
	// ����	: _dimIdxVec
	// ����	: box
	// ˵��	: �õ��������еĳߴ��ע
	//************************************
	bool getBlockDimensions(std::vector<unsigned int>& _dimIdxVec,
							const Box& box) const;
	//************************************
	// ����	: BuildDimDesMap
	// ȫ��	: LongitudinalStructProfile::buildDimDesMap
	// ����	: public 
	// ����	: bool
	// ����	: dimIdxVec
	// ˵��	: ������ֱ��ע�ߴ��ӳ��
	//************************************
	bool buildDimDesMap(const std::vector<unsigned>& dimIdxVec);

	//************************************
	// ����	: IniDimMap
	// ȫ��	: LongitudinalStructProfile::iniDimMap
	// ����	: public 
	// ����	: bool
	// ˵��	: ������Ҫ�ĳߴ�������ӳ��
	//************************************
	bool iniDimMap();

	//************************************
	// ����	: FindBeamOutlineLayer
	// ȫ��	: LongitudinalStructProfile::findBeamOutlineLayer
	// ����	: public 
	// ����	: bool
	// ˵��	: �����������ߵ�ͼ��
	//************************************
	bool findBeamOutlineLayer();

	// todo replace with non-recursive codes
	bool traverseNonBoardConnectedLineChain(const int idx,
											bool firstLevel,
											std::map<std::string, int>& layerCounterMap,
											std::set<int>& excludedIdxSet);

	//************************************
	// ����	: FindBeamOutlineIdx
	// ȫ��	: LongitudinalStructProfile::findBeamOutlineIdx
	// ����	: public 
	// ����	: bool
	// ����	: beamOutlineIdxVec
	// ˵��	: �������е�������������
	//************************************
	bool findBeamOutlineIdx(std::vector<int>& beamOutlineIdxVec);

	bool installElevationForBoards();


	double getEle(const int& lineIdx);

	//************************************
	// ����	: ParseFloor
	// ȫ��	: LongitudinalStructProfile::parseFloor
	// ����	: private 
	// ����	: bool
	// ˵��	: �����õ�¥��
	//************************************
	bool parseFloor();

	//************************************
	// ����	: BuildFloors
	// ȫ��	: LongitudinalStructProfile::buildFloors
	// ����	: private 
	// ����	: bool
	// ����	: 
	// ����	: idxChain
	// ˵��	: ����¥��
	//************************************
	bool buildFloors(std::map<int, std::list<int>>& idxChain);

	//************************************
	// ����	: InitFloor
	// ȫ��	: LongitudinalStructProfile::initFloor
	// ����	: private 
	// ����	: void
	// ����	: flr
	// ����	: second
	// ˵��	: ��ʼ��¥��
	//************************************
	void initFloor(std::shared_ptr<LongitudinalFloor>& flr, std::list<int>& second);

	//************************************
	// ����	: AppendBoardIdxVec2Floor
	// ȫ��	: LongitudinalStructProfile::appendBoardIdxVec2Floor
	// ����	: private 
	// ����	: bool
	// ����	: floor
	// ����	: condVec
	// ˵��	: ��Ӱ�������¥��
	//************************************
	bool appendBoardIdxVec2Floor(std::shared_ptr<LongitudinalFloor>& floor, const std::list<int>& condVec);

	//************************************
	// ����	: CheckBoardIdxForFloor
	// ȫ��	: LongitudinalStructProfile::checkBoardIdxForFloor
	// ����	: private 
	// ����	: bool
	// ����	: condVec
	// ����	: floor
	// ˵��	: ����ѡ�İ������Ƿ��ڸ�¥��
	//************************************
	bool checkBoardIdxForFloor(const std::list<int>& condVec, std::shared_ptr<LongitudinalFloor>& floor);

	//************************************
	// ����	: FindBoardMainIdx
	// ȫ��	: LongitudinalStructProfile::findBoardMainIdx
	// ����	: private 
	// ����	: void
	// ����	: flrItm
	// ˵��	: ���ҵ���¥�����������
	//************************************
	void findBoardMainIdx(std::shared_ptr<LongitudinalFloor>& flrItm);

	//************************************
	// ����	: AppendBeamOutlineIdxVec2Floor
	// ȫ��	: LongitudinalStructProfile::appendBeamOutlineIdxVec2Floor
	// ����	: public 
	// ����	: void
	// ����	: beamOutlineIdxItm
	// ˵��	: ����������ߵ���Ӧ��¥����
	//************************************
	void appendBeamOutlineIdxVec2Floor(const  int beamOutlineIdxItm);


	//************************************
	// ����	: SameFloorCheck
	// ȫ��	: LongitudinalStructProfile::bSameFloorCheck
	// ����	: public 
	// ����	: bool
	// ����	: pre
	// ����	: ne
	// ˵��	: �ж��Ƿ�����������ͬһ¥��
	//************************************
	bool bSameFloorCheck(std::shared_ptr<LongitudinalBoard>& pre, std::shared_ptr<LongitudinalBoard>& ne);

	//************************************
	// ����	: FindConnectedBoardsMap
	// ȫ��	: LongitudinalStructProfile::findConnectedBoardsMap
	// ����	: public 
	// ����	: bool
	// ����	: 
	// ����	: idxChain
	// ˵��	: ����ǰ����ӵİ�ӳ��
	//************************************
	bool findConnectedBoardsMap(std::map<int, std::list<int>>& idxChain);


	//************************************
	// ����	: InstallRelativeElvationForBeam
	// ȫ��	: LongitudinalStructProfile::installRelativeElvationForBeam
	// ����	: public 
	// ����	: bool
	// ����	: beam
	// ����	: beamOutline
	// ˵��	: ������Ա�ߵ�ÿһ��
	//************************************
	bool installRelativeElvationForBeam(std::shared_ptr<Beam> beam,
										std::shared_ptr<LongitudinalBeamOutline> beamOutline);


	std::vector<std::shared_ptr<LongitudinalBoard>> longitudianlBoards;	//���е�����¥��
	std::vector<std::shared_ptr<LongitudinalFloor>> longitudinalFloors;	//���е�����¥��

private:
	std::shared_ptr<Block> block;	//���ڿ�
	std::shared_ptr<Data> data;		//����data
	std::vector<int>* pIdxVec;		// ����������
	std::vector<Line>* pLineVec;	// ������

	ElevationSymbolSet* elevationSymbolSet;
	std::vector<std::shared_ptr<ElevationSymbol>>* elevationSymbols;
private:

	std::map<long long, std::vector<DimensionPair>> vDimDesMap;			//��עyӳ��
	std::vector<unsigned> leftDimIdxVec;								//���������ı�ע����
	std::vector<unsigned> rightDimIdxVec;								//�����Ҳ���ı�ע����
	std::string dimLayer;												//�ߴ��ͼ��

	std::vector<int> beamOutlineIdxVec;				//���е���������
	std::vector<int> boardLineIdxVec;				//���а�������
	long long dynamicThickness;
	std::string boardLayer;			//������ͼ��
	std::string beamOutlineLayer;	//��������ͼ��
};