#include "dwgutil.h"
#include <sstream>
#include <iomanip>
#include "../libdwgr.h"
#include "drw_dbg.h"
#include "rscodec.h"

/** utility function
 * convert a int to string in hex
 **/
namespace DRW
{
	std::string toHexStr(int n) {
		std::ostringstream Convert;
		Convert << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << n;
		return Convert.str();
	}

	bool checkSentinel(dwgBuffer* buf, SecEnum::DWGSection sec, bool start /*= false*/)
	{
		DRW_UNUSED(start);
		std::stringstream ss;
		for (int i = 0; i < 16; i++)
		{
			//DRW_DBGH(buf->getRawChar8());
			ss << DRW::toHexStr(buf->getRawChar8());
			//DRW_DBG(" ");
			ss << " ";
		}
		spdlog::get("all")->info("sentinel:{}", ss.str());
		return true;
	}
}  // namespace DRW

/**
 * @brief dwgRSCodec::decode239I
 * @param in : input data (at least 255*blk bytes)
 * @param out : output data (at least 239*blk bytes)
 * @param blk number of codewords ( 1 cw == 255 bytes)
 */
void dwgRSCodec::decode239I(duint8* in, duint8* out, duint32 blk) {
	int k = 0;
	unsigned char data[255]{};
	RScodec rsc(0x96, 8, 8);  //(255, 239)
	for (duint32 i = 0; i < blk; i++)
	{
		k = i;
		for (int j = 0; j < 255; j++)
		{
			data[j] = in[k];
			k += blk;
		}
		int r = rsc.decode(data);
		if (r < 0)
			DRW_DBG("\nWARNING: dwgRSCodec::decode239I, can't correct all errors");
		k = i * 239;
		for (int j = 0; j < 239; j++)
		{
			out[k++] = data[j];
		}
	}
}

/**
 * @brief dwgRSCodec::decode251I
 * @param in : input data (at least 255*blk bytes)
 * @param out : output data (at least 251*blk bytes)
 * @param blk number of codewords ( 1 cw == 255 bytes)
 */
void dwgRSCodec::decode251I(unsigned char* in, unsigned char* out, duint32 blk) {
	int k = 0;
	unsigned char data[255];
	RScodec rsc(0xB8, 8, 2);  //(255, 251)
	for (duint32 i = 0; i < blk; i++)
	{
		k = i;
		for (int j = 0; j < 255; j++)
		{
			data[j] = in[k];
			k += blk;
		}
		int r = rsc.decode(data);
		if (r < 0)
			DRW_DBG("\nWARNING: dwgRSCodec::decode251I, can't correct all errors");
		k = i * 251;
		for (int j = 0; j < 251; j++)
		{
			out[k++] = data[j];
		}
	}
}

/* two bytes offset */
duint32 dwgCompressor::twoByteOffset(duint32* ll) {
	duint32 offset = 0;
	// read a byte
	duint8 firstByte = bufC[posC++];
	offset = (firstByte >> 2) | (bufC[posC++] << 6);
	*ll = (firstByte & 0x03);
	return offset;
}

/* long compression offset */
duint32 dwgCompressor::longCompressionOffset() {
	duint32 offset = 0;
	// get a bytes
	duint8 ll = bufC[posC++];
	// read until non-zero
	while (ll == 0x00)
	{
		// add 0xff
		offset += 0xFF;
		ll = bufC[posC++];
	}
	offset += ll;
	return offset;
}

duint32 dwgCompressor::long20CompressionOffset() {
	//    duint32 cont = 0;
	duint32 cont = 0x0F;
	duint8 ll = bufC[posC++];
	while (ll == 0x00)
	{
		//        cont += 0xFF;
		ll = bufC[posC++];
	}
	cont += ll;
	return cont;
}

/* get literal length */
duint32 dwgCompressor::literalCount18() {
	duint32 count = 0;
	// get 1 byte
	duint8 ll = bufC[posC++];
	//no literal length, this byte is next opCode
	if (ll > 0x0F)
	{
		posC--;
		return 0;
	}

	if (ll == 0x00)
	{
		// set 0x0f
		count = 0x0F;
		// read next byte
		ll = bufC[posC++];
		//repeat until ll != 0x00
		while (ll == 0x00)
		{
			count += 0xFF;
			ll = bufC[posC++];
		}
	}
	// add the non-zero byte to count
	count += ll;
	// re-add 3 to the result
	count += 3;
	return count;
}

