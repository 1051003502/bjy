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

#include <cstdlib>
#include <iomanip>
#include "intern/dwgutil.h"
#include "drw_entities.h"
#include "intern/dxfreader.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"
//! Calculate arbitrary axis
/*!
*   Calculate arbitrary axis for apply extrusions
*  @author Rallaz
*/
void DRW_Entity::calculateAxis(DRW_Coord extPoint)
{
	// Follow the arbitrary DXF definitions for extrusion axes.
	if (fabs(extPoint.x) < 0.015625 && fabs(extPoint.y) < 0.015625)
	{
		//If we get here, implement Ax = Wy x N where Wy is [0,1,0] per the DXF spec.
		//The cross product works out to Wy.y*N.z-Wy.z*N.y, Wy.z*N.x-Wy.x*N.z, Wy.x*N.y-Wy.y*N.x
		//Factoring in the fixed values for Wy gives N.z,0,-N.x
		extAxisX.x = extPoint.z;
		extAxisX.y = 0;
		extAxisX.z = -extPoint.x;
	}
	else
	{
		//Otherwise, implement Ax = Wz x N where Wz is [0,0,1] per the DXF spec.
		//The cross product works out to Wz.y*N.z-Wz.z*N.y, Wz.z*N.x-Wz.x*N.z, Wz.x*N.y-Wz.y*N.x
		//Factoring in the fixed values for Wz gives -N.y,N.x,0.
		extAxisX.x = -extPoint.y;
		extAxisX.y = extPoint.x;
		extAxisX.z = 0;
	}

	extAxisX.unitize();

	//Ay = N x Ax
	extAxisY.x = (extPoint.y * extAxisX.z) - (extAxisX.y * extPoint.z);
	extAxisY.y = (extPoint.z * extAxisX.x) - (extAxisX.z * extPoint.x);
	extAxisY.z = (extPoint.x * extAxisX.y) - (extAxisX.x * extPoint.y);

	extAxisY.unitize();
}

//! Extrude a point using arbitrary axis
/*!
*   apply extrusion in a point using arbitrary axis (previous calculated)
*  @author Rallaz
*/
void DRW_Entity::extrudePoint(DRW_Coord extPoint, DRW_Coord* point)
{
	double px, py, pz;
	px = (extAxisX.x * point->x) + (extAxisY.x * point->y) + (extPoint.x * point->z);
	py = (extAxisX.y * point->x) + (extAxisY.y * point->y) + (extPoint.y * point->z);
	pz = (extAxisX.z * point->x) + (extAxisY.z * point->y) + (extPoint.z * point->z);

	point->x = px;
	point->y = py;
	point->z = pz;
}

bool DRW_Entity::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 5:
			handle = reader->getHandleString();
			break;
		case 330:
			parentHandle = reader->getHandleString();
			break;
		case 8:
			layer = reader->getUtf8String();
			break;
		case 6:
			lineType = reader->getUtf8String();
			break;
		case 62:
			color = reader->getInt32();
			break;
		case 370:
			lWeight = DRW_LW_Conv::dxfInt2lineWidth(reader->getInt32());
			break;
		case 48:
			linetypeScale = reader->getDouble();
			break;
		case 60:
			invisibleFlag = static_cast<duint8>(reader->getBool());
			break;
		case 420:
			color24 = reader->getInt32();
			break;
		case 430:
			colorName = reader->getString();
			break;
		case 67:
			space = static_cast<DRW::Space>(reader->getInt32());
			break;
		case 102:
			parseDxfGroups(code, reader);
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
	return true;
}

//parses dxf 102 groups to read entity
bool DRW_Entity::parseDxfGroups(int code, dxfReader* reader)
{
	std::list<DRW_Variant> ls;
	DRW_Variant curr;
	int nc;
	std::string appName = reader->getString();
	if (!appName.empty() && appName.at(0) == '{')
	{
		curr.addString(code, appName.substr(1, (int)appName.size() - 1));
		ls.push_back(curr);
		while (code != 102 && appName.at(0) == '}')
		{
			reader->readRec(&nc); //RLZ curr.code = code or nc?
								  //            curr.code = code;
			//RLZ code == 330 || code == 360 OR nc == 330 || nc == 360 ?
			if (code == 330 || code == 360)
				curr.addInt(code, reader->getHandleString()); //RLZ code or nc
			else
			{
				switch (reader->type)
				{
					case dxfReader::STRING:
						curr.addString(code, reader->getString()); //RLZ code or nc
						break;
					case dxfReader::INT32:
					case dxfReader::INT64:
						curr.addInt(code, reader->getInt32()); //RLZ code or nc
						break;
					case dxfReader::DOUBLE:
						curr.addDouble(code, reader->getDouble()); //RLZ code or nc
						break;
					case dxfReader::BOOL:
						curr.addInt(code, reader->getInt32()); //RLZ code or nc
						break;
					default:
						break;
				}
			}
			ls.push_back(curr);
		}
	}

	appData.push_back(ls);
	return true;
}

