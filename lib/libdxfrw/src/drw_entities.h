/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 Jos�� F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include "drw_base.h"
#include "intern/dwgbuffer.h"
#include "drw_color.h"

class dxfReader;
class dwgBuffer;
class DRW_Varient;
class DRW_Polyline;

/* !ENTITY DEFINITION */

#define SET_ENTITY_FRIENDS \
    friend class dxfRW;    \
    friend class DwgReader;

/* If it is somewhat entity */
#define IS_ENTITY(t) if (this->dwgType == t)

/* Parse Common Entity Handle Data */
#define PARSE_DWG_ENTITY_COMMON_HANDLE							\
    ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf); \
    if (!ret)													\
        return ret;

/* Entry Parse */
#define ENTRY_PARSE(e)                    \
    ret = e.parseDwg(version, &buff, bs); \
    parseEntityConfig(&e);                \
    nextEntityLink = e.nextEntityLink;    \
    preEntityLink = e.preEntityLink;

/*! END ENTITY DEFINITION*/




//! Base class for entities
/*!
*  Base class for entities
*  @author Rallaz
*/
class DRW_Entity
{
	SET_ENTITY_FRIENDS
public:
	//initializes default values
	//handles: default no handle (0), color: default BYLAYER (256), 24 bits color: default -1 (not set)
	//line weight: default BYLAYER  (dxf -1, dwg 29), space: default ModelSpace (0)
	DRW_Entity() :
		dwgType(DRW::UNUSED),
		handle(DRW::NoHandle),
		parentHandle(DRW::NoHandle),
		appData(0),
		space(DRW::ModelSpace),
		layer("0"),
		lineType("BYLAYER"),
		material(DRW::MaterialByLayer),
		color(DRW::ColorByLayer),
		dwgColor(),
		lWeight(DRW_LW_Conv::widthByLayer),
		linetypeScale(1.0),
		numProxyGraph(0),
		proxyGraphics(std::string()),
		color24(-1),
		colorName(std::string()),
		transparency(DRW::Opaque),
		plotStyle(DRW::DefaultPlotStyle),
		shadow(DRW::CastAndReceieveShadows),
		haveExtrusion(false),
		extData(),
		noLinks(0),
		plotstyleFlag(0),
		linetypeFlag(0),
		materialFlag(0),
		shadowFlag(0),
		lTypeHandleRef(0),
		layerHandleRef(0),
		nextEntityLink(0),
		preEntityLink(0),
		hasOwner(false),
		xDictMissingFlag(0),
		numReactors(0),
		objBitSize(0),
		extAxisX(DRW_Coord()),
		extAxisY(DRW_Coord()),
		curr(nullptr),
		hasFullVisualStyle(false),
		hasFaceVisualStyle(false),
		hasEdgeVisualStyle(false),
		invisibleFlag(false)
	{}

	DRW_Entity(const DRW_Entity& e)
	{
		dwgType = e.dwgType;
		handle = e.handle;
		parentHandle = e.parentHandle; //no handle (0)
		lineType = e.lineType;
		color = e.color; // default BYLAYER (256)
		dwgColor = e.dwgColor;
		linetypeScale = e.linetypeScale;
		layer = e.layer;
		lWeight = e.lWeight;
		space = e.space;
		haveExtrusion = e.haveExtrusion;
		color24 = e.color24; //default -1 not set
		numProxyGraph = e.numProxyGraph;
		shadow = e.shadow;
		material = e.material;
		plotStyle = e.plotStyle;
		transparency = e.transparency;
		noLinks = e.noLinks;
		plotstyleFlag = e.plotstyleFlag;
		linetypeFlag = e.linetypeFlag;
		materialFlag = e.materialFlag;
		shadowFlag = e.shadowFlag;
		lTypeHandleRef = e.lTypeHandleRef;
		layerHandleRef = e.layerHandleRef;
		nextEntityLink = e.nextEntityLink;
		preEntityLink = e.preEntityLink;
		numReactors = e.numReactors;
		xDictMissingFlag = e.xDictMissingFlag;
		objBitSize = e.objBitSize;
		dwgType = e.dwgType;
		curr = e.curr;
		hasOwner = e.hasOwner;
		for (auto item : e.extData)
		{
			this->extData.push_back(new DRW_Variant(*item));
		}
	}

	virtual ~DRW_Entity()
	{
		for (auto& it : extData)
			delete it;

		extData.clear();
	}

	void reset()
	{
		for (auto& it : extData)
			delete it;
		extData.clear();
	}

	virtual void applyExtrusion() = 0;


	duint32 getLineTypeHandleRef() const
	{
		return lTypeHandleRef;
	}
	duint32 getLayerHandleRef() const
	{
		return layerHandleRef;
	}
	duint8 getLineTypeFlag()const
	{
		return linetypeFlag;
	}
	duint8 getPlotStyleFlag() const
	{
		return plotstyleFlag;
	}


protected:
	// parses dxf pair to read entity
	bool parseCode(int code, dxfReader* reader);
	// calculates extrusion axis (normal vector)
	void calculateAxis(DRW_Coord extPoint);
	// apply extrusion to @extPoint and return data in @point
	void extrudePoint(DRW_Coord extPoint, DRW_Coord* point);
	// parse the entity
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) = 0;
	// parses dwg common start part to read entity
	bool parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* strBuf, duint32 bs = 0);
	// parses dwg common handles part to read entity
	bool parseDwgCommonEntityHandle(DRW::Version version, dwgBuffer* buf);
	//parses dxf 102 groups to read entity
	bool parseDxfGroups(int code, dxfReader* reader);

public:
	DRW::DwgType dwgType;						/*!< enum: dwg type, code 0 */
	duint32 handle;								/*!< entity identifier, code 5 */
	duint32 parentHandle;						/*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
	std::list<std::list<DRW_Variant>> appData;	/*!< list of application data, code 102 */
	DRW::Space space;							/*!< space indicator, code 67*/
	UTF8STRING layer;							/*!< layer name, code 8 */
	UTF8STRING lineType;						/*!< line type, code 6 */
	duint32 material;							/*!< hard pointer id to material object, code 347 */
	int color;									/*!< entity color, code 62 */
	std::shared_ptr<DRW_Color> dwgColor;		/*!< entity color, code 62 */
	enum DRW_LW_Conv::LineWidth lWeight;		/*!< entity line weight, code 370 */
	double linetypeScale;						/*!< line type scale, code 48 */
	int numProxyGraph;							/*!< Number of bytes in proxy graphics, code 92 */
	std::string proxyGraphics;					/*!< proxy graphics bytes, code 310 */
	int color24;								/*!< 24-bit color, code 420 */
	std::string colorName;						/*!< color name, code 430 */
	int transparency;							/*!< transparency, code 440 */
	int plotStyle;								/*!< hard pointer id to plot style object, code 390 */
	DRW::ShadowMode shadow;						/*!< shadow mode, code 284 */
	bool haveExtrusion;							/*!< set to true if the entity have extrusion*/
	std::vector<DRW_Variant*> extData;			/*!< FIFO list of extended data, codes 1000 to 1071*/
	duint8 invisibleFlag;						//!BS bit 0: 0 = visible, 1 = invisible




