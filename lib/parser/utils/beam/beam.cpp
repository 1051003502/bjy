#include "beam.h"

const std::string Beam::BeamReg = u8"(\\w+L[^\\(\\（]*)\\s*(?:[(|（][^\\）|)]*[)|）])?\\s*(\\d+[X|x]?\\d+)?\\s*";

const std::string Beam::BeamReg2 = u8"(\\w+L\\d+[^\\(（ ]*)\\s?([\\(（][^\\)）]*[\\)）])?\\s?(P?Y?\\d+X\\d+)?\\s*";

const std::string Beam::SecSizeReg = u8"(?:P?Y?\\d+X\\d+/?\\d+)";

const std::string Beam::StirrupReg = u8"(?:\\d*.{2}\\d+@\\d+)(?:[\\(（]?\\d+[\\)）])?/?\\d+(?:[\\(（]?\\d+[\\)）])?\\s*";

const std::string Beam::LongitudinalReforceReg = u8"(\\d+.{2}\\d+)\\s?(\\d+/\\d+)?\\s?；\\s?(\\d+.{2}\\d+)\\s?(\\d+/\\d+)?\\s*";

const std::string Beam::LongitudinalReforceReg2 = u8"(?:\\d+.{2}\\d+\\+[(|（]\\d+.{2}\\d+[)|）])";

const std::string Beam::WasitLongitudinalReforceReg = u8"(?:[G|N]\\d+.{2}\\d+)\\s*";

const std::string Beam::ElevationReg = u8"[(|（][+|-]\\d+.{2}\\d+[)|）]\\s*";

const std::string Beam::BeamBracketReg = u8"([(|（]\\d+X\\d+[)|)])?([(|（]\\d+[)|）])?([(|（]\\d+.\\d+.\\d+轴[)|）])?";

const std::string Beam::SpanAxisReg = u8"[(|（](\\d+).(\\d+).(\\d+)轴[)|）]\\s*";


bool Beam::getUUID(std::string strInfo)
{
	std::hash<std::string> stringhasher;

	this->beamId = stringhasher(strInfo.append(std::to_string(std::chrono::system_clock::now().time_since_epoch().count())));
	return true;
}

bool Beam::extractBeamSecSize(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_search(strText, sMatch, std::regex(SecSizeReg, std::regex_constants::icase)))
	{
		for (auto i = 0; i < sMatch.size(); i++)
		{
			auto ss = sMatch[0];
			if (ss.matched)
			{
				this->strSecSize = ss.str();
				return true;
			}

		}
	}
	return false;
}

bool Beam::extractBeamInfo(const std::string& strText)
{
	//对文本的各个模式进行分类的匹配
	this->extractBeamInfo1(strText);
	this->extractBeamSecSize(strText);
	int markcount = 0;
	bool mark = this->extractBeamInfo2(strText);
	if (mark) markcount++;
	mark = this->extractBeamInfo3(strText);
	if (mark) markcount++;
	mark = this->extractBeamInfo4(strText);
	if (mark) markcount++;
	if (markcount >= 1)this->bAssemblyAnnotation = true;
	else this->bAssemblyAnnotation = false;
	this->extractBeamElevation(strText);


	return true;
}

bool Beam::extractBeamInfo1(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_search(strText, sMatch, std::regex(BeamReg2, std::regex_constants::icase)))
	{
		for (auto it = 0; it < sMatch.size(); it++)
		{
			auto ss = sMatch[it];
			if (ss.matched)
				switch (it)
				{
					case 0:
						spdlog::get("all")->info("beam info 1 full :{}", ss.str());
						break;
					case 1:
						spdlog::get("all")->info("beam name :{}", ss.str());
						this->strBeamName = ss.str();
						break;
					case 2:
						spdlog::get("all")->info("beam bracket info:{}", ss.str());
						this->extractBracketInfo(ss.str());
						break;
					case 3:
						spdlog::get("all")->info("beam sec size :{}", ss.str());
						this->strSecSize = ss.str();
						break;
					default:
						break;
				}
		}

	}

	return false;
}