void dwgCompressor::decompress18(duint8* cbuf, duint8* dbuf, duint32 csize, duint32 dsize) {
	bufC = cbuf;
	bufD = dbuf;
	sizeC = csize;
	sizeD = dsize;
	// the lst 2 bytes
	duint8 last2 = bufC[sizeC - 2];
	duint8 last1 = bufC[sizeC - 1];

	duint32 compBytes;     // compressedBytes
	duint32 compOffset;    // compOffset
	duint32 literalCount;  //literalCount

	//current position in compresed buffer
	posC = 0;
	//current position in resulting decompressed buffer
	posD = 0;
	literalCount = literalCount18();

	//copy first literal count bytes
	for (duint32 i = 0; i < literalCount; ++i)
		bufD[posD++] = bufC[posC++];

	while (posC < csize && (posD < dsize + 1))
	{
		//posD < dsize to prevent crash more robust are needed
		duint8 opcode1 = bufC[posC++];  //next opcode
		if (opcode1 == 0x10)
		{
			compBytes = longCompressionOffset() + 9;
			compOffset = twoByteOffset(&literalCount) + 0x3FFF;
			if (literalCount == 0)
				literalCount = literalCount18();
		}
		else if (opcode1 > 0x11 && opcode1 < 0x20)
		{
			compBytes = (opcode1 & 0x0F) + 2;
			compOffset = twoByteOffset(&literalCount) + 0x3FFF;
			if (literalCount == 0)
				literalCount = literalCount18();
		}
		else if (opcode1 == 0x20)
		{
			compBytes = longCompressionOffset() + 0x21;
			compOffset = twoByteOffset(&literalCount);
			if (literalCount == 0)
				literalCount = literalCount18();
			else
				opcode1 = 0x00;  // ? set to 0
		}
		else if (opcode1 > 0x20 && opcode1 < 0x40)
		{
			compBytes = opcode1 - 0x1E;
			compOffset = twoByteOffset(&literalCount);
			if (literalCount == 0) literalCount = literalCount18();
		}
		else if (opcode1 > 0x3F && opcode1 <= 0xff)
		{
			compBytes = ((opcode1 & 0xF0) >> 4) - 1;
			duint8 opcode2 = bufC[posC++];
			compOffset = (opcode2 << 2) | ((opcode1 & 0x0C) >> 2);
			literalCount = opcode1 & 0x03;
			if (literalCount == 0x00) literalCount = literalCount18();
		}
		else if (opcode1 == 0x11)
		{
			posC;
			posD;
			//end of input stream
			return;
		}
		else
		{
			//opcde1 < 0x10 failed
			posC;
			posD;
			return;  //fails, not valid
		}

		// copy "compresed data", TODO Needed verify out of bounds
		duint32 remaining = sizeD - (literalCount + posD);
		if (remaining < compBytes)
		{
			compBytes = remaining;
			DRW_DBG(posC);
			DRW_DBG(posD);
		}

		for (duint32 i = 0, j = posD - compOffset - 1; i < compBytes; i++)
		{
			bufD[posD++] = bufD[j++];
		}

		// copy "uncompresed data", TODO Needed verify out of bounds
		for (duint32 i = 0; i < literalCount; i++)
		{
			bufD[posD++] = bufC[posC++];
		}
	}
	DRW_DBG("WARNING dwgCompressor::decompress, bad out, Cpos: ");
	DRW_DBG(posC);
	DRW_DBG(", Dpos: ");
	DRW_DBG(posD);
	DRW_DBG("\n");
}

void dwgCompressor::decrypt18Hdr(duint8* buf, duint32 size, duint32 offset) {
	duint8 max = size / 4;
	duint32 secMask = 0x4164536b ^ offset;
	duint32* pHdr = (duint32*)buf;
	for (duint8 j = 0; j < max; j++)
		*pHdr++ ^= secMask;
}

