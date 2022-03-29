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

#include "dwgreader21.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../libdwgr.h"
#include "drw_dbg.h"
#include "drw_textcodec.h"

bool DwgReader21::readMetaData()
{
	version = pDwgR->getVersion();
	decoder.setVersion(version, false);
	if (!fileBuf->setPosition(11))
		return false;
	// maintenance verion=
	maintenanceVersion = fileBuf->getRawChar8();
	// byte at 0x0C=
	fileBuf->getRawChar8();
	// previewImagePos (seekerImageData) =
	previewImagePos = fileBuf->getRawLong32();
	// app writer version=
	fileBuf->getRawChar8();
	// app writer maintenance version=
	fileBuf->getRawChar8();
	// codepage=
	duint16 cp = fileBuf->getRawShort16();
	if (cp == 30)
		decoder.setCodePage("ANSI_1252", false);
	// unknown 3 bytes
	fileBuf->getRawShort16();
	fileBuf->getRawChar8();
	// security type flag=
	duint32 secType = fileBuf->getRawLong32();
	// unknown long
	fileBuf->getRawLong32();
	// Summary info address= maybe [AcDb:SummaryInfo]
	fileBuf->getRawLong32();
	// VBA Project Addr
	fileBuf->getRawLong32();
	//  0x00000080 32b=128 fixed
	fileBuf->getRawLong32();
	// App info address=  maybe [AcDb:AppInfo]
	fileBuf->getRawLong32();
	//current position are 0x30 from here to 0x80 are undocumented
	DRW_DBG("\nAnother address? = ");
	DRW_DBGH(fileBuf->getRawLong32());
	return true;
}

bool DwgReader21::parseSysPage(duint64 sizeCompressed, duint64 sizeUncompressed, duint64 correctionFactor, duint64 offset, duint8* decompData)
{
#if 1
	//round to 8
	//duint64 alsize = (sizeCompressed + 7) & (-8);
	duint64 alsize = (sizeCompressed + 7) & (-8);

	//minimum RS chunk:
	duint64 chunks = (((alsize * correctionFactor) + 238) / 239);
	duint64 fpsize = chunks * 255;

	if (!fileBuf->setPosition(offset))
		return false;
	duint8* tmpDataRaw = new duint8[fpsize];
	fileBuf->getBytes(tmpDataRaw, fpsize);
	duint8* tmpDataRS = new duint8[fpsize];
	dwgRSCodec::decode239I(tmpDataRaw, tmpDataRS, static_cast<duint64>(fpsize / 255));
	dwgCompressor::decompress21(tmpDataRS, decompData, sizeCompressed, sizeUncompressed);
	delete[] tmpDataRaw;
	delete[] tmpDataRS;
#endif

#if 0
	// pre RS encoded size(round to a multiple of 8)
	duint64 peSize = ((sizeCompressed + 7) & (-7)) * correctionFactor;
	// divided pre encoded size by RS k-value(239)
	duint64 blockCount = (peSize + 238) / 239;
	// multiply with codeword size(255) and round to a multiple of 8
	duint64 pageSize = (blockCount * 255 + 7) & ~7;

	if (!fileBuf->setPosition(offset))
		return false;
	std::vector<duint8> dataRaw(pageSize);
	std::vector<duint8> dataRS(pageSize);
	dwgRSCodec::decode239I(dataRaw.data(), dataRS.data(), blockCount);
	if (sizeCompressed < sizeUncompressed)
		dwgCompressor::decompress21(dataRS.data(), decompData, sizeCompressed, sizeUncompressed);
	else
		std::copy_n(decompData, sizeUncompressed, dataRS.begin());
#endif
	return true;
}

bool DwgReader21::parseDataPage(DwgSectionInfo si, duint8* dData)
{

	DRW_DBG("parseDataPage, section size:");
	DRW_DBG(si.size);
	DRW_DBG('\n');
	for (auto& page : si.pages)
	{
		DwgPageInfo pi = page.second;
		if (!fileBuf->setPosition(pi.address))
			return false;


		std::vector<duint8> pageRaw(pi.size);
		std::vector<duint8> pageRS(pi.size);
		fileBuf->getBytes(pageRaw.data(), pi.size);
		if (pi.cSize == pi.uSize)
		{
			DRW_DBG("\nthis page in section is not compressed\n");
			memcpy(dData + pi.startOffset, pageRaw.data(), pi.cSize);
			return true;
		}
		duint8 chunks = pi.size / 255;
		dwgRSCodec::decode251I(pageRaw.data(), pageRS.data(), chunks);

		// page uncompressed size :
		DRW_DBG(pi.uSize);
		// comp size:
		DRW_DBG(pi.cSize);
		// offset:
		DRW_DBG(pi.startOffset);
		duint8* pageData = dData + pi.startOffset;

		dwgCompressor::decompress21(pageRS.data(), pageData, pi.cSize, pi.uSize);

	}
	DRW_DBG("\n");
	return true;
}

