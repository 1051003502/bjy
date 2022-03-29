#ifndef FORMAT_PARSE_H
#define FORMAT_PARSE_H
#include "headerFile.h"
#include "log.h"


using SLine = RTree<int, double, 2, float>;

class Parser;

class Entry {
public:
    Box box;
    std::shared_ptr<DRW_Entity> entity;
};

class Parser {
public:
	Parser()
	{
		m_data = std::make_shared<Data>();
	}
	Parser(const std::shared_ptr<dx_data> data) : _data(data)
	{
		reset();
		parse(_data);
	}
	virtual ~Parser()
	{
		reset();
		if (_thread && _thread->joinable())
			_thread->join();
	}

    bool reset();

	bool parse(const std::shared_ptr<dx_data>& data)
	{
		reset();
		if (_thread.get())
			_thread->join();
		_thread.reset(new std::thread([this, data]
		{
			reset();
			_data = data;
			formats();
			sigParsed.call();
		}));
		return true;
	}

	//测试程序调用
	bool parse_testing_programme(const std::shared_ptr<dx_data>& data, std::string testType)
	{
		reset();
		if (_thread.get())
			_thread->join();
		_thread.reset(new std::thread([this, data, testType]
		{
			reset();
			_data = data;
			formats_testing_programme(testType);
			sigParsed.call();
		}));
		return true;
	}
	void formats_testing_programme(std::string testType)
	{
		const Transforms trans{};
		if (!_data->blockInMode->ents.empty())
		{
			_formats = formatBlock(_data->blockInMode, trans, _box, true);
		}
		else
		{
			for (auto& it : _data->blocks)
			{
				auto formats = formatBlock(it.second, trans, _box, true);
				_formats.insert(_formats.end(), formats.begin(), formats.end());
			}
		}
		crossCorners();
		m_data->m_textPointData.sortByRef(compareY);
		m_data->m_kdtTreeData.buildKDTreeData(m_data->m_endpointData.endpoints(),
			m_data->m_textPointData.textpoints());
		findComponents();
		findBorders();
		findAxises();
		findTables();
		findBlocks();
		findSectionSymbols();

		if (testType == "location")
			findWallLocation();
		else if (testType == "profile")
			findWallsSection();
		
	}


	std::shared_ptr<dx_data> cutBy(const Box& box);

	Json::Value serializeTable(const std::shared_ptr<Table>& table);
	Json::Value serializeBorder(const std::shared_ptr<Component>& border);
	Json::Value serializeComponent(const std::shared_ptr<Component>& border);
	Json::Value serializePillar(const std::shared_ptr<Pillar>& pillar);
	Json::Value serializeBlock(const std::shared_ptr<Block>& block);
	Json::Value serializePillarSection(const std::shared_ptr<ColumnSection>& section);
	Json::Value serializeText();
	//返回墙平面图json数据  暂时不用到此函数
	Json::Value serializeWall();
	//返回墙平面图json数据
	Json::Value serializeWallLocation()
	{
		Json::Value body;
		int id = 1;
		for (auto wallLoc : _spWallLcoationVec)
		{
			body.append(wallLoc->getWallLocationJsonData());
		}
		return body;
	}
	//返回纵剖墙json数据
	Json::Value serializeWallHProfile()
	{
		Json::Value body;
		for (auto spWallSection : _spWallVec)
		{
			body.append(spWallSection->getWallSectionJsonData());
		}
		return body;
	};
	Json::Value serializeAxises();
	Json::Value serializeLinehandle();//输出线的handle
	Json::Value serializeColumnSection();//输出柱截面
	
	void Parser::serializeTest();

public:
    //所有成员变量的接口

    //************************************
    // 方法	: m_data
    // 全名	: Parser::getDataInt get data interface
    // 访问	: public
    // 返回	: bool
    // 参数	: data
    // 说明	: 返回公共数据
    //************************************
    std::shared_ptr<Data> getDataInt();


protected:
    bool findComponentsByTexts();
	bool searchSLinesBack(int id);
    bool findDimensionsBlock(std::shared_ptr<Dimension> dim);  //为零散的DIM标注寻找所属块

protected:
    bool findComponents();

	bool findBorders();
	bool findIssues();
	bool findTables();
	bool findAxises();
	bool findBlocks();
	bool findColumns();
	bool findColumnSections();
	bool findBeams();

	std::shared_ptr<Axis> getCompleteAxis();

	bool findBeamSection();
	bool findBeamLongitudinal();
	bool findWallsSection();
	void findSectionSymbols();

	bool findElevationSymbols();

	bool parseLongitudinalSection();
	
	void outputSectionSymbols();
	void outputBoardInfo();

	bool SetBlockInsideInformation(const std::shared_ptr<Block>& block);
	bool findWallLocation();
	bool parseFloorPlan();

protected:
	bool coverByComponents(const Corner& p);
	bool isCandidate(const std::string& text);
	bool crossCorners();
	bool divisionLineLayerIndex();//对所有线的图层进行归类

