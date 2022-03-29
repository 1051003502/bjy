#include "stdafx.h"
#include "Drawing.h"
#include "json/json.h"
#include <fstream>

#define U2PX(x) (_view.WindowLeft + (int)((-_view.ViewLeft + (x)) * _view.PPU * _view.ZoomLevel))
#define U2PY(y) (_view.WindowBottom - (int)((-_view.ViewBottom + (y)) * _view.PPU * _view.ZoomLevel))

CDrawing::CDrawing() : _state(PREPARE)
{}
CDrawing::~CDrawing()
{}

BOOL CDrawing::isOpen()
{
	return TRUE;
}
// Construction & Destruction
BOOL CDrawing::Create()
{
	return TRUE;
}
BOOL CDrawing::Destroy()
{
	return TRUE;
}

// Drawing View
BOOL CDrawing::InitView(int x, int y, int nWidth, int nHeight)
{
	_view.ZoomLevel = 1;
	_view.PPU = 20;

	_view.WindowLeft = x;
	_view.WindowTop = y;
	_view.WindowRight = x + nWidth;
	_view.WindowBottom = y + nHeight;
	_view.ViewLeft = 0;
	_view.ViewBottom = 0;

	index = -1;
	full = true;
	type = DRAW_ALL;
	return TRUE;
}

BOOL CDrawing::drawLines(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(204, 102, 102));
	oldpen = (HPEN)SelectObject(hdc, pen);
	for (auto line : _parser->m_data->m_lineData.lines())
	{
		/*auto line = _parser->_lines[vdx];*/
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
	}
	//for (auto vdx : _parser->_vLinesIndices) {
	//	auto line = _parser->_lines[vdx];
	//	MoveToEx(hdc,
	//		U2PX(line.s.x),
	//		U2PY(line.s.y),
	//		NULL
	//	);
	//	LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
	//}

	//for (auto vdx : _parser->_hLinesIndices) {
	//	auto line = _parser->_lines[vdx];
	//	MoveToEx(hdc,
	//		U2PX(line.s.x),
	//		U2PY(line.s.y),
	//		NULL
	//	);
	//	LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
	//}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawPillars(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	double r = 900;
	pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); //255, 165, 0
	oldpen = (HPEN)SelectObject(hdc, pen);
	for (auto pillar : _parser->_pillars)
	{
		HPEN temp_pen, oldpen;
		if (pillar->getError())
		{
			Ellipse(hdc, U2PX(pillar->getTestPoint().x - r), U2PY(pillar->getTestPoint().y + r),
					U2PX(pillar->getTestPoint().x + r), U2PY(pillar->getTestPoint().y - r));
			continue;
		}

		for (auto line : pillar->getVlines())
		{
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		}
		for (auto line : pillar->getHlines())
		{
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		}
		for (auto line : pillar->getSlines())
		{
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		}
	}



	pen = CreatePen(PS_SOLID, 1, RGB(0, 200, 0));
	SelectObject(hdc, pen);
	for (auto pillar : _parser->_pillars)
	{
		for (auto line : pillar->getDLines())
		{
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		}
	}


#if 1

	pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	SelectObject(hdc, pen);
	for (auto borders : _parser->tmep_pair_index)
	{

		{
			/*MoveToEx(hdc,
				U2PX(_parser->m_data->m_lineData.getLine(borders.first).s.x),
				U2PY(_parser->m_data->m_lineData.getLine(borders.first).s.y),
				NULL);
			LineTo(hdc, U2PX(_parser->m_data->m_lineData.getLine(borders.first).e.x),
				U2PY(_parser->m_data->m_lineData.getLine(borders.first).e.y));*/

			MoveToEx(hdc,
					 U2PX(_parser->m_data->m_lineData.getLine(borders.second).s.x),
					 U2PY(_parser->m_data->m_lineData.getLine(borders.second).s.y),
					 NULL);
			LineTo(hdc, U2PX(_parser->m_data->m_lineData.getLine(borders.second).e.x),
				   U2PY(_parser->m_data->m_lineData.getLine(borders.second).e.y));
		}
	}
#endif
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}


BOOL CDrawing::drawBeams(HDC hdc)
{

	HPEN pen, oldpen;
	COLORREF rgbcolor;
	int z = 0;
	int i = 0, j = 0;

	// beam
#if 0
	for (const auto& ite : _dataInte._beamInfo.spBeamVec)
	{
		for (auto it : ite.second)
		{
			if (it->spanNum == 0)
			{
				continue;
			}

			i += 255;
			if (i > 255)
			{
				i = 0; j += 255;
			}
			if (j > 255) j = 0;

			pen = CreatePen(PS_SOLID, 0, RGB(i, 255, j));
			oldpen = (HPEN)SelectObject(hdc, pen);
			for (auto it1 : it->borderLineVec)
			{
				MoveToEx(hdc,
						 U2PX(it1.s.x),
						 U2PY(it1.s.y + z),
						 nullptr);
				LineTo(hdc, U2PX(it1.e.x),
					   U2PY(it1.e.y + z));
			}

			SelectObject(hdc, oldpen);
			DeleteObject(pen);
			//z += 300;
		}
	}

	for (const auto& itm : _parser->_beams)
	{
		i += 255;
		if (i > 255)
		{
			i = 0; j += 255;
		}
		if (j > 255) j = 0;
		pen = CreatePen(PS_SOLID, 0, RGB(i, 255, j));
		oldpen = static_cast<HPEN>(SelectObject(hdc, pen));
		for (auto lineItm : itm->borderLineVec)
		{
			MoveToEx(hdc,
					 U2PX(lineItm.s.x),
					 U2PY(lineItm.s.y + z),
					 nullptr);
			LineTo(hdc, U2PX(lineItm.e.x),
				   U2PY(lineItm.e.y + z));
		}
		const auto& leadLine = _parser->_beamSet.beamLines.lines().at(
			_parser->_beamSet.leadGroupVec.at(itm->leadIdx).cornerIndex.front().second);
		MoveToEx(hdc,
				 U2PX(leadLine.s.x),
				 U2PY(leadLine.s.y + z),
				 nullptr);
		LineTo(hdc, U2PX(leadLine.e.x),
			   U2PY(leadLine.e.y + z));
		for (auto colIdx : itm->colIdxVec)
		{
			auto& col = _parser->_pillars.at(colIdx);
			MoveToEx(hdc,
					 U2PX(col->box.left),
					 U2PY(col->box.top + z),
					 nullptr);
			LineTo(hdc, U2PX(col->box.right),
				   U2PY(col->box.bottom + z));
		}
		SelectObject(hdc, oldpen);
		DeleteObject(pen);
		//z += 300;
	}
#endif

	// temp_line
#if 0
	pen = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto tpLine : _parser->temp_line)
	{
		i += 255;
		if (i > 255)
		{
			i = 0; j += 255;
		}
		if (j > 255) j = 0;
		/*pen = CreatePen(PS_SOLID, 0, RGB(i, 255, j));
		oldpen = (HPEN)SelectObject(hdc, pen);*/
		MoveToEx(hdc,
				 U2PX(tpLine.s.x),
				 U2PY(tpLine.s.y + z),
				 nullptr);
		LineTo(hdc, U2PX(tpLine.e.x),
			   U2PY(tpLine.e.y + z));
		//z += 300;
		/*SelectObject(hdc, oldpen);
		DeleteObject(pen);*/
	}
	SelectObject(hdc, oldpen);
	DeleteObject(pen);