protected:
	duint8 noLinks;         //aka nolinks //B
	duint8 plotstyleFlag;   //presence of plot style //BB
	duint8 linetypeFlag;    //presence of linetype handle //BB
	duint8 materialFlag;    //presence of material handle //BB
	duint8 shadowFlag;      //presence of shadow handle ?? (in dwg may be plotflag)//RC
	bool hasFullVisualStyle;//!B has full visual style
	bool hasFaceVisualStyle;//!B has face visual style
	bool hasEdgeVisualStyle;//!B has edge visual style
	duint32 lTypeHandleRef;	// linetype handle
	duint32 layerHandleRef; // layer handle
	duint32 nextEntityLink; // next entity link handle
	duint32 preEntityLink;  // pre entity link handle
	bool hasOwner;          // if it has owner

	duint8 xDictMissingFlag; // ! XDictionary handle missing flag
	dint32 numReactors;      // ! reactors number
	duint32 objBitSize;      // ! RL 32bits object data size in bits
	dwgHandle bookColorhandle;	//! color handle

private:
	DRW_Coord extAxisX;
	DRW_Coord extAxisY;
	DRW_Variant* curr;
};

//! Class to handle point entity
/*!
*  Class to handle point entity
*  @author Rallaz
*/
class DRW_Point : public DRW_Entity
{
	SET_ENTITY_FRIENDS
public:
	DRW_Point()
	{
		dwgType = DRW::POINT;
		basePoint.z = extPoint.x = extPoint.y = 0;
		extPoint.z = 1;
		thickness = 0;
	}

	DRW_Point(const DRW_Point& _o) :DRW_Entity(_o)
	{
		this->basePoint = _o.basePoint;
		this->thickness = _o.thickness;
		this->extPoint = _o.extPoint;
	}
	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	DRW_Coord basePoint; /*!<  base point, code 10, 20 & 30 */
	double thickness;    /*!< thickness, code 39 */
	DRW_Coord extPoint;  /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
						 // TNick: we're not handling code 50 - Angle of the X axis for
						 // the UCS in effect when the point was drawn
};

//! Class to handle line entity
/*!
*  Class to handle line entity
*  @author Rallaz
*/
class DRW_Line : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Line()
	{
		dwgType = DRW::LINE;
		secPoint.z = 0;
	}

	virtual	~DRW_Line() = default;
	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	DRW_Coord secPoint; /*!< second point, code 11, 21 & 31 */
};

//! Class to handle ray entity
/*!
*  Class to handle ray entity
*  @author Rallaz
*/
class DRW_Ray : public DRW_Line
{
	SET_ENTITY_FRIENDS
public:
	DRW_Ray()
	{
		dwgType = DRW::RAY;
	}

protected:
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);
};

//! Class to handle xline entity
/*!
*  Class to handle xline entity
*  @author Rallaz
*/
class DRW_Xline : public DRW_Ray
{
public:
	DRW_Xline()
	{
		dwgType = DRW::XLINE;
	}
};

//! Class to handle circle entity
/*!
*  Class to handle circle entity
*  @author Rallaz
*/
class DRW_Circle : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Circle()
	{
		dwgType = DRW::CIRCLE;
		radious = 0;
	}

	virtual void applyExtrusion();

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	double radious; /*!< radius, code 40 */
};

//! Class to handle arc entity
/*!
*  Class to handle arc entity
*  @author Rallaz
*/
class DRW_Arc : public DRW_Circle
{
	SET_ENTITY_FRIENDS
public:
	DRW_Arc()
	{
		dwgType = DRW::ARC;
		isccw = 1;
		this->sAngle = 0.0;
		this->eAngle = 0.0;
	}
	virtual ~DRW_Arc()
	{}

	virtual void applyExtrusion();

	//! center point in OCS
	const DRW_Coord& center()
	{
		return basePoint;
	}
	//! the radius of the circle
	double radius()
	{
		return radious;
	}
	//! start angle in radians
	double startAngle()
	{
		return this->sAngle;
	}
	//! end angle in radians
	double endAngle()
	{
		return this->eAngle;
	}
	//! thickness
	double thick()
	{
		return thickness;
	}
	//! extrusion
	const DRW_Coord& extrusion()
	{
		return extPoint;
	}

protected:
	//! interpret code in dxf reading process or dispatch to inherited class
	void parseCode(int code, dxfReader* reader);
	//! interpret dwg data (was already determined to be part of this object)
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

public:
	double sAngle; /*!< start angle, code 50 in radians*/
	double eAngle; /*!< end angle, code 51 in radians */
	int isccw;       /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle ellipse entity
/*!
*  Class to handle ellipse and elliptic arc entity
*  Note: start/end parameter are in radians for ellipse entity but
*  for hatch boundary are in degrees
*  @author Rallaz
*/
class DRW_Ellipse : public DRW_Line
{
	SET_ENTITY_FRIENDS
public:
	DRW_Ellipse()
	{
		dwgType = DRW::ELLIPSE;
		isccw = 1;
	}

	void toPolyline(DRW_Polyline* pol, int parts = 128);
	virtual void applyExtrusion();

protected:
	//! interpret code in dxf reading process or dispatch to inherited class
	void parseCode(int code, dxfReader* reader);
	//! interpret dwg data (was already determined to be part of this object)
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

private:
	void correctAxis();

public:
	double minormajoratio; /*!< ratio of minor to major axis, code 40 */
	double sAngle;         /*!< start parameter, code 41, 0.0 for full ellipse*/
	double eAngle;         /*!< end parameter, code 42, 2*PI for full ellipse */
	int isccw;       /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle trace entity
/*!
*  Class to handle trace entity
*  @author Rallaz
*/
class DRW_Trace : public DRW_Line
{
	SET_ENTITY_FRIENDS
public:
	DRW_Trace()
	{
		dwgType = DRW::TRACEE;
		thirdPoint.z = 0;
		fourPoint.z = 0;
	}

	virtual void applyExtrusion();

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

public:
	DRW_Coord thirdPoint; /*!< third point, code 12, 22 & 32 */
	DRW_Coord fourPoint;  /*!< four point, code 13, 23 & 33 */
};

//! Class to handle solid entity
/*!
*  Class to handle solid entity
*  @author Rallaz
*/
class DRW_Solid : public DRW_Trace
{
	SET_ENTITY_FRIENDS
public:
	DRW_Solid()
	{
		dwgType = DRW::SOLID;
	}

protected:
	//! interpret code in dxf reading process or dispatch to inherited class
	void parseCode(int code, dxfReader* reader);
	//! interpret dwg data (was already determined to be part of this object)
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

public:
	//! first corner (2D)
	const DRW_Coord& firstCorner()
	{
		return basePoint;
	}
	//! second corner (2D)
	const DRW_Coord& secondCorner()
	{
		return secPoint;
	}
	//! third corner (2D)
	const DRW_Coord& thirdCorner()
	{
		return thirdPoint;
	}
	//! fourth corner (2D)
	const DRW_Coord& fourthCorner()
	{
		return thirdPoint;
	}
	//! thickness
	double thick()
	{
		return thickness;
	}
	//! elevation
	double elevation()
	{
		return basePoint.z;
	}
	//! extrusion
	const DRW_Coord& extrusion()
	{
		return extPoint;
	}
};

//! Class to handle 3dface entity
/*!
*  Class to handle 3dface entity
*  @author Rallaz
*/
class DRW_3Dface : public DRW_Trace
{
	SET_ENTITY_FRIENDS
public:
	enum InvisibleEdgeFlags
	{
		NoEdge = 0x00,
		FirstEdge = 0x01,
		SecodEdge = 0x02,
		ThirdEdge = 0x04,
		FourthEdge = 0x08,
		AllEdges = 0x0F
	};

