unit BMS;
// ================================================
// BMS.PAS
// Created 2007.02.18
//
// Supports reading *.bms, *.bme and related files.
//
// Based on the 'BMS_Open' procedure from earlier
// versions of Be-Pachi Music.
//
// Contains functions and procedures for dealing
// with converting data inside a Be-Music Source
// file into a useable format.
//
// Output data can be found in the DB.pas arrays.
// ------------------------------------------------

interface

uses
  SysUtils, Base36, DB_Main;

function  CountEvents(Path_File: String): Integer;
function  CountTempoChanges(Path_File: String): Word;
function  FindFirstSpace(Data: String): Word;
function  FindHighestReference(Path_File, Reference_Type: String): Word;
function  OpenBMS(Path_File: String): Boolean;

procedure AddMeasuresToEventList;
procedure AddTempoChangesToEventList;
procedure AdjustMeasureTimes;
procedure ApplyTempoChanges;
procedure CollectCustomTempoChanges(Path_File: String);
procedure CollectWavBmpAssigns(Path_File: String);
procedure ConvertDataLine(BMS_Measure: Word; BMS_Function: Byte; BMS_DataLine: String);
procedure ConvertPositionsIntoTime;
procedure ConvertTempoChangesIntoTime;
procedure ParseMainDataField(Path_File: String);
procedure ReadHeader(Path_File: String);
procedure SetMeasureStartTimes;

implementation



// ----------------------------------------------------------



function CountEvents(Path_File: String): Integer;
// =========================================================
// Returns the total number of objects (not just notes but
// other events and also measure lines) in a BMS file.
//
// This also retrieves the total number of measures as well,
// which is placed in Measure_Count for reference.
// ---------------------------------------------------------
var
  BMS_Measure:    word;
  BMS_File:       TextFile;
  Data:           string;
begin

  Result := 0;
  Measure_Count := 0;

  if not FileExists(Path_File) then
  begin
    Result := -1;
    exit;
  end;

  AssignFile(BMS_File, Path_File);
  Reset(BMS_File);

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);
    Data := TrimRight(UpperCase(Data));

    if Length(Data) > 1 then
      if Data[1] = '#' then
        if Data[2] in ['0'..'9'] then
    begin
      BMS_Measure := StrToInt(copy(Data, 2, 3));
      Data := copy(Data, 8, Length(Data) - 7);

      if BMS_Measure > Measure_Count then
        Measure_Count := BMS_Measure;

      if Length(Data) > 1 then
        repeat
          if (Data[1] <> '0') or (Data[2] <> '0') then inc(Result);
          Data := copy(Data, 3, Length(Data) - 2);
        until Data = '';

    end;
  end;

  CloseFile(BMS_File);

  inc(Result, Result); // Double the amount to allow for the sound change events

  inc(Measure_Count);  // Include the measure immediately after the song ends

  inc(Result, Measure_Count); // Make room for the Measure lines, which will be
                              // converted into events, and added to the note
                              // arrays

  inc(Result, 218); // Make room for the end-of-song line graph

  inc(Result, 100); // And some over-flow space, just to be safe! 

end;



// ----------------------------------------------------------------------------



function CountTempoChanges(Path_File: String): Word;
// =================================
// Returns the number of times a BMS
// song changes tempo.
// ---------------------------------
var
  BMS_File: TextFile;
  Data: String;
  i, Data_Length: Word;
begin
  Result := 0;

  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);

    Data_Length := Length(Data);

    if Data_Length > 8 then
      if Data[1] = '#' then
        if Data[2] in ['0'..'9'] then
          if (copy(Data, 5, 2) = '03') or
             (copy(Data, 5, 2) = '08') then
    begin
      i := 8;
      while i < Data_Length do
      begin
        if (Data[i] <> '0') or (Data[i + 1] <> '0') then
          inc(Result);
        inc(i, 2);
      end;
    end;

  end;

  CloseFile(BMS_File);
