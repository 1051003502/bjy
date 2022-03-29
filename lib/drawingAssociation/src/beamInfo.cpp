#include "beamInfo.h"

bool BeamInfo::iniAbBeamLoc(const std::map<std::string, std::shared_ptr<Axis>>& _axisMap,
							std::map<std::string, std::shared_ptr<Data>>& _dataMap)
{
	//std::map<std::string, std::vector<std::shared_ptr<Beam>>>columnLocationMap;//��ͼֽ����Ӧ����λ��
	//!��ʼ����λ����Ϣ�����ں���Ϊ����֧����׼��
	//_columnInfo.iniColumnLoc(_axisMap, _dataMap);

	//��ʼ��ƽ��ͼ������Ϣ
	for (auto it : _axisMap)
	{
		BeamSet tempBeamSet;
		tempBeamSet.setMember(it.second, _columnInfo._pillarVecMap[it.first]);
		tempBeamSet.findBeam(*_dataMap[it.first]/*, temp_index, _corners, temp_line, m_temp_index*/);
		tempBeamSet.beams(_drawing2BeamVecMap[it.first], *_dataMap[it.first]);
		
		/*_beamSet.setMember(it.second, _columnInfo._pillarVecMap[it.first]);
		_beamSet.findBeam(*_dataMap[it.first], temp_index, _corners, temp_line, m_temp_index);
		_beamSet.beams(_drawing2BeamVecMap[it.first], *_dataMap[it.first]);*/
	}

	//!���Ѿ�ʶ���������Ϣ������� this->beamVec ���ں�������
	iniAaBeam(_drawing2BeamVecMap);
	return true;
}

bool BeamInfo::iniBAssemblyAnnotation(const bool& _bAA)
{
	this->bAssemblyAnnotation = _bAA;
	return true;
}


bool BeamInfo::iniAaBeam(const std::map<std::string, std::vector<std::shared_ptr<Beam>>>& _BeamMap)
{
	for (auto it : _BeamMap)
	{
		for (const auto& beam : it.second)
		{
			spBeamVec[it.first].push_back(beam);
		}
	}
	return true;
}

bool BeamInfo::findBeamName()
{
	for (auto it : _drawing2BeamVecMap)
	{
		for (const auto& beam : it.second)
		{
			//�ռ����е�����
			if (std::find(_beamNameVec.begin(), _beamNameVec.end(), beam->strBeamName) == _beamNameVec.end())
			{
				_beamNameVec.push_back(beam->strBeamName);
			}
			//������ֵ�
			if (_beamNameVecMap.find(it.first) == _beamNameVecMap.end())
			{
				_beamNameVecMap[it.first].push_back(beam->strBeamName);
			}
			else if (std::find(_beamNameVecMap[it.first].begin(), _beamNameVecMap[it.first].end(),
							   beam->strBeamName) == _beamNameVecMap[it.first].end())
			{
				_beamNameVecMap[it.first].push_back(beam->strBeamName);
			}
		}
	}
	return false;
}

bool BeamInfo::bBeamSectionBlock(const std::shared_ptr<Block>& _block)
{
	//TODO ��Ҫ����ʵ��
	return true;
}

bool BeamInfo::findFrameBeamBlock(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
								  const std::vector<std::shared_ptr<Block>>& _blocks,
								  const std::map<std::string, int>& _blocksMap,
								  const std::map<std::string, std::vector<int>>& _blockFileMap)
{
	findBeamName();

	//!�����������ҿ����
	for (auto it : _beamNameVec)
	{
		auto goalSectionIte = _blocksMap.find(it);
		if (goalSectionIte == _blocksMap.end())
			continue;

		std::string fileStr;
		for (auto address : _blockFileMap)
		{
			if (std::find(address.second.begin(), address.second.end(), goalSectionIte->second) != address.second.end())
			{
				//data = _dataMap[address.first];
				fileStr = address.first;
				break;
			}
		}

		if (!fileStr.empty())
		{
			auto mark = bBeamSectionBlock(_blocks[goalSectionIte->second]);
			if (mark)
			{
				_BeamFrameBlockVec.emplace_back(_blocks[goalSectionIte->second], _dataMap[fileStr]);
			}
			else if (mark)
			{
				;//��¼��������������ͬ�����
			}

		}
	}

	return true;
}

