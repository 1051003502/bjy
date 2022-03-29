#include "drw_dbg.h"
#include <iomanip>
#include <iostream>

DRW_dbg* DRW_dbg::instance = nullptr;

/*********private clases*************/
class PrintCls
{
public:
	virtual void printS(const std::string& s) = 0;
	virtual void printI(long long int i) = 0;
	virtual void printUI(unsigned long long  int i) = 0;
	virtual void printD(double d) = 0;
	virtual void printH(long long int i) = 0;
	virtual void printB(int i) = 0;
	virtual void printHL(int c, int s, int h) = 0;
	virtual void printPT(double x, double y, double z) = 0;
	virtual ~PrintCls() {};
};

class print_none : public PrintCls
{
public:
	void printS(const std::string& s) override { (void)s; };
	void printI(long long int i) override { (void)i; };
	void printUI(unsigned long long  int i) override { (void)i; };
	void printD(double d) override { (void)d; };
	void printH(long long int i) override { (void)i; };
	void printB(int i) override { (void)i; };
	void printHL(int c, int s, int h) override {
		(void)c;
		(void)s;
		(void)h;
	};
	void printPT(double x, double y, double z) override {
		(void)x;
		(void)y;
		(void)z;
	};

public:
	print_none() {};
	virtual ~print_none() {}
};

class print_debug : public PrintCls
{
public:
	void printS(const std::string& s) override;
	void printI(long long int i) override;
	void printUI(unsigned long long  int i) override;
	void printD(double d) override;
	void printH(long long int i) override;
	void printB(int i) override;
	void printHL(int c, int s, int h) override;
	void printPT(double x, double y, double z) override;
	print_debug();
	virtual ~print_debug() {}

private:
	std::ios_base::fmtflags flags;
};

/**
*file debug
*/
class print_file : public PrintCls
{
public:
	print_file() {
		fileName_ = "read.log";
		pOfstream_ = nullptr;
		fmtFlags_ = std::ios_base::fmtflags(0);
		if (fileName_.length() > 0)
			setOutFile(fileName_);
	}
	print_file(std::string __fileName) {
		pOfstream_ = nullptr;
		fmtFlags_ = std::ios_base::fmtflags(0);
		if (__fileName.length() > 0)
		{
			setOutFile(__fileName);
		}
	}
	virtual ~print_file() {
		if (pOfstream_)
		{
			this->closeFileStream();
			delete pOfstream_;
		}
		pOfstream_ = nullptr;
	}
	void printS(const std::string& s) override;
	void printI(long long int i) override;
	void printUI(unsigned long long  int i) override;
	void printD(double d) override;
	void printH(long long int i) override;
	void printB(int i) override;
	void printHL(int c, int s, int h) override;
	void printPT(double x, double y, double z) override;
	// set print file
	bool setOutFile(std::string _fileName) {
		this->closeFileStream();
		pOfstream_ = new std::ofstream;
		fileName_.assign(_fileName);
		this->openFileStream();
		fmtFlags_ = pOfstream_->flags();
		return true;
	}
	bool openFileStream() {
		if (!pOfstream_->is_open())
		{
			pOfstream_->open(fileName_, std::ios::app);
		}
		return true;
	}
	bool closeFileStream() {
		if (pOfstream_ && pOfstream_->is_open())
		{
			pOfstream_->close();
		}
		return true;
	}

	std::string getPrintFileName() { return this->fileName_; }

private:
	std::string fileName_;
	std::ofstream* pOfstream_;
	// store the origin fmt flag
	std::ios_base::fmtflags fmtFlags_;
};

/********* debug class *************/
DRW_dbg* DRW_dbg::getInstance() {
	if (instance == nullptr)
	{
		instance = new DRW_dbg;
	}
	return instance;
}

DRW_dbg::DRW_dbg() {
	level = LEVEL::DEBUGG;
	prClass = new print_debug();
	flags = std::cerr.flags();
}

