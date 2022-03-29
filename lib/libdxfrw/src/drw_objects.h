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

#ifndef DRW_OBJECTS_H
#define DRW_OBJECTS_H

#include <map>
#include <string>
#include <vector>
#include <map>
#include "drw_base.h"
#include "intern/drw_dbg.h"
#include "drw_color.h"

class dxfReader;
class dxfWriter;
class dwgBuffer;

#define SETOBJFRIENDS   \
    friend class dxfRW; \
    friend class DwgReader;



//! Base class for tables entries
/*!
*  Base class for tables entries
*  @author Rallaz
*/
class DRW_TableEntry
{
public:
	//initializes default values
	DRW_TableEntry()
	{
		dwgType = DRW::UNKNOWN_OBJ;
		flags = 0;
		numReactors = xDictMissingFlag = 0;
		parentHandle = 0;
		curr = nullptr;
		bAssociatedBianryData = false;
		handle = DRW::NoHandle;
		objBitSize = 0;
	}

	virtual ~DRW_TableEntry()
	{
		for (auto& it : extData)
			delete it;
		extData.clear();
	}

	DRW_TableEntry(const DRW_TableEntry& e)
	{
		dwgType = e.dwgType;
		handle = e.handle;
		parentHandle = e.parentHandle;
		entryName = e.entryName;
		flags = e.flags;
		numReactors = e.numReactors;
		xDictMissingFlag = e.xDictMissingFlag;
		curr = e.curr;

		for (auto it : e.extData)
		{
			extData.push_back(new DRW_Variant(*it));
		}
	}

protected:
	virtual void parseCode(int code, dxfReader* reader);
	virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) = 0;
	bool parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* strBuf, duint32 bs = 0);

	void reset()
	{
		flags = 0;
		for (auto& it : extData)
			delete it;
		extData.clear();
	}

public:
	enum DRW::DwgType dwgType;			/*!< enum: entity type, code 0 */
	duint32 handle;                     /*!< entity identifier, code 5 */
	int parentHandle;                   /*!< Soft-pointer ID/handle to owner object, code 330 */
	UTF8STRING entryName;               /*!< entry name, code 2 */
	int flags;                         //!B 70 The 64-bit of the 70 group.
	std::vector<DRW_Variant*> extData; /*!< FIFO list of extended data, codes 1000 to 1071*/

private:
	DRW_Variant* curr;

	/***** dwg parse ********/
protected:
	duint8 xDictMissingFlag;			// ! XDic Missing Flag
	dint32 numReactors;					// ! reactors num
	duint32 objBitSize;					// ! RL 32bits object data size in bits
	bool bAssociatedBianryData;			// indicate if this object has associated data in the data store 
};


//! Class to handle dwg obj control entries
/*!
*  Class to handle dwg obj control entries
*  @author Rallaz
*/
class DRW_ObjControl : public DRW_TableEntry
{
public:
	DRW_ObjControl()
	{
		reset();
	}

	void reset()
	{
		this->dwgType = DRW::UNKNOWN_OBJ;
	}
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;
	std::list<duint32> handleList; /* list of handles */
};


