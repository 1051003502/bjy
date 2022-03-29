#include "Project.h"

CProject::CProject()
{
}

void CProject::SetProjectFile(LPCTSTR FileName, shared_ptr<Transer>transer, shared_ptr<Parser>parser)
{
	files[FileName] = std::make_pair(transer, parser);
}

std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator CProject::ReturnFindFIle(const std::string & file)
{
	
	return files.find(file);
}



void CProject::FilesErase(const std::string & file)
{
	files.erase(file);
}

int CProject::FIlesSize()
{
	return files.size();
}

std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator CProject::FilesBegin()
{
	return files.begin();
}

std::map<std::string, std::pair<std::shared_ptr<Transer>, std::shared_ptr<Parser>>>::iterator CProject::FilesEnd()
{
	return files.end();
}

void CProject::BuildCoordinateSystem()
{
	
	//图纸层
	for (auto it=files.begin();it!=files.end();++it)
	{

		//轴网层
		for (int i = 0; i < it->second.second->_axises.size(); ++i)
		{
			//临时缓存参照轴
			Axis temp_axis;

			//纵轴 假设一个建筑不存在完全同的两个轴网
			if (0 == g_c_system.vlines_coo.size())
			{
				g_c_system.vlines_coo.insert(g_c_system.vlines_coo.begin(),
					it->second.second->_axises[i]->vlines_coo.begin(), 
					it->second.second->_axises[i]->vlines_coo.end());
			}
			else
			{
				
				//寻找参照纵轴
				for (auto axis : it->second.second->_axises[i]->vlines_coo)
				{
					//判断当前轴在轴网中是否存在
					auto temp_v = MyFind(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), axis);

					if (temp_v != g_c_system.vlines_coo.end())
					{
						temp_axis.vlines_coo.push_back(axis);
						break;
					}
					else
					{
						//此处有可能会遇到，需要特殊处理
						;
					}
				}
				for (auto axis : it->second.second->_axises[i]->vlines_coo)
				{
					//判断当前轴在轴网中是否存在
					auto temp_v = MyFind(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), axis);
					//如果不存在加入，存在则跳过
					if (temp_v == g_c_system.vlines_coo.end())
					{
						
						g_c_system.vlines_coo.push_back(axis);
						auto temp_vc_pre = MyFind(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), temp_axis.vlines_coo[0]);
						g_c_system.vlines_coo.back().second = (*temp_vc_pre).second + (axis.second - temp_axis.vlines_coo[0].second);
					}
					else
					{
						continue;
					}
					
				}
			}
			//横轴
			if (0 == g_c_system.hlines_coo.size())
			{
				g_c_system.hlines_coo.insert(g_c_system.hlines_coo.begin(),
					it->second.second->_axises[i]->hlines_coo.begin(),
					it->second.second->_axises[i]->hlines_coo.end());
			}
			else
			{
				
				//寻找参照横轴
				for (auto axis : it->second.second->_axises[i]->hlines_coo)
				{
					//判断当前轴在轴网中是否存在
					auto temp_h = MyFind(g_c_system.hlines_coo.begin(), g_c_system.hlines_coo.end(), axis);
					if (temp_h != g_c_system.hlines_coo.end())
					{
						temp_axis.hlines_coo.push_back(axis);
						break;
					}
					else
					{
						//此处有可能会遇到，需要特殊处理
						;
					}
				}
				for (auto axis : it->second.second->_axises[i]->hlines_coo)
				{
					auto temp_h = MyFind(g_c_system.hlines_coo.begin(), g_c_system.hlines_coo.end(), axis);
					if (temp_h != g_c_system.hlines_coo.end())
					{
						g_c_system.hlines_coo.push_back(axis);
						auto temp_vc_pre = MyFind(g_c_system.hlines_coo.begin(), g_c_system.hlines_coo.end(), temp_axis.hlines_coo[0]);
						g_c_system.hlines_coo.back().second = (*temp_vc_pre).second + (axis.second - temp_axis.hlines_coo[0].second);
					}
					else
					{
						continue;
					}
				}
				
			}
		}
	}
	
	//sort(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), AxisCompare);
	//sort(g_c_system.hlines_coo.begin(), g_c_system.hlines_coo.end(), AxisCompare);
	AxisCompare(g_c_system.vlines_coo, 0, g_c_system.vlines_coo.size());
	AxisCompare(g_c_system.hlines_coo, 0, g_c_system.hlines_coo.size());

	std::fstream of("axisOut.txt", std::ios::out);
	of << "整体坐标系横轴：" << std::endl;
	for (int i = 0; i < g_c_system.hlines_coo.size(); ++i)
	{
		of << "名字：" << g_c_system.hlines_coo[i].first.first.second << "\t" << "距离：" << g_c_system.hlines_coo[i].second << std::endl;
	}
	of << "整体坐标系纵轴：" << std::endl;
	for (int i = 0; i < g_c_system.vlines_coo.size(); ++i)
	{
		of << "名字：" << g_c_system.vlines_coo[i].first.first.second << "\t" << "距离：" << g_c_system.vlines_coo[i].second << std::endl;
	}
	of.close(); 
}

std::vector<Axis::AxisLineCoordinate>::iterator CProject::MyFind(std::vector<Axis::AxisLineCoordinate>::iterator begin,
	std::vector<Axis::AxisLineCoordinate>::iterator end, 
	Axis::AxisLineCoordinate axis)
{
	for (auto it = begin; it != end; ++it)
	{
		if (it->first.first.second == axis.first.first.second)
		{
			return it;
		}
	}
	return end;
}