bool DRW_Entity::parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* strBuf, duint32 bs)
{
	objBitSize = 0;
	DRW_DBG("\n***************************** parsing entity *********************************************\n");
	dwgType = static_cast<DRW::DwgType>(buf->getObjType(version));
	DRW_DBG("Object type: ");
	DRW_DBG(dwgType);
	DRW_DBG(", ");
	DRW_DBGH(dwgType);

	if (version >= DRW::R2000 && version <= DRW::R2007)
	{
		//2000 & 2004
		objBitSize = buf->getRawLong32();
		//RL 32bits object size in bits
		DRW_DBG("\nObject size: ");
		DRW_DBG(objBitSize);
		DRW_DBG("\n");
	}

	if (version >= DRW::R2010)
	{
		//2010+
		duint64 ms = buf->size();
		objBitSize = ms * 8 - bs;
		DRW_DBG(" Object size: ");
		DRW_DBG(objBitSize);
		DRW_DBG("\n");
	}

	if (strBuf != nullptr && version >= DRW::R2007)
	{
		//2007+
		strBuf->moveBitPos(objBitSize - 1);
		DRW_DBG(" strBuf str bit pos 2007: ");
		DRW_DBG(strBuf->getPosition());
		DRW_DBG(" strBuf bit pos 2007: ");
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
			DRW_DBG(" strBuf bpos 2007: ");
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

	dwgHandle entityHandle = buf->getHandle();
	handle = entityHandle.ref;
	DRW_DBG("Entity Handle: ");
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

	duint8 graphPresentFlag = buf->getBit(); //B
	DRW_DBG(" graphFlag: ");
	DRW_DBG(graphPresentFlag);
	DRW_DBG("\n");
	if (graphPresentFlag)
	{
		duint64 graphDataSize{};
		if (version >= DRW::R13 && version <= DRW::R2007)
			graphDataSize = buf->getRawLong32(); //RL 32bits
		// version >= DRW::R2010
		else
			graphDataSize = buf->getBitLongLong();
		DRW_DBG("graphData in bytes: ");
		DRW_DBG(graphDataSize);
		DRW_DBG("\n");
		//todo skip graphData bytes first
		std::vector<duint8> graphData(graphDataSize, 0);
		buf->getBytes(graphData.data(), graphDataSize);
		dwgBuffer tmpGraphDataBuf(graphData.data(), graphDataSize, buf->decoder);
		DRW_DBG("graph data remaining bytes: ");
		DRW_DBG(tmpGraphDataBuf.numRemainingBytes());
		DRW_DBG("\n");

	}

	if (version <= DRW::R14)
	{
		//14-
		objBitSize = buf->getRawLong32(); //RL 32bits object size in bits
		DRW_DBG(" Object size in bits: ");
		DRW_DBG(objBitSize);
		DRW_DBG("\n");
	}

	duint8 entmode = buf->get2Bits();
	assert(entmode < 3);
	if (entmode == 0)
		hasOwner = true;
	if (entmode == 0)//verify the cast
		entmode = 2;
	space = static_cast<DRW::Space>(entmode);
	DRW_DBG("entmode: ");
	DRW_DBG(entmode);

	numReactors = buf->getBitLong(); //BL
	DRW_DBG(", numReactors: ");
	DRW_DBG(numReactors);

	//2004+
	if (version >= DRW::R2004)
	{
		xDictMissingFlag = buf->getBit();
		DRW_DBG(" xDictMissingFlag: ");
		DRW_DBG(xDictMissingFlag);
		DRW_DBG("\n");
	}
	// R2013+
	if (version >= DRW::R2013)
	{
		duint8 hasDSBinaryData = buf->getBit();
		if (hasDSBinaryData)
		{
			// TODO read binary data
		}
	}

	//14-
	if (version <= DRW::R14)
	{
		if (buf->getBit())
		{
			// is bylayer line type
			lineType = "ByLayer";
			linetypeFlag = 0;
		}
		else
		{
			lineType = "";
			linetypeFlag = 3;
		}
		DRW_DBG(" lineType: ");
		DRW_DBG(lineType.c_str());
		DRW_DBG(" ltFlags: ");
		DRW_DBG(linetypeFlag);
	}

	if (version >= DRW::R2004)
		noLinks = 1; // default 1 back and 1 forward
	else
		noLinks = buf->getBit();

#if 0
	//ENC color
	color = buf->getEnColor(version); //BS or CMC //ok for R14 or negate
#endif// todo redundant since it has DRW_Color
#if 1
	//! get entity color
	dwgColor = std::make_shared<DRW_EncColor>();
	dwgColor->parseDwgColor(version, buf);
#endif

	DRW_DBG(" entity color: ");
	DRW_DBG(color);
	linetypeScale = buf->getBitDouble(); //BD
	DRW_DBG(" linetypeScale: ");
	DRW_DBG(linetypeScale);
	DRW_DBG("\n");
	if (version >= DRW::R2000)
	{
		//2000+
		std::string plotStyleName;
		for (duint8 i = 0; i < 2; ++i)
		{
			//two flags in one
			plotstyleFlag = buf->get2Bits(); //BB
			if (plotstyleFlag == 0)
				plotStyleName = "ByLayer";
			else if (plotstyleFlag == 1)
				plotStyleName = "ByBlock";
			else if (plotstyleFlag == 2)
				plotStyleName = "continuous";
			else //handle at end handle stream
				plotStyleName = "";
			if (i == 0)
			{
				linetypeFlag = plotstyleFlag;
				//RLZ: how to solve? if needed plotStyleName;
				lineType = plotStyleName;
				DRW_DBG("ltFlags: "); DRW_DBG(linetypeFlag);
				DRW_DBG(" lineType: "); DRW_DBG(lineType);
			}
			else
			{
				DRW_DBG(", plotFlags: "); DRW_DBG(plotstyleFlag);
			}
		}
	}

	if (version >= DRW::R2007)
	{
		//2007+
		materialFlag = buf->get2Bits(); //BB
		DRW_DBG("materialFlag: ");
		DRW_DBG(materialFlag);
		// todo discriminate the kind of material
		// 00 bylayer
		// 01 byblock
		// 10 global 
		// 11 a handle at handle stream
		shadowFlag = buf->getRawChar8(); //RC
		DRW_DBG("shadowFlag: ");
		DRW_DBG(shadowFlag);
		DRW_DBG("\n");
	}
	if (version >= DRW::R2010)
	{
		//2010+
		hasFullVisualStyle = buf->getBit();
		DRW_DBG("\nhas full visual style:");
		DRW_DBG(hasFullVisualStyle);
		hasFaceVisualStyle = buf->getBit();
		DRW_DBG("\nhas face visual style:");
		DRW_DBG(hasFaceVisualStyle);
		hasEdgeVisualStyle = buf->getBit();
		DRW_DBG("\nhas edge visual style:");
		DRW_DBG(hasEdgeVisualStyle);
	}
	invisibleFlag = buf->getBitShort();//BS
	DRW_DBG(" invisibleFlag: ");
	DRW_DBG(invisibleFlag);
	if ((invisibleFlag & 0x1))
	{
		DRW_DBG("\ninvisible");
	}
	else
	{
		DRW_DBG("\nvisible");
	}

	if (version >= DRW::R2000)
	{
		//2000+
		lWeight = DRW_LW_Conv::dwgInt2lineWidth(buf->getRawChar8()); //RC
		DRW_DBG(" lwFlag (lWeight): ");
		DRW_DBG(lWeight);
		DRW_DBG("\n");
	}

	DRW_DBG("the common size is :");
	DRW_DBG(buf->getPosition());
	return buf->isGood();
}

bool DRW_Entity::parseDwgCommonEntityHandle(DRW::Version version, dwgBuffer* buf)
{

	if (version > DRW::R2004)
	{
		//2007+ skip string area
		buf->setPosition(objBitSize >> 3);
		buf->setBitPos(objBitSize & 7);
	}

	if (hasOwner)
	{
		//entity are in block or in a polyline
		dwgHandle ownerH = buf->getOffsetHandle(handle);
		DRW_DBG("owner (parent) Handle: ");
		DRW_DBGHL(ownerH.code, ownerH.size, ownerH.ref);
		DRW_DBG("\n");
		DRW_DBG("   Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		parentHandle = ownerH.ref;
		DRW_DBG("Block (parent) Handle: ");
		DRW_DBGHL(ownerH.code, ownerH.size, parentHandle);
		DRW_DBG("\n");
	}
	else
		DRW_DBG("NO Block (parent) Handle\n");

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
	{ //linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
	}
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//R14-
	if (version < DRW::R2000)
	{
		//layer handle
		dwgHandle layerHandle = buf->getOffsetHandle(handle);
		DRW_DBG(" layer Handle: ");
		DRW_DBGHL(layerHandle.code, layerHandle.size, layerHandle.ref);
		DRW_DBG("\n");
		layerHandleRef = layerHandle.ref;
		DRW_DBG("   Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		//lineType handle
		if (linetypeFlag == 3)
		{
			dwgHandle lTypeHandle = buf->getOffsetHandle(handle);
			DRW_DBG("linetype Handle: ");
			DRW_DBGHL(lTypeHandle.code, lTypeHandle.size, lTypeHandle.ref);
			DRW_DBG("\n");
			lTypeHandleRef = lTypeHandle.ref;
			DRW_DBG("   Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
		}
	}
	//2000-
	if (version <= DRW::R2000)
	{
		if (noLinks == 0)
		{
			dwgHandle nextLinkH = buf->getOffsetHandle(handle);
			DRW_DBG(" prev nextLinkers Handle: ");
			DRW_DBGHL(nextLinkH.code, nextLinkH.size, nextLinkH.ref);
			DRW_DBG("\n");
			DRW_DBG("\n Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
			preEntityLink = nextLinkH.ref;
			nextLinkH = buf->getOffsetHandle(handle);
			DRW_DBG(" next nextLinkers Handle: ");
			DRW_DBGHL(nextLinkH.code, nextLinkH.size, nextLinkH.ref);
			DRW_DBG("\n");
			DRW_DBG("\n Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
			nextEntityLink = nextLinkH.ref;
		}
		else
		{
			nextEntityLink = handle + 1;
			preEntityLink = handle - 1;
		}
	}

	//2004+
	if (version >= DRW::R2004)
	{
		//Parses Book color handle
		if (dwgColor->bAcDBColorPresent())
		{
			bookColorhandle = buf->getHandle();
		}
	}

	//2000+
	if (version >= DRW::R2000)
	{
		//layer handle
		dwgHandle layerHandle = buf->getOffsetHandle(handle);
		DRW_DBG(" layer Handle: ");
		DRW_DBGHL(layerHandle.code, layerHandle.size, layerHandle.ref);
		DRW_DBG("\n");
		layerHandleRef = layerHandle.ref;
		DRW_DBG("   Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		//lineType handle
		if (linetypeFlag == 3)
		{
			// lTypeH = buf->getOffsetHandle(handle);
			dwgHandle lTypeHandle = buf->getHandle();
			DRW_DBG("linetype Handle: ");
			DRW_DBGHL(lTypeHandle.code, lTypeHandle.size, lTypeHandle.ref);
			DRW_DBG("\n");
			lTypeHandleRef = lTypeHandle.ref;
			DRW_DBG("   Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
		}
	}
	if (version > DRW::R14)
	{
		//2000+
		if (version > DRW::R2004)
		{
			//2007+
			if (materialFlag == 3)
			{
				dwgHandle materialH = buf->getOffsetHandle(handle);
				DRW_DBG(" material Handle: ");
				DRW_DBGHL(materialH.code, materialH.size, materialH.ref);
				DRW_DBG("\n");
				DRW_DBG("\n Remaining bytes: ");
				DRW_DBG(buf->numRemainingBytes());
				DRW_DBG("\n");
			}
			if (shadowFlag == 3)
			{
				dwgHandle shadowH = buf->getOffsetHandle(handle);
				DRW_DBG(" shadow Handle: ");
				DRW_DBGHL(shadowH.code, shadowH.size, shadowH.ref);
				DRW_DBG("\n");
				DRW_DBG("\n Remaining bytes: ");
				DRW_DBG(buf->numRemainingBytes());
				DRW_DBG("\n");
			}
		}
		if (plotstyleFlag == 3)
		{
			//dwgHandle plotStyleH = buf->getOffsetHandle(handle);
			dwgHandle plotStyleH = buf->getHandle();
			DRW_DBG(" plot style Handle: ");
			DRW_DBGHL(plotStyleH.code, plotStyleH.size, plotStyleH.ref);
			DRW_DBG("\n");
			DRW_DBG("\n Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
		}
	}

	if (version >= DRW::R2010)
	{
		if (hasFullVisualStyle)
			auto fullvisualStyleHandle = buf->getHandle();
		if (hasFaceVisualStyle)
			auto faceVisualStyleHandle = buf->getHandle();
		if (hasEdgeVisualStyle)
			auto edgeVisualStyleHandle = buf->getHandle();
	}

	DRW_DBG("\n DRW_Entity::parseDwgEntHandle Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	return buf->isGood();
}

void DRW_Point::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 10:
			basePoint.x = reader->getDouble();
			break;
		case 20:
			basePoint.y = reader->getDouble();
			break;
		case 30:
			basePoint.z = reader->getDouble();
			break;
		case 39:
			thickness = reader->getDouble();
			break;
		case 210:
			haveExtrusion = true;
			extPoint.x = reader->getDouble();
			break;
		case 220:
			extPoint.y = reader->getDouble();
			break;
		case 230:
			extPoint.z = reader->getDouble();
			break;
		default:
			DRW_Entity::parseCode(code, reader);
			break;
	}
}

bool DRW_Point::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing point *********************************************\n");

	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	thickness = buf->getThickness(version > DRW::R14); //BD
	DRW_DBG("\nthickness: ");
	DRW_DBG(thickness);
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG(", Extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);

	double x_axis = buf->getBitDouble(); //BD
	DRW_DBG("\n  x_axis: ");
	DRW_DBG(x_axis);
	DRW_DBG("\n");
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */

	return buf->isGood();
}

void DRW_Line::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 11:
			secPoint.x = reader->getDouble();
			break;
		case 21:
			secPoint.y = reader->getDouble();
			break;
		case 31:
			secPoint.z = reader->getDouble();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

bool DRW_Line::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing line *********************************************\n");

	if (version < DRW::R2000)
	{ //14-
		basePoint.x = buf->getBitDouble();
		basePoint.y = buf->getBitDouble();
		basePoint.z = buf->getBitDouble();
		secPoint.x = buf->getBitDouble();
		secPoint.y = buf->getBitDouble();
		secPoint.z = buf->getBitDouble();
	}
	if (version > DRW::R14)
	{                                                    //2000+
		bool zIsZero = buf->getBit();                    //B
		basePoint.x = buf->getRawDouble();               //RD
		secPoint.x = buf->getDefaultDouble(basePoint.x); //DD
		basePoint.y = buf->getRawDouble();               //RD
		secPoint.y = buf->getDefaultDouble(basePoint.y); //DD
		if (!zIsZero)
		{
			basePoint.z = buf->getRawDouble();               //RD
			secPoint.z = buf->getDefaultDouble(basePoint.z); //DD
		}
	}
	DRW_DBG("start point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\nend point: ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	thickness = buf->getThickness(version > DRW::R14); //BD
	DRW_DBG("\nthickness: ");
	DRW_DBG(thickness);
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG(", Extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	DRW_DBG("\n");
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_Ray::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing ray/xline *********************************************\n");
	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	secPoint.x = buf->getBitDouble();
	secPoint.y = buf->getBitDouble();
	secPoint.z = buf->getBitDouble();
	DRW_DBG("start point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\nvector: ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Circle::applyExtrusion()
{
	if (haveExtrusion)
	{
		//NOTE: Commenting these out causes the the arcs being tested to be located
		//on the other side of the y axis (all x dimensions are negated).
		calculateAxis(extPoint);
		extrudePoint(extPoint, &basePoint);
	}
}

void DRW_Circle::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 40:
			radious = reader->getDouble();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

bool DRW_Circle::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing circle *********************************************\n");

	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("center: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	radious = buf->getBitDouble();
	DRW_DBG("\nradius: ");
	DRW_DBG(radious);

	thickness = buf->getThickness(version > DRW::R14);
	DRW_DBG(" thickness: ");
	DRW_DBG(thickness);
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG("\nextrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Arc::applyExtrusion()
{
	DRW_Circle::applyExtrusion();

	if (haveExtrusion)
	{
		// If the extrusion vector has a z value less than 0, the angles for the arc
		// have to be mirrored since DXF files use the right hand rule.
		// Note that the following code only handles the special case where there is a 2D
		// drawing with the z axis heading into the paper (or rather screen). An arbitrary
		// extrusion axis (with x and y values greater than 1/64) may still have issues.
		if (fabs(extPoint.x) < 0.015625 && fabs(extPoint.y) < 0.015625 && extPoint.z < 0.0)
		{
			sAngle = M_PI - sAngle;
			eAngle = M_PI - eAngle;
			/* swap */
			double temp = sAngle;
			sAngle = eAngle;
			eAngle = temp;
		}
	}
}

void DRW_Arc::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 50:
			sAngle = reader->getDouble() / ARAD;
			break;
		case 51:
			eAngle = reader->getDouble() / ARAD;
			break;
		default:
			DRW_Circle::parseCode(code, reader);
			break;
	}
}

bool DRW_Arc::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing circle arc *********************************************\n");

	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("center point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);

	radious = buf->getBitDouble();
	DRW_DBG("\nradius: ");
	DRW_DBG(radious);
	thickness = buf->getThickness(version > DRW::R14);
	DRW_DBG(" thickness: ");
	DRW_DBG(thickness);
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG("\nextrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	sAngle = buf->getBitDouble();
	DRW_DBG("\nstart angle: ");
	DRW_DBG(sAngle);
	eAngle = buf->getBitDouble();
	DRW_DBG(" end angle: ");
	DRW_DBG(eAngle);
	DRW_DBG("\n");
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	return buf->isGood();
}

void DRW_Ellipse::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 40:
			minormajoratio = reader->getDouble();
			break;
		case 41:
			sAngle = reader->getDouble();
			break;
		case 42:
			eAngle = reader->getDouble();
			break;
		default:
			DRW_Line::parseCode(code, reader);
			break;
	}
}

void DRW_Ellipse::applyExtrusion()
{
	if (haveExtrusion)
	{
		calculateAxis(extPoint);
		extrudePoint(extPoint, &secPoint);
		double intialparam = sAngle;
		if (extPoint.z < 0.)
		{
			sAngle = M_PIx2 - eAngle;
			eAngle = M_PIx2 - intialparam;
		}
	}
}

//if ratio > 1 minor axis are greather than major axis, correct it
void DRW_Ellipse::correctAxis()
{
	bool complete = false;
	if (sAngle == eAngle)
	{
		sAngle = 0.0;
		eAngle = M_PIx2;  //2*M_PI;
		complete = true;
	}
	if (minormajoratio > 1)
	{
		if (fabs(eAngle - sAngle - M_PIx2) < 1.0e-10)
			complete = true;
		double incX = secPoint.x;
		secPoint.x = -(secPoint.y * minormajoratio);
		secPoint.y = incX * minormajoratio;
		minormajoratio = 1 / minormajoratio;
		if (!complete)
		{
			if (sAngle < M_PI_2)
				sAngle += M_PI * 2;
			if (eAngle < M_PI_2)
				eAngle += M_PI * 2;
			eAngle -= M_PI_2;
			sAngle -= M_PI_2;
		}
	}
}

bool DRW_Ellipse::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing ellipse *********************************************\n");

	basePoint = buf->get3BitDouble();
	DRW_DBG("center: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	secPoint = buf->get3BitDouble();
	DRW_DBG(", axis: ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	DRW_DBG("\n");
	extPoint = buf->get3BitDouble();
	DRW_DBG("Extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	minormajoratio = buf->getBitDouble();  //BD
	DRW_DBG("\nratio: ");
	DRW_DBG(minormajoratio);
	sAngle = buf->getBitDouble();  //BD
	DRW_DBG(" start param: ");
	DRW_DBG(sAngle);
	eAngle = buf->getBitDouble();  //BD
	DRW_DBG(" end param: ");
	DRW_DBG(eAngle);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

//parts are the number of vertex to split polyline, default 128
void DRW_Ellipse::toPolyline(DRW_Polyline* pol, int parts)
{
	double radMajor, radMinor, cosRot, sinRot, incAngle, curAngle;
	double cosCurr, sinCurr;
	radMajor = sqrt(secPoint.x * secPoint.x + secPoint.y * secPoint.y);
	radMinor = radMajor * minormajoratio;
	//calculate sin & cos of included angle
	incAngle = atan2(secPoint.y, secPoint.x);
	cosRot = cos(incAngle);
	sinRot = sin(incAngle);
	incAngle = M_PIx2 / parts;
	curAngle = sAngle;
	int i = static_cast<int>(curAngle / incAngle);
	do
	{
		if (curAngle > eAngle)
		{
			curAngle = eAngle;
			i = parts + 2;
		}
		cosCurr = cos(curAngle);
		sinCurr = sin(curAngle);
		double x = basePoint.x + (cosCurr * cosRot * radMajor) - (sinCurr * sinRot * radMinor);
		double y = basePoint.y + (cosCurr * sinRot * radMajor) + (sinCurr * cosRot * radMinor);
		pol->addVertex(DRW_Vertex(x, y, 0.0, 0.0));
		curAngle = (++i) * incAngle;
	} while (i < parts);
	if (fabs(eAngle - sAngle - M_PIx2) < 1.0e-10)
	{
		pol->flags = 1;
	}
	pol->layer = this->layer;
	pol->lineType = this->lineType;
	pol->color = this->color;
	pol->lWeight = this->lWeight;
	pol->extPoint = this->extPoint;
}

void DRW_Trace::applyExtrusion()
{
	if (haveExtrusion)
	{
		calculateAxis(extPoint);
		extrudePoint(extPoint, &basePoint);
		extrudePoint(extPoint, &secPoint);
		extrudePoint(extPoint, &thirdPoint);
		extrudePoint(extPoint, &fourPoint);
	}
}

void DRW_Trace::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 12:
			thirdPoint.x = reader->getDouble();
			break;
		case 22:
			thirdPoint.y = reader->getDouble();
			break;
		case 32:
			thirdPoint.z = reader->getDouble();
			break;
		case 13:
			fourPoint.x = reader->getDouble();
			break;
		case 23:
			fourPoint.y = reader->getDouble();
			break;
		case 33:
			fourPoint.z = reader->getDouble();
			break;
		default:
			DRW_Line::parseCode(code, reader);
			break;
	}
}

bool DRW_Trace::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing Trace *********************************************\n");

	thickness = buf->getThickness(version > DRW::R14);
	basePoint.z = buf->getBitDouble();
	basePoint.x = buf->getRawDouble();
	basePoint.y = buf->getRawDouble();
	secPoint.x = buf->getRawDouble();
	secPoint.y = buf->getRawDouble();
	secPoint.z = basePoint.z;
	thirdPoint.x = buf->getRawDouble();
	thirdPoint.y = buf->getRawDouble();
	thirdPoint.z = basePoint.z;
	fourPoint.x = buf->getRawDouble();
	fourPoint.y = buf->getRawDouble();
	fourPoint.z = basePoint.z;
	extPoint = buf->getExtrusion(version > DRW::R14);

	DRW_DBG(" - base ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n - sec ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	DRW_DBG("\n - third ");
	DRW_DBGPT(thirdPoint.x, thirdPoint.y, thirdPoint.z);
	DRW_DBG("\n - fourth ");
	DRW_DBGPT(fourPoint.x, fourPoint.y, fourPoint.z);
	DRW_DBG("\n - extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	DRW_DBG("\n - thickness: ");
	DRW_DBG(thickness);
	DRW_DBG("\n");

	/* Common Entity Handle Data */
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;

	/* CRC X --- */
	return buf->isGood();
}

void DRW_Solid::parseCode(int code, dxfReader* reader)
{
	DRW_Trace::parseCode(code, reader);
}

bool DRW_Solid::parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs)
{
	DRW_DBG("\n***************************** parsing Solid *********************************************\n");
	return DRW_Trace::parseDwg(v, buf, bs);
}

void DRW_3Dface::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 70:
			invisibleflag = reader->getInt32();
			break;
		default:
			DRW_Trace::parseCode(code, reader);
			break;
	}
}

bool DRW_3Dface::parseDwg(DRW::Version v, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(v, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing 3Dface *********************************************\n");

	if (v < DRW::R2000)
	{ // R13 & R14
		basePoint.x = buf->getBitDouble();
		basePoint.y = buf->getBitDouble();
		basePoint.z = buf->getBitDouble();
		secPoint.x = buf->getBitDouble();
		secPoint.y = buf->getBitDouble();
		secPoint.z = buf->getBitDouble();
		thirdPoint.x = buf->getBitDouble();
		thirdPoint.y = buf->getBitDouble();
		thirdPoint.z = buf->getBitDouble();
		fourPoint.x = buf->getBitDouble();
		fourPoint.y = buf->getBitDouble();
		fourPoint.z = buf->getBitDouble();
		invisibleflag = buf->getBitShort();
	}
	else
	{ // 2000+
		bool has_no_flag = buf->getBit();
		bool z_is_zero = buf->getBit();
		basePoint.x = buf->getRawDouble();
		basePoint.y = buf->getRawDouble();
		basePoint.z = z_is_zero ? 0.0 : buf->getRawDouble();
		secPoint.x = buf->getDefaultDouble(basePoint.x);
		secPoint.y = buf->getDefaultDouble(basePoint.y);
		secPoint.z = buf->getDefaultDouble(basePoint.z);
		thirdPoint.x = buf->getDefaultDouble(secPoint.x);
		thirdPoint.y = buf->getDefaultDouble(secPoint.y);
		thirdPoint.z = buf->getDefaultDouble(secPoint.z);
		fourPoint.x = buf->getDefaultDouble(thirdPoint.x);
		fourPoint.y = buf->getDefaultDouble(thirdPoint.y);
		fourPoint.z = buf->getDefaultDouble(thirdPoint.z);
		invisibleflag = has_no_flag ? (int)NoEdge : buf->getBitShort();
	}
	drw_assert(invisibleflag >= NoEdge);
	drw_assert(invisibleflag <= AllEdges);

	DRW_DBG(" - base ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	DRW_DBG(" - sec ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	DRW_DBG("\n");
	DRW_DBG(" - third ");
	DRW_DBGPT(thirdPoint.x, thirdPoint.y, thirdPoint.z);
	DRW_DBG("\n");
	DRW_DBG(" - fourth ");
	DRW_DBGPT(fourPoint.x, fourPoint.y, fourPoint.z);
	DRW_DBG("\n");
	DRW_DBG(" - Invisibility mask: ");
	DRW_DBG(invisibleflag);
	DRW_DBG("\n");

	/* Common Entity Handle Data */
	ret = DRW_Entity::parseDwgCommonEntityHandle(v, buf);
	if (!ret)
		return ret;
	return buf->isGood();
}

void DRW_Block::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 2:
			name = reader->getUtf8String();
			break;
		case 70:
			flags = reader->getInt32();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

bool DRW_Block::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	//2007+
	if (version >= DRW::R2007)
		sBuf = &sBuff; //separate buffer for strings
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);

	if (!ret)
		return ret;



	if (!isEnd)
	{
		DRW_DBG("\n***************************** parsing block *********************************************\n");
		name = sBuf->getVariableText(version, false);
		DRW_DBG("Block name: ");
		DRW_DBG(name.c_str());
		DRW_DBG("\n");
	}
	else
	{
		DRW_DBG("\n***************************** parsing end block *********************************************\n");
	}
	if (version > DRW::R2004)
	{
		//2007+
		duint8 unk = buf->getBit();
		DRW_DBG("unknown bit: ");
		DRW_DBG(unk);
		DRW_DBG("\n");
	}
	//    X handleAssoc;   //X
	if (this->handle == 0)
	{
		return true;
	}
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Insert::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 2:
			blockName = reader->getUtf8String();
			break;
		case 41:
			xscale = reader->getDouble();
			break;
		case 42:
			yscale = reader->getDouble();
			break;
		case 43:
			zscale = reader->getDouble();
			break;
		case 50:
			angle = reader->getDouble();
			angle = angle / ARAD; //convert to radian
			break;
		case 70:
			colcount = reader->getInt32();
			break;
		case 71:
			rowcount = reader->getInt32();
			break;
		case 44:
			colspace = reader->getDouble();
			break;
		case 45:
			rowspace = reader->getDouble();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

bool DRW_Insert::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dint32 objCount = 0;
	bool ret = DRW_Entity::parseDwg(version, buf, nullptr, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n************************** parsing insert/minsert *****************************************\n");
	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("insertion point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	if (version < DRW::R2000)
	{ //14-
		xscale = buf->getBitDouble();
		yscale = buf->getBitDouble();
		zscale = buf->getBitDouble();
	}
	else
	{
		duint8 dataFlags = buf->get2Bits();
		if (dataFlags == 3)
		{
			//none default value 1,1,1
		}
		else if (dataFlags == 1)
		{ //x default value 1, y & z can be x value
			yscale = buf->getDefaultDouble(xscale);
			zscale = buf->getDefaultDouble(xscale);
		}
		else if (dataFlags == 2)
		{
			xscale = buf->getRawDouble();
			yscale = zscale = xscale;
		}
		else
		{
			//dataFlags == 0
			xscale = buf->getRawDouble();
			yscale = buf->getDefaultDouble(xscale);
			zscale = buf->getDefaultDouble(xscale);
		}
	}
	angle = buf->getBitDouble();
	//? check
	//extPoint = buf->getExtrusion(false); //3BD R14 style
	extPoint = buf->get3BitDouble();
	hasAttrib = buf->getBit() == 1 ? true : false;
	if (hasAttrib && version > DRW::R2000)
	{ //2004+
		objCount = buf->getBitLong();
	}
	if (dwgType == 8)
	{
		//entity are minsert
		colcount = buf->getBitShort();
		rowcount = buf->getBitShort();
		colspace = buf->getBitDouble();
		rowspace = buf->getBitDouble();
	}

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	blockHeaderH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */

	/* attributes follows*/
	if (hasAttrib)
	{
		if (version <= DRW::R2000)
		{
			//2000-
			dwgHandle attH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
			DRW_DBG("first attrib Handle: ");
			DRW_DBGHL(attH.code, attH.size, attH.ref);
			DRW_DBG("\n");
			firstAttribHandleRef = attH.ref;
			attH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
			//DRW_DBG("second attrib Handle: "); DRW_DBGHL(attH.code, attH.size, attH.ref); DRW_DBG("\n");
			DRW_DBG("last attrib Handle: ");
			DRW_DBGHL(attH.code, attH.size, attH.ref);
			DRW_DBG("\n");
			lastAttribHandleRef = attH.ref;
		}
		else
		{
			int count = 0;
			// R2004+
			while (count < objCount)
			{
				dwgHandle attribH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
				//i == 0 ? firstAttribHandleRef = attribH.ref : firstAttribHandleRef;
				//i == objCount - 1 ? lastAttribHandleRef = attribH.ref : lastAttribHandleRef;
				if (count == 0)
					firstAttribHandleRef = attribH.ref;
				if (count == objCount - 1)
				{
					lastAttribHandleRef = attribH.ref;
					break;
				}
				count++;
			}
		}
		seqendH = buf->getHandle(); /* H 2 BLOCK HEADER (hard pointer) */
	}

	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_LWPolyline::applyExtrusion()
{
	if (haveExtrusion)
	{
		calculateAxis(extPoint);
		for (unsigned int i = 0; i < vertlist.size(); i++)
		{
			auto vert = vertlist.at(i);
			DRW_Coord v(vert->x, vert->y, elevation);
			extrudePoint(extPoint, &v);
			vert->x = v.x;
			vert->y = v.y;
		}
	}
}

void DRW_LWPolyline::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 10:
		{
			vertex = new DRW_Vertex2D();
			vertlist.emplace_back(vertex);
			vertex->x = reader->getDouble();
			break;
		}
		case 20:
			if (vertex != NULL)
				vertex->y = reader->getDouble();
			break;
		case 40:
			if (vertex != NULL)
				vertex->stawidth = reader->getDouble();
			break;
		case 41:
			if (vertex != NULL)
				vertex->endwidth = reader->getDouble();
			break;
		case 42:
			if (vertex != NULL)
				vertex->bulge = reader->getDouble();
			break;
		case 38:
			elevation = reader->getDouble();
			break;
		case 39:
			thickness = reader->getDouble();
			break;
		case 43:
			width = reader->getDouble();
			break;
		case 70:
			flags = reader->getInt32();
			break;
		case 90:
			vertexnum = reader->getInt32();
			vertlist.reserve(vertexnum);
			break;
		case 210:
			haveExtrusion = true;
			extPoint.x = reader->getDouble();
			break;
		case 220:
			extPoint.y = reader->getDouble();
			break;
		case 230:
			extPoint.z = reader->getDouble();
			break;
		default:
			DRW_Entity::parseCode(code, reader);
			break;
	}
}

bool DRW_LWPolyline::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing LWPolyline *******************************************\n");

	flags = buf->getBitShort();
	DRW_DBG("flags value: ");
	DRW_DBG(flags);
	if (flags & 4)
		width = buf->getBitDouble();
	if (flags & 8)
		elevation = buf->getBitDouble();
	if (flags & 2)
		thickness = buf->getBitDouble();
	if (flags & 1)
		extPoint = buf->getExtrusion(false);
	vertexnum = buf->getBitLong();

	unsigned int bulgesNum = 0;
	if (flags & 16)
		bulgesNum = buf->getBitLong();

	int vertexIdCount = 0;
	if (version > DRW::R2007)
	{ //2010+
		if (flags & 1024)
			vertexIdCount = buf->getBitLong();
	}

	unsigned int widthsNum = 0;
	if (flags & 32)
		widthsNum = buf->getBitLong();

	DRW_DBG("\nvertex num: ");
	DRW_DBG(vertexnum);
	vertlist.reserve(vertexnum);

	DRW_DBG(" bulges num: ");
	DRW_DBG(bulgesNum);
	DRW_DBG(" vertexIdCount: ");
	DRW_DBG(vertexIdCount);
	DRW_DBG(" widths num: ");
	DRW_DBG(widthsNum);

	//clear all bit except 128 = plinegen and set 1 to open/close //RLZ:verify plinegen & open
	//dxf: plinegen 128 & open 1
	flags = (flags & 512) ? (flags | 1) : (flags | 0);
	flags &= 129;
	DRW_DBG("end flags value: ");
	DRW_DBG(flags);

	if (vertexnum > 0)
	{
		//verify if is lwpol without vertex (empty)
		// add vertexes
		this->vertex = new DRW_Vertex2D();
		this->vertex->x = buf->getRawDouble();
		this->vertex->y = buf->getRawDouble();
		vertlist.emplace_back(vertex);
		DRW_Vertex2D* pv = vertex;
		for (int i = 1; i < vertexnum; i++)
		{
			vertex = new DRW_Vertex2D();
			if (version < DRW::R2000)
			{ //14-
				vertex->x = buf->getRawDouble();
				vertex->y = buf->getRawDouble();
			}
			else
			{
				vertex->x = buf->getDefaultDouble(pv->x);
				vertex->y = buf->getDefaultDouble(pv->y);
			}
			pv = vertex;
			vertlist.emplace_back(vertex);
		}
		// add bulges
		for (unsigned int i = 0; i < bulgesNum; i++)
		{
			double bulge = buf->getBitDouble();
			if (vertlist.size() > i)
				vertlist.at(i)->bulge = bulge;
		}
		// add vertexId
		if (version > DRW::R2007)
		{
			//2010+
			for (int i = 0; i < vertexIdCount; i++)
			{
				vertlist.at(i)->vertexId = buf->getBitLong();
			}
		}
		//add widths
		for (unsigned int i = 0; i < widthsNum; i++)
		{
			if (!vertlist.empty())
			{
				vertlist.at(i)->stawidth = buf->getBitDouble();
				vertlist.at(i)->endwidth = buf->getBitDouble();
			}
		}
	}

	DRW_DBG("\nVertex list: ");
	for (auto it = vertlist.begin(); it != vertlist.end(); ++it)
	{
		const auto& pv = *it;
		DRW_DBG("\n   x: ");
		DRW_DBG(pv->x);
		DRW_DBG(" y: ");
		DRW_DBG(pv->y);
		DRW_DBG(" bulge: ");
		DRW_DBG(pv->bulge);
		DRW_DBG(" stawidth: ");
		DRW_DBG(pv->stawidth);
		DRW_DBG(" endwidth: ");
		DRW_DBG(pv->endwidth);
	}

	DRW_DBG("\n");
	/* Common Entity Handle Data */
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	/* CRC X --- */
	return buf->isGood();
}

void DRW_Text::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 40:
			height = reader->getDouble();
			break;
		case 41:
			rectWidth = reader->getDouble();
			break;
		case 50:
			angle = reader->getDouble();
			break;
		case 51:
			oblique = reader->getDouble();
			break;
		case 71:
			textgen = reader->getInt32();
			break;
		case 72:
			alignH = static_cast<HAlign>(reader->getInt32());
			break;
		case 73:
			alignV = static_cast<VAlign>(reader->getInt32());
			break;
		case 1:
			text = reader->getUtf8String();
			break;
		case 7:
			style = reader->getUtf8String();
			break;
		default:
			DRW_Line::parseCode(code, reader);
			break;
	}
}

bool DRW_Text::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer strBuff = *buf;
	dwgBuffer* strBuf = buf;
	//2007+
	if (version >= DRW::R2007)
	{
		strBuf = &strBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, strBuf, bs);
	if (!ret)
		return ret;

	DRW_DBG("\n***************************** parsing text *********************************************\n");

	// DataFlags RC Used to determine presence of subsequent data, set to 0xFF for R14-
	duint8 data_flags = 0x00;
	if (version >= DRW::R2000)
	{
		//2000+
		data_flags = buf->getRawChar8(); /* DataFlags RC Used to determine presence of subsequent data */
		DRW_DBG("data_flags: ");
		DRW_DBG(data_flags);
		DRW_DBG("\n");
		if (!(data_flags & 0x01))
		{
			/* Elevation RD --- present if !(DataFlags & 0x01) */
			basePoint.z = buf->getRawDouble();
		}
	}
	else//14-
	{
		basePoint.z = buf->getBitDouble(); /* Elevation BD --- */
	}
	basePoint.x = buf->getRawDouble(); /* Insertion pt 2RD 10 */
	basePoint.y = buf->getRawDouble();
	DRW_DBG("Insert point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	if (version >= DRW::R2000)
	{
		//2000+
		if (!(data_flags & 0x02))
		{
			/* Alignment pt 2DD 11 present if !(DataFlags & 0x02), use 10 & 20 values for 2 default values.*/
			secPoint.x = buf->getDefaultDouble(basePoint.x);
			secPoint.y = buf->getDefaultDouble(basePoint.y);
		}
		else
		{
			secPoint = basePoint;
		}
	}
	else
	{                                     //14-
		secPoint.x = buf->getRawDouble(); /* Alignment pt 2RD 11 */
		secPoint.y = buf->getRawDouble();
	}
	secPoint.z = basePoint.z;
	DRW_DBG("Alignment: ");
	DRW_DBGPT(secPoint.x, secPoint.y, basePoint.z);
	DRW_DBG("\n");
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG("Extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	DRW_DBG("\n");
	thickness = buf->getThickness(version > DRW::R14); /* Thickness BD 39 */

	if (version >= DRW::R2000)
	{
		//2000+
		if (!(data_flags & 0x04))
		{ /* Oblique ang RD 51 present if !(DataFlags & 0x04) */
			oblique = buf->getRawDouble();
		}
		if (!(data_flags & 0x08))
		{ /* Rotation ang RD 50 present if !(DataFlags & 0x08) */
			angle = buf->getRawDouble();
		}
		height = buf->getRawDouble(); /* Height RD 40 */
		if (!(data_flags & 0x10))
		{ /* Width factor RD 41 present if !(DataFlags & 0x10) */
			rectWidth = buf->getRawDouble();
		}
	}
	else
	{                                     //14-
		oblique = buf->getBitDouble();    /* Oblique ang BD 51 */
		angle = buf->getBitDouble();      /* Rotation ang BD 50 */
		height = buf->getBitDouble();     /* Height BD 40 */
		rectWidth = buf->getBitDouble(); /* Width factor BD 41 */
	}
	DRW_DBG("thickness: ");
	DRW_DBG(thickness);
	DRW_DBG(", Oblique ang: ");
	DRW_DBG(oblique);
	DRW_DBG(", Width: ");
	DRW_DBG(rectWidth);
	DRW_DBG(", Rotation: ");
	DRW_DBG(angle);
	DRW_DBG(", height: ");
	DRW_DBG(height);
	DRW_DBG("\n");
	text = strBuf->getVariableText(version, false); /* Text value TV 1 */

	DRW_DBG("text string: ");
	DRW_DBG(text.c_str());
	DRW_DBG("\n");
	//textgen, alignH, alignV always present in R14-, data_flags set in initialization
	if (!(data_flags & 0x20))
	{
		/* Generation BS 71 present if !(DataFlags & 0x20) */
		textgen = buf->getBitShort();
		DRW_DBG("textgen: ");
		DRW_DBG(textgen);
	}
	if (!(data_flags & 0x40))
	{
		/* Horizontal align. BS 72 present if !(DataFlags & 0x40) */
		alignH = static_cast<HAlign>(buf->getBitShort());
		DRW_DBG(", alignH: ");
		DRW_DBG(alignH);
	}
	if (!(data_flags & 0x80))
	{ /* Vertical align. BS 73 present if !(DataFlags & 0x80) */
		alignV = (VAlign)buf->getBitShort();
		DRW_DBG(", alignV: ");
		DRW_DBG(alignV);
	}
	DRW_DBG("\n");

	IS_ENTITY(DRW::TEXT)
	{
		PARSE_DWG_ENTITY_COMMON_HANDLE;
		styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
		DRW_DBG("text style Handle: ");
		DRW_DBGHL(styleH.code, styleH.size, styleH.ref);
		DRW_DBG("\n");
	}

	/* CRC X --- */
	return buf->isGood();
}

bool DRW_Text::parseDwgAndOutStrBuf(dwgBuffer* outStrBuf, DRW::Version version, dwgBuffer* buf, duint32 bs /*= 0*/)
{
	dwgBuffer* strBuf = outStrBuf;
	bool ret = DRW_Entity::parseDwg(version, buf, strBuf, bs);
	if (!ret)
		return ret;

	DRW_DBG("\n***************************** parsing text *********************************************\n");

	// DataFlags RC Used to determine presence of subsequent data, set to 0xFF for R14-
	duint8 data_flags = 0x00;
	if (version >= DRW::R2000)
	{
		//2000+
		data_flags = buf->getRawChar8(); /* DataFlags RC Used to determine presence of subsequent data */
		DRW_DBG("data_flags: ");
		DRW_DBG(data_flags);
		DRW_DBG("\n");
		if (!(data_flags & 0x01))
		{
			/* Elevation RD --- present if !(DataFlags & 0x01) */
			basePoint.z = buf->getRawDouble();
		}
	}
	else//14-
	{
		basePoint.z = buf->getBitDouble(); /* Elevation BD --- */
	}
	basePoint.x = buf->getRawDouble(); /* Insertion pt 2RD 10 */
	basePoint.y = buf->getRawDouble();
	DRW_DBG("Insert point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	if (version >= DRW::R2000)
	{
		//2000+
		if (!(data_flags & 0x02))
		{
			/* Alignment pt 2DD 11 present if !(DataFlags & 0x02), use 10 & 20 values for 2 default values.*/
			secPoint.x = buf->getDefaultDouble(basePoint.x);
			secPoint.y = buf->getDefaultDouble(basePoint.y);
		}
		else
		{
			secPoint = basePoint;
		}
	}
	else
	{                                     //14-
		secPoint.x = buf->getRawDouble(); /* Alignment pt 2RD 11 */
		secPoint.y = buf->getRawDouble();
	}
	secPoint.z = basePoint.z;
	DRW_DBG("Alignment: ");
	DRW_DBGPT(secPoint.x, secPoint.y, basePoint.z);
	DRW_DBG("\n");
	extPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG("Extrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	DRW_DBG("\n");
	thickness = buf->getThickness(version > DRW::R14); /* Thickness BD 39 */

	if (version >= DRW::R2000)
	{
		//2000+
		if (!(data_flags & 0x04))
		{ /* Oblique ang RD 51 present if !(DataFlags & 0x04) */
			oblique = buf->getRawDouble();
		}
		if (!(data_flags & 0x08))
		{ /* Rotation ang RD 50 present if !(DataFlags & 0x08) */
			angle = buf->getRawDouble();
		}
		height = buf->getRawDouble(); /* Height RD 40 */
		if (!(data_flags & 0x10))
		{ /* Width factor RD 41 present if !(DataFlags & 0x10) */
			rectWidth = buf->getRawDouble();
		}
	}
	else
	{                                     //14-
		oblique = buf->getBitDouble();    /* Oblique ang BD 51 */
		angle = buf->getBitDouble();      /* Rotation ang BD 50 */
		height = buf->getBitDouble();     /* Height BD 40 */
		rectWidth = buf->getBitDouble(); /* Width factor BD 41 */
	}
	DRW_DBG("thickness: ");
	DRW_DBG(thickness);
	DRW_DBG(", Oblique ang: ");
	DRW_DBG(oblique);
	DRW_DBG(", Width: ");
	DRW_DBG(rectWidth);
	DRW_DBG(", Rotation: ");
	DRW_DBG(angle);
	DRW_DBG(", height: ");
	DRW_DBG(height);
	DRW_DBG("\n");
	text = strBuf->getVariableText(version, false); /* Text value TV 1 */

	DRW_DBG("text string: ");
	DRW_DBG(text.c_str());
	DRW_DBG("\n");
	//textgen, alignH, alignV always present in R14-, data_flags set in initialization
	if (!(data_flags & 0x20))
	{
		/* Generation BS 71 present if !(DataFlags & 0x20) */
		textgen = buf->getBitShort();
		DRW_DBG("textgen: ");
		DRW_DBG(textgen);
	}
	if (!(data_flags & 0x40))
	{
		/* Horizontal align. BS 72 present if !(DataFlags & 0x40) */
		alignH = static_cast<HAlign>(buf->getBitShort());
		DRW_DBG(", alignH: ");
		DRW_DBG(alignH);
	}
	if (!(data_flags & 0x80))
	{ /* Vertical align. BS 73 present if !(DataFlags & 0x80) */
		alignV = (VAlign)buf->getBitShort();
		DRW_DBG(", alignV: ");
		DRW_DBG(alignV);
	}
	DRW_DBG("\n");

	IS_ENTITY(DRW::TEXT)
	{
		PARSE_DWG_ENTITY_COMMON_HANDLE;
		styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
		DRW_DBG("text style Handle: ");
		DRW_DBGHL(styleH.code, styleH.size, styleH.ref);
		DRW_DBG("\n");
	}

	/* CRC X --- */
	return buf->isGood();
}

void DRW_MText::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 1:
			text += reader->getString();
			text = reader->toUtf8String(text);
			break;
		case 11:
			haveXAxis = true;
			DRW_Text::parseCode(code, reader);
			break;
		case 3:
			text += reader->getString();
			break;
		case 44:
			interlin = reader->getDouble();
			break;
		default:
			DRW_Text::parseCode(code, reader);
			break;
	}
}

bool DRW_MText::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing mtext *********************************************\n");

	basePoint = buf->get3BitDouble(); /* Insertion pt 3BD 10 - First picked point. */
	DRW_DBG("Insertion: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\n");
	extPoint = buf->get3BitDouble(); /* Extrusion 3BD 210 Undocumented; */
	secPoint = buf->get3BitDouble(); /* X-axis dir 3BD 11 */
	updateAngle();
	rectWidth = buf->getBitDouble(); /* Rect width BD 41 */
	if (version > DRW::R2004)
	{   //2007+
		rectHeight = buf->getBitDouble();
	}
	height = buf->getBitDouble();
	textgen = buf->getBitShort(); /* Attachment BS 71 Similar to justification; */
	/* Drawing dir BS 72 Left to right, etc.; see DXF doc */
	dint16 draw_dir = buf->getBitShort();
	DRW_UNUSED(draw_dir);
	/* Extents ht BD Undocumented and not present in DXF or entget */
	extentsHeight = buf->getBitDouble();
	DRW_UNUSED(extentsHeight);
	/* Extents wid BD Undocumented and not present in DXF or entget The extents
	rectangle, when rotated the same as the text, fits the actual text image on
	the screen (although we've seen it include an extra row of text in height). */
	extentsWidth = buf->getBitDouble();
	DRW_UNUSED(extentsWidth);
	/* Text TV 1 All text in one long string (without '\n's 3 for line wrapping).
	ACAD seems to add braces ({ }) and backslash-P's to indicate paragraphs
	based on the "\r\n"'s found in the imported file. But, all the text is in
	this one long string -- not broken into 1- and 3-groups as in DXF and
	entget. ACAD's entget breaks this string into 250-char pieces (not 255 as
	doc'd) �C even if it's mid-word. The 1-group always gets the tag end;
	therefore, the 3's are always 250 chars long. */
	text = sBuf->getVariableText(version, false); /* Text value TV 1 */


	if (version > DRW::R14)
	{                        //2000+
		buf->getBitShort();  /* Line spacing Style BS 73 */
		buf->getBitDouble(); /* Line spacing Factor BD 44 */
		buf->getBit();       /* Unknown bit B */
	}

	if (version > DRW::R2000)
	{ //2004+
		backGroundFlags = buf->getBitLong();
		/** @todo add to DRW_MText */
		if ((backGroundFlags & 0x1) == 0x1 ||
			(version >= DRW::R2018
			 && (backGroundFlags & 0x10) == 0x10))
		{
			bkScaleFactor = buf->getBitLong();
			bkColor = buf->getCmColor(version); //RLZ: warning CMC or ENC
			/** @todo buf->getCMC */
			bkTransparency = buf->getBitLong();
		}
	}

#if 0
	if (version >= DRW::R2018)
	{
		bIsNAnnotative = buf->getBit();
		if (bIsNAnnotative)
		{
			mtextVersion = buf->getBitShort();
			defaultFlag = buf->getBit();
		}
		registerdApp = buf->getHandle().ref;
		attachPoint = buf->getBitLong();
		secPoint = buf->get3BitDouble();
		basePoint = buf->get3BitDouble();
		rectWidth = buf->getBitDouble();
		rectHeight = buf->getBitDouble();
		extentsWidth = buf->getBitDouble();
		extentsHeight = buf->getBitDouble();
		columnType = buf->getBitShort();
		if (columnType != 0)
		{
			colHeightCount = buf->getBitLong();
			colWidth = buf->getBitDouble();
			gutter = buf->getBitDouble();
			autoHeight = buf->getBit();
			flowReversed = buf->getBit();
			if (!autoHeight && columnType == 0x2)
			{
				for (int i = 0; i < colHeightCount; i++)
				{
					colHeightsVec.push_back(buf->getBitDouble());
				}
			}
		}
	}
#endif

	//! may correct the string data
	if (version >= DRW::R2007)
		text = buf->getVariableText(version, false);

	/* Common Entity Handle Data */
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;

	styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("text style Handle: ");
	DRW_DBG(styleH.ref);
	DRW_DBG("\n");

	/* CRC X --- */
	return buf->isGood();
}

void DRW_MText::updateAngle()
{
	if (haveXAxis)
	{
		angle = atan2(secPoint.y, secPoint.x) /** 180 / M_PI*/;
		//ensure it is positive,just around [0,6.28...]
		if (angle < 0)
			angle += M_PIx2;

	}
}

void DRW_Polyline::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 70:
			flags = reader->getInt32();
			break;
		case 40:
			defStartWidth = reader->getDouble();
			break;
		case 41:
			defEndWidth = reader->getDouble();
			break;
		case 71:
			vertCount = reader->getInt32();
			break;
		case 72:
			faceCount = reader->getInt32();
			break;
		case 73:
			smoothMDensity = reader->getInt32();
			break;
		case 74:
			smoothNDensity = reader->getInt32();
			break;
		case 75:
			curvetype = reader->getInt32();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

//0x0F polyline 2D bit 4(8) & 5(16) NOT set
//0x10 polyline 3D bit 4(8) set
//0x1D PFACE bit 5(16) set
//0x1E MESH 
bool DRW_Polyline::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing polyline *********************************************\n");

	if (dwgType == DRW::POLYLINE_2D)
	{
		DRW_DBG("2d polyline\n");
		flags = buf->getBitShort();
		curvetype = buf->getBitShort();
		defStartWidth = buf->getBitDouble();
		defEndWidth = buf->getBitDouble();
		thickness = buf->getThickness(version > DRW::R14);
		basePoint = DRW_Coord(0, 0, buf->getBitDouble());
		extPoint = buf->getExtrusion(version > DRW::R14);
	}
	else if (dwgType == DRW::POLYLINE_3D)
	{
		duint8 tmpFlag = buf->getRawChar8();
		DRW_DBG("flags 1 value: ");
		DRW_DBG(tmpFlag);
		if (tmpFlag & 0x1)
			curvetype = 5;
		else if (tmpFlag & 2)
			curvetype = 6;
		if (tmpFlag & 3)
		{
			curvetype = 8;//! pending to verify
			flags |= 4;
		}
		tmpFlag = buf->getRawChar8();
		DRW_DBG("flags 2 value: ");
		DRW_DBG(tmpFlag);
		if (tmpFlag & 1)
			flags |= 1;
		flags |= 8; //! indicate 3D polyline indicator
	}
	else if (dwgType == DRW::POLYLINE_PFACE)
	{
		flags = 64;
		DRW_DBG("flags value: ");
		DRW_DBG(flags);
		vertCount = buf->getBitShort();
		DRW_DBG("vertex count: ");
		DRW_DBG(vertCount);
		faceCount = buf->getBitShort();
		DRW_DBG("face count: ");
		DRW_DBG(faceCount);
	}
	else if (dwgType == DRW::POLYLINE_MESH)
	{
		flags = buf->getBitShort();
		DRW_DBG("flags value: ");
		DRW_DBG(flags);
		curvetype = buf->getBitShort();
		vertCount = buf->getBitShort();//! aka M vert count
		faceCount == buf->getBitShort();//! aka N vert count
		smoothMDensity = buf->getBitShort();
		smoothNDensity = buf->getBitShort();

	}

	dint32 ownedObjectCount = 0;
	if (version > DRW::R2000)
	{ //2004+
		ownedObjectCount = buf->getBitLong();
	}


	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;

	if (version < DRW::R2004)
	{
		//2000-
		dwgHandle vertH = buf->getOffsetHandle(handle);
		firstEH = vertH.ref;
		DRW_DBG(" first Vertex Handle: ");
		DRW_DBGHL(vertH.code, vertH.size, vertH.ref);
		DRW_DBG("\n");
		vertH = buf->getOffsetHandle(handle);
		lastEH = vertH.ref;
		DRW_DBG(" last Vertex Handle: ");
		DRW_DBGHL(vertH.code, vertH.size, vertH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	else
	{
		for (dint32 i = 0; i < ownedObjectCount; ++i)
		{
			dwgHandle objectH = buf->getOffsetHandle(handle);
			ownedObjhandles.push_back(objectH.ref);
			DRW_DBG(" Vertex Handle: ");
			DRW_DBGHL(objectH.code, objectH.size, objectH.ref);
			DRW_DBG("\n");
			DRW_DBG("Remaining bytes: ");
			DRW_DBG(buf->numRemainingBytes());
			DRW_DBG("\n");
		}
	}
	seqEndH = buf->getOffsetHandle(handle);

	if (dwgType == DRW::POLYLINE_MESH)
		assert(seqEndH.code == 3);

	DRW_DBG(" SEQEND Handle: ");
	DRW_DBGHL(seqEndH.code, seqEndH.size, seqEndH.ref);
	DRW_DBG("\n");

	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Vertex::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 70:
			flags = reader->getInt32();
			break;
		case 40:
			stawidth = reader->getDouble();
			break;
		case 41:
			endwidth = reader->getDouble();
			break;
		case 42:
			bulge = reader->getDouble();
			break;
		case 50:
			tgdir = reader->getDouble();
			break;
		case 71:
			vindex1 = reader->getInt32();
			break;
		case 72:
			vindex2 = reader->getInt32();
			break;
		case 73:
			vindex3 = reader->getInt32();
			break;
		case 74:
			vindex4 = reader->getInt32();
			break;
		case 91:
			identifier = reader->getInt32();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

//0x0A vertex 2D
//0x0B vertex 3D
//0x0C MESH
//0x0D PFACE
//0x0E PFACE FACE
bool DRW_Vertex::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs, double el)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing pline Vertex *********************************************\n");

	if (dwgType == 0x0A)
	{                               //pline 2D, needed example
		flags = buf->getRawChar8(); //RLZ: EC  unknown type
		DRW_DBG("flags value: ");
		DRW_DBG(flags);
		basePoint = buf->get3BitDouble();
		basePoint.z = el;
		DRW_DBG("basePoint: ");
		DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
		stawidth = buf->getBitDouble();
		if (stawidth < 0)
			endwidth = stawidth = fabs(stawidth);
		else
			endwidth = buf->getBitDouble();
		bulge = buf->getBitDouble();
		if (version > DRW::R2007) //2010+
			DRW_DBG("Vertex ID: ");
		DRW_DBG(buf->getBitLong());
		tgdir = buf->getBitDouble();
	}
	else if (dwgType == 0x0B || dwgType == 0x0C || dwgType == 0x0D)
	{                               //PFACE
		flags = buf->getRawChar8(); //RLZ: EC  unknown type
		DRW_DBG("flags value: ");
		DRW_DBG(flags);
		basePoint = buf->get3BitDouble();
		DRW_DBG("basePoint: ");
		DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	}
	else if (dwgType == 0x0E)
	{ //PFACE FACE
		vindex1 = buf->getBitShort();
		vindex2 = buf->getBitShort();
		vindex3 = buf->getBitShort();
		vindex4 = buf->getBitShort();
	}

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Hatch::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 2:
			name = reader->getUtf8String();
			break;
		case 70:
			solidFill = reader->getInt32();
			break;
		case 71:
			associative = reader->getInt32();
			break;
		case 72: /*edge type*/
			if (ispol)
			{ //if is polyline is a as_bulge flag
				break;
			}
			else if (reader->getInt32() == 1)
			{ //line
				addLine();
			}
			else if (reader->getInt32() == 2)
			{ //arc
				addArc();
			}
			else if (reader->getInt32() == 3)
			{ //elliptic arc
				addEllipse();
			}
			else if (reader->getInt32() == 4)
			{ //spline
				addSpline();
			}
			break;
		case 10:
			if (m_pPt)
				m_pPt->basePoint.x = reader->getDouble();
			else if (m_pLWPolyline)
			{
				m_pVertex2D = m_pLWPolyline->addVertex();
				m_pVertex2D->x = reader->getDouble();
			}
			break;
		case 20:
			if (m_pPt)
				m_pPt->basePoint.y = reader->getDouble();
			else if (m_pVertex2D)
				m_pVertex2D->y = reader->getDouble();
			break;
		case 11:
			if (m_pLine)
				m_pLine->secPoint.x = reader->getDouble();
			else if (m_pEllipse)
				m_pEllipse->secPoint.x = reader->getDouble();
			break;
		case 21:
			if (m_pLine)
				m_pLine->secPoint.y = reader->getDouble();
			else if (m_pEllipse)
				m_pEllipse->secPoint.y = reader->getDouble();
			break;
		case 40:
			if (m_pArc)
				m_pArc->radious = reader->getDouble();
			else if (m_pEllipse)
				m_pEllipse->minormajoratio = reader->getDouble();
			break;
		case 41:
			scaleOrSpacing = reader->getDouble();
			break;
		case 42:
			if (m_pVertex2D)
				m_pVertex2D->bulge = reader->getDouble();
			break;
		case 50:
			if (m_pArc)
				m_pArc->sAngle = reader->getDouble() / ARAD;
			else if (m_pEllipse)
				m_pEllipse->sAngle = reader->getDouble() / ARAD;
			break;
		case 51:
			if (m_pArc)
				m_pArc->eAngle = reader->getDouble() / ARAD;
			else if (m_pEllipse)
				m_pEllipse->eAngle = reader->getDouble() / ARAD;
			break;
		case 52:
			hatchAngle = reader->getDouble();
			break;
		case 73:
			if (m_pArc)
				m_pArc->isccw = reader->getInt32();
			else if (m_pLWPolyline)
				m_pLWPolyline->flags = reader->getInt32();
			break;
		case 75:
			hatchStyle = reader->getInt32();
			break;
		case 76:
			hatchPatternType = reader->getInt32();
			break;
		case 77:
			doubleHatchFlag = reader->getInt32();
			break;
		case 78:
			defLinesNum = reader->getInt32();
			break;
		case 91:
			numPaths = reader->getInt32();
			paths.reserve(numPaths);
			break;
		case 92:
			m_pHatchPath = new DRW_HatchPath(reader->getInt32());
			paths.emplace_back(m_pHatchPath);
			if (reader->getInt32() & 2)
			{
				ispol = true;
				resetEntsPtrs();
				m_pLWPolyline = new DRW_LWPolyline;
				m_pHatchPath->entities.emplace_back(m_pLWPolyline);
			}
			else
				ispol = false;
			break;
		case 93:
			if (m_pLWPolyline)
				m_pLWPolyline->vertexnum = reader->getInt32();
			else
				m_pHatchPath->edgesNum = reader->getInt32();  //aqui reserve
			break;
		case 98: //seed points ??
			resetEntsPtrs();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}

bool DRW_Hatch::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	duint32 boundaryObjHandlesNum = 0;
	bool havePixelSize = false;

	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing hatch *********************************************\n");

	//Gradient data, RLZ: is ok or if grad > 0 continue read ?
	if (version > DRW::R2000)
	{ //2004+
		isGradientFill = buf->getBitLong();
		DRW_DBG("is Gradient: ");
		DRW_DBG(isGradientFill);
		reversed = buf->getBitLong();
		DRW_DBG(" reserved: ");
		DRW_DBG(reversed);
		gradientAngle = buf->getBitDouble();
		DRW_DBG(" Gradient angle: ");
		DRW_DBG(gradientAngle);
		double gradShift = buf->getBitDouble();
		DRW_DBG(" Gradient shift: ");
		DRW_DBG(gradShift);
		dint32 singleCol = buf->getBitLong();
		DRW_DBG("\nsingle color Grad: ");
		DRW_DBG(singleCol);
		double gradTint = buf->getBitDouble();
		DRW_DBG(" Gradient tint: ");
		DRW_DBG(gradTint);
		dint32 colorNum = buf->getBitLong();
		DRW_DBG(" num colors: ");
		DRW_DBG(colorNum);
		for (dint32 i = 0; i < colorNum; ++i)
		{
			// named to shift value
			double shift_value = buf->getBitDouble();
			DRW_DBG("\nshift_value: ");
			DRW_DBG(shift_value);
			duint16 unkShort = buf->getBitShort();
			DRW_DBG(" unkShort: ");
			DRW_DBG(unkShort);
			duint32 rgbCol = buf->getBitLong();
			DRW_DBG(" rgb color: ");
			DRW_DBG(rgbCol);
			duint8 ignCol = buf->getRawChar8();
			DRW_DBG(" ignored color: ");
			DRW_DBG(ignCol);
		}
		UTF8STRING gradName = sBuf->getVariableText(version, false);
		DRW_DBG("\ngradient name: ");
		DRW_DBG(gradName.c_str());
		DRW_DBG("\n");
	}
	double elevation = buf->getBitDouble();
	basePoint.z = elevation < 1e-10 ? 0 : elevation;
	extPoint = buf->get3BitDouble();
	DRW_DBG("base point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	DRW_DBG("\nextrusion: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	name = sBuf->getVariableText(version, false);
	DRW_DBG("\nhatch pattern name: ");
	DRW_DBG(name.c_str());
	DRW_DBG("\n");
	solidFill = buf->getBit();
	associative = buf->getBit();
	numPaths = buf->getBitLong();

	//read paths
	for (dint32 i = 0; i < numPaths; ++i)
	{
		dint32 pathFlag = buf->getBitLong();
		//if (pathFlag > 50)
		//{
		//	//!msclock
		//	continue;
		//}
		m_pHatchPath = new DRW_HatchPath(pathFlag);
		havePixelSize |= (m_pHatchPath->pathFlag & 4);
		//Not polyline
		if (!(m_pHatchPath->pathFlag & 2))
		{
			dint32 numPathSeg = buf->getBitLong();
			for (dint32 j = 0; j < numPathSeg; ++j)
			{
				duint8 pathTypeStatus = buf->getRawChar8();
				//line
				if (pathTypeStatus == 1)
				{
					addLine();
					m_pLine->basePoint = buf->get2RawDouble();
					m_pLine->secPoint = buf->get2RawDouble();
				}
				//circular arc
				else if (pathTypeStatus == 2)
				{
					addArc();
					m_pArc->basePoint = buf->get2RawDouble(); /*center*/
					m_pArc->radious = buf->getBitDouble();
					m_pArc->sAngle = buf->getBitDouble();
					m_pArc->eAngle = buf->getBitDouble();
					m_pArc->isccw = buf->getBit();
				}
				//ellipse arc
				else if (pathTypeStatus == 3)
				{
					addEllipse();
					m_pEllipse->basePoint = buf->get2RawDouble();
					m_pEllipse->secPoint = buf->get2RawDouble();
					m_pEllipse->minormajoratio = buf->getBitDouble();
					m_pEllipse->sAngle = buf->getBitDouble();
					m_pEllipse->eAngle = buf->getBitDouble();
					m_pEllipse->isccw = buf->getBit();
				}
				//spline
				else if (pathTypeStatus == 4)
				{
					addSpline();
					m_pSpline->degree = buf->getBitLong();
					bool isRational = buf->getBit();
					m_pSpline->flags |= (isRational << 2);     //rational
					m_pSpline->flags |= (buf->getBit() << 1);  //periodic
					m_pSpline->knotsNum = buf->getBitLong(); /* knots num */
					m_pSpline->ctlPtsNum = buf->getBitLong();/* control points num */
					m_pSpline->knotslist.reserve(m_pSpline->knotsNum);
					m_pSpline->controllist.reserve(m_pSpline->ctlPtsNum);
					for (dint32 j = 0; j < m_pSpline->knotsNum; ++j)
						m_pSpline->knotslist.push_back(buf->getBitDouble());
					for (dint32 j = 0; j < m_pSpline->ctlPtsNum; ++j)
					{
						DRW_Coord* crd = new DRW_Coord(buf->get2RawDouble());
						if (isRational)
							crd->z = buf->getBitDouble();  // weight
						m_pSpline->controllist.emplace_back(crd);
					}
					if (version > DRW::R2007)
					{  //2010+
						m_pSpline->fitPtsNum = buf->getBitLong();
						m_pSpline->fitlist.reserve(m_pSpline->fitPtsNum);
						for (dint32 j = 0; j < m_pSpline->fitPtsNum; ++j)
						{
							DRW_Coord* crd = new DRW_Coord(buf->get2RawDouble());
							m_pSpline->fitlist.emplace_back(crd);
						}
						m_pSpline->sTangent = buf->get2RawDouble();
						m_pSpline->eTangent = buf->get2RawDouble();
					}
				}
			}
		}
		else
		{
			//polyline path
			//end not pline, start polyline
			m_pLWPolyline = new DRW_LWPolyline();
			bool bBuglesPresent = buf->getBit();
			m_pLWPolyline->flags = buf->getBit();  //closed bit
			dint32 numPathSegs = buf->getBitLong();
			for (dint32 j = 0; j < numPathSegs; ++j)
			{
				DRW_Vertex2D v;
				v.x = buf->getRawDouble();
				v.y = buf->getRawDouble();
				if (bBuglesPresent)
					v.bulge = buf->getBitDouble();
				m_pLWPolyline->addVertex(v);
			}
			m_pHatchPath->entities.emplace_back(m_pLWPolyline);
		}  //end polyline
		m_pHatchPath->updateEdgesNum();
		paths.emplace_back(m_pHatchPath);
		boundaryObjHandlesNum += buf->getBitLong();
		DRW_DBG(" totalBoundItems: ");
		DRW_DBG(boundaryObjHandlesNum);
	}  //end read loops

	hatchStyle = buf->getBitShort();
	hatchPatternType = buf->getBitShort();
	DRW_DBG("\nhatch style: ");
	DRW_DBG(hatchStyle);
	DRW_DBG(" pattern type");
	DRW_DBG(hatchPatternType);
	if (!solidFill)
	{
		hatchAngle = buf->getBitDouble();
		scaleOrSpacing = buf->getBitDouble();
		doubleHatchFlag = buf->getBit();
		defLinesNum = buf->getBitShort();
		for (dint32 i = 0; i < defLinesNum; ++i)
		{
			DRW_Coord linePt0, lineOffset;
			double lineAngle = buf->getBitDouble();
			// pattern through this point (X,Y)
			linePt0.x = buf->getBitDouble();
			linePt0.y = buf->getBitDouble();
			// pattern line offset
			lineOffset.x = buf->getBitDouble();
			lineOffset.y = buf->getBitDouble();
			duint16 dashesNum = buf->getBitShort();
			DRW_DBG("\ndef line: ");
			DRW_DBG(lineAngle);
			DRW_DBG(",");
			DRW_DBG(linePt0.x);
			DRW_DBG(",");
			DRW_DBG(linePt0.y);
			DRW_DBG(",");
			DRW_DBG(lineOffset.x);
			DRW_DBG(",");
			DRW_DBG(lineOffset.y);
			DRW_DBG(",");
			DRW_DBG(lineAngle);
			for (duint16 i = 0; i < dashesNum; ++i)
			{
				double lineDashLength = buf->getBitDouble();
				DRW_DBG(",");
				DRW_DBG(lineDashLength);
			}
		}  //end def lines
	} //end not solid

	if (havePixelSize)
	{
		double pixelSize = buf->getBitDouble();
		DRW_DBG("\npixel size: ");
		DRW_DBG(pixelSize);
	}

	dint32 seedPointsNum = buf->getBitLong();
	DRW_DBG("\nnum Seed Points  ");
	DRW_DBG(seedPointsNum);

	//read Seed Points
	DRW_Coord seedPt;
	for (dint32 i = 0; i < seedPointsNum; ++i)
	{
		seedPt.x = buf->getRawDouble();
		seedPt.y = buf->getRawDouble();
		DRW_DBG("\n  ");
		DRW_DBG(seedPt.x);
		DRW_DBG(",");
		DRW_DBG(seedPt.y);
	}
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	for (duint32 i = 0; i < boundaryObjHandlesNum; ++i)
	{
		dwgHandle biH = buf->getHandle();
		DRW_DBG("Boundary Items Handle: ");
		DRW_DBGHL(biH.code, biH.size, biH.ref);
	}
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Spline::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 210:
			normalVec.x = reader->getDouble();
			break;
		case 220:
			normalVec.y = reader->getDouble();
			break;
		case 230:
			normalVec.z = reader->getDouble();
			break;
		case 12:
			sTangent.x = reader->getDouble();
			break;
		case 22:
			sTangent.y = reader->getDouble();
			break;
		case 32:
			sTangent.z = reader->getDouble();
			break;
		case 13:
			eTangent.x = reader->getDouble();
			break;
		case 23:
			eTangent.y = reader->getDouble();
			break;
		case 33:
			eTangent.z = reader->getDouble();
			break;
		case 70:
			flags = reader->getInt32();
			break;
		case 71:
			degree = reader->getInt32();
			break;
		case 72:
			knotsNum = reader->getInt32();
			break;
		case 73:
			ctlPtsNum = reader->getInt32();
			break;
		case 74:
			fitPtsNum = reader->getInt32();
			break;
		case 42:
			tolknot = reader->getDouble();
			break;
		case 43:
			tolcontrol = reader->getDouble();
			break;
		case 44:
			tolfit = reader->getDouble();
			break;
		case 10:
		{
			controlpoint = new DRW_Coord();
			controllist.emplace_back(controlpoint);
			controlpoint->x = reader->getDouble();
			break;
		}
		case 20:
			if (controlpoint != NULL)
				controlpoint->y = reader->getDouble();
			break;
		case 30:
			if (controlpoint != NULL)
				controlpoint->z = reader->getDouble();
			break;
		case 11:
		{
			fitpoint = new DRW_Coord();
			fitlist.emplace_back(fitpoint);
			fitpoint->x = reader->getDouble();
			break;
		}
		case 21:
			if (fitpoint != NULL)
				fitpoint->y = reader->getDouble();
			break;
		case 31:
			if (fitpoint != NULL)
				fitpoint->z = reader->getDouble();
			break;
		case 40:
			knotslist.push_back(reader->getDouble());
			break;
			//    case 41:
			//        break;
		default:
			DRW_Entity::parseCode(code, reader);
			break;
	}
}

bool DRW_Spline::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	bool ret = DRW_Entity::parseDwg(version, buf, NULL, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing spline *********************************************\n");
	duint8 weight = 0;  // RLZ ??? flags, weight, code 70, bit 4 (16)

	dint32 scenario = buf->getBitLong();
	DRW_DBG("scenario: ");
	DRW_DBG(scenario);
	if (version > DRW::R2010)
	{
		dint32 splFlag1 = buf->getBitLong();
		if (splFlag1 & 1)
			scenario = 2;
		dint32 knotParam = buf->getBitLong();
		DRW_DBG("2013 splFlag1: ");
		DRW_DBG(splFlag1);
		DRW_DBG(" 2013 knotParam: ");
		DRW_DBG(knotParam);
		//        DRW_DBG("unk bit: "); DRW_DBG(buf->getBit());
	}
	degree = buf->getBitLong();  //RLZ: code 71, verify with dxf
	DRW_DBG(" degree: ");
	DRW_DBG(degree);
	DRW_DBG("\n");
	if (scenario == 2)
	{
		flags = 8;                     //scenario 2 = not rational & planar
		tolfit = buf->getBitDouble();  //BD
		DRW_DBG("flags: ");
		DRW_DBG(flags);
		DRW_DBG(" tolfit: ");
		DRW_DBG(tolfit);
		sTangent = buf->get3BitDouble();
		DRW_DBG(" Start Tangent: ");
		DRW_DBGPT(sTangent.x, sTangent.y, sTangent.z);
		eTangent = buf->get3BitDouble();
		DRW_DBG("\nEnd Tangent: ");
		DRW_DBGPT(eTangent.x, eTangent.y, eTangent.z);
		fitPtsNum = buf->getBitLong();
		DRW_DBG("\nnumber of fit points: ");
		DRW_DBG(fitPtsNum);
	}
	else if (scenario == 1)
	{
		flags = 8;                    //scenario 1 = rational & planar
		flags |= buf->getBit() << 2;  //flags, rational, code 70, bit 2 (4)
		flags |= buf->getBit();       //flags, closed, code 70, bit 0 (1)
		flags |= buf->getBit() << 1;  //flags, periodic, code 70, bit 1 (2)
		tolknot = buf->getBitDouble();
		tolcontrol = buf->getBitDouble();
		DRW_DBG("flags: ");
		DRW_DBG(flags);
		DRW_DBG(" knot tolerance: ");
		DRW_DBG(tolknot);
		DRW_DBG(" control point tolerance: ");
		DRW_DBG(tolcontrol);
		knotsNum = buf->getBitLong();
		ctlPtsNum = buf->getBitLong();
		weight = buf->getBit();  // RLZ ??? flags, weight, code 70, bit 4 (16)
		DRW_DBG("\nnum of knots: ");
		DRW_DBG(knotsNum);
		DRW_DBG(" num of control pt: ");
		DRW_DBG(ctlPtsNum);
		DRW_DBG(" weight bit: ");
		DRW_DBG(weight);
	}
	else
	{
		DRW_DBG("\ndwg Ellipse, unknouwn scenario\n");
		return false;  //RLZ: from doc only 1 or 2 are ok ?
	}

	knotslist.reserve(knotsNum);
	for (dint32 i = 0; i < knotsNum; ++i)
	{
		knotslist.push_back(buf->getBitDouble());
	}
	controllist.reserve(ctlPtsNum);
	for (dint32 i = 0; i < ctlPtsNum; ++i)
	{
		DRW_Coord* crd = new DRW_Coord(buf->get3BitDouble());
		controllist.emplace_back(crd);
		if (weight)
		{
			DRW_DBG("\n w: ");
			DRW_DBG(buf->getBitDouble());  //RLZ Warning: D (BD or RD)
		}
	}
	fitlist.reserve(fitPtsNum);
	for (dint32 i = 0; i < fitPtsNum; ++i)
	{
		DRW_Coord* crd = new DRW_Coord(buf->get3BitDouble());
		fitlist.emplace_back(crd);
	}
	if (DRW_DBGGL == DRW_dbg::DEBUGG)
	{
		DRW_DBG("\nknots list: ");
		for (std::vector<double>::iterator it = knotslist.begin(); it != knotslist.end(); ++it)
		{
			DRW_DBG("\n");
			DRW_DBG(*it);
		}
		DRW_DBG("\ncontrol point list: ");
		for (auto it = controllist.begin(); it != controllist.end(); ++it)
		{
			DRW_DBG("\n");
			DRW_DBGPT((*it)->x, (*it)->y, (*it)->z);
		}
		DRW_DBG("\nfit point list: ");
		for (auto it = fitlist.begin(); it != fitlist.end(); ++it)
		{
			DRW_DBG("\n");
			DRW_DBGPT((*it)->x, (*it)->y, (*it)->z);
		}
	}

	/* Common Entity Handle Data */
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Image::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 12:
			vVector.x = reader->getDouble();
			break;
		case 22:
			vVector.y = reader->getDouble();
			break;
		case 32:
			vVector.z = reader->getDouble();
			break;
		case 13:
			sizeu = reader->getDouble();
			break;
		case 23:
			sizev = reader->getDouble();
			break;
		case 340:
			ref = reader->getHandleString();
			break;
		case 280:
			clip = reader->getInt32();
			break;
		case 281:
			brightness = reader->getInt32();
			break;
		case 282:
			contrast = reader->getInt32();
			break;
		case 283:
			fade = reader->getInt32();
			break;
		default:
			DRW_Line::parseCode(code, reader);
			break;
	}
}

bool DRW_Image::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing image *********************************************\n");

	dint32 classVersion = buf->getBitLong();
	DRW_DBG("class Version: ");
	DRW_DBG(classVersion);
	basePoint = buf->get3BitDouble();
	DRW_DBG("\nbase point: ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	secPoint = buf->get3BitDouble();
	DRW_DBG("\nU vector: ");
	DRW_DBGPT(secPoint.x, secPoint.y, secPoint.z);
	vVector = buf->get3BitDouble();
	DRW_DBG("\nV vector: ");
	DRW_DBGPT(vVector.x, vVector.y, vVector.z);
	sizeu = buf->getRawDouble();
	sizev = buf->getRawDouble();
	DRW_DBG("\nsize U: ");
	DRW_DBG(sizeu);
	DRW_DBG("\nsize V: ");
	DRW_DBG(sizev);
	duint16 displayProps = buf->getBitShort();
	DRW_UNUSED(displayProps); //RLZ: temporary, complete API
	clip = buf->getBit();
	brightness = buf->getRawChar8();
	contrast = buf->getRawChar8();
	fade = buf->getRawChar8();
	if (version > DRW::R2007)
	{ //2010+
		bool clipMode = buf->getBit();
		DRW_UNUSED(clipMode); //RLZ: temporary, complete API
	}
	duint16 clipType = buf->getBitShort();
	if (clipType == 1)
	{
		buf->get2RawDouble();
		buf->get2RawDouble();
	}
	else
	{ //clipType == 2
		dint32 numVerts = buf->getBitLong();
		for (int i = 0; i < numVerts; ++i)
			buf->get2RawDouble();
	}

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	dwgHandle biH = buf->getHandle();
	DRW_DBG("ImageDef Handle: ");
	DRW_DBGHL(biH.code, biH.size, biH.ref);
	ref = biH.ref;
	biH = buf->getHandle();
	DRW_DBG("ImageDefReactor Handle: ");
	DRW_DBGHL(biH.code, biH.size, biH.ref);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Dimension::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 1:
			userText = reader->getUtf8String();
			break;
		case 2:
			name = reader->getString();
			break;
		case 3:
			style = reader->getUtf8String();
			break;
		case 70:
			type = reader->getInt32();
			break;
		case 71:
			attchPt = reader->getInt32();
			break;
		case 72:
			lineSpaceStyle = reader->getInt32();
			break;
		case 10:
			pt10.x = reader->getDouble();
			break;
		case 20:
			pt10.y = reader->getDouble();
			break;
		case 30:
			pt10.z = reader->getDouble();
			break;
		case 11:
			textPoint.x = reader->getDouble();
			break;
		case 21:
			textPoint.y = reader->getDouble();
			break;
		case 31:
			textPoint.z = reader->getDouble();
			break;
		case 12:
			clonePoint.x = reader->getDouble();
			break;
		case 22:
			clonePoint.y = reader->getDouble();
			break;
		case 32:
			clonePoint.z = reader->getDouble();
			break;
		case 13:
			pt13.x = reader->getDouble();
			break;
		case 23:
			pt13.y = reader->getDouble();
			break;
		case 33:
			pt13.z = reader->getDouble();
			break;
		case 14:
			pt14.x = reader->getDouble();
			break;
		case 24:
			pt14.y = reader->getDouble();
			break;
		case 34:
			pt14.z = reader->getDouble();
			break;
		case 15:
			pt15.x = reader->getDouble();
			break;
		case 25:
			pt15.y = reader->getDouble();
			break;
		case 35:
			pt15.z = reader->getDouble();
			break;
		case 16:
			arcPoint.x = reader->getDouble();
			break;
		case 26:
			arcPoint.y = reader->getDouble();
			break;
		case 36:
			arcPoint.z = reader->getDouble();
			break;
		case 41:
			lineSpaceFactor = reader->getDouble();
			break;
		case 53:
			textRot = reader->getDouble();
			break;
		case 50:
			angle = reader->getDouble();
			break;
		case 52:
			oblique = reader->getDouble();
			break;
		case 40:
			length = reader->getDouble();
			break;
		case 51:
			horizDir = reader->getDouble();
			break;
		default:
			DRW_Entity::parseCode(code, reader);
			break;
	}
}

bool DRW_Dimension::parseDwg(DRW::Version version, dwgBuffer* buf, dwgBuffer* sBuf)
{
	DRW_DBG("\n***************************** parsing dimension *********************************************");
	if (version >= DRW::R2010)
	{
		//2010+
		duint8 dimVersion = buf->getRawChar8();
		DRW_DBG("\ndimVersion: ");
		DRW_DBG(dimVersion);
	}

	//extPoint = buf->getExtrusion(version > DRW::R14);
	extPoint = buf->get3BitDouble();
	DRW_DBG("\nextPoint: ");
	DRW_DBGPT(extPoint.x, extPoint.y, extPoint.z);
	//if SINCE(DRW::R2000)
	//{
	//	//2000+
	//	DRW_DBG("\nFive unknown bits: ");
	//	DRW_DBG(buf->getBit());
	//	DRW_DBG(buf->getBit());
	//	DRW_DBG(buf->getBit());
	//	DRW_DBG(buf->getBit());
	//	DRW_DBG(buf->getBit());
	//}
	textPoint.x = buf->getRawDouble();
	textPoint.y = buf->getRawDouble();
	textPoint.z = buf->getBitDouble();
	DRW_DBG("\ntextPoint: ");
	DRW_DBGPT(textPoint.x, textPoint.y, textPoint.z);
	type = buf->getRawChar8();
	DRW_DBG("\ntype (70) read: ");
	DRW_DBG(type);
	type = (type & 1) ? type & 0x7F : type | 0x80; //set bit 7
	type = (type & 2) ? type | 0x20 : type & 0xDF; //set bit 5
	DRW_DBG(" type (70) set: ");
	DRW_DBG(type);
	//clear last 3 bits to set integer dim type
	type &= 0xF8;
	userText = sBuf->getVariableText(version, false);
	DRW_DBG("\n forced dim text: ");
	DRW_DBG(userText.c_str());
	textRot = buf->getBitDouble();
	horizDir = buf->getBitDouble();
	DRW_Coord insScale = buf->get3BitDouble();
	DRW_DBG("\ninsScale: ");
	DRW_DBGPT(insScale.x, insScale.y, insScale.z);
	double insRot = buf->getBitDouble(); //RLZ: unknown, investigate
	DRW_DBG(" insRot: ");
	DRW_DBG(insRot);
	if (version >= DRW::R2000)
	{ //2000+
		attchPt = buf->getBitShort();
		lineSpaceStyle = buf->getBitShort();
		lineSpaceFactor = buf->getBitDouble();
		actualMeasurement = buf->getBitDouble();
		DRW_DBG("\n  actualMeasurement_code42: ");
		DRW_DBG(actualMeasurement);
		if (version >= DRW::R2007)
		{
			//2007+
			bool unk = buf->getBit();
			bool flip1 = buf->getBit();
			bool flip2 = buf->getBit();
			DRW_DBG("\n2007, unk, flip1, flip2: ");
			DRW_DBG(unk);
			DRW_DBG(flip1);
			DRW_DBG(flip2);
		}
	}
	clonePoint.x = buf->getRawDouble();
	clonePoint.y = buf->getRawDouble();
	DRW_DBG("\nclonePoint: ");
	DRW_DBGPT(clonePoint.x, clonePoint.y, clonePoint.z);

	return buf->isGood();
}

bool DRW_DimAligned::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	//2007+
	if (version >= DRW::R2007)
		sBuf = &sBuff; //separate buffer for strings

	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	if (dwgType == DRW::DIMENSION_LINEAR)
		DRW_DBG("\n***************************** parsing dim linear *********************************************\n");
	else
		DRW_DBG("\n***************************** parsing dim aligned *********************************************\n");
	DRW_Coord pt = buf->get3BitDouble();
	set13pt(pt); //def1
	DRW_DBG("def1: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set14pt(pt);
	DRW_DBG("\ndef2: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set10pt(pt);
	DRW_DBG("\ndefPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	setOblique(buf->getBitDouble());
	if (dwgType == DRW::DIMENSION_LINEAR)
		setAngle(buf->getBitDouble());
	else
		type |= 1;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_DimRadius::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing dim radial *********************************************\n");
	DRW_Coord pt = buf->get3BitDouble();
	set10pt(pt); //code 10
	DRW_DBG("defPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	setPt15(pt); //center pt  code 15
	DRW_DBG("\ncenter point: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	setRa40(buf->getBitDouble()); //leader length code 40
	DRW_DBG("\nleader length: ");
	DRW_DBG(getRa40());
	type |= 4;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_DimDiameter::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing dim diametric *********************************************\n");
	DRW_Coord pt = buf->get3BitDouble();
	setPt15(pt); //center pt  code 15
	DRW_DBG("center point: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set10pt(pt); //code 10
	DRW_DBG("\ndefPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	setRa40(buf->getBitDouble()); //leader length code 40
	DRW_DBG("\nleader length: ");
	DRW_DBG(getRa40());
	type |= 3;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_DimAngular::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing dim angular *********************************************\n");
	DRW_Coord pt;
	pt.x = buf->getRawDouble();
	pt.y = buf->getRawDouble();
	setPt16(pt); //code 16
	DRW_DBG("arc Point: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set13pt(pt); //def1  code 13
	DRW_DBG("\ndef1: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set14pt(pt); //def2  code 14
	DRW_DBG("\ndef2: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	setPt15(pt); //center pt  code 15
	DRW_DBG("\ncenter point: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set10pt(pt); //code 10
	DRW_DBG("\ndefPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	type |= 0x02;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_DimAngular3p::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing dim angular3p *********************************************\n");
	DRW_Coord pt = buf->get3BitDouble();
	set10pt(pt); //code 10
	DRW_DBG("defPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set13pt(pt); //def1  code 13
	DRW_DBG("\ndef1: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set14pt(pt); //def2  code 14
	DRW_DBG("\ndef2: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	setPt15(pt); //center pt  code 15
	DRW_DBG("\ncenter point: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	type |= 0x05;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);
	DRW_DBG("\n");

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

bool DRW_DimOrdinate::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	ret = DRW_Dimension::parseDwg(version, buf, sBuf);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing dim ordinate *********************************************\n");
	DRW_Coord pt = buf->get3BitDouble();
	set10pt(pt);
	DRW_DBG("defPoint: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set13pt(pt); //def1
	DRW_DBG("\ndef1: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	pt = buf->get3BitDouble();
	set14pt(pt);
	DRW_DBG("\ndef2: ");
	DRW_DBGPT(pt.x, pt.y, pt.z);
	duint8 type2 = buf->getRawChar8(); //RLZ: correct this
	DRW_DBG("type2 (70) read: ");
	DRW_DBG(type2);
	type = (type2 & 1) ? type | 0x80 : type & 0xBF; //set bit 6
	DRW_DBG(" type (70) set: ");
	DRW_DBG(type);
	type |= 6;
	DRW_DBG("\n  type (70) final: ");
	DRW_DBG(type);

	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	if (!ret)
		return ret;
	dimStyleH = buf->getHandle();
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	blockH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("anon block Handle: ");
	DRW_DBGHL(blockH.code, blockH.size, blockH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Leader::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 3:
			style = reader->getUtf8String();
			break;
		case 71:
			arrow = reader->getInt32();
			break;
		case 72:
			leadertype = reader->getInt32();
			break;
		case 73:
			flag = reader->getInt32();
			break;
		case 74:
			hookline = reader->getInt32();
			break;
		case 75:
			hookflag = reader->getInt32();
			break;
		case 76:
			vertnum = reader->getInt32();
			break;
		case 77:
			coloruse = reader->getInt32();
			break;
		case 40:
			textheight = reader->getDouble();
			break;
		case 41:
			textwidth = reader->getDouble();
			break;
		case 10:
		{
			vertexpoint = new DRW_Coord();
			vertexlist.push_back(vertexpoint);
			vertexpoint->x = reader->getDouble();
			break;
		}
		case 20:
			if (vertexpoint != NULL)
				vertexpoint->y = reader->getDouble();
			break;
		case 30:
			if (vertexpoint != NULL)
				vertexpoint->z = reader->getDouble();
			break;
		case 340:
			annotHandle = reader->getHandleString();
			break;
		case 210:
			extrusionPoint.x = reader->getDouble();
			break;
		case 220:
			extrusionPoint.y = reader->getDouble();
			break;
		case 230:
			extrusionPoint.z = reader->getDouble();
			break;
		case 211:
			horizdir.x = reader->getDouble();
			break;
		case 221:
			horizdir.y = reader->getDouble();
			break;
		case 231:
			horizdir.z = reader->getDouble();
			break;
		case 212:
			offsetblock.x = reader->getDouble();
			break;
		case 222:
			offsetblock.y = reader->getDouble();
			break;
		case 232:
			offsetblock.z = reader->getDouble();
			break;
		case 213:
			offsettext.x = reader->getDouble();
			break;
		case 223:
			offsettext.y = reader->getDouble();
			break;
		case 233:
			offsettext.z = reader->getDouble();
			break;
		default:
			DRW_Entity::parseCode(code, reader);
			break;
	}
}

bool DRW_Leader::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing leader *********************************************\n");
	DRW_DBG("unknown bit ");
	DRW_DBG(buf->getBit());
	DRW_DBG(" annot type ");
	DRW_DBG(buf->getBitShort());
	DRW_DBG(" Path type ");
	DRW_DBG(buf->getBitShort());
	dint32 nPt = buf->getBitLong();
	DRW_DBG(" Num pts ");
	DRW_DBG(nPt);

	// add vertexs
	for (int i = 0; i < nPt; i++)
	{
		DRW_Coord* vertex = new DRW_Coord(buf->get3BitDouble());
		vertexlist.push_back(vertex);
		DRW_DBG("\nvertex ");
		DRW_DBGPT(vertex->x, vertex->y, vertex->z);
	}
	DRW_Coord Endptproj = buf->get3BitDouble();
	DRW_DBG("\nEndptproj ");
	DRW_DBGPT(Endptproj.x, Endptproj.y, Endptproj.z);
	extrusionPoint = buf->getExtrusion(version > DRW::R14);
	DRW_DBG("\nextrusionPoint ");
	DRW_DBGPT(extrusionPoint.x, extrusionPoint.y, extrusionPoint.z);
	if (version > DRW::R14)
	{ //2000+
		DRW_DBG("\nFive unknown bits: ");
		DRW_DBG(buf->getBit());
		DRW_DBG(buf->getBit());
		DRW_DBG(buf->getBit());
		DRW_DBG(buf->getBit());
		DRW_DBG(buf->getBit());
	}
	horizdir = buf->get3BitDouble();
	DRW_DBG("\nhorizdir ");
	DRW_DBGPT(horizdir.x, horizdir.y, horizdir.z);
	offsetblock = buf->get3BitDouble();
	DRW_DBG("\noffsetblock ");
	DRW_DBGPT(offsetblock.x, offsetblock.y, offsetblock.z);
	if (version > DRW::R13)
	{ //R14+
		DRW_Coord unk = buf->get3BitDouble();
		DRW_DBG("\nunknown ");
		DRW_DBGPT(unk.x, unk.y, unk.z);
	}
	if (version < DRW::R2000)
	{ //R14 -
		DRW_DBG("\ndimgap ");
		DRW_DBG(buf->getBitDouble());
	}
	if (version < DRW::R2010)
	{ //2010-
		textheight = buf->getBitDouble();
		textwidth = buf->getBitDouble();
		DRW_DBG("\ntextheight ");
		DRW_DBG(textheight);
		DRW_DBG(" textwidth ");
		DRW_DBG(textwidth);
	}
	hookline = buf->getBit();
	arrow = buf->getBit();
	DRW_DBG(" hookline ");
	DRW_DBG(hookline);
	DRW_DBG(" arrow flag ");
	DRW_DBG(arrow);

	if (version < DRW::R2000)
	{ //R14 -
		DRW_DBG("\nArrow head type ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("dimasz ");
		DRW_DBG(buf->getBitDouble());
		DRW_DBG("\nunk bit ");
		DRW_DBG(buf->getBit());
		DRW_DBG(" unk bit ");
		DRW_DBG(buf->getBit());
		DRW_DBG(" unk short ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG(" byBlock color ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG(" unk bit ");
		DRW_DBG(buf->getBit());
		DRW_DBG(" unk bit ");
		DRW_DBG(buf->getBit());
	}
	else
	{ //R2000+
		DRW_DBG("\nunk short ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG(" unk bit ");
		DRW_DBG(buf->getBit());
		DRW_DBG(" unk bit ");
		DRW_DBG(buf->getBit());
	}
	DRW_DBG("\n");
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);
	if (!ret)
		return ret;
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	AnnotH = buf->getHandle();
	annotHandle = AnnotH.ref;
	DRW_DBG("annot block Handle: ");
	DRW_DBGHL(AnnotH.code, AnnotH.size, dimStyleH.ref);
	DRW_DBG("\n");
	dimStyleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("dim style Handle: ");
	DRW_DBGHL(dimStyleH.code, dimStyleH.size, dimStyleH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");
	//    RS crc;   //RS */
	return buf->isGood();
}

void DRW_Viewport::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 40:
			pswidth = reader->getDouble();
			break;
		case 41:
			psheight = reader->getDouble();
			break;
		case 68:
			vpstatus = reader->getInt32();
			break;
		case 69:
			vpID = reader->getInt32();
			break;
		case 12:
		{
			centerPX = reader->getDouble();
			break;
		}
		case 22:
			centerPY = reader->getDouble();
			break;
		default:
			DRW_Point::parseCode(code, reader);
			break;
	}
}
//ex 22 dec 34
bool DRW_Viewport::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer sBuff = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{                  //2007+
		sBuf = &sBuff; //separate buffer for strings
	}
	bool ret = DRW_Entity::parseDwg(version, buf, sBuf, bs);
	if (!ret)
		return ret;
	DRW_DBG("\n***************************** parsing viewport *****************************************\n");
	basePoint.x = buf->getBitDouble();
	basePoint.y = buf->getBitDouble();
	basePoint.z = buf->getBitDouble();
	DRW_DBG("center ");
	DRW_DBGPT(basePoint.x, basePoint.y, basePoint.z);
	pswidth = buf->getBitDouble();
	psheight = buf->getBitDouble();
	DRW_DBG("\nWidth: ");
	DRW_DBG(pswidth);
	DRW_DBG(", Height: ");
	DRW_DBG(psheight);
	DRW_DBG("\n");
	//RLZ TODO: complete in dxf
	if (version > DRW::R14)
	{ //2000+
		viewTarget.x = buf->getBitDouble();
		viewTarget.y = buf->getBitDouble();
		viewTarget.z = buf->getBitDouble();
		DRW_DBG("view Target ");
		DRW_DBGPT(viewTarget.x, viewTarget.y, viewTarget.z);
		viewDir.x = buf->getBitDouble();
		viewDir.y = buf->getBitDouble();
		viewDir.z = buf->getBitDouble();
		DRW_DBG("\nview direction ");
		DRW_DBGPT(viewDir.x, viewDir.y, viewDir.z);
		twistAngle = buf->getBitDouble();
		DRW_DBG("\nView twist Angle: ");
		DRW_DBG(twistAngle);
		viewHeight = buf->getBitDouble();
		DRW_DBG("\nview Height: ");
		DRW_DBG(viewHeight);
		viewLength = buf->getBitDouble();
		DRW_DBG(" Lens Length: ");
		DRW_DBG(viewLength);
		frontClip = buf->getBitDouble();
		DRW_DBG("\nfront Clip Z: ");
		DRW_DBG(frontClip);
		backClip = buf->getBitDouble();
		DRW_DBG(" back Clip Z: ");
		DRW_DBG(backClip);
		snapAngle = buf->getBitDouble();
		DRW_DBG("\n snap Angle: ");
		DRW_DBG(snapAngle);
		centerPX = buf->getRawDouble();
		centerPY = buf->getRawDouble();
		DRW_DBG("\nview center X: ");
		DRW_DBG(centerPX);
		DRW_DBG(", Y: ");
		DRW_DBG(centerPX);
		snapPX = buf->getRawDouble();
		snapPY = buf->getRawDouble();
		DRW_DBG("\nSnap base point X: ");
		DRW_DBG(snapPX);
		DRW_DBG(", Y: ");
		DRW_DBG(snapPY);
		snapSpPX = buf->getRawDouble();
		snapSpPY = buf->getRawDouble();
		DRW_DBG("\nSnap spacing X: ");
		DRW_DBG(snapSpPX);
		DRW_DBG(", Y: ");
		DRW_DBG(snapSpPY);
		//RLZ: need to complete
		DRW_DBG("\nGrid spacing X: ");
		DRW_DBG(buf->getRawDouble());
		DRW_DBG(", Y: ");
		DRW_DBG(buf->getRawDouble());
		DRW_DBG("\n");
		DRW_DBG("Circle zoom?: ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("\n");
	}
	if (version > DRW::R2004)
	{ //2007+
		DRW_DBG("Grid major?: ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("\n");
	}
	if (version > DRW::R14)
	{ //2000+
		frozenLyCount = buf->getBitLong();
		DRW_DBG("Frozen Layer count?: ");
		DRW_DBG(frozenLyCount);
		DRW_DBG("\n");
		DRW_DBG("Status Flags?: ");
		DRW_DBG(buf->getBitLong());
		DRW_DBG("\n");
		//RLZ: Warning needed separate string bufer
		DRW_DBG("Style sheet?: ");
		DRW_DBG(sBuf->getVariableText(version, false));
		DRW_DBG("\n");
		DRW_DBG("Render mode?: ");
		DRW_DBG(buf->getRawChar8());
		DRW_DBG("\n");
		DRW_DBG("UCS OMore...: ");
		DRW_DBG(buf->getBit());
		DRW_DBG("\n");
		DRW_DBG("UCS VMore...: ");
		DRW_DBG(buf->getBit());
		DRW_DBG("\n");
		DRW_DBG("UCS OMore...: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\n");
		DRW_DBG("ucs XAMore...: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\n");
		DRW_DBG("UCS YMore....: ");
		DRW_DBGPT(buf->getBitDouble(), buf->getBitDouble(), buf->getBitDouble());
		DRW_DBG("\n");
		DRW_DBG("UCS EMore...: ");
		DRW_DBG(buf->getBitDouble());
		DRW_DBG("\n");
		DRW_DBG("UCS OVMore...: ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("\n");
	}
	if (version > DRW::R2000)
	{ //2004+
		DRW_DBG("ShadePlot Mode...: ");
		DRW_DBG(buf->getBitShort());
		DRW_DBG("\n");
	}
	if (version > DRW::R2004)
	{ //2007+
		DRW_DBG("Use def Ligth...: ");
		DRW_DBG(buf->getBit());
		DRW_DBG("\n");
		DRW_DBG("Def ligth tipe?: ");
		DRW_DBG(buf->getRawChar8());
		DRW_DBG("\n");
		DRW_DBG("Brightness: ");
		DRW_DBG(buf->getBitDouble());
		DRW_DBG("\n");
		DRW_DBG("Contrast: ");
		DRW_DBG(buf->getBitDouble());
		DRW_DBG("\n");
		//        DRW_DBG("Ambient Cmc or Enc: "); DRW_DBG(buf->getCmColor(version)); DRW_DBG("\n");
		DRW_DBG("Ambient (Cmc or Enc?), Enc: ");
		// todo fix entity color ?
		DRW_DBG(buf->getEnColor(version));
		DRW_DBG("\n");
	}
	ret = DRW_Entity::parseDwgCommonEntityHandle(version, buf);

	dwgHandle someHdl;
	if (version < DRW::R2000)
	{ //R13 & R14 only
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		someHdl = buf->getHandle();
		DRW_DBG("ViewPort ent header: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
	}
	if (version > DRW::R14)
	{ //2000+
		for (duint8 i = 0; i < frozenLyCount; ++i)
		{
			someHdl = buf->getHandle();
			DRW_DBG("Frozen layer handle ");
			DRW_DBG(i);
			DRW_DBG(": ");
			DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
			DRW_DBG("\n");
		}
		someHdl = buf->getHandle();
		DRW_DBG("Clip bpundary handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
		if (version == DRW::R2000)
		{ //2000 only
			someHdl = buf->getHandle();
			DRW_DBG("ViewPort ent header: ");
			DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
			DRW_DBG("\n");
		}
		someHdl = buf->getHandle();
		DRW_DBG("Named ucs handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		someHdl = buf->getHandle();
		DRW_DBG("base ucs handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
	}
	if (version > DRW::R2004)
	{ //2007+
		someHdl = buf->getHandle();
		DRW_DBG("background handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
		someHdl = buf->getHandle();
		DRW_DBG("visual style handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
		someHdl = buf->getHandle();
		DRW_DBG("shadeplot ID handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
		DRW_DBG("\n Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
		someHdl = buf->getHandle();
		DRW_DBG("SUN handle: ");
		DRW_DBGHL(someHdl.code, someHdl.size, someHdl.ref);
		DRW_DBG("\n");
	}
	DRW_DBG("\n Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	if (!ret)
		return ret;
	return buf->isGood();
}

void DRW_Attdef::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 3:
			prompt = reader->getUtf8String();
			break;
		default:
			DRW_Attrib::parseCode(code, reader);
			break;
	}
}

bool DRW_Attdef::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer attdefStrBuf = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{
		//2007+
		sBuf = &attdefStrBuf; //separate buffer for strings
	}
	bool ret = DRW_Attrib::parseDwgAndOutStrBuf(sBuf, version, buf, bs);
	if (!ret)
		return ret;

	DRW_DBG("--------------parse the attdef private ------------------");
	if (version >= DRW::R2010)
	{
		classVersion = buf->getRawChar8();
	}

	prompt = sBuf->getVariableText(version, false);

	PARSE_DWG_ENTITY_COMMON_HANDLE;
	styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
	DRW_DBG("text style Handle: ");
	DRW_DBGHL(styleH.code, styleH.size, styleH.ref);
	DRW_DBG("\n");
	return true;
}

void DRW_Attrib::parseCode(int code, dxfReader* reader)
{
	switch (code)
	{
		case 71:
			/*	attribType = static_cast<AttribType>(reader->getInt32());*/
			break;
		case 2:
			break;
		case 70:
			break;
		case 73:
			break;
		case 280:
			break;
		default:
			DRW_Text::parseCode(code, reader);
			break;
	}
}


bool DRW_Attrib::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	dwgBuffer attribStrBuf = *buf;
	dwgBuffer* sBuf = buf;
	if (version > DRW::R2004)
	{
		//2007+
		sBuf = &attribStrBuf; //separate buffer for strings
	}

	bool ret = DRW_Text::parseDwgAndOutStrBuf(sBuf, version, buf, bs);
	if (!ret)
		return ret;
	DRW_DBG("------------------------- parse the attrib private ------------------\n");

	//2010+
	if (version >= DRW::R2010)
		classVersion = buf->getRawChar8();
	//2018+
	if (version >= DRW::R2018)
	{
		attribType = static_cast<AttribType>(buf->getRawChar8());
		if (attribType == AttribType::MultiLine)
		{
			// TODO MTEXT fields objects
			dint32 annotativeDataSize = buf->getBitShort();
			if (annotativeDataSize > 0)
			{
				std::vector<duint8> bytes(annotativeDataSize);
				buf->getBytes(bytes.data(), annotativeDataSize);
				dwgHandle registerHandle = buf->getHandle();
				duint16 unkown = buf->getBitShort();//! value 0
			}

			tagString = buf->getVariableText(version, false);
			duint16 unkown = buf->getBitShort();//! value  0
			attribFlags = static_cast<AttribFlags>(buf->getRawChar8());
			lockPostion = buf->getBit();
		}
	}

	// ` common
	if (attribType == AttribType::SingleLine)
	{
		tag = sBuf->getVariableText(version, false);//A
		fieldLength = buf->getBitShort();
		bitPairCoded = buf->getRawChar8();
		if (version >= DRW::R2007)
			lockPostionFlag = buf->getBit();
	}

	IS_ENTITY(DRW::ATTRIB)
	{
		PARSE_DWG_ENTITY_COMMON_HANDLE;
		styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
		DRW_DBG("text style Handle: ");
		DRW_DBGHL(styleH.code, styleH.size, styleH.ref);
		DRW_DBG("\n");
	}

	return buf->isGood();
}

bool DRW_Attrib::parseDwgAndOutStrBuf(dwgBuffer* outStrBuf, DRW::Version version, dwgBuffer* buf, duint32 bs /*= 0*/)
{
	dwgBuffer* sBuf = outStrBuf;
	bool ret = DRW_Text::parseDwgAndOutStrBuf(sBuf, version, buf, bs);
	if (!ret)
		return ret;
	DRW_DBG("------------------------- parse the attrib private ------------------\n");

	//2010+
	if (version >= DRW::R2010)
		classVersion = buf->getRawChar8();
	//2018+
	if (version >= DRW::R2018)
	{
		attribType = static_cast<AttribType>(buf->getRawChar8());
		if (attribType == AttribType::MultiLine)
		{
			// TODO MTEXT fields objects
			dint32 annotativeDataSize = buf->getBitShort();
			if (annotativeDataSize > 0)
			{
				std::vector<duint8> bytes(annotativeDataSize);
				buf->getBytes(bytes.data(), annotativeDataSize);
				dwgHandle registerHandle = buf->getHandle();
				duint16 unkown = buf->getBitShort();//! value 0
			}

			tagString = buf->getVariableText(version, false);
			duint16 unkown = buf->getBitShort();//! value  0
			attribFlags = static_cast<AttribFlags>(buf->getRawChar8());
			lockPostion = buf->getBit();
		}
	}

	// ` common
	if (attribType == AttribType::SingleLine)
	{
		tag = sBuf->getVariableText(version, false);//A
		fieldLength = buf->getBitShort();
		bitPairCoded = buf->getRawChar8();
		if (version >= DRW::R2007)
			lockPostionFlag = buf->getBit();
	}

	IS_ENTITY(DRW::ATTRIB)
	{
		PARSE_DWG_ENTITY_COMMON_HANDLE;
		styleH = buf->getHandle(); /* H 7 STYLE (hard pointer) */
		DRW_DBG("text style Handle: ");
		DRW_DBGHL(styleH.code, styleH.size, styleH.ref);
		DRW_DBG("\n");
	}

	return buf->isGood();
}