//! Class to handle dim style entries
/*!
*  Class to handle dim style symbol table entries
*  @author Rallaz
*/
class DRW_Dimstyle : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_Dimstyle()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::DIMSTYLE;
		dimasz = dimtxt = dimexe = 0.18;
		dimexo = 0.0625;
		dimgap = dimcen = 0.09;
		dimtxsty = "Standard";
		dimscale = dimlfac = dimtfac = dimfxl = 1.0;
		dimdli = 0.38;
		dimrnd = dimdle = dimtp = dimtm = dimtsz = dimtvp = 0.0;
		dimaltf = 25.4;
		dimtol = dimlim = dimse1 = dimse2 = dimtad = dimzin = 0;
		dimtoh = dimtolj = 1;
		dimalt = dimtofl = dimsah = dimtix = dimsoxd = dimfxlon = 0;
		dimaltd = dimunit = dimaltu = dimalttd = dimlunit = 2;
		dimclrd = dimclre = dimclrt = dimjust = dimupt = 0;
		dimazin = dimaltz = dimalttz = dimtzin = dimfrac = 0;
		dimtih = dimadec = dimaunit = dimsd1 = dimsd2 = dimtmove = 0;
		dimaltrnd = 0.0;
		dimdec = dimtdec = 4;
		dimfit = dimatfit = 3;
		dimdsep = '.';
		dimlwd = dimlwe = -2;
		DRW_TableEntry::reset();
	}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	//V12
	UTF8STRING dimpost;  /*!< code 3 */
	UTF8STRING dimapost; /*!< code 4 */
						 /* handle are code 105 */
	UTF8STRING dimblk;   /*!< code 5, code 342 V2000+ */
	UTF8STRING dimblk1;  /*!< code 6, code 343 V2000+ */
	UTF8STRING dimblk2;  /*!< code 7, code 344 V2000+ */
	double dimscale;     /*!< code 40 */
	double dimasz;       /*!< code 41 */
	double dimexo;       /*!< code 42 */
	double dimdli;       /*!< code 43 */
	double dimexe;       /*!< code 44 */
	double dimrnd;       /*!< code 45 */
	double dimdle;       /*!< code 46 */
	double dimtp;        /*!< code 47 */
	double dimtm;        /*!< code 48 */
	double dimfxl;       /*!< code 49 V2007+ */
	double dimjogang; /*!< code 50 */
	double dimaltmzf; /*!< code ? */
	double dimmzf;    /*!< code ? */
	double dimtxt;    /*!< code 140 */
	double dimcen;    /*!< code 141 */
	double dimtsz;    /*!< code 142 */
	double dimaltf;   /*!< code 143 */
	double dimlfac;   /*!< code 144 */
	double dimtvp;    /*!< code 145 */
	double dimtfac;   /*!< code 146 */
	double dimgap;    /*!< code 147 */
	double dimaltrnd; /*!< code 148 V2000+ */
	int dimtfill;         /*!< code 69 */
	int dimtfillclr;      /*!< code 70 */
	int dimarcsym;        /*!< code 90 */
	int dimtxtdirection;  /*!< code 295 */
	UTF8STRING dimaltmzs; /*!< code ? */
	UTF8STRING dimmzs;    /*!< code ? */
	int dimtol;           /*!< code 71 */
	int dimlim;           /*!< code 72 */
	int dimtih;           /*!< code 73 */
	int dimtoh;           /*!< code 74 */
	int dimse1;           /*!< code 75 */
	int dimse2;           /*!< code 76 */
	int dimtad;           /*!< code 77 */
	int dimzin;           /*!< code 78 */
	int dimazin;          /*!< code 79 V2000+ */
	int dimalt;           /*!< code 170 */
	int dimaltd;          /*!< code 171 */
	int dimtofl;          /*!< code 172 */
	int dimsah;           /*!< code 173 */
	int dimtix;           /*!< code 174 */
	int dimsoxd;          /*!< code 175 */
	int dimclrd;          /*!< code 176 */
	int dimclre;          /*!< code 177 */
	int dimclrt;          /*!< code 178 */
	int dimadec;          /*!< code 179 V2000+ */
	int dimunit;          /*!< code 270 R13+ (obsolete 2000+, use dimlunit & dimfrac) */
	int dimdec;           /*!< code 271 R13+ */
	int dimtdec;          /*!< code 272 R13+ */
	int dimaltu;          /*!< code 273 R13+ */
	int dimalttd;         /*!< code 274 R13+ */
	int dimaunit;         /*!< code 275 R13+ */
	int dimfrac;          /*!< code 276 V2000+ */
	int dimlunit;         /*!< code 277 V2000+ */
	int dimdsep;          /*!< code 278 V2000+ */
	int dimtmove;         /*!< code 279 V2000+ */
	int dimjust;          /*!< code 280 R13+ */
	int dimsd1;           /*!< code 281 R13+ */
	int dimsd2;           /*!< code 282 R13+ */
	int dimtolj;          /*!< code 283 R13+ */
	int dimtzin;          /*!< code 284 R13+ */
	int dimaltz;          /*!< code 285 R13+ */
	int dimalttz;         /*!< code 286 R13+ */
	int dimfit;           /*!< code 287 R13+  (obsolete 2000+, use dimatfit & dimtmove)*/
	int dimupt;           /*!< code 288 R13+ */
	int dimatfit;         /*!< code 289 V2000+ */
	int dimfxlon;         /*!< code 290 V2007+ */
	UTF8STRING dimtxsty;  /*!< code 340 R13+ */
	UTF8STRING dimldrblk; /*!< code 341 V2000+ */
	int dimlwd;           /*!< code 371 V2000+ */
	int dimlwe;           /*!< code 372 V2000+ */
};

