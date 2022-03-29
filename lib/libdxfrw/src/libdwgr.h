#ifndef LIBDWGR_H
#define LIBDWGR_H

#include <string>
#include "drw_interface.h"
#include "intern/drw_textcodec.h"
#include "intern/dwgreader.h"


class DwgReader;

class DwgR
{
public:
	DwgR(const char* name);
	~DwgR();

	const std::string& getFilePath() const;

	/* return true if all ok */
	bool read(DRW_Interface* interface_, bool ext);
	bool getPreview();


	DRW::Version getVersion() const
	{
		return  version_;
	}

	DRW::Error getError() const
	{
		return  error_;
	}

	static void setDebug(DRW::DBG_LEVEL lvl);


private:
	// set up logger for every file 
	bool createLogger() const;

	bool openFile(std::ifstream* filestr);

	bool processDwg();

	DRW::Version version_;
	DRW::Error error_;
	std::string filePath_;
	bool applyExt; /* apply extrusion in entities to conv in 2D? */
	std::string strCodePage_;
	DRW_Interface* m_pIface;
	DwgReader* pReader_;
};

#endif // LIBDWGR_H
