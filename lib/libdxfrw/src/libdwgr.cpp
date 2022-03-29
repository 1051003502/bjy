#include "libdwgr.h"
#include "intern/dwgreader18.h"
#include "intern/dwgreader15.h"
#include "intern/dwgreader32.h"
#include "intern/dwgreader24.h"
#include "intern/dwgreader21.h"



#define FIRSTHANDLE 48

DwgR::DwgR(const char* name) {
	filePath_ = name;
	pReader_ = nullptr;
	applyExt = false;
	version_ = DRW::UNKNOWN_VERSION;
	error_ = DRW::Error::BAD_NONE;
	
	createLogger();
}

DwgR::~DwgR() {
	if (pReader_ != nullptr)
	{
		spdlog::drop(this->filePath_);
		delete pReader_;
		pReader_ = nullptr;
	}
}


const std::string& DwgR::getFilePath() const
{
	return this->filePath_;
}

void DwgR::setDebug(const DRW::DBG_LEVEL lvl) {
	switch (lvl)
	{
		case DRW::DEBUG:
		{
			DRW_dbg::getInstance()->setLevel(DRW_dbg::DEBUGG);
		}
		break;
		case DRW::NONE:
			DRW_dbg::getInstance()->setLevel(DRW_dbg::NONE);
			break;
		default:
			break;
	}
}

/**
 * reads metadata and loads image preview
 * */
bool DwgR::getPreview() {
	bool bOk = false;

	std::ifstream filestr;
	bOk = openFile(&filestr);
	if (!bOk) return false;

	bOk = pReader_->readMetaData();
	if (bOk)
	{
		bOk = pReader_->readPreview();
	}
	else
	{
		error_ = DRW::Error::BAD_READ_METADATA;
	}

	filestr.close();
	if (pReader_ != nullptr)
	{
		delete pReader_;
		pReader_ = nullptr;
	}
	return bOk;
}

/**
 * start reading dwg file header and,if can read it, continue reading all
 * */
bool DwgR::read(DRW_Interface* interface_, bool ext) {
	bool bOk = false;
	applyExt = ext;
	m_pIface = interface_;
	std::ifstream fileStream;

	bOk = openFile(&fileStream);
	if (!bOk)
		return false;

	bOk = pReader_->readMetaData();
	if (bOk)
	{
		bOk = pReader_->readFileHeader();
		if (bOk)
			bOk = processDwg();
		else
		{
			error_ = DRW::Error::BAD_READ_FILE_HEADER;
			spdlog::get(this->getFilePath())->error("read file header error");
		}
	}
	else
	{
		error_ = DRW::Error::BAD_READ_METADATA;
		spdlog::get(this->getFilePath())->error("read meta data error");
	}

	fileStream.close();
	//delete pReader_;
	//pReader_ = nullptr;
	return bOk;
}

bool DwgR::createLogger() const
{
	const std::string pattern = "[%d/%m/%Y %H:%M:%S][%=l] %v";
	const auto found = this->filePath_.find_last_of('.');
	auto fileNoExt = this->filePath_.substr(0, found);
	auto dxf = spdlog::basic_logger_mt(filePath_, fileNoExt.append(".log"), true);
	dxf->set_pattern(pattern);
	return true;
}

