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
	//typedef enum _WallStructType	//用来体现带有轴的块属于哪一类
	//{
	//	PLAN,//平面
	//	HPROFILE,//横剖
	//	VPROFILE//纵剖
	//}WallStructType;
private://process funcation
	//校验此块是否满足提取信息的基本条件
	bool isPlanBlock(const std::shared_ptr<Block>& _block);
	
	//初始化基础数据
	bool setBaseData(const std::shared_ptr<Data>& spData,const std::shared_ptr<Block>& _block, std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec);
	
	//查找墙线图层
	bool findWallLayer();
	
	//获得纵向轴线索引
	std::vector<int> returnAxisLineVIndex(/*LineData& lineData*/);
	
	//根据轴线的相交情况得到图层的map映射字典
	bool findCornerLineLayerMap(std::map<std::string, int>& layerWeight, const std::vector<int>& axisIndexVec);
	
	//根据已经获得的图层权重算出真正的墙图层
	std::string returnWallLayer(std::map<std::string, int>& layerWeight);
	
	//获得墙线索引
	bool findWallIndex();
	
	//用已知线条构建墙
	bool buildWall();
	
	void cleanRepeat();
	// 说明	: 找一根线的平行且最近的线 用于找墙的配对线
	int findNearestLine(int lineIndex, std::set<int>& hasDeal);//z

	//墙的两根线向一端延申 寻找新的墙线
	void extendWallLine(int lineIndex1, int lineIndex2, 
		std::set<int>& hasDealWallLineSet, 
		std::vector<std::vector<int> >& wallLineVec2);

	// 说明	: 墙的两根配对线生成一个墙
	bool twoLineBuildWall(int lineIndex1, int lineIndex2);//z 
	
	//设置墙边界,由两个边界生成墙的矩形边界
	std::vector<Line>setBorderLine(const Line& line1, const Line& line2,
		Point& startPoint, Point& endpoint, double& thick);
	
	

	//根据已知边生成矩形，即墙的矩形包围盒
	std::vector<Line>creatRectangular(const Line& lien1, const Line& line2, 
		Point& startPoint, Point& endpoint, double& thick, const char& ch = 'H');

	//获取某墙端部中点的参照轴
	std::vector<std::pair<std::string, double>>findPointReferenceAxis(const Point&point);//会产生两个断点参照轴信息，第一个为横，第二个为纵


	//int getTwoLineDistance(Line& l1, Line& l2);

	
	
private:
	//WallStructType _structTpye;//结构图纸类型
	std::shared_ptr<Data> _spData;//基础数据
	std::shared_ptr<Block> _spBlock;//块
	std::vector<std::shared_ptr<SectionSymbolGroup>> _ssgVec;//剖切线数据
	std::string _wallLayer;//墙线图层
	std::vector<Axis::AxisLine> _blockAxisVLines;
	std::vector<Axis::AxisLine> _blockAxisHLines;
	std::shared_ptr<ReferencePointGenerator> _spRefG;
	std::vector<int> _wallLineIndex;//z墙体结构线索引
	std::vector<int> _wallVLineIndex;//z墙体垂直线索引 索引低->高  线左->右
	std::vector<int> _wallHLineIndex;//z墙体水平线索引 索引低->高  线低->高
	std::vector<int> _wallSLineIndex;//斜线
private:
	std::vector<std::shared_ptr<WallLocation> > _spWallLocVec;//z 墙vector
	friend class WallLocationSetAdapter;
};
//封装WallLocationSet的接口
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
		//查找墙图层
		wallLocationSet.findWallLayer();
		return wallLocationSet._wallLayer;
	}
	std::vector<int> getHLineIndices(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec;
		
		wallLocationSet.setBaseData(spData, spBlock, ssgVec);
		//查找墙图层
		wallLocationSet.findWallLayer();
		wallLocationSet.findWallIndex();
		return wallLocationSet._wallHLineIndex;
	}
	std::vector<int> getHAndSLineIndices(std::shared_ptr<Data> spData, std::shared_ptr<Block> spBlock)
	{
		std::vector<std::shared_ptr<SectionSymbolGroup>> ssgVec;

		wallLocationSet.setBaseData(spData, spBlock, ssgVec);
		//查找墙图层
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