bool Beam::extractBracketInfo(const std::string& strText)
{
	//可能有跨，轴，尺寸等信息
	// todo 处理括号内的不必要的信息
	this->extractBeamSecSize(strText);

#if 0
	if (std::regex_match(strText, sMatch, std::regex(BeamBracketReg)))
	{
		for (int it = 0; it < sMatch.size(); ++it)
		{
			std::ssub_match ss = sMatch[it];
			if (ss.matched)
				switch (it)
				{
					case 0:
						spdlog::get("all")->info("full text:{}", ss.str());
						break;
					case 1:
						this->strSecSize = ss.str();
						spdlog::get("all")->info("section size:{}", this->strSecSize);
						break;
					case 2:
						this->spanNum = static_cast<int>(std::strtol(ss.str().c_str(), nullptr, 10));
						spdlog::get("all")->info("span count:{}", this->spanNum);
						break;
					case 3:
						extractSpanAxis(ss.str());
						spdlog::get("all")->info("span or axis:{}", ss.str());
					default:
						break;
				}
		}
		return true;
	}
#endif
	return false;
}

bool Beam::extractSpanAxis(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_match(strText, sMatch, std::regex(SpanAxisReg)))
	{
		for (int it = 0; it < sMatch.size(); ++it)
		{
			std::ssub_match ss = sMatch[it];
			if (ss.matched)
				switch (it)
				{
					case 0:
						spdlog::get("all")->info("full text:{}", ss.str());
						break;
					case 1:
						this->spanNum = static_cast<int>(std::strtol(ss.str().c_str(), nullptr, 10));
						spdlog::get("all")->info("span count:{}", this->spanNum);

						break;
					case 2:
						this->sAxis = ss.str();
						spdlog::get("all")->info("begin axis:{}", this->sAxis);
						break;
					case 3:
						this->eAxis = ss.str();
						spdlog::get("all")->info("end axis:{}", this->eAxis);
					default:
						break;
				}
		}
		return true;
	}
	return false;
}

bool Beam::extractBeamInfo2(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_search(strText, sMatch, std::regex(StirrupReg)))
	{
		if (!sMatch.empty() && sMatch[0].matched)
		{
			this->strStirrup = sMatch[0].str();
			spdlog::get("all")->info("beam stirrup:{}", this->strStirrup);
			return true;
		}
	}
	return false;
}

bool Beam::extractBeamInfo3(const std::string& strText)
{
	std::smatch sMatch;

	if (std::regex_search(strText, sMatch, std::regex(LongitudinalReforceReg)))
	{
		for (int it = 0; it < sMatch.size(); it++)
		{
			std::ssub_match ssMatch = sMatch[it];
			if (ssMatch.matched)
			{
				switch (it)
				{
					case 0:
						spdlog::get("all")->info("beam Longitudinal:{}", ssMatch.str());
						break;
					case 1:
						this->topLongitudinalReinforce.assign(ssMatch.str());
						spdlog::get("all")->info("beam top 1:{}", ssMatch.str());
						break;
					case 2:
						this->topLongitudinalReinforce.append(" " + ssMatch.str());
						spdlog::get("all")->info("beam top 2:{}", ssMatch.str());
						break;
					case 3:
						this->btmLongitudinalReinforce.assign(ssMatch.str());
						spdlog::get("all")->info("beam bottom 1:{}", ssMatch.str());
						break;
					case 4:
						this->btmLongitudinalReinforce.append(" " + ssMatch.str());
						spdlog::get("all")->info("beam bottom 2:{}", ssMatch.str());
						break;
					default:
						break;
				}
			}
		}
		return true;
	}
	else if (std::regex_search(strText, sMatch, std::regex(LongitudinalReforceReg2)))
	{
		topLongitudinalReinforce = btmLongitudinalReinforce = strText;
		spdlog::get("all")->info("beam longitudianl 2:{}", strText);
		return true;
	}
	return false;
}

bool Beam::extractBeamInfo4(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_search(strText, sMatch, std::regex(WasitLongitudinalReforceReg)))
	{
		waistLongitudinalReinforce = strText;
		spdlog::get("all")->info("beam wasit longitudianl:{}", strText);
		return true;
	}
	return false;
}

