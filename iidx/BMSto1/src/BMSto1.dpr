program BMSto1;
// ==============================================
// BMSto1 v0.06
// Created 2007.04.03
// Updated 2007.04.07
//
// Converts a Be-Music Source file to .1 format.
// Also combines WAV files to a .2dx format file.
// ----------------------------------------------


{$APPTYPE CONSOLE}

uses
  SysUtils,
  Convert in 'Convert.pas',
  Utils in 'Utils.pas',
  DXAC2DX in 'DXAC2DX.pas';

// ----------------------------------------------------------------------------



function CheckForParam(Param: String): Boolean;
var
  i: Word;
begin

  Result := False;
  Param := UpperCase(Param);

  for i := 1 to ParamCount do
    if UpperCase(ParamStr(i)) = Param then
      Result := True;

end;



// ----------------------------------------------------------------------------



procedure ScanForFiles;
const
  SlashRANDOM:  array[1..7] of Char = '/RANDOM';
  SlashMIRROR:  array[1..7] of Char = '/MIRROR';
  SlashVMIRROR: array[1..8] of Char = '/VMIRROR';
var
  Rec:         TSearchRec;
  Path:        String;
  DoRandom:    Boolean;
  DoMirror:    Boolean;
  DoVMirror:   Boolean;
  i:           Word;
begin

  DoRandom    := False;
  DoMirror    := False;
  DoVMirror   := False;

  if CheckForParam(SlashRANDOM)  then DoRandom    := True;
  if CheckForParam(SlashMIRROR)  then DoMirror    := True;
  if CheckForParam(SlashVMIRROR) then DoVMirror   := True;

  for i := 1 to ParamCount do
  begin

    Path := NoFile(ParamStr(i));
    if Path = '' then Path := '.\';
    if FindFirst(ParamStr(i),
                 faAnyFile - faDirectory,
                 Rec) = 0 then
    try
      repeat
        write(Rec.Name + ' > ' + NoExt(Rec.Name) + '.1 + .2dx ...');
        case Do_Stuff(Path + Rec.Name, DoRandom, DoMirror, DoVMirror) of
          True:  writeln('OK!');
          False: writeln('Error!');
        end;
      until FindNext(Rec) <> 0;
    finally
      FindClose(Rec);
    end;

  end;

  writeln('All tasks complete.');
  writeln;

end;



// ----------------------------------------------------------------------------



procedure DisplayInfo;
// ==============================================
// Displays information on how to use the program
// when no command line parameters exist.
// ----------------------------------------------
begin
  writeln;
  write  ('================================================================================');
  write  ('BMSto1  --  v0.06!  -  Be-Music Source conversion utility  -  created 2007.04.07');
  write  ('--------------------------------------------------------------------------------');
  writeln;
  writeln(' To use:  BMSto1 [/RANDOM] [/MIRROR] [/VMIRROR] InputFile1 InputFile2 ...');
  writeln;
  writeln('    InputFile  - The BMS file(s) to convert. Wildcards ? and * can be used in');
  writeln('                 any InputFile name parameter. All Output file(s) will have .1');
  writeln('                 as their new file extension. BMS WAV sounds will be combined');
  writeln('                 into .2dx files.');
  writeln('      /RANDOM  - Applies the Random Modifier effect to the note-chart.');
  writeln('      /MIRROR  - Applies the Mirror Modifier effect to the note-chart.');
  writeln('     /VMIRROR  - Vertically flips the note-chart. Last note becomes first!');
  writeln;
  writeln(' This version of BMSto1 currently isn''t able to combine multiple BMS files of');
  writeln(' the same song. Instead, each BMS will be converted to its own .1 file.');
  writeln;
  writeln;
  writeln(' Use of BMSto1 is AT YOUR OWN RISK.  It has been tested, however I will not');
  writeln(' take responsibility for damaged or corrupted files.');
  writeln;
end;



// ----------------------------------------------------------------------------


begin
  { TODO -oUser -cConsole Main : Insert code here }

  if ParamStr(1) = '' then
  begin
    DisplayInfo;
    exit;
  end;
  ScanForFiles;
end.

