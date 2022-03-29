/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 Jos???? F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "dwgreader.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "drw_dbg.h"
#include "drw_textcodec.h"

#define FREE_MAP(a)              \
    for (auto &item : a)         \
	{                            \
            item.second.reset(); \
    }

DwgReader::~DwgReader()
{
	FREE_MAP(this->layerMap);
	FREE_MAP(this->lineTypeMap);
	FREE_MAP(this->blockMap);
	FREE_MAP(this->styleMap);
	FREE_MAP(this->dimstyleMap);
	FREE_MAP(this->vportMap);
	FREE_MAP(this->classesmap);
	FREE_MAP(this->blockHeaderMap);
	FREE_MAP(this->appIdMap);
	delete fileBuf;
}

void DwgReader::parseEntityConfig(DRW_Entity* e)
{
	if (e)
	{
		installEntityLineTypeConf(e);
		installEntityLayerConf(e);
	}
}

void DwgReader::installEntityLayerConf(DRW_Entity* e)
{
	const auto& layerIt = layerMap.find(e->getLayerHandleRef());
	if (layerIt != layerMap.end())
		e->layer = (layerIt->second)->entryName;
}

void DwgReader::installEntityLineTypeConf(DRW_Entity* e)
{
	//find its linetype
	switch (e->getLineTypeFlag())
	{
		//ByLayer
		case 0:
		{
			const auto& layerIt = layerMap.find(e->getLayerHandleRef());
			if (layerIt != layerMap.end())
			{
				const auto& lineTypeIt = lineTypeMap.find(layerIt->second->lTypeHandleRef);
				e->lTypeHandleRef = lineTypeIt->first;
			}
			break;
		}
		// ByBlock
		case 1:
		{
			//todo find its block and assign its linetype handle ref
			// set default linetype
			if (!e->hasOwner)
			{
				for (auto& mapIt : lineTypeMap)
				{
					if (mapIt.second->entryName == "Continuous")
					{
						e->lTypeHandleRef = mapIt.first;
					}
				}
			}
			break;
		}
		// Continues
		case 2:
		{
			for (auto& mapIt : lineTypeMap)
			{
				if (mapIt.second->entryName == "Continuous")
				{
					e->lTypeHandleRef = mapIt.first;
				}
			}
			break;
		}
		default:
		{
			const auto& lineTypeIt = lineTypeMap.find(e->getLineTypeHandleRef());
			if (lineTypeIt != lineTypeMap.end())
				e->lineType = lineTypeIt->second->entryName;
			break;
		}
	}
}

std::string DwgReader::findTableName(DRW::DwgType table, dint32 handle)
{
	switch (table)
	{
		case DRW::STYLE:
		{
			auto textStyleIt = styleMap.find(handle);
			if (textStyleIt != styleMap.end())
				return (textStyleIt->second)->entryName;
			break;
		}
		case DRW::DIMSTYLE:
		{
			auto dimStyleIt = dimstyleMap.find(handle);
			if (dimStyleIt != dimstyleMap.end())
				return (dimStyleIt->second)->entryName;
			break;
		}
		case DRW::BLOCK_HEADER:
		{
			auto blockHeaderIt = blockHeaderMap.find(handle);
			if (blockHeaderIt != blockHeaderMap.end())
				return (blockHeaderIt->second)->entryName;
			break;
		}
		case DRW::VPORT:
		{
			auto vp_it = vportMap.find(handle);
			if (vp_it != vportMap.end())
				return (vp_it->second)->entryName;
			break;
		}
		case DRW::LAYER:
		{
			auto layerIt = layerMap.find(handle);
			if (layerIt != layerMap.end())
			{
				return (layerIt->second)->entryName;
			}
			break;
		}
		case DRW::LTYPE:
		{
			auto lineTypeIt = lineTypeMap.find(handle);
			if (lineTypeIt != lineTypeMap.end())
				return (lineTypeIt->second)->entryName;
			break;
		}
		default:
			break;
	}
	return std::string("");
}

bool DwgReader::readDwgHeader(DRW_Header& hdr, dwgBuffer* buf, dwgBuffer* hBuf) const
{
	const bool ret = hdr.parseDwg(version, buf, hBuf, maintenanceVersion);
	return ret;
}