end;



// ----------------------------------------------------------------------------



function FindFirstSpace(Data: String): Word;
// =======================================
// Returns the location of the first space
// (ASCII value 32) in a string.
// ---------------------------------------
var
  Len: word;
begin
  Result := 0;
  Len := Length(Data);
  if Len = 0 then exit;
  while (Result < Len) and (Data[Result] <> ' ') do inc(Result);
end;



// ----------------------------------------------------------------------------



function FindHighestReference(Path_File, Reference_Type: String): Word;
// ==============================================
// Returns the highest location used by the
// reference type (#WAV, #BMP, #BPM, or other)
//
// For example: if a BMS contains the following
// 5 #WAV reference lines:
//
//   #WAV01 1.wav
//   #WAV02 2.wav
//   #WAV03 3.wav
//   #WAV04 4.wav
//   #WAV0F 5.wav
//
// ...the function would return 15 (0F in base36)
// ----------------------------------------------
var
  Current_Value, Reference_Length: Word;
  BMS_File:                        TextFile;
  Data:                            String;
begin

  Result := 0;
  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  Reference_Length := Length(Reference_Type);

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);
    if Length(Data) > Reference_Length then
      if copy(Data, 1, Reference_Length) = Reference_Type then
    begin
      Current_Value := Base36.Base36ToInt(copy(Data, Reference_Length + 1, 2));
      if Current_Value > Result then Result := Current_Value;
    end;
  end;

  CloseFile(BMS_File);
end;



// ----------------------------------------------------------



function OpenBMS(Path_File: String): Boolean;
// ===================================================
// Opens a BMS file through the use of sub-procedures,
// which convert and load the data into the various
// arrays and variables.
// ---------------------------------------------------
begin
  Result := False;
  BGA_OK := False;
  Event_Count := CountEvents(Path_File);
  if Event_Count < 0 then exit;

  ReadHeader(Path_File);

  WAV_Count               := FindHighestReference(Path_File, '#WAV');
  BMP_Count               := FindHighestReference(Path_File, '#BMP');
  CustomTempoChange_Count := FindHighestReference(Path_File, '#BPM');
  TempoChange_Count       := CountTempoChanges(Path_File);

  InitializeArrays;

  CollectCustomTempoChanges(Path_File);
  CollectWavBmpAssigns(Path_File);

  ParseMainDataField(Path_File);

  SetMeasureStartTimes;
  ConvertPositionsIntoTime;
  ConvertTempoChangesIntoTime;
  AdjustMeasureTimes;
  SortTempoChangeArrays;
  AddMeasuresToEventList;
  AddTempoChangesToEventList;
  AssignNotesToMeasure;

  Result := True;
end;



// ----------------------------------------------------------------------------



procedure AddMeasuresToEventList;
// ============================================
// This converts the measure lines into events,
// which are then added into the note arrays.
// --------------------------------------------
var
  i, ff: Integer;
begin

  ff := GetFirstFreeEvent;
  if ff < 0 then exit; // Error

  for i := 0 to Measure_Count - 1 do
  begin
    Note_Column[ff] := 10;
    Note_Time[ff] := Measure_Time[i];
    inc(ff);
  end;

end;



// ----------------------------------------------------------------------------



procedure AddTempoChangesToEventList;
// ===========================================
// This converts tempo changes into events and
// adds those to the note arrays.
// -------------------------------------------
var
  i, ff: Integer;
begin

  ff := GetFirstFreeEvent;
  if ff < 0 then exit; // Error

  for i := 0 to TempoChange_Count - 1 do
  begin
    Note_Column[ff] := 8;
    Note_Time[ff] := TempoChange_Position[i];
    Note_Sound[ff] := Trunc(TempoChange_Amount[i]);
    Note_Position[ff] := TempoChange_Amount[i]; // Provides better accuracy when
                                                // using the Be-Music tempo
                                                // change effect!
    inc(ff);
  end;

end;