	DRW_3Dface()
	{
		dwgType = DRW::FACE_3D;
		invisibleflag = 0;
	}

	virtual void applyExtrusion()
	{}

	//! first corner in WCS
	const DRW_Coord& firstCorner()
	{
		return basePoint;
	}
	//! second corner in WCS
	const DRW_Coord& secondCorner()
	{
		return secPoint;
	}
	//! third corner in WCS
	const DRW_Coord& thirdCorner()
	{
		return thirdPoint;
	}
	//! fourth corner in WCS
	const DRW_Coord& fourthCorner()
	{
		return fourPoint;
	}
	//! edge visibility flags
	InvisibleEdgeFlags edgeFlags()
	{
		return (InvisibleEdgeFlags)invisibleflag;
	}

protected:
	//! interpret code in dxf reading process or dispatch to inherited class
	void parseCode(int code, dxfReader* reader);
	//! interpret dwg data (was already determined to be part of this object)
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

public:
	int invisibleflag; /*!< invisible edge flag, code 70 */
};

//! Class to handle block entries
/*!
*  Class to handle block entries
*/
class DRW_Block : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Block()
	{
		dwgType = DRW::BLOCK;
		layer = "0";
		flags = 0;
		name = "*U0";
		isEnd = false;
	}

	void applyExtrusion() override
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0) override;

public:
	UTF8STRING name; /*!< block name, code 2 */
	int flags;       /*!< block type, code 70 */
private:
	bool isEnd; //for dwg parsing
};

//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Insert : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Insert()
	{
		dwgType = DRW::INSERT;
		xscale = 1;
		yscale = 1;
		zscale = 1;
		angle = 0;
		colcount = 1;
		rowcount = 1;
		colspace = 0;
		rowspace = 0;
		hasAttrib = false;
		firstAttribHandleRef = lastAttribHandleRef = 0;
		blockHeaderH = dwgHandle();
	}

	DRW_Insert(const DRW_Insert& i) : DRW_Point(i)
	{
		blockName = i.blockName;
		dwgType = i.dwgType;
		xscale = i.xscale;
		yscale = i.yscale;
		zscale = i.zscale;
		angle = i.angle;
		colcount = i.colcount;
		rowcount = i.rowcount;
		colspace = i.colspace;
		rowspace = i.rowspace;
		hasAttrib = i.hasAttrib;
		firstAttribHandleRef = i.firstAttribHandleRef;
		lastAttribHandleRef = i.lastAttribHandleRef;
		blockHeaderH = i.blockHeaderH;
		seqendH = i.seqendH;
	}

	virtual void applyExtrusion()
	{
		DRW_Point::applyExtrusion();
	}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0);

public:
	UTF8STRING blockName; /*!< block name, code 2 */
	double xscale;        /*!< x scale factor, code 41 */
	double yscale;        /*!< y scale factor, code 42 */
	double zscale;        /*!< z scale factor, code 43 */
	double angle;         /*!< rotation angle in radians, code 50 */
	int colcount;         /*!< column count, code 70 */
	int rowcount;         /*!< row count, code 71 */
	double colspace;      /*!< column space, code 44 */
	double rowspace;      /*!< row space, code 45 */
	bool hasAttrib;       // ` if has attrib
	duint32 firstAttribHandleRef;
	duint32 lastAttribHandleRef;

public: //only for read dwg
	dwgHandle blockHeaderH;
	dwgHandle seqendH; //RLZ: on implement attrib remove this handle from obj list (see pline/vertex code)
};

//! Class to handle lwpolyline entity
/*!
*  Class to handle lwpolyline entity
*  @author Rallaz
*/
class DRW_LWPolyline : public DRW_Entity
{
	SET_ENTITY_FRIENDS
public:
	DRW_LWPolyline()
	{
		dwgType = DRW::DwgType::LWPOLYLINE;
		elevation = thickness = width = 0.0;
		flags = 0;
		extPoint.x = extPoint.y = 0;
		extPoint.z = 1;
		vertex = nullptr;
		vertexnum = -1;
	}

	DRW_LWPolyline(const DRW_LWPolyline& p) : DRW_Entity(p)
	{
		this->dwgType = p.dwgType;
		this->elevation = p.elevation;
		this->thickness = p.thickness;
		this->width = p.width;
		this->flags = p.flags;
		this->extPoint = p.extPoint;
		this->vertex = nullptr;
		for (const auto& i : p.vertlist) // RLZ ok or new
			this->vertlist.emplace_back(new DRW_Vertex2D(*i));

		//this->vertex = NULL;
		//this->vertlist = p.vertlist;
	}

	~DRW_LWPolyline()
	{
		while (!vertlist.empty())
		{
			vertlist.pop_back();
		}
	}
	void applyExtrusion() override;

	void addVertex(DRW_Vertex2D v)
	{
		DRW_Vertex2D* vert = new DRW_Vertex2D();
		vert->x = v.x;
		vert->y = v.y;
		vert->stawidth = v.stawidth;
		vert->endwidth = v.endwidth;
		vert->bulge = v.bulge;
		vertlist.emplace_back(vert);
	}
	DRW_Vertex2D* addVertex()
	{
		auto* vert = new DRW_Vertex2D();
		vert->stawidth = 0;
		vert->endwidth = 0;
		vert->bulge = 0;
		vertlist.emplace_back(vert);
		return vert;
	}

	inline bool bExistWithFlag() const
	{
		return width > 0 || flags & 0x20;
	}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs = 0) override;

public:
	int vertexnum;                        /*!< number of vertex, code 90 */
	int flags;                            /*!< polyline flag, code 70, default 0 */
	double width;                         /*!< constant width, code 43 */
	double elevation;                     /*!< elevation, code 38 */
	double thickness;                     /*!< thickness, code 39 */
	DRW_Coord extPoint;                   /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
	DRW_Vertex2D* vertex;                 /*!< current vertex to add data */
	std::vector<std::shared_ptr<DRW_Vertex2D>> vertlist; /*!< vertex list */
};

