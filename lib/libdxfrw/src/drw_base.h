
#ifndef DRW_BASE_H
#define DRW_BASE_H

#define DRW_VERSION "0.6.3"
#pragma warning(disable : 4996) //Ignore C4996, unsafe strncpy. TODO use safe alternative
#include <cmath>
#include <list>
#include <memory>
#include <string>
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/spdlog.h"

#ifdef DRW_ASSERTS
#define drw_assert(a) assert(a)
#else
#define drw_assert(a)
#endif

#define UTF8STRING std::string
#define DRW_UNUSED(x) (void)x

// check platform
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define DRW_WIN
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define DRW_WIN
#elif defined(__MWERKS__) && defined(__INTEL__)
#define DRW_WIN
#else
#define DRW_POSIX
#endif

#ifndef M_E
#define M_E        2.71828182845904523536   // e
#endif // !M_E
#ifndef M_LOG2E
#define M_LOG2E    1.44269504088896340736   // log2(e)
#endif // !M_LOG2E
#ifndef M_LOG10E
#define M_LOG10E   0.434294481903251827651  // log10(e)
#endif // !M_LOG10E
#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif	// !M_PI
#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923   // pi/2
#endif	// !M_PI_2
#ifndef M_PIx2
#define M_PIx2		6.283185307179586		// 2*PI
#endif // !M_PIx2
#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616  // pi/4
#endif // !M_PI_4
#ifndef M_LN2
#define M_LN2      0.693147180559945309417  // ln(2)
#endif // !M_LN2
#ifndef M_LN10
#define M_LN10     2.30258509299404568402   // ln(10)
#endif // !M_LN10
#ifndef M_1_PI
#define M_1_PI     0.318309886183790671538  // 1/pi
#endif // !M_1_PI
#ifndef M_2_PI
#define M_2_PI     0.636619772367581343076  // 2/pi
#endif // !M_2_PI
#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#endif // !M_2_SQRTPI
#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#endif // !M_SQRT2
#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif // !M_SQRT1_2

#define ARAD 57.29577951308232

#define ang2Deg(ang) ((ang)*90.0/M_PI_2)
#define deg2Ang(deg) ((deg)*M_PI_2/90.0)



// memory leak detection
//#define LEAK_DETECTION new (_CLIENT_BLOCK, __FILE__, __LINE__)
//#define new LEAK_DETECTION

typedef signed char dint8;    /* 8 bit signed */
typedef signed short dint16;  /* 16 bit signed */
typedef signed int dint32;    /* 32 bit signed */
typedef long long int dint64; /* 64 bit signed */

typedef unsigned char duint8;           /* 8 bit unsigned */
typedef unsigned short duint16;         /* 16 bit unsigned */
typedef unsigned int duint32;           /* 32 bit unsigned */
typedef unsigned long long int duint64; /* 64 bit unsigned */

typedef float dfloat32;        /* 32 bit floating point */
typedef double ddouble64;      /* 64 bit floating point */
typedef long double ddouble80; /* 80 bit floating point */

namespace DRW
{

	//! Version numbers for the DXF Format.
	typedef enum emAcadVersion
	{
		UNKNOWN_VERSION = -1, /*!< UNKNOWN VERSION>. */
		AC1006 = 1006,	/*!< R10. */
		AC1009 = 1009,	/*!< R11 & R12. */
		R13 = 1012,		/*!< R13. */
		R14 = 1014,		/*!< R14. */
		R2000 = 2000,	/*!< ACAD 2000. */
		R2004 = 2004,	/*!< ACAD 2004. */
		R2007 = 2007,	/*!< ACAD 2007. */
		R2010 = 2010,	/*!< ACAD 2010. */
		R2013 = 2013,	/*!< ACAD 2013. */
		R2018 = 2018	/*!< ACAD 2018>.*/
	} Version;

	enum class DwgSentinel
	{
		DWG_SENTINEL_HEADER_END,
		DWG_SENTINEL_THUMBNAIL_BEGIN,
		DWG_SENTINEL_THUMBNAIL_END,
		DWG_SENTINEL_VARIABLE_BEGIN,
		DWG_SENTINEL_VARIABLE_END,
		DWG_SENTINEL_CLASS_BEGIN,
		DWG_SENTINEL_CLASS_END,
		DWG_SENTINEL_SECOND_HEADER_BEGIN,
		DWG_SENTINEL_SECOND_HEADER_END
	};