// ----------------------------------------------------------------------------



procedure AdjustMeasureTimes;
// =========================================
// This procedure applies the song's tempo
// to the current values in the Measure_Time
// array.
// -----------------------------------------
var
  i: Integer;
begin
  for i := 0 to Measure_Count - 1 do
    Measure_Time[i] := Measure_Time[i] / Tempo * 240;
end;



// ----------------------------------------------------------------------------



procedure ApplyTempoChanges;
// ========================================================
// This modifies the trigger time of all objects that exist
// after each tempo change, including later tempo changes.
//
// Only used for "BPM Classic Style" tempo change effect
// or Constant Speed is selected.
//
// Not needed for normal "Be-Music" tempo change effect.
// --------------------------------------------------------
var
  i, i2: Integer;
  Current_Tempo: Single;
begin
  Current_Tempo := Tempo;
  for i := 0 to TempoChange_Count - 1 do
  begin
    if TempoChange_Amount[i] <> 0 then
    begin

      for i2 := 0 to Event_Count - 1 do
        if Note_Time[i2] > TempoChange_Position[i] then
          Note_Time[i2] := (Note_Time[i2] - TempoChange_Position[i]) / TempoChange_Amount[i] * Current_Tempo + TempoChange_Position[i];

      for i2 := 0 to Measure_Count - 1 do
        if Measure_Time[i2] > TempoChange_Position[i] then
          Measure_Time[i2] := (Measure_Time[i2] - TempoChange_Position[i]) / TempoChange_Amount[i] * Current_Tempo + TempoChange_Position[i];

      for i2 := 0 to TempoChange_Count - 1 do
        if TempoChange_Position[i2] > TempoChange_Position[i] then
          TempoChange_Position[i2] := (TempoChange_Position[i2] - TempoChange_Position[i]) / TempoChange_Amount[i] * Current_Tempo + TempoChange_Position[i];

    end;
    Current_Tempo := TempoChange_Amount[i];
  end;
end;



// ----------------------------------------------------------------------------



procedure CollectCustomTempoChanges(Path_File: String);
// ===============================
// Looks for and retrieves all of
// the customized tempo changes,
// which are prefixed with #BPMxx.
// -------------------------------
var
  BMS_File:    TextFile;
  Data:        String;
  Data_Length: Word;
begin
  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);
    Data_Length := Length(Data);

    if Data_Length >= 8 then
      if copy(Data, 1, 4) = '#BPM' then
        if Data[5] <> ' ' then
          CustomTempoChange_Amount[Base36.Base36ToInt(copy(Data, 5, 2))] := StrToFloat(copy(Data, 8, Data_Length - 7));
  end;

  CloseFile(BMS_File);
end;



// ----------------------------------------------------------------------------



procedure CollectWavBmpAssigns(Path_File: String);
// ========================================
// This looks for #WAV and #BMP file assign
// lines and stores the file name in the
// WAV_Name / BMP_Name arrays.
//
// Even though the file assign lines expect
// WAV or BMP files, the actual file types
// can be any format supported by the BMS
// player.
// ----------------------------------------
var
  BMS_File:    TextFile;
  Data:        String;
  Data_Length: Word;
begin

  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);
    Data_Length := Length(Data);
    if Data_Length >= 8 then
    begin
      if copy(Data, 1, 4) = '#BMP' then
        BMP_Name[Base36.Base36ToInt(copy(Data, 5, 2))] := copy(Data, 8, Data_Length - 7);
      if copy(Data, 1, 4) = '#WAV' then
        WAV_Name[Base36.Base36ToInt(copy(Data, 5, 2))] := copy(Data, 8, Data_Length - 7);
    end;
  end;
  CloseFile(BMS_File);
end;



// ----------------------------------------------------------



procedure ConvertDataLine(BMS_Measure: Word; BMS_Function: Byte; BMS_DataLine: String);
// ==============================================
// Converts a series of numerical data from a BMS
// into events (including type, column, position)
//
// Output data will be stored in all the defined
// variable arrays.
// ----------------------------------------------
var
  i, Note, Data_Length, Denominator: Word;
