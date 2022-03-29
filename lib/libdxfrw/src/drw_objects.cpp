/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "drw_objects.h"
#include <cmath>
#include <iostream>
#include "intern/drw_dbg.h"
#include "intern/dwgbuffer.h"
#include "intern/dwgutil.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"

//! Base class for tables entries
/*!
*  Base class for tables entries
*  @author Rallaz
*/
void DRW_TableEntry::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 5:
			handle = reader->getHandleString();
			break;
		case 330:
			parentHandle = reader->getHandleString();
			break;
		case 2:
			entryName = reader->getUtf8String();
			break;
		case 70:
			flags = reader->getInt32();
			break;
		case 1000:
		case 1001:
		case 1002:
		case 1003:
		case 1004:
		case 1005:
			extData.push_back(new DRW_Variant(code, reader->getString()));
			break;
		case 1010:
		case 1011:
		case 1012:
		case 1013:
			curr = new DRW_Variant(code, DRW_Coord(reader->getDouble(), 0.0, 0.0));
			extData.push_back(curr);
			break;
		case 1020:
		case 1021:
		case 1022:
		case 1023:
			if (curr)
				curr->setCoordY(reader->getDouble());
			break;
		case 1030:
		case 1031:
		case 1032:
		case 1033:
			if (curr)
				curr->setCoordZ(reader->getDouble());
			curr = NULL;
			break;
		case 1040:
		case 1041:
		case 1042:
			extData.push_back(new DRW_Variant(code, reader->getDouble()));
			break;
		case 1070:
		case 1071:
			extData.push_back(new DRW_Variant(code, reader->getInt32()));
			break;
		default:
			break;
	}
}

bool DRW_TableEntry::parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* strBuf, duint32 bs)
{

	DRW_DBG("\n***************************** parsing table entry *********************************************\n");
	objBitSize = 0;
	dwgType = static_cast<DRW::DwgType>(buf->getObjType(version));
	DRW_DBG("Object type: ");
	DRW_DBG(dwgType);

	if (version > DRW::R14 && version < DRW::R2010)
	{                                     //2000 to 2007
		objBitSize = buf->getRawLong32(); //RL 32bits object size in bits
		DRW_DBG(" Object size: ");
		DRW_DBG(objBitSize);
		DRW_DBG("\n");
	}

	if (version > DRW::R2007)
	{
		//2010+
		duint32 ms = static_cast<duint32>(buf->size());
		objBitSize = ms * 8 - bs;
		DRW_DBG(" Object size: ");
		DRW_DBG(objBitSize);
		DRW_DBG("\n");
	}

	if (strBuf != nullptr && version > DRW::R2004)
	{
		//2007+
		strBuf->moveBitPos(objBitSize - 1);
		DRW_DBG(" strBuf str bit pos 2007: ");
		DRW_DBG(strBuf->getPosition());
		DRW_DBG(" strBuf b pos 2007: ");
		DRW_DBG(strBuf->getBitPos());
		DRW_DBG("\n");
		if (strBuf->getBit() == 1)
		{
			DRW_DBG("DRW_TableEntry::parseDwg string bit is 1\n");
			strBuf->moveBitPos(-17);
			duint16 strDataSize = strBuf->getRawShort16();
			DRW_DBG("\nDRW_TableEntry::parseDwg string strDataSize: ");
			DRW_DBGH(strDataSize);
			DRW_DBG("\n");
			if ((strDataSize & 0x8000) == 0x8000)
			{
				DRW_DBG("\nDRW_TableEntry::parseDwg string 0x8000 bit is set");
				strBuf->moveBitPos(-33); //RLZ pending to verify
				duint16 hiSize = strBuf->getRawShort16();
				strDataSize = ((strDataSize & 0x7fff) | (hiSize << 15));
			}
			strBuf->moveBitPos(-strDataSize - 16); //-14
			DRW_DBG("strBuf start strDataSize pos 2007: ");
			DRW_DBG(strBuf->getPosition());
			DRW_DBG(" strBuf b pos 2007: ");
			DRW_DBG(strBuf->getBitPos());
			DRW_DBG("\n");
		}
		else
			DRW_DBG("\nDRW_TableEntry::parseDwg string bit is 0");
		DRW_DBG("strBuf start pos 2007: ");
		DRW_DBG(strBuf->getPosition());
		DRW_DBG(" strBuf bpos 2007: ");
		DRW_DBG(strBuf->getBitPos());
		DRW_DBG("\n");
	}

	const dwgHandle entityHandle = buf->getHandle();
	handle = entityHandle.ref; //* current object or entity handle
	DRW_DBG("TableEntry Handle: ");
	DRW_DBGHL(entityHandle.code, entityHandle.size, entityHandle.ref);

	//EED
	dint16 eedSize = buf->getBitShort();//BS
	while (eedSize > 0 && buf->isGood())
	{
		dwgHandle h = buf->getHandle();
		std::vector<duint8> eedData(eedSize);
		buf->getBytes(eedData.data(), eedSize);
		dwgBuffer eedBuf(eedData.data(), eedSize, buf->decoder);
		duint8 dxfCode;
		while ((eedBuf.size() != eedBuf.getPosition()))
		{
			dxfCode = eedBuf.getRawChar8();
			switch (dxfCode)
			{
				// string
				case 0:
				{
					if (version < DRW::R2007)
					{
						duint8 len = eedBuf.getRawChar8();
						std::string s(len + 1, 0);
						duint16 cp = eedBuf.getBERawShort16();
						for (int i = 0; i < len; i++)
						{
							//string length + null terminating char
							duint8 dxfChar = eedBuf.getRawChar8();
							s[i] = dxfChar;
						}
						extData.push_back(new DRW_Variant(1000, s));
					}
					else//R2007
					{
						duint16 len = eedBuf.getRawShort16();
						std::string s(len + 1, 0);
						for (auto i = 0; i < len; i++)
						{
							s[i] = (eedBuf.getRawShort16());
						}
						extData.push_back(new DRW_Variant(1000, s));
					}
					break;
				}
				// invalid code that seems to be redundant or irrelevant here.
				case 1:
				{
					DRW_DBG("\ninvalid code");
					break;
				}
				// a '{':0 , or '}':1
				case 2:
				{
					//1002 0:{; 1:}
					dint8 i8 = eedBuf.getRawChar8();
					extData.push_back(new DRW_Variant(1002, i8));
					break;
				}
				// a layer table reference
				case  3:
				{
					dint64 i64 = eedBuf.getRawLong64();//64?32
					extData.push_back(new DRW_Variant(1003, i64));
					break;
				}
				//binary chunk
				case 4:
				{
					std::string s;
					duint8 i8 = eedBuf.getRawChar8();
					for (auto i = 0; i < i8; i++)
					{
						s.append(DRW::toHexStr(eedBuf.getRawChar8()));
					}
					extData.push_back(new DRW_Variant(dxfCode + 4, s));
					break;
				}
				// an entity handle reference
				case 5:
				{
					dint64 i64 = eedBuf.getRawLong64();//64?32
					extData.push_back(new DRW_Variant(dxfCode + 1000, i64));
					break;
				}
				// pt 
				case 10:
				case 11:
				case 12:
				case 13:
				{
					DRW_Coord crd;
					crd.x = eedBuf.getRawDouble();
					crd.y = eedBuf.getRawDouble();
					crd.z = eedBuf.getRawDouble();
					extData.push_back(new DRW_Variant(dxfCode + 1000, crd));
					break;
				}
				// reals
				case 40:
				case 41:
				case 42:
				{
					double reals = eedBuf.getRawDouble();
					extData.push_back(new DRW_Variant(dxfCode + 1000, reals));
					break;
				}
				// a short int
				case 70:
				{
					dint16 sht = eedBuf.getRawShort16();
					extData.push_back(new DRW_Variant(1070, sht));
					break;
				}
				// a long int
				case 71:
				{
					dint32 lg = eedBuf.getRawLong32();
					extData.push_back(new DRW_Variant(1071, lg));
					break;
				}
				default:
					break;
			}
		}
		eedSize = buf->getBitShort();
	}

	if (version < DRW::R2000)
	{
		//14-
		objBitSize = buf->getRawLong32(); //RL 32bits size in bits
	}
	DRW_DBG(" objBitSize in bits: ");
	DRW_DBG(objBitSize);

	numReactors = buf->getBitLong(); //BL
	DRW_DBG(", numReactors: ");
	DRW_DBG(numReactors);
	DRW_DBG("\n");

	if (version >= DRW::R2004)
	{
		//2004+
		xDictMissingFlag = buf->getBit();
		DRW_DBG("xDictMissingFlag: ");
		DRW_DBG(xDictMissingFlag);
	}

	if (version >= DRW::R2013)
	{
		//2013+
		bAssociatedBianryData = buf->getBit();
		DRW_DBG(" Whether has binary data associated: ");
		DRW_DBG(bAssociatedBianryData);
		DRW_DBG("\n");
	}
	return buf->isGood();
}

