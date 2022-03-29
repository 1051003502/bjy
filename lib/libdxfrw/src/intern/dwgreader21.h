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

#ifndef DWGREADER21_H
#define DWGREADER21_H


#include "dwgbuffer.h"
#include "dwgreader.h"

//reader for R2007 aka v2007, chapter 5
class DwgReader21 : public DwgReader
{
public:
	DwgReader21(std::ifstream* stream, DwgR* p) :
		DwgReader(stream, p),
		dataSize{}{}
	virtual ~DwgReader21() = default;

	bool readMetaData() override;
	bool readFileHeader() override;
	bool readDwgHeader(DRW_Header& hdr) override;
	bool readDwgClasses() override;
	bool readDwgHandles() override;
	bool readDwgTables(DRW_Header& hdr) override;
	bool readDwgBlocks(DRW_Interface& intfa) override;
	bool readDwgEntities(DRW_Interface& intfa) override;

	bool readDwgObjects(DRW_Interface& intfa) override;

private:
	bool parseSysPage(duint64 sizeCompressed,
					  duint64 sizeUncompressed, 
					  duint64 correctionFactor, 
					  duint64 offset, 
					  duint8* decompData);
	
	bool parseDataPage(DwgSectionInfo si, duint8* dData);

	//duint8 *objData;
	std::vector<duint8> objData;
	duint64 dataSize;
};

#endif  // DWGREADER21_H