/* Open the file and stores it in filestr, install the correct reader version.
 * If fail opening file, error are set as DRW::BAD_OPEN
 * If not are DWG or are unsupported version, error are set as DRW::BAD_VERSION
 * and closes filestr.
 * Return true on succeed or false on fail
*/
bool DwgR::openFile(std::ifstream* _pFileStream) {
	bool bOk = false;
	DRW_DBG("dwgR::read 1\n");
	_pFileStream->open(filePath_.c_str(), std::ios_base::in | std::ios::binary);
	if (!_pFileStream->is_open() || !_pFileStream->good())
	{
		error_ = DRW::Error::BAD_OPEN;
		return bOk;
	}


	char szVersion[7]{};
	_pFileStream->read(szVersion, 6);
	szVersion[6] = '\0';
	DRW_DBG("dwgR::read line version: ");
	DRW_DBG(szVersion);
	DRW_DBG("\n");

	duint8 nVerFlag = szVersion[4] - 0x30;
	nVerFlag = nVerFlag * 10 + (szVersion[5] - 0x30);

	switch (nVerFlag)
	{
		case 6:
			version_ = DRW::R13;
			break;
		case 9:
			version_ = DRW::R14;
			break;
		case 15:
			version_ = DRW::R2000;
			pReader_ = new DwgReader15(_pFileStream, this);
			break;
		case 18:
			version_ = DRW::R2004;
			pReader_ = new DwgReader18(_pFileStream, this);
			break;
		case 21:
			version_ = DRW::R2007;
			pReader_ = new DwgReader21(_pFileStream, this);
			break;
		case 24:
			version_ = DRW::R2010;
			pReader_ = new DwgReader24(_pFileStream, this);
			break;
		case 27:
			version_ = DRW::R2013;
			pReader_ = new DwgReader27(_pFileStream, this);
			break;
		case 32:
			version_ = DRW::R2018;
			pReader_ = new DwgReader32(_pFileStream, this);
			break;
		default:
			version_ = DRW::UNKNOWN_VERSION;
			break;
	}

	if (pReader_ == NULL || version_ == DRW::UNKNOWN_VERSION)
	{
		bOk = false;
		error_ = DRW::Error::BAD_VERSION;
		_pFileStream->close();
	}
	else if (version_ <= DRW::R14)
	{
		bOk = false;
		error_ = DRW::Error::BAD_NONE;
		_pFileStream->close();
	}
	else
	{
		bOk = true;
	}
	return bOk;
}

bool DwgR::processDwg() {
	DRW_DBG("dwgR::processDwg() start processing dwg\n");
	DRW_Header hdr;
	bool ret = pReader_->readDwgHeader(hdr);
	if (!ret)
	{
		spdlog::get(filePath_)->error("an error appeared on reading dwg header");
		error_ = DRW::Error::BAD_READ_HEADER;
	}

	bool ret2 = pReader_->readDwgClasses();
	if (ret && !ret2)
	{
		spdlog::get(filePath_)->error("an error appeared on reading dwg classes");
		error_ = DRW::Error::BAD_READ_CLASSES;
		ret = ret2;
	}

	ret2 = pReader_->readDwgHandles();
	if (ret && !ret2)
	{
		spdlog::get(this->filePath_)->error("an error appeared on reading dwg handles");
		error_ = DRW::Error::BAD_READ_HANDLES;
		ret = ret2;
	}

	ret2 = pReader_->readDwgTables(hdr);
	if (ret && !ret2)
	{
		spdlog::get(this->filePath_)->error("an error appeared on reading dwg tables");
		error_ = DRW::Error::BAD_READ_TABLES;
		ret = ret2;
	}

	m_pIface->addHeader(&hdr);

	for (auto& it : pReader_->lineTypeMap)
	{
		m_pIface->addLType(it.second);
	}

	for (auto& it : pReader_->layerMap)
	{
		m_pIface->addLayer(it.second);
	}

	for (auto& it : pReader_->styleMap)
	{
		m_pIface->addTextStyle(it.second);
	}

	for (auto& it : pReader_->dimstyleMap)
	{
		m_pIface->addDimStyle(it.second);
	}

	for (auto& it : pReader_->vportMap)
	{
		m_pIface->addVport(it.second);
	}

	for (auto& it : pReader_->appIdMap)
	{
		m_pIface->addAppId(it.second);
	}
	
	ret2 = pReader_->readDwgBlocks(*m_pIface);
	if (ret && !ret2)
	{
		spdlog::get(this->filePath_)->error("an error appeared on reading dwg blocks");
		error_ = DRW::Error::BAD_READ_BLOCKS;
		ret = ret2;
	}

	ret2 = pReader_->readDwgEntities(*m_pIface);
	if (ret && !ret2)
	{
		spdlog::get(this->filePath_)->error("an error appeared on reading dwg entities");
		error_ = DRW::Error::BAD_READ_ENTITIES;
		ret = ret2;
	}

	ret2 = pReader_->readDwgObjects(*m_pIface);
	if (ret && !ret2)
	{
		spdlog::get(this->filePath_)->error("an error appeared on reading dwg objects");
		error_ = DRW::Error::BAD_READ_OBJECTS;
		ret = ret2;
	}

	if (!ret)
		spdlog::get(this->filePath_)->error("error occurred as {}", error_);
	else
		spdlog::get(this->filePath_)->info("not found errors on processing dwg file");

	return ret;
}