//! Class to handle dimstyle entries
/*!
*  Class to handle ldim style symbol table entries
*  @author Rallaz
*/
void DRW_Dimstyle::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 105:
			handle = reader->getHandleString();
			break;
		case 3:
			dimpost = reader->getUtf8String();
			break;
		case 4:
			dimapost = reader->getUtf8String();
			break;
		case 5:
			dimblk = reader->getUtf8String();
			break;
		case 6:
			dimblk1 = reader->getUtf8String();
			break;
		case 7:
			dimblk2 = reader->getUtf8String();
			break;
		case 40:
			dimscale = reader->getDouble();
			break;
		case 41:
			dimasz = reader->getDouble();
			break;
		case 42:
			dimexo = reader->getDouble();
			break;
		case 43:
			dimdli = reader->getDouble();
			break;
		case 44:
			dimexe = reader->getDouble();
			break;
		case 45:
			dimrnd = reader->getDouble();
			break;
		case 46:
			dimdle = reader->getDouble();
			break;
		case 47:
			dimtp = reader->getDouble();
			break;
		case 48:
			dimtm = reader->getDouble();
			break;
		case 49:
			dimfxl = reader->getDouble();
			break;
		case 140:
			dimtxt = reader->getDouble();
			break;
		case 141:
			dimcen = reader->getDouble();
			break;
		case 142:
			dimtsz = reader->getDouble();
			break;
		case 143:
			dimaltf = reader->getDouble();
			break;
		case 144:
			dimlfac = reader->getDouble();
			break;
		case 145:
			dimtvp = reader->getDouble();
			break;
		case 146:
			dimtfac = reader->getDouble();
			break;
		case 147:
			dimgap = reader->getDouble();
			break;
		case 148:
			dimaltrnd = reader->getDouble();
			break;
		case 71:
			dimtol = reader->getInt32();
			break;
		case 72:
			dimlim = reader->getInt32();
			break;
		case 73:
			dimtih = reader->getInt32();
			break;
		case 74:
			dimtoh = reader->getInt32();
			break;
		case 75:
			dimse1 = reader->getInt32();
			break;
		case 76:
			dimse2 = reader->getInt32();
			break;
		case 77:
			dimtad = reader->getInt32();
			break;
		case 78:
			dimzin = reader->getInt32();
			break;
		case 79:
			dimazin = reader->getInt32();
			break;
		case 170:
			dimalt = reader->getInt32();
			break;
		case 171:
			dimaltd = reader->getInt32();
			break;
		case 172:
			dimtofl = reader->getInt32();
			break;
		case 173:
			dimsah = reader->getInt32();
			break;
		case 174:
			dimtix = reader->getInt32();
			break;
		case 175:
			dimsoxd = reader->getInt32();
			break;
		case 176:
			dimclrd = reader->getInt32();
			break;
		case 177:
			dimclre = reader->getInt32();
			break;
		case 178:
			dimclrt = reader->getInt32();
			break;
		case 179:
			dimadec = reader->getInt32();
			break;
		case 270:
			dimunit = reader->getInt32();
			break;
		case 271:
			dimdec = reader->getInt32();
			break;
		case 272:
			dimtdec = reader->getInt32();
			break;
		case 273:
			dimaltu = reader->getInt32();
			break;
		case 274:
			dimalttd = reader->getInt32();
			break;
		case 275:
			dimaunit = reader->getInt32();
			break;
		case 276:
			dimfrac = reader->getInt32();
			break;
		case 277:
			dimlunit = reader->getInt32();
			break;
		case 278:
			dimdsep = reader->getInt32();
			break;
		case 279:
			dimtmove = reader->getInt32();
			break;
		case 280:
			dimjust = reader->getInt32();
			break;
		case 281:
			dimsd1 = reader->getInt32();
			break;
		case 282:
			dimsd2 = reader->getInt32();
			break;
		case 283:
			dimtolj = reader->getInt32();
			break;
		case 284:
			dimtzin = reader->getInt32();
			break;
		case 285:
			dimaltz = reader->getInt32();
			break;
		case 286:
			dimalttz = reader->getInt32();
			break;
		case 287:
			dimfit = reader->getInt32();
			break;
		case 288:
			dimupt = reader->getInt32();
			break;
		case 289:
			dimatfit = reader->getInt32();
			break;
		case 290:
			dimfxlon = reader->getInt32();
			break;
		case 340:
			dimtxsty = reader->getUtf8String();
			break;
		case 341:
			dimldrblk = reader->getUtf8String();
			break;
		case 342:
			dimblk = reader->getUtf8String();
			break;
		case 343:
			dimblk1 = reader->getUtf8String();
			break;
		case 344:
			dimblk2 = reader->getUtf8String();
			break;
		default:
			DRW_TableEntry::parseCode(code, reader);
			break;
	}
}

bool DRW_Dimstyle::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	//2007+ //separate buffer for strings
	if (version > DRW::R2004)
		sBuf = &sBuff;
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing dimension style **************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("dimension style name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");
	// pending to tackle
#if 0

#if 0
	//* mslcock:the styles are added
	flags = buf->getBit() << 6;
	duint16 xrefindex = buf->getBitShort() - 1;
	DRW_DBG("dimension sytle xrefindex:");
	DRW_DBG(xrefindex);
	DRW_DBG("\n");

	duint8 xdep = buf->getBit();
	//flags |= xdep << 3;
	flags |= xdep << 4;