	inline unsigned char* dwgSentinel(DwgSentinel s)
	{
		static unsigned char sentinels[9][16] = { \
			// DWG_SENTINEL_HEADER_END
			{ 0x95, 0xA0, 0x4E, 0x28, 0x99, 0x82, 0x1A, 0xE5, 0x5E, 0x41, 0xE0, 0x5F,
			  0x9D, 0x3A, 0x4D, 0x00 },\
			// DWG_SENTINEL_THUMBNAIL_BEGIN
			{ 0x1F, 0x25, 0x6D, 0x07, 0xD4, 0x36, 0x28, 0x28, 0x9D, 0x57, 0xCA, 0x3F,
			  0x9D, 0x44, 0x10, 0x2B },\
			// DWG_SENTINEL_THUMBNAIL_END
			{ 0xE0, 0xDA, 0x92, 0xF8, 0x2B, 0xc9, 0xD7, 0xD7, 0x62, 0xA8, 0x35, 0xC0,
			  0x62, 0xBB, 0xEF, 0xD4 },\
			// DWG_SENTINEL_VARIABLE_BEGIN
			{ 0xCF, 0x7B, 0x1F, 0x23, 0xFD, 0xDE, 0x38, 0xA9, 0x5F, 0x7C, 0x68, 0xB8,
			  0x4E, 0x6D, 0x33, 0x5F },\
			// DWG_SENTINEL_VARIABLE_END
			{ 0x30, 0x84, 0xE0, 0xDC, 0x02, 0x21, 0xC7, 0x56, 0xA0, 0x83, 0x97, 0x47,
			  0xB1, 0x92, 0xCC, 0xA0 },\
			// DWG_SENTINEL_CLASS_BEGIN
			{ 0x8D, 0xA1, 0xC4, 0xB8, 0xC4, 0xA9, 0xF8, 0xC5, 0xC0, 0xDC, 0xF4, 0x5F,
			  0xE7, 0xCF, 0xB6, 0x8A },\
			// DWG_SENTINEL_CLASS_END
			{ 0x72, 0x5E, 0x3B, 0x47, 0x3B, 0x56, 0x07, 0x3A, 0x3F, 0x23, 0x0B, 0xA0,
			  0x18, 0x30, 0x49, 0x75 },\
			// DWG_SENTINEL_SECOND_HEADER_BEGIN
			{ 0xD4, 0x7B, 0x21, 0xCE, 0x28, 0x93, 0x9F, 0xBF, 0x53, 0x24, 0x40, 0x09,
			  0x12, 0x3C, 0xAA, 0x01 },\
			// DWG_SENTINEL_SECOND_HEADER_END
			{ 0x2B, 0x84, 0xDE, 0x31, 0xD7, 0x6C, 0x60, 0x40, 0xAC, 0xDB, 0xBF, 0xF6,
			  0xED, 0xC3, 0x55, 0xFE }\
		};
		return (sentinels[static_cast<int>(s)]);
	}


	enum class Error
	{
		BAD_NONE,             /*!< No error. */
		BAD_UNKNOWN,          /*!< UNKNOWN. */
		BAD_OPEN,             /*!< error opening file. */
		BAD_VERSION,          /*!< unsupported version. */
		BAD_READ_METADATA,    /*!< error reading metadata. */
		BAD_READ_FILE_HEADER, /*!< error in file header read process. */
		BAD_READ_HEADER,      /*!< error in header vars read process. */
		BAD_READ_HANDLES,     /*!< error in object map read process. */
		BAD_READ_CLASSES,     /*!< error in classes read process. */
		BAD_READ_TABLES,      /*!< error in tables read process. */
		BAD_READ_BLOCKS,      /*!< error in block read process. */
		BAD_READ_ENTITIES,    /*!< error in entities read process. */
		BAD_READ_OBJECTS      /*!< error in objects read process. */
	};

	enum DBG_LEVEL
	{
		NONE,
		DEBUG
	};

	//! Special codes for colors
	enum ColorCodes
	{
		ColorByLayer = 256,
		ColorByBlock = 0
	};

	//! Spaces
	enum Space
	{
		PaperSpace = 1,
		ModelSpace = 2
	};

	//! Special kinds of handles
	enum HandleCodes
	{
		NoHandle = 0
	};

	//! Shadow mode
	enum ShadowMode
	{
		CastAndReceieveShadows = 0,
		CastShadows = 1,
		ReceiveShadows = 2,
		IgnoreShadows = 3
	};

	//! Special kinds of materials
	enum MaterialCodes
	{
		MaterialByLayer = 0
	};