//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Text : public DRW_Line
{
	SET_ENTITY_FRIENDS
public:
	//! Vertical alignments.
	enum VAlign
	{
		VBaseLine = 0, /*!< Top = 0 */
		VBottom,       /*!< Bottom = 1 */
		VMiddle,       /*!< Middle = 2 */
		VTop           /*!< Top = 3 */
	};

	//! Horizontal alignments.
	enum HAlign
	{
		HLeft = 0, /*!< Left = 0 */
		HCenter,   /*!< Centered = 1 */
		HRight,    /*!< Right = 2 */
		HAligned,  /*!< Aligned = 3 (if VAlign==0) */
		HMiddle,   /*!< middle = 4 (if VAlign==0) */
		HFit       /*!< fit into point = 5 (if VAlign==0) */

	};

	DRW_Text() :DRW_Line(), colHeightsVec{}{
		dwgType = DRW::TEXT;
		angle = 0;
		rectWidth = 1;
		oblique = 0;
		style = "STANDARD";
		textgen = 0;
		alignH = HAlign::HLeft;
		alignV = VBaseLine;
	}

	DRW_Text(const DRW_Text&) = default;

	virtual ~DRW_Text() = default;

	void applyExtrusion() override
	{}

protected:
	void parseCode(int code, dxfReader* reader);

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

	bool parseDwgAndOutStrBuf(dwgBuffer* outStrBuf, DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	double height;       /* Text height BD 40 Undocumented */
	UTF8STRING text;    /*!< text string, code 1 */
	double angle;       /*!< rotation angle in degrees (360), code 50 */
	/*Reference rectangle width (width picked by the user).*/
	double rectWidth;  /*!< width factor, code 41 */
	/*Reference rectangle height.*/
	double rectHeight;
	double oblique;     /*!< oblique angle, code 51 */
	UTF8STRING style;   /*!< style name, code 7 */
	int textgen;        /*!< text generation, code 71 */
	enum HAlign alignH; /*!< horizontal align, code 72 */
	enum VAlign alignV; /*!< vertical align, code 73 */
	double extentsWidth;
	double extentsHeight;
	dint32 backGroundFlags;/* 0 = no background, 1 = background fill, 2 = background fill with drawing fill color, 0x10 = text frame (R2018+)*/
	dint32 bkScaleFactor;	/* Background scale factor BL Present if background flags = 1, default = 1.5*/
	duint32 bkColor;		/* Background color CMC Present if background flags = 1 */
	dint32 bkTransparency;/* Background transparency BL Present if background flags = 1 */
	bool bIsNAnnotative;/*Is not annotative*/
	duint32 mtextVersion;/*Version,default 0*/
	bool defaultFlag;/*default flag,default true*/
	duint32 registerdApp;/*Registered application,hard handle*/
	duint64 attachPoint;/*Attachment point*/
	duint16 columnType;/*0 = No columns, 1 = static columns, 2 = dynamic columns*/
	dint32 colHeightCount;
	double colWidth;
	double gutter;
	bool autoHeight;
	bool flowReversed;
	std::vector<double> colHeightsVec;
	dwgHandle styleH;   /*!< handle for text style */
};

//! Class to handle attrib entries
/*!
*  Class to handle attdef entries
*  @author msclock
*/
class DRW_Attrib : public DRW_Text
{
	SET_ENTITY_FRIENDS
public:
	//` for 2018+
	enum AttribType
	{
		Unkown,
		SingleLine = 1,
		MultiLine = 2,
		MuiltieLine = 4
	};
	enum AttribFlags
	{
		None = 0,
		Invisible = 1,
		Constnat = 2,
		InputVertificatoinRequired = 4,
		Preset = 8
	};

	DRW_Attrib() : DRW_Text()
	{
		dwgType = DRW::ATTRIB;
		classVersion = 0;
		tagString = "";
		tag = "";
		lockPostion = 0;
		lockPostionFlag = 0;
		this->attribType = AttribType::SingleLine;
	}

	virtual void applyExtrusion()
	{} //RLZ TODO
protected:
	// todo for dxf
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs) override;
	bool parseDwgAndOutStrBuf(dwgBuffer* outStrBuf, DRW::Version version, dwgBuffer* buf, duint32 bs = 0);
public:
	//`2010+
	duint8 classVersion;			//!  RC    Version
	//` 2018+
	enum AttribType attribType;		// ! RC
	std::string tagString;			//! VT
	//! BS value=0
	enum AttribFlags attribFlags;	//! RC
	duint8 lockPostion;				//! B
	//` common
	//` single line
	std::string tag;
	duint16 fieldLength; // ! unused
	duint8 bitPairCoded; //! RC
	//` 2007+
	duint8 lockPostionFlag; //! B
};

//! Class to handle attdef entries
/*!
*  Class to handle attdef entries
*  @author msclock
*/
class DRW_Attdef : public DRW_Attrib
{
	SET_ENTITY_FRIENDS
public:
	DRW_Attdef() : DRW_Attrib()
	{
		dwgType = DRW::ATTDEF;
	}

protected:
	//TODO
	void parseCode(int code, dxfReader* reader);

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	// ` common
	// ` prompt
	std::string prompt; // ! TV
};

//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_MText : public DRW_Text
{
	SET_ENTITY_FRIENDS
public:
	//! Attachments.
	enum Attach
	{
		TopLeft = 1,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BottomLeft,
		BottomCenter,
		BottomRight
	};

	DRW_MText()
	{
		dwgType = DRW::MTEXT;
		interlin = 1;
		alignV = (VAlign)TopLeft;
		textgen = 1;
		haveXAxis = true; //if true needed to recalculate angle
	}

protected:
	void parseCode(int code, dxfReader* reader);
	void updateAngle(); //recalculate angle if 'haveXAxis' is true
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	double interlin; /*!< width factor, code 44 */
private:
	bool haveXAxis;
};

//! Class to handle vertex
/*!
*  Class to handle vertex  for polyline entity
*  @author Rallaz
*/
class DRW_Vertex : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Vertex()
	{
		dwgType = DRW::VERTEX_2D;
		stawidth = endwidth = bulge = 0;
		vindex1 = vindex2 = vindex3 = vindex4 = 0;
		flags = identifier = 0;
	}
	DRW_Vertex(double sx, double sy, double sz, double b)
	{
		stawidth = endwidth = 0;
		vindex1 = vindex2 = vindex3 = vindex4 = 0;
		flags = identifier = 0;
		basePoint.x = sx;
		basePoint.y = sy;
		basePoint.z = sz;
		bulge = b;
	}

protected:
	void parseCode(int code, dxfReader* reader);

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0, double el = 0);

	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0)
	{
		DRW_UNUSED(version);
		DRW_UNUSED(buf);
		DRW_UNUSED(bs);
		return true;
	}

public:
	double stawidth; /*!< Start width, code 40 */
	double endwidth; /*!< End width, code 41 */
	double bulge;    /*!< bulge, code 42 */

	int flags;      /*!< vertex flag, code 70, default 0 */
	double tgdir;   /*!< curve fit tangent direction, code 50 */
	int vindex1;    /*!< poly face mesh vertex index, code 71, default 0 */
	int vindex2;    /*!< poly face mesh vertex index, code 72, default 0 */
	int vindex3;    /*!< poly face mesh vertex index, code 73, default 0 */
	int vindex4;    /*!< poly face mesh vertex index, code 74, default 0 */
	int identifier; /*!< vertex identifier, code 91, default 0 */
};



//! Class to handle polyline entity
/*!
*  Class to handle polyline entity
*  @author Rallaz
*/
class DRW_Polyline : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Polyline()
	{
		dwgType = DRW::UNUSED;
		defStartWidth = defEndWidth = 0.0;
		basePoint.x = basePoint.y = 0.0;
		flags = vertCount = faceCount = 0;
		smoothMDensity = smoothNDensity = curvetype = 0;
		firstEH = lastEH = 0;
	}
	~DRW_Polyline()
	{
		while (!verts.empty())
		{
			verts.pop_back();
		}
	}
	void addVertex(DRW_Vertex v)
	{
		auto* vert = new DRW_Vertex();
		vert->basePoint.x = v.basePoint.x;
		vert->basePoint.y = v.basePoint.y;
		vert->basePoint.z = v.basePoint.z;
		vert->stawidth = v.stawidth;
		vert->endwidth = v.endwidth;
		vert->bulge = v.bulge;
		verts.push_back(vert);
	}
	void appendVertex(DRW_Vertex* v)
	{
		verts.push_back(v);
	}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	dint32 flags;				/*!< polyline flag, code 70, default 0 */
	dint16 curvetype;			/*!< curves & smooth surface type, code 75, default 0 */
	double defStartWidth;	/*!< Start width, code 40, default 0 */
	double defEndWidth;		/*!< End width, code 41, default 0 */
	dint16 vertCount;			//!BS 71 Number of vertices in the mesh or M vertex count default 0
	dint16 faceCount;			//!BS 72 Number of faces or N vertex count default 0
	dint16 smoothMDensity;		/*!< smooth surface M density, code 73, default 0 */
	dint16 smoothNDensity;		/*!< smooth surface N density, code 74, default 0 */

	std::vector<DRW_Vertex*> verts; /*!< vertex list */

