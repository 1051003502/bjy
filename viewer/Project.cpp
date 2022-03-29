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
	
	//ͼֽ��
	for (auto it=files.begin();it!=files.end();++it)
	{

		//������
		for (int i = 0; i < it->second.second->_axises.size(); ++i)
		{
			//��ʱ���������
			Axis temp_axis;

			//���� ����һ��������������ȫͬ����������
			if (0 == g_c_system.vlines_coo.size())
			{
				g_c_system.vlines_coo.insert(g_c_system.vlines_coo.begin(),
					it->second.second->_axises[i]->vlines_coo.begin(), 
					it->second.second->_axises[i]->vlines_coo.end());
			}
			else
			{
				
				//Ѱ�Ҳ�������
				for (auto axis : it->second.second->_axises[i]->vlines_coo)
				{
					//�жϵ�ǰ�����������Ƿ����
					auto temp_v = MyFind(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), axis);

					if (temp_v != g_c_system.vlines_coo.end())
					{
						temp_axis.vlines_coo.push_back(axis);
						break;
					}
					else
					{
						//�˴��п��ܻ���������Ҫ���⴦��
						;
					}
				}
				for (auto axis : it->second.second->_axises[i]->vlines_coo)
				{
					//�жϵ�ǰ�����������Ƿ����
					auto temp_v = MyFind(g_c_system.vlines_coo.begin(), g_c_system.vlines_coo.end(), axis);
					//��������ڼ��룬����������
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
			//����
			if (0 == g_c_system.hlines_coo.size())
			{
				g_c_system.hlines_coo.insert(g_c_system.hlines_coo.begin(),
					it->second.second->_axises[i]->hlines_coo.begin(),
					it->second.second->_axises[i]->hlines_coo.end());
			}
			else
			{
				
				//Ѱ�Ҳ��պ���
				for (auto axis : it->second.second->_axises[i]->hlines_coo)
				{
					//�жϵ�ǰ�����������Ƿ����
					auto temp_h = MyFind(g_c_system.hlines_coo.begin(), g_c_system.hlines_coo.end(), axis);
					if (temp_h != g_c_system.hlines_coo.end())
					{
						temp_axis.hlines_coo.push_back(axis);
						break;
					}
					else
					{
						//�˴��п��ܻ���������Ҫ���⴦��
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
	of << "��������ϵ���᣺" << std::endl;
	for (int i = 0; i < g_c_system.hlines_coo.size(); ++i)
	{
		of << "���֣�" << g_c_system.hlines_coo[i].first.first.second << "\t" << "���룺" << g_c_system.hlines_coo[i].second << std::endl;
	}
	of << "��������ϵ���᣺" << std::endl;
	for (int i = 0; i < g_c_system.vlines_coo.size(); ++i)
	{
		of << "���֣�" << g_c_system.vlines_coo[i].first.first.second << "\t" << "���룺" << g_c_system.vlines_coo[i].second << std::endl;
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

//��ͼֽ��ۺ�����Ϣ
void CProject::GitPillarInformation()
{
	
	for (auto file1 : files)
	{

		auto pillars = file1.second.second->_pillars;
		
		for (auto temp_pill : pillars)
		{
			//ͼֽ��������Ϣ������
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
						//�ض����봦��
						last2 = temp_pill->name.find("�");
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
							//std::string temp_str = "�ļ���" + file2.first + "\t" + "�飺" + it->name;
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
						//�˻���������������ϵ����Ӧ���ǲ��һ������ģ���Ҫ�޸�FindPillar����
						//���ļ��������ӣ��ҵ�����ӣ�����������
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
			of << "����" << it.first << "\t" << it.second.name << "\t" << ":��������" << "\t";
		}
		else
		{
			of << "����" << it.first << "\t" << it.second.name << "\t" << "���᣺" << it.second.hdistance.first << "\t"
				<< it.second.hdistance.second.first << "\t" << it.second.hdistance.second.second
				<< "���᣺" << it.second.vdistance.first << "\t" << it.second.vdistance.second.first << "\t"
				<< it.second.vdistance.second.second << "\t";
		}
		of << "�������Ϣ��";
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
		
		of << "ͼֽ·����" << it.first << "\t";
		if (it.second.error)
		{
			of << "��ͼֽ������" << it.second.name << "���ܴ�������" << "\t";
		}
		of << "������" << it.second.name << "\t";
		of << "��������" << it.second.hdistance.first << "\t" << "�׺���ࣺ" << it.second.hdistance.second.first << "\t" 
			<< "������ࣺ" << it.second.hdistance.second.second << "\t";
		of << "������" << it.second.vdistance.first << "������ࣺ" << it.second.vdistance.second.first << "\t" 
			<< "������ࣺ" << it.second.vdistance.second.second << std::endl;
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