/*********** objects map ************************/
/** Note: object map are split in sections with max size 2035?
 *  head section are 2 bytes size + data bytes + 2 bytes crc
 *  size value are data bytes + 2 and to calculate crc are used
 *  2 bytes size + data bytes
 *  last section are 2 bytes size + 2 bytes crc (size value always 2)
**/
bool DwgReader::readDwgHandles(dwgBuffer* dbuf, duint32 offset, duint32 size)
{
	DRW_DBG("\ndwgReader::readDwgHandles\n");
	if (!dbuf->setPosition(offset))
		return false;

	duint32 maxPos = offset + size;
	DRW_DBG("\nSection HANDLES offset= ");
	DRW_DBG(offset);
	DRW_DBG("\nSection HANDLES size= ");
	DRW_DBG(size);
	DRW_DBG("\nSection HANDLES maxPos= ");
	DRW_DBG(maxPos);

	duint64 startPos = offset;
	duint16 sectionSize = 0;
	do
	{
		DRW_DBG("\nstart handles section buf->curPosition()= ");
		DRW_DBG(dbuf->getPosition()); DRW_DBG("\n");
		sectionSize = dbuf->getBERawShort16();
		DRW_DBG("object map section size= ");
		DRW_DBG(sectionSize); DRW_DBG("\n");
		if (sectionSize > 2040)
		{
			DRW_DBG("\nObject-map/handles page size greater than 2040!\n");
			return false;
		}
		dbuf->setPosition(startPos);
		std::vector<duint8> sectionVec;
		sectionVec.resize(sectionSize);
		dbuf->getBytes(sectionVec.data(), sectionSize);
		dwgBuffer buff(sectionVec.data(), sectionSize, &decoder);
		int lastHandle = 0;
		int lastLoc = 0;
		int offset = -1;
		buff.setPosition(2);
		//read data
		while (buff.getPosition() < sectionSize)
		{
			lastHandle += buff.getUModularChar();
			DRW_DBG("object map lastHandle= ");
			DRW_DBGH(lastHandle);
			offset = lastLoc;
			lastLoc += buff.getModularChar();
			DRW_DBG(" lastLoc= ");
			DRW_DBG(lastLoc);
			DRW_DBG("\n");
			//? fix this when diffOffset is 1:ÖÐÔ­Â·×ÝÆÊÍ¼
			if (lastLoc - offset == 1)
			{
				DRW_DBG("\ninvalid offset location offset\n");
				break;
			}
			objectMap[lastHandle] = ObjHandle(0, lastHandle, lastLoc);
		}
		//verify crc
		duint16 crcCalc = buff.crc8(0xc0c1, 0, sectionSize);
		//delete[] tmpByteStr;
		duint16 crcRead = dbuf->getBERawShort16();
		DRW_DBG("object map section crc8 read= ");
		DRW_DBG(crcRead);
		DRW_DBG("\nobject map section crc8 calculated= ");
		DRW_DBG(crcCalc);
		DRW_DBG("\nobject section buf->curPosition()= ");
		DRW_DBG(dbuf->getPosition());
		DRW_DBG("\n");
		startPos = dbuf->getPosition();
		if (maxPos <= dbuf->getPosition())
			break;
	} while (sectionSize > 2);

	/*for (const auto& mapItm : objectMap)
	{
		spdlog::get("all")->info("handle:{}", mapItm.second.handle);
	}*/

	const bool ret = dbuf->isGood();
	return ret;
}

/*********** objects ************************/
/**
 * Reads all the object referred in the object map section of the DWG file
 * (using their object file offsets)
 */