bool BeamInfo::parserFrameBeam(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
							   const std::vector<std::shared_ptr<Block>>& _blocks,
							   const std::map<std::string, int>& _blocksMap,
							   const std::map<std::string, std::vector<int>>& _blockFileMap)
{
	//!��ʼ�����ݽ����Ϣ
	findFrameBeamBlock(_dataMap, _blocks, _blocksMap, _blockFileMap);
	//������<�����������data idx>
	std::map<std::string, std::vector<std::pair<std::shared_ptr<BeamLongitudinal>, int>>> beamName2FrameIdxMap;

	for (auto i = 0; i < _BeamFrameBlockVec.size(); i++)
	{
		const auto& frameBlock = _BeamFrameBlockVec.at(i);
		auto frame = _beamLongitudinalSet.beamLongitudinal(frameBlock.first,
														   frameBlock.second,
														   temp_index,
														   temp_line);
		if (frame)
		{
			//!��ʱֻ��������������
			beamName2FrameIdxMap[frame->spBlock->name].emplace_back(frame, i);
		}
	}

	spdlog::get("all")->info('\n');


	//��ֹ���������ط�����vector��
	std::vector<std::string> strVec;
	for (auto ite : spBeamVec)
	{
		for (auto j = 0; j < ite.second.size(); ++j)
		{
			auto spBeam = ite.second[j];
			spdlog::get("all")->info("install info to a new beam:{}", spBeam->strBeamName);
			//�ҵ������������װ������
			//�õ���ѡ�Ŀ����
			auto beamFrameIt = beamName2FrameIdxMap.find(spBeam->strBeamName);
			if (beamFrameIt == beamName2FrameIdxMap.end())
			{
				spdlog::get("all")->warn("no beam frame for this beam:{}\n", spBeam->strBeamName);
				continue;
			}

			spdlog::get("all")->info("it has columns for this beam:{}", spBeam->strBeamName);
			//�������ܵ������
			for (const auto& frameDataIdxPair : beamFrameIt->second)
			{
				std::shared_ptr<BeamLongitudinal> frame = frameDataIdxPair.first;
				std::shared_ptr<Data> data = _BeamFrameBlockVec.at(frameDataIdxPair.second).second;

				//ˮƽ����б����
				if (spBeam->direction != Beam::Direction::V)
				{
					//������
					if (frame->spBlock->bAxis)
					{
						spdlog::get("all")->info("this beam frame has axis:{}", frame->spBlock->name);
						spdlog::get("all")->info("match {} spans in beam", spBeam->spanNum);
						auto pillarVecIt = _columnInfo._pillarVecMap.find(ite.first);
						if (pillarVecIt != _columnInfo._pillarVecMap.end())
							frame->installFrameInfo(spBeam, data, pillarVecIt->second);
						else continue;
					}
					//��������
					else
					{

					}
				}
				//��ֱ��
				else
				{
					//����
				}
			}

			//�ڴ˳�ʼ�����Ľ������ƣ��Լ����Ľ����ֵ�
			for (auto beamSpanSection : spBeam->spSpanVec)
			{
				if (beamSpanSection->left.spBeamSection != nullptr &&
					beamSpanSection->left.spBeamSection->name != "" &&
					std::find(strVec.begin(), strVec.end(),
							  beamSpanSection->left.spBeamSection->name) == strVec.end())
				{

					std::string name = beamSpanSection->left.spBeamSection->name + "-"
						+ beamSpanSection->left.spBeamSection->name;
					strVec.push_back(beamSpanSection->left.spBeamSection->name);
					_beamSectionNameVec.push_back(name);
				}
				else if (beamSpanSection->right.spBeamSection != nullptr &&
						 beamSpanSection->right.spBeamSection->name != "" &&
						 std::find(strVec.begin(), strVec.end(),
								   beamSpanSection->right.spBeamSection->name) == strVec.end())
				{

					std::string name = beamSpanSection->right.spBeamSection->name + "-"
						+ beamSpanSection->right.spBeamSection->name;
					strVec.push_back(beamSpanSection->right.spBeamSection->name);
					_beamSectionNameVec.push_back(name);
				}
				else if (beamSpanSection->mid.spBeamSection != nullptr &&
						 beamSpanSection->mid.spBeamSection->name != "" &&
						 std::find(strVec.begin(), strVec.end(),
								   beamSpanSection->mid.spBeamSection->name) == strVec.end())
				{

					std::string name = beamSpanSection->mid.spBeamSection->name + "-"
						+ beamSpanSection->mid.spBeamSection->name;
					strVec.push_back(beamSpanSection->mid.spBeamSection->name);
					_beamSectionNameVec.push_back(name);
					//_beamSpanSectionInfoMap[ite.first][spBeam->strBeamName]

				}
			}
		}

	}

	return false;
}



