#include "common/IFileStream.h"

IDebugLog	gLog("hsdump.log");

#pragma pack (push, 1)
struct GCHeader
{
	char	sig[4];
	UInt32	unk1;
	UInt32	unk2;
	UInt16	width;
	UInt16	height;
	UInt32	unk3;
	UInt32	unk4;

	void Swap(void)
	{
		width = Swap16(width);
		height = Swap16(height);
	}
};

struct TGAHeader
{
	UInt8	identSize;
	UInt8	palType;
	UInt8	imgType;
	UInt16	palStart;
	UInt16	palLen;
	UInt8	palBpp;
	UInt16	xOrigin;
	UInt16	yOrigin;
	UInt16	width;
	UInt16	height;
	UInt8	bpp;
	UInt8	flags;
};

int main(int argc, char ** argv)
{
	if(argc != 3)
	{
		_ERROR("usage: hsdump [in] [out]");
		return -1;
	}

	IFileStream	src, dst;
	if(src.Open(argv[1]) && dst.Create(argv[2]))
	{
		GCHeader	header;

		src.ReadBuf(&header, sizeof(header));
		header.Swap();

		if((header.sig[0] == 'G') && (header.sig[1] == 'C'))
		{
			_MESSAGE("size %dx%d", header.width, header.height);

			TGAHeader	tgaHeader;
			memset(&tgaHeader, 0, sizeof(tgaHeader));
			tgaHeader.imgType = 2;	// rgb
			tgaHeader.width = header.width;
			tgaHeader.height = header.height;
			tgaHeader.bpp = 16;
			tgaHeader.flags = 0x21;	// y goes down, 1 alpha bit

			dst.WriteBuf(&tgaHeader, sizeof(tgaHeader));

			UInt32	expectedLength = header.width * header.height * 2;
			UInt32	actualLength = src.GetRemain();
			if(expectedLength < actualLength)
				actualLength = expectedLength;

			IDataStream::CopySubStreams(&dst, &src, actualLength);

			UInt32	padding = expectedLength - actualLength;
			for(UInt32 i = 0; i < padding; i++)
				dst.Write8(0);
		}
		else
		{
			_ERROR("bad sig %.4s", header.sig);
		}
	}
	else
	{
		_ERROR("couldn't open src file %s", argv[1]);
	}
}

#pragma pack (pop, 1)

// 47432000 001CA018 00000000 03940400 00000000 001CA000
// 47432000 00020018 00000000 01000100 00000000 00020000