#if 0
	for (auto& line : _dataInte._beamInfo.temp_line)
	{
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y + z),
				 nullptr);
		LineTo(hdc, U2PX(line.e.x),
			   U2PY(line.e.y + z));
	}
#endif


#endif



	//temp_index
#if 1
	pen = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	oldpen = static_cast<HPEN>(SelectObject(hdc, pen));
	for (const auto tpIdx : _parser->temp_index)
	{
		MoveToEx(hdc,
				 U2PX(_parser->m_data->m_lineData.lines()[tpIdx].s.x),
				 U2PY(_parser->m_data->m_lineData.lines()[tpIdx].s.y + z),
				 nullptr);
		LineTo(hdc, U2PX(_parser->m_data->m_lineData.lines()[tpIdx].e.x),
			   U2PY(_parser->m_data->m_lineData.lines()[tpIdx].e.y + z));
		//z += 300;
	}
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
#endif


#if 0
	for (const auto& boardItm : _parser->_longitudinalSection.longitudianlBoards)
	{
		if (CadColor[i] == RGB(204, 102, 102)) i += 15;

		pen = CreatePen(PS_SOLID, 0.5, CadColor[i]);
		oldpen = static_cast<HPEN>(SelectObject(hdc, pen));

		i = i > 250 ? 50 : i + 15;
#if 1
		for (const auto idx : boardItm->vec1)
		{
			MoveToEx(hdc,
					 U2PX(_parser->m_data->m_lineData.lines()[idx].s.x),
					 U2PY(_parser->m_data->m_lineData.lines()[idx].s.y + z),
					 nullptr);
			LineTo(hdc, U2PX(_parser->m_data->m_lineData.lines()[idx].e.x),
				   U2PY(_parser->m_data->m_lineData.lines()[idx].e.y + z));

		}
		for (const auto idx : boardItm->vec2)
		{
			MoveToEx(hdc,
					 U2PX(_parser->m_data->m_lineData.lines()[idx].s.x),
					 U2PY(_parser->m_data->m_lineData.lines()[idx].s.y + z),
					 nullptr);
			LineTo(hdc, U2PX(_parser->m_data->m_lineData.lines()[idx].e.x),
				   U2PY(_parser->m_data->m_lineData.lines()[idx].e.y + z));
		}
		for (const auto idx : boardItm->vec3)
		{
			MoveToEx(hdc,
					 U2PX(_parser->m_data->m_lineData.lines()[idx].s.x),
					 U2PY(_parser->m_data->m_lineData.lines()[idx].s.y + z),
					 nullptr);
			LineTo(hdc, U2PX(_parser->m_data->m_lineData.lines()[idx].e.x),
				   U2PY(_parser->m_data->m_lineData.lines()[idx].e.y + z));
		}
#endif 

#if 0
		for (const auto& tpLine : boardItm->vec4)
		{
			MoveToEx(hdc,
					 U2PX(tpLine.s.x),
					 U2PY(tpLine.s.y + z),
					 nullptr);
			LineTo(hdc, U2PX(tpLine.e.x),
				   U2PY(tpLine.e.y + z));
		}
#endif
		//z += 300;
		SelectObject(hdc, oldpen);
		DeleteObject(pen);
	}
#endif


	return true;
}

BOOL CDrawing::drawWalls(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;
	pen = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);
	double r = 100;
	auto lineData = _parser->m_data->m_lineData.lines();
	int colorIndex = 0;
#if 0
	colorIndex = 0;
	for (auto wall : _parser->_spWallLcoationVec)
	{

		if (CadColor[colorIndex] == RGB(204, 102, 102))colorIndex += 7;
		if (colorIndex >= 255)colorIndex = 0;
		pen = CreatePen(PS_SOLID, 2, CadColor[colorIndex]);
		SelectObject(hdc, pen);
		colorIndex += 7;
		if (colorIndex > 255)
			colorIndex = 0;

		for (auto lien : wall->getBorder())
		{
			MoveToEx(hdc,
				U2PX(lien.s.x),
				U2PY(lien.s.y),
				nullptr);
			LineTo(hdc, U2PX(lien.e.x),
				U2PY(lien.e.y));
		}
		DeleteObject(pen);
	}

#endif

#if 0
	pen = CreatePen(PS_SOLID, 0, CadColor[0]);
	for (auto it : _parser->temp_index)
	{
		MoveToEx(hdc,
			U2PX(lineData[it].s.x),
			U2PY(lineData[it].s.y),
			nullptr);
		LineTo(hdc, U2PX(lineData[it].e.x),
			U2PY(lineData[it].e.y));
	}
	DeleteObject(pen);
#endif
#if 1
	colorIndex = 0;
	for (auto blockVec : _parser->temp_index_colorBlocks) {
		pen = CreatePen(PS_SOLID, 4, CadColor[colorIndex]);
		SelectObject(hdc, pen);
		colorIndex += 2;
		if (colorIndex >= 255)break;//colorIndex = 0;
		for (auto index : blockVec) {
			auto line = lineData[index];
			MoveToEx(hdc,
				U2PX(line.s.x),
				U2PY(line.s.y),
				nullptr);
			LineTo(hdc, U2PX(line.e.x),
				U2PY(line.e.y));
		}
		DeleteObject(pen);
	}
#endif