begin
  Data_Length := Length(BMS_DataLine);
  if Data_Length and 1 = 1 then exit; // Odd data length? Bad! Skip the line!

  Denominator := Data_Length shr 1;
  i := 1;

  while i < Data_Length do
  begin
    Note := Base36.Base36ToInt(copy(BMS_DataLine, i, 2));
    if Note > 0 then
    begin

      case BMS_Function of
        3: begin // ----------------------------- Tempo Change
             TempoChange_Measure[TempoChange_Pointer] := BMS_Measure;
             TempoChange_Position[TempoChange_Pointer] := (i shr 1) / Denominator;
             TempoChange_Amount[TempoChange_Pointer] := StrToInt('$' + copy(BMS_DataLine, i, 2)); // Base16-to-Decimal
             inc(TempoChange_Pointer);
           end;
        8: begin // ----------------------------- Custom Tempo Change
             TempoChange_Measure[TempoChange_Pointer] := BMS_Measure;
             TempoChange_Position[TempoChange_Pointer] := (i shr 1) / Denominator;
             TempoChange_Amount[TempoChange_Pointer] := CustomTempoChange_Amount[Note];
             inc(TempoChange_Pointer);
           end;
        4, 6, 7: begin // ----------------------- BG Animations
                   BGA_OK := True;
                   Note_Sound[NoteArray_Pointer]    := Note;
                   Note_Measure[NoteArray_Pointer]  := BMS_Measure;
                   Note_Position[NoteArray_Pointer] := (i shr 1) / Denominator;
                   Note_Column[NoteArray_Pointer]   := BMS_Function;
                   inc(NoteArray_Pointer);
                   if BMP_Name[Note] = '' then
                     BMP_Name[Note] := ' '; // Prepare blank BGA image
                 end;
         1, 11..49: begin // -------------------- Key Sound
                      Note_Sound[NoteArray_Pointer] := Note;
                      Note_Measure[NoteArray_Pointer] := BMS_Measure;
                      Note_Position[NoteArray_Pointer] := (i shr 1) / Denominator;
                      Note_Column[NoteArray_Pointer] := BMS_Function;
                      if BMS_Function in [11..29] then
                        inc(Measure_NoteAmount[Note_Measure[NoteArray_Pointer]]);
                      if BMS_Function = 1 then
                        Note_Flag[NoteArray_Pointer] := Note_Flag[NoteArray_Pointer] or Flag_AutoPlay;
                      inc(NoteArray_Pointer);
                      if WAV_Name[Note] = '' then
                        WAV_Name[Note] := ' '; // Prepare blank WAV sound
                    end;
      end;
    end;
    inc(i, 2);
  end;

end;



// ----------------------------------------------------------------------------



procedure ConvertPositionsIntoTime;
// ==============================================
// This converts Event Position values
// into the time the event will be
// triggered.
//
// Formula:
// Time in Seconds = 240 * (Measure_Size / Tempo)
// ----------------------------------------------
var
  i: Integer;
begin
  for i := 0 to Event_Count - 1 do
    Note_Time[i] := (Measure_Time[Note_Measure[i]] / Tempo * 240) +
                    (Measure_Size[Note_Measure[i]] / Tempo * 240) * Note_Position[i];

  FillChar(Note_Position[0], Event_Count shl 2, 0); // Removing now un-needed
end;                                                // data



// ----------------------------------------------------------------------------



procedure ConvertTempoChangesIntoTime;
// =======================================
// Similar to the above procedure, this
// converts Tempo Change positions into
// the time the tempo change takes effect.
//
// Same formula used as the above.
// ---------------------------------------
var
  i: Integer;
