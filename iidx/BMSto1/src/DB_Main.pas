unit DB_Main;
// ===================================================
// DB_Main.pas
// Created 2007.02.24
//
// This file contains the various arrays and variables
// for storing converted note/event data.
// ---------------------------------------------------

interface

const
  Flag_Hidden:   byte = 1; // Constants used for Note_Flag[]
  Flag_Judged:   byte = 2; //
  Flag_Autoplay: byte = 4; //
  Flag_Player1:  byte = 8; //

var
  Note_Time:     array of Single;            // How long before the note is played.
  Note_Sound:    array of Word;              // # of sound file to play.
  Note_Column:   array of Byte;              // The column the note appears in.
  Note_Measure:  array of Word;              // The measure the note appears in.
  Note_Position: array of Single;            // Where in the measure it is.
                                             //   0 meaning measure start, and
                                             //   0.999~ meaning real close to
                                             //   the next measure line.
  Note_Flag:     array of Byte;              // What options are set on the note.
                                             //
                                             //   Byte and 1: Hidden. Don't Display!
                                             //   Byte and 2: Judged. Don't re-judge!
                                             //   Byte and 4: Autoplay.
                                             //
                                             //   For use later:
                                             //   Byte and 8: Player number?

  TempoChange_Measure: array of Word;        // Where the tempo change is.
  TempoChange_Position: array of Single;     // How far above the divider.
  TempoChange_Amount: array of Single;       // Amount to change tempo to.

  CustomTempoChange_Amount: array of Single; // For the #BPMxx lines.

  Measure_Time: array of Single;             // Divider line's placing.
  Measure_Size: array of Single;             // Distance between 2 lines.
  Measure_NoteAmount: array of Byte;         //
  Measure_HitNotes: array of Byte;           //
                                             // These 4 arrays are only
                                             // used when loading BMS
                                             // format songs.

  WAV_Name: array of string;                 // Name of sounds used in the song.
  BMP_Name: array of string;                 // Name of images used in the song.

  KeyAmount:                                  Byte; // (5, 7, 9, 10, 14?)

  Tempo:                                      Single;
  Event_Count:                                Integer;
  Title, Artist, Genre, FileName:             String;
  Measure_Count, WAV_Count, BMP_Count,
  TempoChange_Count, CustomTempoChange_Count,
  NoteArray_Pointer, TempoChange_Pointer,
  TotalPlayableNotes, TotalNotes:             Word;
  Players, PlayLevel, Judgement:              Byte;
  BGA_OK:                                     Boolean;


function  GetFirstFreeBMP: Integer;
function  GetFirstFreeEvent: Integer;
function  GetFirstFreeWAV: Integer;
function  GetKeyAmount: Byte;
function  GetLastWavNumber: Word;
function  ReCalculatePlayableNotes: Word;

procedure AssignNotesToMeasure;
procedure FreeArrays;
procedure InitializeArrays;
procedure SortBmpNames;
procedure SortEvents;
procedure SortTempoChangeArrays;
procedure SortWavNames;
procedure SwapBmpNames(X1, X2: Word);
procedure SwapEvents(X1, X2: Word);
procedure SwapTempoChanges(X1, X2: Word);
procedure SwapWavNames(X1, X2: Word);


implementation



// ----------------------------------------------------------------------------



function GetFirstFreeBMP: Integer;
// ==============================================
// Returns the first un-used location in the BMP
// name array.
//
// If all locations are in use, this will return
// a value of -1.
// ----------------------------------------------
var
  i: Integer;
begin
  Result := -1;

  for i := BMP_Count downto 1 do
    if BMP_Name[i] = '' then
      Result := i;

end;



// ----------------------------------------------------------------------------



function GetFirstFreeEvent: Integer;
// ==============================================
// Returns the first un-used location in the note
// variable arrays.
//
// If all locations are in use, this will return
// a value of -1.
// ----------------------------------------------
var
  i: Integer;
