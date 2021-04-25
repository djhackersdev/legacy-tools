unit DB_XForm;
// ===============================================
// DB_XForm.pas
// Created 2007.02.24
//
// Contains procedures to transform and manipulate
// the data stored in the DB_Main.pas arrays.
// -----------------------------------------------

interface

uses
  DB_Main;

procedure AddBackgroundStars;
procedure AddSoundChanges;
procedure ApplyAutoPlay;
procedure ApplyAutoScratch;
procedure ApplyEasy;
procedure ApplyMirror;
procedure ApplyRandom;
procedure ApplyMirrorPlus;
procedure CheckNoteSounds;
procedure DelayEvents(Seconds: Single);
procedure MakeKeyAmount(Amount: Byte);
procedure MakePlayerAmount(Amount: Byte);


implementation



// ----------------------------------------------------------------------------



procedure AddBackgroundStars;
// ===========================================================
// Adds the Background star-field into the note pattern array,
// so they can be properly Z-plane sorted too!
//
// Only needed for the 3D mode.
//
// It's a good idea to call SortEvents after using this!
// -----------------------------------------------------------
var
  Latest_Note: Single;
  i, ff: Integer;
  Star_Type: Byte;
begin

  ff := GetFirstFreeEvent;
  if ff < 0 then exit;

  Latest_Note := 0;
  Star_Type := 0;

  for i := 0 to Event_Count - 1 do
    if Note_Time[i] > Latest_Note then
      Latest_Note := Note_Time[i];

  repeat
    Note_Column[ff] := 5;
    Note_Time[ff] := Latest_Note;
    Note_Sound[ff] := (Star_Type and 1) + 2;
    Latest_Note := Latest_Note - 0.3;
    inc(ff);
    inc(Star_Type);
  until Latest_Note < 0;

end;



// ----------------------------------------------------------------------------



procedure AddSoundChanges;
// ==============================================
// Creates a list of 'current sound to play when
// a button is pressed' events, and adds them to
// the note arrays.
//
// The sound change events will be placed halfway
// between 2 notes in a column.
// ----------------------------------------------
var
  i, ff:          Integer;
  Current_Time:   Single;
  Current_Sound:  Word;
  Current_Column: Byte;
begin

  ff := GetFirstFreeEvent;
  if ff < 0 then exit;     // Error! To handle later

  for Current_Column := 11 to 29 do
  begin
    Current_Sound := 0;
    Current_Time := 0;

    for i := 0 to Event_Count - 1 do
      if (Note_Column[i] = Current_Column) or
         (Note_Column[i] = Current_Column + 20) then
      begin
        if Note_Sound[i] <> Current_Sound then
        begin
          if Current_Time = 0 then Note_Time[ff] := 0
          else Note_Time[ff] := ((Note_Time[i] - Current_Time) / 2) + Current_Time;
          Note_Column[ff] := Note_Column[i] + 20;
          Current_Sound := Note_Sound[i];
          Note_Sound[ff] := Current_Sound;
          inc(ff);
        end;
        Current_Time := Note_Time[i];
      end;

  end;

end;



// -------------------------------------------------------------------------



procedure ApplyAutoPlay;
// ==================================
// This procedure sets the 'AutoPlay'
// flag of all notes.
// ----------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    if Note_Column[i] in [11..29] then
      Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;

end;



// -------------------------------------------------------------------------



procedure ApplyAutoScratch;
// =================================
// Converts all scratch-column notes
// into auto-play notes.
// ---------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    if (Note_Column[i] = 16) or
       (Note_Column[i] = 26) then
      Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;

end;



// -------------------------------------------------------------------------



procedure ApplyEasy;
// ==================================
// This Auto-plays and hides all blue
// and scratch notes!
// ----------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    if Note_Time[i] > 0 then
      case Note_Column[i] of
        12, 14, 16, 18, 22, 24: Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;
      end;

end;



// -------------------------------------------------------------------------



procedure ApplyMirror;
// =====================================
// Horizontally flips the note patterns.
// For now, only works on 5 and 7 key
// note charts.
// -------------------------------------
var
  i: word;
  Column_From, Column_Base: byte;
const
  M_5: array[1..6] of byte = (5, 4, 3, 2, 1, 6);
  M_7: array[1..9] of byte = (9, 8, 5, 4, 3, 6, 0, 2, 1);
begin
  for i := 0 to Event_Count - 1 do
  begin

    if Note_Column[i] > 10 then
    begin
      Column_From := Note_Column[i];
      while Column_From > 10 do dec(Column_From, 10);
      Column_Base := Note_Column[i] - Column_From;
      case KeyAmount of
        5: Note_Column[i] := Column_Base + M_5[Column_From];
        7: Note_Column[i] := Column_Base + M_7[Column_From];
      end;
    end;

  end;
end;



// ----------------------------------------------------------------------------