/*void dwgCompressor::decrypt18Data(duint8 *buf, duint32 size, duint32 offset){
	duint8 max = size / 4;
	duint32 secMask = 0x4164536b ^ offset;
	duint32* pHdr = (duint32*)buf;
	for (duint8 j = 0; j < max; j++)
		*pHdr++ ^= secMask;
}*/

/**
 * @brief get literal length
 * @param cbuf compressed buff
 * @param opcode operation code
 * @param si current pointer of source index in compressed buff
 */
duint32 dwgCompressor::litLength21(duint8* cbuf, duint8 opcode, duint32* si) {
	duint32 sourceIndex = *si;
	duint32 length = opcode + 8;
	if (length == 0x17)
	{
		duint32 n = cbuf[sourceIndex++];
		length += n;
		if (n == 0xff)
		{
			do
			{
				n = cbuf[sourceIndex++];
				n |= static_cast<duint32>(cbuf[sourceIndex++] << 8);
				length += n;
			} while (n == 0xffff);
		}
	}
	*si = sourceIndex;
	return length;
}

void dwgCompressor::decompress21(duint8* cbuf, duint8* dbuf, duint32 csize, duint32 dsize) {
	duint32 srcIndex = 0;
	duint32 dstIndex = 0;
	duint32 length = 0;
	duint32 sourceOffset;
	duint8 opCode = 0;

	DRW_DBG("\n decompressed size:"); DRW_DBG(dsize);
	DRW_DBG("\ncompressed size:"); DRW_DBG(csize);

	opCode = cbuf[srcIndex++];
	// hight nibble == 2
	// todo replace with single bit check
	//if ((opCode >> 4) == 2)
	if ((opCode & 0xf0) == 0x20)
	{
		srcIndex += 2;
		length = cbuf[srcIndex++] & 0x07;
		if (length == 0)
		{
			DRW_DBG("\ninvalid opCode read");
			return;
		}
	}
	//dstIndex < dsize to prevent crash more robust are needed
	while (srcIndex < csize && (dstIndex < dsize + 1))
		//while (dstIndex < dsize + 1)
	{
		if (length == 0)
			length = litLength21(cbuf, opCode, &srcIndex);
		if (dstIndex + length > dsize)
		{
			DRW_DBG("\ndecompressed error:overflow");
			return;
		}
		copyCompBytes21(cbuf, dbuf, length, srcIndex, dstIndex);
		srcIndex += length;
		dstIndex += length;
		if (dstIndex >= dsize)
			break;  //check if last chunk are compresed & terminate

		length = 0;
		opCode = cbuf[srcIndex++];
		readInstructions21(cbuf, &srcIndex, &opCode, &sourceOffset, &length);
		while (true)
		{
			// prevent crash with corrupted data
			if (sourceOffset > dstIndex)
			{
				DRW_DBG("\nWARNING dwgCompressor::decompress21 => sourceOffset> dstIndex.\n");
				DRW_DBG("csize = ");
				DRW_DBG(csize);
				DRW_DBG("  srcIndex = ");
				DRW_DBG(srcIndex);
				DRW_DBG("\ndsize = ");
				DRW_DBG(dsize);
				DRW_DBG("  dstIndex = ");
				DRW_DBG(dstIndex);
				sourceOffset = dstIndex;
			}
			//prevent crash with corrupted data
			if (length > dsize - dstIndex)
			{
				DRW_DBG("\nWARNING dwgCompressor::decompress21 => length > dsize - dstIndex.\n");
				DRW_DBG("csize = ");
				DRW_DBG(csize);
				DRW_DBG("  srcIndex = ");
				DRW_DBG(srcIndex);
				DRW_DBG("\ndsize = ");
				DRW_DBG(dsize);
				DRW_DBG("  dstIndex = ");
				DRW_DBG(dstIndex);
				length = dsize - dstIndex;
				srcIndex = csize;  //force exit
			}
			sourceOffset = dstIndex - sourceOffset;
			for (duint32 i = 0; i < length; i++)
				dbuf[dstIndex++] = dbuf[sourceOffset + i];

			length = opCode & 7;
			if ((length != 0) || (srcIndex >= csize))
				break;
			opCode = cbuf[srcIndex++];

			if ((opCode >> 4) == 0)
				break;

			if ((opCode >> 4) == 0x0f)
				opCode &= 0x0f;

			readInstructions21(cbuf, &srcIndex, &opCode, &sourceOffset, &length);
		}
	}
	DRW_DBG("\ncsize = ");
	DRW_DBG(csize);
	DRW_DBG("  srcIndex = ");
	DRW_DBG(srcIndex);
	DRW_DBG("\ndsize = ");
	DRW_DBG(dsize);
	DRW_DBG("  dstIndex = ");
	DRW_DBG(dstIndex);
	DRW_DBG("\n");
}