//! Class to handle line type entries
/*!
*  Class to handle line type symbol table entries
*  @author Rallaz
*/
class DRW_LType : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_LType()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::LTYPE;
		description = "";
		dashesNum = 0;
		patternLen = 0.0;
		pathIdx = 0;
		DRW_TableEntry::reset();
		dashes.clear();
	}

	struct DashShape
	{
		double dashLength;//!Dash or dot specifier.
		duint16 complexShapecode; //!Shape number if shapeflag is 2, or index into the string area if shapeflag is 4.
		double xOffset;//!(0.0 for a simple dash.)
		double yOffset;//!(0.0 for a simple dash.)
		double scale;//!(1.0 for a simple dash.)
		double rotation;//!(0.0 for a simple dash.)
		/*!bit coded :
		if (shapeflag & 1), text is rotated 0 degrees,otherwise it follows the segment
		if (shapeflag & 2), complex shape-code holds the index of the shape to be drawn
		if (shapeflag & 4), complex shape-code holds the index into the text area of the string to be drawn.
		*/
		duint16 shapeflag;
	};

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

	//! Update line type
	void updateTotalLengthOfPattern()
	{
		dashesNum = static_cast<duint32>(dashes.size());
		for (duint32 i = 0; i < dashesNum; i++)
			patternLen += fabs(dashes.at(i).dashLength);
	}
public:
	UTF8STRING description;			/*!< descriptive string, code 3 */
	duint32 dashesNum;              /*!< dashes number, code 73 */
	double patternLen;				/*!< total length of pattern, code 40 */
	duint8 alignment;				//! always 'A' code 72
	std::vector<DashShape> dashes;	//! dash-shape 
private:
	int pathIdx;					// aka complex shape-code
};

//! Class to handle layer entries
/*!
*  Class to handle layer symbol table entries
*  @author Rallaz
*/
class DRW_Layer : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_Layer()
	{
		reset();
	}
	void reset()
	{
		dwgType = DRW::LAYER;
		lineType = "CONTINUOUS";
		color = DRW::ColorByLayer;           // default BYLAYER (256)
		plotF = true;                        // default TRUE (plot yes)
		lWeight = DRW_LW_Conv::widthDefault; // default BYDEFAULT (dxf -3, dwg 31)
		color24 = -1;                        // default -1 not set
		DRW_TableEntry::reset();
	}

	DRW_Layer(const DRW_Layer& l) = default;

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	UTF8STRING lineType;                 /*!< line type, code 6 */
	std::shared_ptr<DRW_Color> dwgColor;
	int color;                           /*!< layer color, code 62 */
	int color24;                         /*!< 24-bit color, code 420 */
	bool plotF;                          /*!< Plot flag, code 290 */
	enum DRW_LW_Conv::LineWidth lWeight; /*!< layer line weight(width), code 370 */
	std::string handlePlotS;             /*!< Hard-pointer ID/handle of plotstyle, code 390 */
	std::string handleMaterialS;         /*!< Hard-pointer ID/handle of materialstyle, code 347 */
										 /*only used for read dwg*/
	//dwgHandle lTypeH;                    /*!< linetype handle */
	duint32 lTypeHandleRef;
};

//! Class to handle block record entries
/*!
*  Class to handle block record table entries
*  @author Rallaz
*/
class DRW_Block_Header : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_Block_Header()
	{
		reset();
	}
	void reset()
	{
		dwgType = DRW::BLOCK_HEADER;
		flags = 0;
		firstEH = lastEH = DRW::NoHandle;
		DRW_TableEntry::reset();
	}

protected:
	//    void parseCode(int code, dxfReader *reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	//Note:    int DRW_TableEntry::flags; contains code 70 of block
	int insUnits;        /*!< block insertion units, code 70 of block_record*/
	bool explodable;	//if block can be exploded,280
	dint8 blkScaling;//block scaling ,281
	DRW_Coord basePoint; /*!<  block insertion base point dwg only */
protected:
	//dwg parser