bool Beam::extractBeamElevation(const std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_search(strText, sMatch, std::regex(ElevationReg)))
	{
		this->relativeElevation = strText;
		spdlog::get("all")->info("beam elevation:{}", strText);
		return true;
	}
	return false;
}


bool Beam::updateDirectionWidth()
{
	if (this->borderLineVec.size() >= 2)
	{
		Line line1 = this->borderLineVec.at(0);
		Line line2 = this->borderLineVec.at(1);
		//得到方向
		if (line1.horizontal() && line2.horizontal())
		{
			this->widSize = std::abs(line1.s.y - line2.s.y);
			spdlog::get("all")->info("beam width:{}", widSize);
			this->direction = Beam::Direction::H;
			spdlog::get("all")->info("ensure horizontal");
		}
		else if (line1.vertical() && line2.vertical())
		{
			this->widSize = std::abs(line1.s.x - line2.s.x);
			this->direction = Beam::Direction::V;
			spdlog::get("all")->info("beam width:{}", widSize);
			spdlog::get("all")->info("ensure vertical");
		}
		else
		{
			this->widSize = slantingDistanceLine(line1, line2);
			this->direction = Beam::Direction::S;
			spdlog::get("all")->info("beam width:{}", widSize);
			spdlog::get("all")->info("ensure slash");
		}
		return true;
	}
	else if (borderLineVec.size() == 1)
	{
		if (borderLineVec.front().horizontal())
		{
			this->widSize = 0;
			spdlog::get("all")->warn("maybe horizontal");
			this->direction = Beam::Direction::H;
		}
		else if (borderLineVec.front().vertical())
		{
			this->widSize = 0;
			spdlog::get("all")->warn("maybe vertical");
			this->direction = Beam::Direction::V;
		}
		else
		{
			this->widSize = 0;//todo
			this->direction = Beam::Direction::S;
			spdlog::get("all")->warn("maybe slash");
		}
		return true;
	}
	return true;
}


bool Beam::appendSpan(std::shared_ptr<BeamSpan> span)
{
	//后续可能会将集中标注的信息加入到span中
	this->spSpanVec.push_back(span);
	return true;
}

std::string Beam::getName()
{
	return this->strBeamName;
}

unsigned char Beam::getSpanNum()
{
	return this->spanNum;
}

bool Beam::getbAssemblyAnnotation()
{
	return this->bAssemblyAnnotation;
}

std::vector<std::shared_ptr<BeamSpan>> Beam::getBeamSpanVec()
{
	return this->spSpanVec;
}




bool operator==(std::shared_ptr<Beam>& beam1, std::shared_ptr<Beam>& beam2)
{
	if (beam1 == nullptr || beam2 == nullptr)
		return false;

	//todo 暂时只比较梁名，断面尺寸，箍筋 ，标高,腰筋，上部纵筋，底部纵筋
	if (beam1->strBeamName == beam2->strBeamName &&
		beam1->strSecSize == beam2->strSecSize &&
		beam1->strStirrup == beam2->strStirrup &&
		beam1->relativeElevation == beam2->relativeElevation &&
		beam1->waistLongitudinalReinforce == beam2->waistLongitudinalReinforce &&
		beam1->topLongitudinalReinforce == beam2->topLongitudinalReinforce &&
		beam1->btmLongitudinalReinforce == beam2->btmLongitudinalReinforce)
	{
		return true;
	}
	return false;
}

const std::string BeamPosition::LongitudianlPriorityReg = u8"(?:\\d+.{2}\\d+\\s\\d*\\/?\\d*\\/?\\d*)\\s*";


bool BeamPosition::extractPriorityAnnotation(LongitudinalPos pos, const  std::string& strText)
{
	std::smatch sMatch;
	if (std::regex_match(strText, sMatch, std::regex(LongitudianlPriorityReg)))
	{
		if (!sMatch.empty())
		{
			if (pos == LongitudinalPos::Up)
			{
				topLongitudinalReinforce = strText;
				spdlog::get("all")->info("top:{}", strText);
				return true;
			}
			else
			{
				btmLongitudinalReinforce = strText;
				spdlog::get("all")->info("down:{}", strText);
				return true;
			}
		}
	}
	return false;
}