begin
  Result := -1;

  for i := Event_Count - 1 downto 0 do
    if Note_Column[i] = 0 then
      Result := i;

end;



// ----------------------------------------------------------------------------



function GetFirstFreeWAV: Integer;
// ==============================================
// Returns the first un-used location in the WAV
// name array.
//
// If all locations are in use, this will return
// a value of -1.
// ----------------------------------------------
var
  i: Integer;
begin
  Result := -1;

  for i := WAV_Count downto 1 do
    if WAV_Name[i] = '' then
      Result := i;

end;



// ----------------------------------------------------------------------------



function GetKeyAmount: Byte;
// =========================================
// Returns the play mode of the loaded song.
// (5 or 7 keys)
// -----------------------------------------
var
  i: Integer;
  Key: Array[11..29] of Boolean;
begin

  Result := 5; // Default return value

  for i := 11 to 29 do
    Key[i] := False;

  for i := 0 to Event_Count - 1 do
    if Note_Column[i] in [11..29] then
      Key[Note_Column[i]] := True;

  if Key[18] or Key[19] or Key[28] or Key[29] then
    Result := 7;

  if Players in [2..3] then
    inc(Result, Result);    // from 5-7 to 10-14

end;



// ----------------------------------------------------------------------------



function GetLastWavNumber: Word;
// ===================================
// Looks for and returns the highest
// WAV assign number currently in use.
// -----------------------------------
var
  i: Word;
begin
  Result := 0;

  for i := Wav_Count downto 1 do
    if Wav_Name[i] <> '' then
      if Result = 0 then
        Result := i;
end;



// ----------------------------------------------------------------------------



function ReCalculatePlayableNotes: Word;
// ===================================================
// Does what it says.
//
// This is useful after using the DB_XForm procedures.
// ---------------------------------------------------
var
  i: Word;
begin
  Result := 0;

  for i := 0 to Event_Count - 1 do
    if Note_Column[i] in [11..29] then
      inc(Result);

end;



// ----------------------------------------------------------------------------



procedure AssignNotesToMeasure;
// ==============================================
// This procedure sets the notes' "measure" value
// to the number of the measure the note exists
// in.
//
// This is used so Be-Pachi mode can detect when
// all notes in a measure have been played.
//
// This works best if the event/note arrays have
// been sorted with SortEvents procedure first.
// ----------------------------------------------
var
  i:               Integer;
  Current_Measure: Word;
begin

  Current_Measure := 0;

  for i := 0 to Event_Count - 1 do
  begin

    case Note_Column[i] of
      10: begin
            Measure_NoteAmount[Current_Measure] := 0;
            inc(Current_Measure);
          end;
      else begin
             Note_Measure[i] := Current_Measure;
             if Note_Column[i] in [11..29] then inc(Measure_NoteAmount[Current_Measure]);
           end;
    end;

  end;

end;



// ----------------------------------------------------------



procedure FreeArrays;
// ===================================================
// Sets all arrays to nil, freeing up any memory used,
// and erases all stored array data as well.
// ---------------------------------------------------
begin

  Note_Time                := nil;
  Note_Sound               := nil;
  Note_Column              := nil;
  Note_Measure             := nil;
  Note_Position            := nil;
  Note_Flag                := nil;
  Measure_Time             := nil;
  Measure_Size             := nil;
  Measure_NoteAmount       := nil;
  Measure_HitNotes         := nil;
  WAV_Name                 := nil;
  BMP_Name                 := nil;
  TempoChange_Measure      := nil;
  TempoChange_Position     := nil;
  TempoChange_Amount       := nil;
  CustomTempoChange_Amount := nil;

end;



// ----------------------------------------------------------



procedure InitializeArrays;
// ==============================================
// Prepares the variable arrays for receiving the
// proper amounts of data.
// ----------------------------------------------
var
  i: Word;