private:
	duint32 block;    //handle for block entity
	duint32 endBlock; //handle for end block entity

	duint32 firstEH;                        // first entity handle, only in pre-2004
	duint32 lastEH;                         // last entity handle, only in pre-2004
	std::vector<duint32> ownedObjHandles;   // owned obj handles vec
	std::vector<duint32> insertEnHandles;   //insert entity handles vec
};

//! Class to handle text style entries
/*!
*  Class to handle text style symbol table entries
*  @author Rallaz
*/
class DRW_Textstyle : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_Textstyle()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::STYLE;
		height = oblique = 0.0;
		width = lastHeight = 1.0;
		font = "txt";
		genFlag = 0; //2= X mirror, 4= Y mirror
		fontFamily = 0;
		DRW_TableEntry::reset();
	}

protected:
	void parseCode(int code, dxfReader* reader);
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0);

public:
	// !msclock:add two members below:
	duint8 verticalFlag; /* if vertical */
	duint8 shapfileFLag; /* if a shape file rather than a font */

	double height;      /*!< Fixed text height (0 not set), code 40 */
	double width;       /*!< Width factor, code 41 */
	double oblique;     /*!< Oblique angle, code 50 */
	int genFlag;        /*!< Text generation flags, code 71 */
	double lastHeight;  /*!< Last height used, code 42 */
	UTF8STRING font;    /*!< primary font file name, code 3 */
	UTF8STRING bigFont; /*!< bigfont file name or blank if none, code 4 */
	int fontFamily;     /*!< ttf font family, italic and bold flags, code 1071 */
};

//! Class to handle vport entries
/*!
*  Class to handle vport symbol table entries
*  @author Rallaz
*/
class DRW_Vport : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_Vport()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::VPORT;
		upperRight.x = upperRight.y = 1.0;
		snapSpacing.x = snapSpacing.y = 10.0;
		gridSpacing = snapSpacing;
		viewCenter.x = 0.651828;
		viewCenter.y = -0.16;
		viewDir.z = 1;
		viewHeight = 5.13732;
		aspectRatio = 2.4426877;
		lensLen = 50;
		frontClip = backClip = snapAngle = viewTwistAngle = 0.0;
		viewMode = snap = grid = snapStyle = snapIsopair = 0;
		fastZoom = 1;
		circleZoom = 100;
		ucsIcon = 3;
		gridBehavior = 7;
		DRW_TableEntry::reset();
	}

protected:
	void parseCode(int code, dxfReader* reader) override;

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	double viewHeight;         /*!< BD 40 view height, code 40 */
	double aspectRatio;          /*!< viewport aspect ratio, code 41 */
	DRW_Coord viewCenter;      /*!< center point in WCS, code 12 & 22 */
	DRW_Coord viewTarget;  /*!< view target point, code 17, 27 & 37 */
	DRW_Coord viewDir;     /*!< view direction from target point, code 16, 26 & 36 */
	double viewTwistAngle;     /*!< view twist angle, code 51 */
	double lensLen;     /*!< lens height, code 42 */
	double frontClip;      /*!< front clipping plane, code 43 */
	double backClip;       /*!< back clipping plane, code 44 */

	int viewMode;          /*!< view mode, code 71 */


	DRW_Coord lowerLeft;   /*!< Lower left corner, code 10 & 20 */
	DRW_Coord upperRight;  /*!< Upper right corner, code 11 & 21 */
	DRW_Coord snapBase;    /*!< snap base point in DCS, code 13 & 23 */
	DRW_Coord snapSpacing; /*!< snap Spacing, code 14 & 24 */
	DRW_Coord gridSpacing; /*!< grid Spacing, code 15 & 25 */
	double snapAngle;      /*!< snap rotation angle, code 50 */
	int circleZoom;        /*!< circle zoom percent, code 72 */
	int fastZoom;          /*!< fast zoom setting, code 73 */
	int ucsIcon;           /*!< UCSICON setting, code 74 */
	int snap;              /*!< snap on/off, code 75 */
	int grid;              /*!< grid on/off, code 76 */
	int snapStyle;         /*!< snap style, code 77 */
	int snapIsopair;       /*!< snap isopair, code 78 */
	int gridBehavior;      /*!< grid behavior, code 60, undocummented */
	/** code 60, bit coded possible value are
	* bit 1 (1) show out of limits
	* bit 2 (2) adaptive grid
	* bit 3 (4) allow subdivision
	* bit 4 (8) follow dinamic SCP
	**/
};