bool DwgReader::readDwgTables(DRW_Header& hdr, dwgBuffer* dbuf)
{
	DRW_DBG("\ndwgReader::readDwgTables start\n");
	bool ret = true;
	bool ret2 = true;
	ObjHandle objH;
	std::map<duint32, ObjHandle>::iterator mit;
	dint16 dwgType;
	duint32 bs = 0;  //bit size of handle stream 2010+
	duint8* tmpByteStr;

	//parse line types, start with line type Control
	mit = objectMap.find(hdr.linetypeCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: LineType control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing LineType control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_ObjControl ltControl;
		dbuf->setPosition(objH.loc);
		int csize = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[csize];
		dbuf->getBytes(tmpByteStr, csize);
		dwgBuffer cbuff(tmpByteStr, csize, &decoder);
		//verify if object are correct
		dwgType = cbuff.getObjType(version);
		if (dwgType != 0x38)
		{
			DRW_DBG("\nWARNING: Not LineType control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x38\n");
			ret = false;
		}
		else
		{  //reset position
			cbuff.resetPosition();
			ret2 = ltControl.parseDwg(version, &cbuff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse  LineType control error,handle at {}", objH.handle);
			}
			if (ret)
				ret = ret2;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : ltControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: LineType not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				objectMap.erase(mit);
				DRW_DBG("\nLineType Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" loc.: ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_LType* lt = new DRW_LType();
				std::shared_ptr<DRW_LType> lineType = std::make_shared<DRW_LType>();
				dbuf->setPosition(objH.loc);
				int lsize = dbuf->getModularShort();
				DRW_DBG("LineType size in bytes= ");
				DRW_DBG(lsize);
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[lsize];
				dbuf->getBytes(tmpByteStr, lsize);
				dwgBuffer lbuff(tmpByteStr, lsize, &decoder);
				ret2 = lineType->parseDwg(version, &lbuff, bs);
				if (!ret2)
				{
					spdlog::get(pDwgR->getFilePath())->error("parse DRW_LType error,handle at {}", objH.handle);
				}
				lineTypeMap[lineType->handle] = lineType;
				if (ret)
					ret = ret2;
				delete[] tmpByteStr;
			}
		}
	}

	//parse layers, start with layer Control
	mit = objectMap.find(hdr.layerCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: Layer control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing Layer control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_ObjControl layControl;
		dbuf->setPosition(objH.loc);
		int size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		//verify if object are correct
		dwgType = buff.getObjType(version);
		if (dwgType != 0x32)
		{
			DRW_DBG("\nWARNING: Not Layer control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x32\n");
			ret = false;
		}
		else
		{  //reset position
			buff.resetPosition();
			ret2 = layControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse Layer control error,handle at {}", objH.handle);
			}
			if (ret)
				ret = ret2;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : layControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: Layer not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				objectMap.erase(mit);
				DRW_DBG("Layer Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_Layer* la = new DRW_Layer();
				std::shared_ptr<DRW_Layer> layer = std::make_shared<DRW_Layer>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				ret2 = layer->parseDwg(version, &buff, bs);
				if (!ret2)
				{
					spdlog::get(pDwgR->getFilePath())->error("parse DRW_Layer error,handle at {}", objH.handle);
				}
				layerMap[layer->handle] = layer;
				if (ret)
					ret = ret2;
				delete[] tmpByteStr;
			}
		}
	}

	//set linetype for every layer
	for (auto& layerItm : layerMap)
	{
		std::shared_ptr<DRW_Layer> layer = layerItm.second;
		duint32 ref = layer->lTypeHandleRef;
		auto lineTypeMapIt = lineTypeMap.find(ref);
		if (lineTypeMapIt != lineTypeMap.end())
		{
			layer->lineType = lineTypeMapIt->second->entryName;// redundant but necessary
		}
	}

	//parse text styles, start with style Control
	mit = objectMap.find(hdr.styleCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: Style control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing Style control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_ObjControl styControl;
		dbuf->setPosition(objH.loc);
		int size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		//verify if object are correct
		dwgType = buff.getObjType(version);
		if (dwgType != 0x34)
		{
			DRW_DBG("\nWARNING: Not Text Style control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x34\n");
			ret = false;
		}
		else
		{
			//reset position
			buff.resetPosition();
			ret2 = styControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse Text Style control error,handle at {}", objH.handle);
			}
			if (ret)
				ret = ret2;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : styControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: Style not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				objectMap.erase(mit);
				DRW_DBG("Style Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_Textstyle* sty = new DRW_Textstyle();
				auto textStyle = std::make_shared<DRW_Textstyle>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				ret2 = textStyle->parseDwg(version, &buff, bs);
				if (!ret2)
				{
					spdlog::get(pDwgR->getFilePath())->error("parse text style error,handle at {}", objH.handle);
				}
				styleMap[textStyle->handle] = textStyle;
				if (ret)
					ret = ret2;
				delete[] tmpByteStr;
			}
		}
	}

	//parse dim styles, start with dimstyle Control
	mit = objectMap.find(hdr.dimstyleCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: Dimension Style control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing Dimension Style control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_ObjControl dimstyControl;
		dbuf->setPosition(objH.loc);
		duint32 size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		//verify if object are correct
		dwgType = buff.getObjType(version);
		if (dwgType != 0x44)
		{
			DRW_DBG("\nWARNING: Not Dim Style control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x44\n");
			ret = false;
		}
		else
		{  //reset position
			buff.resetPosition();
			ret2 = dimstyControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse dimension style control error,handle at {}", objH.handle);
			}
			if (ret)
				ret = ret2;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : dimstyControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: Dimension Style not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				objectMap.erase(mit);
				DRW_DBG("Dimstyle Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_Dimstyle* sty = new DRW_Dimstyle();
				auto dimStyle = std::make_shared<DRW_Dimstyle>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				ret2 = dimStyle->parseDwg(version, &buff, bs);
				if (!ret2)
				{
					spdlog::get(pDwgR->getFilePath())->error("parse dimension style error,handle at {}", objH.handle);
				}
				dimstyleMap[dimStyle->handle] = dimStyle;
				if (ret)
					ret = ret2;
				delete[] tmpByteStr;
			}
		}
	}

	//parse vports, start with vports Control
	mit = objectMap.find(hdr.vportCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: vports control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing vports control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_ObjControl vportControl;
		dbuf->setPosition(objH.loc);
		int size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		// verify if object are correct
		dwgType = buff.getObjType(version);
		if (dwgType != 0x40)
		{
			DRW_DBG("\nWARNING: Not VPorts control object, found dwgType: ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x40\n");
			ret = false;
		}
		else
		{  //reset position
			buff.resetPosition();
			ret2 = vportControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse vport control error,handle at {}", objH.handle);
			}
			if (ret)
				ret = ret2;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : vportControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: vport not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				objectMap.erase(mit);
				DRW_DBG("Vport Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_Vport* vp = new DRW_Vport();
				auto vport = std::make_shared<DRW_Vport>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				ret2 = vport->parseDwg(version, &buff, bs);
				if (!ret2)
				{
					spdlog::get(pDwgR->getFilePath())->error("parse vport error,handle at {}", objH.handle);
				}
				vportMap[vport->handle] = vport;
				if (ret) ret = ret2;
				delete[] tmpByteStr;
			}
		}
	}


	mit = objectMap.find(hdr.blockCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: Block_record control not found\n");
		ret = false;
	}
	else
	{
		objH = mit->second;
		DRW_ObjControl blockControl;
		dbuf->setPosition(objH.loc);
		int csize = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[csize];
		dbuf->getBytes(tmpByteStr, csize);
		dwgBuffer buff(tmpByteStr, csize, &decoder);
		dwgType = buff.getObjType(version);
		//verify block control dwg type
		if (dwgType == 0x30)
		{
			objectMap.erase(mit);
			//reset position
			buff.resetPosition();
			ret2 = blockControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse block control error,handle at {}", objH.handle);
			}
			if (ret) ret = ret2;
		}
		else
		{
			DRW_DBG("\nWARNING: Not block control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x30\n");
			ret = false;
		}
		delete[] tmpByteStr;

		for (const auto& it : blockControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				spdlog::get(pDwgR->getFilePath())->info("invalid handle,not found handle {} in handle list", it);
				/*ret = false;*/
			}
			else
			{
				objH = mit->second;
				auto blockHeader = std::make_shared<DRW_Block_Header>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				dwgType = buff.getObjType(version);
				// verify block header dwg type
				if (dwgType == 0x31)
				{
					objectMap.erase(mit);
					buff.resetPosition();
					ret2 = blockHeader->parseDwg(version, &buff, bs);
					if (!ret2)
						spdlog::get(pDwgR->getFilePath())->error("parse block header error,handle at {}", objH.handle);
					blockHeaderMap[blockHeader->handle] = blockHeader;
					if (ret) ret = ret2;
				}
				delete[] tmpByteStr;
			}
		}
	}


	//parse appId , start with appId Control
	mit = objectMap.find(hdr.appidCtrl);
	if (mit == objectMap.end())
	{
		DRW_DBG("\nWARNING: AppId control not found\n");
		ret = false;
	}
	else
	{
		DRW_DBG("\n**********Parsing AppId control*******\n");
		objH = mit->second;
		objectMap.erase(mit);
		DRW_DBG("AppId Control Obj Handle= ");
		DRW_DBGH(objH.handle);
		DRW_DBG(" ");
		DRW_DBG(objH.loc);
		DRW_DBG("\n");
		DRW_ObjControl appIdControl;
		dbuf->setPosition(objH.loc);
		int size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		// verify if object are correct
		dwgType = buff.getObjType(version);
		if (dwgType == 0x42)
		{
			// reset position
			buff.resetPosition();
			ret2 = appIdControl.parseDwg(version, &buff, bs);
			if (!ret2)
			{
				spdlog::get(pDwgR->getFilePath())->error("parse AppId Control error,handle at {}", objH.handle);
			}
			if (ret) ret = ret2;
		}
		else
		{
			DRW_DBG("\nWARNING: Not AppId control object, found dwgType ");
			DRW_DBG(dwgType);
			DRW_DBG(" instead 0x42\n");
			ret = false;
		}
		delete[] tmpByteStr;
		for (unsigned int& it : appIdControl.handleList)
		{
			mit = objectMap.find(it);
			if (mit == objectMap.end())
			{
				DRW_DBG("\nWARNING: AppId not found\n");
				ret = false;
			}
			else
			{
				objH = mit->second;
				DRW_DBG("AppId Handle= ");
				DRW_DBGH(objH.handle);
				DRW_DBG(" ");
				DRW_DBG(objH.loc);
				DRW_DBG("\n");
				//DRW_AppId* ai = new DRW_AppId();
				auto appId = std::make_shared<DRW_AppId>();
				dbuf->setPosition(objH.loc);
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)  //2010+
					bs = dbuf->getUModularChar();
				else
					bs = 0;
				tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				dwgType = buff.getObjType(version);
				// verify appid dwg type
				if (dwgType == 0x43)
				{
					objectMap.erase(mit);
					buff.resetPosition();
					ret2 = appId->parseDwg(version, &buff, bs);
					if (!ret2)
					{
						spdlog::get(pDwgR->getFilePath())->error("parse DRW_AppId error,handle at {}", objH.handle);
					}
					appIdMap[appId->handle] = appId;
					if (ret) ret = ret2;
				}
				delete[] tmpByteStr;
			}
		}
	}



	//ucs

	return ret;
}