bool DwgReader21::readFileHeader()
{
	DRW_DBG("\n\ndwgReader21::parsing file header\n");
	if (!fileBuf->setPosition(0x80))
		return false;

	duint8 fileHdrRaw[0x3D8]{};
	fileBuf->getBytes(fileHdrRaw, 0x3D8);
	duint8 fileHdrdRS[0x3D8]{};
	dwgRSCodec::decode239I(fileHdrRaw, fileHdrdRS, 3);

#if 0
	DRW_DBG("\ndwgReader21::parsed Reed Solomon decode:\n");
	int j = 0;
	for (int i = 0, j = 0; i < 0x2CD; i++)
	{
		DRW_DBGH((unsigned char)fileHdrdRS[i]);
		if (j == 15)
		{
			j = 0;
			DRW_DBG("\n");
		}
		else
		{
			j++;
			DRW_DBG(", ");
		}
	}
	DRW_DBG("\n");
#endif
	dwgBuffer fileHdrBuf(fileHdrdRS, 0x2CD, &decoder);
	DRW_DBG("\nCRC 64b= ");
	DRW_DBGH(fileHdrBuf.getRawLong64());
	DRW_DBG("\nunknown key 64b= ");
	DRW_DBGH(fileHdrBuf.getRawLong64());
	DRW_DBG("\ncomp data CRC 64b= ");
	DRW_DBGH(fileHdrBuf.getRawLong64());
	DRW_DBG("\ncompr len 4bytes= ");
	dint32 fileHdrComprLen = fileHdrBuf.getRawLong32();
	DRW_DBG(fileHdrComprLen);
	DRW_DBG("\nlength2 4bytes= ");
	dint32 fileHdrLength2 = fileHdrBuf.getRawLong32();
	DRW_DBG(fileHdrLength2);// 0 when compressed


	int fileHdrDataLength = 0x110;  // fixed size
	std::vector<duint8> fileHdrData;
	// not decompressed
	if (fileHdrComprLen < 0)
	{
		fileHdrDataLength = fileHdrComprLen * -1;
		fileHdrData.resize(fileHdrDataLength);
		fileHdrBuf.getBytes(fileHdrData.data(), fileHdrDataLength);
	}
	// compressed
	else
	{
		DRW_DBG("\ndwgReader21:: file header are compresed:\n");
		std::vector<duint8> fileHdrComprData(fileHdrComprLen);
		fileHdrBuf.getBytes(fileHdrComprData.data(), fileHdrComprLen);
		fileHdrData.resize(fileHdrDataLength);
		dwgCompressor::decompress21(fileHdrComprData.data(), fileHdrData.data(), fileHdrComprLen, fileHdrDataLength);
	}

#if 0
	DRW_DBG("\ndwgReader21::parsed file header:\n");
	for (int i = 0, j = 0; i < fileHdrDataLength; i++)
	{
		DRW_DBGH((unsigned char)fileHdrData[i]);
		if (j == 15)
		{
			j = 0;
			DRW_DBG("\n");
		}
		else
		{
			j++;
			DRW_DBG(", ");
		}
	}
	DRW_DBG("\n");
#endif

	dwgBuffer fileHdrDataBuf(fileHdrData.data(), fileHdrDataLength, &this->decoder);
	DRW_DBG("\nHeader size = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nFile size = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nPagesMapCrcCompressed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	duint64 PagesMapCorrectionFactor = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nPagesMapCorrectionFactor = ");
	DRW_DBG(PagesMapCorrectionFactor);
	DRW_DBG("\nPagesMapCrcSeed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nPages map2offset = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());  //relative to data page map 1, add 0x480 to get stream position
	DRW_DBG("\nPages map2Id = ");
	DRW_DBG(fileHdrDataBuf.getRawLong64());
	duint64 PagesMapOffset = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nPagesMapOffset = ");
	DRW_DBGH(PagesMapOffset);  //relative to data page map 1, add 0x480 to get stream position
	DRW_DBG("\nPagesMapId = ");
	DRW_DBG(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nHeader2offset = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());  //relative to data page map 1, add 0x480 to get stream position
	duint64 PagesMapSizeCompressed = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nPagesMapSizeCompressed = ");
	DRW_DBG(PagesMapSizeCompressed);
	duint64 PagesMapSizeUncompressed = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nPagesMapSizeUncompressed = ");
	DRW_DBG(PagesMapSizeUncompressed);
	DRW_DBG("\nPagesAmount = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	duint64 PagesMaxId = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nPagesMaxId = ");
	DRW_DBG(PagesMaxId);
	DRW_DBG("\nUnknown (normally 0x20) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nUnknown (normally 0x40) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nPagesMapCrcUncompressed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nUnknown (normally 0xf800) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nUnknown (normally 4) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nUnknown (normally 1) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nSectionsAmount (number of sections + 1) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nSectionsMapCrcUncompressed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	duint64 SectionsMapSizeCompressed = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nSectionsMapSizeCompressed = ");
	DRW_DBGH(SectionsMapSizeCompressed);
	DRW_DBG("\nSectionsMap2Id = ");
	DRW_DBG(fileHdrDataBuf.getRawLong64());
	duint64 SectionsMapId = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nSectionsMapId = ");
	DRW_DBG(SectionsMapId);
	duint64 SectionsMapSizeUncompressed = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nSectionsMapSizeUncompressed = ");
	DRW_DBGH(SectionsMapSizeUncompressed);
	DRW_DBG("\nSectionsMapCrcCompressed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	duint64 SectionsMapCorrectionFactor = fileHdrDataBuf.getRawLong64();
	DRW_DBG("\nSectionsMapCorrectionFactor = ");
	DRW_DBG(SectionsMapCorrectionFactor);
	DRW_DBG("\nSectionsMapCrcSeed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nStreamVersion (normally 0x60100) = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nCrcSeed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nCrcSeedEncoded = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nRandomSeed = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\nHeader CRC64 = ");
	DRW_DBGH(fileHdrDataBuf.getRawLong64());
	DRW_DBG("\n");


	DRW_DBG("\ndwgReader21::parse pages map:\n");
#pragma region pages map
	std::vector<duint8> PagesMapData(PagesMapSizeUncompressed);
	//system pages map
	bool ret = parseSysPage(PagesMapSizeCompressed, PagesMapSizeUncompressed, PagesMapCorrectionFactor, 0x480 + PagesMapOffset, PagesMapData.data());
	if (!ret)
		return false;

	duint64 address = 0x480;
	duint64 i = 0;
	dwgBuffer PagesMapBuf(PagesMapData.data(), PagesMapSizeUncompressed, &decoder);
	//stores temporally info of all pages:
	std::map<duint32, DwgPageInfo> sectionPageMapTmp;

	while (PagesMapSizeUncompressed > i)
	{
		duint64 size = PagesMapBuf.getRawLong64();
		dint64 id = PagesMapBuf.getRawLong64();
		duint64 ind = id > 0 ? id : -id;
		i += 16;

		DRW_DBG("Page gap= ");
		DRW_DBG(id);
		DRW_DBG(" Page num= ");
		DRW_DBG(ind);
		DRW_DBG(" size= ");
		DRW_DBGH(size);
		DRW_DBG(" address= ");
		DRW_DBGH(address);
		DRW_DBG("\n");
		sectionPageMapTmp[ind] = DwgPageInfo(ind, address, size);
		address += size;
		//TODO num can be negative indicating gap
		//        seek += offset;
	}
#pragma endregion pages map
	DRW_DBG("\n*** dwgReader21: Processing Section Map ***\n");

#pragma region sections map
	std::vector<duint8> SectionsMapData(SectionsMapSizeUncompressed + 0x1000);
	//todo check max page id
	DwgPageInfo sectionMap = sectionPageMapTmp[SectionsMapId];

	ret = parseSysPage(SectionsMapSizeCompressed, SectionsMapSizeUncompressed,
					   SectionsMapCorrectionFactor, sectionMap.address, SectionsMapData.data());
	if (!ret)
		return false;

	//reads sections:
	//Note: compressed value are not stored in file then, compressed field are use to store
	// encoding value
	dwgBuffer SectionsMapBuf(SectionsMapData.data(), SectionsMapSizeUncompressed, &decoder);
	duint8 nextId = 1;
	while (SectionsMapBuf.getPosition() < SectionsMapBuf.size())
	{
		DwgSectionInfo secInfo;
		secInfo.size = SectionsMapBuf.getRawLong64();
		DRW_DBG("\nSize of section (data size)= ");
		DRW_DBGH(secInfo.size);
		secInfo.maxSize = SectionsMapBuf.getRawLong64();
		DRW_DBG("\nMax Decompressed Size= ");
		DRW_DBGH(secInfo.maxSize);
		secInfo.encrypted = SectionsMapBuf.getRawLong64();
		//encrypted (doc: 0 no, 1 yes, 2 unknown) on read: objects 0 and encrypted yes
		DRW_DBG("\nencription= ");
		DRW_DBGH(secInfo.encrypted);
		DRW_DBG("\nHashCode = ");
		DRW_DBGH(SectionsMapBuf.getRawLong64());
		duint64 SectionNameLength = SectionsMapBuf.getRawLong64();
		DRW_DBG("\nSectionNameLength = ");
		DRW_DBG(SectionNameLength);
		DRW_DBG("\nUnknown = ");
		DRW_DBGH(SectionsMapBuf.getRawLong64());
		secInfo.compresed = SectionsMapBuf.getRawLong64();
		DRW_DBG("\nEncoding (compresed) = ");
		DRW_DBGH(secInfo.compresed);
		secInfo.pageCount = SectionsMapBuf.getRawLong64();
		DRW_DBG("\nPage count= ");
		DRW_DBGH(secInfo.pageCount);
		secInfo.name = SectionsMapBuf.getUCSStr(SectionNameLength);
		DRW_DBG("\nSection name = ");
		DRW_DBG(secInfo.name);
		DRW_DBG("\n");

		for (unsigned int i = 0; i < secInfo.pageCount; i++)
		{
			duint64 po = SectionsMapBuf.getRawLong64();
			duint32 ds = SectionsMapBuf.getRawLong64();
			duint32 pn = SectionsMapBuf.getRawLong64();
			DRW_DBG("  page Id = ");
			DRW_DBGH(pn);
			DRW_DBG(" data size = ");
			DRW_DBGH(ds);
			DwgPageInfo pi = sectionPageMapTmp[pn];  //get a copy
			pi.dataSize = ds;
			pi.startOffset = po;
			pi.uSize = SectionsMapBuf.getRawLong64();
			pi.cSize = SectionsMapBuf.getRawLong64();
			secInfo.pages[pn] = pi;  //complete copy in secInfo
			DRW_DBG("\n    Page number= ");
			DRW_DBGH(secInfo.pages[pn].id);
			DRW_DBG("\n    address in file= ");
			DRW_DBGH(secInfo.pages[pn].address);
			DRW_DBG("\n    size in file= ");
			DRW_DBGH(secInfo.pages[pn].size);
			DRW_DBG("\n    Data size= ");
			DRW_DBGH(secInfo.pages[pn].dataSize);
			DRW_DBG("\n    Start offset= ");
			DRW_DBGH(secInfo.pages[pn].startOffset);
			DRW_DBG("\n    Page uncompressed size = ");
			DRW_DBGH(secInfo.pages[pn].uSize);
			DRW_DBG("\n    Page compressed size = ");
			DRW_DBGH(secInfo.pages[pn].cSize);

			DRW_DBG("\n    Page checksum = ");
			DRW_DBGH(SectionsMapBuf.getRawLong64());
			DRW_DBG("\n    Page CRC = ");
			DRW_DBGH(SectionsMapBuf.getRawLong64());
			DRW_DBG("\n");
		}

		if (!secInfo.name.empty())
		{
			secInfo.id = nextId++;
			DRW_DBG("Saved section Name= ");
			DRW_DBG(secInfo.name.c_str());
			DRW_DBG("\n");
			sections[SecEnum::getEnum(secInfo.name)] = secInfo;
		}
	}
#pragma endregion regionName
	if (!fileBuf->isGood())
		return false;

	DRW_DBG("\ndwgReader21::readFileHeader END\n");
	return true;
}

bool DwgReader21::readDwgHeader(DRW_Header& hdr)
{
	DRW_DBG("\ndwgReader21::readDwgHeader\n");
	DwgSectionInfo si = sections[SecEnum::DWGSection::HEADER];
	if (si.id < 0)  //not found, ends
		return false;
	DRW_DBG("\nprepare section of size ");
	DRW_DBG(si.size);
	DRW_DBG("\n");
	duint8* tmpHeaderData = new duint8[si.size];
	bool ret = DwgReader21::parseDataPage(si, tmpHeaderData);
	if (!ret)
	{
		delete[] tmpHeaderData;
		return ret;
	}

	dwgBuffer dataBuf(tmpHeaderData, si.size, &decoder);
	dwgBuffer handleBuf(tmpHeaderData, si.size, &decoder);
	DRW_DBG("Header section sentinel= ");
	DRW::checkSentinel(&dataBuf, SecEnum::DWGSection::HEADER, true);
	ret = DwgReader::readDwgHeader(hdr, &dataBuf, &handleBuf);
	delete[] tmpHeaderData;

	return ret;
}

bool DwgReader21::readDwgClasses()
{
	DRW_DBG("\ndwgReader21::readDwgClasses");
	DwgSectionInfo si = sections[SecEnum::DWGSection::CLASSES];
	if (si.id < 0)  //not found, ends
		return false;

	DRW_DBG("\nprepare section of size ");
	DRW_DBG(si.size);
	DRW_DBG("\n");
	duint8* tmpClassesData = new duint8[si.size];
	bool ret = DwgReader21::parseDataPage(si, tmpClassesData);
	if (!ret)
		return ret;

	dwgBuffer buff(tmpClassesData, si.size, &decoder);
	DRW_DBG("classes section sentinel= ");
	DRW::checkSentinel(&buff, SecEnum::DWGSection::CLASSES, true);

	duint32 size = buff.getRawLong32();
	DRW_DBG("\ndata size in bytes ");
	DRW_DBG(size);

	duint32 bitSize = buff.getRawLong32();
	DRW_DBG("\ntotal size in bits ");
	DRW_DBG(bitSize);

	duint32 maxClassNum = buff.getBitShort();
	DRW_DBG("\nMaximum class number ");
	DRW_DBG(maxClassNum);
	DRW_DBG("\nRc 1 ");
	DRW_DBG(buff.getRawChar8());
	DRW_DBG("\nRc 2 ");
	DRW_DBG(buff.getRawChar8());
	DRW_DBG("\nBit ");
	DRW_DBG(buff.getBit());

	/*******************************/
	//prepare string stream
	dwgBuffer strBuff(tmpClassesData, si.size, &decoder);
	duint32 strStartPos = bitSize + 159;  //size in bits + 20 bytes (sn+size) - 1 bit (endbit)
	DRW_DBG("\nstrStartPos: ");
	DRW_DBG(strStartPos);
	strBuff.setPosition(strStartPos >> 3);
	strBuff.setBitPos(strStartPos & 7);
	DRW_DBG("\nclasses strings buff.getPosition: ");
	DRW_DBG(strBuff.getPosition());
	DRW_DBG("\nclasses strings buff.getBitPos: ");
	DRW_DBG(strBuff.getBitPos());
	DRW_DBG("\nendBit ");
	DRW_DBG(strBuff.getBit());
	strStartPos -= 16;  //decrement 16 bits
	DRW_DBG("\nstrStartPos: ");
	DRW_DBG(strStartPos);
	strBuff.setPosition(strStartPos >> 3);
	strBuff.setBitPos(strStartPos & 7);
	DRW_DBG("\nclasses strings buff.getPosition: ");
	DRW_DBG(strBuff.getPosition());
	DRW_DBG("\nclasses strings buff.getBitPos: ");
	DRW_DBG(strBuff.getBitPos());
	duint32 strDataSize = strBuff.getRawShort16();
	DRW_DBG("\nstrDataSize: ");
	DRW_DBG(strDataSize);
	if (strDataSize & 0x8000)
	{
		strStartPos -= 16;      //decrement 16 bits
		strDataSize &= 0x7FFF;  //strip 0x8000;
		strBuff.setPosition(strStartPos >> 3);
		strBuff.setBitPos(strStartPos & 7);
		duint32 hiSize = strBuff.getRawShort16();
		strDataSize |= (hiSize << 15);
	}
	strStartPos -= strDataSize;
	DRW_DBG("\nstrStartPos: ");
	DRW_DBG(strStartPos);
	strBuff.setPosition(strStartPos >> 3);
	strBuff.setBitPos(strStartPos & 7);
	DRW_DBG("\nclasses strings buff.getPosition: ");
	DRW_DBG(strBuff.getPosition());
	DRW_DBG("\nclasses strings buff.getBitPos: ");
	DRW_DBG(strBuff.getBitPos());

	/*******************************/

	duint32 endDataPos = maxClassNum - 499;
	DRW_DBG("\nbuff.getPosition: ");
	DRW_DBG(buff.getPosition());
	for (duint32 i = 0; i < endDataPos; i++)
	{
		auto cl = std::make_shared<DRW_Class>();
		cl->parseDwg(version, &buff, &strBuff);
		classesmap[cl->classNum] = cl;
		DRW_DBG("\nbuff.getPosition: ");
		DRW_DBG(buff.getPosition());
	}
	DRW_DBG("\nend classes data buff.getPosition: ");
	DRW_DBG(buff.getPosition());
	DRW_DBG("\nend classes data buff.getBitPos: ");
	DRW_DBG(buff.getBitPos());

	buff.setPosition(size + 20);  //sizeVal+sn+32bSize
	DRW_DBG("\nCRC: ");
	DRW_DBGH(buff.getRawShort16());
	DRW_DBG("\nclasses section end sentinel= ");
	DRW::checkSentinel(&buff, SecEnum::DWGSection::CLASSES, true);
	delete[] tmpClassesData;

	return buff.isGood();
}

bool DwgReader21::readDwgHandles()
{
	DRW_DBG("\ndwgReader21::readDwgHandles");
	DwgSectionInfo si = sections[SecEnum::DWGSection::HANDLES];
	if (si.id < 0)  //not found, ends
		return false;

	DRW_DBG("\nprepare section of size ");
	DRW_DBG(si.size);
	DRW_DBG("\n");

	if (si.maxSize != 0xf800)
	{
		DRW_DBG("\ninvalid page size on read handle section\n");
		return false;
	}
	// ! protect temporary
	//duint8* tmpHandlesData = new duint8[si.size + 0x100];
	std::vector<duint8> handlesData(si.size + 0x100);
	// ! write to the overflow
	bool ret = DwgReader21::parseDataPage(si, handlesData.data());
	if (ret)
	{
		dwgBuffer dataBuf(handlesData.data(), si.size, &decoder);
		ret = DwgReader::readDwgHandles(&dataBuf, 0, si.size);
	}

	//delete[] tmpHandlesData;
	return ret;
}

/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool DwgReader21::readDwgTables(DRW_Header& hdr)
{
	DRW_DBG("\ndwgReader21::readDwgTables\n");
	DwgSectionInfo si = sections[SecEnum::DWGSection::OBJECTS];
	if (si.id < 0)  //not found, ends
		return false;

	DRW_DBG("\nprepare section of size ");
	DRW_DBG(si.size);
	DRW_DBG("\n");

	if (si.maxSize != 0xf800)
	{
		DRW_DBG("\ninvalid page size on read objects section\n");
		return false;
	}

	if (si.size > 0x2f000000)
	{
		DRW_DBG("\ninvalid max decompression size");
		DRW_DBG(si.size);
		return false;
	}

	dataSize = si.size;

	// ? global buffer wrong
	// ! protect temporary
	objData.resize(dataSize + 0x100);
	bool ret = DwgReader21::parseDataPage(si, objData.data());
	if (!ret)
		return ret;

	DRW_DBG("readDwgTables total data size= ");
	DRW_DBG(dataSize);
	DRW_DBG("\n");
	dwgBuffer dataBuf(objData.data(), dataSize, &decoder);
	ret = DwgReader::readDwgTables(hdr, &dataBuf);

	return ret;
}

bool DwgReader21::readDwgBlocks(DRW_Interface& intfa)
{

	bool ret = true;
	dwgBuffer dataBuf(objData.data(), dataSize, &decoder);
	ret = DwgReader::readDwgBlocks(intfa, &dataBuf);
	return ret;
}

bool DwgReader21::readDwgEntities(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(objData.data(), dataSize, &decoder);
	ret = DwgReader::readDwgEntities(intfa, &dataBuf);
	return ret;
}

bool DwgReader21::readDwgObjects(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(objData.data(), dataSize, &decoder);
	ret = DwgReader::readDwgObjects(intfa, &dataBuf);
	return ret;
}