#if 1
	colorIndex = 0;
	for (auto lineVec : _parser->temp_line_colorBlocks) {
		pen = CreatePen(PS_SOLID, 4, CadColor[colorIndex]);
		SelectObject(hdc, pen);
		colorIndex += 3;
		if (colorIndex >= 255)colorIndex = 0;
		for (auto line : lineVec) {
			MoveToEx(hdc,
				U2PX(line.s.x),
				U2PY(line.s.y),
				nullptr);
			LineTo(hdc, U2PX(line.e.x),
				U2PY(line.e.y));
		}
		DeleteObject(pen);
	}
#endif
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return 0;
}



BOOL CDrawing::drawCircles(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto circle : _parser->m_data->m_circleData.circles())
	{
		Ellipse(hdc, U2PX(circle.x - circle.r), U2PY(circle.y + circle.r), U2PX(circle.x + circle.r), U2PY(circle.y - circle.r));
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);

	return TRUE;
}

BOOL CDrawing::drawBlocks(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;
	int block_size = _parser->_blocks.size();
	double colour_step = 255.0 / block_size;
	double rgb_r = 0.0;
	double rgb_b = 0.0;
	double rgb_g = 0.0;
	srand((unsigned)time(NULL));
	for (auto block : _parser->_blocks)
	{
		//rgb_g = rand() % 255;
		pen = CreatePen(PS_SOLID, 0, RGB(rgb_r, 127, rgb_b));
		oldpen = (HPEN)SelectObject(hdc, pen);

		MoveToEx(hdc, U2PX(block->box.left), U2PY(block->box.top), NULL);
		LineTo(hdc, U2PX(block->box.right), U2PY(block->box.top));
		LineTo(hdc, U2PX(block->box.right), U2PY(block->box.bottom));
		LineTo(hdc, U2PX(block->box.left), U2PY(block->box.bottom));
		LineTo(hdc, U2PX(block->box.left), U2PY(block->box.top));

		rgb_b += colour_step;
		rgb_g += colour_step;
		rgb_r = 255 - rgb_b;

		SelectObject(hdc, oldpen);
		DeleteObject(pen);
	}
#if 0
#endif
#if 1
	pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
	oldpen = (HPEN)SelectObject(hdc, pen);

#if 0
	for (int i = 0; i < _parser->_beamSections.size(); ++i)
	{

		/*	for (auto bp : _parser->_beamSections[i]->break_pointVec)
		{
		double r = bp.radius;
		Ellipse(hdc, U2PX(bp.center_mind.x - r), U2PY(bp.center_mind.y + r),
		U2PX(bp.center_mind.x + r), U2PY(bp.center_mind.y - r));
		}*/

		/*for (auto dim : _parser->_beamSections[i]->dimensionsVec)
		{
			for (auto dimLine : dim.lines)
			{
				MoveToEx(hdc, U2PX(dimLine.s.x), U2PY(dimLine.s.y), NULL);
				LineTo(hdc, U2PX(dimLine.e.x), U2PY(dimLine.e.y));
			}
		}*/

		/*MoveToEx(hdc, U2PX(_parser->_beamSections[i]->bpBox.left), U2PY(_parser->_beamSections[i]->bpBox.top), NULL);
		LineTo(hdc, U2PX(_parser->_beamSections[i]->bpBox.right), U2PY(_parser->_beamSections[i]->bpBox.top));
		LineTo(hdc, U2PX(_parser->_beamSections[i]->bpBox.right), U2PY(_parser->_beamSections[i]->bpBox.bottom));
		LineTo(hdc, U2PX(_parser->_beamSections[i]->bpBox.left), U2PY(_parser->_beamSections[i]->bpBox.bottom));
		LineTo(hdc, U2PX(_parser->_beamSections[i]->bpBox.left), U2PY(_parser->_beamSections[i]->bpBox.top));*/

		/*for (auto it : _parser->_beamSections[i]->stirrupIndexVec)
		{
		auto temp_line = _parser->m_data.m_lineData.lines()[it];
		MoveToEx(hdc, U2PX(temp_line.s.x), U2PY(temp_line.s.y), NULL);
		LineTo(hdc, U2PX(temp_line.e.x), U2PY(temp_line.e.y));
		}*/

		/*for (int j = 0; j < _parser->_beamSections[i]->beamHookVec.size(); ++j)
		{
			int hook1 = _parser->_beamSections[i]->beamHookVec[j].hook_index1;
			int hook2 = _parser->_beamSections[i]->beamHookVec[j].hook_index2;
			Line hL1 = _parser->m_data->m_lineData.getLine(hook1);
			Line hL2 = _parser->m_data->m_lineData.getLine(hook2);
			MoveToEx(hdc, U2PX(hL1.s.x), U2PY(hL1.s.y), NULL);
			LineTo(hdc, U2PX(hL1.e.x), U2PY(hL1.e.y));

			MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
			LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));
		}*/


		for (auto it : _parser->_beamSections[i]->beamStirrupVec)
		{
			//it = _parser->_beamSections[i]->beamStirrupVec[0];
			if (it.single)
				continue;
			for (auto it1 : it.circle_line)
			{
				MoveToEx(hdc, U2PX(it1.s.x), U2PY(it1.s.y), NULL);
				LineTo(hdc, U2PX(it1.e.x), U2PY(it1.e.y));
			}
		}

		/*for (auto it : _parser->_beamSections[i]->beamBPinfoVec)
		{
			for (auto lineIndex : it.bpLeadVec)
			{
				Line hL2 = _parser->m_data->m_lineData.getLine(lineIndex);
				MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
				LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));
			}
		}*/
		//int hook1 = _parser->_beamSections[i]->testLeft;
		//int hook2 = _parser->_beamSections[i]->testBottom;
		//Line hL1 = _parser->m_data->m_lineData.getLine(247);
		//Line hL2 = _parser->m_data->m_lineData.getLine(259);
		/*MoveToEx(hdc, U2PX(hL1.s.x), U2PY(hL1.s.y), NULL);
		LineTo(hdc, U2PX(hL1.e.x), U2PY(hL1.e.y));*/

		/*MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
		LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));*/

		/*for (auto it : _parser->_beamSections[i]->beamStirInfo)
		{
			for (auto it1 : it.stirLeadVec)
			{
				Line hL2 = _parser->m_data->m_lineData.getLine(it1);
				MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
				LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));
			}
		}*/

		/*Line hl1 = _parser->m_data.m_lineData.getLine(_parser->_beamSections[i]->testLeft);
		Line hl2 = _parser->m_data.m_lineData.getLine(_parser->_beamSections[i]->testBottom);
		MoveToEx(hdc, U2PX(hl1.s.x), U2PY(hl1.s.y), NULL);
		LineTo(hdc, U2PX(hl1.e.x), U2PY(hl1.e.y));

		MoveToEx(hdc, U2PX(hl2.s.x), U2PY(hl2.s.y), NULL);
		LineTo(hdc, U2PX(hl2.e.x), U2PY(hl2.e.y));*/

		/*Line hl2 = _parser->m_data.m_lineData.getLine(511);
		MoveToEx(hdc, U2PX(hl2.s.x), U2PY(hl2.s.y), NULL);
		LineTo(hdc, U2PX(hl2.e.x), U2PY(hl2.e.y));*/


	}

#endif

	for (auto it : _parser->_section)
	{

		for (auto dim : it->testIndexVec)
		{
			/*for (auto line : dim.lines)
			{
				MoveToEx(hdc, U2PX(line.s.x), U2PY(line.s.y), NULL);
				LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			}*/

			/*auto temp_line = _parser->m_data->m_lineData.lines()[dim];
			MoveToEx(hdc, U2PX(temp_line.s.x), U2PY(temp_line.s.y), NULL);
			LineTo(hdc, U2PX(temp_line.e.x), U2PY(temp_line.e.y));*/
		}
		//for (int i=3;i< 4/*it->_columnHookVec.size()*/;++i)
		//{
		//	int hook1 = it->_columnHookVec[i].hook_index1;
		//	int hook2 = it->_columnHookVec[i].hook_index2;
		//	Line hL1 = _parser->m_data->m_lineData.getLine(hook1);
		//	Line hL2 = _parser->m_data->m_lineData.getLine(hook2);
		//	MoveToEx(hdc, U2PX(hL1.s.x), U2PY(hL1.s.y), NULL);
		//	LineTo(hdc, U2PX(hL1.e.x), U2PY(hL1.e.y));

		//	MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
		//	LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));
		//}

		/*auto box = it->testBox;
		MoveToEx(hdc, U2PX(box.left), U2PY(box.top), NULL);
		LineTo(hdc, U2PX(box.right), U2PY(box.top));
		LineTo(hdc, U2PX(box.right), U2PY(box.bottom));
		LineTo(hdc, U2PX(box.left), U2PY(box.bottom));
		LineTo(hdc, U2PX(box.left), U2PY(box.top));*/
	}

	for (auto it : _parser->temp_index)
	{
		auto temp_line = _parser->m_data->m_lineData.lines()[it];
		MoveToEx(hdc, U2PX(temp_line.s.x), U2PY(temp_line.s.y), NULL);
		LineTo(hdc, U2PX(temp_line.e.x), U2PY(temp_line.e.y));
	}

	//int hook1 = 68;
	////int hook2 = 211;
	//Line hL1 = _parser->m_data->m_lineData.getLine(hook1);
	////Line hL2 = _parser->m_data->m_lineData.getLine(hook2);
	//MoveToEx(hdc, U2PX(hL1.s.x), U2PY(hL1.s.y), NULL);
	//LineTo(hdc, U2PX(hL1.e.x), U2PY(hL1.e.y));

	/*MoveToEx(hdc, U2PX(hL2.s.x), U2PY(hL2.s.y), NULL);
	LineTo(hdc, U2PX(hL2.e.x), U2PY(hL2.e.y));*/

	/*int i = 0;
	for (auto it : _parser->_corners)
	{

		if (i < 18)
		Ellipse(hdc, U2PX(it.x - 80), U2PY(it.y + 80),
			U2PX(it.x + 80), U2PY(it.y - 80));
		i++;
	}*/

	for (auto section : _parser->_section)
	{
		/*for (auto it : section->_columnStirrupVec)
		{
			for (auto line : it.circle_line)
			{
				MoveToEx(hdc, U2PX(line.s.x), U2PY(line.s.y), NULL);
				LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			}
		}*/
		/*for (auto it : section->testLine)
		{
			MoveToEx(hdc, U2PX(it.s.x), U2PY(it.s.y), NULL);
			LineTo(hdc, U2PX(it.e.x), U2PY(it.e.y));
		}*/
	}


	SelectObject(hdc, oldpen);
	DeleteObject(pen);
#endif





	return TRUE;
}