bool DwgReader::readDwgBlocks(DRW_Interface& intfa, dwgBuffer* dbuf)
{
	bool ret = true;
	bool ret2 = true;
	duint32 bs = 0;
	duint8* tmpByteStr = nullptr;
	std::map<duint32, ObjHandle>::iterator mapIt{};

	// parse block headers
	for (auto it = blockHeaderMap.begin(); it != blockHeaderMap.end(); ++it)
	{
		auto& blockHeader = it->second;
#pragma region parse first block
		mapIt = objectMap.find(blockHeader->block);
		if (mapIt == objectMap.end())
		{
			/*ret = false;*/
			DRW_DBG("\nnot found first block handle of block header in global handle scope\n");
			continue;
		}
		ObjHandle objH = mapIt->second;
		if (!(dbuf->setPosition(objH.loc)))
		{
			ret = false;
			continue;
		}
		int size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff(tmpByteStr, size, &decoder);
		auto dwgType = buff.getObjType(version);
		if (dwgType != 4)
		{
			delete[] tmpByteStr;
			continue;
		}
		buff.resetPosition();
		DRW_Block firstBlk;
		ret2 = firstBlk.parseDwg(version, &buff, bs);
		objectMap.erase(mapIt);
		if (!ret2)
		{
			spdlog::get(pDwgR->getFilePath())->info("parse first block in block header error,handle at {},dwgtype :{}", objH.handle, dwgType);
		}
		delete[] tmpByteStr;
		ret = ret && ret2;
		parseEntityConfig(&firstBlk);
		// complete block entity with block header data
		firstBlk.basePoint = blockHeader->basePoint;
		firstBlk.flags = blockHeader->flags;
		intfa.addBlock(firstBlk);
		//and update block header name
		blockHeader->entryName = firstBlk.name;

#pragma endregion
		/**read & send block entities**/
		// in dwg code 330 are not set like dxf in ModelSpace & PaperSpace, set it (RLZ: only tested in 2000)
		if (firstBlk.parentHandle == DRW::NoHandle/*false*/)
		{
			// in dwg code 330 are not set like dxf in ModelSpace & PaperSpace, set it
			firstBlk.parentHandle = blockHeader->handle;
			//and do not send block entities like dxf
		}
		else
		{
			//firstBlk.parentHandle = pBlockHeader->handle;
			if (version < DRW::R2004)
			{
#pragma region parse the first to the last entities R2000 -
				//R2000-
				duint32 nextH = blockHeader->firstEH;
				while (nextH != 0)
				{
					mapIt = objectMap.find(nextH);
					if (mapIt == objectMap.end())
					{
						nextH = blockHeader->lastEH;  //end while if entity not found
						//ret = false; // invalid next handle
						continue;
					}
					else  //found entity reads it
					{
						objH = mapIt->second;
						objectMap.erase(mapIt);
						ret2 = readDwgEntity(dbuf, objH, intfa);
						if (!ret2)
						{
							spdlog::get(pDwgR->getFilePath())->info("parse entities in block header error,handle at {}", objH.handle);
						}
						ret = ret && ret2;
					}
					if (nextH == blockHeader->lastEH)
						nextH = 0;  //redundant, but prevent read errors
					else
						nextH = nextEntityLink;
				}
#pragma endregion
			}
			else
			{
#pragma region parse owned obj R2004 +
				if (!blockHeader->ownedObjHandles.empty())
				{
					for (auto it = blockHeader->ownedObjHandles.begin(); it != blockHeader->ownedObjHandles.end(); ++it)
					{
						duint32 nextH = *it;
						mapIt = objectMap.find(nextH);
						if (mapIt == objectMap.end())
						{
							spdlog::get(pDwgR->getFilePath())->info("\nWARNING: Entity of block header not found\n");
							//todo ret = false;
							continue;
						}
						else
						{
							//found entity,parse it
							objH = mapIt->second;
							objectMap.erase(mapIt);
							ret2 = readDwgEntity(dbuf, objH, intfa);
							if (!ret2)
							{
								spdlog::get(pDwgR->getFilePath())->info("parse owned objs in block header error,handle at {}", objH.handle);
							}
							ret = ret && ret2;
						}
					}
				}
#pragma endregion
			}
		}

#pragma region parse end block
		//end block entity, really needed to parse a dummy entity??
		mapIt = objectMap.find(blockHeader->endBlock);
		if (mapIt == objectMap.end())
		{
			/*ret = false;*/
			spdlog::get(pDwgR->getFilePath())->info("\nnot found last block handle of block header in global handle scope\n");
			continue;
		}
		objH = mapIt->second;
		objectMap.erase(mapIt);
		dbuf->setPosition(objH.loc);
		size = dbuf->getModularShort();
		if (version > DRW::R2007)  //2010+
			bs = dbuf->getUModularChar();
		else
			bs = 0;
		tmpByteStr = new duint8[size];
		dbuf->getBytes(tmpByteStr, size);
		dwgBuffer buff1(tmpByteStr, size, &decoder);
		DRW_Block endBlk;
		endBlk.isEnd = true;
		ret2 = endBlk.parseDwg(version, &buff1, bs);
		if (!ret2)
		{
			spdlog::get(pDwgR->getFilePath())->info("parse end block in block header error,handle at {}", objH.handle);
		}
		delete[] tmpByteStr;
		ret = ret && ret2;
		if (firstBlk.parentHandle == DRW::NoHandle)
			firstBlk.parentHandle = blockHeader->handle;
		parseEntityConfig(&endBlk);
		intfa.endBlock();
#pragma endregion
	}

	return ret;
}

