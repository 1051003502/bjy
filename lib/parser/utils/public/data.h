#ifndef __PARSER_UTILS_DATA_H__
#define __PARSER_UTILS_DATA_H__
#include <iostream>
#include "line.h"
#include "corner.h"
#include "circle.h"
#include "dimension.h"
#include "RTree.h"
#include "kdt.h"
#include <queue>
#include "axis.h"


typedef RTree<int, double, 2, float> RLine;
using compRef = bool (*)(const Point&, const Point&);



class LineData
{
public:
	using Ref = bool (*)(const std::vector<Line>&, int, int);
	enum cmLINEDATAMODE
	{
		LINEDATAMODE_LINES,  /* lines */
		LINEDATAMODE_VINDEX, /* vertical index */
		LINEDATAMODE_HINDEX, /* horizontal index */
		LINEDATAMODE_SINDEX  /* slash index */
	};

	bool clearLineData();
	bool pushBackLines(const Line& line);
	bool pushBackVLinesIndices(const int& index);
	bool pushBackHLinesIndices(const int& index);
	bool pushBackSLinesIndices(const int& index);
	bool setLines(const std::vector<Line>& line);
	bool setVLinesIndex(const std::vector<int>& vIndex);
	bool setHLinesIndex(const std::vector<int>& hIndex);
	bool setSLinesIndex(const std::vector<int>& sIndex);
	/*set lines start and end*/
	bool setLines(const int& index, const Point& start, const Point& end);

	/* get the size of vars */
	int getLinesSize();
	int getVLinesSize();
	int getHLinesSize();
	int getSLinesSize();

	Line getLine(const int& index);
	int getVLineIndex(const int& index);
	int getHLineIndex(const int& index);
	int getSLineIndex(const int& index);

	/* access to vars */
	std::vector<Line>& lines();
	const std::vector<int>& vLinesIndices();
	const std::vector<int>& hLinesIndices();
	const std::vector<int>& sLinesIndices();

	void sortLinesByBindRef(enum cmLINEDATAMODE mode, LineData::Ref ref);

	/* find the min and the max boundary */
	int findLowerBound(enum cmLINEDATAMODE mode, double val, double precision = Precision, bool h = true);
	int findUpperBound(enum cmLINEDATAMODE mode, double val, double precision = Precision, bool h = true);

	/* lines sort */
	bool linesSort(std::vector<Line>& lines, enum cmLINEDATAMODE mode);
	bool quickSortLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode);
	int divisionLine(std::vector<Line>& lines, int left, int right, enum cmLINEDATAMODE mode);

	/* lines sort index */
	bool linesSortIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode);
	bool quickSortLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode);
	int divisionLineIndex(std::vector<int>& lines, int left, int right, enum cmLINEDATAMODE mode);

	/* merge lines */
	//һ�ϳ���Ϊ׼���ϲ�ͬһλ�ô�����
	bool mergeLines(std::vector<Line>& lines, enum cmLINEDATAMODE mode); //�ϲ�����
	/* merge lines index */
	bool mergeLinesIndex(std::vector<int>& lines, enum cmLINEDATAMODE mode);
	//���ܺ���
	bool lineTypeVHS(const std::vector<int>& index, std::vector<int>& v_lines,
					 std::vector<int>& h_lines, std::vector<int>& s_lines);





private:
	std::vector<Line> _lines;		 /* lines */
	std::vector<int> _vLinesIndices; /* vertical lines indices */
	std::vector<int> _hLinesIndices; /* horizontal lines indices */
	std::vector<int> _sLinesIndices; /* slash lines indices */
};




//Ȧ����Բ
class CirclesData
{
public:
	void clearCircles();
	bool pushBackCircles(const Circle&);
	const std::vector<Circle>& circles();
	//����Բ������
	bool sortByRef(compRef ref);

private:
	std::vector<Circle> _circles;
};

class CornerData
{
	/*������_corners��ʼ�����ǲ����Ա����ĵģ���Ϊ ͼֽ�Ӷ�����һ�̾��ǲ���ģ����ǲ�����д������
	���ֻ���ã�1.��ʼ���ӿ�2.���ҽӿڣ�3.ĩβ�ӿ�*/
public:
	bool clearCorners(); //�������

	const std::map<int, std::vector<Corner>>& corners() const
	{
		return _corners;
	};

	bool crossLinesS(const std::vector<Line>& lines, int idxs, int idx, double xp = Precision, double yp = Precision);
	bool crossLinesVH(const std::vector<Line>& lines, int idxv, int idxh, double xp = Precision, double yp = Precision);
	bool crossLinesSS(const std::vector<Line>& lines, int idxs1, int idxs2);
	bool setCorners(const std::map<int, std::vector<Corner>>& corner);