BOOL CDrawing::drawAxis(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;
	//_dataInte.iniCoordinateSystem();
	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);
#if 1
	for (auto i = 0; i < _parser->_axises.size(); ++i)
	{

		for (auto c : _parser->_axises[i]->hlines)
		{
			auto line = c.second;
			auto circle = c.first.first;
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			Ellipse(hdc, U2PX(circle.x - circle.r), U2PY(circle.y + circle.r), U2PX(circle.x + circle.r), U2PY(circle.y - circle.r));
		}

		for (auto c : _parser->_axises[i]->vlines)
		{
			auto line = c.second;
			auto circle = c.first.first;
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			Ellipse(hdc, U2PX(circle.x - circle.r), U2PY(circle.y + circle.r), U2PX(circle.x + circle.r), U2PY(circle.y - circle.r));
		}
	}
#endif
	/*for (auto haxisLine : _parser->m_data->m_axisLineData.axisHLines())
	{
		MoveToEx(hdc, U2PX(haxisLine.second.s.x), U2PY(haxisLine.second.s.y), NULL);
		LineTo(hdc, U2PX(haxisLine.second.e.x), U2PY(haxisLine.second.e.y));
	}
	for (auto vaxisLine : _parser->m_data->m_axisLineData.axisVLines())
	{
		MoveToEx(hdc, U2PX(vaxisLine.second.s.x), U2PY(vaxisLine.second.s.y), NULL);
		LineTo(hdc, U2PX(vaxisLine.second.e.x), U2PY(vaxisLine.second.e.y));
	}*/



	SelectObject(hdc, oldpen);
	DeleteObject(pen);

	return TRUE;
}