bool DwgReader::readPlineVertex(DRW_Polyline& pline, dwgBuffer* dbuf)
{
	bool ret = true;
	bool ret2 = true;
	ObjHandle oc;
	duint32 bs = 0;
	std::map<duint32, ObjHandle>::iterator mit;

	if (version < DRW::R2004)
	{  //pre 2004
		duint32 nextH = pline.firstEH;
		while (nextH != 0)
		{
			mit = objectMap.find(nextH);
			if (mit == objectMap.end())
			{
				nextH = pline.lastEH;  //end while if entity not foud
				DRW_DBG("\nWARNING: pline vertex not found\n");
				ret = false;
				continue;
			}
			else
			{  //foud entity reads it
				oc = mit->second;
				objectMap.erase(mit);
				DRW_Vertex vt;
				dbuf->setPosition(oc.loc);
				//RLZ: verify if pos is ok
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)
				{  //2010+
					bs = dbuf->getUModularChar();
				}
				duint8* tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				dint16 dwgType = buff.getObjType(version);
				buff.resetPosition();
				DRW_DBG(" object type= ");
				DRW_DBG(dwgType);
				DRW_DBG("\n");
				ret2 = vt.parseDwg(version, &buff, bs, pline.basePoint.z);
				delete[] tmpByteStr;
				pline.addVertex(vt);
				nextEntityLink = vt.nextEntityLink;
				preEntityLink = vt.preEntityLink;
				ret = ret && ret2;
			}
			if (nextH == pline.lastEH)
				nextH = 0;  //redundant, but prevent read errors
			else
			{
				if (nextEntityLink != pline.handle)
					nextH = nextEntityLink;
				else
					nextH++;
			}
		}
	}
	else
	{  //2004+
		for (auto it = pline.ownedObjhandles.begin(); it != pline.ownedObjhandles.end(); ++it)
		{
			duint32 nextH = *it;
			mit = objectMap.find(nextH);
			if (mit == objectMap.end())
			{
				nextH = pline.lastEH;
				DRW_DBG("\nWARNING: Entity of block not found\n");
				ret = false;
				continue;
			}
			else
			{
				//found entity reads it
				oc = mit->second;
				objectMap.erase(mit);
				DRW_DBG("\nPline vertex, parsing entity: ");
				DRW_DBGH(oc.handle);
				DRW_DBG(", pos: ");
				DRW_DBG(oc.loc);
				DRW_DBG("\n");
				DRW_Vertex vt;
				dbuf->setPosition(oc.loc);
				//RLZ: verify if pos is ok
				int size = dbuf->getModularShort();
				if (version > DRW::R2007)
				{  //2010+
					bs = dbuf->getUModularChar();
				}
				auto* tmpByteStr = new duint8[size];
				dbuf->getBytes(tmpByteStr, size);
				dwgBuffer buff(tmpByteStr, size, &decoder);
				dint16 dwgType = buff.getObjType(version);
				buff.resetPosition();
				DRW_DBG(" object type= ");
				DRW_DBG(dwgType);
				DRW_DBG("\n");
				ret2 = vt.parseDwg(version, &buff, bs, pline.basePoint.z);
				delete[] tmpByteStr;
				pline.addVertex(vt);
				nextEntityLink = vt.nextEntityLink;
				preEntityLink = vt.preEntityLink;
				ret = ret && ret2;
			}
		}
	}  //end 2004+
	DRW_DBG("\nRemoved SEQEND entity: ");
	DRW_DBGH(pline.seqEndH.ref);
	DRW_DBG("\n");
	objectMap.erase(pline.seqEndH.ref);
	return ret;
}

