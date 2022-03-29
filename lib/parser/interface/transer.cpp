#include "transer.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include "json/json.h"
#include "libdwgr.h"
#include "libdxfrw.h"

bool Transer::fileImport(const std::string& fileI)
{
	this->file = fileI;
	thread = std::make_shared<std::thread>([this, fileI]
	{
		const auto found = static_cast<int>(fileI.find_last_of("."));
		std::string fileExt = fileI.substr((long long)found + 1);
		std::string fileNoExt = fileI.substr(0, found);
		std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::toupper);

		dxData = std::make_shared<dx_data>();
		currentBlock = dxData->blockInMode;
		DRW_ExtConverter conv("gb2312");
		std::string msg = conv.toUtf8(const_cast<std::string*>(&fileI));
		spdlog::get("dxfrwResult")->info("{}:start", msg);
		spdlog::get("dxfrwResult")->flush();
		if (fileExt == "DXF")
		{
			auto* dxf = new dxfRW(fileI.c_str());
			const bool success = dxf->read(this, false);
			delete dxf;
			sigReaded.call();
			return success;
		}
		if (fileExt == "DWG")
		{
			auto* dwg = new DwgR(fileI.c_str());
			const bool success = dwg->read(this, false);

			if (success)
			{
				spdlog::get("dxfrwResult")->info("{}:{}", msg, "succeed");
				spdlog::get("dxfrwResult")->flush();
			}
			else
			{
				spdlog::get("dxfrwResult")->error("{}:{}", msg, "failed");
				spdlog::get("dxfrwResult")->flush();
			}

			delete dwg;
			sigReaded.call();
			return success;
		}
		if (fileExt == "JSON")
		{
			readJson(fileI);
			sigReaded.call();
			return true;
		}
		spdlog::get("dxfrwResult")->warn("{}:{}", msg, u8"file extension can be dxf or dwg");
		spdlog::get("dxfrwResult")->flush();
		sigFailed.call();
		return true;
	});
	return true;
}

void Transer::readJson(const std::string& file)
{
	ifstream is;
	is.open(file);
	std::string str;
	is >> str;

	Json::Value body;
	Json::Reader reader;
	reader.parse(str, body);
	Json::Value type = body["type"];

	Json::Value list;
	if (type == "table")
	{
		list = body["tables"];
	}
	else if (type == "component")
	{
		list = body["components"];
	}
	for (auto table = list.begin(); table != list.end(); ++table)
	{
		Json::Value lines = (*table)["lines"];
		for (auto line = lines.begin(); line != lines.end(); ++line)
		{
			Json::Value s = (*line)["s"];
			Json::Value e = (*line)["e"];
			double x = s["x"].asDouble();
			double y = s["y"].asDouble();
			DRW_Line l;
			l.basePoint.x = x;
			l.basePoint.y = y;
			l.secPoint.x = e["x"].asDouble();
			l.secPoint.y = e["y"].asDouble();
			addLine(l);
		}
		if (!(*table).isMember("texts"))
			return;
		Json::Value texts = body["texts"];
		for (auto text = texts.begin(); text != texts.end(); ++text)
		{
			Json::Value t = (*text)["text"];
			Json::Value row = (*text)["row"];
			Json::Value col = (*text)["col"];
			Json::Value x = (*text)["x"];
			Json::Value y = (*text)["y"];
			DRW_Text tt;
			tt.basePoint.x = x.asDouble();
			tt.basePoint.y = y.asDouble();
			tt.text = t.asString();
			addText(tt);
		}
	}
}

int Transer::getColor(std::shared_ptr<DRW_Entity> entity)
{
	if (!entity.get())
		return 0;
	if (entity->color == 0 || entity->color == 256)
	{
		for (auto it : dxData->layers)
		{
			if (it.second->entryName == entity->layer)
			{
				return it.second->color;
			}
		}
	}
	return entity->color;
}

bool Transer::fileExport(const std::string& file, DRW::Version v, bool binary)
{
	if (dxData == NULL)
	{
		return false;
	}
	dxfW.reset(new dxfRW(file.c_str()));
	bool success = dxfW->write(this, v, binary);
	return success;
}

void Transer::writeEntity(DRW_Entity* e)
{
	switch (e->dwgType)
	{
		case DRW::POINT:
			dxfW->writePoint(static_cast<DRW_Point*>(e));
			break;
		case DRW::LINE:
			dxfW->writeLine(static_cast<DRW_Line*>(e));
			break;
		case DRW::CIRCLE:
			dxfW->writeCircle(static_cast<DRW_Circle*>(e));
			break;
		case DRW::RAY:
			dxfW->writeRay(static_cast<DRW_Ray*>(e));
			break;
		case DRW::XLINE:
			dxfW->writeXline(static_cast<DRW_Xline*>(e));
			break;
		case DRW::ARC:
			dxfW->writeArc(static_cast<DRW_Arc*>(e));
			break;
		case DRW::SOLID:
			dxfW->writeSolid(static_cast<DRW_Solid*>(e));
			break;
		case DRW::ELLIPSE:
			dxfW->writeEllipse(static_cast<DRW_Ellipse*>(e));
			break;
		case DRW::LWPOLYLINE:
			dxfW->writeLWPolyline(static_cast<DRW_LWPolyline*>(e));
			break;
		case DRW::POLYLINE_2D:
		case DRW::POLYLINE_3D:
		case DRW::POLYLINE_PFACE:
		case DRW::POLYLINE_MESH:
			dxfW->writePolyline(static_cast<DRW_Polyline*>(e));
			break;
		case DRW::SPLINE:
			dxfW->writeSpline(static_cast<DRW_Spline*>(e));
			break;
		case DRW::INSERT:
			dxfW->writeInsert(static_cast<DRW_Insert*>(e));
			break;
		case DRW::MTEXT:
			dxfW->writeMText(static_cast<DRW_MText*>(e));
			break;
		case DRW::TEXT:
			dxfW->writeText(static_cast<DRW_Text*>(e));
			break;
		case DRW::DIMENSION_LINEAR:
		case DRW::DIMENSION_ALIGNED:
		case DRW::DIMENSION_ANG2LN:
		case DRW::DIMENSION_ANG3PT:
		case DRW::DIMENSION_RADIUS:
		case DRW::DIMENSION_DIAMETER:
		case DRW::DIMENSION_ORDINATE:
			dxfW->writeDimension(static_cast<DRW_Dimension*>(e));
			break;
		case DRW::LEADER:
			dxfW->writeLeader(static_cast<DRW_Leader*>(e));
			break;
		case DRW::HATCH:
			dxfW->writeHatch(static_cast<DRW_Hatch*>(e));
			break;
		case DRW::IMAGE:
			dxfW->writeImage(static_cast<DRW_Image*>(e), static_cast<dx_ifaceImg*>(e)->path);
			break;
		default:
			break;
	}
}