BOOL CDrawing::drawCorners(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto corner : _parser->m_data->m_cornerData.corners())
	{
		for (auto cn : corner.second)
		{
			Ellipse(hdc, U2PX(cn.x - 60), U2PY(cn.y + 60), U2PX(cn.x + 60), U2PY(cn.y - 60));
		}
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawIssues(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 2, RGB(220, 220, 220));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto component : _parser->_issues)
	{
		for (auto vline : component->vlines)
		{
			MoveToEx(hdc,
					 U2PX(vline.s.x),
					 U2PY(vline.s.y),
					 NULL);
			LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
		}
		for (auto hline : component->hlines)
		{
			MoveToEx(hdc,
					 U2PX(hline.s.x),
					 U2PY(hline.s.y),
					 NULL);
			LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
		}
	}
	DeleteObject(pen);
	pen = CreatePen(PS_SOLID, 2, RGB(220, 20, 20));
	SelectObject(hdc, pen);
	for (auto component : _parser->_covers)
	{
		for (auto vline : component->vlines)
		{
			MoveToEx(hdc,
					 U2PX(vline.s.x),
					 U2PY(vline.s.y),
					 NULL);
			LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
		}
		for (auto hline : component->hlines)
		{
			MoveToEx(hdc,
					 U2PX(hline.s.x),
					 U2PY(hline.s.y),
					 NULL);
			LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
		}
	}

	pen = CreatePen(PS_SOLID, 2, RGB(20, 220, 20));
	SelectObject(hdc, pen);
	for (auto component : _parser->_covers)
	{
		MoveToEx(hdc, U2PX(component->box.left), U2PY(component->box.top), NULL);
		LineTo(hdc, U2PX(component->box.right), U2PY(component->box.top));
		LineTo(hdc, U2PX(component->box.right), U2PY(component->box.bottom));
		LineTo(hdc, U2PX(component->box.left), U2PY(component->box.bottom));
		LineTo(hdc, U2PX(component->box.left), U2PY(component->box.top));
	}
	for (auto component : _parser->_issues)
	{
		MoveToEx(hdc, U2PX(component->box.left), U2PY(component->box.top), NULL);
		LineTo(hdc, U2PX(component->box.right), U2PY(component->box.top));
		LineTo(hdc, U2PX(component->box.right), U2PY(component->box.bottom));
		LineTo(hdc, U2PX(component->box.left), U2PY(component->box.bottom));
		LineTo(hdc, U2PX(component->box.left), U2PY(component->box.top));
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawTables(HDC hdc)
{
	fstream of(u8"检测.txt", std::ios::out);
	for (auto it : _parser->m_data->m_textPointData.textpoints())
	{
		auto tt = std::static_pointer_cast<DRW_Text>(it.entity);
		of << tt->text << std::endl;
	}
	of.close();
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 1, RGB(191, 191, 191));
	oldpen = (HPEN)SelectObject(hdc, pen);

	if (full)
	{
		for (auto component : _parser->_tables)
		{
			for (auto vline : component->vlines)
			{
				MoveToEx(hdc,
						 U2PX(vline.s.x),
						 U2PY(vline.s.y),
						 NULL);
				LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
			}
			for (auto hline : component->hlines)
			{
				MoveToEx(hdc,
						 U2PX(hline.s.x),
						 U2PY(hline.s.y),
						 NULL);
				LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
			}
		}
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 2, RGB(0, 191, 255));
		SelectObject(hdc, pen);
		for (auto table : _parser->_tables)
		{
			auto line = table->vlines[table->left];
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			line = table->vlines[table->right];
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			line = table->hlines[table->bottom];
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			line = table->hlines[table->top];
			MoveToEx(hdc,
					 U2PX(line.s.x),
					 U2PY(line.s.y),
					 NULL);
			LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
			/*for (auto extract : table->table) {
				for (auto it : extract.second) {
					Ellipse(hdc, U2PX(it.second->basePoint.x - table->precisionx * 3),
						U2PY(it.second->basePoint.y + table->precisiony * 3),
						U2PX(it.second->basePoint.x + table->precisionx * 3),
						U2PY(it.second->basePoint.y - table->precisiony * 3));
				}
			}*/
		}

		SelectObject(hdc, oldpen);
		DeleteObject(pen);
	}
	else
	{
		auto table = _parser->_tables[index];

		for (auto vline : table->vlines)
		{
			MoveToEx(hdc,
					 U2PX(vline.s.x),
					 U2PY(vline.s.y),
					 NULL);
			LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
		}
		for (auto hline : table->hlines)
		{
			MoveToEx(hdc,
					 U2PX(hline.s.x),
					 U2PY(hline.s.y),
					 NULL);
			LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
		}
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 2, RGB(0, 191, 255));
		SelectObject(hdc, pen);
		auto line = table->vlines[table->left];
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		line = table->vlines[table->right];
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		line = table->hlines[table->bottom];
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		line = table->hlines[table->top];
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		/*for (auto extract : table->table) {
			for (auto it : extract.second) {
				Ellipse(hdc, U2PX(it.second->basePoint.x - table->precisionx * 3),
					U2PY(it.second->basePoint.y + table->precisiony * 3),
					U2PX(it.second->basePoint.x + table->precisionx * 3),
					U2PY(it.second->basePoint.y - table->precisiony * 3));
			}
		}*/
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawBorders(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(204, 102, 102));
	oldpen = (HPEN)SelectObject(hdc, pen);

	if (full)
	{
		if (type == DRAW_BORDERS)
		{

			for (auto component : _parser->_borders)
			{
				for (auto com : _parser->_components)
				{
					if (!component->cover(*com))
					{
						continue;
					}
					for (auto vline : com->vlines)
					{
						MoveToEx(hdc,
								 U2PX(vline.s.x),
								 U2PY(vline.s.y),
								 NULL);
						LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
					}
					for (auto hline : com->hlines)
					{
						MoveToEx(hdc,
								 U2PX(hline.s.x),
								 U2PY(hline.s.y),
								 NULL);
						LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
					}
				}
			}
		}
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
		SelectObject(hdc, pen);
		for (auto component : _parser->_borders)
		{
			for (auto vline : component->vlines)
			{
				MoveToEx(hdc,
						 U2PX(vline.s.x),
						 U2PY(vline.s.y),
						 NULL);
				LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
			}
			for (auto hline : component->hlines)
			{
				MoveToEx(hdc,
						 U2PX(hline.s.x),
						 U2PY(hline.s.y),
						 NULL);
				LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
			}
		}
	}
	else
	{
		auto border = _parser->_borders[index];
		if (type == DRAW_BORDERS)
		{

			for (auto com : _parser->_components)
			{
				if (!border->cover(*com))
				{
					continue;
				}
				for (auto vline : com->vlines)
				{
					MoveToEx(hdc,
							 U2PX(vline.s.x),
							 U2PY(vline.s.y),
							 NULL);
					LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
				}
				for (auto hline : com->hlines)
				{
					MoveToEx(hdc,
							 U2PX(hline.s.x),
							 U2PY(hline.s.y),
							 NULL);
					LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
				}
			}
		}
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
		SelectObject(hdc, pen);
		for (auto vline : border->vlines)
		{
			MoveToEx(hdc,
					 U2PX(vline.s.x),
					 U2PY(vline.s.y),
					 NULL);
			LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
		}
		for (auto hline : border->hlines)
		{
			MoveToEx(hdc,
					 U2PX(hline.s.x),
					 U2PY(hline.s.y),
					 NULL);
			LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
		}
	}
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawExtracts(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);

	//for (auto point : _parser->_textpointsx) {
	//	Ellipse(hdc, U2PX(point.x - 60), U2PY(point.y + 60), U2PX(point.x + 60), U2PY(point.y - 60));
	//}
	//if (_extracts.size() <= 0) {
	//	for (auto component : _parser->_components) {
	//		_extracts.push_back(component->extract(_parser->_kdttexts, _parser->_textpoints));

	//	}
	//}
	//for (auto extract : _extracts) {
	//	for (auto it : extract) {
	//		for (auto jt : it.second) {
	//			//jt.second->basePoint.x;
	//			Ellipse(hdc, U2PX(jt.second->basePoint.x - 60), U2PY(jt.second->basePoint.y + 60), U2PX(jt.second->basePoint.x + 60), U2PY(jt.second->basePoint.y - 60));
	//		}
	//	}
	//}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawTexts(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto& point : _parser->m_data->m_textPointData.textpoints())
	{

		Ellipse(hdc, U2PX(point.x - 60), U2PY(point.y + 60), U2PX(point.x + 60), U2PY(point.y - 60));
	}

	for (auto hline : _parser->_textlines)
	{
		MoveToEx(hdc,
				 U2PX(hline.s.x),
				 U2PY(hline.s.y),
				 NULL);
		LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawDebug(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;
	//temp_index

#if 1
	pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 0));
	oldpen = (HPEN)SelectObject(hdc, pen);
	auto pt = _parser->point;
	Ellipse(hdc, U2PX(pt.x - 60), U2PY(pt.y + 60), U2PX(pt.x + 60), U2PY(pt.y - 60));

	for (auto cn : _parser->cs)
	{
		//for (auto cn : corner.second) {
		Ellipse(hdc, U2PX(cn.x - 60), U2PY(cn.y + 60), U2PX(cn.x + 60), U2PY(cn.y - 60));
		//}
		//auto& line = _parser->_lines[cn.l1];
		//MoveToEx(hdc,
		//	U2PX(line.s.x),
		//	U2PY(line.s.y),
		//	NULL
		//);
		//LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
		//auto& line2 = _parser->_lines[cn.l2];
		//MoveToEx(hdc,
		//	U2PX(line2.s.x),
		//	U2PY(line2.s.y),
		//	NULL
		//);
		//LineTo(hdc, U2PX(line2.e.x), U2PY(line2.e.y));
	}
	for (auto li : _parser->ls)
	{
		auto& line = _parser->m_data->m_lineData.getLine(li);
		MoveToEx(hdc,
				 U2PX(line.s.x),
				 U2PY(line.s.y),
				 NULL);
		LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
#endif
	return true;
}