bool DwgReader::readDwgEntities(DRW_Interface& intfa, dwgBuffer* dbuf)
{
	bool ret = true;
	bool ret2 = true;
	auto itB = objectMap.begin();
	while (itB != objectMap.end())
	{
		ret2 = readDwgEntity(dbuf, itB->second, intfa);
		if (ret2 == false)
			return false;
		objectMap.erase(itB);
		itB = objectMap.begin();
		if (ret)
			ret = ret2;
	}

	return ret;
}

/**
 * Reads a dwg drawing entity (dwg object entity) given its offset in the file
 */
bool DwgReader::readDwgEntity(dwgBuffer* dbuf, ObjHandle& obj, DRW_Interface& intfa)
{
	bool ret = true;
	duint32 bs = 0;
	/*     -----------       parse entity start        -------------           */
	// set to 0 to skip unimplemented entities
	nextEntityLink = preEntityLink = 0;
	dbuf->setPosition(obj.loc);
	if (!dbuf->isGood())
	{
		// Warning: readDwgEntity, bad location
		DRW_DBG("Warning: readDwgEntity, bad location\n");
		return true;
	}

	// entity size not including the size of obj itself
	int size = dbuf->getModularShort();

	//2010+
	if (version >= DRW::R2010)
	{
		bs = dbuf->getUModularChar();
	}

	std::vector<duint8> tempByteVec(size);
	dbuf->getBytes(tempByteVec.data(), size);
	if (!dbuf->isGood())
	{
		return true;
	}
	dwgBuffer buff(tempByteVec.data(), size, &decoder);
	dint16 dwgType = buff.getObjType(version);
	buff.resetPosition();

	//500+
	if (dwgType > 499)
	{
		auto it = classesmap.find(dwgType);
		if (it == classesmap.end())
		{
			//fail, not found in classes map and set error
			spdlog::get(pDwgR->getFilePath())->error("fail, not found in classes set error\n");
			return true;
		}
		auto& cla = it->second;
		if (cla->dwgType != 0)
		{
			dwgType = cla->dwgType;
			//todo maybe need to be implemented
		}
	}

	obj.type = dwgType;

	switch (dwgType)
	{
		case DRW::TEXT:
		{
			DRW_Text e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::STYLE, e.styleH.ref);
			intfa.addText(e);
			break;
		}
		case DRW::ATTRIB:
		{
			DRW_Attrib e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::STYLE, e.styleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addAttrib(e);
			break;
		}
		case DRW::ATTDEF:
		{
			DRW_Attdef e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::STYLE, e.styleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addAttdef(e);
			break;
		}
		case DRW::INSERT:  // INSERT
		{
			DRW_Insert e;
			ENTRY_PARSE(e);
			duint32 next = nextEntityLink;
			duint32 pre = preEntityLink;
			e.blockName = findTableName(DRW::BLOCK_HEADER, e.blockHeaderH.ref);
			intfa.addInsert(e);
			if (e.hasAttrib)
			{
				for (duint32 i = e.firstAttribHandleRef; i <= e.lastAttribHandleRef; i++)
				{
					auto mapIt = objectMap.find(i);
					if (mapIt == objectMap.end())
					{
						DRW_DBG("\nwarning:not found attrib handle in object map:");
						DRW_DBG(i);
						DRW_DBG('\n');
					}
					else
					{
						obj = mapIt->second;
						objectMap.erase(i);
						bool ret3 = readDwgEntity(dbuf, obj, intfa);
						ret = ret & ret3;
					}
				}
			}
			nextEntityLink = next;
			preEntityLink = pre;
			break;
		}
		// MINSERT
		case DRW::MINSERT:
		case DRW::MINSERT + 1:
		{
			throw std::runtime_error("there are minsert entities without implementation");
		}
		case DRW::ARC:
		{
			DRW_Arc e;
			ENTRY_PARSE(e);
			intfa.addArc(e);
			break;
		}
		case DRW::CIRCLE:
		{
			DRW_Circle e;
			ENTRY_PARSE(e);
			intfa.addCircle(e);
			break;
		}
		case DRW::LINE:
		{
			DRW_Line e;
			ENTRY_PARSE(e);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addLine(e);
			break;
		}
		case DRW::DIMENSION_ORDINATE:
		{
			DRW_DimOrdinate e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addDimOrdinate(&e);
			break;
		}
		case DRW::DIMENSION_LINEAR:
		{
			DRW_DimLinear e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addDimLinear(&e);
			break;
		}
		case DRW::DIMENSION_ALIGNED:
		{
			DRW_DimAligned e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addDimAlign(&e);
			break;
		}
		case DRW::DIMENSION_ANG3PT:
		{
			DRW_DimAngular3p e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			intfa.addDimAngular3P(&e);
			break;
		}
		case DRW::DIMENSION_ANG2LN:
		{
			DRW_DimAngular e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addDimAngular(&e);
			break;
		}
		case DRW::DIMENSION_RADIUS:
		{
			DRW_DimRadius e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			e.layer = findTableName(DRW::LAYER, e.layerHandleRef);
			intfa.addDimRadius(&e);
			break;
		}
		case DRW::DIMENSION_DIAMETER:
		{
			DRW_DimDiameter e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			intfa.addDimDiameter(&e);
			break;
		}
		case DRW::POINT:
		{
			DRW_Point e;
			ENTRY_PARSE(e);
			intfa.addPoint(e);
			break;
		}
		case DRW::FACE_3D:
		{
			DRW_3Dface e;
			ENTRY_PARSE(e);
			intfa.add3dFace(e);
			break;
		}
		case DRW::POLYLINE_2D:		// pline 2D
		case DRW::POLYLINE_3D:		// pline 3D
		//case DRW::POLYLINE_PFACE:	// p face
		//case DRW::POLYLINE_MESH:	// mesh
		{
			DRW_Polyline e;
			ENTRY_PARSE(e);
			readPlineVertex(e, dbuf);
			intfa.addPolyline(e);
			break;
		}
		case DRW::SOLID:
		{
			DRW_Solid e;
			ENTRY_PARSE(e);
			intfa.addSolid(e);
			break;
		}
		case DRW::DwgType::TRACEE:
		{
			DRW_Trace e;
			ENTRY_PARSE(e);
			intfa.addTrace(e);
			break;
		}
		case DRW::VIEWPORT:
		{
			DRW_Viewport e;
			ENTRY_PARSE(e);
			intfa.addViewport(e);
			break;
		}
		case DRW::ELLIPSE:
		{
			DRW_Ellipse e;
			ENTRY_PARSE(e);
			intfa.addEllipse(e);
			break;
		}
		case DRW::SPLINE:
		{
			DRW_Spline e;
			ENTRY_PARSE(e);
			intfa.addSpline(&e);
			break;
		}
		case DRW::RAY:
		{
			DRW_Ray e;
			ENTRY_PARSE(e);
			intfa.addRay(e);
			break;
		}
		case DRW::XLINE:
		{
			DRW_Xline e;
			ENTRY_PARSE(e);
			intfa.addXline(e);
			break;
		}
		case DRW::MTEXT:
		{
			DRW_MText e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::STYLE, e.styleH.ref);
			intfa.addMText(e);
			break;
		}
		case DRW::LEADER:
		{
			DRW_Leader e;
			ENTRY_PARSE(e);
			e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
			intfa.addLeader(&e);
			break;
		}
		case DRW::LWPOLYLINE:
		{
			DRW_LWPolyline e;
			ENTRY_PARSE(e);
			intfa.addLWPolyline(e);
			break;
		}
		//case 78:
		//{
		//	//1
		//	DRW_Hatch e;
		//	ENTRY_PARSE(e);
		//	intfa.addHatch(&e);
		//	break;
		//}
		//? pending to verify
		/*case 101:
		{
			DRW_Image e;
			ENTRY_PARSE(e);
			intfa.addImage(&e);
			break;
		}*/
		default:
		{
			//not supported or are object add to remaining map
			objObjectMap[obj.handle] = obj;
			DRW_DBG("\nobjType=");
			DRW_DBG(obj.type);
			DRW_DBG("\n");
			DRW_DBG("\nnot supported or are object add to remaining map\n");
		}
		break;
	}
	DRW_DBG("\nret=");
	DRW_DBG(ret);
	return ret;
}