bool BeamInfo::integrateBeamSectionName()
{
	for (auto it : spBeamVec)
	{
		//TODO ����ʵ��
		//_beamSectionNameVec.pushBack();
	}
	return true;
}

bool BeamInfo::findBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
								   const std::vector<std::shared_ptr<Block>>& _blocks,
								   const std::map<std::string, int>& _blocksMap,
								   const std::map<std::string, std::vector<int>>& _blockFileMap)
{
	//!�������ݽ��
	parserFrameBeam(_dataMap, _blocks, _blocksMap, _blockFileMap);

	//!�������Ķ��������Ҷ���
	for (auto it : _beamSectionNameVec)
	{
		auto goalSectionIte = _blocksMap.find(it);
		if (goalSectionIte == _blocksMap.end())
			continue;

		std::string fileStr;
		for (auto address : _blockFileMap)
		{
			if (std::find(address.second.begin(), address.second.end(), goalSectionIte->second) != address.second.end())
			{
				//data = _dataMap[address.first];
				fileStr = address.first;
				break;
			}
		}
		if (!fileStr.empty() && goalSectionIte != _blocksMap.end())
		{

			_beamSectionBlockVec.push_back(std::pair<std::shared_ptr<Block>, std::shared_ptr<Data>>(_blocks[goalSectionIte->second], _dataMap[fileStr]));

		}
	}

	return true;
}

bool BeamInfo::iniBeamSpanSection(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
								  const std::vector<std::shared_ptr<Block>>& _blocks,
								  const std::map<std::string, int>& _blocksMap,
								  const std::map<std::string, std::vector<int>>& _blockFileMap)
{
	//�����������
	findBeamSpanSection(_dataMap, _blocks, _blocksMap, _blockFileMap);
	//�����������
	for (auto it : _beamSectionBlockVec)
	{
		if (_beamSectionSet.bBeamSectionBlock(it.first, *it.second))
		{
			auto temp_block = BeamSection::retrunBeamSection(*it.first, *it.second);
			std::shared_ptr<BeamSection>pBeamSection(new BeamSection(temp_block));
			_beamSectionMap[it.first->name] = pBeamSection;
		}
	}

	//��ȫ������Ϣ
	//supplementaryBeamSectionInfo();
	return false;
}

bool BeamInfo::supplementaryBeamSectionInfo()
{
	for (auto ite : spBeamVec)
	{
		for (auto j = 0; j < ite.second.size(); ++j)
		{
			auto spBeam = ite.second[j];

			//�ڴ˳�ʼ�����Ľ������ƣ��Լ����Ľ����ֵ�
			for (auto beamSpanSection : spBeam->spSpanVec)
			{
				if (beamSpanSection->left.spBeamSection != nullptr)
				{
					std::string name = beamSpanSection->left.spBeamSection->name + "-"
						+ beamSpanSection->left.spBeamSection->name;
					auto spanSectionIte = _beamSectionMap.find(name);
					if (spanSectionIte != _beamSectionMap.end())
					{
						beamSpanSection->left.spBeamSection.reset();
						beamSpanSection->left.spBeamSection = spanSectionIte->second;
					}
				}

				if (beamSpanSection->right.spBeamSection != nullptr)
				{

					std::string name = beamSpanSection->right.spBeamSection->name + "-"
						+ beamSpanSection->right.spBeamSection->name;
					auto spanSectionIte = _beamSectionMap.find(name);
					if (spanSectionIte != _beamSectionMap.end())
					{
						beamSpanSection->right.spBeamSection.reset();
						beamSpanSection->right.spBeamSection = spanSectionIte->second;
					}
				}

				if (beamSpanSection->mid.spBeamSection != nullptr)
				{

					std::string name = beamSpanSection->mid.spBeamSection->name + "-"
						+ beamSpanSection->mid.spBeamSection->name;
					auto spanSectionIte = _beamSectionMap.find(name);
					if (spanSectionIte != _beamSectionMap.end())
					{
						beamSpanSection->mid.spBeamSection.reset();
						beamSpanSection->mid.spBeamSection = spanSectionIte->second;
					}

				}
			}
		}

	}
	return true;
}