BOOL CDrawing::drawLine(HDC hdc, const Line& line)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;
	int color;

	color = _transer->getColor(line.entity);
	if (color >= 256)
	{
		color = 0;
	}

	rgbcolor = DRW::CadColor[color];

	pen = CreatePen(PS_SOLID, 0, RGB(204, 102, 102));
	oldpen = (HPEN)SelectObject(hdc, pen);
	MoveToEx(hdc,
			 U2PX(line.s.x),
			 U2PY(line.s.y),
			 NULL);
	LineTo(hdc, U2PX(line.e.x), U2PY(line.e.y));
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::drawComponents(HDC hdc)
{
	HPEN pen, oldpen;
	COLORREF rgbcolor;

	pen = CreatePen(PS_SOLID, 0, RGB(204, 102, 102));
	oldpen = (HPEN)SelectObject(hdc, pen);

	for (auto component : _parser->_components)
	{
		for (auto vline : component->vlines)
		{
			MoveToEx(hdc,
					 U2PX(vline.s.x),
					 U2PY(vline.s.y),
					 NULL);
			LineTo(hdc, U2PX(vline.e.x), U2PY(vline.e.y));
		}
		for (auto hline : component->hlines)
		{
			MoveToEx(hdc,
					 U2PX(hline.s.x),
					 U2PY(hline.s.y),
					 NULL);
			LineTo(hdc, U2PX(hline.e.x), U2PY(hline.e.y));
		}
		for (auto sline : component->slines)
		{
			MoveToEx(hdc,
					 U2PX(sline.s.x),
					 U2PY(sline.s.y),
					 NULL);
			LineTo(hdc, U2PX(sline.e.x), U2PY(sline.e.y));
		}
	}

	SelectObject(hdc, oldpen);
	DeleteObject(pen);
	return TRUE;
}