//! Class to handle imagedef entries
/*!
*  Class to handle image definitions object entries
*  @author Rallaz
*/
class DRW_ImageDef : public DRW_TableEntry
{ //
	SETOBJFRIENDS
public:
	DRW_ImageDef()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::IMAGEDEF;
		imgVersion = 0;
		DRW_TableEntry::reset();
	}

protected:
	void parseCode(int code, dxfReader* reader) override;
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

public:
	//    std::string handle;       /*!< entity identifier, code 5 */
	UTF8STRING entryName; /*!< File name of image, code 1 */
	int imgVersion;  /*!< class version, code 90, 0=R14 version */
	double u;        /*!< image size in pixels U value, code 10 */
	double v;        /*!< image size in pixels V value, code 20 */
	double up;       /*!< default size of one pixel U value, code 11 */
	double vp;       /*!< default size of one pixel V value, code 12 really is 21*/
	int loaded;      /*!< image is loaded flag, code 280, 0=unloaded, 1=loaded */
	int resolution;  /*!< resolution units, code 281, 0=no, 2=centimeters, 5=inch */

	std::map<std::string, std::string> reactors;
};

//! Class to handle AppId entries
/*!
*  Class to handle AppId symbol table entries
*  @author Rallaz
*/
class DRW_AppId : public DRW_TableEntry
{
	SETOBJFRIENDS
public:
	DRW_AppId()
	{
		reset();
	}

	void reset()
	{
		dwgType = DRW::APPID;
		flags = 0;
		entryName = "";
	}

protected:
	void parseCode(int code, dxfReader* reader) override
	{
		DRW_TableEntry::parseCode(code, reader);
	}
	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;
};

//! class to handle layout entries
/*!
*  Class to handle layout table entries
*  @author msclock
*/
class DRW_Layout :public DRW_TableEntry
{
	SETOBJFRIENDS;
public:
	DRW_Layout() = default;
	~DRW_Layout() = default;

protected:
	void parseCode(int code, dxfReader* reader) override
	{
		DRW_TableEntry::parseCode(code, reader);
	}

	bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs = 0) override;

	UTF8STRING pageSetupName;			//!TV 1 plot-settings page setup name
	UTF8STRING printerOrConfig;			//!TV 2 plot-settings printer or configuration file
	duint16 plotLayoutFlags;			//!plot-settings plot layout flag
	//!plot-settings left/bottom/right/top margin in millimeters
	double leftMargin;					//!BD 40 
	double bottomMargin;				//!BD 41
	double rightMargin;					//!BD 42
	double topMargin;					//!BD 43
	double paperWith;					//!BD 44 plot-settings paper width in millimeters
	double paperHeight;					//!BD 45 plot-settings paper height in millimeters
	UTF8STRING paperSize;				//!TV 4 plot-settings paper size
	DRW_Coord plotOrigin;				//!2BD 46,47 plot-settings origin offset in millimeters
	duint16 paperUnits;					//!BS 72 plot-settings plot paper units
	duint16 plotRotation;				//!BS 73 plot-settings plot rotation
	duint16 plotType;					//!BS 74 plot-settings plot type
	DRW_Coord windowMin;				//!2BD 48,49 plot-settings plot window area lower left
	DRW_Coord windowMax;				//!2BD 140,141 plot-settings plot window area upper right
	UTF8STRING plotViewName;			//!T 6 plot-settings plot view name
	double realWorldUnits;				//!plot-settings numerator of custom print scale
	double drawingUnits;				//!BD 143 plot-settings denominator of custom print scale
	UTF8STRING currentStyleSheet;		//!TV 7 plot-settings current style sheet
	duint16 scaleType;					//!BS 75 plot-settings standard scale type
	double scaleFactor;					//!BD 147 plot-settings scale factor
	DRW_Coord paperImageOrigin;			//!2BD 148,149 plot-settings paper image origin
	duint16 shadePlotMode;				//!BS 76
	duint16 shadePlotResLevel;			//!BS 77
	duint16 shadePlotCustomDPI;			//!BS 78
	UTF8STRING layoutName;				//!TV 1 layout name
	dint32 tabOrder;					//!BL 71 layout tab order
	duint16 flag;						//!BS 70 layout flags
	DRW_Coord ucsOrigin;				//!3BD 13 layout ucs origin
	DRW_Coord limmin;					//!2RD 10 layout minimum limits
	DRW_Coord limmax;					//!2RD 11 layout maximum limits
	DRW_Coord inspoint;					//!3BD 12 layout insertion base point
	DRW_Coord ucsXAxis;					//!3BD 16 layout ucs x axis direction
	DRW_Coord ucsYAxis;					//!3BD 17 layout ucs y axis direction
	double elevation;					//!BD 147layout elevation
	duint16 orthoviewType;				//!BS 76 layout orthographic view type of UCS
	DRW_Coord extmin;					//!3BD 14 layout extent min
	DRW_Coord extmax;					//!3BD 15 layout extent max
	duint32 viewportCount;				//!RL # of viewports in this layout
};

