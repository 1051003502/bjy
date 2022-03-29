#ifndef __DRAWING_H__
#define __DRAWING_H__
//#include <windows.h>
//#include "transer.h"
//#include "parser.h"

#include "../drawingAssociation/src/dataIntegration.h"
static COLORREF CadColor[256] =
{
	RGB(255,255,255),
	RGB(255,  0,  0), RGB(255,255,  0), RGB(0,255,  0), RGB(0,255,255), RGB(0,  0,255),	// 5
	RGB(255,  0,255), RGB(255,255,255), RGB(128,128,128), RGB(192,192,192), RGB(255,  0,  0),	// 10
	RGB(255,127,127), RGB(204,  0,  0), RGB(204,102,102), RGB(153,  0,  0), RGB(153, 76, 76),	// 15
	RGB(127,  0,  0), RGB(127, 63, 63), RGB(76,  0,  0), RGB(76, 38, 38), RGB(255, 63,  0),	// 20
	RGB(255,159,127), RGB(204, 51,  0), RGB(204,127,102), RGB(153, 38,  0), RGB(153, 95, 76),	// 25
	RGB(127, 31,  0), RGB(127, 79, 63), RGB(76, 19,  0), RGB(76, 47, 38), RGB(255,127,  0),	// 30
	RGB(255,191,127), RGB(204,102,  0), RGB(204,153,102), RGB(153, 76,  0), RGB(153,114, 76),	// 35
	RGB(127, 63,  0), RGB(127, 95, 63), RGB(76, 38,  0), RGB(76, 57, 38), RGB(255,191,  0),	// 40
	RGB(255,223,127), RGB(204,153,  0), RGB(204,178,102), RGB(153,114,  0), RGB(153,133, 76),	// 45
	RGB(127, 95,  0), RGB(127,111, 63), RGB(76, 57,  0), RGB(76, 66, 38), RGB(255,255,  0),	// 50
	RGB(255,255,127), RGB(204,204,  0), RGB(204,204,102), RGB(153,153,  0), RGB(153,153, 76),	// 55
	RGB(127,127,  0), RGB(127,127, 63), RGB(76, 76,  0), RGB(76, 76, 38), RGB(191,255,  0),	// 60
	RGB(223,255,127), RGB(153,204,  0), RGB(178,204,102), RGB(114,153,  0), RGB(133,153, 76),	// 65
	RGB(95,127,  0), RGB(111,127, 63), RGB(57, 76,  0), RGB(66, 76, 38), RGB(127,255,  0),	// 70
	RGB(191,255,127), RGB(102,204,  0), RGB(153,204,102), RGB(76,153,  0), RGB(114,153, 76),	// 75
	RGB(63,127,  0), RGB(95,127, 63), RGB(38, 76,  0), RGB(57, 76, 38), RGB(63,255,  0),	// 80
	RGB(159,255,127), RGB(51,204,  0), RGB(127,204,102), RGB(38,153,  0), RGB(95,153, 76),	// 85
	RGB(31,127,  0), RGB(79,127, 63), RGB(19, 76,  0), RGB(47, 76, 38), RGB(0,255,  0),	// 90
	RGB(127,255,127), RGB(0,204,  0), RGB(102,204,102), RGB(0,153,  0), RGB(76,153, 76),	// 95
	RGB(0,127,  0), RGB(63,127, 63), RGB(0, 76,  0), RGB(38, 76, 38), RGB(0,255, 63),	// 100
	RGB(127,255,129), RGB(0,204, 51), RGB(102,204,127), RGB(0,153, 38), RGB(76,153, 95),	// 105
	RGB(0,127, 31), RGB(63,127, 79), RGB(0, 76, 19), RGB(38, 76, 47), RGB(0,255,127),	// 110
	RGB(127,255,191), RGB(0,204,102), RGB(102,204,153), RGB(0,153, 76), RGB(76,153,114),	// 115
	RGB(0,127, 63), RGB(63,127, 95), RGB(0, 76, 38), RGB(38, 76, 57), RGB(0,255,191),	// 120
	RGB(127,255,223), RGB(0,204,153), RGB(102,204,178), RGB(0,153,114), RGB(76,153,133),	// 125
	RGB(0,127, 95), RGB(63,127,111), RGB(0, 76, 57), RGB(38, 76, 66), RGB(0,255,255),	// 130
	RGB(127,255,255), RGB(0,204,204), RGB(102,204,204), RGB(0,153,153), RGB(76,153,153),	// 135
	RGB(0,127,127), RGB(63,127,127), RGB(0, 76, 76), RGB(38, 76, 76), RGB(0,191,255),	// 140
	RGB(127,223,255), RGB(0,153,204), RGB(102,178,204), RGB(0,114,153), RGB(76,133,153),	// 145
	RGB(0, 95,127), RGB(63,111,127), RGB(0, 57, 76), RGB(38, 66, 76), RGB(0,127,255),	// 150
	RGB(127,191,255), RGB(0,102,204), RGB(102,153,204), RGB(0, 76,153), RGB(76,114,153),	// 155
	RGB(0, 63,127), RGB(63, 95,127), RGB(0, 38, 76), RGB(38, 57, 76), RGB(0, 63,255),	// 160
	RGB(127,159,255), RGB(0, 51,204), RGB(102,127,204), RGB(0, 38,153), RGB(76, 95,153),	// 165
	RGB(0, 31,127), RGB(63, 79,127), RGB(0, 19, 76), RGB(38, 47, 76), RGB(0,  0,255),	// 170
	RGB(127,127,255), RGB(0,  0,204), RGB(102,102,204), RGB(0,  0,153), RGB(76, 76,153),	// 175
	RGB(0,  0,127), RGB(63, 63,127), RGB(0,  0, 76), RGB(38, 38, 76), RGB(63,  0,255),	// 180
	RGB(159,127,255), RGB(51,  0,204), RGB(127,102,204), RGB(38,  0,153), RGB(95, 76,153),	// 185
	RGB(31,  0,127), RGB(79, 63,127), RGB(19,  0, 76), RGB(47, 38, 76), RGB(127,  0,255),	// 190
	RGB(191,127,255), RGB(102,  0,204), RGB(153,102,204), RGB(76,  0,153), RGB(114, 76,153),	// 195
	RGB(63,  0,127), RGB(95, 63,127), RGB(38,  0, 76), RGB(57, 38, 76), RGB(191,  0,255),	// 200
	RGB(223,127,255), RGB(153,  0,204), RGB(178,102,204), RGB(114,  0,153), RGB(133, 76,153),	// 205
	RGB(95,  0,127), RGB(111, 63,127), RGB(57,  0, 76), RGB(66, 38, 76), RGB(255,  0,255),	// 210
	RGB(255,127,255), RGB(204,  0,204), RGB(204,102,204), RGB(153,  0,153), RGB(153, 76,153),	// 215
	RGB(127,  0,127), RGB(127, 63,127), RGB(76,  0, 76), RGB(76, 38, 76), RGB(255,  0,191),	// 220
	RGB(255,127,223), RGB(204,  0,153), RGB(204,102,178), RGB(153,  0,114), RGB(153, 76,133),	// 225
	RGB(127,  0, 95), RGB(127, 63,111), RGB(76,  0, 57), RGB(76, 38, 66), RGB(255,  0,127),	// 230
	RGB(255,127,191), RGB(204,  0,102), RGB(204,102,153), RGB(153,  0, 76), RGB(153, 76,114),	// 235
	RGB(127,  0, 63), RGB(127, 63, 95), RGB(76,  0, 38), RGB(76, 38, 57), RGB(255,  0, 63),	// 240
	RGB(255,127,159), RGB(204,  0, 51), RGB(204,102,127), RGB(153,  0, 38), RGB(153, 76, 95),	// 245
	RGB(127,  0, 31), RGB(127, 63, 79), RGB(76,  0, 19), RGB(76, 38, 47), RGB(51, 51, 51),	// 250
	RGB(91, 91, 91), RGB(132,132,132), RGB(173,173,173), RGB(214,214,214), RGB(255,255,255)	// 255
};
class MarkDrawing
{
public:
	MarkDrawing() {}
	~MarkDrawing()
	{
		if (_spParser != nullptr)
		{
			_spParser.reset();
		}
		if (_spTranser != nullptr)
		{
			_spTranser.reset();
		}
	}
	MarkDrawing(const std::shared_ptr<Parser>& _parser, const std::shared_ptr<Transer>& _transer, const std::string& drawingType)
	{
		_spParser = _parser;
		_spTranser = _transer;
		_drawingId = getRandom16Chars();//生成图纸ID
		_drawingType = drawingType;
	}
	std::string findPath()
	{
		int found = static_cast<int>(_drawingType.find_last_of("|"));
		return _drawingType.substr((long long)found + 1);

	}
	std::string returnDrawingType()
	{
		return _drawingType;
	}



private:

	std::string _drawingId;
	std::string _type;
public:
	std::shared_ptr<Parser>_spParser;
	std::shared_ptr<Transer>_spTranser;
	std::string _drawingType;

};


typedef enum {
	DRAW_ALL = 0,
	DRAW_COMPONENTS,
	DRAW_BORDERS,
	DRAW_WALL,
	DRAW_TABLES,
	DRAW_AXIS,
	DRAW_BLOCKS,
	DRAW_PILLARS,
	DRAW_GIRDER,
	DRAW_LINES,
	DRAW_CIRCLES,
	DRAW_DEBUG,
	DRAW_CORNERS,
	DRAW_TEXTS,
	DRAW_EXTRACTS,
	DRAW_ISSUES,
	DRAW_NUMBER,
}DrawType;

typedef struct tag_REALRECT
{
	double	top;
	double	left;
	double	bottom;
	double	right;
} REALRECT, * PREALRECT;

typedef enum {
	PREPARE,
	LOADING,
	LOADFAILED,
	PARSING,
	PARSED,
}DrawingState;

// View Structure **********************************************
typedef struct tag_VIEW
{
	BOOL	Viewable;		// TRUE = Drawing can be viewed
	int		WindowLeft;		// Drawing Window Boundary Properties (In Pixels)
	int		WindowTop;		// Drawing Window Boundary Properties (In Pixels)
	int		WindowRight;	// Drawing Window Boundary Properties (In Pixels)
	int		WindowBottom;	// Drawing Window Boundary Properties (In Pixels)
	double	ViewLeft;		// Drawing View Properties (In Units)
	double	ViewBottom;		// Drawing View Properties (In Units)
	double	ViewRight;		// Drawing View Properties (In Units)
	double	ViewTop;		// Drawing View Properties (In Units)
	double	PPU;			// Pixels Per Unit
	double	ZoomLevel;		// Zoom Level
} VIEW, * PVIEW;