	//! Special kinds of plot styles
	enum PlotStyleCodes
	{
		DefaultPlotStyle = 0
	};

	//! Special kinds of transparencies
	enum TransparencyCodes
	{
		Opaque = 0,
		Transparent = -1
	};

	//! Entity's type.
	enum DwgType
	{
		UNUSED = 0x00,
		TEXT = 0x01,
		ATTRIB = 0x02,
		ATTDEF = 0x03,
		BLOCK = 0x04,
		ENDBLK = 0x05,
		SEQEND = 0x06,
		INSERT = 0x07,
		MINSERT = 0x08,
		//9
		VERTEX_2D = 0x0a,
		VERTEX_3D = 0x0b,
		VERTEX_MESH = 0x0c,
		VERTEX_PFACE = 0x0d,
		VERTEX_PFACE_FACE = 0x0e,
		POLYLINE_2D = 0x0f,
		POLYLINE_3D = 0x10,
		ARC = 0x11,
		CIRCLE = 0x12,
		LINE = 0x13,
		DIMENSION_ORDINATE = 0x14,
		DIMENSION_LINEAR = 0x15,
		DIMENSION_ALIGNED = 0x16,
		DIMENSION_ANG3PT = 0x17,
		DIMENSION_ANG2LN = 0x18,
		DIMENSION_RADIUS = 0x19,
		DIMENSION_DIAMETER = 0x1a,
		POINT = 0x1b,
		FACE_3D = 0x1c,
		POLYLINE_PFACE = 0x1d,
		POLYLINE_MESH = 0x1e,
		SOLID = 0x1f,
		TRACEE = 0x20,
		SHAPE = 0x21,
		VIEWPORT = 0x22,
		ELLIPSE = 0x23,
		SPLINE = 0x24,
		REGION = 0x25,
		SOLID_3D = 0x26,
		BODY = 0x27,
		RAY = 0x28,
		XLINE = 0x29,
		DICTIONARY = 0x2a,
		OLEFRAME = 0x2b,
		MTEXT = 0x2c,
		LEADER = 0x2d,
		TOLERANCE = 0x2e,
		MLINE = 0x2f,

		// control objects at another place
		BLOCK_CONTROL_OBJ = 0x30,
		BLOCK_HEADER = 0x31,
		LAYER_CONTROL_OBJ = 0x32,
		LAYER = 0x33,
		STYLE_CONTROL_OBJ = 0x34,
		STYLE = 0x35,
		//36,37
		LTYPE_CONTROL_OBJ = 0x38,
		LTYPE = 0x39,
		//3a,3b
		VIEW_CONTROL_OBJ = 0x3c,
		VIEW = 0x3d,
		UCS_CONTROL_OBJ = 0x3e,
		UCS = 0x3f,
		VPORT_CONTROL_OBJ = 0x40,
		VPORT = 0x41,
		APPID_CONTROL_OBJ = 0x42,
		APPID = 0x43,
		DIMSTYLE_CONTROL_OBJ = 0x44,
		DIMSTYLE = 0x45,
		VP_ENTITY_HEADER_CONTROL_OBJ = 0x46,
		VP_ENTITY_HEADER = 0x47,

		GROUP = 0x48,
		MLINESTYLE = 0x49,
		OLE2FRAME = 0x4a,
		DUMMY = 0x4b,
		LONG_TRANSACTION = 0x4c,
		LWPOLYLINE = 0x4d,
		HATCH = 0x4e,
		XRECORD = 0x4f,
		ACDBPLACEHOLDER = 0x50,
		VBA_PROJECT = 0x51,
		LAYOUT = 0x52,

		ACAD_PROXY_ENTITY = 0x1F2,
		ACAD_PROXY_OBJECT = 0x1F3,