	bool formats();

	bool searchSLinesBack(int id) const;

	bool pushLine(const Point& p1,
				  const Point& p2,
				  bool side = false);

	std::shared_ptr<DRW_Entity> formatEntity(const std::shared_ptr<DRW_Entity>& entity,
											 const Transforms& trans, Box& box);

	std::vector<std::shared_ptr<DRW_Entity>> formatBlock(const std::shared_ptr<dx_ifaceBlock>& block,
														 const Transforms& trans, 
														 Box& box, 
														 bool entry /*=false*/);

	std::shared_ptr<dx_ifaceBlock> getBlock(const std::string& name);

protected:
    //柱断面处理集
    //查找柱断面图中的标注信息
    bool FindSectionDIm(const Box &box, std::vector<Dimension> &temp_dim);

public:
    std::shared_ptr<Data> m_data;
    signal<void(void)> sigParsed;
    std::shared_ptr<std::thread> _thread;

	std::shared_ptr<dx_data> _data;
	std::vector<std::shared_ptr<DRW_Entity>> _formats;
	std::vector<std::shared_ptr<DRW_Solid>> _solids;
	std::vector<std::shared_ptr<DRW_Text>> _texts;//所有的文本
	std::vector<std::shared_ptr<Component>> _components;//组件
	std::vector<std::shared_ptr<Component>> _borders;//签名栏边框
	std::vector<std::shared_ptr<Component>> _issues;
	std::vector<std::shared_ptr<Component>> _covers;
	std::vector<std::shared_ptr<Table>> _tables;//表格
	std::vector<std::shared_ptr<Axis>> _axises;//轴网

	std::vector<std::shared_ptr<Block>> _blocks;   //划分好的区块
	std::vector<std::shared_ptr<Dimension>> _dimensions;//标注

	std::vector<Axis::AxisLine> m_axisHLines;
	std::vector<Axis::AxisLine> m_axisVLines;

	std::vector<std::shared_ptr<SectionSymbolGroup>> _sectionSybmols;	//所有的剖切符号
	std::vector<std::shared_ptr<ElevationSymbol>> _elevationSymbols;		//所有的标高符号

	
	/*****************************************/

	std::vector<std::shared_ptr<ColumnSection>> _section;//柱子的截面
	std::vector<std::shared_ptr<Beam>> _beams;//梁，位置信息，加入是平法则会包含梁的所有信息
	std::vector<std::shared_ptr<BeamSection>>_beamSections;//梁截面，非平法时使用
	std::vector<std::shared_ptr<BeamLongitudinal>> _beamLongitudinals;//得到梁的纵梁配筋信息
	std::vector<std::shared_ptr<Pillar>> _pillars;//所有的柱位置信息
    std::vector<std::shared_ptr<WallSection>>_spWallVec;//所有的墙信息
	std::vector<std::shared_ptr<WallLocation>>_spWallLcoationVec;//组织墙的位置信息
    

	std::vector<Entry> _enties;//所有的实体
	std::vector<Line> _textlines;
	std::vector<Point> _circlepoints;//所有圆的，圆心
	Box _box;//所有的组件的包围盒


public:
	BeamSet _beamSet;//组织梁信息的集合
	BorderSet _borderSet;//组织签名栏的结合
	AxisSet _axisSet;//组织轴网的结合
	BlockSet _blockSet;//组织区块的集合
	SectionSymbolSet _sectionSymbolSet;//组织剖线信息
	PillarSet _pillarSet;//组织柱信息的集合
	ColumnSectionSet _sectionSet;//组织柱的截面信息
	BeamSectionSet _beamSectionSet;//梁的截面信息 
	BeamLongitudinalSet _beamLongitudinalSet;//纵梁箍筋信息

	WallSectionSet _wallSectionSet;                       //整合墙信息
	MainStructureSection _mainStructureSection;//解析横剖配筋图，获取基础数据
	ElevationSymbolSet _elevationSymbolSet; //组织解析得到标高符
	LongitudinalStructProfile _longitudinalStructProfile; //解析的纵剖平面图数据
	


	/********************************/
	bool record{ false };
	Point point;
	std::vector<int> ends;
	std::vector<Corner> cs;
	std::vector<int> ls;
	//测试
	std::vector<int> temp_index;
	std::vector<int> temp_index_write;
	std::vector<int> temp_index_green;
	std::vector<std::vector<int> > temp_index_colorBlocks;
	std::vector<std::vector<Line>> temp_line_colorBlocks;
	std::vector<Line> temp_line;
	std::vector<Point> _corners;
	std::vector<std::vector<int>>m_temp_index;
    std::vector<std::pair<int, int>>tmep_pair_index;
    std::vector<Box> testBoxVec;

};

#endif
