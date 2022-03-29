#ifndef DWGREADER32_H
#define DWGREADER32_H

#include "dwgreader27.h"

// `dwg reader 32 for R2018+
class DwgReader32 :public DwgReader27
{
public:
	DwgReader32(std::ifstream* _pIfstream, DwgR* _pDwgR) :
		DwgReader27(_pIfstream, _pDwgR) {}

	virtual ~DwgReader32(){}
	bool readFileHeader() override;
	bool readDwgHeader(DRW_Header& _rHdr) override;
	bool readDwgClasses() override;
	bool readDwgBlock(DRW_Interface& _rInterf);
	bool readDwgEntities(DRW_Interface& _rInterf) override;
	bool readDwgObjects(DRW_Interface& intfa) override;
};

#endif //DWGREADER32_H