begin
  for i := 0 to TempoChange_Count - 1 do
    TempoChange_Position[i] := (Measure_Time[TempoChange_Measure[i]] / Tempo * 240)
                             + (Measure_Size[TempoChange_Measure[i]] / Tempo * 240)
                             * TempoChange_Position[i];
end;



// ----------------------------------------------------------



procedure ParseMainDataField(Path_File: String);
// ===============================================
// Reads the numerical prefixed BMS command lines
// and prepares the data for conversion into notes
// and other events.
// -----------------------------------------------
var
  BMS_File:           TextFile;
  Data, BMS_DataLine: String;
  BMS_Measure:        Word;
  BMS_Function:       Byte;
begin

  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  NoteArray_Pointer := 0;   // Start from the beginning!
  TempoChange_Pointer := 0;

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data);

    if Length(Data) >= 8 then
      if Data[1] = '#' then
        if Data[2] in ['0'..'9'] then
    begin

      BMS_Measure := StrToInt(copy(Data, 2, 3));
      BMS_Function := StrToInt(copy(Data, 5, 2));
      BMS_DataLine := copy(Data, 8, Length(Data) - 7);

      case BMS_Function of
        1, 3, 4, 6, 7, 8,
        11..19,
        21..49: ConvertDataLine(BMS_Measure, BMS_Function, BMS_DataLine);
        2:      Measure_Size[BMS_Measure] := StrToFloat(BMS_DataLine);
      end;

    end;
  end;

  CloseFile(BMS_File);
end;



// ----------------------------------------------------------------------------



procedure ReadHeader(Path_File: String);
// =========================================
// Obtains info about the song, ignoring the
// actual note data, as well as the WAV and
// BMP file name references.
// -----------------------------------------
var
  BMS_File: TextFile;
  SpacePosition, Len: Word;
  Data1, Data2: String;
begin

  Tempo := 120;
  Title := '';
  Genre := '';
  Artist := '';
  Players := 0;
  Judgement := 0;
  PlayLevel := 0;

  AssignFile(BMS_File, Path_File); // CountEvents function already checked
  Reset(BMS_File);                 // if the file existed, so no need to here.

  while not EOF(BMS_File) do
  begin
    ReadLn(BMS_File, Data1);

    Len := Length(Data1);
    SpacePosition := FindFirstSpace(Data1);

    if Len > SpacePosition then
      if Data1[SpacePosition] = ' ' then
    begin
      Data2 := copy(Data1, SpacePosition + 1, (Len - SpacePosition) + 1);
      Data1 := copy(Data1, 1, SpacePosition - 1);
    end;

    if Data1 = '#GENRE'     then Genre := Data2;
    if Data1 = '#ARTIST'    then Artist := Data2;
    if Data1 = '#TITLE'     then Title := Data2;

    if Data2 <> '' then
      if (Data2[1] < '0') or   // Some songs have non-numerical values for the
         (Data2[1] > '9') then // below lines (such as #PLAYLEVEL ?).
        Data2 := '0';          // This works around those weird lines.

    if Data1 = '#PLAYER'    then Players := StrToInt(Data2);
    if Data1 = '#BPM'       then Tempo := StrToFloat(Data2);
    if Data1 = '#RANK'      then Judgement := StrToInt(Data2);
    if Data1 = '#PLAYLEVEL' then PlayLevel := StrToInt(Data2);

  end;

  CloseFile(BMS_File);
end;



// ----------------------------------------------------------------------------



procedure SetMeasureStartTimes;
// =====================================
// This adds up the total sizes of all
// in-use measures.
//
// Un-used measures with negative values
// will be ignored.
// -------------------------------------
var
  i:          Integer;
  Total_Size: Single;
begin
  Total_Size := 0;
  for i := 0 to Measure_Count - 1 do
  begin
    Measure_Time[i] := Total_Size;
    Total_Size := Total_Size + Measure_Size[i]; // No need to abs(Measure_Size),
  end;                                          // was done in MarkUsedMeasures!
end;



// ----------------------------------------------------------------------------



end.