		ACDSRECORD = 0x1ff + 1,
		ACDSSCHEMA,
		ACMECOMMANDHISTORY,
		ACMESCOPE,
		ACMESTATEMGR,
		ACSH_BOX_CLASS,
		ACSH_EXTRUSION_CLASS,
		ACSH_HISTORY_CLASS,
		ACSH_REVOLVE_CLASS,
		ACSH_SPHERE_CLASS,
		ACSH_SWEEP_CLASS,
		ANNOTSCALEOBJECTCONTEXTDATA,
		ARCALIGNEDTEXT,
		ARC_DIMENSION,
		ASSOC2DCONSTRAINTGROUP,
		ASSOCACTION,
		ASSOCALIGNEDDIMACTIONBODY,
		ASSOCDEPENDENCY,
		ASSOCGEOMDEPENDENCY,
		ASSOCNETWORK,
		ASSOCOSNAPPOINTREFACTIONPARAM,
		ASSOCPERSSUBENTMANAGER,
		ASSOCPLANESURFACEACTIONBODY,
		ASSOCVERTEXACTIONPARAM,
		CAMERA,
		CELLSTYLEMAP,
		DATALINK,
		DATATABLE,
		DBCOLOR,
		DETAILVIEWSTYLE,
		DICTIONARYVAR,
		DICTIONARYWDFLT,
		DIMASSOC,
		DOCUMENTOPTIONS,
		DYNAMICBLOCKPURGEPREVENTER,
		EVALUATION_GRAPH,
		FIELD,
		FIELDLIST,
		GEODATA,
		GEOMAPIMAGE,
		GEOPOSITIONMARKER,
		HELIX,
		IDBUFFER,
		IMAGE,
		IMAGEDEF,
		IMAGEDEF_REACTOR,
		LAYER_FILTER,
		LAYER_INDEX,
		LAYOUTPRINTCONFIG,
		LIGHT,
		LIGHTLIST,
		MATERIAL,
		MESH,
		MLEADERSTYLE,
		MULTILEADER,
		NAVISWORKSMODELDEF,
		NPOCOLLECTION,
		OBJECTCONTEXTDATA,
		OBJECT_PTR,
		PERSSUBENTMANAGER,
		PLOTSETTINGS,
		POINTCLOUD,
		RASTERVARIABLES,
		RENDERENVIRONMENT,
		RENDERGLOBAL,
		MENTALRAYRENDERSETTINGS,
		RAPIDRTRENDERENVIRONMENT,
		RAPIDRTRENDERSETTINGS,
		RTEXT,
		SCALE,
		SECTIONVIEWSTYLE,
		SORTENTSTABLE,
		SPATIAL_FILTER,
		SPATIAL_INDEX,
		SUN,
		SUNSTUDY,
		SURFACE,
		PLANESURFACE,
		EXTRUDEDSURFACE,
		LOFTEDSURFACE,
		REVOLVEDSURFACE,
		SWEPTSURFACE,
		TABLE,
		TABLECONTENT,
		TABLEGEOMETRY,
		TABLESTYLE,
		UNDERLAY,
		UNDERLAYDEFINITION,
		VISUALSTYLE,
		WIPEOUT,
		WIPEOUTVARIABLES,
		XREFPANELOBJECT,

		FREED = 0xfffd,
		UNKNOWN_ENT = 0xfffe,
		UNKNOWN_OBJ = 0xffff,
	};
};// namespace DRW

//! Class to handle 3D coordinate point
/*!
*  Class to handle 3D coordinate point
*  @author Rallaz
*/
class DRW_Coord
{
public:
	DRW_Coord() : x(0), y(0), z(0)
	{}
	DRW_Coord(double ix, double iy, double iz) : x(ix), y(iy), z(iz)
	{}

	DRW_Coord& operator=(const DRW_Coord& data)
	{
		x = data.x;
		y = data.y;
		z = data.z;
		return *this;
	}
	/*!< convert to unitary vector */
	void unitize()
	{
		double dist;
		dist = sqrt(x * x + y * y + z * z);
		if (dist > 0.0)
		{
			x = x / dist;
			y = y / dist;
			z = z / dist;
		}
	}

public:
	double x;
	double y;
	double z;  // elevation
};

//! Class to handle vertex
/*!
*  Class to handle vertex for lwpolyline entity
*  @author Rallaz
*/
class DRW_Vertex2D
{
public:
	DRW_Vertex2D(double sx = 0, double sy = 0, double sbugle = 0) :
		x(sx), y(sy),
		stawidth(0),
		endwidth(0),
		bulge(sbugle),
		vertexId(0)
	{}

public:
	double x;        /*!< x coordinate, code 10 */
	double y;        /*!< y coordinate, code 20 */
	double stawidth; /*!< Start width, code 40 */
	double endwidth; /*!< End width, code 41 */
	double bulge;    /*!< bulge, code 42 */
	// R2010+
	duint32 vertexId;
};

//! Class to handle header vars
/*!
*  Class to handle header vars
*  @author Rallaz
*/
class DRW_Variant
{
public:
	enum VariantType
	{
		INVALID,
		STRING,
		INTEGER,
		DOUBLE,
		COORD
	};