begin

  FreeArrays; // To completely get rid of any data that might still exist
              // in the arrays

  SetLength(Note_Time,     Event_Count);
  SetLength(Note_Sound,    Event_Count);
  SetLength(Note_Column,   Event_Count);
  SetLength(Note_Measure,  Event_Count);
  SetLength(Note_Position, Event_Count);
  SetLength(Note_Flag,     Event_Count);

  SetLength(Measure_Time,       Measure_Count);
  SetLength(Measure_Size,       Measure_Count);
  SetLength(Measure_NoteAmount, Measure_Count);
  SetLength(Measure_HitNotes,   Measure_Count);

  SetLength(WAV_Name, WAV_Count + 2); // An extra value for blank sounds
  SetLength(BMP_Name, BMP_Count + 2); // and images

  SetLength(TempoChange_Measure,  TempoChange_Count);
  SetLength(TempoChange_Position, TempoChange_Count);
  SetLength(TempoChange_Amount,   TempoChange_Count);

  SetLength(CustomTempoChange_Amount, CustomTempoChange_Count + 1);

  for i := 0 to Measure_Count - 1 do
    Measure_Size[i] := 1;

end;



// ----------------------------------------------------------------------------



procedure SortBmpNames;
// ===============================================
// Sorts the BMP names so that in-use names appear
// at the start of the array.
//
// The Note_Sound[] Array will also be modified to
// reflect the changes in the BMP_Name[] array,
// despite the name.
// -----------------------------------------------
var
  i:       Integer;
  Swapped: Boolean;
begin

  repeat
    Swapped := False;

    for i := 1 to BMP_Count - 1 do    // Start counter at 1.
      if BMP_Name[i] = '' then        // 0 references the 'Miss' BMP image.
        if BMP_Name[i + 1] <> '' then
        begin
          SwapBmpNames(i, i + 1);
          Swapped := True;
        end;

  until not Swapped;

end;



// ----------------------------------------------------------------------------



procedure SortEvents;
// ===============================================
// This sorts the events in the various Note_...[]
// arrays from earliest to latest.
// -----------------------------------------------
var
  i:       Integer;
  Swapped: Boolean;
begin

  repeat
    Swapped := False;
    for i := 0 to Event_Count - 2 do
      if Note_Time[i] > Note_Time[i + 1] then
        if Note_Column[i + 1] <> 0 then
      begin
        SwapEvents(i, i + 1);
        Swapped := True;
      end;
  until not Swapped;

  repeat                                       // This places 'Measure Line'
    Swapped := False;                          // events earlier in the note
    for i := 0 to Event_Count - 2 do           // array if they appear at the
      if Note_Time[i] = Note_Time[i + 1] then  // same time as other events or
        if Note_Column[i + 1] = 10 then        // notes.
          if Note_Column[i + 1] <> 0 then      //
            if Note_Column[i] <> 10 then       //
            begin                              //
              SwapEvents(i, i + 1);            //
              Swapped := True;                 //
            end;                               //
  until not Swapped;                           //

end;



// ----------------------------------------------------------------------------



procedure SortTempoChangeArrays;
// ============================================
// Sort the BPM changes from first-appearing to
// last-appearing.
// --------------------------------------------
var
  i:       Integer;
  Swapped: Boolean;
begin

  repeat
    Swapped := False;

    for i := 0 to TempoChange_Count - 2 do
      if TempoChange_Position[i] > TempoChange_Position[i + 1] then
      begin
        SwapTempoChanges(i, i + 1);
        Swapped := True;
      end;

  until not Swapped;

end;



// ----------------------------------------------------------------------------



procedure SortWavNames;
// ===============================================
// Sorts the WAV names so that in-use names appear
// at the start of the array.
//
// The Note_Sound[] Array will also be modified to
// reflect the changes in the WAV_Name[] array.
// -----------------------------------------------
var
  i:       Integer;
  Swapped: Boolean;
begin

  repeat
    Swapped := False;

    for i := 1 to WAV_Count - 1 do
      if WAV_Name[i] = '' then
        if WAV_Name[i + 1] <> '' then
        begin
          SwapWavNames(i, i + 1);
          Swapped := True;
        end;

  until not Swapped;

