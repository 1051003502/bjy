#ifndef __PARSER_UTILS_WALLLOCATIONSET_H__
#define __PARSER_UTILS_WALLLOCATIONSET_H__
#include "block.h"
#include "wallLocation.h"

class WallLocationSet
{
public://interface funcation
	
	std::vector<std::shared_ptr<WallLocation>>returnWallLocation(const std::shared_ptr<Data>& spData, const std::shared_ptr<Block>& _block
		, std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec);
	void iniDrawing(std::vector<std::vector<int> >* temp_index_colorBlocks,
		std::vector<std::vector<Line> >* temp_line_colorBlocks) {
		this->pColorLineIndexVec = temp_index_colorBlocks;
		this->pColorLineVec = temp_line_colorBlocks;
	}
	void drawWall()
	{
		for (auto spWallLoc : _spWallLocVec)
		{
			pColorLineVec->push_back(spWallLoc->getDrawingLineVec());
		}
	}
	void printWallLocationData()
	{
		std::fstream of("wallLocation.json", std::ios::out);
		Json::FastWriter fastWriter;
		Json::Value body;
		for (auto wallLoc : _spWallLocVec)
		{
			body.append(wallLoc->getWallLocationJsonData());
		}
		
		of << fastWriter.write(body);
		of.close();
	}
	std::vector<std::vector<int> >* pColorLineIndexVec;
	std::vector<std::vector<Line> >* pColorLineVec;
private:
	//typedef enum _WallStructType	//�������ִ�����Ŀ�������һ��
	//{
	//	PLAN,//ƽ��
	//	HPROFILE,//����
	//	VPROFILE//����
	//}WallStructType;
private://process funcation
	//У��˿��Ƿ�������ȡ��Ϣ�Ļ�������
	bool isPlanBlock(const std::shared_ptr<Block>& _block);
	
	//��ʼ����������
	bool setBaseData(const std::shared_ptr<Data>& spData,const std::shared_ptr<Block>& _block, std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec);
	
	//����ǽ��ͼ��
	bool findWallLayer();
	
	//���������������
	std::vector<int> returnAxisLineVIndex(/*LineData& lineData*/);
	
	//�������ߵ��ཻ����õ�ͼ���mapӳ���ֵ�
	bool findCornerLineLayerMap(std::map<std::string, int>& layerWeight, const std::vector<int>& axisIndexVec);
	
	//�����Ѿ���õ�ͼ��Ȩ�����������ǽͼ��
	std::string returnWallLayer(std::map<std::string, int>& layerWeight);
	
	//���ǽ������
	bool findWallIndex();
	
	//����֪��������ǽ
	bool buildWall();
	
	void cleanRepeat();
	// ˵��	: ��һ���ߵ�ƽ����������� ������ǽ�������
	int findNearestLine(int lineIndex, std::set<int>& hasDeal);//z

	//ǽ����������һ������ Ѱ���µ�ǽ��
	void extendWallLine(int lineIndex1, int lineIndex2, 
		std::set<int>& hasDealWallLineSet, 
		std::vector<std::vector<int> >& wallLineVec2);

	// ˵��	: ǽ���������������һ��ǽ
	bool twoLineBuildWall(int lineIndex1, int lineIndex2);//z 
	
	//����ǽ�߽�,�������߽�����ǽ�ľ��α߽�
	std::vector<Line>setBorderLine(const Line& line1, const Line& line2,
		Point& startPoint, Point& endpoint, double& thick);
	
	

	//������֪�����ɾ��Σ���ǽ�ľ��ΰ�Χ��
	std::vector<Line>creatRectangular(const Line& lien1, const Line& line2, 
		Point& startPoint, Point& endpoint, double& thick, const char& ch = 'H');

	//��ȡĳǽ�˲��е�Ĳ�����
	std::vector<std::pair<std::string, double>>findPointReferenceAxis(const Point&point);//����������ϵ��������Ϣ����һ��Ϊ�ᣬ�ڶ���Ϊ��


	//int getTwoLineDistance(Line& l1, Line& l2);

	
	
private:
	//WallStructType _structTpye;//�ṹͼֽ����
	std::shared_ptr<Data> _spData;//��������
	std::shared_ptr<Block> _spBlock;//��
	std::vector<std::shared_ptr<SectionSymbolGroup>> _ssgVec;//����������
	std::string _wallLayer;//ǽ��ͼ��
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<ReferencePointGenerator> _spRefG;
	std::vector<int> _wallLineIndex;//zǽ��ṹ������
	std::vector<int> _wallVLineIndex;//zǽ�崹ֱ������ ������->��  ����->��
	std::vector<int> _wallHLineIndex;//zǽ��ˮƽ������ ������->��  �ߵ�->��
	std::vector<int> _wallSLineIndex;//б��
private:
	std::vector<std::shared_ptr<WallLocation> > _spWallLocVec;//z ǽvector
	friend class WallLocationSetAdapter;
};
//��װWallLocationSet�Ľӿ�
class WallLocationSetAdapter
{
private:
	//std::shared_ptr<Data> _spData;
	//std::shared_ptr<Block> _spBlock;
	WallLocationSet wallLocationSet;
public:
	/*void setReference(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		this->_spData = spData;
		this->_spBlock = spBlock;
	}*/
	std::string findWallLayer(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		//setReference(spData, spBlock);
		std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec;
		WallLocationSet wallLocationSet;
		wallLocationSet.setBaseData(spData, spBlock, ssgVec);
		//����ǽͼ��
		wallLocationSet.findWallLayer();
		return wallLocationSet._wallLayer;
	}
	std::vector<int> getHLineIndices(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec;
		
		wallLocationSet.setBaseData(spData, spBlock, ssgVec);
		//����ǽͼ��
		wallLocationSet.findWallLayer();
		wallLocationSet.findWallIndex();
		return wallLocationSet._wallHLineIndex;
	}
	std::vector<int> getHAndSLineIndices(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec;

		wallLocationSet.setBaseData(spData, spBlock, ssgVec);
		//����ǽͼ��
		wallLocationSet.findWallLayer();
		wallLocationSet.findWallIndex();
		std::vector<int> hAndS;
		hAndS.insert(hAndS.end(), wallLocationSet._wallHLineIndex.begin(), wallLocationSet._wallHLineIndex.end());
		hAndS.insert(hAndS.end(), wallLocationSet._wallSLineIndex .begin(), wallLocationSet._wallSLineIndex.end());
		return hAndS;
	}
	std::vector<int> getVLineIndicesPassingly()
	{
		return wallLocationSet._wallVLineIndex;
	}
	std::string getWallLayer()
	{
		return wallLocationSet._wallLayer;
	}
};
#endif