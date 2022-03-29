#include "../drawingAssociation/src/dataIntegration.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>
#if __linux__
#include <unistd.h>
#else
#include<io.h>
#endif

void initLogger()
{
	using namespace spdlog;

	const std::string formate1 = "[%d/%m/%Y %H:%M:%S][%=n][%=l] %v";

	spdlog::init_thread_pool(10000, 1);

	//sinks
	std::vector<sink_ptr> sinkVec;
	std::vector<std::shared_ptr<logger>> loggers;


	auto allSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(".log/all.log", true);
	allSink->set_pattern(formate1);
	allSink->set_level(level::trace);
	auto all = std::make_shared<spdlog::logger>("all", allSink);
	loggers.push_back(all);


	//loggers
	auto dxfrwResult = spdlog::basic_logger_mt<spdlog::async_factory>("dxfrwResult", ".log/dxfrwResult.log");
	dxfrwResult->set_level(level::info);
	dxfrwResult->set_pattern(formate1);

	for (const auto& it : loggers)
	{
		spdlog::register_logger(it);
	}
	DRW_dbg::getInstance()->setLevel(DRW_dbg::NONE);
}



bool quit;

Transer transer;
Parser parser;
char* input = NULL;
char* output = NULL;
slot s1;
slot s2;
slot s3;

void axisInfomation();
void textInformation();
void parsed();
void failed() {
	quit = true;
}
class WallTest
{
public:
	WallTest(char* input = NULL, char* output = NULL)
	{
		this->input = input;
		this->output = output;
	}
	WallTest(int argc, char* argv[])
	{
		bool FaXing = true;
		if (FaXing)
		{
			if (argc < 2)
			{
				printf("No drawing path parameter\n");
				return;
			}
			if (argc > 2)

			{
				printf("parameter too much , please check spaces in the file name!");
				return;
			}
			printf("parameter correcting");
			char* cmdInput = argv[1];
			input = cmdInput;
		}
		else
		{
			//input = "C:\\Users\\PC\\Desktop\\board\\板测试图纸\\嵩山\\平面图\\底板配筋平面图(一).dwg";
			input = "F:\\板测试图纸 - 板测试图纸 - 板测试图纸 - 板测试图纸 - 板测试图纸\\淮河路\\ZG5HX - S - 04 - CZ29 - 02JG - 02 - 31底板配筋图（一）20151118.dwg.dwg";

		}
	}
	void testingProgramme(std::string testType, void (WallTest::* pf)())
	{
		if (input == NULL || input == "")
		{
			std::cout << "未输入文件名" << std::endl;
			return;
		}
		//std::cout <<"解析->"<< input << std::endl;
		initLogger();
		quit = false;
		s1 = transer.sigFailed.connect(std::bind(&WallTest::failed, this));
		s2 = transer.sigReaded.connect(std::bind(&WallTest::parserParse, this, testType));
		s3 = parser.sigParsed.connect(std::bind(pf, this));
		transer.fileImport(input);
		do
		{
			Sleep(10);
		} while (!quit);
		cout << "over";
	}
	//平面图测试
	void locationTest()
	{
		testingProgramme("location", &WallTest::printWallInformation);
	}
	//横剖配筋图测试
	void hProfileTest()
	{
		testingProgramme("profile", &WallTest::printWallHProfileInformation);
	}
	//端墙测试
	void endProfileTest()
	{
		testingProgramme("profile", &WallTest::printWallEndProfile);
	}

private:
	bool quit;
	Transer transer;
	Parser parser;
	char* input;
	char* output;
	slot s1;
	slot s2;
	slot s3;
private:
	void printWallInformation()
	{
		std::fstream of("wallLocation.json", std::ios::out);
		Json::FastWriter fastWriter;
		auto jsonData = parser.serializeWallLocation();
		of << fastWriter.write(jsonData);
		of.close();
		quit = true;
	}
	void printWallHProfileInformation()
	{
		/*std::fstream of("横剖配筋.json", std::ios::out);
		Json::FastWriter fastWriter;
		auto jsonData = parser.serializeWallHProfile();
		of << fastWriter.write(jsonData);
		of.close();*/
		quit = true;
	}
	void printWallEndProfile() { quit = true; }
	void failed() {
		quit = true;
	}
	void parserParse(std::string dataType) {
		cout << "parser.parse" << endl;
		parser.parse_testing_programme(transer.getData(), dataType);
	}
};
void testFunction_checkCrossingAndGetCorner()
{
	Point p00(10, 10);
	Point p01(10, 100);
	Point p10(100, 0);
	Point p11(100, 100);
	Line line1(Point(-10000, -100), Point(10000, -100));
	Line line2(Point(-100, -10000), Point(-100, 10000));

	Line line3(Point(0, -100), Point(100, 0));
	Line line4(Point(103, -10000), Point(103, 10000));
	Corner corner1;
	Corner corner2;
	bool res1 = publicFunction::checkCrossingAndGetCorner(line3, line4, corner1);
	bool res2 = publicFunction::checkCrossingAndGetCorner(line4, line3, corner2);
	//bool res3=crossLineVH(left)
	std::cout << res1 << "  " << corner1.getShowString();
	std::cout << std::endl;
	std::cout << res2 << "  " << corner2.getShowString();
	std::cout << std::endl;
}
void testFunction_parallelTo()
{
	Line line1(Point(-10000, -100), Point(10000, -100));
	Line line2(Point(-100, -100), Point(99, -100));
	bool result1=line1.parallelTo(line2);
	Line line3(Point(-100, -100), Point(100, 102));
	Line line4(Point(0, -100), Point(200, 100));
	bool result2 = line3.parallelTo(line4);
	std::cout << result1 << std::endl;
	std::cout << result2 << std::endl;
}
int main(int argc, char* argv[]) {

	testFunction_parallelTo();
	/*WallTest walltest(argc, argv);
	walltest.hProfileTest();*/
	//walltest.locationTest();

}
