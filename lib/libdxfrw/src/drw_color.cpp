#include "drw_color.h"

duint8 DRW_Color::getColorIndex()
{
	return colorIndex;
}

duint32 DRW_Color::getRGB()
{
	return rgb;
}

bool DRW_Color::bAcDBColorPresent()
{
	return false;
}

DRW_Color::ColorType DRW_Color::getColorType()
{
	return colorType;
}

duint8 DRW_CmcColor::getColorIndex()
{
	return DRW_Color::getColorIndex();
}

void DRW_CmcColor::parseDwgColor(DRW::Version v, dwgBuffer* buf)
{
	//2000-
	if (v < DRW::R2004)
	{
		colorIndex = buf->getSBitShort();
		colorType = ColorType::INDEX;
		return;
	}
	colorType = ColorType::Cmc;
	colorIndex = buf->getBitShort();// always 0
	rgb = buf->getBitLong();

	auto red = getRed(rgb);
	auto green = getGreen(rgb);
	auto blue = getBlue(rgb);

	colorByte = buf->getRawChar8();



	if (colorByte & 1)
	{
		colorName = buf->getVariableText(v, false);
		// ?redundant
	}
	if (colorByte & 2)
	{
		bookName = buf->getVariableText(v, false);
		//?redundant
	}
	duint8 colorTp = rgb >> 24;

	switch (colorTp)
	{
		case 0xc0:
			colorType = ColorType::BYLAYER;
			break;
		case 0xc1:
			colorType = ColorType::BYBLOCK;
			break;
		case 0xc2:
			//colorType = ColorType::RGB24;
			break;
		case 0xc3:
			//colorType = ColorType::ACIS;
			break;
		default:
			break;
	}
}

duint8 DRW_EncColor::getColorIndex()
{
	return DRW_Color::getColorIndex();
}

duint32 DRW_EncColor::getRGB()
{
	return DRW_Color::getRGB();
}

bool DRW_EncColor::bAcDBColorPresent()
{
	return bAcDbColorPresent;
}

void DRW_EncColor::parseDwgColor(DRW::Version v, dwgBuffer* buf)
{
	//2000-
	if (v < DRW::R2004)
	{
		colorIndex = buf->getSBitShort();
		colorType = ColorType::INDEX;
		return;
	}
	colorType = ColorType::Enc;
	colorNumber = buf->getBitShort();
	colorFlags = colorNumber & 0xFF00;
	colorIndex = colorNumber & 0x00FF;

	if (colorFlags & 0x4000)
	{
		// 0x80 is set in this case
		bAcDbColorPresent = true;
		colorType = ColorType::ACDBCLR;
	}
	else if (colorFlags & 0x8000)
	{
		rgb = buf->getBitLong();
		//colorType = ColorType::RGB24;
		auto red = getRed(rgb);
		auto green = getGreen(rgb);
		auto blue = getBlue(rgb);
	}
	// todo for transparency codes
	else if (colorFlags & 0x2000)
	{
		//Transparency COLOR: 
		transparencyCodes = buf->getBitLong();

#if 0
		// first byte 
		// 0 = BYLAYER,
		// 1 = BYBLOCK,
		// 2 = the transparency value in the last byte.
		duint8 cbFlags = transparencyCodes & 0xFF;
		duint8 lastByte = 0;
		if (cbFlags == 0x0)
		{
			//BYLAYER
		}
		else if (cbFlags == 0x1)
		{
			//BYBLOCK
		}
		else if (cbFlags == 0x2)
		{
			duint8 transparentcyVal = colorByte & 0xFF0000;
		}
#endif
	}
}

