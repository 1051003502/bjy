#ifndef DWGREADER_H
#define DWGREADER_H

#include <fstream>
#include <list>
#include <map>
#include <string>
#include "../libdwgr.h"
#include "drw_textcodec.h"
#include "dwgbuffer.h"
#include "dwgutil.h"
#include "drw_classes.h"


class DwgR;

class ObjHandle
{
public:
	ObjHandle() = default;
	ObjHandle(duint32 t, duint32 h, duint32 l) {
		type = t;
		handle = h;
		loc = l;
	}
	duint32 type;
	duint32 handle;
	duint32 loc;
};

//until 2000 = 2000-
//since 2004 except 2007 = 2004+
// 2007 = 2007
// pages of section
/**
 * 2000-: No pages, only sections
 * 2004+: Id, page number (index)
 *        size, size of page in file stream
 *        address, address in file stream
 *        dataSize, data size for this page
 *        startOffset, start offset for this page
 *        cSize, compresed size of data
 *        uSize, uncompressed size of data
 * 2007: page Id, pageCount & pages
 *       size, size in file
 *       address, address in file stream
 *       dataSize
 *       startOffset, start position in decompressed data stream
 *       cSize, compresed size of data
 *       uSize, uncompressed size of data
 *
 * */
class DwgPageInfo
{
public:
	DwgPageInfo() = default;
	DwgPageInfo(duint64 i, duint64 ad, duint32 sz) {
		id = i;
		address = ad;
		size = sz;
		cSize = dataSize = startOffset = uSize = 0;
	}
	DwgPageInfo(const DwgPageInfo&) = default;
	DwgPageInfo& operator=(const DwgPageInfo&) = default;
	~DwgPageInfo() = default;

	duint64 id;           // page index
	duint64 address;      // in file stream, for rd18, rd21
	duint64 size;         // in file stream, for rd18, rd21
	duint64 dataSize;     // for rd18, rd21
	duint64 startOffset;  // for rd18, rd21
	duint64 cSize;        // compresed page size, for rd21
	duint64 uSize;        // uncompressed page size, for rd21
};

// sections of file
/* 2000-: No pages, only section Id, size  & address in file
 * 2004+: Id, Section Id
 *        size, total size of uncompressed data
 *        pageCount & pages, number of pages in section
 *        maxSize, max decompressed Size per page
 *        compresed, (1 = no, 2 = yes, normally 2)
 *        encrypted, (0 = no, 1 = yes, 2 = unknown)
 *        name, read & stored but not used
 * 2007: same as 2004+ except encoding, saved in compresed field
 * */
class DwgSectionInfo
{
public:
	DwgSectionInfo() {
		compresed = 1;  //1=no, 2=yes
		encrypted = 0;  //
		pageCount = 0;
		id = -1;
		address = size = maxSize = 0;
	}
	~DwgSectionInfo() = default;
	dint32 id;                             //section Id, 2000-   rd15 rd18
	std::string name;                      //section name rd18
	duint32 compresed;                     //is compresed? 1=no, 2=yes rd18, rd21(encoding)
	duint32 encrypted;                     //encrypted (doc: 0=no, 1=yes, 2=unkn) on read: objects 0 and encrypted yes rd18
	std::map<duint32, DwgPageInfo> pages;  //index, size, offset
	duint64 size;                          //size of section,  2000- rd15, rd18, rd21 (data size)
	duint64 pageCount;                     //number of pages (dwgPageInfo) in section rd18, rd21
	duint64 maxSize;                       //max decompressed size (needed??) rd18 rd21
	duint64 address;                       //address (seek) , 2000-
};