BOOL CDrawing::Paint(HDC hdc)
{
	HRGN ViewRgn;
	BOOL result;

	// Initialize pDrawing ------------------
	_mutex.lock();

	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(
		hdc,
		_view.WindowLeft,
		_view.WindowTop,
		_view.WindowRight,
		_view.WindowBottom);

	ViewRgn = CreateRectRgn(
		_view.WindowLeft,
		_view.WindowTop,
		_view.WindowRight,
		_view.WindowBottom);
	SelectObject(hdc, ViewRgn);

	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	//result = drwPlot(hDrawing, hdc);
	//drawLines(hdc);
	//drawCorners(hdc);
	if (_state == PREPARE || !_transer.get())
	{
		SetTextColor(hdc, RGB(255, 0, 0));
		SetBkColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 100, "选择要分析的文件", 16);
	}
	else if (_state == LOADING)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
		SetBkColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 100, "正在加载...", 11);
	}
	else if (_state == LOADFAILED)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
		SetBkColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 100, "加载失败，请确认文件格式！", 26);
	}
	else if (_state == PARSING)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
		SetBkColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 100, "正在分析...", 11);
	}
	else if (_state == PARSED)
	{
		if (type == DRAW_ALL)
		{
			drawLines(hdc);
			drawCorners(hdc);
			drawCircles(hdc);
			//drawTexts(hdc);
			//drawComponents(hdc);
			//drawBorders(hdc);
			//drawTables(hdc);
			//drawIssues(hdc);
			//drawTexts(hdc);
			//drawCircles(hdc);
			//draw
		}
		else if (type == DRAW_PILLARS)
		{
			drawComponents(hdc);
			//drawBlocks(hdc);
			drawPillars(hdc);
		}
		else if (type == DRAW_CIRCLES)
		{
			drawCircles(hdc);
			drawTexts(hdc);
		}
		else if (type == DRAW_TABLES)
		{
			drawTables(hdc);
		}
		else if (type == DRAW_BORDERS)
		{
			drawBorders(hdc);
		}
		else if (type == DRAW_COMPONENTS)
		{
			drawComponents(hdc);
		}
		else if (type == DRAW_ISSUES)
		{
			drawIssues(hdc);
		}
		else if (type == DRAW_CORNERS)
		{
			drawCorners(hdc);
		}
		else if (type == DRAW_LINES)
		{
			drawLines(hdc);
		}
		else if (type == DRAW_AXIS)
		{
			drawAxis(hdc);
		}
		else if (type == DRAW_BLOCKS)
		{
			//drawLines(hdc);
			drawComponents(hdc);
			drawBlocks(hdc);
		}
		else if (type == DRAW_GIRDER)
		{
			drawComponents(hdc);
			drawBeams(hdc);
		}
		else if (type == DRAW_WALL)
		{
			drawComponents(hdc);
			drawWalls(hdc);
			//drawBlocks(hdc);
			//drawAxis(hdc);
		}
		else if (type == DRAW_DEBUG)
		{
			drawComponents(hdc);
			drawDebug(hdc);
		}
	}

	//drawExtracts(hdc);
	//drawTexts(hdc);

	DeleteObject(ViewRgn);

	// UnInitilize pDrawing -----------------
	//UnInitilizePDRAWING(hDrawing);
	_mutex.unlock();

	return TRUE;
}
double CDrawing::GetZoomLevel()
{
	return _view.ZoomLevel;
}

BOOL CDrawing::SetZoomLevel(double ZoomLevel)
{
	_view.ZoomLevel = ZoomLevel;
	return TRUE;
}

BOOL CDrawing::GetViewProperties(PVIEW pView)
{
	*pView = _view;
	return TRUE;
}

BOOL CDrawing::SetViewProperties(PVIEW pView)
{
	_view = *pView;
	return TRUE;
}

BOOL CDrawing::GetDrawingBorder(PREALRECT pRect)
{
	Box box;
	if (type == DRAW_TABLES && !full)
	{
		auto table = _parser->_tables[index];
		box = table->box;
	}
	else if (type == DRAW_BORDERS && !full)
	{
		auto border = _parser->_borders[index];
		box = border->box;
	}
	else if (type == DRAW_COMPONENTS && !full)
	{
		auto com = _parser->_components[index];
		box = com->box;
	}
	else if (type == DRAW_BLOCKS && !full)
	{
		auto block = _parser->_blocks[index];
		box = block->box;
	}
	else if (type == DRAW_AXIS && !full)
	{
		auto axis = _parser->_axises[index];
		box = axis->box;
	}
	/*else if (type == DRAW_GIRDER && !full)
	{
		auto girder = _parser->_girder[index];

	}*/
	else if (type == DRAW_DEBUG)
	{
		box.expand(_parser->point);
		for (auto cn : _parser->cs)
		{
			box.expand(cn);
		}
	}
	else
	{
		if (_parser != nullptr)
			box = _parser->_box;
	}
	pRect->left = box.left;
	pRect->right = box.right;
	pRect->top = box.top;
	pRect->bottom = box.bottom;
	return TRUE;
}

BOOL CDrawing::ZoomExtents()
{
	REALRECT rect;
	double ZoomLevelX, ZoomLevelY;

	GetDrawingBorder(&rect);

	// Change Drawing View Properties
	ZoomLevelX = (_view.WindowBottom - _view.WindowTop - 20) / ((rect.top - rect.bottom) * _view.PPU);
	ZoomLevelY = (_view.WindowRight - _view.WindowLeft - 20) / ((rect.right - rect.left) * _view.PPU);
	_view.ZoomLevel = (ZoomLevelX < ZoomLevelY ? ZoomLevelX : ZoomLevelY);

	_view.ViewBottom = rect.bottom - ((_view.WindowBottom - _view.WindowTop) / (_view.ZoomLevel * _view.PPU) - (rect.top - rect.bottom)) / 2;
	_view.ViewLeft = rect.left - ((_view.WindowRight - _view.WindowLeft) / (_view.ZoomLevel * _view.PPU) - (rect.right - rect.left)) / 2;

	_view.ViewTop = _view.ViewBottom + (_view.WindowBottom - _view.WindowTop) / _view.ZoomLevel / _view.PPU;
	_view.ViewRight = _view.ViewLeft + (_view.WindowRight - _view.WindowLeft) / _view.ZoomLevel / _view.PPU;

	return TRUE;
}

BOOL CDrawing::SaveFile(LPCTSTR FileName)
{
	_transer->fileExport(FileName, DRW::R2004, false);
	return TRUE;
}

void CDrawing::onParsed(MarkDrawing& drawing)
{
	_state = PARSED;
	_dataInte.iniFileDataMap(drawing._drawingType, drawing._spTranser, drawing._spParser);
	ZoomExtents();
}

void CDrawing::onReaded(MarkDrawing& drawing/*const std::string& drawingId*/)
{

	//auto drawing = findDrawing(drawingId);
	/*if (drawing->_spTranser->_thread->joinable())
	{
		drawing->_spTranser->_thread->join();
	}*/
	_state = PARSING;
	drawing._spParser->sigParsed.clean();
	_parsed.push_back(drawing._spParser->sigParsed.connect(std::bind(&CDrawing::onParsed, this, drawing)));
	auto data = drawing._spTranser->getData();
	if (data != nullptr)
		drawing._spParser->parse(drawing._spTranser->getData());

}

void CDrawing::onFailed()
{
	_state = LOADFAILED;
}

BOOL CDrawing::LoadFile(MarkDrawing& markDrawing)
{

	_state = LOADING;

	_mutex.lock();
	MarkDrawing temp_markerDrawing = markDrawing;
	auto fileName = temp_markerDrawing.findPath();

	_transer = temp_markerDrawing._spTranser;
	_parser = temp_markerDrawing._spParser;

	_failed.push_back(temp_markerDrawing._spTranser->sigFailed.connect(std::bind(&CDrawing::onFailed, this)));
	_readed.push_back(temp_markerDrawing._spTranser->sigReaded.connect(std::bind(&CDrawing::onReaded, this, markDrawing)));
	temp_markerDrawing._spTranser->fileImport(fileName);
	_mutex.unlock();

	return TRUE;
}