bool DwgReader::readDwgObjects(DRW_Interface& intfa, dwgBuffer* dbuf)
{
	bool ret = true;
	duint32 i = 0;
	auto itB = objObjectMap.begin();
	const auto itE = objObjectMap.end();
	while (itB != itE)
	{
		bool ret2 = readDwgObject(dbuf, itB->second, intfa);
		objObjectMap.erase(itB);
		itB = objObjectMap.begin();
		if (ret)
			ret = ret2;
	}
	return ret;
}

/**
 * Reads a dwg drawing object (dwg object object) given its offset in the file
 */
bool DwgReader::readDwgObject(dwgBuffer* dbuf, ObjHandle& obj, DRW_Interface& intfa)
{
	bool ret = true;
	duint32 bs = 0;
	dbuf->setPosition(obj.loc);
	//verify if position is ok:
	if (!dbuf->isGood())
	{
		// Warning: readDwgEntity, bad location
		DRW_DBG("Warning: readDwgObject, bad location\n");
		return true;
	}
	const int size = dbuf->getModularShort();
	if (version > DRW::R2007)
	{
		//2010+
		bs = dbuf->getUModularChar();
	}
	std::vector<duint8> tempByteVec(size);
	dbuf->getBytes(tempByteVec.data(), size);
	if (!dbuf->isGood())
	{
		return true;
	}
	dwgBuffer buff(tempByteVec.data(), size, &decoder);
	//dwgType are set parsing entities
	const auto dwgType = static_cast<DRW::DwgType>(buff.getObjType(version));
	buff.resetPosition();
	obj.type = dwgType;

	switch (dwgType)
	{
		/*case DRW::IMAGEDEF:
		{
			DRW_ImageDef e;
			ret = e.parseDwg(version, &buff, bs);
			intfa.linkImage(&e);
			break;
		}*/
		case DRW::LAYOUT:
		{
			DRW_Layout e;
			ret = e.parseDwg(version, &buff, bs);
			break;
		}
		case DRW::LAYER_INDEX:
		{
			//false
			break;
		}
		default:
			//not supported object and add to remaining map for debug
			unhandledObjMap[obj.handle] = obj;
			break;
	}
	return ret;
}