bool BeamPosition::extractMidPriortyAnnotation(const std::string& strText)
{
	std::smatch sMatch;

	if (std::regex_search(strText, sMatch, std::regex(LongitudianlPriorityReg)))
	{
		if (!sMatch.empty())
		{
			this->bMiddle = true;
			this->waistLongitudinalReinforce = sMatch[1].str();
		}
		return true;
	}
	else if (std::regex_search(strText, sMatch, std::regex(Beam::ElevationReg)))
	{
		if (!sMatch.empty())
		{
			this->bMiddle = true;
			this->relativeElevation = sMatch[1].str();
		}
		return true;
	}
	return false;
}



bool BeamSpan::searchAnnotationPriority(std::set<int>& textPtSet,
										std::shared_ptr<Beam> spBeam,
										const Box& box, Data& data,
										BeamSpan::SpanPosition spanPos)
{
	//找到柱的中心
	Point midPt((box.left + box.right) / 2, (box.top + box.bottom) / 2);
	//查找原位标注进行过滤（4个）
	std::vector<int> textPointIdxVec = data.m_kdtTreeData.kdtTexts().knnSearch(midPt, 4);

	for (const auto& textIdx : textPointIdxVec)
	{
		//过滤集中标注的文本点
		if (spBeam->textPtIdxSet.find(textIdx) == spBeam->textPtIdxSet.end())
		{
			auto& tp = data.m_textPointData.textpoints()[textIdx];
			auto t = std::static_pointer_cast<DRW_Text>(tp.entity);
			int count = t->text.size();
			double textWidth = t->height / 2 * count;
			if (spBeam->direction == Beam::Direction::H || spBeam->direction == Beam::Direction::S)
			{

				Point textMidPt(tp.x + textWidth / 2, tp.y);
				//左端 o---
				if (spanPos == SpanPosition::Left)
				{
					//判断是否是有效的左端纵筋标注
					if (textMidPt.x > box.left)
						if (tp.y < box.bottom && tp.y >= box.bottom + std::abs(box.left - box.right))//修正上下边界
						{
							this->left.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Down, t->text);
							textPtSet.insert(textIdx);
						}
						else if (tp.y > box.top && tp.y <= box.top + std::abs(box.left - box.right))
						{
							this->left.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Up, t->text);
							textPtSet.insert(textIdx);
						}
				}
				//右端 ---o
				else
				{
					//同上
					if (textMidPt.x < box.right)
						if (tp.y < box.bottom && tp.y >= box.bottom + std::abs(box.left - box.right))
						{
							this->right.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Down, t->text);
							textPtSet.insert(textIdx);
						}
						else if (tp.y > box.top && tp.y <= box.top + std::abs(box.left - box.right))
						{
							this->right.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Up, t->text);
							textPtSet.insert(textIdx);
						}
				}
			}
			else if (spBeam->direction == Beam::Direction::V)
			{
				Point textMidPt(tp.x, tp.y + textWidth / 2);
				//下端
				if (spanPos == SpanPosition::Left)
				{
					if (textMidPt.y > box.bottom)
						if (tp.x < box.left && tp.x >= box.left + std::abs(box.bottom - box.top))//修正左右边界
						{
							this->left.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Up, t->text);
							textPtSet.insert(textIdx);
						}
						else if (tp.x > box.right && tp.x <= box.right + std::abs(box.bottom - box.top))
						{
							this->left.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Down, t->text);
							textPtSet.insert(textIdx);
						}
				}
				//上端
				else
				{
					if (textMidPt.y < box.top)
						if (tp.x < box.left && tp.x >= box.left + std::abs(box.bottom - box.top))
						{
							this->right.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Up, t->text);
							textPtSet.insert(textIdx);
						}

						else if (tp.x > box.right && tp.x <= box.right + std::abs(box.bottom - box.top))
						{
							this->right.extractPriorityAnnotation(BeamPosition::LongitudinalPos::Down, t->text);
							textPtSet.insert(textIdx);
						}

				}
			}
		}
	}
	if (textPointIdxVec.empty())
	{
		spdlog::get("all")->info("there are no texts for priority,maybe it can not be triggered");
		return true;
	}
	return false;
}

