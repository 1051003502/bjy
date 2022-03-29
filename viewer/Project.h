#pragma once
#include <windows.h>
#include "transer.h"
#include "parser.h"
#include "utils/pillar.h"
#include <fstream>
typedef std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>> ProjectFile;
typedef std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>> SingleDrawingFIle;
typedef std::pair<std::string, Pillar> PillarInformation;
typedef std::pair<Pillar, std::string>PillarBlockInformation;


class CProject 
{
public:
	CProject();
	void SetProjectFile(LPCTSTR FileName, shared_ptr<Transer>, shared_ptr<Parser>);
	std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator 
		ReturnFindFIle(const std::string& file);
	void FilesErase(const std::string& file);
	int FIlesSize();
	std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator  FilesBegin();
	std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator  FilesEnd();
	void BuildCoordinateSystem();
	std::vector<Axis::AxisLineCoordinate>::iterator MyFind(std::vector<Axis::AxisLineCoordinate>::iterator begin,
		std::vector<Axis::AxisLineCoordinate>::iterator end, 
		Axis::AxisLineCoordinate axis);
	void GitPillarInformation();
public:
	void AxisCompare( std::vector<Axis::AxisLineCoordinate> &axis1, int left, int right);
protected:

	std::vector<PillarInformation>::iterator FindPillar(std::vector<PillarInformation>::iterator ite1,
		std::vector<PillarInformation>::iterator ite2, const Pillar &pillar);
	std::vector<std::shared_ptr<Pillar>>::iterator FindPillar(std::vector<std::shared_ptr<Pillar>>::iterator ite1,
		std::vector<std::shared_ptr<Pillar>>::iterator ite2, const Pillar &pillar);
private:
	ProjectFile files;
	
public:
	Axis g_c_system;			//global_coordinate_system ºÚ–¥ g_c_system
	std::vector<PillarInformation> g_p_information;		//global_pillar_information ºÚ–¥ g_p_information
};