#pragma once

#include "drw_base.h"
#include "intern/dwgbuffer.h"

#ifndef RGB
#define RGB(r,g,b) ((duint32)(((duint8)(r)|((duint16)((duint8)(g))<<8))|(((duint16)(duint8)(b))<<16)))
#endif
#ifndef PALETTERGB
#define PALETTERGB(r,g,b)   (0x02000000 | RGB(r,g,b))
#endif
#ifndef PALETTEINDEX
#define PALETTEINDEX(i)     ((duint32)(0x01000000 | (duint32)(duint16)(i)))
#endif




#define getRed(rgb)	((rgb)&0xff)
#define getGreen(rgb) (((rgb)>>8)&0xff)
#define getBlue(rgb) (((rgb)>>16)&0xff)


namespace DRW
{
	//! CAD color index mapped to rgb value
	static duint32 CadColor[256] =
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
};

//! class to handle Color 
/*
 *class to handle color
 *@author msclock
 **/
class DRW_Color
{
public:
	enum class ColorType
	{
		UNKNOWN,
		// todo extract this for bylayer or byblock
		BYLAYER,
		BYBLOCK,
		// todo end extract this for bylayer or byblock
		INDEX,			// index
		//RGB24,			// 24-color rgb
		ACDBCLR,			// acdb color
		//ACIS,
		Cmc,
		Enc
	};

	DRW_Color()
		:colorIndex(),
		rgb(),
		colorType(ColorType::UNKNOWN)
	{}


	virtual ~DRW_Color() = default;
	virtual duint8 getColorIndex();
	virtual duint32 getRGB();
	virtual bool bAcDBColorPresent();
	virtual ColorType getColorType();
	virtual void parseDwgColor(DRW::Version v, dwgBuffer* buf) = 0;
protected:
	duint8 colorIndex;		//! color index from 0 to 255
	duint32 rgb;			//! rgb
	ColorType colorType;	//! color type
};

class DRW_CmcColor :public DRW_Color
{
public:
	DRW_CmcColor() = default;
	virtual ~DRW_CmcColor() = default;
	duint8 getColorIndex() override;
	void parseDwgColor(DRW::Version v, dwgBuffer* buf) override;
protected:
	dint8 colorByte;		//! color indicator
public:
	UTF8STRING colorName;	//! color name
	UTF8STRING bookName;	//! book name
};

class DRW_EncColor :public DRW_Color
{
public:
	DRW_EncColor() = default;
	virtual ~DRW_EncColor() = default;
	duint8 getColorIndex() override;
	duint32 getRGB() override;
	bool bAcDBColorPresent() override;
	void parseDwgColor(DRW::Version v, dwgBuffer* buf) override;

public:
	bool bAcDbColorPresent;	//! true if acdbColor COLOR are present

private:
	duint16 colorNumber;		//! color number of entity
	duint16 colorFlags;			//! color flags
	dint32 transparencyCodes;	//! transparency codes
};