bool BeamSpan::searchMidPriority(std::set<int>& textPtSet, std::shared_ptr<Beam> spBeam, Box box1, Box box2, Data& data)
{
	Point midPt1((box1.left + box1.right) / 2, (box1.top + box1.bottom) / 2);
	Point midPt2((box2.left + box2.right) / 2, (box2.top + box2.bottom) / 2);
	Point midPt((midPt1.x + midPt2.x) / 2, (midPt1.y + midPt2.y) / 2);
	std::vector<int> textPointIdxVec = data.m_kdtTreeData.kdtTexts().radiusSearch(midPt, midPt1.distance(midPt2) / 4);
	for (const auto& textIdx : textPointIdxVec)
	{
		if (spBeam->textPtIdxSet.find(textIdx) == spBeam->textPtIdxSet.end() &&
			textPtSet.find(textIdx) == textPtSet.end())
		{
			auto& tp = data.m_textPointData.textpoints().at(textIdx);
			auto t = std::static_pointer_cast<DRW_Text>(tp.entity);
			this->mid.extractMidPriortyAnnotation(t->text);
#if 0
			// 横梁
			if (spBeam->direction == Beam::Direction::H || spBeam->direction == Beam::Direction::S)
			{

			}
			//纵梁
			else
			{

			}
#endif //!暂不区分
		}
	}
	if (textPointIdxVec.empty())
		return false;
	else
		return true;
}

void BeamSpan::searchBorders(const std::shared_ptr<Pillar>& col2, const std::shared_ptr<Pillar>& col1, std::shared_ptr<Beam> _spBeam)
{
	double with = col2->getBox().left - col1->getBox().right;
	if (with > 0)
	{
		//x
		for (const auto& lineItm : _spBeam->borderLineVec)
		{
			const double l = lineItm.s.x < col1->getBox().left ? col1->getBox().left : lineItm.s.x;
			const double r = lineItm.e.x > col2->getBox().right ? col2->getBox().right : lineItm.e.x;
			if ((r - l) / with > 0.7)
			{
				borders.push_back(lineItm);
			}
		}
	}
	else
	{
		with = col2->getBox().bottom - col1->getBox().top;
		//y
		for (const auto& lineItm : _spBeam->borderLineVec)
		{
			const double b = lineItm.s.y < col1->getBox().bottom ? col1->getBox().bottom : lineItm.s.y;
			const double t = lineItm.e.y > col2->getBox().top ? col2->getBox().top : lineItm.e.y;
			if ((t - b) / with > 0.7)
			{
				borders.push_back(lineItm);
			}
		}
	}
}

bool BeamSpan::updateWithAndDirection()
{
	if (borders.size() <= 1)
		return false;
	const auto& l1 = borders.at(0);
	const auto& l2 = borders.at(1);

	if (l1.horizontal() && l2.horizontal())
	{
		this->spanDirection = SpanDirection::Horz;
		this->redundantWidth = abs(l1.s.y - l2.s.y);
	}
	else if (l1.vertical() && l2.vertical())
	{
		this->spanDirection = SpanDirection::Vert;
		this->redundantWidth = abs(l1.s.x - l2.s.x);
	}
	//斜的
	else
	{
		spanDirection = SpanDirection::Slash;
		this->redundantWidth = slantingDistanceLine(l1, l2);
	}
	return true;
}

bool BeamSpan::updateNetLength(const std::shared_ptr<Pillar>& col2, const std::shared_ptr<Pillar>& col1)
{
	if (col2->getBox().left - col1->getBox().right > 0)
		spanNetLength = col2->getBox().left - col1->getBox().right;
	else
		spanNetLength = col2->getBox().bottom - col1->getBox().top;
	spdlog::get("all")->info("span net long:{}", spanNetLength);
	return true;
}