namespace DRW
{

	// Extended color palette:
	// The first entry is only for direct indexing starting with [1]
	// Color 1 is red (1,0,0)
	const unsigned char dxfColors[][3] = {
		{0, 0, 0},       // unused
		{255, 0, 0},     // 1 red
		{255, 255, 0},   // 2 yellow
		{0, 255, 0},     // 3 green
		{0, 255, 255},   // 4 cyan
		{0, 0, 255},     // 5 blue
		{255, 0, 255},   // 6 magenta
		{0, 0, 0},       // 7 black or white
		{128, 128, 128}, // 8 50% gray
		{192, 192, 192}, // 9 75% gray
		{255, 0, 0},     // 10
		{255, 127, 127},
		{204, 0, 0},
		{204, 102, 102},
		{153, 0, 0},
		{153, 76, 76}, // 15
		{127, 0, 0},
		{127, 63, 63},
		{76, 0, 0},
		{76, 38, 38},
		{255, 63, 0}, // 20
		{255, 159, 127},
		{204, 51, 0},
		{204, 127, 102},
		{153, 38, 0},
		{153, 95, 76}, // 25
		{127, 31, 0},
		{127, 79, 63},
		{76, 19, 0},
		{76, 47, 38},
		{255, 127, 0}, // 30
		{255, 191, 127},
		{204, 102, 0},
		{204, 153, 102},
		{153, 76, 0},
		{153, 114, 76}, // 35
		{127, 63, 0},
		{127, 95, 63},
		{76, 38, 0},
		{76, 57, 38},
		{255, 191, 0}, // 40
		{255, 223, 127},
		{204, 153, 0},
		{204, 178, 102},
		{153, 114, 0},
		{153, 133, 76}, // 45
		{127, 95, 0},
		{127, 111, 63},
		{76, 57, 0},
		{76, 66, 38},
		{255, 255, 0}, // 50
		{255, 255, 127},
		{204, 204, 0},
		{204, 204, 102},
		{153, 153, 0},
		{153, 153, 76}, // 55
		{127, 127, 0},
		{127, 127, 63},
		{76, 76, 0},
		{76, 76, 38},
		{191, 255, 0}, // 60
		{223, 255, 127},
		{153, 204, 0},
		{178, 204, 102},
		{114, 153, 0},
		{133, 153, 76}, // 65
		{95, 127, 0},
		{111, 127, 63},
		{57, 76, 0},
		{66, 76, 38},
		{127, 255, 0}, // 70
		{191, 255, 127},
		{102, 204, 0},
		{153, 204, 102},
		{76, 153, 0},
		{114, 153, 76}, // 75
		{63, 127, 0},
		{95, 127, 63},
		{38, 76, 0},
		{57, 76, 38},
		{63, 255, 0}, // 80
		{159, 255, 127},
		{51, 204, 0},
		{127, 204, 102},
		{38, 153, 0},
		{95, 153, 76}, // 85
		{31, 127, 0},
		{79, 127, 63},
		{19, 76, 0},
		{47, 76, 38},
		{0, 255, 0}, // 90
		{127, 255, 127},
		{0, 204, 0},
		{102, 204, 102},
		{0, 153, 0},
		{76, 153, 76}, // 95
		{0, 127, 0},
		{63, 127, 63},
		{0, 76, 0},
		{38, 76, 38},
		{0, 255, 63}, // 100
		{127, 255, 159},
		{0, 204, 51},
		{102, 204, 127},
		{0, 153, 38},
		{76, 153, 95}, // 105
		{0, 127, 31},
		{63, 127, 79},
		{0, 76, 19},
		{38, 76, 47},
		{0, 255, 127}, // 110
		{127, 255, 191},
		{0, 204, 102},
		{102, 204, 153},
		{0, 153, 76},
		{76, 153, 114}, // 115
		{0, 127, 63},
		{63, 127, 95},
		{0, 76, 38},
		{38, 76, 57},
		{0, 255, 191}, // 120
		{127, 255, 223},
		{0, 204, 153},
		{102, 204, 178},
		{0, 153, 114},
		{76, 153, 133}, // 125
		{0, 127, 95},
		{63, 127, 111},
		{0, 76, 57},
		{38, 76, 66},
		{0, 255, 255}, // 130
		{127, 255, 255},
		{0, 204, 204},
		{102, 204, 204},
		{0, 153, 153},
		{76, 153, 153}, // 135
		{0, 127, 127},
		{63, 127, 127},
		{0, 76, 76},
		{38, 76, 76},
		{0, 191, 255}, // 140
		{127, 223, 255},
		{0, 153, 204},
		{102, 178, 204},
		{0, 114, 153},
		{76, 133, 153}, // 145
		{0, 95, 127},
		{63, 111, 127},
		{0, 57, 76},
		{38, 66, 76},
		{0, 127, 255}, // 150
		{127, 191, 255},
		{0, 102, 204},
		{102, 153, 204},
		{0, 76, 153},
		{76, 114, 153}, // 155
		{0, 63, 127},
		{63, 95, 127},
		{0, 38, 76},
		{38, 57, 76},
		{0, 66, 255}, // 160
		{127, 159, 255},
		{0, 51, 204},
		{102, 127, 204},
		{0, 38, 153},
		{76, 95, 153}, // 165
		{0, 31, 127},
		{63, 79, 127},
		{0, 19, 76},
		{38, 47, 76},
		{0, 0, 255}, // 170
		{127, 127, 255},
		{0, 0, 204},
		{102, 102, 204},
		{0, 0, 153},
		{76, 76, 153}, // 175
		{0, 0, 127},
		{63, 63, 127},
		{0, 0, 76},
		{38, 38, 76},
		{63, 0, 255}, // 180
		{159, 127, 255},
		{50, 0, 204},
		{127, 102, 204},
		{38, 0, 153},
		{95, 76, 153}, // 185
		{31, 0, 127},
		{79, 63, 127},
		{19, 0, 76},
		{47, 38, 76},
		{127, 0, 255}, // 190
		{191, 127, 255},
		{102, 0, 204},
		{153, 102, 204},
		{76, 0, 153},
		{114, 76, 153}, // 195
		{63, 0, 127},
		{95, 63, 127},
		{38, 0, 76},
		{57, 38, 76},
		{191, 0, 255}, // 200
		{223, 127, 255},
		{153, 0, 204},
		{178, 102, 204},
		{114, 0, 153},
		{133, 76, 153}, // 205
		{95, 0, 127},
		{111, 63, 127},
		{57, 0, 76},
		{66, 38, 76},
		{255, 0, 255}, // 210
		{255, 127, 255},
		{204, 0, 204},
		{204, 102, 204},
		{153, 0, 153},
		{153, 76, 153}, // 215
		{127, 0, 127},
		{127, 63, 127},
		{76, 0, 76},
		{76, 38, 76},
		{255, 0, 191}, // 220
		{255, 127, 223},
		{204, 0, 153},
		{204, 102, 178},
		{153, 0, 114},
		{153, 76, 133}, // 225
		{127, 0, 95},
		{127, 63, 11},
		{76, 0, 57},
		{76, 38, 66},
		{255, 0, 127}, // 230
		{255, 127, 191},
		{204, 0, 102},
		{204, 102, 153},
		{153, 0, 76},
		{153, 76, 114}, // 235
		{127, 0, 63},
		{127, 63, 95},
		{76, 0, 38},
		{76, 38, 57},
		{255, 0, 63}, // 240
		{255, 127, 159},
		{204, 0, 51},
		{204, 102, 127},
		{153, 0, 38},
		{153, 76, 95}, // 245
		{127, 0, 31},
		{127, 63, 79},
		{76, 0, 19},
		{76, 38, 47},
		{51, 51, 51}, // 250
		{91, 91, 91},
		{132, 132, 132},
		{173, 173, 173},
		{214, 214, 214},
		{255, 255, 255} // 255
	};

} // namespace DRW
#endif
