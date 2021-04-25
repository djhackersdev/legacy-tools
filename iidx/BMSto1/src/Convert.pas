unit Convert;
// =============================
// Convert.pas
// Created 2007.04.04
// Updated 2007.04.07
//
// Holds the code that calls the
// various conversion routines.
// -----------------------------

interface

uses
  BMS, DB_Main, DB_XForm, DXAC1, DXAC2DX, Utils;


function Do_Stuff(Path_Name: String; DoRandom, DoMirror, DoVMirror: Boolean): Boolean;


implementation



// ---------------------------------------------------------------------------



function Do_Stuff(Path_Name: String; DoRandom, DoMirror, DoVMirror: Boolean): Boolean;
begin

  OpenBMS(Path_Name);
  SortEvents;                   // Database always needs sorting after opening.
  KeyAmount := GetKeyAmount;
  TotalNotes := ReCalculatePlayableNotes;
  ApplyTempoChanges;
  if DoRandom then ApplyRandom;
  if DoMirror then ApplyMirror;
  if DoVMirror then ApplyMirrorPlus;
  SortEvents;
  SortWAVNames;                 // Compress the WAV list!
  AddSoundChanges;
  SortEvents;                   // All that modifying! Let's re-sort!

  Save1(NoExt(Path_Name) + '.1');     // Generate and save the .1 file
  Save2dx(NoExt(Path_Name) + '.2dx'); // Generate and save the .2dx file

  Result := True;
end;



// ---------------------------------------------------------------------------



end.
