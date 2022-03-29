#include "dwgreader32.h"

bool DwgReader32::readFileHeader()
{
	DRW_DBG("dwgReader32::readFileHeader\n");
	bool ret = DwgReader18::readFileHeader();
	DRW_DBG("dwgReader32::readFileHeader end\n");
	return ret;
}

bool DwgReader32::readDwgHeader(DRW_Header& _rHdr)
{
	bool ret = true;
	DRW_DBG("dwgRead32::readDwgHeader\n");
	ret = DwgReader18::readDwgHeader(_rHdr);
	DRW_DBG("dwgReader32::readDwgHeader end\n");
	return ret;
}

bool DwgReader32::readDwgClasses()
{
	bool ret = true;
	DRW_DBG("dwgRead32::readDwgClasses\n");
	ret = DwgReader18::readDwgClasses();
	DRW_DBG("dwgReader32::readDwgClasses end\n");
	return ret;
}

bool DwgReader32::readDwgBlock(DRW_Interface& _rInterf)
{
	bool ret = true;
	dwgBuffer dataBuf(this->objData, this->uncompSize, &this->decoder);
	ret = DwgReader::readDwgBlocks(_rInterf, &dataBuf);
	return ret;
}

bool DwgReader32::readDwgEntities(DRW_Interface& _rInterf)
{
	bool ret = true;
	dwgBuffer dataBuf(this->objData, this->uncompSize, &this->decoder);
	ret = DwgReader::readDwgEntities(_rInterf, &dataBuf);
	return ret;
}

bool DwgReader32::readDwgObjects(DRW_Interface& intfa)
{
	bool ret = true;
	dwgBuffer dataBuf(this->objData, this->uncompSize, &this->decoder);
	ret = DwgReader::readDwgEntities(intfa, &dataBuf);
	return ret;
}