public:
	std::vector<duint32> ownedObjhandles; //!list of handles, only in 2004+
	duint32 firstEH;               //!handle of first entity, only in pre-2004
	duint32 lastEH;                //!handle of last entity, only in pre-2004
	dwgHandle seqEndH;             //!handle of SEQEND entity
};


//! Class to handle spline entity
/*!
*  Class to handle spline entity
*  @author Rallaz
*/
class DRW_Spline : public DRW_Entity
{
	SET_ENTITY_FRIENDS
public:
	DRW_Spline()
	{
		dwgType = DRW::SPLINE;
		flags = knotsNum = ctlPtsNum = fitPtsNum = 0;
		tolknot = tolcontrol = tolfit = 0.0000001;
	}
	DRW_Spline(const DRW_Spline& _o) :DRW_Entity(_o)
	{
		this->normalVec = _o.normalVec;
		this->sTangent = _o.sTangent;
		this->eTangent = _o.eTangent;
		this->flags = _o.flags;
		this->knotsNum = _o.knotsNum;
		this->ctlPtsNum = _o.ctlPtsNum;
		this->fitPtsNum = _o.fitPtsNum;
		this->tolknot = _o.tolknot;
		this->tolcontrol = _o.tolcontrol;
		this->tolfit = _o.tolfit;
		this->knotslist = _o.knotslist;
		this->controllist = _o.controllist;
		this->fitlist = _o.fitlist;
	}
	~DRW_Spline()
	{
		while (!controllist.empty())
		{
			controllist.pop_back();
		}
		while (!fitlist.empty())
		{
			fitlist.pop_back();
		}
	}
	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	//    double ex;                /*!< normal vector x coordinate, code 210 */
	//    double ey;                /*!< normal vector y coordinate, code 220 */
	//    double ez;                /*!< normal vector z coordinate, code 230 */
	DRW_Coord normalVec; /*!< normal vector, code 210, 220, 230 */
	DRW_Coord sTangent;   /*!< start tangent, code 12, 22, 32 */
						 //    double tgsx;              /*!< start tangent x coordinate, code 12 */
						 //    double tgsy;              /*!< start tangent y coordinate, code 22 */
						 //    double tgsz;              /*!< start tangent z coordinate, code 32 */
	DRW_Coord eTangent;     /*!< end tangent, code 13, 23, 33 */
						 //    double tgex;              /*!< end tangent x coordinate, code 13 */
						 //    double tgey;              /*!< end tangent y coordinate, code 23 */
						 //    double tgez;              /*!< end tangent z coordinate, code 33 */
	int flags;           /*!< spline flag, code 70 */
	int degree;          /*!< degree of the spline, code 71 */
	dint32 knotsNum;     /*!< number of knots, code 72, default 0 */
	dint32 ctlPtsNum;    /*!< number of control points, code 73, default 0 */
	dint32 fitPtsNum;         /*!< number of fit points, code 74, default 0 */
	double tolknot;      /*!< knot tolerance, code 42, default 0.0000001 */
	double tolcontrol;   /*!< control point tolerance, code 43, default 0.0000001 */
	double tolfit;       /*!< fit point tolerance, code 44, default 0.0000001 */

	std::vector<double> knotslist;        /*!< knots list, code 40 */
	std::vector<std::shared_ptr<DRW_Coord>> controllist; /*!< control points list, code 10, 20 & 30 */
	std::vector<std::shared_ptr<DRW_Coord>> fitlist;     /*!< fit points list, code 11, 21 & 31 */

private:
	DRW_Coord* controlpoint; /*!< current control point to add data */
	DRW_Coord* fitpoint;     /*!< current fit point to add data */
};

//! Class to handle hatch loop
/*!
*  Class to handle hatch loop
*  @author Rallaz
*/
class DRW_HatchPath
{
public:
	DRW_HatchPath(int t)
	{
		pathFlag = t;
		edgesNum = 0;
	}

	DRW_HatchPath(const DRW_HatchPath& _o)
	{
		this->pathFlag = _o.pathFlag;
		this->edgesNum = _o.edgesNum;
		this->entities = _o.entities;
	}
	~DRW_HatchPath()
	{
		while (!entities.empty())
			entities.pop_back();
	}

	void updateEdgesNum()
	{
		edgesNum = static_cast<duint32>(entities.size());
	}

public:
	int pathFlag;
	duint32 edgesNum;
	//TODO: store lwpolylines as entities
	//    std::vector<DRW_LWPolyline *> pollist;  /*!< polyline list */
	std::vector<std::shared_ptr<DRW_Entity>> entities; /*!< entities list */
};

//! Class to handle hatch entity
/*!
*  Class to handle hatch entity
*  @author Rallaz
*/
//TODO: handle lwpolylines, splines and ellipses
class DRW_Hatch : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Hatch()
	{
		dwgType = DRW::HATCH;
		hatchAngle = scaleOrSpacing = 0.0;
		basePoint.x = basePoint.y = basePoint.z = 0.0;
		numPaths = hatchStyle = associative = 0;
		solidFill = hatchPatternType = 1;
		defLinesNum = doubleHatchFlag = 0;
		m_pHatchPath = nullptr;
		resetEntsPtrs();
	}
	DRW_Hatch(const DRW_Hatch& _o) :DRW_Point(_o)
	{
		this->name = _o.name;
		this->hatchAngle = _o.hatchAngle;
		this->scaleOrSpacing = _o.scaleOrSpacing;
		this->numPaths = _o.numPaths;
		this->hatchStyle = _o.hatchStyle;
		this->associative = _o.associative;
		this->solidFill = _o.solidFill;
		this->hatchPatternType = _o.hatchPatternType;
		this->defLinesNum = _o.defLinesNum;
		this->doubleHatchFlag = _o.doubleHatchFlag;
		this->paths = _o.paths;
	}

	~DRW_Hatch()
	{
		while (!paths.empty())
		{
			paths.pop_back();
		}
	}

	void appendLoop(DRW_HatchPath* v)
	{
		paths.emplace_back(v);
	}

	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	// R2004+
	dint32 isGradientFill;//Non-zero indicates a gradient fill is used.
	dint32 reversed;
	double gradientAngle;
	double shift;
	UTF8STRING name; /*!< hatch pattern name, code 2 */
	int solidFill;   /*!< solid fill flag, code 70, solid=1, pattern=0 */
	int associative; /*!< associativity, code 71, associatve=1, non-assoc.=0 */
	int hatchStyle;      /*!< hatch style, code 75 */
	int hatchPatternType;    /*!< hatch pattern type, code 76 */
	int doubleHatchFlag;  /*!< hatch pattern double flag, code 77, double=1, single=0 */
	int numPaths;    /*!< number of path enclosing the hatch, code 91 */
	double hatchAngle;    /*!< hatch pattern angle, code 52 */
	double scaleOrSpacing;    /*!< hatch pattern scale or spacing, code 41 */
	int defLinesNum;    /*!< number of pattern definition lines, code 78 */

	std::vector<std::shared_ptr<DRW_HatchPath>> paths; /*!< path vec */