BOOL CDrawing::DataSorting()
{
	//_dataInte.iniObjectAndType("2", "2");
	//_dataInte.iniBlocks();
	//_dataInte.iniCoordinateSystem();
	//_dataInte.parserTarget();


	//_dataInte.parserDrawingWriteFile();//解析
	_dataInte.processClueFile();//整理：一次解析数据

	return true;
}

bool CDrawing::select(const std::string& file)
{
	//
	//test 测试部分
	//_project.BuildCoordinateSystem();

	/***************************/
	_mutex.lock();
	//auto pair = _project.ReturnFindFIle(file);
	auto pair = _dataInte.returnFindFIle(file);
	//_transer = pair->second.first;
	//_parser = pair->second.second;
	_transer = pair.spDraTranser;
	_parser = pair.spDarParser;
	full = true;
	index = 0;
	ZoomExtents();
	//
	_mutex.unlock();
	return true;
}

bool CDrawing::close1(const std::string& file)
{

	_mutex.lock();
	if (_transer.get() && _transer->file == file)
	{
		_transer.reset();
		_parser.reset();
	}
	//_files.erase(file);
	//_project.FilesErase(file);
	_dataInte.filesErase(file);
	if (/*_project.FIlesSize()*/ _dataInte.filesSize() > 0)
	{
		full = true;
		index = 0;
		//auto it = _project.FilesBegin();
		auto it = _dataInte.filesBegin();
		//_transer = it->second.first;
		//_parser = it->second.second;
		_transer = it->second.spDraTranser;
		_parser = it->second.spDarParser;
		ZoomExtents();
	}
	_mutex.unlock();
	return true;
}

bool CDrawing::step(int d)
{
	if (type == DRAW_TABLES)
	{
		if (_parser->_tables.size() <= 0)
			return false;
		full = false;
		index = (index + d + _parser->_tables.size()) % _parser->_tables.size();
		ZoomExtents();
	}
	else if (type == DRAW_BORDERS)
	{
		if (_parser->_borders.size() <= 0)
			return false;
		index = (index + d + _parser->_borders.size()) % _parser->_borders.size();
		full = false;
		ZoomExtents();
	}
	else if (type == DRAW_COMPONENTS)
	{
		if (_parser->_components.size() <= 0)
			return false;
		index = (index + d + _parser->_components.size()) % _parser->_components.size();
		full = false;
		ZoomExtents();
	}
	else if (type == DRAW_BLOCKS)
	{
		if (_parser->_blocks.size() <= 0)
			return false;
		index = (index + d + _parser->_blocks.size()) % _parser->_blocks.size();
		full = false;
		ZoomExtents();
	}
	else if (type == DRAW_AXIS)
	{
		if (_parser->_axises.size() <= 0)
			return false;
		index = (index + d + _parser->_axises.size()) % _parser->_axises.size();
		full = false;
		ZoomExtents();
	}
	return true;
}

bool CDrawing::next()
{
	return step(1);
}

bool CDrawing::prev()
{
	return step(-1);
}

bool CDrawing::getFull()
{
	return full;
}

int CDrawing::getIndex()
{
	return index;
}

DrawType CDrawing::get()
{
	return type;
}

bool CDrawing::select(DrawType cur)
{
	if (cur >= DRAW_NUMBER)
	{
		cur = DRAW_ALL;
	}
	full = true;
	type = (DrawType)cur;
	index = 0;
	ZoomExtents();
	return true;
}

bool CDrawing::write(const std::string& file)
{

	Json::Value body;
	Json::FastWriter writer;

	if (type == DRAW_TABLES)
	{
		if (index >= _parser->_tables.size())
		{
			index = -1;
		}
		if (index == -1)
		{
			for (auto table : _parser->_tables)
			{
				body["tables"].append(_parser->serializeTable(table));
			}
		}
		else
		{
			body["tables"].append(_parser->serializeTable(_parser->_tables[index]));
		}
		body["type"] = "table";

		ofstream of;
		of.open(file);
		of << writer.write(body);
		of.close();
	}
	else if (type == DRAW_COMPONENTS)
	{
		if (index >= _parser->_components.size())
		{
			index = -1;
		}
		if (index == -1)
		{
			for (auto component : _parser->_components)
			{
				body["components"].append(_parser->serializeComponent(component));
			}
		}
		else
		{
			body["components"].append(_parser->serializeComponent(_parser->_components[index]));
		}
		body["type"] = "component";

		ofstream of;
		of.open(file);
		of << writer.write(body);
		of.close();
	}
	else if (type == DRAW_PILLARS)
	{
		if (index >= _parser->_components.size())
		{
			index = -1;
		}
		if (index == -1)
		{
			for (auto pillar : _parser->_pillars)
			{
				body["pillars"].append(_parser->serializePillar(pillar));
			}
		}
		else
		{
			body["pillars"].append(_parser->serializePillar(_parser->_pillars[index]));
		}
		body["type"] = "pillar";
		ofstream of;
		of.open(file);
		of << writer.write(body);
		of.close();
	}
	else if (type == DRAW_BORDERS)
	{
		if (index >= _parser->_borders.size())
		{
			index = -1;
		}
		if (index == -1)
		{
			return false;
		}
		else
		{
			auto border = _parser->_borders[index];
			auto data = _parser->cutBy(border->box);
			_transer->resetData(data);
			_transer->fileExport(file, DRW::R2004, false);
		}
	}
	else if (type == DRAW_BLOCKS)
	{
		if (index >= _parser->_blocks.size())
		{
			index = -1;
		}
		/*if (index == -1)
			return false;
		auto block = _parser->_blocks[index];
		auto data = _parser->cutBy(block->box);
		_transer->resetData(data);
		_transer->fileExport(file, DRW::AC1018, false);*/
		if (index == -1)
		{
			for (auto block : _parser->_blocks)
			{
				body["blocks"].append(_parser->serializeBlock(block));
			}
		}
		else
		{
			body["block"].append(_parser->serializeBlock(_parser->_blocks[index]));
		}
		body["type"] = "block";
		ofstream of;
		of.open(file);
		of << writer.write(body);
		of.close();
	}
	else
	{
		_transer->fileExport(file, DRW::R2004, false);
	}
	return true;
}