	//************************************
	// ����	: FindCross
	// ȫ��	: CornerData::findCross
	// ����	: public 
	// ����	: bool
	// ����	: _fromLineIdxVec
	// ����	: _excludeLineIdxVec
	// ����	: target
	// ����	: _pred
	// ˵��	: �����ཻ�ߣ���ִ��_pred
	//************************************
	template<typename _Pr>
	bool findCross(std::vector<int>& _fromLineIdxVec,
				   std::vector<int>& _excludeLineIdxVec,
				   int targetIdx,
				   _Pr _pred)
	{
		int idx = -1;
		bool bRet = false;
		const auto& cornerVecIt = _corners.find(targetIdx);
		if (cornerVecIt != _corners.end())
		{
			bool bRet2;
			//��������
			for (const auto& cornerItm : cornerVecIt->second)
			{
				bRet2 = false;
				//��������
				idx = (cornerItm.l2 == targetIdx) ? cornerItm.l1 : cornerItm.l2;
				//������Ҫ�ų�����
				if (std::find(_excludeLineIdxVec.begin(), _excludeLineIdxVec.end(), idx) == _excludeLineIdxVec.end() &&
					std::find(_fromLineIdxVec.begin(), _fromLineIdxVec.end(), idx) != _fromLineIdxVec.end())
				{
					bRet2 = _pred(idx, cornerItm);
					if (!bRet)
						bRet = bRet2;
				}
			}
		}
		return bRet;
	}


private:
	std::map<int, std::vector<Corner>> _corners;
};
//��չ����
bool extendCorner(std::vector<Corner>& corners, const Line& line, LineData& lineData);






class EndpointData
{
public:
	bool clearEndpoints(); //�������
	/*ͬ��endpoint��ʾ���ߵĶ˵㣬�Լ��ߵ���������ͼֽ������һ�̽���������ı�
	��һ����CornereData����ʹ�ÿ��ٶ�λ���㣬�Լ���ص��ߣ�endpoint�������������ҽ��㣬�Ӷ��ҵ���֮�����
	��cornerҲ����ͨ������ȥ�ҵ�����صĶ˵㣩��
	��װ1.��ʼ�� 2.ͨ���������Ҷ˵㣬������*/
	bool pushBackEndpoint(const Endpoint& endpoint); //��ʼ��

	Endpoint getEndpoint(const int& index);			 //����ֵ ͨ����������

	const std::vector<Endpoint>& endpoints();

private:
	std::vector<Endpoint> _endpoints;
};

//KDTree������---->��������һά���ݵ�����������кܸߵ�Ч��
/*KDTree ����������
*/
class KDTreeData
{
public:
	bool kdttextsClear();
	bool kdtendsClear();
	bool buildKDTreeData(const std::vector<Endpoint>& _endpoints, const std::vector<Point>& _textPoints);
	const kdt::KDTree<Point>& kdtTexts() const
	{
		return _kdttexts;
	};
	const kdt::KDTree<Endpoint>& kdtEndpoints() const
	{
		return _kdtends;
	}

private:
	kdt::KDTree<Point> _kdttexts;
	kdt::KDTree<Endpoint> _kdtends;
};

//RTree �ռ������� Ŀǰ��ֻ�ǽ�����Ӧ�õ���б�����棬������Ӧ�ú�������Ҳ��Ҫ�����������ٶȻ���������
class RTreeData
{
public:


	bool clearSLine();
	bool clearVLine();
	bool clearHLine();
	bool clearSBackIndex();
	bool insertSLines(const double min[], const double max[], const int id);
	bool pushBackSBackIndex(int id);

	const RLine& sLines();
	const std::vector<int>& SBackIndex();


private:
	RLine s_lines;
	RLine v_lines;
	RLine h_lines;
	std::vector<int> s_back_index;
};

class TextPointData
{
public:
	bool pushBackPoint(const Point& point);
	bool clearPoints();
	bool sortByRef(compRef ref);
	const std::vector<Point>& textpoints();

	int findLowerBound(double val, double precision = Precision);
	int findUpperBound(double val, double precision = Precision);

private:
	std::vector<Point> _textpoints;
};

//����ͬ��ͼ��д��map������ͬͼ���������������һ��
class LineLayerData
{
public:
	//************************************
	// ����	: bInLineLayerMap
	// ȫ��	: LineLayerData::getGoalLayerLineVec
	// ����	: private 
	// ����	: std::vector<int>
	// ����	: goalLayer ��Ҫ�жϵ�ͼ��
	// ˵��	: �鿴�Ƿ���ڴ�ͼ��
	//************************************
	std::vector<int> getGoalLayerLineVec(const std::string& goalLayer);
	//************************************
	// ����	: setLineLayerMapVec
	// ȫ��	: LineLayerData::setLineLayerMapVec
	// ����	: private 
	// ����	: bool
	// ����	: lineIndex ��ǰ����������
	// ���� : line  ����ʵ��
	// ˵��	: �鿴�Ƿ���ڴ�ͼ��
	//************************************
	bool setLineLayerMapVec(const int& lineIndex, const Line& line);
	//************************************
	// ����	: lineLayerMapVec
	// ȫ��	: LineLayerData::lineLayerMapVec
	// ����	: private 
	// ����	: std::map<std::string, std::vector<int>>
	// ˵��	: �������ͼ�������Ϣ
	//************************************
	const std::map<std::string, std::vector<int>>& lineLayerMapVec();
	//endָ��
	std::map<std::string, std::vector<int>>::const_iterator lineLayerEndP();
private:

	std::map<std::string, std::vector<int>> _lineLayerMapVec;//����ͬͼ����������з���洢
};

//��ͼֽ�ڵ������ݴ�ųɹ��õ�data�ṹ����
class AxisLineData
{
public:

	bool setAxisHLine(const std::vector<Axis::AxisLine>& axisHLines);
	bool setAxisVLine(const std::vector<Axis::AxisLine>& axisVLines);
	bool setAxisSLine(const std::vector<Axis::AxisLine>& axisSLines);
	const std::vector<Axis::AxisLine>& axisHLines();
	const std::vector<Axis::AxisLine>& axisVLines();
	const std::vector<Axis::AxisLine>& axisSLines();
private:
	std::vector<Axis::AxisLine> _axisHLines;
	std::vector<Axis::AxisLine> _axisVLines;
	std::vector<Axis::AxisLine> _axisSLines;
};
class Data;

class DimData
{
public:

	void setDim(const std::shared_ptr<Dimension> dim);

	//************************************
	// ����	: FindHVDimension
	// ȫ��	: DimData::findHVDimension
	// ����	: public 
	// ����	: bool
	// ����	: _dimPairVec
	// ����	: tick
	// ����	: bIncrement
	// ����	: bHorizontal
	// ����	: data
	// ˵��	: ����ˮƽ����ֱ��ע
	//************************************
	bool findHVDimension(std::vector<DimensionPair>& _dimPairVec,
						 const Point& tick,
						 bool bIncrement,
						 bool bHorizontal);

	bool findHDimension(std::vector<DimensionPair>& _dimPairVec,
						const double xTick,
						const double offset = Precision);

	bool findVDimension(std::vector<DimensionPair>& _dimPairVec,
						const double yTick,
						const double offset = Precision);


	//************************************
	// ����	: FindAngularDimension
	// ȫ��	: DimData::findAngularDimension
	// ����	: public 
	// ����	: std::vector<int> ��ע����������
	// ����	: lineIdxVec	
	// ����	: data
	// ˵��	: ���ҽǶȱ�ע
	//************************************
	std::vector<int> findAngularDimension(const std::vector<unsigned>& lineIdxVec,
										  const std::shared_ptr<Data>& data)const;

	//************************************
	// ����	: FindRadialAndDiameterDimension
	// ȫ��	: DimData::findRadialAndDiameterDimension
	// ����	: public 
	// ����	: unsigned int
	// ����	: pt		Բ��
	// ˵��	: ���Ұ뾶��ע��ֱ����ע
	//************************************
	unsigned int findRadialAndDiameterDimension(const Point& pt) const;

	//************************************
	// ����	: GetSlashDimRTree
	// ȫ��	: DimData::getSlashDimRTree
	// ����	: public 
	// ����	: const RTreeData&
	// ˵��	: ���� ���ڲ�����б��ע��RTree
	//************************************
	const RTreeData& getSlashDimRTree() const
	{
		return sDimRTree;
	}

	//************************************
	// ����	: Dimensions
	// ȫ��	: DimData::dimensions
	// ����	: public 
	// ����	: const std::vector<std::shared_ptr<Dimension>>
	// ˵��	: ��ע�ļ���
	//************************************
	const std::vector<std::shared_ptr<Dimension>> dimensions()const
	{
		return _dimensions;
	}

private:
	void setDimRadius(const std::shared_ptr<Dimension> dim);
	void setDimAngular(const std::shared_ptr<Dimension> dim);
	void setDimLinearAligned(const std::shared_ptr<Dimension> dim);

	std::vector<std::pair<Point, unsigned>> circlePtVec;
	std::vector<std::pair<Line, unsigned>> angularLineVec;
	std::map<long long, std::vector<DimensionPair>> hDimDesMap;	//ˮƽ��עxӳ��
	std::map<long long, std::vector<DimensionPair>> vDimDesMap;	//��ֱ��עyӳ��
	std::map<long long, std::vector<unsigned>> lenDimDesMap;	//��ע����ӳ��
	RTreeData sDimRTree;										//б��עRtree
	std::vector<std::shared_ptr<Dimension>> _dimensions;		//���еĳߴ�
};

//���ݼ�
class Data
{

public:
	LineData m_lineData;
	CornerData m_cornerData;
	TextPointData m_textPointData;
	RTreeData m_rtreeData;
	KDTreeData m_kdtTreeData;
	EndpointData m_endpointData;
	CirclesData m_circleData;
	LineLayerData m_lineLayerData;
	AxisLineData m_axisLineData;
	DimData m_dimData;
	std::vector<std::shared_ptr<Dimension>> m_dimensions;
};
#endif