//多图纸查聚合柱信息
void CProject::GitPillarInformation()
{
	
	for (auto file1 : files)
	{

		auto pillars = file1.second.second->_pillars;
		
		for (auto temp_pill : pillars)
		{
			//图纸内轴网信息的连立
			//bool mark_last = false;
			auto it_pill1 = FindPillar(g_p_information.begin(), g_p_information.end(), *temp_pill);
			if (it_pill1 == g_p_information.end())
			{
				//mark_last = true;
				g_p_information.push_back(PillarInformation(file1.first,*temp_pill));
				for (auto file2 : files)
				{
					std::string pillar_name;
					auto last1 = temp_pill->name.find("|"); 
					int last2 = -1;
					last2 = temp_pill->name.find("(");
					if (last2 == std::string::npos)
					{
						//特定编码处理
						last2 = temp_pill->name.find("锛");
					}
						
					if (last1 != std::string::npos)
					{
						if (last2 != std::string::npos&&last1 - last2>0)
						{
							pillar_name = temp_pill->name.substr(0, last2);
						}
						else
						{
							pillar_name = temp_pill->name.substr(0, last1);
						}
					}
					else if (last1 == std::string::npos)
					{
						if (last2 != std::string::npos)
						{
							pillar_name = temp_pill->name.substr(0, last2);
						}
						else
						{
							pillar_name = temp_pill->name;
						}
					}
					auto blocks = file2.second.second->_blocks;
					for (auto it : blocks)
					{
						if (it->name.find(pillar_name) != std::string::npos)
						{
							//std::string temp_str = "文件：" + file2.first + "\t" + "块：" + it->name;
							//temp_pill->other_information.push_back(temp_str);
							temp_pill->other_information = it->block_information;
				
							//g_p_information.back().second.other_information.push_back(temp_str);
							g_p_information.back().second.other_information = it->block_information;
							
						}
					}
				}


				for (auto file2 : files)
				{
					if (file2 != file1)
					{
						//此基础上用整体坐标系做差应该是查找会是最快的，需要修改FindPillar函数
						//在文件中找柱子，找到就添加，否则跳过；
						auto it_pill2 = FindPillar(file2.second.second->_pillars.begin(), file2.second.second->_pillars.end(), *temp_pill);
						if (it_pill2 != file2.second.second->_pillars.end())
						{

							g_p_information.push_back(PillarInformation(file2.first,*temp_pill));
						}
					}
				}
			}

		}
		
	}
	
	std::fstream of("connectPoill.txt", std::ios::out);
	for (auto it : g_p_information)
	{
		
		if (it.second.error)
		{
			of << "柱：" << it.first << "\t" << it.second.name << "\t" << ":存在问题" << "\t";
		}
		else
		{
			of << "柱：" << it.first << "\t" << it.second.name << "\t" << "横轴：" << it.second.hdistance.first << "\t"
				<< it.second.hdistance.second.first << "\t" << it.second.hdistance.second.second
				<< "纵轴：" << it.second.vdistance.first << "\t" << it.second.vdistance.second.first << "\t"
				<< it.second.vdistance.second.second << "\t";
		}
		of << "相关联信息：";
		for (auto it1 : it.second.other_information)
		{
			of << it1 << "\t";
		}
		of << std::endl;
		
	}
	of.close();


	/*std::fstream of("PillarInformation.txt", std::ios::out);
	for (auto it : g_p_information)
	{
		
		of << "图纸路径：" << it.first << "\t";
		if (it.second.error)
		{
			of << "此图纸中柱：" << it.second.name << "可能存在问题" << "\t";
		}
		of << "柱名：" << it.second.name << "\t";
		of << "横轴名：" << it.second.hdistance.first << "\t" << "底横轴距：" << it.second.hdistance.second.first << "\t" 
			<< "顶横轴距：" << it.second.hdistance.second.second << "\t";
		of << "纵轴名" << it.second.vdistance.first << "左纵轴距：" << it.second.vdistance.second.first << "\t" 
			<< "右纵轴距：" << it.second.vdistance.second.second << std::endl;
	}
	of.close();*/

	
}

void CProject::AxisCompare(std::vector<Axis::AxisLineCoordinate> &axis, int left, int right)
{
	if (left < right)
	{
		int i = left, j = right - 1;
		Axis::AxisLineCoordinate x = axis[left];
		while (i < j)
		{
			while (i < j&&axis[j].second >= x.second)
			{
				j--;
			}
			if (i < j)
			{
				axis[i++] = axis[j];
			}
			while (i < j&&axis[i].second < x.second)
				i++;
			if (i < j)
			{
				axis[j--] = axis[i];
			}
		}
		axis[i] = x;
		AxisCompare(axis, left, i - 1);
		AxisCompare(axis, i + 1, right);

	}
}

std::vector<PillarInformation>::iterator CProject::FindPillar(std::vector<PillarInformation>::iterator ite1, std::vector<PillarInformation>::iterator ite2,
	const Pillar &pillar)
{
	for (auto it = ite1; it != ite2; ++it)
	{
		if (IsItTheSamePillar((*it).second, pillar))
			return it;
	}
	return ite2;
}

std::vector<std::shared_ptr<Pillar>>::iterator CProject::FindPillar(std::vector<std::shared_ptr<Pillar>>::iterator ite1, 
	std::vector<std::shared_ptr<Pillar>>::iterator ite2, const Pillar &pillar)
{
	for (auto it = ite1; it != ite2; ++it)
	{
		if (IsItTheSamePillar(**it, pillar))
			return it;
	}
	return ite2;
}