	DRW_Variant() : str(std::string()), crd(), content(0), vType(INVALID), vCode(0)
	{}

	DRW_Variant(int c, dint32 i) : str(std::string()), crd(), content(i), vType(INTEGER), vCode(c)
	{}

	DRW_Variant(int c, duint32 i) : str(std::string()), crd(), content(static_cast<dint32>(i)), vType(INTEGER), vCode(c)
	{}

	DRW_Variant(int c, dint64 i) : str(std::string()), crd(), content(i), vType(INTEGER), vCode(c)
	{}

	DRW_Variant(int c, duint64 i) : str(std::string()), crd(), content(static_cast<dint64>(i)), vType(INTEGER), vCode(c)
	{}

	DRW_Variant(int c, double d) : str(std::string()), crd(), content(d), vType(DOUBLE), vCode(c)
	{}

	DRW_Variant(int c, UTF8STRING s) : str(s), crd(), content(&str), vType(STRING), vCode(c)
	{}

	DRW_Variant(int c, DRW_Coord crd) : str(std::string()), crd(crd), content(&crd), vType(COORD), vCode(c)
	{}

	DRW_Variant(const DRW_Variant& d) : str(d.str), crd(d.crd), content(d.content), vType(d.vType), vCode(d.vCode)
	{
		if (d.vType == COORD)
			content.pCrd = &crd;
		if (d.vType == STRING)
			content.pStr = &str;
	}

	~DRW_Variant()
		= default;

	void addString(int c, UTF8STRING s)
	{
		vType = STRING;
		str = s;
		content.pStr = &str;
		vCode = c;
	}
	void addInt(int c, int i)
	{
		vType = INTEGER;
		content.i = i;
		vCode = c;
	}

	void addInt(int c, dint64 i)
	{
		vType = INTEGER;
		content.i64 = i;
		vCode = c;
	}

	void addDouble(int c, double d)
	{
		vType = DOUBLE;
		content.d = d;
		vCode = c;
	}
	void addCoord(int c, DRW_Coord v)
	{
		vType = COORD;
		crd = v;
		content.pCrd = &crd;
		vCode = c;
	}
	void setCoordX(double d)
	{
		if (vType == COORD)
			crd.x = d;
	}
	void setCoordY(double d)
	{
		if (vType == COORD)
			crd.y = d;
	}
	void setCoordZ(double d)
	{
		if (vType == COORD)
			crd.z = d;
	}
	enum VariantType type()
	{
		return vType;
	}

	/*!< returns dxf code of this value*/
	int code()
	{
		return vCode;
	}

private:
	std::string str;
	DRW_Coord crd;

private:
	union DRW_VarContent
	{
		UTF8STRING* pStr;
		dint32 i;
		dint64 i64;
		double d;
		DRW_Coord* pCrd;

		DRW_VarContent(UTF8STRING* sd) : pStr(sd)
		{}
		DRW_VarContent(dint32 id) : i(id)
		{}
		DRW_VarContent(dint64 id) : i64(id)
		{}
		DRW_VarContent(double dd) : d(dd)
		{}
		DRW_VarContent(DRW_Coord* vd) : pCrd(vd)
		{}
	};

public:
	DRW_VarContent content;

private:
	enum VariantType vType;
	int vCode; /*!< dxf code of this value*/
};

//! Class to handle dwg handles
/*!
*  Class to handle dwg handles
*  @author Rallaz
*/
struct dwgHandle
{
public:
	dwgHandle() : code(0), size(0), ref(0)
	{}
	dwgHandle(const dwgHandle& other)
	{
		this->code = other.code;
		this->size = other.size;
		this->ref = other.ref;
	}
	~dwgHandle()
		= default;
	duint8 code;
	duint8 size;
	duint32 ref;
};