private:
	void resetEntsPtrs()
	{
		m_pPt = m_pLine = nullptr;
		m_pLWPolyline = nullptr;
		m_pArc = nullptr;
		m_pEllipse = nullptr;
		m_pSpline = nullptr;
		m_pVertex2D = nullptr;
	}

	void addLine()
	{
		resetEntsPtrs();
		if (m_pHatchPath)
		{
			m_pPt = m_pLine = new DRW_Line;
			m_pHatchPath->entities.emplace_back(m_pLine);
		}
	}

	void addArc()
	{
		resetEntsPtrs();
		if (m_pHatchPath)
		{
			m_pPt = m_pArc = new DRW_Arc;
			m_pHatchPath->entities.emplace_back(m_pArc);
		}
	}

	void addEllipse()
	{
		resetEntsPtrs();
		if (m_pHatchPath)
		{
			m_pPt = m_pEllipse = new DRW_Ellipse;
			m_pHatchPath->entities.emplace_back(m_pEllipse);
		}
	}

	void addSpline()
	{
		resetEntsPtrs();
		if (m_pHatchPath)
		{
			m_pPt = nullptr;
			m_pSpline = new DRW_Spline;
			m_pHatchPath->entities.emplace_back(m_pSpline);
		}
	}

	DRW_HatchPath* m_pHatchPath; /*!< current loop to add data */
	DRW_Line* m_pLine;
	DRW_Arc* m_pArc;
	DRW_Ellipse* m_pEllipse;
	DRW_Spline* m_pSpline;
	DRW_LWPolyline* m_pLWPolyline;
	DRW_Point* m_pPt;
	DRW_Vertex2D* m_pVertex2D;
	bool ispol;
};

//! Class to handle image entity
/*!
*  Class to handle image entity
*  @author Rallaz
*/
class DRW_Image : public DRW_Line
{
	SET_ENTITY_FRIENDS
public:
	DRW_Image()
	{
		dwgType = DRW::IMAGE;
		fade = clip = 0;
		brightness = contrast = 50;
	}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	duint32 ref;       /*!< Hard reference to imagedef object, code 340 */
	DRW_Coord vVector; /*!< V-vector of single pixel, x coordinate, code 12, 22 & 32 */
					   //    double vx;                 /*!< V-vector of single pixel, x coordinate, code 12 */
					   //    double vy;                 /*!< V-vector of single pixel, y coordinate, code 22 */
					   //    double vz;                 /*!< V-vector of single pixel, z coordinate, code 32 */
	double sizeu;      /*!< image size in pixels, U value, code 13 */
	double sizev;      /*!< image size in pixels, V value, code 23 */
	double dz;         /*!< z coordinate, code 33 */
	int clip;          /*!< Clipping state, code 280, 0=off 1=on */
	int brightness;    /*!< Brightness value, code 281, (0-100) default 50 */
	int contrast;      /*!< Brightness value, code 282, (0-100) default 50 */
	int fade;          /*!< Brightness value, code 283, (0-100) default 0 */
};

//! Base class for dimension entity
/*!
*  Base class for dimension entity
*  @author Rallaz
*/
class DRW_Dimension : public DRW_Entity
{
	SET_ENTITY_FRIENDS
public:


	DRW_Dimension()
	{
		dwgType = DRW::UNUSED;
		type = 0;
		lineSpaceStyle = 1;
		lineSpaceFactor = extPoint.z = 1.0;
		horizDir = angle = oblique = textRot = 0.0;
		attchPt = 5;
		style = "STANDARD";
		pt10.z = extPoint.x = extPoint.y = 0;
		textPoint.z = textRot = 0;
		actualMeasurement = length = -1;
		clonePoint.x = clonePoint.y = clonePoint.z = 0;
	}

	DRW_Dimension(const DRW_Dimension& d) : DRW_Entity(d)
	{
		dwgType = DRW::UNUSED;
		type = d.type;
		name = d.name;
		pt10 = d.pt10;
		textPoint = d.textPoint;
		userText = d.userText;
		horizDir = d.horizDir;
		style = d.style;
		attchPt = d.attchPt;
		lineSpaceStyle = d.lineSpaceStyle;
		lineSpaceFactor = d.lineSpaceFactor;
		textRot = d.textRot;
		extPoint = d.extPoint;
		clonePoint = d.clonePoint;
		pt13 = d.pt13;
		pt14 = d.pt14;
		angle = d.angle;
		oblique = d.oblique;
		arcPoint = d.arcPoint;
		pt15 = d.pt15;
		length = d.length;
		actualMeasurement = d.actualMeasurement;
		dimStyleH = d.dimStyleH;
		blockH = d.blockH;
		//RLZ needed a def value for this: horizDir = ???
	}
	virtual ~DRW_Dimension()
	{}

	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* sBuf);

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override
	{
		DRW_UNUSED(version);
		DRW_UNUSED(buf);
		DRW_UNUSED(bs);
		return true;
	}

public:
	DRW_Coord get10pt() const
	{
		return pt10;
	}
	/*!< dimension location point, code 10, 20 & 30 */
	void set10pt(const DRW_Coord p)
	{
		pt10 = p;
	}
	DRW_Coord getTextPoint() const
	{
		return textPoint;
	}

	/*!< Middle point of text, code 11, 21 & 31 */
	void setTextPoint(const DRW_Coord p)
	{
		textPoint = p;
	}

	std::string getStyle() const
	{
		return style;
	}
	/*!< Dimension style, code 3 */
	void setStyle(const std::string s)
	{
		style = s;
	}
	int getAlign() const
	{
		return attchPt;
	}
	/*!< attachment point, code 71 */
	void setAlign(const int a)
	{
		attchPt = a;
	}
	int getTextLineStyle() const
	{
		return lineSpaceStyle;
	}
	/*!< Dimension text line spacing style, code 72, default 1 */
	void setTextLineStyle(const int l)
	{
		lineSpaceStyle = l;
	}
	std::string getText() const
	{
		return userText;
	}
	/*!< Dimension text explicitly entered by the user, code 1 */
	void setText(const std::string t)
	{
		userText = t;
	}
	double getTextLineFactor() const
	{
		return lineSpaceFactor;
	}
	/*!< Dimension text line spacing factor, code 41, default 1? */
	void setTextLineFactor(const double l)
	{
		lineSpaceFactor = l;
	}
	double getDir() const
	{
		return textRot;
	}

	/*!< rotation angle of the dimension text, code 53 (optional) default 0 */
	void setDir(const double d)
	{
		textRot = d;
	}

	DRW_Coord getExtrusion()
	{
		return extPoint;
	}
	/*!< extrusion, code 210, 220 & 230 */
	void setExtrusion(const DRW_Coord p)
	{
		extPoint = p;
	}
	std::string getName()
	{
		return name;
	}
	/*!< Name of the block that contains the entities, code 2 */
	void setName(const std::string s)
	{
		name = s;
	}
	//    int getType(){ return type;}                      /*!< Dimension type, code 70 */

	// actual measurement or user text edited
	double getActualMeasurement()
	{
		return actualMeasurement;
	}

	UTF8STRING getUserText()
	{
		return userText;
	}

	double getAngle() const
	{
		return angle;
	}

	/*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
	void setAngle(const double d)
	{
		angle = d;
	}


	double getOblique() const
	{
		return oblique;
	}

	/*!< oblique angle, code 52 */
	void setOblique(const double d)
	{
		oblique = d;
	}

