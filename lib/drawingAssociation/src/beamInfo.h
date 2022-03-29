#ifndef __DRAWINGRELATED_BEAMINFO_H__
#define __DRAWINGRELATED_BEAMINFO_H__
#include "beamSectionSet.h"
#include "beamSet.h"
#include "pillarSet.h"
#include "columnInfo.h"
#include "beamLongitudinalSet.h"

class BeamSectionInfo
{
public:
	BeamSectionInfo() {}

	//!�ֱ��������������������ҵĹ�������
	typedef enum _LT//longitudinal tendons �ݽ����д
	{
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	}LT;
public:
	double _seale;//!�����������Ϣ
	double _width, _height;//!���
	std::vector<std::string> _singleStirrupInfoVec;//!��ֻ��
	std::vector<std::string> _doubleStirrupInfoVec;//!˫֧��
	std::vector<std::vector<Line>>_singleStirrupVec;
	std::vector<std::vector<Line>>_doubleStirrupVec;
	//std::vector<std::pair<_LT, std::vector<std::string>>>_lonTen;//�����ݽ������
	std::map<LT, std::vector<std::string>>_lonTen;//�����ݽ������
	Point referencePoint;//!����Ĳο���
	int _hsize, _vSize;//!�ᣬ��֧��
	std::string name;
	std::string style;//�����񣬾��λ�������

};

class BeamInfo
{
public:
	//************************************
	// ����	: iniBeamLoc
	// ȫ��	: BeamInfo::iniBeamLoc
	// ����	: public 
	// ����	: bool
	// ����	: std::map<std::string
	// ����	: _axisMap  ����ƽ��ͼ
	// ����	: _dataMap
	// ˵��	: ��ʼ������λ����Ϣ
	//************************************
	bool iniAbBeamLoc(const std::map<std::string, std::shared_ptr<Axis>>& _axisMap,
					  std::map<std::string, std::shared_ptr<Data>>& _dataMap);

	//************************************
	// ����	: iniBAssemblyAnnotation
	// ȫ��	: BeamInfo::iniBAssemblyAnnotation
	// ����	: public 
	// ����	: bool
	// ����	: _bAA
	// ˵��	: ��ʼ������ͼֽ���Ƿ�Ϊƽ����ʾ
	//************************************
	bool iniBAssemblyAnnotation(const bool& _bAA = false);



	//************************************
	// ����	: iniAaBeam
	// ȫ��	: BeamInfo::iniAaBeam
	// ����	: public 
	// ����	: bool
	// ����	: _BeamSectionMap
	// ˵��	: ƽ����ʽ��ʼ������Ϣ
	//************************************
	bool iniAaBeam(const std::map<std::string, std::vector<std::shared_ptr<Beam>>>& _BeamMap);

	//************************************
	// ����	: findBeamName
	// ȫ��	: BeamInfo::findBeamName
	// ����	: public 
	// ����	: bool
	// ˵��	: ��ʼ����ͬͼֽ�ڵ���
	//************************************
	bool findBeamName();

	//************************************
	// ����	: bBeamSectionBlock
	// ȫ��	: BeamInfo::bBeamSectionBlock
	// ����	: public 
	// ����	: bool
	// ����	: _block
	// ˵��	: �жϵ�ǰ���������滹�����ݽ�ͼ
	//************************************
	bool bBeamSectionBlock(const std::shared_ptr<Block>& _block);

	//************************************
	// ����	: findFrameBeamBlock
	// ȫ��	: BeamInfo::findFrameBeamBlock
	// ����	: public 
	// ����	: bool
	// ����	: _dataMap
	// ����	: _blocks
	// ����	: _blocksMap
	// ����	: _blockFileMap
	// ˵��	: ������ؿ����
	//************************************
	bool findFrameBeamBlock(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							const std::vector<std::shared_ptr<Block>>& _blocks,
							const std::map<std::string, int>& _blocksMap,
							const std::map<std::string, std::vector<int>>& _blockFileMap);

	//!�����ݽ���н���
	bool parserFrameBeam(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
						 const std::vector<std::shared_ptr<Block>>& _blocks,
						 const std::map<std::string, int>& _blocksMap,
						 const std::map<std::string, std::vector<int>>& _blockFileMap);

	

	//************************************
	// ����	: integrateBeamSectionName
	// ȫ��	: BeamInfo::integrateBeamSectionName
	// ����	: public 
	// ����	: bool
	// ˵��	: ÿ�������ж�����棬��ô������֮�����ǵĽ����п�����ͬ�п��ܲ�ͬ����˹����ҳ����в�ͬ�Ľ������������ƥ�����ƥ��
	bool integrateBeamSectionName();