void dwgCompressor::readInstructions21(duint8* cbuf, duint32* si, duint8* oc, duint32* so, duint32* l) {
	duint32 length;
	duint32 srcIndex = *si;
	duint32 sourceOffset;
	unsigned char opCode = *oc;
	switch ((opCode >> 4))
	{
		case 0:
			length = (opCode & 0xf) + 0x13;
			sourceOffset = cbuf[srcIndex++];
			opCode = cbuf[srcIndex++];
			length = ((opCode >> 3) & 0x10) + length;
			sourceOffset = ((opCode & 0x78) << 5) + 1 + sourceOffset;
			break;
		case 1:
			length = (opCode & 0xf) + 3;
			sourceOffset = cbuf[srcIndex++];
			opCode = cbuf[srcIndex++];
			sourceOffset = ((opCode & 0xf8) << 5) + 1 + sourceOffset;
			break;
		case 2:
			sourceOffset = cbuf[srcIndex++];
			sourceOffset = ((cbuf[srcIndex++] << 8) & 0xff00) | sourceOffset;
			length = opCode & 7;
			if ((opCode & 8) == 0)
			{
				opCode = cbuf[srcIndex++];
				length = (opCode & 0xf8) + length;
			}
			else
			{
				sourceOffset++;
				length = (cbuf[srcIndex++] << 3) + length;
				opCode = cbuf[srcIndex++];
				length = (((opCode & 0xf8) << 8) + length) + 0x100;
			}
			break;
		default:
			length = opCode >> 4;
			sourceOffset = opCode & 15;
			opCode = cbuf[srcIndex++];
			sourceOffset = (((opCode & 0xf8) << 1) + sourceOffset) + 1;
			break;
	}
	*oc = opCode;
	*si = srcIndex;
	*so = sourceOffset;
	*l = length;
}

/**
 * @brief copy l bytes long from cbuf to dbuf
 * @param cbuf compressed buff
 * @param dbuf decomressed buff
 * @param l length long
 * @param si source index in cbuff
 * @param di dest index in dbuff
 */