//bool BeamInfo::pushBeamSpanSection()
//{
//	//������
//	for (auto ite : spBeamVec)
//	{
//		for (auto beam : ite.second)
//		{
//			//ÿ�����ж����
//			for (auto span : beam->spSpanVec)
//			{
//				//for (auto section : span->spBeamSectionVec)
//				//{
//				//	auto beamSectionIte = _beamSectionMap.find(section->name);
//				//	if (beamSectionIte != _beamSectionMap.end())
//				//	{
//				//		//pushBack//beamSection;
//				//	}
//				//}
//			}
//		}
//		
//	}
//	return true;
//}

BeamSectionInfo BeamInfo::returnBeamSectionInfo(const std::shared_ptr<BeamSection>& _beamSectionInfo)
{
	BeamSectionInfo sectionInfo;
	sectionInfo.name = _beamSectionInfo->name;
	sectionInfo._seale = _beamSectionInfo->seale;
	sectionInfo._height = _beamSectionInfo->maxHeight;
	sectionInfo._width = _beamSectionInfo->maxWidth;
	sectionInfo.style = _beamSectionInfo->sectionType;
	sectionInfo.referencePoint = _beamSectionInfo->referencePoint;
	//!ͳ�ƹ�������֧��
	std::vector<Line>hLinesVec;
	std::vector<Line>vLinesVec;
	std::vector<Line>sLinesVec;
	for (auto stir : _beamSectionInfo->beamStirrupVec)
	{
		for (auto line : stir.circle_line)
		{
			if (line.horizontal())
			{
				hLinesVec.push_back(line);
			}
			else if (line.vertical())
			{
				vLinesVec.push_back(line);
			}
			else
			{
				sLinesVec.push_back(line);
			}
		}
	}
	//!����
	LinesSort(hLinesVec, 'H');
	LinesSort(vLinesVec, 'V');
	//!�ϲ�
	MergeLines(hLinesVec, 'H');
	MergeLines(vLinesVec, 'V');

	sectionInfo._hsize = static_cast<int>(hLinesVec.size());
	sectionInfo._vSize = static_cast<int>(vLinesVec.size());
	//!��ʼ������ı�ע��Ϣ
	for (auto it : _beamSectionInfo->beamStirInfo)
	{
		if (it.bSingle)
		{
			sectionInfo._singleStirrupInfoVec.push_back(it.stirInfo);
		}
		else
		{
			sectionInfo._doubleStirrupInfoVec.push_back(it.stirInfo);
		}
	}
	//!��ʼ������list
	for (auto it : _beamSectionInfo->beamStirrupVec)
	{
		if (it.single)
		{
			sectionInfo._singleStirrupVec.push_back(it.circle_line);
		}
		else
		{
			sectionInfo._doubleStirrupVec.push_back(it.circle_line);
		}
	}
	//!��ʼ���ݽ���Ϣ
	for (auto it : _beamSectionInfo->beamBPinfoVec)
	{
		std::vector<std::string> infoVec;
		for (auto info : it.bpInfoVec)
		{
			infoVec.push_back(info);
		}
		if (it.dir == BeamBPInfo::Direction::LEFT)
		{
			sectionInfo._lonTen[BeamSectionInfo::LT::LEFT] = infoVec;
		}
		else if (it.dir == BeamBPInfo::Direction::RIGHT)
		{
			sectionInfo._lonTen[BeamSectionInfo::LT::RIGHT] = infoVec;
		}
		else if (it.dir == BeamBPInfo::Direction::TOP)
		{
			sectionInfo._lonTen[BeamSectionInfo::LT::TOP] = infoVec;
		}
		else if (it.dir == BeamBPInfo::Direction::BOTTOM)
		{
			sectionInfo._lonTen[BeamSectionInfo::LT::BOTTOM] = infoVec;
		}

	}
	return sectionInfo;
}