class DwgReader
{
	friend class DwgR;

public:
	DwgReader(std::ifstream* stream, DwgR* p) {
		fileBuf = new dwgBuffer(stream);
		pDwgR = p;
		decoder.setVersion(DRW::R2007, false);  //default 2007 in utf8(no convert)
		decoder.setCodePage("UTF-16", false);
		previewImagePos = -1;
		version = DRW::Version::UNKNOWN_VERSION;
		nextEntityLink = preEntityLink = 0;
		maintenanceVersion = 0;
	}
	virtual ~DwgReader();

protected:
	virtual bool readMetaData() = 0;
	virtual bool readPreview() { return false; }
	virtual bool readFileHeader() = 0;
	virtual bool readDwgHeader(DRW_Header& hdr) = 0;
	virtual bool readDwgClasses() = 0;
	virtual bool readDwgHandles() = 0;
	virtual bool readDwgTables(DRW_Header& hdr) = 0;
	virtual bool readDwgBlocks(DRW_Interface& intfa) = 0;
	virtual bool readDwgEntities(DRW_Interface& intfa) = 0;
	virtual bool readDwgObjects(DRW_Interface& intfa) = 0;



	virtual bool readDwgEntity(dwgBuffer* dbuf, ObjHandle& obj, DRW_Interface& intfa);
	
	bool readDwgObject(dwgBuffer* dbuf, ObjHandle& obj, DRW_Interface& intfa);
	
	void parseEntityConfig(DRW_Entity* e);

	void installEntityLayerConf(DRW_Entity* e);

	void installEntityLineTypeConf(DRW_Entity* e);

	std::string findTableName(DRW::DwgType table, dint32 handle);

	void setCodePage(std::string* c) { decoder.setCodePage(c, false); }
	std::string getCodePage() { return decoder.getCodePage(); }
	bool readDwgHeader(DRW_Header& hdr, dwgBuffer* buf, dwgBuffer* hBuf) const;
	bool readDwgHandles(dwgBuffer* dbuf, duint32 offset, duint32 size);
	bool readDwgTables(DRW_Header& hdr, dwgBuffer* dbuf);
	
	

	bool readDwgBlocks(DRW_Interface& intfa, dwgBuffer* dbuf);
	bool readDwgEntities(DRW_Interface& intfa, dwgBuffer* dbuf);
	bool readDwgObjects(DRW_Interface& intfa, dwgBuffer* dbuf);
	bool readPlineVertex(DRW_Polyline& pline, dwgBuffer* dbuf);

public:
	std::map<duint32, ObjHandle> objectMap;				// all objects start from here
	std::map<duint32, ObjHandle> objObjectMap;			//stores the objects & entities not read in readDwgEntities
	std::map<duint32, ObjHandle> unhandledObjMap;		//stores the objects & entities not read in all process, for debug only

	std::map<duint32, std::shared_ptr<DRW_LType>> lineTypeMap;          // line type
	std::map<duint32, std::shared_ptr<DRW_Layer>> layerMap;             // layer
	std::map<duint32, std::shared_ptr<DRW_Block>> blockMap;             // block
	std::map<duint32, std::shared_ptr<DRW_Textstyle>> styleMap;         // style(shape style)
	std::map<duint32, std::shared_ptr<DRW_Dimstyle>> dimstyleMap;       // dim style
	std::map<duint32, std::shared_ptr<DRW_Vport>> vportMap;             // vports
	std::map<duint32, std::shared_ptr<DRW_Block_Header>> blockHeaderMap;// block headers
	std::map<duint32, std::shared_ptr<DRW_AppId>> appIdMap;

	duint8 maintenanceVersion;

protected:
	dwgBuffer* fileBuf;
	DwgR* pDwgR;
	DRW::Version version;

	//seeker (position) for the beginning sentinel of the image data (R13 to R15)
	duint32 previewImagePos;

	//sections map
	std::map<SecEnum::DWGSection, DwgSectionInfo> sections;
	std::map<duint32, std::shared_ptr<DRW_Class>> classesmap;

protected:
	DRW_TextCodec decoder;

protected:
	duint32 nextEntityLink;
	duint32 preEntityLink;
};
#endif  // DWGREADER_H