	//************************************
	// ����	: findBeamSpanSection
	// ȫ��	: BeamInfo::findBeamSpanSection
	// ����	: public 
	// ����	: bool
	// ����	: _dataMap
	// ����	: _blocks
	// ����	: _blocksMap
	// ����	: _blockFileMap
	// ˵��	: ����ÿ�����ؽ���
	bool findBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							 const std::vector<std::shared_ptr<Block>>& _blocks,
							 const std::map<std::string, int>& _blocksMap,
							 const std::map<std::string, std::vector<int>>& _blockFileMap);
	//************************************
	// ����	: iniBeamSpanSection
	// ȫ��	: BeamInfo::iniBeamSpanSection
	// ����	: public 
	// ����	: bool
	// ˵��	: �����������н���
	//************************************
	bool iniBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							const std::vector<std::shared_ptr<Block>>& _blocks, const std::map<std::string, int>& _blocksMap,
							const std::map<std::string, std::vector<int>>& _blockFileMap);

	//************************************
	// ����	: supplementaryBeamSectionInfo
	// ȫ��	: BeamInfo::supplementaryBeamSectionInfo
	// ����	: public 
	// ����	: bool
	// ˵��	: ���Ѿ������õ���������Ϣ����������Ľ�����Ϣ��
	bool supplementaryBeamSectionInfo();
	
	//************************************
	// ����	: returnBeamSectionInfo
	// ȫ��	: BeamInfo::returnBeamSectionInfo
	// ����	: public 
	// ����	: bool
	// ����	: _beamSectionInfo ��Ҫ���������Ϣ
	// ˵��	: ������������������Ϣ��������Ȼ���ʽ�����
	BeamSectionInfo returnBeamSectionInfo(const std::shared_ptr<BeamSection>& _beamSectionInfo);
	//************************************
	// ����	: outputBeamSection
	// ȫ��	: BeamInfo::outputBeamSection
	// ����	: public 
	// ����	: bool
	// ˵��	: ����������Ϣ���Ϊjson�ļ�
	Json::Value outputBeamSection(const std::shared_ptr<BeamSection>& _beamSection);

	//************************************
	// ����	: outputBeams
	// ȫ��	: BeamInfo::outputBeams
	// ����	: public 
	// ����	: bool
	// ˵��	: ��Json��ʽ�ļ����
	//************************************
	bool outputBeams();

	//************************************
	// ����	: ClearBeamVec
	// ȫ��	: BeamInfo::clearBeamVec
	// ����	: public 
	// ����	: bool
	// ˵��	: ���BeamVec
	//************************************
	bool clearBeamVec();

	//************************************
	// ����	: SerializeAaBeamInfo
	// ȫ��	: BeamInfo::serializeAaBeamInfo
	// ����	: public 
	// ����	: bool
	// ˵��	: ���ƽ������Ϣ
	//************************************
	bool serializeAaBeamInfo();

	//************************************
	// ����	: serializeElevationProfileBeamInfo
	// ȫ��	: BeamInfo::serializeElevationProfileBeamInfo
	// ����	: public 
	// ����	: bool
	// ˵��	: ���������������Ϣ
	//************************************
	bool serializeElevationProfileBeamInfo();

	/*���Գ������ڲ�����������Ϣ����*/
	bool testBeamSectionParser(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							   const std::vector<std::shared_ptr<Block>>& _blocks, const std::map<std::string, int>& _blocksMap,
							   const std::map<std::string, std::vector<int>>& _blockFileMap);

public:
	bool bAssemblyAnnotation;//�ж����Ƿ�Ϊƽ����ע
	std::map<std::string,std::vector<std::shared_ptr<Beam>>> spBeamVec;
	ColumnInfo _columnInfo;//!��������Ϣ
private:
	BeamSectionSet _beamSectionSet;                                               //!������ʱ��ʹ��
	BeamLongitudinalSet _beamLongitudinalSet;
	std::map<std::string, std::shared_ptr<BeamSection>> _beamSectionMap;            //���������������Ϣ��ӳ��
	std::map<std::string, std::vector<std::shared_ptr<Beam>>> _drawing2BeamVecMap; //��ͼֽ����Ӧ����λ��
	std::vector<std::string> _beamNameVec;                                         //����
	std::map<std::string, std::vector<std::string>> _beamNameVecMap;               //��¼��ͬͼ�ڵ�����
	std::vector<std::shared_ptr<BeamSection>> _spBeamSectionVec;                   //����������

	std::vector<std::pair<std::shared_ptr<Block>, std::shared_ptr<Data>>> _BeamFrameBlockVec;   //����ͨ�������Ϣ
	std::vector<std::pair<std::shared_ptr<Block>, std::shared_ptr<Data>>> _beamSectionBlockVec;//���������Ϣ
	std::vector<std::string> _beamSectionNameVec;                                //����������
	std::map<std::string, std::map<std::string, std::vector<std::string>>>_beamSpanSectionInfoMap;//ĳͼֽ�У�ĳ���Ľ���

public:
	//����
	std::vector<int> temp_index;
	std::vector<Line> temp_line;
	std::vector<Point> _corners;
	std::vector<std::vector<int>> m_temp_index;
};
#endif