//! Class to convert between line width and integer
/*!
*  Class to convert between line width and integer
*  verifying valid values, if value is not valid
*  returns widthDefault.
*  @author Rallaz
*/
class DRW_LW_Conv
{
public:
	enum LineWidth
	{
		width00 = 0,       /*!< 0.00mm (dxf 0)*/
		width01 = 1,       /*!< 0.05mm (dxf 5)*/
		width02 = 2,       /*!< 0.09mm (dxf 9)*/
		width03 = 3,       /*!< 0.13mm (dxf 13)*/
		width04 = 4,       /*!< 0.15mm (dxf 15)*/
		width05 = 5,       /*!< 0.18mm (dxf 18)*/
		width06 = 6,       /*!< 0.20mm (dxf 20)*/
		width07 = 7,       /*!< 0.25mm (dxf 25)*/
		width08 = 8,       /*!< 0.30mm (dxf 30)*/
		width09 = 9,       /*!< 0.35mm (dxf 35)*/
		width10 = 10,      /*!< 0.40mm (dxf 40)*/
		width11 = 11,      /*!< 0.50mm (dxf 50)*/
		width12 = 12,      /*!< 0.53mm (dxf 53)*/
		width13 = 13,      /*!< 0.60mm (dxf 60)*/
		width14 = 14,      /*!< 0.70mm (dxf 70)*/
		width15 = 15,      /*!< 0.80mm (dxf 80)*/
		width16 = 16,      /*!< 0.90mm (dxf 90)*/
		width17 = 17,      /*!< 1.00mm (dxf 100)*/
		width18 = 18,      /*!< 1.06mm (dxf 106)*/
		width19 = 19,      /*!< 1.20mm (dxf 120)*/
		width20 = 20,      /*!< 1.40mm (dxf 140)*/
		width21 = 21,      /*!< 1.58mm (dxf 158)*/
		width22 = 22,      /*!< 2.00mm (dxf 200)*/
		width23 = 23,      /*!< 2.11mm (dxf 211)*/
		widthByLayer = 29, /*!< by layer (dxf -1) */
		widthByBlock = 30, /*!< by block (dxf -2) */
		widthDefault = 31  /*!< by default (dxf -3) */
	};

	static int lineWidth2dxfInt(enum LineWidth lw)
	{
		switch (lw)
		{
			case widthByLayer:
				return -1;
			case widthByBlock:
				return -2;
			case widthDefault:
				return -3;
			case width00:
				return 0;
			case width01:
				return 5;
			case width02:
				return 9;
			case width03:
				return 13;
			case width04:
				return 15;
			case width05:
				return 18;
			case width06:
				return 20;
			case width07:
				return 25;
			case width08:
				return 30;
			case width09:
				return 35;
			case width10:
				return 40;
			case width11:
				return 50;
			case width12:
				return 53;
			case width13:
				return 60;
			case width14:
				return 70;
			case width15:
				return 80;
			case width16:
				return 90;
			case width17:
				return 100;
			case width18:
				return 106;
			case width19:
				return 120;
			case width20:
				return 140;
			case width21:
				return 158;
			case width22:
				return 200;
			case width23:
				return 211;
			default:
				break;
		}
		return -3;
	}

	static int lineWidth2dwgInt(enum LineWidth lw)
	{
		return static_cast<int>(lw);
	}

	static enum LineWidth dxfInt2lineWidth(int i)
	{
		if (i < 0)
		{
			if (i == -1)
				return widthByLayer;
			else if (i == -2)
				return widthByBlock;
			else if (i == -3)
				return widthDefault;
		}
		else if (i < 3)
		{
			return width00;
		}
		else if (i < 7)
		{
			return width01;
		}
		else if (i < 11)
		{
			return width02;
		}
		else if (i < 14)
		{
			return width03;
		}
		else if (i < 16)
		{
			return width04;
		}
		else if (i < 19)
		{
			return width05;
		}
		else if (i < 22)
		{
			return width06;
		}
		else if (i < 27)
		{
			return width07;
		}
		else if (i < 32)
		{
			return width08;
		}
		else if (i < 37)
		{
			return width09;
		}
		else if (i < 45)
		{
			return width10;
		}
		else if (i < 52)
		{
			return width11;
		}
		else if (i < 57)
		{
			return width12;
		}
		else if (i < 65)
		{
			return width13;
		}
		else if (i < 75)
		{
			return width14;
		}
		else if (i < 85)
		{
			return width15;
		}
		else if (i < 95)
		{
			return width16;
		}
		else if (i < 103)
		{
			return width17;
		}
		else if (i < 112)
		{
			return width18;
		}
		else if (i < 130)
		{
			return width19;
		}
		else if (i < 149)
		{
			return width20;
		}
		else if (i < 180)
		{
			return width21;
		}
		else if (i < 205)
		{
			return width22;
		}
		else
		{
			return width23;
		}
		// default by default
		return widthDefault;
	}

	static enum LineWidth dwgInt2lineWidth(int i)
	{
		if ((i >= LineWidth::width00 && i <= LineWidth::width23) ||
			(i >= LineWidth::widthByBlock && i < LineWidth::widthDefault))
		{
			return static_cast<LineWidth>(i);
		}
		//default by default
		return widthDefault;
	}
};

#endif
