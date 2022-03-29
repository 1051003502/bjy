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

#ifndef DWGREADER24_H
#define DWGREADER24_H


#include "dwgbuffer.h"
#include "dwgreader18.h"

class DwgReader24 : public DwgReader18
{
public:
	DwgReader24(std::ifstream* stream, DwgR* p);

	virtual ~DwgReader24();

	bool readFileHeader() override;
	bool readDwgHeader(DRW_Header& hdr) override;
	bool readDwgClasses() override;

	bool readDwgBlocks(DRW_Interface& intfa) override;

	bool readDwgEntities(DRW_Interface& intfa) override;

	bool readDwgObjects(DRW_Interface& intfa) override;
};

#endif // DWGREADER24_H