class CDrawing
{
public:
	// Initialization & UnInitialization
	CDrawing();
	~CDrawing();

	//
	void onReaded(MarkDrawing& drawing/*const std::string& drawingId*/);
	void onFailed();
	void onParsed(MarkDrawing& drawing);

	BOOL isOpen();
	// Construction & Destruction
	BOOL Create();
	BOOL Destroy();

	// Drawing View
	BOOL InitView(int x, int y, int nWidth, int nHeight);
	BOOL Paint(HDC hdc);
	double GetZoomLevel();
	BOOL   SetZoomLevel(double ZoomLevel);
	BOOL GetViewProperties(PVIEW pView);
	BOOL SetViewProperties(PVIEW pView);
	BOOL GetDrawingBorder(PREALRECT pRect);
	BOOL ZoomExtents();

	BOOL SaveFile(LPCTSTR FileName);
	BOOL LoadFile(MarkDrawing& markDrawing);
	//整理数据
	BOOL DataSorting();

	BOOL drawComponents(HDC hdc);
	BOOL drawLines(HDC hdc);
	BOOL drawCorners(HDC hdc);
	BOOL drawTexts(HDC hdc);
	BOOL drawExtracts(HDC hdc);
	BOOL drawBorders(HDC hdc);
	BOOL drawIssues(HDC hdc);
	BOOL drawTables(HDC hdc);
	BOOL drawCircles(HDC hdc);
	BOOL drawAxis(HDC hdc);
	BOOL drawBlocks(HDC hdc);
	BOOL drawPillars(HDC hdc);
	BOOL drawBeams(HDC hdc);
	BOOL drawWalls(HDC hdc);
	BOOL drawDebug(HDC hdc);

	std::vector<MarkDrawing>::iterator findDrawing(const std::string& drawingType)
	{
		for (auto it = _markDrawingVec.begin(); it != _markDrawingVec.end(); ++it)
		{
			if (it->returnDrawingType() == drawingType)
			{
				return it;
			}
		}
	}

public:
	bool next();
	bool prev();
	bool step(int d);
	int getIndex();
	bool select(DrawType cur);
	DrawType get();
	bool getFull();

	bool write(const std::string& file);

	bool select(const std::string& file);
	bool close1(const std::string& file);

protected:
	BOOL drawLine(HDC, const Line& line);
protected:
	PREALRECT _p_real_rect;
	VIEW _view;

	std::shared_ptr<Transer> _transer;
	std::shared_ptr<Parser> _parser;
	std::vector<MarkDrawing>_markDrawingVec;

	//std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>> _files;
	//CProject _project;
	DataIntegration _dataInte; //data integration

	/*slot _readed;
	slot _failed;
	slot _writed;
	slot _parsed;*/
	std::vector<slot> _readed;
	std::vector<slot> _failed;
	std::vector<slot> _writed;
	std::vector<slot> _parsed;
	std::vector<signal<void(void)>>_drawSigVec;

	//std::vector<std::map<int, std::map<int, std::shared_ptr<DRW_Text>>>> _extracts;
	DrawingState _state;

	std::mutex _mutex;
	bool full;
	int index;
	DrawType type;
};

#endif