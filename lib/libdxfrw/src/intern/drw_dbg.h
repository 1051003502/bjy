#ifndef DRW_DBG_H
#define DRW_DBG_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "../drw_base.h"

#define DRW_DBGSL(a) DRW_dbg::getInstance()->setLevel(a)
#define DRW_DBGGL DRW_dbg::getInstance()->getLevel()
#define DRW_DBG(a) DRW_dbg::getInstance()->print(a)
#define DRW_DBGH(a) DRW_dbg::getInstance()->printH(a)
#define DRW_DBGB(a) DRW_dbg::getInstance()->printB(a)
#define DRW_DBGHL(a, b, c) DRW_dbg::getInstance()->printHL(a, b, c)
#define DRW_DBGPT(a, b, c) DRW_dbg::getInstance()->printPT(a, b, c)
#define DRW_NONE DRW_dbg::getInstance()->setLevel(DRW_dbg::NONE)



class PrintCls;

class DRW_dbg {
public:
    using LEVEL = enum EmLevel {
        NONE,
        DEBUGG,
        FILE,
    };
    void setLevel(LEVEL lvl, std::string _fileName = "temp.log");
    LEVEL getLevel();
    static DRW_dbg* getInstance();
    void print(const std::string s);
    void print(int i);
    void print(unsigned int i);
    void print(long long int i);
    void print(long unsigned int i);
    void print(unsigned long long  int i);
    void print(double d);
    void printH(long long int i);
    void printB(int i);
    void printHL(int c, int s, int h);
    void printPT(double x, double y, double z);
    void deleteDRW_dbg();

private:
    DRW_dbg();
    static DRW_dbg* instance;
    LEVEL level;
    mutable std::mutex mtx;
    std::ios_base::fmtflags flags;
    PrintCls* prClass;
};

#endif  // DRW_DBG_H
