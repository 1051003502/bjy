#ifndef __PARSER_UTILS_COMMON_H__
#define __PARSER_UTILS_COMMON_H__

#include <memory>
#include <algorithm>
#include <thread>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <random>
#include <cmath>
#include <math.h>
#include <array>
#include <map>
#include <limits>

#if __linux__
#include <unistd.h>
#endif

#include "json/json.h"

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/spdlog.h"



#include "cereal/archives/json.hpp"
#include <cereal/archives/xml.hpp>
#include <cereal/archives/binary.hpp>

#if __linux__
#define Sleep(a) sleep(a)
#elif _WIN32
#define Sleep(a) _sleep(a) 
#endif

#define Precision (1.101)
#define PrecisionAngle 0.01
#define MIN_TABLE_LINES 6
#define TableHeightPrecision 0.78

#ifndef ROUND
#define ROUND(d) std::floor(double( d + 0.5))//四舍五入宏定义
#endif

#if _WIN32
#pragma  warning (once :26812)
#pragma  warning (disable: 4996)
#endif

const std::string emptyStr = "";

using namespace spdlog;

#pragma region Redefine
#define pseudoDouble(v) std::llround((v)*100)

using uuid = unsigned long long;
#pragma endregion  Redefine


//#define LEAK_DETECTION new (_CLIENT_BLOCK, __FILE__, __LINE__)
//#define new LEAK_DETECTION
#endif