procedure ApplyMirrorPlus;
// ===========================================
// This flips the note patterns upside down...
// Last note first, and first note last!
//
// Only transforms the visible keys. BG sounds
// and other things aren't modified.
//
// A call to DB.SortEvents will definitely be
// needed after calling this.
// -------------------------------------------
var
  i:           Word;
  MaxNoteTime: Single;
begin
  MaxNoteTime := 0;

  for i := 0 to Event_Count - 1 do       // Find the note with the
    if Note_Time[i] > MaxNoteTime then   // latest play-time.
      if Note_Column[i] in [11..29] then //
        MaxNoteTime := Note_Time[i];     //

  for i := 0 to Event_Count - 1 do
    if Note_Column[i] >= 11 then
    begin
      Note_Time[i] := MaxNoteTime - Note_Time[i];
      Note_Measure[i] := Measure_Count - Note_Measure[i];
    end;

end;



// -------------------------------------------------------------------------



procedure ApplyRandom;
// =============================================
// Randomizes the columns of notes, not the note
// positions themselves.
// ---------------------------------------------
var
  i: word;
  i2, r_i, pl_num, column: byte;
  R_9, R2_9: array[1..10] of byte;
  ok: boolean;
begin

  for i2 := 1 to 10 do
    R_9[i2] := i2;

  R2_9 := R_9;

  if KeyAmount <> 9 then
    R2_9[6] := 10;

  if KeyAmount = 5 then
    for i2 := 7 to 9 do R2_9[i2] := 10;

  if KeyAmount = 7 then
    R2_9[7] := 10;

  Randomize;

  for i2 := 1 to 9 do
  begin
    ok := True;
    if R2_9[i2] = 10 then ok := False;
    if ok then
    begin
      r_i := Random(9);
      while (R2_9[r_i] = 0) or
            (R2_9[r_i] = 10) do
      begin
        inc(r_i);
        if r_i = 10 then r_i := 1
      end;
      R_9[i2] := r_i;
      R2_9[r_i] := 0;
    end;
  end;

  for i := 0 to Event_Count - 1 do             // New column places generated,
  begin                                        // now look through the arrays
    Column := Note_Column[i];                  // and change the needed notes.
    while Column > 10 do dec(Column, 10);
    pl_num := Note_Column[i] - Column;
    if pl_num > 0 then
      if Column > 0 then
        Note_Column[i] := pl_num + R_9[Column];
  end;

end;



// ----------------------------------------------------------------------------



procedure CheckNoteSounds;
// ======================================
// This checks to make sure the values in
// the Note_Sound array don't exceed the
// total amount of WAV files.
//
// If a note does, then it is set to 0,
// a blank WAV.
// --------------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    if Note_Sound[i] > WAV_Count then
      if (Note_Column[i] = 1) or
         (Note_Column[i] >= 11) then
        Note_Sound[i] := 0;

end;



// ----------------------------------------------------------------------------



procedure DelayEvents(Seconds: Single);
// ========================================================
// Re-positions the events so that the first playable note
// is no less than the specified time in seconds from the
// song's beginning.
//
// If the note array is already sorted before calling this,
// another sort after won't be necessary. The notes will
// still be in the same time order as before this procedure
// is called.
// --------------------------------------------------------
var
  i: Integer;
  Delay_Time: single;
begin

  Delay_Time := Seconds;

  for i := 0 to Event_Count - 1 do          // Find the first playable note
    if Note_Column[i] in [11..29] then      // closest to the song's start,
      if Note_Time[i] < Seconds then        // but which also appears before
        if Note_Time[i] >= 0 then           // the song's 3-second mark.
          if Note_Time[i] < Delay_Time then //
            Delay_Time := Note_Time[i];     //

  Delay_Time := Seconds - Delay_Time;       // Change the note's time into the
                                            // desired amount to add to all notes.

  for i := 0 to Event_Count - 1 do
    Note_Time[i] := Note_Time[i] + Delay_Time;

end;



// ----------------------------------------------------------------------------



procedure MakeKeyAmount(Amount: byte);
// =========================================
// This optimizes a note layout to match the
// specified key amount.
//
// 7-key note layouts don't seem to need any
// modification.
// -----------------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    case Amount of
      5: if (Note_Column[i] in [18..19]) or
            (Note_Column[i] in [28..29]) then
         begin
           Note_Column[i] := 1;
           Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;
         end;
      9: if (Note_Column[i] in [16..21]) or
            (Note_Column[i] in [26..29]) then
         begin
           Note_Column[i] := 1;
           Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;
         end;
    end;

end;



// -------------------------------------------------------------------------



procedure MakePlayerAmount(Amount: Byte);
// ================================================
// This will optimize a note-chart for the intended
// amount of players.
// ------------------------------------------------
var
  i: Word;
begin

  for i := 0 to Event_Count - 1 do
    if Amount = 1 then
      if Note_Column[i] in [21..29] then
       begin
         Note_Column[i] := 1;
         Note_Flag[i] := Note_Flag[i] or Flag_AutoPlay;
       end;

end;



// -------------------------------------------------------------------------



end.
