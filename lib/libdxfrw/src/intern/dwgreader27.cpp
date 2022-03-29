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
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "drw_dbg.h"
#include "dwgreader27.h"
#include "drw_textcodec.h"
#include "../libdwgr.h"


bool DwgReader27::readFileHeader()
{
	DRW_DBG("dwgReader27::readFileHeader\n");
	bool ret = DwgReader18::readFileHeader();
	DRW_DBG("dwgReader27::readFileHeader END\n");
	return ret;
}

bool DwgReader27::readDwgHeader(DRW_Header& hdr)
{
	DRW_DBG("dwgReader27::readDwgHeader\n");
	bool ret = DwgReader18::readDwgHeader(hdr);
	DRW_DBG("dwgReader27::readDwgHeader END\n");

	return ret;
}

bool DwgReader27::readDwgClasses()
{
	DRW_DBG("dwgReader27::readDwgClasses");
	bool ret = DwgReader18::readDwgClasses();
	DRW_DBG("\ndwgReader27::readDwgClasses END\n");

	return ret;
}

bool DwgReader27::readDwgBlocks(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(objData, uncompSize, &decoder);
	ret = DwgReader::readDwgBlocks(intfa, &dataBuf);
	return ret;
}

bool DwgReader27::readDwgEntities(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(objData, uncompSize, &decoder);
	ret = DwgReader::readDwgEntities(intfa, &dataBuf);
	return ret;
}

bool DwgReader27::readDwgObjects(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(objData, uncompSize, &decoder);
	ret = DwgReader::readDwgObjects(intfa, &dataBuf);
	return ret;
}

