/******************************************************************************
**  dwg2dxf - Program to convert dwg/dxf to dxf(ascii & binary)              **
**                                                                           **
**  Copyright (C) 2015 José F. Soriano, rallazz@gmail.com                    **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DX_IFACE_H
#define DX_IFACE_H

#include <iostream>
#include <memory>
#include <thread>
#include "drw_interface.h"
#include "intern/drw_textcodec.h"
#include "libdxfrw.h"
#include "signal.h"

using namespace std;

class dx_ifaceImg : public DRW_Image
{
public:
	dx_ifaceImg()
	{}
	dx_ifaceImg(const DRW_Image& p) :
		DRW_Image(p)
	{}
	~dx_ifaceImg()
	{}
	std::string path;  //stores the image path
};

//container class to store entities.
class dx_ifaceBlock : public DRW_Block
{
public:
	dx_ifaceBlock(){}
	dx_ifaceBlock(const DRW_Block& p) : DRW_Block(p){}
	~dx_ifaceBlock(){}
	std::list<std::shared_ptr<DRW_Entity>> ents;  //stores the entities list
};

//container class to store full dwg/dxf data.
class dx_data
{
public:
	dx_data()
	{
		blockInMode.reset(new dx_ifaceBlock());
	}
	~dx_data()
	{}

	DRW_Header header;													//stores a copy of the header vars
	std::map<duint32, std::shared_ptr<DRW_LType>> lineTypes;			//stores a copy of all line types
	std::map<duint32, std::shared_ptr<DRW_Layer>> layers;				//stores a copy of all layers
	std::map<duint32, std::shared_ptr<DRW_Dimstyle>> dimStyles;			//stores a copy of all dimension styles
	std::map<duint32, std::shared_ptr<DRW_Vport>> VPorts;				//stores a copy of all vports
	std::map<duint32, std::shared_ptr<DRW_Textstyle>> textStyles;		//stores a copy of all text styles
	std::map<duint32, std::shared_ptr<DRW_AppId>> appIds;				//stores a copy of all appIds
	std::map<duint32, std::shared_ptr<dx_ifaceBlock>> blocks;			//stores a copy of all blocks and the entities in it
	std::map<duint32, std::shared_ptr<dx_ifaceImg>> images;				//temporary list to find images for link with DRW_ImageDef. Do not delete it!!

	std::shared_ptr<dx_ifaceBlock> blockInMode;			//container to store model entities
};

class Transer : public DRW_Interface
{
public:
	Transer()
	{
		dxfW = NULL;
		dxData = NULL;
	}
	~Transer()
	{
		if (thread != nullptr && thread->joinable())
			thread->join();
	}
	bool fileImport(const std::string& fileI);
	bool fileExport(const std::string& file, DRW::Version v, bool binary);
	int getColor(std::shared_ptr<DRW_Entity> entity);

    void writeEntity(DRW_Entity *e);

    void readJson(const std::string &file);
    //reimplement virtual DRW_Interface functions

	//reader part, stores all in class dx_data
	//header
	void addHeader(const DRW_Header* data)
	{
		dxData->header = *data;
	}

	/** Called for every line Type.  */
	virtual void addLType(const std::shared_ptr<DRW_LType>& data)
	{
		dxData->lineTypes[data->handle] = data;
	}
	/** Called for every layer. */
	virtual void addLayer(const std::shared_ptr<DRW_Layer>& data)
	{
		dxData->layers[data->handle] = data;
	}
	/** Called for every dim style. */
	virtual void addDimStyle(const std::shared_ptr<DRW_Dimstyle>& data)
	{
		dxData->dimStyles[data->handle] = data;
	}
	/** Called for every VPORT table. */
	virtual void addVport(const std::shared_ptr<DRW_Vport>& data)
	{
		dxData->VPorts[data->handle] = data;
	}
	/** Called for every text style. */
	virtual void addTextStyle(const std::shared_ptr<DRW_Textstyle>& data)
	{
		dxData->textStyles[data->handle] = data;
	}
	/** Called for every AppId entry. */
	virtual void addAppId(const std::shared_ptr<DRW_AppId>& data)
	{
		dxData->appIds[data->handle] = data;
	}

	//blocks
	virtual void addBlock(const DRW_Block& data)
	{
		std::shared_ptr<dx_ifaceBlock> bk(new dx_ifaceBlock(data));
		currentBlock = bk;
		dxData->blocks[bk->handle] = bk;
	}
	virtual void endBlock()
	{
		currentBlock = dxData->blockInMode;
	}

	virtual void setBlock(const int /*handle*/)
	{
		/*cout << "setBlock" << endl;*/
	}  //unused


	virtual void addPoint(const DRW_Point& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Point(data)));
	}
	virtual void addLine(const DRW_Line& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Line(data)));
	}

	/* Called for every attribute */
	virtual void addAttrib(const DRW_Attrib& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Attrib(data)));
	}

	/* Called for every attribute definition */
	virtual void addAttdef(const DRW_Attdef& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Attdef(data)));
	}

	virtual void addRay(const DRW_Ray& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Ray(data)));
	}
	virtual void addXline(const DRW_Xline& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Xline(data)));
	}
	virtual void addArc(const DRW_Arc& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Arc(data)));
	}
	virtual void addCircle(const DRW_Circle& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Circle(data)));
	}
	virtual void addEllipse(const DRW_Ellipse& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Ellipse(data)));
	}
	virtual void addLWPolyline(const DRW_LWPolyline& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_LWPolyline(data)));
	}
	virtual void addPolyline(const DRW_Polyline& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Polyline(data)));
	}
	virtual void addSpline(const DRW_Spline* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Spline(*data)));
	}

	virtual void addKnot(const DRW_Entity& data)
	{
		//currentBlock->ent.push_back(new DRW_Entity(data));
		/*std::cout << "addKnot" << endl;*/
	}

	virtual void addInsert(const DRW_Insert& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Insert(data)));
	}
	virtual void addTrace(const DRW_Trace& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Trace(data)));
	}
	virtual void add3dFace(const DRW_3Dface& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_3Dface(data)));
	}
	virtual void addSolid(const DRW_Solid& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Solid(data)));
	}
	virtual void addMText(const DRW_MText& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_MText(data)));
	}
	virtual void addText(const DRW_Text& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Text(data)));
	}
	virtual void addDimAlign(const DRW_DimAligned* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimAligned(*data)));
	}
	virtual void addDimLinear(const DRW_DimLinear* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimLinear(*data)));
	}
	virtual void addDimRadius(const DRW_DimRadius* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimRadius(*data)));
	}
	virtual void addDimDiameter(const DRW_DimDiameter* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimDiameter(*data)));
	}
	virtual void addDimAngular(const DRW_DimAngular* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimAngular(*data)));
	}
	virtual void addDimAngular3P(const DRW_DimAngular3p* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimAngular3p(*data)));
	}
	virtual void addDimOrdinate(const DRW_DimOrdinate* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_DimOrdinate(*data)));
	}
	virtual void addLeader(const DRW_Leader* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Leader(*data)));
	}
	virtual void addHatch(const DRW_Hatch* data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Hatch(*data)));
	}
	virtual void addViewport(const DRW_Viewport& data)
	{
		currentBlock->ents.push_back(std::shared_ptr<DRW_Entity>(new DRW_Viewport(data)));
	}
	virtual void addImage(const DRW_Image* data)
	{
		std::shared_ptr<dx_ifaceImg> img(new dx_ifaceImg(*data));
		currentBlock->ents.push_back(img);
		dxData->images[img->handle] = img;
	}

	virtual void linkImage(const DRW_ImageDef* data)
	{
		duint32 handle = data->handle;
		std::string path(data->entryName);
		for (auto it = dxData->images.begin(); it != dxData->images.end(); ++it)
		{
			if ((it->second)->ref == handle)
			{
				//dx_ifaceImg *img = *it;
				//img->path = path;
				it->second->path = path;
			}
		}
	}

	//writer part, send all in class dx_data to writer
	virtual void addComment(const char* /*comment*/)
	{
		//cout << "addComment" << endl;
	}

	virtual void writeHeader(DRW_Header& data)
	{
		//complete copy of header vars:
		data = dxData->header;
	}

	virtual void writeBlocks()
	{
		//write each block
		//for (auto it = cData->blocks.begin(); it != cData->blocks.end(); ++it) {
		//	std::shared_ptr<dx_ifaceBlock> bk = *it;
		//	dxfW->writeBlock(bk.get());
		//	//and write each entity in block
		//	for (auto jt = bk->ent.begin(); jt != bk->ent.end(); ++jt)
		//		writeEntity(jt->get());
		//}
		int i = 0;

		for (auto it = dxData->blocks.begin(); it != dxData->blocks.end(); ++it, ++i)
		{
			std::shared_ptr<dx_ifaceBlock> bk = it->second;
			dxfW->writeBlock(bk.get());
			int j = 0;
			for (auto jt = bk->ents.begin(); jt != bk->ents.end(); ++jt, ++j)
				writeEntity(jt->get());
		}
	}
	//only send the name, needed by the reader to prepare handles of blocks & blockRecords
	virtual void writeBlockRecords()
	{
		for (auto it = dxData->blocks.begin(); it != dxData->blocks.end(); ++it)
			dxfW->writeBlockRecord(it->second->name);
	}
	//write entities of model space and first paper_space
	virtual void writeEntities()
	{
		for (auto it = dxData->blockInMode->ents.begin(); it != dxData->blockInMode->ents.end(); ++it)
			writeEntity(it->get());
	}

	virtual void writeLTypes()
	{
		for (auto it = dxData->lineTypes.begin(); it != dxData->lineTypes.end(); ++it)
			dxfW->writeLineType(&(*(it->second)));
	}

	virtual void writeLayers()
	{
		for (auto it = dxData->layers.begin(); it != dxData->layers.end(); ++it)
			dxfW->writeLayer(&(*(it->second)));
	}

	virtual void writeTextstyles()
	{
		for (auto it = dxData->textStyles.begin(); it != dxData->textStyles.end(); ++it)
			dxfW->writeTextstyle(&(*(it->second)));
	}

	virtual void writeVports()
	{
		for (auto it = dxData->VPorts.begin(); it != dxData->VPorts.end(); ++it)
			dxfW->writeVport(&(*(it->second)));
	}

	virtual void writeDimstyles()
	{
		for (auto it = dxData->dimStyles.begin(); it != dxData->dimStyles.end(); ++it)
			dxfW->writeDimstyle(&(*(it->second)));
	}

	virtual void writeAppId()
	{
		for (auto it = dxData->appIds.begin(); it != dxData->appIds.end(); ++it)
			dxfW->writeAppId(&(*(it->second)));
	}

	std::shared_ptr<dx_data> getData()
	{
		return dxData;
	}

	bool resetData(std::shared_ptr<dx_data> data)
	{
		dxData = data;
		return true;
	}

    signal<void(void)> sigReaded;
    signal<void(void)> sigWrited;
    signal<void(void)> sigFailed;

	std::shared_ptr<std::thread> thread;

	std::shared_ptr<dxfRW> dxfW;                  //pointer to writer, needed to send data
	std::shared_ptr<dx_data> dxData;               // class to store or read data
	std::shared_ptr<dx_ifaceBlock> currentBlock;  // current block operation

    std::string file;
};

#endif  // DX_IFACE_H