bool BeamSpan::updateSpanMidlinePosition(const std::shared_ptr<Pillar>& col1,
										 const std::shared_ptr<Pillar>& col2,
										 std::shared_ptr<Axis>& axis,
										 std::vector<int>& idxVec,
										 std::vector<Line>& lineVec)
{
#if 1
	pIdxVec = &idxVec;
	pLineVec = &lineVec;
#endif //test print

	if (borders.size() <= 1)
		return false;

	auto& l1 = borders.at(0);
	auto& l2 = borders.at(1);
	bool bAck = false;

	//trim 2 lines
	if (l1.horizontal() && l2.horizontal())
	{
		Point s(col1->getBox().left + col1->getBox().width() / 2, (l1.s.y + l2.s.y) / 2);
		Point e(col2->getBox().left + col2->getBox().width() / 2, (l1.s.y + l2.s.y) / 2);

		midlineStart.pt = s;
		findClosestHAxis(midlineStart.relativeHDistance, axis->hlines, s.y);
		findClosestVAxis(midlineStart.relativeVDistance, axis->vlines, s.x);
		midlineEnd.pt = e;
		findClosestHAxis(midlineEnd.relativeHDistance, axis->hlines, e.y);
		findClosestVAxis(midlineEnd.relativeVDistance, axis->vlines, e.x);
		bAck = true;
	}
	else if (l1.vertical() && l2.vertical())
	{
		Point s((l1.s.x + l2.s.x) / 2, col1->getBox().bottom + col1->getBox().height() / 2);
		Point e((l1.s.x + l2.s.x) / 2, col2->getBox().bottom + col2->getBox().height() / 2);

		midlineStart.pt = s;
		findClosestHAxis(midlineStart.relativeHDistance, axis->hlines, s.y);
		findClosestVAxis(midlineStart.relativeVDistance, axis->vlines, s.x);
		midlineEnd.pt = e;
		findClosestHAxis(midlineEnd.relativeHDistance, axis->hlines, e.y);
		findClosestVAxis(midlineEnd.relativeVDistance, axis->vlines, e.x);
		bAck = true;
	}
	else
	{
		Line checkLine = l1.length() > l2.length() ? l1 : l2;
		//斜线
		Point pt1, pt2;
		bool bCheck = false;
		Line tpLine;
		bCheck = checkLeftBorderBound(pt1, col1, checkLine);
		if (bCheck)
			tpLine.s = pt1;
		else
			getLeftStartOfMidline(l1, l2, tpLine);

		bCheck = false;
		bCheck = checkRightBorderBound(pt2, col2, checkLine);
		if (bCheck)
			tpLine.e = pt2;
		else
			getRightEndOfMidline(l1, l2, tpLine);

		midlineStart.pt = tpLine.s;
		findClosestHAxis(midlineStart.relativeHDistance, axis->hlines, tpLine.s.y);
		findClosestVAxis(midlineStart.relativeVDistance, axis->vlines, tpLine.s.x);
		midlineEnd.pt = tpLine.e;
		findClosestHAxis(midlineEnd.relativeHDistance, axis->hlines, tpLine.e.y);
		findClosestVAxis(midlineEnd.relativeVDistance, axis->vlines, tpLine.e.x);
		bAck = true;
	}
	/*if (bAck)
		pLineVec->push_back(Line(midlineStart.pt, midlineEnd.pt));*/
	return bAck;
}

void BeamSpan::getLeftStartOfMidline(Line& l1, Line& l2, Line& tpLine)
{
	Line midline = getMidline(l1, l2);
	bool bAbove = isAbove(l1, l2);

	if (bAbove)
	{
		Point tpPt;
		tpLine.s = getLeftStartInMidlineOfTopLine(l1, l2, midline);
		tpPt = getLeftStartInMidlineOfBtmLine(l2, l1, midline);
		if (tpPt.x < tpLine.s.x)
			tpLine.s = tpPt;
	}
	else
	{
		Point tpPt;
		tpLine.s = getLeftStartInMidlineOfTopLine(l2, l1, midline);
		tpPt = getLeftStartInMidlineOfBtmLine(l1, l2, midline);
		if (tpPt.x < tpLine.s.x)
			tpLine.s = tpPt;

	}
}