void dwgCompressor::copyCompBytes21(duint8* cbuf, duint8* dbuf, duint32 l, duint32 si, duint32 di) {
	duint32 length = l;
	duint32 dix = di;
	duint32 six = si;

	while (length >= 32)
	{
		//in doc: 16-31, 0-15
		for (duint32 i = six + 24; i < six + 32; i++)
			dbuf[dix++] = cbuf[i];
		for (duint32 i = six + 16; i < six + 24; i++)
			dbuf[dix++] = cbuf[i];
		for (duint32 i = six + 8; i < six + 16; i++)
			dbuf[dix++] = cbuf[i];
		for (duint32 i = six; i < six + 8; i++)
			dbuf[dix++] = cbuf[i];
		six += 32;
		length -= 32;
	}

	switch (length)
	{
		case 0:
			break;
		case 1:  //Ok
			dbuf[dix] = cbuf[six];
			break;
		case 2:  //Ok
			dbuf[dix++] = cbuf[six + 1];
			dbuf[dix] = cbuf[six];
			break;
		case 3:  //Ok
			dbuf[dix++] = cbuf[six + 2];
			dbuf[dix++] = cbuf[six + 1];
			dbuf[dix] = cbuf[six];
			break;
		case 4:  //Ok
			for (int i = 0; i < 4; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 5:  //Ok
			dbuf[dix++] = cbuf[six + 4];//4
			for (int i = 0; i < 4; i++)
				dbuf[dix++] = cbuf[six++];//0-4
			break;
		case 6:  //Ok
			dbuf[dix++] = cbuf[six + 5];//5
			for (int i = 1; i < 5; i++)
				dbuf[dix++] = cbuf[six + i];//1-4
			dbuf[dix] = cbuf[six];//0
			break;
		case 7:
			//in doc: six+5, six+6, 1-5, six+0
			dbuf[dix++] = cbuf[six + 6];//6
			dbuf[dix++] = cbuf[six + 5];//5
			for (int i = 1; i < 5; i++)//1-4
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];//0
		case 8:                          //Ok
			for (int i = 0; i < 8; i++)  //RLZ 4[0],4[4] or 4[4],4[0]
				dbuf[dix++] = cbuf[six++];//0-7
			break;
		case 9:  //Ok
			dbuf[dix++] = cbuf[six + 8];//8
			for (int i = 0; i < 8; i++)//0-7
				dbuf[dix++] = cbuf[six++];
			break;
		case 10:  //Ok
			dbuf[dix++] = cbuf[six + 9];//9
			for (int i = 1; i < 9; i++)//1-8
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];//0
			break;
		case 11:
			//in doc: six+9, six+10, 1-9, six+0
			dbuf[dix++] = cbuf[six + 10];
			dbuf[dix++] = cbuf[six + 9];
			for (int i = 1; i < 9; i++)//1-8
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];//0
			break;
		case 12:  //Ok
			for (int i = 8; i < 12; i++)//8-11
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)//0-7
				dbuf[dix++] = cbuf[six++];
			break;
		case 13:  //Ok
			dbuf[dix++] = cbuf[six + 12];//12
			for (int i = 8; i < 12; i++)//8-11
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)//0-7
				dbuf[dix++] = cbuf[six++];
			break;
		case 14:  //Ok
			dbuf[dix++] = cbuf[six + 13];//13
			for (int i = 9; i < 13; i++)//9-12
				dbuf[dix++] = cbuf[six + i];
			for (int i = 1; i < 9; i++)//1-8
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];//0
			break;
		case 15:
			//in doc: six+13, six+14, 9-12, 1-8, six+0
			dbuf[dix++] = cbuf[six + 14];
			dbuf[dix++] = cbuf[six + 13];
			for (int i = 9; i < 13; i++)//9-12
				dbuf[dix++] = cbuf[six + i];
			for (int i = 1; i < 9; i++)//1-8
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];//0
			break;
		case 16:  //Ok
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 17:  //Seems Ok
			for (int i = 9; i < 17; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix++] = cbuf[six + 8];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 18:
			//in doc: six+17, 1-16, six+0
			dbuf[dix++] = cbuf[six + 17];
			for (int i = 9; i < 17; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 1; i < 9; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix] = cbuf[six];
			break;
		case 19:
			//in doc: 16-18, 0-15
			dbuf[dix++] = cbuf[six + 18];
			dbuf[dix++] = cbuf[six + 17];
			dbuf[dix++] = cbuf[six + 16];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 20:
			//in doc: 16-19, 0-15
			for (int i = 16; i < 20; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 21:
			//in doc: six+20, 16-19, 0-15
			dbuf[dix++] = cbuf[six + 20];
			for (int i = 16; i < 20; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six + i];
			break;
		case 22:
			//in doc: six+20, six+21, 16-19, 0-15
			dbuf[dix++] = cbuf[six + 21];
			dbuf[dix++] = cbuf[six + 20];
			for (int i = 16; i < 20; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 23:
			//in doc: six+20, six+21, six+22, 16-19, 0-15
			dbuf[dix++] = cbuf[six + 22];
			dbuf[dix++] = cbuf[six + 21];
			dbuf[dix++] = cbuf[six + 20];
			for (int i = 16; i < 20; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six + i];
			break;
		case 24:
			//in doc: 16-23, 0-15
			for (int i = 16; i < 24; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 25:
			//in doc: 17-24, six+16, 0-15
			for (int i = 17; i < 25; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix++] = cbuf[six + 16];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 26:
			//in doc: six+25, 17-24, six+16, 0-15
			dbuf[dix++] = cbuf[six + 25];
			for (int i = 17; i < 25; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix++] = cbuf[six + 16];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 27:
			//in doc: six+25, six+26, 17-24, six+16, 0-15
			dbuf[dix++] = cbuf[six + 26];
			dbuf[dix++] = cbuf[six + 25];
			for (int i = 17; i < 25; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix++] = cbuf[six + 16];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 28:
			//in doc: 24-27, 16-23, 0-15
			for (int i = 24; i < 28; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 16; i < 24; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 29:
			//in doc: six+28, 24-27, 16-23, 0-15
			dbuf[dix++] = cbuf[six + 28];
			for (int i = 24; i < 28; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 16; i < 24; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 30:
			//in doc: six+28, six+29, 24-27, 16-23, 0-15
			dbuf[dix++] = cbuf[six + 29];
			dbuf[dix++] = cbuf[six + 28];
			for (int i = 24; i < 28; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 16; i < 24; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 8; i < 16; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 0; i < 8; i++)
				dbuf[dix++] = cbuf[six++];
			break;
		case 31:
			//in doc: six+30, 26-29, 18-25, 2-17, 0-1
			dbuf[dix++] = cbuf[six + 30];
			for (int i = 26; i < 30; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 18; i < 26; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 10; i < 18; i++)
				dbuf[dix++] = cbuf[six + i];
			for (int i = 2; i < 10; i++)
				dbuf[dix++] = cbuf[six + i];
			dbuf[dix++] = cbuf[six + 1];
			dbuf[dix] = cbuf[six];
			break;
		default:
			DRW_DBG("WARNING dwgCompressor::copyCompBytes21, wrong length.\n");
			break;
	}
}

/**
 * @brief return dwgsection based on name
 * @param nameSec string of section
 */
SecEnum::DWGSection SecEnum::getEnum(std::string nameSec) {
	if (nameSec.empty())
	{
		return SecEnum::DWGSection::EMPTY;
	}
	else if (nameSec == "AcDb:Header")
	{
		return SecEnum::DWGSection::HEADER;
	}
	else if (nameSec == "AcDb:Classes")
	{
		return SecEnum::DWGSection::CLASSES;
	}
	else if (nameSec == "AcDb:SummaryInfo")
	{
		return SecEnum::DWGSection::SUMARYINFO;
	}
	else if (nameSec == "AcDb:Preview")
	{
		return SecEnum::DWGSection::PREVIEW;
	}
	else if (nameSec == "AcDb:VBAProject")
	{
		return SecEnum::DWGSection::VBAPROY;
	}
	else if (nameSec == "AcDb:AppInfo")
	{
		return SecEnum::DWGSection::APPINFO;
	}
	else if (nameSec == "AcDb:FileDepList")
	{
		return SecEnum::DWGSection::FILEDEP;
	}
	else if (nameSec == "AcDb:RevHistory")
	{
		return SecEnum::DWGSection::REVHISTORY;
	}
	else if (nameSec == "AcDb:Security")
	{
		return SecEnum::DWGSection::SECURITY;
	}
	else if (nameSec == "AcDb:AcDbObjects")
	{
		return SecEnum::DWGSection::OBJECTS;
	}
	else if (nameSec == "AcDb:ObjFreeSpace")
	{
		return SecEnum::DWGSection::OBJFREESPACE;
	}
	else if (nameSec == "AcDb:Template")
	{
		return SecEnum::DWGSection::TEMPLATE;
	}
	else if (nameSec == "AcDb:Handles")
	{
		return SecEnum::DWGSection::HANDLES;
	}
	else if (nameSec == "AcDb:AcDsPrototype_1b")
	{
		return SecEnum::DWGSection::PROTOTYPE;
	}
	else if (nameSec == "AcDb:AuxHeader")
	{
		return SecEnum::DWGSection::AUXHEADER;
	}
	else if (nameSec == "AcDb:Signature")
	{
		return SecEnum::DWGSection::SIGNATURE;
	}
	else if (nameSec == "AcDb:AppInfoHistory")
	{
		//in R2007
		return SecEnum::DWGSection::APPINFOHISTORY;
	}
	else if (nameSec == "AcDb:Extended Entity Data")
	{
		return DWGSection::EXTEDATA;
	}
	else if (nameSec == "AcDb:PROXY ENTITY GRAPHICS")
	{
		return DWGSection::PROXYGRAPHICS;
	}
	return SecEnum::DWGSection::UNKNOWNS;
}