protected:

	DRW_Coord getPt2() const
	{
		return clonePoint;
	}

	void setPt2(const DRW_Coord p)
	{
		clonePoint = p;
	}

	DRW_Coord get13pt() const
	{
		return pt13;
	}

	void set13pt(const DRW_Coord p)
	{
		pt13 = p;
	}

	DRW_Coord get14pt() const
	{
		return pt14;
	}

	void set14pt(const DRW_Coord p)
	{
		pt14 = p;
	}

	DRW_Coord getPt15() const
	{
		return pt15;
	}

	void setPt15(const DRW_Coord p)
	{
		pt15 = p;
	}

	DRW_Coord getPt16() const
	{
		return arcPoint;
	}

	void setPt16(const DRW_Coord p)
	{
		arcPoint = p;
	}



	double getRa40() const
	{
		return length;
	}

	/*!< Leader length, code 40 */
	void setRa40(const double d)
	{
		length = d;
	}

public:
	int type;				/*!< Dimension type, code 70 */
private:
	std::string name;       /*!< Name of the block that contains the entities, code 2 */
	DRW_Coord pt10;			/*!< dimension location, code 10, 20 & 30 (WCS) */
	DRW_Coord textPoint;    /*!< Middle point of text, code 11, 21 & 31 (OCS) */
	UTF8STRING userText;    /*!< Dimension text explicitly entered by the user, code 1 */
	UTF8STRING style;       /*!< Dimension style, code 3 */
	int attchPt;            /*!< attachment point, code 71 */
	int lineSpaceStyle;     /*!< Dimension text line spacing style, code 72, default 1 */
	double lineSpaceFactor; /*!< Dimension text line spacing factor, code 41, default 1? (value range 0.25 to 4.00*/
	double textRot;         /*!< rotation angle of the dimension text, code 53 */
	DRW_Coord extPoint;     /*!< extrusion normal vector, code 210, 220 & 230 */

	double horizDir;		/*!< horizontal direction for the dimension, code 51, default ? */
	DRW_Coord clonePoint;	/*!< Insertion point for clones (Baseline & Continue), code 12, 22 & 32 (OCS) */
	DRW_Coord pt13;			/*!< Definition point 1 for linear & angular, code 13, 23 & 33 (WCS) */
	DRW_Coord pt14;			/*!< Definition point 2, code 14, 24 & 34 (WCS) */
	double angle;			/*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
	double oblique;			/*!< oblique angle, code 52 */

	DRW_Coord pt15;         /*!< Definition point for diameter, radius & angular dims code 15, 25 & 35 (WCS) */
	DRW_Coord arcPoint;     /*!< Point defining dimension arc, x coordinate, code 16, 26 & 36 (OCS) */
	double length;          /*!< Leader length, code 40 */
	double actualMeasurement; /*!< actual measurement>*/

protected:
	dwgHandle dimStyleH;
	dwgHandle blockH;
};

//! Class to handle aligned dimension entity
/*!
*  Class to handle aligned dimension entity
*  @author Rallaz
*/
class DRW_DimAligned : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimAligned()
	{
		dwgType = DRW::DIMENSION_ALIGNED;
	}
	DRW_DimAligned(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ALIGNED;
	}
	DRW_DimAligned(const DRW_DimAligned& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ALIGNED;
	}

	DRW_Coord getClonepoint() const
	{
		return getPt2();
	} /*!< Insertion for clones (Baseline & Continue), 12, 22 & 32 */
	void setClonePoint(DRW_Coord c)
	{
		setPt2(c);
	}

	DRW_Coord getDimPoint() const
	{
		return get10pt();
	} /*!< dim line location point, code 10, 20 & 30 */
	void setDimPoint(const DRW_Coord p)
	{
		set10pt(p);
	}
	DRW_Coord getDef1Point() const
	{
		return get13pt();
	} /*!< Definition point 1, code 13, 23 & 33 */
	void setDef1Point(const DRW_Coord p)
	{
		set13pt(p);
	}
	DRW_Coord getDef2Point() const
	{
		return get14pt();
	} /*!< Definition point 2, code 14, 24 & 34 */
	void setDef2Point(const DRW_Coord p)
	{
		set14pt(p);
	}

protected:
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;
};

//! Class to handle  linear or rotated dimension entity
/*!
*  Class to handle linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimLinear : public DRW_DimAligned
{
public:
	DRW_DimLinear()
	{
		dwgType = DRW::DIMENSION_LINEAR;
	}
	DRW_DimLinear(const DRW_DimAligned& d) : DRW_DimAligned(d)
	{
		dwgType = DRW::DIMENSION_LINEAR;
	}
	DRW_DimLinear(const DRW_DimLinear& d) : DRW_DimAligned(d)
	{
		dwgType = DRW::DIMENSION_LINEAR;
	}
};

//! Class to handle radial dimension entity
/*!
*  Class to handle aligned, linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimRadius : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimRadius()
	{
		dwgType = DRW::DIMENSION_RADIUS;
	}
	DRW_DimRadius(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_RADIUS;
	}
	DRW_DimRadius(const DRW_DimRadius& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_RADIUS;
	}

	DRW_Coord getCenterPoint() const
	{
		return get10pt();
	}
	/*!< center point, code 10, 20 & 30 */
	void setCenterPoint(const DRW_Coord p)
	{
		set10pt(p);
	}
	DRW_Coord getDiameterPoint() const
	{
		return getPt15();
	}
	/*!< Definition point for radius, code 15, 25 & 35 */
	void setDiameterPoint(const DRW_Coord p)
	{
		setPt15(p);
	}
	double getLeaderLength() const
	{
		return getRa40();
	}
	/*!< Leader length, code 40 */
	void setLeaderLength(const double d)
	{
		setRa40(d);
	}

protected:
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;
};

//! Class to handle radial dimension entity
/*!
*  Class to handle aligned, linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimDiameter : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimDiameter()
	{
		dwgType = DRW::DIMENSION_DIAMETER;
	}
	DRW_DimDiameter(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_DIAMETER;
	}
	DRW_DimDiameter(const DRW_DimDiameter& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_DIAMETER;
	}

	DRW_Coord getDiameter1Point() const
	{
		return getPt15();
	}
	/*!< First definition point for diameter, code 15, 25 & 35 */
	void setDiameter1Point(const DRW_Coord p)
	{
		setPt15(p);
	}
	DRW_Coord getDiameter2Point() const
	{
		return get10pt();
	}
	/*!< opposite point for diameter, code 10, 20 & 30 */
	void setDiameter2Point(const DRW_Coord p)
	{
		set10pt(p);
	}
	double getLeaderLength() const
	{
		return getRa40();
	}
	/*!< Leader length, code 40 */
	void setLeaderLength(const double d)
	{
		setRa40(d);
	}

protected:
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);
};

//! Class to handle angular dimension entity
/*!
*  Class to handle angular dimension entity
*  @author Rallaz
*/
class DRW_DimAngular : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimAngular() : DRW_Dimension()
	{
		dwgType = DRW::DIMENSION_ANG2LN;
	}

	DRW_DimAngular(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ANG2LN;
	}

	DRW_Coord getFirstLine1() const
	{
		return get13pt();
	}

	/*!< Definition point line 1-1, code 13, 23 & 33 */
	void setFirstLine1(const DRW_Coord p)
	{
		set13pt(p);
	}

	DRW_Coord getFirstLine2() const
	{
		return get14pt();
	}

	/*!< Definition point line 1-2, code 14, 24 & 34 */
	void setFirstLine2(const DRW_Coord p)
	{
		set14pt(p);
	}

	DRW_Coord getSecondLine1() const
	{
		return getPt15();
	}

	/*!< Definition point line 2-1, code 15, 25 & 35 */
	void setSecondLine1(const DRW_Coord p)
	{
		setPt15(p);
	}

	DRW_Coord getSecondLine2() const
	{
		return get10pt();
	}

	/*!< Definition point line 2-2, code 10, 20 & 30 */
	void setSecondLine2(const DRW_Coord p)
	{
		set10pt(p);
	}

	DRW_Coord getDimPoint() const
	{
		return getPt16();
	}

	/*!< Dimension definition point, code 16, 26 & 36 */
	void setDimPoint(const DRW_Coord p)
	{
		setPt16(p);
	}