void DRW_dbg::deleteDRW_dbg() {
	if (prClass != nullptr)
	{
		delete prClass;
		prClass = nullptr;
	}
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void DRW_dbg::setLevel(LEVEL lvl, std::string _fileName) {
	level = lvl;
	if (prClass)
	{
		delete prClass;
		prClass = nullptr;
	}
	switch (level)
	{
		case NONE:
			prClass = new print_none();
			break;
		case FILE:
			prClass = new print_file(_fileName);
			break;
		case DEBUGG:
			prClass = new print_debug();
			break;
		default:
			break;
	}
}

DRW_dbg::LEVEL DRW_dbg::getLevel() {
	return level;
}

void DRW_dbg::print(const std::string s) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printS(s);
}

void DRW_dbg::print(int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printI(i);
}

void DRW_dbg::print(unsigned int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printUI(i);
}

void DRW_dbg::print(long long int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printI(i);
}

void DRW_dbg::print(long unsigned int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printUI(i);
}

void DRW_dbg::print(unsigned long long  int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printUI(i);
}

void DRW_dbg::print(double d) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printD(d);
}

void DRW_dbg::printH(long long int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printH(i);
}

void DRW_dbg::printB(int i) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printB(i);
}

void DRW_dbg::printHL(int c, int s, int h) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printHL(c, s, h);
}

void DRW_dbg::printPT(double x, double y, double z) {
	std::unique_lock<std::mutex> lk(this->mtx);
	prClass->printPT(x, y, z);
}

print_debug::print_debug() {
	flags = std::cerr.flags();
}

void print_debug::printS(const std::string& s) {
	std::cerr << s;
}

void print_debug::printI(long long int i) {
	std::cerr << i;
}

void print_debug::printUI(unsigned long long  int i) {
	std::cerr << i;
}

void print_debug::printD(double d) {
	std::cerr << std::fixed << d;
}

void print_debug::printH(long long int i) {
	std::cerr << "0x" << std::setw(2) << std::setfill('0');
	std::cerr << std::hex << i;
	std::cerr.flags(flags);
}

void print_debug::printB(int i) {
	std::cerr << std::setw(8) << std::setfill('0');
	std::cerr << std::setbase(2) << i;
	std::cerr.flags(flags);
}

void print_debug::printHL(int c, int s, int h) {
	std::cerr << c << '.' << s << '.';
	std::cerr << "0x" << std::setw(2) << std::setfill('0');
	std::cerr << std::hex << h;
	std::cerr.flags(flags);
}

void print_debug::printPT(double x, double y, double z) {
	std::cerr << std::fixed << "x: " << x << ", y: " << y << ", z: " << z;
}

/*    print file        */

// print string to file
void print_file::printS(const std::string& s) {
	*pOfstream_ << s;
	this->pOfstream_->flush();
}

void print_file::printI(long long int i) {
	*pOfstream_ << i;
	this->pOfstream_->flush();
}

void print_file::printUI(unsigned long long  int i) {
	*pOfstream_ << i;
	this->pOfstream_->flush();
}

void print_file::printD(double d) {
	*pOfstream_ << d;
	this->pOfstream_->flush();
}

void print_file::printH(long long int i) {
	*pOfstream_ << "0x" << std::setw(2) << std::setfill('0');
	*pOfstream_ << std::hex << i;
	pOfstream_->setf(fmtFlags_);
	this->pOfstream_->flush();
}

void print_file::printB(int i) {
	*pOfstream_ << std::setw(8) << std::setfill('0');
	*pOfstream_ << std::setbase(2) << i;
	pOfstream_->setf(fmtFlags_);
	this->pOfstream_->flush();
}

void print_file::printHL(int c, int s, int h) {
	*pOfstream_ << c << '.' << s << '.';
	*pOfstream_ << "0x" << std::setw(2) << std::setfill('0');
	*pOfstream_ << std::hex << h;
	pOfstream_->setf(fmtFlags_);
	this->pOfstream_->flush();
}

void print_file::printPT(double x, double y, double z) {
	*pOfstream_ << std::fixed << "x: " << x << ", y: " << y << ", z: " << z;
	this->pOfstream_->flush();
}