void BeamSpan::getRightEndOfMidline(Line& l1, Line& l2, Line& tpLine)
{
	Line midline = getMidline(l1, l2);
	bool bAbove = isAbove(l1, l2);

	if (bAbove)
	{
		Point tpPt;
		tpLine.e = getRightEndInMidlineOfTopLine(l1, l2, midline);
		tpPt = getRightEndInMidlineOfBtmLine(l2, l1, midline);
		if (tpPt.x > tpLine.e.x)
			tpLine.e = tpPt;
	}
	else
	{
		Point tpPt;
		tpLine.e = getRightEndInMidlineOfTopLine(l2, l1, midline);
		tpPt = getRightEndInMidlineOfBtmLine(l1, l2, midline);
		if (tpPt.x > tpLine.e.x)
			tpLine.e = tpPt;
	}
}


Point BeamSpan::getLeftStartInMidlineOfBtmLine(Line& btmLine, Line& topLine, Line midline)
{
	Line midlineCrossLine = getVLineBasedOnStartOfLine2(topLine, btmLine);
	double halfLenOfCrossLine = midlineCrossLine.length() / 2;
	double rad = acos((this->redundantWidth / 2) / halfLenOfCrossLine);
	double fixedLen = sin(rad) * this->redundantWidth / 2;
	double xInMidlineOfStart = midlineCrossLine.s.x - fixedLen;
	double yInMidlineOfStart = knowXToY(midline, xInMidlineOfStart);
	return Point(xInMidlineOfStart, yInMidlineOfStart);
}

Point BeamSpan::getRightEndInMidlineOfBtmLine(Line& btmLine, Line& topLine, Line midline)
{
	Line midlineCrossLine = getVLineBasedOnEndOfLine2(topLine, btmLine);
	double halfLenOfCrossLine = midlineCrossLine.length() / 2;
	double rad = acos((this->redundantWidth / 2) / halfLenOfCrossLine);
	double fixedLen = sin(rad) * this->redundantWidth / 2;
	double xInMidlineOfEnd = midlineCrossLine.e.x - fixedLen;
	double yInMidlineOfEnd = knowXToY(midline, xInMidlineOfEnd);
	return Point(xInMidlineOfEnd, yInMidlineOfEnd);
}

Point BeamSpan::getLeftStartInMidlineOfTopLine(Line& topLine, Line& btmLine, Line midline)
{
	Line midlineCrossLine = getVLineBasedOnStartOfLine2(btmLine, topLine);
	double halfLenOfCrossLine = midlineCrossLine.length() / 2;
	double rad = acos((this->redundantWidth / 2) / halfLenOfCrossLine);
	double fixedLen = sin(rad) * this->redundantWidth / 2;
	double xInMidlineOfStart = midlineCrossLine.s.x + fixedLen;
	double yInMidlineOfStart = knowXToY(midline, xInMidlineOfStart);
	return  Point(xInMidlineOfStart, yInMidlineOfStart);
}

Point BeamSpan::getRightEndInMidlineOfTopLine(Line& topLine, Line& btmLine, Line& midline)
{
	Line midlineCrossLine = getVLineBasedOnEndOfLine2(btmLine, topLine);
	double halfLenOfCrossLine = midlineCrossLine.length() / 2;
	double rad = acos((this->redundantWidth / 2) / halfLenOfCrossLine);
	double fixedLen = sin(rad) * this->redundantWidth / 2;
	double xInMidlineOfEnd = midlineCrossLine.e.x + fixedLen;
	double yInMidlineOfEnd = knowXToY(midline, xInMidlineOfEnd);
	return  Point(xInMidlineOfEnd, yInMidlineOfEnd);
}

Line BeamSpan::getMidline(Line& l1, Line& l2)
{
	double offset = 0.0;
	if (abs(l1.s.x - l2.s.x) < 1)
		offset += 100;
	Line midLineCrossLine1, midLineCrossLine2;
	// the same x based on l2
	midLineCrossLine1 = getVLineBasedOnStartOfLine2(l1, l2, offset);
	// the same x based on l1
	midLineCrossLine2 = getVLineBasedOnStartOfLine2(l2, l1);

	Point pt1((midLineCrossLine1.s.x + midLineCrossLine1.e.x) / 2,
			  (midLineCrossLine1.s.y + midLineCrossLine1.e.y) / 2),
		pt2((midLineCrossLine2.s.x + midLineCrossLine2.e.x) / 2,
			(midLineCrossLine2.s.y + midLineCrossLine2.e.y) / 2);

	Line midline;
	if (pt1.x < pt2.x)
	{
		midline.s = pt1;
		midline.e = pt2;
	}
	else
	{
		midline.s = pt2;
		midline.e = pt1;
	}
	return midline;
}