Json::Value BeamInfo::outputBeamSection(const std::shared_ptr<BeamSection>& _beamSection)
{
	//!��ʼ������ BeamSectionInfo����
	auto temp_beamSection = returnBeamSectionInfo(_beamSection);
	Json::Value information;
	information["name"] = temp_beamSection.name;
	information["scale"] = strimDouble(temp_beamSection._seale, 2);
	information["sectionSize"] = numToString(temp_beamSection._width) + "X" + numToString(temp_beamSection._height);

	//!��״����
	information["style"] = temp_beamSection.style;
	information["count"] = numToString(temp_beamSection._hsize) + "X" + numToString(temp_beamSection._vSize);
	//!�ݽ�������Ϣ
	Json::Value Bars;
	for (auto it : temp_beamSection._lonTen)
	{
		if (temp_beamSection._lonTen.find(BeamSectionInfo::LT::LEFT)
			!= temp_beamSection._lonTen.end())
		{
			Json::Value num;

			for (auto it1 : temp_beamSection._lonTen[BeamSectionInfo::LT::LEFT])
			{
				num.append(it1);

			}
			Bars["left"] = num;
		}
		if (temp_beamSection._lonTen.find(BeamSectionInfo::LT::RIGHT)
			!= temp_beamSection._lonTen.end())
		{
			Json::Value num;
			for (auto it1 : temp_beamSection._lonTen[BeamSectionInfo::LT::RIGHT])
			{
				num.append(it1);
			}
			Bars["right"] = num;
		}
		if (temp_beamSection._lonTen.find(BeamSectionInfo::LT::BOTTOM)
			!= temp_beamSection._lonTen.end())
		{
			Json::Value num;

			for (auto it1 : temp_beamSection._lonTen[BeamSectionInfo::LT::BOTTOM])
			{
				num.append(it1);
			}
			Bars["bottom"] = num;
		}
		if (temp_beamSection._lonTen.find(BeamSectionInfo::LT::TOP)
			!= temp_beamSection._lonTen.end())
		{
			Json::Value num;
			int count = 0;
			for (auto it1 : temp_beamSection._lonTen[BeamSectionInfo::LT::TOP])
			{
				num.append(it1);
			}
			Bars["top"] = num;
		}
	}
	information["bars"].append(Bars);
	//!���������,��֧��
	Json::Value stirrup;
	Json::Value single;
	//Json::Value singleInfo;
	Json::Value singleList;
	//singleInfo["dimensions"] = temp_beamSection._singleStirrupInfoVec.front();
	//int count = 0;
	for (auto it : temp_beamSection._singleStirrupVec)
	{
		Box tempBox;
		for (auto expend : it)
		{
			tempBox.expand(expend.s);
			tempBox.expand(expend.e);
		}
		auto point = temp_beamSection.referencePoint;
		singleList.append(numToString(std::abs(tempBox.right - tempBox.left)) + " X "
						  + numToString(std::abs(tempBox.top - tempBox.bottom)) +
						  "(" + numToString(std::abs(tempBox.left - point.x)) + "," + numToString(std::abs(tempBox.bottom - point.y)) + ")");
		//count++;
	}

	if (!temp_beamSection._singleStirrupInfoVec.empty())
	{
		single["dimensions"] = temp_beamSection._singleStirrupInfoVec.front();
	}
	else
	{
		single["dimensions"] = "";
	}

	single["stirrupList"] = singleList;
	stirrup["singleStirrup"] = single;
	//!˫֧��
	Json::Value _double;
	//Json::Value singleInfo;
	Json::Value doubleList;
	//int count = 0;
	for (auto it : temp_beamSection._doubleStirrupVec)
	{
		Box tempBox;
		for (auto expend : it)
		{
			tempBox.expand(expend.s);
			tempBox.expand(expend.e);
		}
		auto point = temp_beamSection.referencePoint;
		doubleList.append(numToString(std::abs(tempBox.right - tempBox.left)) + " X "
						  + numToString(std::abs(tempBox.top - tempBox.bottom)) +
						  "(" + numToString(tempBox.left - point.x) + "," + numToString(tempBox.bottom - point.y) + ")");
		//count++;
	}
	if (!temp_beamSection._doubleStirrupInfoVec.empty())
	{
		_double["dimensions"] = temp_beamSection._doubleStirrupInfoVec.front();
	}
	else
	{
		_double["dimensions"] = "";
	}

	_double["stirrupList"] = doubleList;
	stirrup["doubleStirrup"] = _double;

	information["stirrup"] = stirrup;

	/*Json::FastWriter writer;
	std::fstream of("beamSection.json", std::ios::out);
	of << writer.write(information);
	of.close();*/

	return information;
}