end;



// ----------------------------------------------------------



procedure SwapBmpNames(X1, X2: Word);
// ======================================
// Swaps the 2 file names in the BMP_Name
// array, and also all respective values
// in Note_Sound array (Also used for
// image numbers, despite the name)
// --------------------------------------
var
  Temp_BMP:   String;
  i:          Word;
begin
  Temp_BMP     := BMP_Name[X1];
  BMP_Name[X1] := BMP_Name[X2];
  BMP_Name[X2] := Temp_BMP;

  for i := 0 to Event_Count - 1 do
    if Note_Column[i] in [4..7] then
    begin
      if Note_Sound[i] = X1 then
        Note_Sound[i] := X2
      else
      if Note_Sound[i] = X2 then
        Note_Sound[i] := X1;
    end;
    
end;



// ----------------------------------------------------------------------------



procedure SwapEvents(X1, X2: Word);
// ==================================
// Swaps the positions of 2 events in
// all related variable arrays.
// ----------------------------------
var
  Temp_NT, Temp_NP: Single;
  Temp_NM, Temp_NS: Word;
  Temp_NC, Temp_NF: Byte;
begin
  Temp_NT := Note_Time[X1];
  Temp_NP := Note_Position[X1];
  Temp_NM := Note_Measure[X1];
  Temp_NS := Note_Sound[X1];
  Temp_NC := Note_Column[X1];
  Temp_NF := Note_Flag[X1];

  Note_Time[X1]     := Note_Time[X2];
  Note_Position[X1] := Note_Position[X2];
  Note_Measure[X1]  := Note_Measure[X2];
  Note_Sound[X1]    := Note_Sound[X2];
  Note_Column[X1]   := Note_Column[X2];
  Note_Flag[X1]     := Note_Flag[X2];

  Note_Time[X2]     := Temp_NT;
  Note_Position[X2] := Temp_NP;
  Note_Measure[X2]  := Temp_NM;
  Note_Sound[X2]    := Temp_NS;
  Note_Column[X2]   := Temp_NC;
  Note_Flag[X2]     := Temp_NF;
end;



// ----------------------------------------------------------------------------



procedure SwapTempoChanges(X1, X2: Word);
// ====================================
// Swaps 2 values in each of the arrays
// related to Tempo Changes.
// ------------------------------------
var
  Temp_TCP, Temp_TCA: Single;
  Temp_TCM:           Word;
begin
  Temp_TCM := TempoChange_Measure[X1];
  Temp_TCP := TempoChange_Position[X1];
  Temp_TCA := TempoChange_Amount[X1];

  TempoChange_Measure[X1]  := TempoChange_Measure[X2];
  TempoChange_Amount[X1]   := TempoChange_Amount[X2];
  TempoChange_Position[X1] := TempoChange_Position[X2];

  TempoChange_Measure[X2]  := Temp_TCM;
  TempoChange_Position[X2] := Temp_TCP;
  TempoChange_Amount[X2]   := Temp_TCA;
end;



// ----------------------------------------------------------



procedure SwapWavNames(X1, X2: Word);
// ======================================
// Swaps the 2 file names in the WAV_Name
// array, and also all respective values
// in Note_Sound array.
// --------------------------------------
var
  Temp_WAV:   String;
  i:          Word;
begin
  Temp_WAV     := WAV_Name[X1];
  WAV_Name[X1] := WAV_Name[X2];
  WAV_Name[X2] := Temp_WAV;

  for i := 0 to Event_Count - 1 do
    if (Note_Column[i] = 1) or
       (Note_Column[i] >= 11) then
    begin
      if Note_Sound[i] = X1 then
        Note_Sound[i] := X2
      else
      if Note_Sound[i] = X2 then
        Note_Sound[i] := X1;
    end;

end;



// ----------------------------------------------------------



end.
