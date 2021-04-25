unit Utils;
// =================================================
// Utils.pas
// Created 2007.02.27
// Updated 2007.04.05 - Fixed a NoFile bug.
// Updated 2007.04.07 - Added DupChar and FileSize2.
//                      Re-wrote IsIn function.
//
// A set of various often-used routines.
// -------------------------------------------------


interface

uses
  SysUtils;

function DupChar(Amount: Word; Value: Byte): String;
function FileSize2(Path_File: String):       Integer;
function HexToInt(HexNum: String):           Integer;
function IntToBool(i: Integer):              Boolean;
function IsIn(Text1, Text2: String):         Boolean;
function NoExt(Text: String):                String;
function NoFile(Text: String):               String;
function NoPath(Text: String):               String;
function TitleOnly(Title: String):           String;


implementation



// ---------------------------------------------------------------------------



function DupChar(Amount: Word; Value: Byte): String;
// ===========================================
// Creates a text string consisting of a given
// ASCII value repeated a specified number of
// times.
// -------------------------------------------
var
  i: Byte;
begin
  Result := '';
  for i := 1 to Amount do
    Result := Result + Char(Value);
end;



// ---------------------------------------------------------------------------



function FileSize2(Path_File: String): Integer;
// ===============================
// An easier way to call FileSize.
// -------------------------------
var
  F: File;
begin
  AssignFile(F, Path_File);
  Reset(F, 1);
  Result := FileSize(F);
  CloseFile(f);
end;



// ---------------------------------------------------------------------------



function HexToInt(HexNum: string): Integer;
// ==============================================
// Converts a Hexadecimal number to decimal form.
// ----------------------------------------------
begin
  Result := StrToInt('$' + HexNum);
end;



// ---------------------------------------------------------------------------



function IntToBool(i: Integer): Boolean;
// ============================================
// Converts a number into a boolean expression.
// --------------------------------------------
begin
  if i <> 0 then Result := True
  else Result := False;
end;



// ---------------------------------------------------------------------------



function IsIn(Text1, Text2: String): Boolean;
// =============================================
// Checks to see if the letter/word/phrase Text2
// exists somewhere in Text1.
// ---------------------------------------------
var
  Amount_Similar, i1, i2, L1, L2: Word;
begin

  Result := False;
  L1 := Length(Text1);
  L2 := Length(Text2);

  if (L1 = 0) or          // Search fails with Zero-length strings.
     (L2 = 0) or          //
     (L2 > L1) then exit; // Also fails if the text to look for
                          // is larger than the text to look in.

  for i1 := 0 to L1 - L2 do
  begin
    Amount_Similar := 0;

    for i2 := 1 to L2 do
      if Text1[i1 + i2] = Text2[i2] then
        inc(Amount_Similar);

    if Amount_Similar = L2 then
      Result := True;

  end;

end;



// ---------------------------------------------------------------------------



function NoExt(Text: String): String;
// ============================================
// Removes a file extension from a text string.
//
// For example, NoExt('C:\PATH\FILE.TXT')
// would return C:\PATH\FILE
// --------------------------------------------
var
  i: Byte;
begin
  i := Length(Text);
  while (i > 1) and (Text[i] <> '.') and (Text[i] <> '\') do dec(i);
  if (i > 1) and (Text[i] <> '\') then Result := copy(Text, 1, i - 1)
  else Result := Text; // No extension found!
end;



// ---------------------------------------------------------------------------



function NoFile(Text: String): String;
// =========================================
// Removes the file name from a text string.
//
// For example, NoFile('C:\PATH\FILE.TXT')
// would return C:\PATH\
// -----------------------------------------
var
  i: Word;
begin
  i := Length(Text);
  while (i >= 1) and (Text[i] <> '\') do dec(i);
  if i >= 1 then Result := copy(Text, 1, i)
  else Result := ''; // No path found. Erase it!
end;



// ---------------------------------------------------------------------------



function NoPath(Text: String): String;
// =======================================
// Removes the path from a text string.
// For example, NoPath('C:\PATH\FILE.TXT')
// would return FILE.TXT
// ----------------------------------------
var
  i: Word;
begin
  i := Length(Text);
  while (i > 1) and (Text[i] <> '\') do dec(i);
  if i > 1 then Result := copy(Text, i + 1, Length(Text) - i)
  else Result := Text; // Already file-only!
end;



// ---------------------------------------------------------------------------



function TitleOnly(Title: String): String;
// =====================================================
// Almost every BMS song contains multiple difficulties.
//
// As a way to mark the different difficulties, authors
// usually add in some text to the title of the song.
//
// This function attempts to find and remove such text,
// to make the song names match a little better.
//
// This isn't a catch-all, but it's successful on over
// 99% of existing BMS songs.
//
// First it looks for text surrounded by () [] or --.
//
// Then it checks if the text contains a certain phrase.
// If it does, the text starting with and following the
// first bracket will be removed from the song title.
// -----------------------------------------------------
var
  i, i2:      Byte;
  chr1, chr2: Char;
  txt:        String;
  RemoveOK:   Boolean;
begin
  Result := Title;
  i2 := Length(Title);
  i := i2;
  if i <= 1 then exit;
  chr2 := Title[i];
  chr1 := ' ';
  if chr2 = '-' then chr1 := chr2;
  if chr2 = ')' then chr1 := '(';
  if chr2 = ']' then chr1 := '[';
  if chr1 = ' ' then exit;
  dec(i);
  while (i > 1) and (Title[i] <> chr1) do dec(i);
  if i = 1 then exit;

  txt := UpperCase(copy(Title, i + 1, i2 - (i + 1))); // Get and check the text
  RemoveOK := False;                                  // between the 2 symbols
  if IsIn(txt, 'BEGINNER') then RemoveOK := True;
  if IsIn(txt, 'NORMAL') then RemoveOK := True;
  if IsIn(txt, 'LIGHT') then RemoveOK := True;
  if IsIn(txt, 'HYPER') then RemoveOK := True;
  if IsIn(txt, 'ANOTHER') then RemoveOK := True;
  if IsIn(txt, '5K') then RemoveOK := True;
  if IsIn(txt, '7K') then RemoveOK := True;
  if IsIn(txt, '9K') then RemoveOK := True;
  if IsIn(txt, 'HARD') then RemoveOK := True;
  if IsIn(txt, 'EASY') then RemoveOK := True;

  if RemoveOK then Result := TrimRight(copy(Title, 1, i - 1));

end;



// ---------------------------------------------------------------------------



end.