bool BeamInfo::outputBeams()
{
	this->serializeAaBeamInfo();
	return true;
}

bool BeamInfo::clearBeamVec()
{
	this->spBeamVec.clear();
	return true;
}

bool BeamInfo::serializeAaBeamInfo()
{
	Json::FastWriter writer;
	Json::Value body;
	if (this->bAssemblyAnnotation)
		body["type"] = u8"flat";
	else
		body["type"] = u8"profile";
	//���漰��ͼֽ
	auto getSupport = [](Json::Value& pos, BeamPosition::SupportType s)
	{
		switch (s)
		{
			case BeamPosition::SupportType::Column:
				pos["supportType"] = u8"column"; break;
			case  BeamPosition::SupportType::Beam:
				pos["supportType"] = u8"beam"; break;
			case BeamPosition::SupportType::Wall:
				pos["supportType"] = u8"wall"; break;
			case BeamPosition::SupportType::Pending:
				pos["supportType"] = u8"pending"; break;
			case BeamPosition::SupportType::Unknown:
				pos["supportType"] = u8"unknown"; break;
			default:
				break;
		}
	};
	auto getPosition = [](Json::Value& endpointOfSpanMidline, Distance2AxisInCartesianCoordinates& dist)
	{
		Json::Value relativeHorizontalAxis, relativeVerticalAxis;
		relativeHorizontalAxis["axis"] = dist.relativeHDistance.first;
		relativeHorizontalAxis["offset"] = dist.relativeHDistance.second;
		relativeVerticalAxis["axis"] = dist.relativeVDistance.first;
		relativeVerticalAxis["offset"] = dist.relativeVDistance.second;
		endpointOfSpanMidline["relativeHorizontalAxis"] = relativeHorizontalAxis;
		endpointOfSpanMidline["relativeVerticalAxis"] = relativeVerticalAxis;
	};

	for (auto ite : this->spBeamVec)
	{
		for (auto beamItm : ite.second)
		{
			Json::Value beam;
			//����
			beam["name"] = beamItm->strBeamName;
			//��id
			beam["uuid"] = beamItm->beamId;
			//����
			beam["spanNum"] = beamItm->spanNum;
			//�׿�id
			beam["firstSpanIndex"] = 0;
			//��ע������
			if (beamItm->bAssemblyAnnotation)
				beam["category"] = u8"assembly";
			else
				beam["category"] = u8"disassembly";
			//��
			Json::Value spans;
			for (auto& spanItm : beamItm->spSpanVec)
			{
				Json::Value span;
				span["spanId"] = spanItm->spanId;
				span["sectionSize"] = beamItm->strSecSize;
				span["netLength"] = strimDouble(spanItm->spanNetLength, 2);
				span["relativeElevation"] = beamItm->relativeElevation;

				// position
				Json::Value spanPosition;
				Json::Value midlineStart, midlineEnd;
#if 1
				getPosition(midlineStart, spanItm->midlineStart);
				spanPosition["midlineStart"] = midlineStart;
				getPosition(midlineEnd, spanItm->midlineEnd);
				spanPosition["midlineEnd"] = midlineEnd;
				span["spanPosition"] = spanPosition;
#endif

#if 0
				Json::Value relativeHorizontalAxisStart, relativeVerticalAxisStart,
					relativeHorizontalAxisEnd, relativeVerticalAxisEnd;
				relativeHorizontalAxisStart["axis"] = spanItm->midlineStart.relativeHDistance.first;
				relativeHorizontalAxisStart["offset"] = spanItm->midlineStart.relativeHDistance.second;
				relativeVerticalAxisStart["axis"] = spanItm->midlineStart.relativeVDistance.first;
				relativeVerticalAxisStart["offset"] = spanItm->midlineStart.relativeVDistance.second;
				midlineStart["relativeHorizontalAxis"] = relativeHorizontalAxisStart;
				midlineStart["relativeVerticalAxis"] = relativeVerticalAxisStart;

				spanPosition["midlineStart"] = midlineStart;

				relativeHorizontalAxisEnd["axis"] = spanItm->midlineEnd.relativeHDistance.first;
				relativeHorizontalAxisEnd["offset"] = spanItm->midlineEnd.relativeHDistance.second;
				relativeVerticalAxisEnd["axis"] = spanItm->midlineEnd.relativeVDistance.first;
				relativeVerticalAxisEnd["offset"] = spanItm->midlineEnd.relativeVDistance.second;
				midlineEnd["relativeHorizontalAxis"] = relativeHorizontalAxisEnd;
				midlineEnd["relativeVerticalAxis"] = relativeVerticalAxisEnd;
				spanPosition["midlineEnd"] = midlineEnd;
#endif



				//���
				Json::Value l, m, r;
				const auto& left = spanItm->left;
				getSupport(l, left.supportType);
				l["bottomLongitudinalReinforcement"] = beamItm->btmLongitudinalReinforce;
				if (!left.btmLongitudinalReinforce.empty())
					l["bottomLongitudinalReinforcement"] = left.btmLongitudinalReinforce;
				l["topLongitudinalReinforcement"] = beamItm->topLongitudinalReinforce;
				if (!left.topLongitudinalReinforce.empty())
					l["topLongitudinalReinforcement"] = left.topLongitudinalReinforce;
				//�ضϳ���
				Json::Value lTopTruncatedLength, lBtmTruncatedLength;
				if (!left.topTruncatedLength.empty())
					for (const auto& truncatedLengthItm : left.topTruncatedLength)
						lTopTruncatedLength.append(truncatedLengthItm.second);
				l["topTruncatedLength"] = lTopTruncatedLength;

				if (!left.btmTruncatedLength.empty())
					for (const auto& truncatedLengthItm : left.btmTruncatedLength)
						lBtmTruncatedLength.append(truncatedLengthItm.second);
				l["bottomTruncatedLength"] = lBtmTruncatedLength;

				//ê�̳���
				Json::Value lTopAnchorLen, lBtmAnchorLen;
				if (!left.topAnchorageLength.empty())
					for (const auto& anchorLenItm : left.topAnchorageLength)
						lTopAnchorLen.append(anchorLenItm.second);

				l["topAnchorageLength"] = lTopAnchorLen;

				if (!left.btmAnchorageLength.empty())
					for (const auto& anchorLenItm : left.btmAnchorageLength)
						lBtmAnchorLen.append(anchorLenItm.second);
				l["btmAnchorageLength"] = lBtmAnchorLen;


				//֧�ſ��
				l["supportWidth"] = left.supportWidth;

				//��˽���
				if (left.spBeamSection != nullptr)
				{
					l["sectionInfo"].append(outputBeamSection(left.spBeamSection));
				}
				else
				{
					l["sectionInfo"] = "";
				}


				//�в�
				const auto& mid = spanItm->mid;
				//���
				if (mid.bMiddle && !mid.relativeElevation.empty())
					span["relativeElevation"] = mid.relativeElevation;
				//����
				m["waistLongitudinalReinforcement"] = beamItm->waistLongitudinalReinforce;
				if (mid.bMiddle && !mid.waistLongitudinalReinforce.empty())
					m["waistLongitudinalReinforcement"] = mid.waistLongitudinalReinforce;
				//�����ݽ�
				m["bottomLongitudinalReinforcement"] = beamItm->btmLongitudinalReinforce;
				if (!mid.btmLongitudinalReinforce.empty())
					m["bottomLongitudinalReinforcement"] = mid.btmLongitudinalReinforce;
				m["topLongitudinalReinforcement"] = beamItm->topLongitudinalReinforce;
				if (!mid.topLongitudinalReinforce.empty())
					m["topLongitudinalReinforcement"] = mid.topLongitudinalReinforce;

				//�в�����
				if (mid.spBeamSection != nullptr)
				{
					m["sectionInfo"].append(outputBeamSection(mid.spBeamSection));
				}
				else
				{
					m["sectionInfo"] = "";
				}

				//�Ҷ�
				const auto& right = spanItm->right;
				getSupport(r, right.supportType);
				r["bottomLongitudinalReinforcement"] = beamItm->btmLongitudinalReinforce;
				if (!right.btmLongitudinalReinforce.empty())
					r["bottomLongitudinalReinforcement"] = right.btmLongitudinalReinforce;
				r["topLongitudinalReinforcement"] = beamItm->topLongitudinalReinforce;
				if (!right.topLongitudinalReinforce.empty())
					r["topLongitudinalReinforcement"] = right.topLongitudinalReinforce;

				//�ضϳ���
				Json::Value rTopTruncatedLength, rBtmTruncatedLength;
				if (!right.topTruncatedLength.empty())
					for (const auto& truncatedLengthItm : right.topTruncatedLength)
						rTopTruncatedLength.append(truncatedLengthItm.second);
				r["topTruncatedLength"] = rTopTruncatedLength;

				if (!right.btmTruncatedLength.empty())
					for (const auto& truncatedLengthItm : right.btmTruncatedLength)
						rBtmTruncatedLength.append(truncatedLengthItm.second);
				r["bottomTruncatedLength"] = rBtmTruncatedLength;


				//ê�̳���
				Json::Value rTopAnchorLen, rBtmAnchorLen;
				if (!right.topAnchorageLength.empty())
					for (const auto& anchorLenItm : right.topAnchorageLength)
						rTopAnchorLen.append(anchorLenItm.second);
				r["topAnchorageLength"] = rTopAnchorLen;

				if (!right.btmAnchorageLength.empty())
					for (const auto& anchorLenItm : right.btmAnchorageLength)
						rBtmAnchorLen.append(anchorLenItm.second);
				r["btmAnchorageLength"] = rBtmAnchorLen;

				//֧�ſ��
				r["supportWidth"] = right.supportWidth;
				//�Ҷ˽���
				if (right.spBeamSection != nullptr)
				{
					r["sectionInfo"].append(outputBeamSection(right.spBeamSection));
				}
				else
				{
					r["sectionInfo"] = "";
				}


				span["left"] = l;
				span["middle"] = m;
				span["right"] = r;

				/*	Json::Value beamSections;
					for (const auto& beamSectionItm : spanItm->spBeamSectionVec)
					{
						Json::Value beamSection;
						beamSections["section"] = nullptr;
					}
					span["beamSections"] = beamSections;*/


				span["stirrup"] = beamItm->strStirrup;
				span["beamId"] = beamItm->beamId;
				spans[spanItm->spanId] = span;
			}
			beam["spans"] = spans;
			body["beams"].append(beam);
		}
		std::fstream of("AaBeamInformation.json", std::ios::out);
		of << writer.write(body);
		of.close();
	}
	return true;
}

bool BeamInfo::serializeElevationProfileBeamInfo()
{

	Json::Value information;
	for (auto it : _beamSectionMap)
	{

		//outputBeamSection(it.second);
		information.append(outputBeamSection(it.second));

	}
	Json::FastWriter writer;
	std::fstream of("beamSection.json", std::ios::out);
	of << writer.write(information);
	of.close();
	return true;
}

bool BeamInfo::testBeamSectionParser(std::map<std::string, std::shared_ptr<Data>>& _dataMap,
									 const std::vector<std::shared_ptr<Block>>& _blocks,
									 const std::map<std::string, int>& _blocksMap,
									 const std::map<std::string, std::vector<int>>& _blockFileMap)
{

	for (auto block : _blocks)
	{
		if (_beamSectionSet.bBeamSectionBlock(block, *_dataMap.begin()->second))
		{
			auto temp_block = BeamSection::retrunBeamSection(*block, *_dataMap.begin()->second);
			std::shared_ptr<BeamSection>pBeamSection(new BeamSection(temp_block));
			_beamSectionMap[block->name] = pBeamSection;
		}
	}

	return true;
}