bool DRW_ObjControl::parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs)
{
	int unkData = 0;
	bool ret = DRW_TableEntry::parseDwg(version, buf, NULL, bs);
	DRW_DBG("\n***************************** parsing object control entry *********************************************\n");
	if (!ret)
		return ret;
	//last parsed is XDic Missing Flag 2004+
	int numEntries = buf->getBitLong();
	DRW_DBG(" num entries: ");
	DRW_DBG(numEntries);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    if (dwgType == 68 && version== DRW::R2000){//V2000 dimstyle seems have one unknown byte hard handle counter??
	if (dwgType == 68 && version > DRW::R14)
	{  //dimstyle seems have one unknown byte hard handle counter??
		unkData = buf->getRawChar8();
		DRW_DBG(" unknown v2000 byte: ");
		DRW_DBG(unkData);
		DRW_DBG("\n");
	}

	if (version > DRW::R2004)
	{  //from v2007+ have a bit for strings follows
//wrong in this (ObjControl do not have)
		int stringBit = buf->getBit();
		DRW_DBG(" string bit for  v2007+: ");
		DRW_DBG(stringBit);
		DRW_DBG("\n");
	}

	dwgHandle objectH = buf->getHandle();
	DRW_DBG(" NULL Handle: ");
	DRW_DBGHL(objectH.code, objectH.size, objectH.ref);
	DRW_DBG("\n");
	DRW_DBG("Remaining bytes: ");
	DRW_DBG(buf->numRemainingBytes());
	DRW_DBG("\n");

	//    if (dwgType == 56 && version== DRW::R2000){//linetype in 2004 seems not have XDicObjH or NULL handle
	if (xDictMissingFlag != 1)
	{  //linetype in 2004 seems not have XDicObjH or NULL handle
		dwgHandle XDicObjH = buf->getHandle();
		DRW_DBG(" XDicObj control Handle: ");
		DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	//add 2 for modelspace, paperspace blocks & bylayer, byblock linetypes
	numEntries = ((dwgType == 48) || (dwgType == 56)) ? (numEntries + 2) : numEntries;

	for (int i = 0; i < numEntries; i++)
	{
		objectH = buf->getOffsetHandle(handle);
		if (objectH.ref != 0)  //in vports R14  I found some NULL handles
			handleList.push_back(objectH.ref);
		DRW_DBG(" objectH Handle: ");
		DRW_DBGHL(objectH.code, objectH.size, objectH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}

	for (int i = 0; i < unkData; i++)
	{
		objectH = buf->getOffsetHandle(handle);
		DRW_DBG(" unknown Handle: ");
		DRW_DBGHL(objectH.code, objectH.size, objectH.ref);
		DRW_DBG("\n");
		DRW_DBG("Remaining bytes: ");
		DRW_DBG(buf->numRemainingBytes());
		DRW_DBG("\n");
	}
	return buf->isGood();
}