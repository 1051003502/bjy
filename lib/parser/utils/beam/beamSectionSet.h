#ifndef __PARSER_UTILS_SET1_BEAMSECTIONSET_H__
#define __PARSER_UTILS_SET1_BEAMSECTIONSET_H__
#include<iostream>
#include"block.h"
#include"beamSection.h"

#include<vector>

class BeamSectionSet
{
public:
	
	BeamSectionSet() = default;

	//************************************
	// ����	: iniBlockNameVec
	// ȫ��	: BeamSectionSet::iniBlockNameVec
	// ����	: public 
	// ����	: bool
	// ����	: _blockNmaeVec
	//TODO:�ڶ�ͼֽ��ϵ�ʱ����õ���Ŀǰֻ��һ���ӿڷ���������
	// ˵��	: ��ʼ�������棬ͼ������ִӶ�����ͼ�ڵ����ж��沢������
	//************************************
	bool findBeamSectionBlock(const std::vector<std::string>&_blockNmaeVec);

	//************************************
	// ����	: setMember
	// ȫ��	: BeamSectionSet::setMember
	// ����	: public 
	// ����	: bool
	// ����	: _spBlocksVec
	// ˵��	: ��ʼ������ͼֽ�ڵ����п���Ϣ
	//************************************
	bool setMember(const std::vector<std::shared_ptr<Block>>& _spBlocksVec,
		const std::vector<std::shared_ptr<Dimension>> &_spDimensions);

	//************************************
	// ����	: returnBeamSection
	// ȫ��	: BeamSectionSet::returnBeamSection
	// ����	: public 
	// ����	: std::vector<BeamSection>
	// ˵��	: �˺������ڷ��ؽ�����ģ���������Ϣ�����Ժ�ͳ��Ϣʹ�õĽӿ�
	//************************************
	std::vector<BeamSection> beamSections(Data &data, std::vector<std::shared_ptr<BeamSection>>&_beamSections);

private:

	//************************************
	// ����	: iniGoalBlockVec
	// ȫ��	: BeamSectionSet::iniGoalBlockVec
	// ����	: private 
	// ����	: bool
	// ����	: spBlocksVec
	// ˵��	: ��ʱ�����ã�����Ѱ���������ͼ�飬������ɾ��
	//************************************
	bool iniGoalBlockVec(std::vector<bool>&mapVec, Data&data);
public:
	//************************************
	// ����	: bBeamSectionBlock
	// ȫ��	: BeamSectionSet::bBeamSectionBlock
	// ����	: private 
	// ����	: bool
	// ����	: _block
	// ˵��	: �����жϴ˿��Ƿ�Ϊ������
	bool bBeamSectionBlock(const std::shared_ptr<Block>& _block,Data&data);



public:
	std::vector<std::shared_ptr<BeamSection>>spBeamSectionVec;//!����������������Ϣ
	std::vector<std::string>blockNameVec;//!Ŀ������֣�������������� �˴����ܲ���Ҫ�����Ż�
	std::vector<std::shared_ptr<Dimension>> spDimensions;//!���б�ע��Ϣ
private:
	std::vector<std::shared_ptr<Block>> spBlocksVec;//!����ͼֽ�ڵĿ���Ϣ
	std::vector<Block>goalBlockVec;//!ȷ����ǰͼֽ��ɸѡ�����Ŀ���Ϣ






};
#endif // !1