protected:
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;
};

//! Class to handle angular 3p dimension entity
/*!
*  Class to handle angular 3p dimension entity
*  @author Rallaz
*/
class DRW_DimAngular3p : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimAngular3p()
	{
		dwgType = DRW::DIMENSION_ANG3PT;
	}
	DRW_DimAngular3p(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ANG3PT;
	}
	DRW_DimAngular3p(const DRW_DimAngular3p& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ANG3PT;
	}

	DRW_Coord getFirstLine() const
	{
		return get13pt();
	}
	/*!< Definition point line 1, code 13, 23 & 33 */
	void setFirstLine(const DRW_Coord p)
	{
		set13pt(p);
	}
	DRW_Coord getSecondLine() const
	{
		return get14pt();
	}
	/*!< Definition point line 2, code 14, 24 & 34 */
	void setSecondLine(const DRW_Coord p)
	{
		set14pt(p);
	}
	DRW_Coord getVertexPoint() const
	{
		return getPt15();
	}
	/*!< Vertex point, code 15, 25 & 35 */
	void SetVertexPoint(const DRW_Coord p)
	{
		setPt15(p);
	}
	DRW_Coord getDimPoint() const
	{
		return get10pt();
	}
	/*!< Dimension definition point, code 10, 20 & 30 */
	void setDimPoint(const DRW_Coord p)
	{
		set10pt(p);
	}

protected:
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);
};

//! Class to handle ordinate dimension entity
/*!
*  Class to handle ordinate dimension entity
*  @author Rallaz
*/
class DRW_DimOrdinate : public DRW_Dimension
{
	SET_ENTITY_FRIENDS
public:
	DRW_DimOrdinate()
	{
		dwgType = DRW::DIMENSION_ORDINATE;
	}
	DRW_DimOrdinate(const DRW_Dimension& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ORDINATE;
	}
	DRW_DimOrdinate(const DRW_DimOrdinate& d) : DRW_Dimension(d)
	{
		dwgType = DRW::DIMENSION_ORDINATE;
	}

	DRW_Coord getOriginPoint() const
	{
		return get10pt();
	} /*!< Origin definition point, code 10, 20 & 30 */
	void setOriginPoint(const DRW_Coord p)
	{
		set10pt(p);
	}
	DRW_Coord getFirstLine() const
	{
		return get13pt();
	} /*!< Feature location point, code 13, 23 & 33 */
	void setFirstLine(const DRW_Coord p)
	{
		set13pt(p);
	}
	DRW_Coord getSecondLine() const
	{
		return get14pt();
	} /*!< Leader end point, code 14, 24 & 34 */
	void setSecondLine(const DRW_Coord p)
	{
		set14pt(p);
	}

protected:
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);
};

//! Class to handle leader entity
/*!
*  Class to handle leader entity
*  @author Rallaz
*/
class DRW_Leader : public DRW_Entity
{
	SET_ENTITY_FRIENDS
public:
	DRW_Leader()
	{
		dwgType = DRW::LEADER;
		flag = 3;
		hookflag = vertnum = leadertype = 0;
		extrusionPoint.x = extrusionPoint.y = 0.0;
		arrow = 1;
		extrusionPoint.z = 1.0;
	}
	~DRW_Leader()
	{
		while (!vertexlist.empty())
		{
			vertexlist.pop_back();
		}
	}

	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	UTF8STRING style;         /*!< Dimension style name, code 3 */
	int arrow;                /*!< Arrowhead flag, code 71, 0=Disabled; 1=Enabled */
	int leadertype;           /*!< Leader path type, code 72, 0=Straight line segments; 1=Spline */
	int flag;                 /*!< Leader creation flag, code 73, default 3 */
	int hookline;             /*!< Hook line direction flag, code 74, default 1 */
	int hookflag;             /*!< Hook line flag, code 75 */
	double textheight;        /*!< Text annotation height, code 40 */
	double textwidth;         /*!< Text annotation width, code 41 */
	int vertnum;              /*!< Number of vertices, code 76 */
	int coloruse;             /*!< Color to use if leader's DIMCLRD = BYBLOCK, code 77 */
	duint32 annotHandle;      /*!< Hard reference to associated annotation, code 340 */
	DRW_Coord extrusionPoint; /*!< Normal vector, code 210, 220 & 230 */
	DRW_Coord horizdir;       /*!< "Horizontal" direction for leader, code 211, 221 & 231 */
	DRW_Coord offsetblock;    /*!< Offset of last leader vertex from block, code 212, 222 & 232 */
	DRW_Coord offsettext;     /*!< Offset of last leader vertex from annotation, code 213, 223 & 233 */

	std::vector<DRW_Coord*> vertexlist; /*!< vertex points list, code 10, 20 & 30 */

private:
	DRW_Coord* vertexpoint; /*!< current control point to add data */
	dwgHandle dimStyleH;
	dwgHandle AnnotH;
};

//! Class to handle viewport entity
/*!
*  Class to handle viewport entity
*  @author Rallaz
*/
class DRW_Viewport : public DRW_Point
{
	SET_ENTITY_FRIENDS
public:
	DRW_Viewport()
	{
		dwgType = DRW::DwgType::VIEWPORT;
		vpstatus = 0;
		pswidth = 205;
		psheight = 156;
		centerPX = 128.5;
		centerPY = 97.5;
	}

	virtual void applyExtrusion()
	{}

protected:
	void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	double pswidth;  /*!< Width in paper space units, code 40 */
	double psheight; /*!< Height in paper space units, code 41 */
	int vpstatus;    /*!< Viewport status, code 68 */
	int vpID;        /*!< Viewport ID, code 69 */
	double centerPX; /*!< view center point X, code 12 */
	double centerPY; /*!< view center point Y, code 22 */
	double snapPX;   /*!< Snap base point X, code 13 */
	double snapPY;   /*!< Snap base point Y, code 23 */
	double snapSpPX; /*!< Snap spacing X, code 14 */
	double snapSpPY; /*!< Snap spacing Y, code 24 */
	//TODO: complete in dxf
	DRW_Coord viewDir;    /*!< View direction vector, code 16, 26 & 36 */
	DRW_Coord viewTarget; /*!< View target point, code 17, 27, 37 */
	double viewLength;    /*!< Perspective lens length, code 42 */
	double frontClip;     /*!< Front clip plane Z value, code 43 */
	double backClip;      /*!< Back clip plane Z value, code 44 */
	double viewHeight;    /*!< View height in model space units, code 45 */
	double snapAngle;     /*!< Snap angle, code 50 */
	double twistAngle;    /*!< view twist angle, code 51 */

private:
	duint32 frozenLyCount;
}; //RLZ: missing 15,25, 72, 331, 90, 340, 1, 281, 71, 74, 110, 120, 130, 111, 121,131, 112,122, 132, 345,346, and more...


