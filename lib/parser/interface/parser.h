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

	//���Գ������
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
	//����ǽƽ��ͼjson����  ��ʱ���õ��˺���
	Json::Value serializeWall();
	//����ǽƽ��ͼjson����
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
	//��������ǽjson����
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
	Json::Value serializeLinehandle();//����ߵ�handle
	Json::Value serializeColumnSection();//���������
	
	void Parser::serializeTest();

public:
    //���г�Ա�����Ľӿ�

    //************************************
    // ����	: m_data
    // ȫ��	: Parser::getDataInt get data interface
    // ����	: public
    // ����	: bool
    // ����	: data
    // ˵��	: ���ع�������
    //************************************
    std::shared_ptr<Data> getDataInt();


protected:
    bool findComponentsByTexts();
	bool searchSLinesBack(int id);
    bool findDimensionsBlock(std::shared_ptr<Dimension> dim);  //Ϊ��ɢ��DIM��עѰ��������

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
	bool divisionLineLayerIndex();//�������ߵ�ͼ����й���

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
    //�����洦��
    //����������ͼ�еı�ע��Ϣ
    bool FindSectionDIm(const Box &box, std::vector<Dimension> &temp_dim);

public:
    std::shared_ptr<Data> m_data;
    signal<void(void)> sigParsed;
    std::shared_ptr<std::thread> _thread;

	std::shared_ptr<dx_data> _data;
	std::vector<std::shared_ptr<DRW_Entity>> _formats;
	std::vector<std::shared_ptr<DRW_Solid>> _solids;
	std::vector<std::shared_ptr<DRW_Text>> _texts;//���е��ı�
	std::vector<std::shared_ptr<Component>> _components;//���
	std::vector<std::shared_ptr<Component>> _borders;//ǩ�����߿�
	std::vector<std::shared_ptr<Component>> _issues;
	std::vector<std::shared_ptr<Component>> _covers;
	std::vector<std::shared_ptr<Table>> _tables;//���
	std::vector<std::shared_ptr<Axis>> _axises;//����

	std::vector<std::shared_ptr<Block>> _blocks;   //���ֺõ�����
	std::vector<std::shared_ptr<Dimension>> _dimensions;//��ע

	std::vector<Axis::AxisLine> m_axisHLines;
	std::vector<Axis::AxisLine> m_axisVLines;

	std::vector<std::shared_ptr<SectionSymbolGroup>> _sectionSybmols;	//���е����з���
	std::vector<std::shared_ptr<ElevationSymbol>> _elevationSymbols;		//���еı�߷���

	
	/*****************************************/

	std::vector<std::shared_ptr<ColumnSection>> _section;//���ӵĽ���
	std::vector<std::shared_ptr<Beam>> _beams;//����λ����Ϣ��������ƽ������������������Ϣ
	std::vector<std::shared_ptr<BeamSection>>_beamSections;//�����棬��ƽ��ʱʹ��
	std::vector<std::shared_ptr<BeamLongitudinal>> _beamLongitudinals;//�õ��������������Ϣ
	std::vector<std::shared_ptr<Pillar>> _pillars;//���е���λ����Ϣ
    std::vector<std::shared_ptr<WallSection>>_spWallVec;//���е�ǽ��Ϣ
	std::vector<std::shared_ptr<WallLocation>>_spWallLcoationVec;//��֯ǽ��λ����Ϣ
    

	std::vector<Entry> _enties;//���е�ʵ��
	std::vector<Line> _textlines;
	std::vector<Point> _circlepoints;//����Բ�ģ�Բ��
	Box _box;//���е�����İ�Χ��


public:
	BeamSet _beamSet;//��֯����Ϣ�ļ���
	BorderSet _borderSet;//��֯ǩ�����Ľ��
	AxisSet _axisSet;//��֯�����Ľ��
	BlockSet _blockSet;//��֯����ļ���
	SectionSymbolSet _sectionSymbolSet;//��֯������Ϣ
	PillarSet _pillarSet;//��֯����Ϣ�ļ���
	ColumnSectionSet _sectionSet;//��֯���Ľ�����Ϣ
	BeamSectionSet _beamSectionSet;//���Ľ�����Ϣ 
	BeamLongitudinalSet _beamLongitudinalSet;//����������Ϣ

	WallSectionSet _wallSectionSet;                       //����ǽ��Ϣ
	MainStructureSection _mainStructureSection;//�����������ͼ����ȡ��������
	ElevationSymbolSet _elevationSymbolSet; //��֯�����õ���߷�
	LongitudinalStructProfile _longitudinalStructProfile; //����������ƽ��ͼ����
	


	/********************************/
	bool record{ false };
	Point point;
	std::vector<int> ends;
	std::vector<Corner> cs;
	std::vector<int> ls;
	//����
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
