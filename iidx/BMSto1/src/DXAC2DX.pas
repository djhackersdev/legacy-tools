unit DXAC2DX;
// ==========================================
// DXAC2DX.pas
// Created 2007.04.07
//
// Compiles all the WAV sounds of a BMS song
// and combines them into 1 .2dx file.
//
// Thanks to Kitaru and L for 2dx info.
// ------------------------------------------

interface

uses
  SysUtils, Classes, DB_Main, Utils;

var
  DX2DXStream: TMemoryStream;


function Save2DX(Path_File: String): Boolean;
function Create2DX(Path_File: String): Boolean;


implementation



// ----------------------------------------------------------------------------



function Save2DX(Path_File: String): Boolean;
// =====================================
// Creates a 2DX file through the use of
// other functions and procedures, then
// saves it.
// -------------------------------------
begin
  Create2DX(Path_File);
  DX2DXStream.SaveToFile(Path_File);
  DX2DXStream.Free;
  Result := True;
end;



// ----------------------------------------------------------------------------



function Create2DX(Path_File: String): Boolean;
// ====================================
// Generates a .2dx combined audio file
// from the currently loaded BMS, and
// keeps the data stream in memory.
// ------------------------------------
const
  TwoDX9:      Array[1..4] of Char    = '2DX9';
  HeaderSize:                 Integer = 24;
  Unknown1:    Array[1..4] of Char    = '02'#255#255; // Maybe L/R Volume?
  Unknown2:    Array[1..4] of Char    = #64#0#1#0;    // WAV format info?
  Unknown3:    Array[1..4] of Char    = #0#0#0#0;     // No idea...

var
  i, TotalWavs,
  CurrentOffset,
  WavSize:       Integer;
  Wav:           TFileStream;
  L:             Word;
  Text:          String;
begin

  DX2DXStream := TMemoryStream.Create;

  Result := False;                                  // At least 1 WAV is
  TotalWavs := GetLastWavNumber;                    // needed for 2dx file
  if TotalWavs = 0 then exit;                       // creation.


  // ======================================
  // Create and write the .2dx header here.
  // --------------------------------------

  Text := copy(Artist, 1, 16);                      // First 16 bytes of a 2dx
  L := Length(Text);                                // file are ignored.
  if L < 16 then Text := Text + DupChar(16 - L, 0); // BMSto1 will use the BMS
  DX2DXStream.WriteBuffer(Text[1], 16);             // Artist's name here.

  TotalWavs := GetLastWavNumber;                    // Location of the first
  CurrentOffset := 72 + TotalWavs * 4;              // keysound is next.
  DX2DXStream.WriteBuffer(CurrentOffset, 4);        // Its location is usually
                                                    // Header Size + number of
                                                    // 4-byte WAV entries.

  DX2DXStream.WriteBuffer(TotalWavs, 4);            // Write the total number
                                                    // of WAV sounds.

  Text := copy(TitleOnly(Title), 1, 48);            // Next 48 bytes are also
  L := Length(Text);                                // ignored.
  if L < 48 then Text := Text + DupChar(48 - L, 0); // The BMS Title will be
  DX2DXStream.WriteBuffer(Text[1], 48);             // entered here.


  // ================================
  // Now for the .2dx WAV entry list.
  // --------------------------------
  for i := 1 to TotalWavs do
  begin
    DX2DXStream.WriteBuffer(CurrentOffset, 4);             // In a 2dx file,
    WavSize := FileSize2(NoFile(Path_File) + Wav_Name[i]); // WAV files have 24
    inc(CurrentOffset, WavSize + 24);                      // extra bytes added.
  end;


  // ================================
  // Now for the WAV files + headers.
  // --------------------------------
  for i := 1 to TotalWavs do
  begin

    DX2DXStream.WriteBuffer(TwoDX9[1], 4);                 // Header begins with
    DX2DXStream.WriteBuffer(HeaderSize, 4);                // '2DX9'. Size of
                                                           // header is 24 bytes

    WavSize := FileSize2(NoFile(Path_File) + Wav_Name[i]); // Actual WAV file
    DX2DXStream.WriteBuffer(WavSize, 4);                   // size (no header)

    DX2DXStream.WriteBuffer(Unknown1,  4);                 // Volume/pan?
    DX2DXStream.WriteBuffer(Unknown2,  4);                 // WAV Format info?
    DX2DXStream.WriteBuffer(Unknown3,  4);                 // Null?

    Wav := TFileStream.Create(NoFile(Path_File) + Wav_Name[i], fmOpenRead);
    DX2DXStream.CopyFrom(Wav, WavSize);
    Wav.Free;

  end;

  Result := True;
end;



// ----------------------------------------------------------------------------



end.
