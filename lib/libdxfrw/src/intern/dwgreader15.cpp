/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 Jos¨¦ F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "dwgreader15.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../libdwgr.h"
#include "drw_dbg.h"
#include "drw_textcodec.h"

/* read Meta data */
bool DwgReader15::readMetaData()
{
	version = pDwgR->getVersion();
	decoder.setVersion(version, false);
	// for locating the pos of preview img
	if (!fileBuf->setPosition(13))
		return false;
	previewImagePos = fileBuf->getRawLong32();
	/* MEASUREMENT system variable 2 bytes*/
	duint16 meas = fileBuf->getRawShort16();
	DRW_DBG("\nMEASUREMENT (0 = English, 1 = Metric)= "); DRW_DBG(meas);
	duint16 cp = fileBuf->getRawShort16();
	DRW_DBG("\ncodepage= "); DRW_DBG(cp); DRW_DBG("\n");
	if (cp == 29) //TODO RLZ: locate wath code page and correct this
		decoder.setCodePage("ANSI_1252", false);
	if (cp == 30)
		decoder.setCodePage("ANSI_1252", false);
	return true;
}

/* read File Header */
bool DwgReader15::readFileHeader()
{
	bool ret = true;
	DRW_DBG("dwgReader15::readFileHeader\n");
	if (!fileBuf->setPosition(21))
		return false;
	duint32 count = fileBuf->getRawLong32();
	DRW_DBG("count records= "); DRW_DBG(count); DRW_DBG("\n");

	for (unsigned int i = 0; i < count; i++)
	{
		duint8 rec = fileBuf->getRawChar8();
		duint32 address = fileBuf->getRawLong32();
		duint32 size = fileBuf->getRawLong32();
		DwgSectionInfo si;
		si.id = rec;
		si.size = size;
		si.address = address;
		if (rec == 0)
		{
			DRW_DBG("\nSection HEADERS address= ");
			sections[SecEnum::DWGSection::HEADER] = si;
		}
		else if (rec == 1)
		{
			DRW_DBG("\nSection CLASSES address= ");
			sections[SecEnum::DWGSection::CLASSES] = si;
		}
		else if (rec == 2)
		{
			DRW_DBG("\nSection OBJECTS (handles) address= ");
			sections[SecEnum::DWGSection::HANDLES] = si;
		}
		else if (rec == 3)
		{
			DRW_DBG("\nSection UNKNOWN address= ");
			sections[SecEnum::DWGSection::UNKNOWNS] = si;
		}
		else if (rec == 4)
		{
			DRW_DBG("\nSection R14DATA (AcDb:Template) address= ");
			sections[SecEnum::DWGSection::TEMPLATE] = si;
		}
		else if (rec == 5)
		{
			DRW_DBG("\nSection R14REC5 (AcDb:AuxHeader) address= ");
			sections[SecEnum::DWGSection::AUXHEADER] = si;
		}
		else
		{
			DRW_DBG("\nUnsupported section number\n");
		}
	}
	if (!fileBuf->isGood())
		return false;
	duint32 ckcrc = fileBuf->crc8(0, 0, static_cast<duint32>(fileBuf->getPosition()));

	switch (count)
	{
		case 3:
			ckcrc = ckcrc ^ 0xA598;
			break;
		case 4:
			ckcrc = ckcrc ^ 0x8101;
			break;
		case 5:
			ckcrc = ckcrc ^ 0x3CC4;
			break;
		case 6:
			ckcrc = ckcrc ^ 0x8461;
		default:;
	}

	DRW_DBG("\nfile header CRC= ");
	DRW_DBG(fileBuf->getRawShort16());
	DRW_DBG("\nfile header sentinel= ");
	DRW::checkSentinel(fileBuf, SecEnum::DWGSection::FILEHEADER, false);
	return ret;
}

bool DwgReader15::readDwgHeader(DRW_Header& hdr)
{
	DRW_DBG("dwgReader15::readDwgHeader\n");
	DwgSectionInfo si = sections[SecEnum::DWGSection::HEADER];
	if (si.id < 0)//not found, ends
		return false;
	if (!fileBuf->setPosition(si.address))
		return false;
	duint8* tmpByteStr = new duint8[si.size];
	fileBuf->getBytes(tmpByteStr, si.size);
	dwgBuffer buff(tmpByteStr, si.size, &decoder);
	DRW_DBG("Header section sentinel= ");
	DRW::checkSentinel(&buff, SecEnum::DWGSection::HEADER, true);
	bool ret = DwgReader::readDwgHeader(hdr, &buff, &buff);
	delete[]tmpByteStr;

	return ret;
}


bool DwgReader15::readDwgClasses()
{
	DRW_DBG("\ndwgReader15::readDwgClasses\n");
	DwgSectionInfo si = sections[SecEnum::DWGSection::CLASSES];
	if (si.id < 0) return false;
	// locate classes 
	if (!fileBuf->setPosition(si.address)) return false;

	DRW_DBG("classes section sentinel= ");
	DRW::checkSentinel(fileBuf, SecEnum::DWGSection::CLASSES, true);

	duint32 size = fileBuf->getRawLong32();
	if (size != (si.size - 38))
	{
		DRW_DBG("\nWARNING dwgReader15::readDwgClasses size are ");
		DRW_DBG(size);
		DRW_DBG(" and secSize - 38 are ");
		DRW_DBG(si.size - 38);
		DRW_DBG("\n");
	}
	std::vector<duint8> tempByte(size);
	fileBuf->getBytes(tempByte.data(), size);
	dwgBuffer buff(tempByte.data(), size, &decoder);
	size--; //reduce 1 byte instead of check pos + bitPos
	while (size > buff.getPosition())
	{
		auto cl = std::make_shared<DRW_Class>();
		cl->parseDwg(version, &buff, &buff);
		classesmap[cl->classNum] = cl;
	}
	DRW_DBG("\nCRC: ");
	DRW_DBGH(fileBuf->getRawShort16());
	DRW_DBG("\nclasses section end sentinel= ");
	DRW::checkSentinel(fileBuf, SecEnum::DWGSection::CLASSES, false);
	const bool ret = buff.isGood();
	return ret;
}

bool DwgReader15::readDwgHandles()
{
	DRW_DBG("\ndwgReader15::readDwgHandles\n");
	DwgSectionInfo si = sections[SecEnum::DWGSection::HANDLES];
	if (si.id < 0) return false;
	bool ret = DwgReader::readDwgHandles(fileBuf, static_cast<duint32>(si.address), static_cast<duint32>(si.size));
	return ret;
}

/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool DwgReader15::readDwgTables(DRW_Header& hdr)
{
	bool ret = DwgReader::readDwgTables(hdr, fileBuf);
	return ret;
}

/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool DwgReader15::readDwgBlocks(DRW_Interface& intfa)
{
	bool ret = true;
	ret = DwgReader::readDwgBlocks(intfa, fileBuf);
	return ret;
}

bool DwgReader15::readDwgEntities(DRW_Interface& intfa)
{
	bool ret = true;
	ret = DwgReader::readDwgEntities(intfa, fileBuf);
	if (ret == false)
		return false;
	return ret;
}

bool DwgReader15::readDwgObjects(DRW_Interface& intfa)
{
	bool ret = true;
	ret = DwgReader::readDwgObjects(intfa, fileBuf);
	return ret;
}