#endif

	//70 bit 7
	duint8 xrefref = buf->getBit();
	duint8 xrefdep = 0;
	duint16 xrefindex_plus1 = 0;
	if (version < DRW::R2007)
	{
		xrefindex_plus1 = buf->getBitShort();
		xrefdep = buf->getBit();
		flags = xrefdep << 4 | xrefref << 6;
	}
	else
	{
		if ((xrefdep = buf->getBit()) == 1)
		{
			xrefindex_plus1 = buf->getBitShort();
		}
		flags = xrefdep << 4 | xrefref << 6;
		// FIXME for R2007+ system vars
		return true;
	}


	//R13-R14
	if (version == DRW::R13 || version == DRW::R14)
	{
		dimtol = buf->getBit();
		dimlim = buf->getBit();
		dimtih = buf->getBit();
		dimtoh = buf->getBit();
		dimse1 = buf->getBit();
		dimse2 = buf->getBit();
		dimalt = buf->getBit();
		dimtofl = buf->getBit();
		dimsah = buf->getBit();
		dimtix = buf->getBit();
		dimsoxd = buf->getBit();
		dimaltd = buf->getRawChar8();
		dimzin = buf->getRawChar8();
		dimsd1 = buf->getBit();
		dimsd2 = buf->getBit();
		dimtolj = buf->getRawChar8();
		dimjust = buf->getRawChar8();
		dimfit = buf->getRawChar8();
		dimupt = buf->getBit();
		dimtzin = buf->getRawChar8();
		dimaltz = buf->getRawChar8();
		dimalttz = buf->getRawChar8();
		dimtad = buf->getRawChar8();
		dimunit = buf->getBitShort();
		dimaunit = buf->getBitShort();
		dimdec = buf->getBitShort();
		dimtdec = buf->getBitShort();
		dimaltu = buf->getBitShort();
		dimalttd = buf->getBitShort();
		dimscale = buf->getBitDouble();
		dimasz = buf->getBitDouble();
		dimexo = buf->getBitDouble();
		dimdli = buf->getBitDouble();
		dimexe = buf->getBitDouble();
		dimrnd = buf->getBitDouble();
		dimdle = buf->getBitDouble();
		dimtp = buf->getBitDouble();
		dimtm = buf->getBitDouble();
		dimtxt = buf->getBitDouble();
		dimcen = buf->getBitDouble();
		dimtsz = buf->getBitDouble();
		dimaltf = buf->getBitDouble();
		dimlfac = buf->getBitDouble();
		dimtvp = buf->getBitDouble();
		dimtfac = buf->getBitDouble();
		dimgap = buf->getBitDouble();
		dimpost = buf->getCP8Text();
		dimapost = buf->getCP8Text();
		dimblk = buf->getCP8Text();
		dimblk1 = buf->getCP8Text();
		dimblk2 = buf->getCP8Text();
		dimclrd = buf->getBitShort();
		dimclre = buf->getBitShort();
		dimclrt = buf->getBitShort();
	}

	// 2000+
	if (version > DRW::R14)
	{
		dimpost = buf->getVariableText(version, true);
		dimapost = buf->getVariableText(version, true);
		dimscale = buf->getBitDouble();
		dimasz = buf->getBitDouble();
		dimexo = buf->getBitDouble();
		dimdli = buf->getBitDouble();
		dimexe = buf->getBitDouble();
		dimrnd = buf->getBitDouble();
		dimdle = buf->getBitDouble();
		dimtp = buf->getBitDouble();
		dimtm = buf->getBitDouble();
	}

	// 2007+
	if (version > DRW::R2004)
	{
		dimfxl = buf->getBitDouble();
		dimjogang = buf->getBitDouble();
		dimtfill = buf->getBitShort();
		dimtfillclr = buf->getCmColor(version);
	}

	// R2000+
	if (version > DRW::R14)
	{
		dimtol = buf->getBit();
		dimlim = buf->getBit();
		dimtih = buf->getBit();
		dimtoh = buf->getBit();
		dimse1 = buf->getBit();
		dimse2 = buf->getBit();
		dimtad = buf->getBitShort();
		dimzin = buf->getBitShort();
		dimazin = buf->getBitShort();
	}

	// 2007+
	if (version > DRW::R2004)
	{
		dimarcsym = buf->getBitShort();
	}

	// 2000+
	if (version > DRW::R14)
	{
		dimtxt = buf->getBitDouble();
		dimcen = buf->getBitDouble();
		dimtsz = buf->getBitDouble();
		dimaltf = buf->getBitDouble();
		dimlfac = buf->getBitDouble();
		dimtvp = buf->getBitDouble();
		dimtfac = buf->getBitDouble();
		dimgap = buf->getBitDouble();
		dimaltrnd = buf->getBitDouble();
		dimalt = buf->getBit();
		dimaltd = buf->getBitShort();
		dimtofl = buf->getBit();
		dimsah = buf->getBit();
		dimtix = buf->getBit();
		dimsoxd = buf->getBit();
		// FIXME R2004
	/*    dimclrd = buf->getBitShort();
		dimclre = buf->getBitShort();
		dimclrt = buf->getBitShort();*/
		dimclrd = buf->getCmColor(version);
		dimclre = buf->getCmColor(version);
		dimclrt = buf->getCmColor(version);
		dimadec = buf->getBitShort();
		dimdec = buf->getBitShort();
		dimtdec = buf->getBitShort();
		dimaltu = buf->getBitShort();
		dimalttd = buf->getBitShort();
		dimaunit = buf->getBitShort();
		dimfrac = buf->getBitShort();
		dimlunit = buf->getBitShort();
		dimdsep = buf->getBitShort();
		dimtmove = buf->getBitShort();
		dimjust = buf->getBitShort();
		dimsd1 = buf->getBit();
		dimsd2 = buf->getBit();
		dimtolj = buf->getBitShort();
		dimtzin = buf->getBitShort();
		dimaltz = buf->getBitShort();
		dimalttz = buf->getBitShort();
		dimupt = buf->getBit();
		dimfit = buf->getBitShort();
	}
	// 2007+
	if (version > DRW::R2004)
	{
		dimfxlon = buf->getBit();
	}
	// 2010+
	if (version > DRW::R2007)
	{
		dimtxtdirection = buf->getBit();
		dimaltmzf = buf->getBitDouble();
		dimaltmzs == buf->getCP8Text();
		// !msclock:pending to verify
		dimmzs = buf->getCP8Text();
		dimmzf = buf->getBitDouble();
	}

	// 2000+
	if (version > DRW::R14)
	{
		dimlwd = buf->getBitShort();
		dimlwe = buf->getBitShort();
	}
	duint8 unknown = buf->getBit();

	//* msclock:pending to verify,but ok
	if (version > DRW::R2004)
	{
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}

	dwgHandle ctrlH = buf->getHandle();
	DRW_DBG("dimension control Handle: ");
	DRW_DBGHL(ctrlH.code, ctrlH.size, ctrlH.ref);
	parentHandle = ctrlH.ref;
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	for (int i = 0; i < numReactors; ++i)
	{
		dwgHandle reactorsH = buf->getHandle();
		DRW_DBG(" reactorsH control Handle: ");
		DRW_DBGHL(reactorsH.code, reactorsH.size, reactorsH.ref);
		DRW_DBG("\n");
	}
	if (xDictMissingFlag != 1)
	{
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	//* xref handle:external reference block handle
	dwgHandle XRefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
	DRW_DBG("\n");
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	dwgHandle shapefileH = buf->getHandle();
	DRW_DBG(" shapefileH control Handle: ");
	DRW_DBGHL(shapefileH.code, shapefileH.size, shapefileH.ref);
	DRW_DBG("\n");
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	if (version > DRW::R14)
	{
		dwgHandle leaderblockH = buf->getHandle();
		DRW_DBG(" leaderblockH control Handle: ");
		DRW_DBGHL(leaderblockH.code, leaderblockH.size, leaderblockH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");

		dwgHandle dimblkH = buf->getHandle();
		DRW_DBG(" dimblkH control Handle: ");
		DRW_DBGHL(dimblkH.code, dimblkH.size, dimblkH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");

		dwgHandle dimblk1H = buf->getHandle();
		DRW_DBG(" dimblk1H control Handle: ");
		DRW_DBGHL(dimblk1H.code, dimblk1H.size, dimblk1H.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");

		dwgHandle dimblk2H = buf->getHandle();
		DRW_DBG(" dimblk2H control Handle: ");
		DRW_DBGHL(dimblk2H.code, dimblk2H.size, dimblk2H.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}

	if (version > DRW::R2004)
	{
		dwgHandle dimtypeH = buf->getHandle();
		DRW_DBG(" dimtypeH control Handle: ");
		DRW_DBGHL(dimtypeH.code, dimtypeH.size, dimtypeH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");

		dwgHandle dimltex1H = buf->getHandle();
		DRW_DBG(" dimltex1H control Handle: ");
		DRW_DBGHL(dimltex1H.code, dimltex1H.size, dimltex1H.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");

		dwgHandle dimltex2H = buf->getHandle();
		DRW_DBG(" dimltex2H control Handle: ");
		DRW_DBGHL(dimltex2H.code, dimltex2H.size, dimltex2H.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
#endif
	//    RS crc;   //RS */
	return buf->isGood();
}

//! Class to handle line type entries
/*!
*  Class to handle line type symbol table entries
*  @author Rallaz
*/
void DRW_LType::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 3:
			description = reader->getUtf8String();
			break;
		case 73:
			dashesNum = reader->getInt32();
			dashes.reserve(dashesNum);
			break;
		case 40:
			patternLen = reader->getDouble();
			break;
		case 49:
		{
			dashes.emplace_back();
			dashes.back().dashLength = reader->getDouble();
			pathIdx++;
			break;
		}
		default:
			DRW_TableEntry::parseCode(code, reader);
			break;
	}
}




bool DRW_LType::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;

	if (version > DRW::R2004)
	{
		//2007+
		//separate buffer for strings
		sBuf = &sBuff;
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);

	DRW_DBG("\n***************************** parsing line type *********************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("linetype name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");
	flags = buf->getBit() << 6;
	DRW_DBG("flags: ");
	DRW_DBG(flags);
	if (version > DRW::R2004)
	{
		//2007+
	}
	else
	{
		//2004- 
		//RLZ: verify in 2004, 2010 &2013
		dint16 xrefindex = buf->getBitShort();
		DRW_DBG(" xrefindex-1: ");
		DRW_DBG(xrefindex - 1);
	}
	duint8 xdep = buf->getBit();
	DRW_DBG(" xdep(bit 16): ");
	DRW_DBG(xdep);
	flags |= xdep << 4;
	DRW_DBG(" flags: ");
	DRW_DBG(flags);

	description = sBuf->getVariableText(version, false);
	DRW_DBG(" desc: ");
	DRW_DBG(description.c_str());
	patternLen = buf->getBitDouble();
	DRW_DBG(" pattern length: ");
	DRW_DBG(patternLen);
	alignment = buf->getRawChar8();
	DRW_DBG(" align: ");
	DRW_DBG(std::string(1, static_cast<char>(alignment)));
	dashesNum = buf->getRawChar8();
	DRW_DBG(" num dashes, size: ");
	DRW_DBG(dashesNum);
	DRW_DBG("\n    dashes:\n");

	bool bHasStrArea = false;
	for (auto i = 0; i < dashesNum; i++)
	{
		dashes.emplace_back();
		dashes.back().dashLength = buf->getBitDouble();
		dashes.back().complexShapecode = buf->getBitShort();
		dashes.back().xOffset = buf->getRawDouble();
		dashes.back().yOffset = buf->getRawDouble();
		dashes.back().scale = buf->getBitDouble();
		dashes.back().rotation = buf->getBitDouble();
		dashes.back().shapeflag = buf->getBitShort();
		if ((dashes.back().shapeflag & 0x2))
			bHasStrArea = true;
	}

	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	updateTotalLengthOfPattern();

	std::vector<duint8> strArea;
	//2004-
	if (version < DRW::R2007)
	{
		strArea.resize(256);
		buf->getBytes(strArea.data(), 256);
		DRW_DBG("string area 256 bytes:\n");
		DRW_DBG(reinterpret_cast<char*>(strArea.data()));
		DRW_DBG("\n");
		bHasStrArea = true;
	}
	//2007+
	else
	{
		//first verify flag
		if (bHasStrArea)
		{
			strArea.resize(512);
			buf->getBytes(strArea.data(), 512);
			DRW_DBG("string area 512 bytes:\n");
			DRW_DBG(reinterpret_cast<char*>(strArea.data()));
			DRW_DBG("\n");
		}
		else
			DRW_DBG("string area 512 bytes not present\n");
	}

	if (version > DRW::R2007)
	{
		//2007+ skip string area
		DRW_DBG(" line type end of object data pos 2010: ");
		DRW_DBG(buf->getPosition());
		DRW_DBG(" strBuf b pos 2007: ");
		DRW_DBG(buf->getBitPos());
		DRW_DBG("\n");
	}

	if (version > DRW::R2004)
	{
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}

	if (version > DRW::R2007)
	{
		//2007+ skip string area
		DRW_DBG(" ltype start of handles data pos 2010: ");
		DRW_DBG(buf->getPosition());
		DRW_DBG(" strBuf bpos 2007: ");
		DRW_DBG(buf->getBitPos());
		DRW_DBG("\n");
	}

	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	dwgHandle ltControlH = buf->getHandle();
	DRW_DBG("linetype control Handle: ");
	DRW_DBGHL(ltControlH.code, ltControlH.size, ltControlH.ref);
	parentHandle = ltControlH.ref;
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	for (int i = 0; i < numReactors; ++i)
	{
		dwgHandle reactorsH = buf->getHandle();
		DRW_DBG(" reactorsH control Handle: ");
		DRW_DBGHL(reactorsH.code, reactorsH.size, reactorsH.ref);
		DRW_DBG("\n");
	}
	if (xDictMissingFlag != 1)
	{
		//linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	if (dashesNum > 0)
	{
		dwgHandle XRefH = buf->getHandle();
		DRW_DBG(" XRefH control Handle: ");
		DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
		DRW_DBG("\n");
		dwgHandle dashShapeHandle = buf->getHandle();
		DRW_DBG(" dash shape Handle: ");
		DRW_DBGHL(dashShapeHandle.code, dashShapeHandle.size, dashShapeHandle.ref);
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	dwgHandle shapeHandle = buf->getHandle();
	DRW_DBG(" shape Handle : ");
	DRW_DBGHL(shapeHandle.code, shapeHandle.size, shapeHandle.ref);
	DRW_DBG("\n");

	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

//! Class to handle layer entries
/*!
*  Class to handle layer symbol table entries
*  @author Rallaz
*/
void DRW_Layer::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 6:
			lineType = reader->getUtf8String();
			break;
		case 62:
			color = reader->getInt32();
			break;
		case 290:
			plotF = reader->getBool();
			break;
		case 370:
			lWeight = DRW_LW_Conv::dxfInt2lineWidth(reader->getInt32());
			break;
		case 390:
			handlePlotS = reader->getString();
			break;
		case 347:
			handleMaterialS = reader->getString();
			break;
		case 420:
			color24 = reader->getInt32();
			break;
		default:
			DRW_TableEntry::parseCode(code, reader);
			break;
	}
}

bool DRW_Layer::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing layer *********************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("layer name: ");
	DRW_DBG(entryName.c_str());

	flags |= buf->getBit() << 6; //layer have entity
	if (version < DRW::R2007)
	{ //2004-
		DRW_DBG(", xrefindex = ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("\n");
		//dint16 xrefindex = buf->getBitShort();
	}
	flags |= buf->getBit() << 4; //is refx dependent
	if (version < DRW::R2000)
	{                                //14-
		flags |= buf->getBit();      //layer frozen
		/*flags |=*/buf->getBit();   //unused, negate the color
		flags |= buf->getBit() << 1; //frozen in new
		flags |= buf->getBit() << 3; //locked
	}
	if (version > DRW::R14)
	{                                   //2000+
		dint16 f = buf->getSBitShort(); //bit2 are layer on
		DRW_DBG(", flags 2000+: ");
		DRW_DBG(f);
		DRW_DBG("\n");
		flags |= f & 0x0001;        //layer frozen
		flags |= (f >> 1) & 0x0002; //frozen in new
		flags |= (f >> 1) & 0x0004; //locked
		plotF = (f >> 4) & 0x0001;
		lWeight = DRW_LW_Conv::dwgInt2lineWidth((f & 0x03E0) >> 5);
	}
#if 0
	color = buf->getCmColor(version); //BS or CMC //ok for R14 or negate
#endif
	dwgColor = std::make_shared<DRW_CmcColor>();
	dwgColor->parseDwgColor(version, buf);

	DRW_DBG(", entity color: ");
	DRW_DBG(color);
	DRW_DBG("\n");

	if (version > DRW::R2004)
	{ 
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	dwgHandle layerControlH = buf->getHandle();
	DRW_DBG("layer control Handle: ");
	DRW_DBGHL(layerControlH.code, layerControlH.size, layerControlH.ref);
	parentHandle = layerControlH.ref;

	if (xDictMissingFlag != 1)
	{ 
		//linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
	}
	dwgHandle XRefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
	DRW_DBG("\n");
	if (version > DRW::R14)
	{ //2000+
		dwgHandle plotStyH = buf->getHandle();
		DRW_DBG(" PLot style control Handle: ");
		DRW_DBGHL(plotStyH.code, plotStyH.size, plotStyH.ref);
		DRW_DBG("\n");
		handlePlotS = DRW::toHexStr(plotStyH.ref); // std::string(plotStyH.ref);//RLZ: verify conversion
	}
	if (version > DRW::R2004)
	{ //2007+
		dwgHandle materialH = buf->getHandle();
		DRW_DBG(" Material control Handle: ");
		DRW_DBGHL(materialH.code, materialH.size, materialH.ref);
		DRW_DBG("\n");
		handleMaterialS = DRW::toHexStr(materialH.ref); //RLZ: verify conversion
	}
	//lineType handle
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	dwgHandle lTypeH = buf->getHandle();
	DRW_DBG("line type Handle: ");
	DRW_DBGHL(lTypeH.code, lTypeH.size, lTypeH.ref);
	lTypeHandleRef = lTypeH.ref;
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_Block_Header::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{

	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing block record ******************************************\n");
	if (!ret)
		return ret;
	duint32 insertCount = 0; //only 2000+
	dint64 objectCount = 0;  //only 2004+

	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("block record name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");

#if 0
	flags |= buf->getBit() << 6; //referenced external reference, block code 70, bit 7 (64)
#if 1
	if (version > DRW::R2004)
	{ //2007+

	}
	else
	{
		//2004- //RLZ: verify in 2004, 2010 &2013
		dint16 xrefindex = buf->getBitShort();
		DRW_DBG(" xrefindex: ");
		DRW_DBG(xrefindex);
		DRW_DBG("\n");
	}
#endif
	flags |= buf->getBit() << 4;       //is refx dependent, block code 70, bit 5 (16)
	flags |= buf->getBit();            //if is anonimous block (*U) block code 70, bit 1 (1)
	flags |= buf->getBit() << 1;       //if block contains attdefs, block code 70, bit 2 (2)
	bool blkIsXref = buf->getBit();  //if is a Xref, block code 70, bit 3 (4)
	bool xrefOverlaid = buf->getBit(); //if is a overlaid Xref, block code 70, bit 4 (8)
	flags |= blkIsXref << 2;         //if is a Xref, block code 70, bit 3 (4)
	flags |= xrefOverlaid << 3;        //if is a overlaid Xref, block code 70, bit 4 (8)
	if (version > DRW::R14)
	{                                //2000+
		flags |= buf->getBit() << 5; //if is a loaded Xref, block code 70, bit 6 (32)
	}
	DRW_DBG("flags: ");
	DRW_DBG(flags);
	DRW_DBG(", ");
#endif

#if 1
	duint8 xrefref, xrefdep;
	duint16 xrefindex_plus1;
	//70 bit 7
	xrefref = buf->getBit();
	if (version < DRW::R2007)
	{
		xrefindex_plus1 = buf->getBitShort();
		xrefdep = buf->getBit();
	}
	else // later
	{
		xrefdep = buf->getBit();
		if (xrefdep)
		{
			xrefindex_plus1 = buf->getBitShort();
		}
	}
	flags = flags | xrefdep << 4 | xrefref << 6;

	duint8 anonymous, hasAttrs, blkIsXref, xrefOverlaid, loadedBit;
	anonymous = buf->getBit();// bit1
	hasAttrs = buf->getBit(); //bit 2
	blkIsXref = buf->getBit();// bit 4
	xrefOverlaid = buf->getBit();// bit 8
	if (version >= DRW::R2000)
		loadedBit = buf->getBit();// bit 32

	flags = anonymous |
		hasAttrs << 1 |
		blkIsXref << 2 |
		xrefOverlaid << 3 |
		xrefdep << 4 |
		xrefref << 6;
#endif

	//R2004+
	if (version > DRW::R2000)
	{
		// FIXME:TODO
		//2004+ fails in 2007
		objectCount = buf->getBitLong(); //Number of objects owned by this block
		if (objectCount < 0)
			return true;
		ownedObjHandles.reserve(objectCount);
	}
	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("insertion point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	bool checkState = sBuf->isGood();
	UTF8STRING path = sBuf->getVariableText(version, false);
	DRW_DBG("XRef path name: ");
	if (checkState = sBuf->isGood())
	{
		spdlog::get("Xref path read  was invalid");
	}

	DRW_DBG(path.c_str());
	DRW_DBG("\n");

	if (version > DRW::R14)
	{ //2000+
		insertCount = 0;
		//修复外部参照读取错误

		if (!blkIsXref)
		{
			while (duint8 i = buf->getRawChar8() != 0)
				insertCount += i;
			UTF8STRING bkdesc = sBuf->getVariableText(version, false);
			DRW_DBG("Block description: ");
			DRW_DBG(bkdesc.c_str());
			DRW_DBG("\n");

			duint32 prevData = buf->getBitLong();
			for (duint32 j = 0; j < prevData; ++j)
				buf->getRawChar8();
		}
	}
	if (version > DRW::R2004)
	{ //2007+
		insUnits = buf->getBitShort();
		explodable = buf->getBit();
		blkScaling = buf->getRawChar8();
	}

	if (version > DRW::R2004)
	{
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	buf->setPosition(objBitSize >> 3);
	buf->setBitPos(objBitSize & 7);

	dwgHandle blockControlH = buf->getHandle();
	DRW_DBG("block control Handle: ");
	DRW_DBGHL(blockControlH.code, blockControlH.size, blockControlH.ref);
	DRW_DBG("\n");
	parentHandle = blockControlH.ref;

	for (int i = 0; i < numReactors; i++)
	{
		dwgHandle reactorH = buf->getHandle();
		DRW_DBG(" reactor Handle #");
		DRW_DBG(i);
		DRW_DBG(": ");
		DRW_DBGHL(reactorH.code, reactorH.size, reactorH.ref);
		DRW_DBG("\n");
	}
	if (xDictMissingFlag != 1)
	{ //R14+ //seems present in 2000
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
	}
	if (version != DRW::R2007)
	{ //2007+ XDicObjH or NullH not present
	}
	dwgHandle NullH = buf->getHandle();
	DRW_DBG(" NullH control Handle: ");
	DRW_DBGHL(NullH.code, NullH.size, NullH.ref);
	DRW_DBG("\n");
	dwgHandle blockH = buf->getOffsetHandle(handle);
	DRW_DBG(" blockH Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	block = blockH.ref;

	if (version > DRW::R2000)
	{
		if (objectCount > buf->size() - buf->getPosition())
		{
			objectCount = 0;
			DRW_DBG("reset objcount");
			return false;
		}
		else
		{
			//2004+
			for (unsigned int i = 0; i < objectCount; i++)
			{
				dwgHandle entityH = buf->getHandle();
				DRW_DBG(" entityH Handle #");
				DRW_DBG(i);
				DRW_DBG(": ");
				DRW_DBGHL(entityH.code, entityH.size, entityH.ref);
				DRW_DBG("\n");
				ownedObjHandles.push_back(entityH.ref);
			}
		}
	}
	else
	{
		//2000-
		if (!blkIsXref && !xrefOverlaid)
		{
			dwgHandle firstEntityH = buf->getHandle();
			DRW_DBG(" firstEntityH entity Handle: ");
			DRW_DBGHL(firstEntityH.code, firstEntityH.size, firstEntityH.ref);
			DRW_DBG("\n");
			firstEH = firstEntityH.ref;
			dwgHandle lastEntityH = buf->getHandle();
			DRW_DBG(" lastEntityH entity Handle: ");
			DRW_DBGHL(lastEntityH.code, lastEntityH.size, lastEntityH.ref);
			DRW_DBG("\n");
			lastEH = lastEntityH.ref;
		}
	}
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	dwgHandle endBlockH = buf->getOffsetHandle(handle);
	DRW_DBG(" endBlockH Handle: ");
	DRW_DBGHL(endBlockH.code, endBlockH.size, endBlockH.ref);
	DRW_DBG("\n");
	endBlock = endBlockH.ref;

	if (version > DRW::R14)
	{
		//过滤无效的insert读取
		if (block + 1 != endBlock)
		{	//2000+
			for (unsigned int i = 0; i < insertCount; i++)
			{
				dwgHandle insertH = buf->getHandle();
				DRW_DBG(" insertsH Handle #");
				DRW_DBG(i);
				DRW_DBG(": ");
				insertEnHandles.push_back(insertH.ref);
				DRW_DBGHL(insertH.code, insertH.size, insertH.ref);
				DRW_DBG("\n");
			}
		}
		else
		{
			// reset insertCount
			insertCount = 0;
		}
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		dwgHandle layoutH = buf->getHandle();
		DRW_DBG(" layoutH Handle: ");
		DRW_DBGHL(layoutH.code, layoutH.size, layoutH.ref);
		DRW_DBG("\n");
	}

	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n\n");
	//LOG_NONE;

	//    RS crc;   //RS */
	if (checkState)
		return buf->isGood();
	else
		return true;
}

//! Class to handle text style entries
/*!
*  Class to handle text style symbol table entries
*  @author Rallaz
*/
void DRW_Textstyle::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 3:
			font = reader->getUtf8String();
			break;
		case 4:
			bigFont = reader->getUtf8String();
			break;
		case 40:
			height = reader->getDouble();
			break;
		case 41:
			width = reader->getDouble();
			break;
		case 50:
			oblique = reader->getDouble();
			break;
		case 42:
			lastHeight = reader->getDouble();
			break;
		case 71:
			genFlag = reader->getInt32();
			break;
		case 1071:
			fontFamily = reader->getInt32();
			break;
		default:
			DRW_TableEntry::parseCode(code, reader);
			break;
	}
}

bool DRW_Textstyle::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing text style *********************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("text style name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");
	flags |= buf->getBit() << 6;  //style are referenced for a entity, style code 70, bit 7 (64)
	if (version <= DRW::R2004)
		dint16 xrefindex = buf->getBitShort();
	flags |= buf->getBit() << 4;  //is refx dependent, style code 70, bit 5 (16)
	flags |= buf->getBit();  //vertical text,1 bit of flag,stile code 70, bit 3 (4)
	flags |= buf->getBit() << 3;                   // if a shape file rather than a font,style code 70, bit 1 (1)
	height = buf->getBitDouble();                  // fixed height
	width = buf->getBitDouble();                   //width factor
	oblique = buf->getBitDouble();                 //oblique ang
	genFlag = buf->getRawChar8();                  //gen flag
	lastHeight = buf->getBitDouble();              // last height
	font = sBuf->getVariableText(version, false);  //font name
	bigFont = sBuf->getVariableText(version, false);
	if (version > DRW::R2004)
	{ //2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	dwgHandle shpControlH = buf->getHandle();
	DRW_DBG(" parentControlH Handle: ");
	DRW_DBGHL(shpControlH.code, shpControlH.size, shpControlH.ref);
	DRW_DBG("\n");
	parentHandle = shpControlH.ref;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (xDictMissingFlag != 1)
	{ //linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	/*RLZ: fails verify this part*/ dwgHandle XRefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
	DRW_DBG("\n");

	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

//! Class to handle vport entries
/*!
*  Class to handle vport symbol table entries
*  @author Rallaz
*/
void DRW_Vport::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 10:
			lowerLeft.x = reader->getDouble();
			break;
		case 20:
			lowerLeft.y = reader->getDouble();
			break;
		case 11:
			upperRight.x = reader->getDouble();
			break;
		case 21:
			upperRight.y = reader->getDouble();
			break;
		case 12:
			viewCenter.x = reader->getDouble();
			break;
		case 22:
			viewCenter.y = reader->getDouble();
			break;
		case 13:
			snapBase.x = reader->getDouble();
			break;
		case 23:
			snapBase.y = reader->getDouble();
			break;
		case 14:
			snapSpacing.x = reader->getDouble();
			break;
		case 24:
			snapSpacing.y = reader->getDouble();
			break;
		case 15:
			gridSpacing.x = reader->getDouble();
			break;
		case 25:
			gridSpacing.y = reader->getDouble();
			break;
		case 16:
			viewDir.x = reader->getDouble();
			break;
		case 26:
			viewDir.y = reader->getDouble();
			break;
		case 36:
			viewDir.z = reader->getDouble();
			break;
		case 17:
			viewTarget.x = reader->getDouble();
			break;
		case 27:
			viewTarget.y = reader->getDouble();
			break;
		case 37:
			viewTarget.z = reader->getDouble();
			break;
		case 40:
			viewHeight = reader->getDouble();
			break;
		case 41:
			aspectRatio = reader->getDouble();
			break;
		case 42:
			lensLen = reader->getDouble();
			break;
		case 43:
			frontClip = reader->getDouble();
			break;
		case 44:
			backClip = reader->getDouble();
			break;
		case 50:
			snapAngle = reader->getDouble();
			break;
		case 51:
			viewTwistAngle = reader->getDouble();
			break;
		case 71:
			viewMode = reader->getInt32();
			break;
		case 72:
			circleZoom = reader->getInt32();
			break;
		case 73:
			fastZoom = reader->getInt32();
			break;
		case 74:
			ucsIcon = reader->getInt32();
			break;
		case 75:
			snap = reader->getInt32();
			break;
		case 76:
			grid = reader->getInt32();
			break;
		case 77:
			snapStyle = reader->getInt32();
			break;
		case 78:
			snapIsopair = reader->getInt32();
			break;
		default:
			DRW_TableEntry::parseCode(code, reader);
			break;
	}
}

bool DRW_Vport::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing VPort ************************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("vport name: ");
	DRW_DBG(entryName);
	DRW_DBG("\n");
	flags |= buf->getBit() << 6; // code 70, bit 7 (64)
	if (version < DRW::R2007)
	{
		//2004-
		dint16 xrefindex = buf->getBitShort() - 1;

	}
	//! xdep B 70 dependent on an xref. (16 bit)bit 5
	flags |= buf->getBit() << 4;
	viewHeight = buf->getBitDouble();
	aspectRatio = buf->getBitDouble();
	DRW_DBG("flags: ");
	DRW_DBG(flags);
	DRW_DBG(" height: ");
	DRW_DBG(viewHeight);
	DRW_DBG(" ratio: ");
	DRW_DBG(aspectRatio);
	viewCenter = buf->get2RawDouble();
	DRW_DBG("\nview center: ");
	DRW_DBGPT(viewCenter.x, viewCenter.y, viewCenter.z);
	viewTarget.x = buf->getBitDouble();
	viewTarget.y = buf->getBitDouble();
	viewTarget.z = buf->getBitDouble();
	DRW_DBG("\nview target: ");
	DRW_DBGPT(viewTarget.x, viewTarget.y, viewTarget.z);
	viewDir.x = buf->getBitDouble();
	viewDir.y = buf->getBitDouble();
	viewDir.z = buf->getBitDouble();
	DRW_DBG("\nview dir: ");
	DRW_DBGPT(viewDir.x, viewDir.y, viewDir.z);
	viewTwistAngle = buf->getBitDouble();
	lensLen = buf->getBitDouble();
	frontClip = buf->getBitDouble();
	backClip = buf->getBitDouble();
	DRW_DBG("\ntwistAngle: ");
	DRW_DBG(viewTwistAngle);
	DRW_DBG(" lensHeight: ");
	DRW_DBG(lensLen);
	DRW_DBG(" frontClip: ");
	DRW_DBG(frontClip);
	DRW_DBG(" backClip: ");
	DRW_DBG(backClip);
	viewMode = buf->getBit();       //view mode, code 71, bit 0 (1)
	viewMode |= buf->getBit() << 1; //view mode, code 71, bit 1 (2)
	viewMode |= buf->getBit() << 2; //view mode, code 71, bit 2 (4)
	viewMode |= buf->getBit() << 4; //view mode, code 71, bit 4 (16)

	if (version > DRW::R14)
	{
		//2000+
		//duint8 renderMode = buf->getRawChar8();
		DRW_DBG("\n renderMode: ");
		const dint8 renderMode = buf->getRawChar8();
		DRW_DBG(renderMode);
		if (version > DRW::R2004)
		{
			//2007+
			DRW_DBG("\n use default lights: ");
			const auto useDefaultLights = buf->getBit();
			DRW_DBG(useDefaultLights);
			DRW_DBG(" default lighting type: ");
			const auto defaultLightType = buf->getRawChar8();
			DRW_DBG(defaultLightType);
			DRW_DBG(" brightness: ");
			const auto brightness = buf->getBitDouble();
			DRW_DBG(brightness);
			DRW_DBG("\n contrast: ");
			const auto contrast = buf->getBitDouble();
			DRW_DBG(contrast);
			DRW_DBG("\n");
			DRW_DBG(" ambient color CMC: ");
			const auto ambientColor = buf->getCmColor(version);
			DRW_DBG(ambientColor);
		}
	}

	lowerLeft = buf->get2RawDouble();
	DRW_DBG("\nlowerLeft: ");
	DRW_DBGPT(lowerLeft.x, lowerLeft.y, lowerLeft.z);
	upperRight = buf->get2RawDouble();
	DRW_DBG("\nUpperRight: ");
	DRW_DBGPT(upperRight.x, upperRight.y, upperRight.z);
	viewMode |= buf->getBit() << 3; //UCSFOLLOW, view mode, code 71, bit 3 (8)
	circleZoom = buf->getBitShort();
	fastZoom = buf->getBit();
	DRW_DBG("\nviewMode: ");
	DRW_DBG(viewMode);
	DRW_DBG(" circleZoom: ");
	DRW_DBG(circleZoom);
	DRW_DBG(" fastZoom: ");
	DRW_DBG(fastZoom);
	ucsIcon = buf->getBit();       //ucs Icon, code 74, bit 0 (1)
	ucsIcon |= buf->getBit() << 1; //ucs Icon, code 74, bit 1 (2)
	grid = buf->getBit();
	DRW_DBG("\nucsIcon: ");
	DRW_DBG(ucsIcon);
	DRW_DBG(" grid: ");
	DRW_DBG(grid);
	gridSpacing = buf->get2RawDouble();
	DRW_DBG("\ngrid Spacing: ");
	DRW_DBGPT(gridSpacing.x, gridSpacing.y, gridSpacing.z);
	snap = buf->getBit();
	snapStyle = buf->getBit();
	DRW_DBG("\nsnap on/off: ");
	DRW_DBG(snap);
	DRW_DBG(" snap Style: ");
	DRW_DBG(snapStyle);
	snapIsopair = buf->getBitShort();
	snapAngle = buf->getBitDouble();
	DRW_DBG("\nsnap Isopair: ");
	DRW_DBG(snapIsopair);
	DRW_DBG(" snap Angle: ");
	DRW_DBG(snapAngle);
	snapBase = buf->get2RawDouble();
	DRW_DBG("\nsnap Base: ");
	DRW_DBGPT(snapBase.x, snapBase.y, snapBase.z);
	snapSpacing = buf->get2RawDouble();
	DRW_DBG("\nsnap Spacing: ");
	DRW_DBGPT(snapSpacing.x, snapSpacing.y, snapSpacing.z);
	if (version > DRW::R14)
	{
		//2000+
		DRW_DBG("\n Unknown: ");
		DRW_DBG(buf->getBit());
		DRW_DBG(" UCS per Viewport: ");
		DRW_DBG(buf->getBit());
		DRW_DBG("\nUCS origin: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\nUCS X Axis: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\nUCS Y Axis: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\nUCS elevation: ");
		DRW_DBG(buf->getBitDouble());
		DRW_DBG(" UCS Orthographic type: ");
		DRW_DBG(buf->getBitShort());

		if (version > DRW::R2004)
		{
			//2007+
			gridBehavior = buf->getBitShort();
			DRW_DBG(" gridBehavior (flags): ");
			DRW_DBG(gridBehavior);
			DRW_DBG(" Grid major: ");
			DRW_DBG(buf->getBitShort());
		}
	}

	//common handles
	if (version > DRW::R2004)
	{
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	const auto vpControlH = buf->getHandle();
	DRW_DBG("\n parentControlH Handle: ");
	DRW_DBGHL(vpControlH.code, vpControlH.size, vpControlH.ref);
	DRW_DBG("\n");
	parentHandle = vpControlH.ref;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (xDictMissingFlag != 1)
	{
		const auto  XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	/*RLZ: fails verify this part*/
	//! External reference block handle (hard pointer)
	const auto xrefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(xrefH.code, xrefH.size, xrefH.ref);

	if (version > DRW::R14)
	{
		//2000+
		DRW_DBG("\nRemaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		if (version > DRW::R2004)
		{ //2007+
			const auto  backgroundHandle = buf->getHandle();
			DRW_DBG(" background Handle: ");
			DRW_DBGHL(backgroundHandle.code, backgroundHandle.size, backgroundHandle.ref);
			DRW_DBG("\nRemaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			const auto visualStyleHandle = buf->getHandle();
			DRW_DBG(" visual style Handle: ");
			DRW_DBGHL(visualStyleHandle.code, visualStyleHandle.size, visualStyleHandle.ref);
			DRW_DBG("\nRemaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			const auto  sunHandle = buf->getHandle();
			DRW_DBG(" sun Handle: ");
			DRW_DBGHL(sunHandle.code, sunHandle.size, sunHandle.ref);
			DRW_DBG("\nRemaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
		}
		const auto namedUCSHandle = buf->getHandle();
		DRW_DBG(" named UCS Handle: ");
		DRW_DBGHL(namedUCSHandle.code, namedUCSHandle.size, namedUCSHandle.ref);
		DRW_DBG("\nRemaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		const auto baseUCSHandle = buf->getHandle();
		DRW_DBG(" base UCS Handle: ");
		DRW_DBGHL(baseUCSHandle.code, baseUCSHandle.size, baseUCSHandle.ref);
	}
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_ImageDef::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 1:
			entryName = reader->getUtf8String();
			break;
		case 5:
			handle = reader->getHandleString();
			break;
		case 10:
			u = reader->getDouble();
			break;
		case 20:
			v = reader->getDouble();
			break;
		case 11:
			up = reader->getDouble();
			break;
		case 12:
			vp = reader->getDouble();
			break;
		case 21:
			vp = reader->getDouble();
			break;
		case 280:
			loaded = reader->getInt32();
			break;
		case 281:
			resolution = reader->getInt32();
			break;
		default:
			break;
	}
}

bool DRW_ImageDef::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing Image Def *********************************************\n");
	if (!ret)
		return ret;
	dint32 imgVersion = buf->getBitLong();
	DRW_DBG("class Version: ");
	DRW_DBG(imgVersion);
	DRW_Coord size = buf->get2RawDouble();
	DRW_UNUSED(size); //RLZ: temporary, complete API
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("appId name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");
	loaded = buf->getBit();
	resolution = buf->getRawChar8();
	up = buf->getRawDouble();
	vp = buf->getRawDouble();

	dwgHandle parentH = buf->getHandle();
	DRW_DBG(" parentH Handle: ");
	DRW_DBGHL(parentH.code, parentH.size, parentH.ref);
	DRW_DBG("\n");
	parentHandle = parentH.ref;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//RLZ: Reactors handles
	if (xDictMissingFlag != 1)
	{
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	/*RLZ: fails verify this part*/ dwgHandle XRefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
	DRW_DBG("\n");

	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_AppId::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing app Id *********************************************\n");
	if (!ret)
		return ret;
	entryName = sBuf->getVariableText(version, false);
	DRW_DBG("appId name: ");
	DRW_DBG(entryName.c_str());
	DRW_DBG("\n");
	flags |= buf->getBit() << 6; // code 70, bit 7 (64)
	/*dint16 xrefindex =*/buf->getBitShort();
	flags |= buf->getBit() << 4;         //is refx dependent, style code 70, bit 5 (16)
	duint8 unknown = buf->getRawChar8(); // unknown code 71
	DRW_DBG("unknown code 71: ");
	DRW_DBG(unknown);
	DRW_DBG("\n");
	if (version > DRW::R2004)
	{ //2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	dwgHandle appIdControlH = buf->getHandle();
	DRW_DBG(" parentControlH Handle: ");
	DRW_DBGHL(appIdControlH.code, appIdControlH.size, appIdControlH.ref);
	DRW_DBG("\n");
	parentHandle = appIdControlH.ref;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (xDictMissingFlag != 1)
	{ //linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	/*RLZ: fails verify this part*/ dwgHandle XRefH = buf->getHandle();
	DRW_DBG(" XRefH control Handle: ");
	DRW_DBGHL(XRefH.code, XRefH.size, XRefH.ref);
	DRW_DBG("\n");

	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_Layout::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_TableEntry::parseDwg(version, buf, sBuf, bs);
	DRW_DBG("\n***************************** parsing layout *********************************************\n");
	pageSetupName = sBuf->getVariableText(version, false);
	printerOrConfig = sBuf->getVariableText(version, false);

	plotLayoutFlags = buf->getBitShort();
	leftMargin = buf->getBitDouble();
	bottomMargin = buf->getBitDouble();
	rightMargin = buf->getBitDouble();
	topMargin = buf->getBitDouble();

	paperWith = buf->getBitDouble();
	paperHeight = buf->getBitDouble();

	paperSize = sBuf->getVariableText(version, false);

	plotOrigin.x = buf->getBitDouble();
	plotOrigin.y = buf->getBitDouble();
	paperUnits = buf->getBitShort();
	plotRotation = buf->getBitShort();
	plotType = buf->getBitShort();
	windowMin.y = buf->getBitDouble();
	windowMin.x = buf->getBitDouble();
	windowMax.y = buf->getBitDouble();
	windowMax.x = buf->getBitDouble();

	if (version <= DRW::R2000)
		plotViewName = sBuf->getCP8Text();
	realWorldUnits = buf->getBitDouble();
	drawingUnits = buf->getBitDouble();
	currentStyleSheet = sBuf->getVariableText(version, false);
	scaleType = buf->getBitShort();
	scaleFactor = buf->getBitDouble();
	paperImageOrigin.x = buf->getBitDouble();
	paperImageOrigin.y = buf->getBitDouble();

	if (version >= DRW::R2004)
	{
		shadePlotMode = buf->getBitShort();
		shadePlotResLevel = buf->getBitShort();
		shadePlotCustomDPI = buf->getBitShort();
	}

	layoutName = sBuf->getVariableText(version, false);
	tabOrder = buf->getBitLong();
	flag = buf->getBitShort();
	ucsOrigin = buf->get3BitDouble();
	limmin = buf->get2RawDouble();
	limmax = buf->get2RawDouble();
	inspoint = buf->get3BitDouble();
	ucsXAxis = buf->get3BitDouble();
	ucsYAxis = buf->get3BitDouble();
	elevation = buf->getBitDouble();
	orthoviewType = buf->getBitShort();
	extmin = buf->get3BitDouble();
	extmax = buf->get3BitDouble();

	if (version >= DRW::R2004)
		viewportCount = buf->getBitLong();// should 0 for test

	if (version > DRW::R2004)
	{ //2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}
	dwgHandle appIdControlH = buf->getHandle();
	DRW_DBG(" parentControlH Handle: ");
	DRW_DBGHL(appIdControlH.code, appIdControlH.size, appIdControlH.ref);
	DRW_DBG("\n");
	parentHandle = appIdControlH.ref;
	for (int i = 0; i < numReactors; ++i)
	{
		dwgHandle reactorsH = buf->getHandle();
		DRW_DBG(" reactorsH control Handle: ");
		DRW_DBGHL(reactorsH.code, reactorsH.size, reactorsH.ref);
		DRW_DBG("\n");
	}
	if (xDictMissingFlag != 1)
	{
		//linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}

	if (version >= DRW::R2004)
	{
		dwgHandle plotViewHandle = buf->getHandle();
	}
	if (version >= DRW::R2007)
	{
		dwgHandle visualStyleHandle = buf->getHandle();
	}

	dwgHandle associatedPaperspaceBlockRecordHandle = buf->getHandle();
	dwgHandle lastActiveViewportHandle = buf->getHandle();
	dwgHandle baseUcsHandle = buf->getHandle();
	dwgHandle namedUcsHandle = buf->getHandle();

	if (version >= DRW::R2004)
	{
		for (auto i = 0; i < viewportCount; ++i)
		{
			dwgHandle viewpportHandle = buf->getHandle();
		}
	}
	return true;
}
