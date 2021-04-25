#include <cctype>
#include "common/IFileStream.h"

void main(int argc, char ** argv)
{
	if(argc != 3)
	{
		_FATALERROR("usage: firebeatdump [in] [out]");
		return;
	}

	IFileStream	in;
	IFileStream	out;

	if(!in.Open(argv[1]))
	{
		_FATALERROR("couldn't open input file (%s)", argv[1]);
		return;
	}

	if(!out.Create(argv[2]))
	{
		_FATALERROR("couldn't open output file (%s)", argv[2]);
		return;
	}

	UInt32	header = in.Read32();

	_MESSAGE("header = %08X", header);

	UInt8	history[0x1000];
	memset(&history, 0, sizeof(history));
	UInt32	historyOffset = 0x0FEE;	// wtf

	while(in.GetRemain() >= 9)
	{
		UInt32	offset = in.GetOffset();
		UInt8	select = in.Read8();

		_MESSAGE("%08X: %02X", offset, select);

		gLog.Indent();

		try
		{
			for(UInt8 bit = 0; bit < 8; bit++)
			{
				if(select & (1 << bit))
				{
					UInt8	data = in.Read8();

					if(std::isprint(data))
						_MESSAGE("%c (%02X)", data, data);
					else
						_MESSAGE("%02X", data);

					out.Write8(data);

					history[historyOffset] = data;
					historyOffset = (historyOffset + 1) & 0x0FFF;
				}
				else
				{
					UInt8	data0 = in.Read8();
					UInt8	data1 = in.Read8();

					UInt8	chunkLen = (data1 & 0x0F) + 3;
					UInt16	offset = ((data1 & 0xF0) << 4) | data0;

					_MESSAGE("%02X %02X (len = %02X, offset = %04X)", data0, data1, chunkLen, offset);

					for(UInt32 i = 0; i < chunkLen; i++)
					{
						UInt8	data = history[offset];
						
						out.Write8(data);

						history[historyOffset] = data;
						historyOffset = (historyOffset + 1) & 0x0FFF;

						offset = (offset + 1) & 0x0FFF;
					}
				}
			}
		}
		catch(...)
		{
			_MESSAGE("### EOF");
		}

		gLog.Outdent();
	}
}