Line BeamSpan::getVLineBasedOnStartOfLine2(Line& l1, Line l2, double offset /*= 0.0*/)
{
	Line crossLine;
	l2.s.x = l2.s.x + offset;
	double yInPointOfLine1 = knowXToY(l1, l2.s.x);
	if (yInPointOfLine1 > l2.s.y)
		crossLine = Line(l2.s, Point(l2.s.x, yInPointOfLine1));
	else
		crossLine = Line(Point(l2.s.x, yInPointOfLine1), l2.s);
	return crossLine;
}

Line BeamSpan::getVLineBasedOnEndOfLine2(Line& l1, Line l2, double offset /*= 0.0*/)
{
	Line crossLine;
	l2.e.x = l2.e.x + offset;
	double yInPointOfLine1 = knowXToY(l1, l2.e.x);
	if (yInPointOfLine1 > l2.e.y)
		crossLine = Line(l2.e, Point(l2.e.x, yInPointOfLine1));
	else
		crossLine = Line(Point(l2.e.x, yInPointOfLine1), l2.e);
	return crossLine;
}

bool BeamSpan::checkRightBorderBound(Point& pt, const std::shared_ptr<Pillar>& pillar2, Line& l1)
{
	const Point centerPt2((pillar2->getBox().left + pillar2->getBox().right) / 2, (pillar2->getBox().bottom + pillar2->getBox().top) / 2);
	//检测右边
	if (l1.e.x > pillar2->getBox().right)
	{
		double yInLine1 = knowXToY(l1, pillar2->getBox().right);
		double exceedLength = Point(pillar2->getBox().right, yInLine1).distance(l1.e);
		if (exceedLength / (pillar2->getBox().width() / 2) > 1)
		{
			pt = centerPt2;
			return true;
		}
	}
	return false;
}

bool BeamSpan::checkLeftBorderBound(Point& pt, const std::shared_ptr<Pillar>& pillar1, Line& l1)
{
	const Point centerPt((pillar1->getBox().left + pillar1->getBox().right) / 2, (pillar1->getBox().bottom + pillar1->getBox().top) / 2);
	Line tpLine;
	//检测左边
	if (l1.s.x < pillar1->getBox().left)
	{
		double yInLine1 = knowXToY(l1, pillar1->getBox().left);
		double exceedLength = Point(pillar1->getBox().left, yInLine1).distance(l1.s);
		if ((exceedLength / (pillar1->getBox().width() / 2)) > 1)
		{
			pt = centerPt;
			return  true;
		}
	}
	return false;
}

unsigned char BeamSpan::getSpanId()
{
	return spanId;
}

std::pair<std::string, std::string> BeamSpan::getSize()
{
	return std::pair<std::string, std::string>(this->strSecWidth, this->strSecHeight);
}

double BeamSpan::getSpanNetLength()
{
	return this->spanNetLength;
}

double BeamSpan::getRelativeElevation()
{
	return 0.0;
}

std::vector<Line> BeamSpan::getBorders()
{
	return this->borders;
}

Distance2AxisInCartesianCoordinates BeamSpan::beamLocStart()
{
	return this->midlineStart;
}

Distance2AxisInCartesianCoordinates BeamSpan::beamLocEnd()
{
	return this->midlineEnd;
}

std::vector<BeamPosition> BeamSpan::getBeamPosition()
{
	std::vector<BeamPosition> tempBeamPositionVec;
	tempBeamPositionVec.push_back(left);
	tempBeamPositionVec.push_back(mid);
	tempBeamPositionVec.push_back(right);
	return tempBeamPositionVec;
}

