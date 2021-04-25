unit Base36;
// ====================================================
// Base36.pas v0.11
// Created 2005.06.02
// Modified 2007.02.20
//
// This unit converts a Base-36 number to decimal.
// Base-36 alpha: 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ
//
// A function to convert from Decimal will most likely
// be added soon. Maybe.
//
// This could be easily modified to support a different
// number base, just modify the 2 constants below.
// ----------------------------------------------------

interface

uses SysUtils;

const
  B36: byte = 36;
  Base36String: String = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';

Function Base36ToInt(B36Num: string): Integer;
Function Exp(Base: Integer; Power: Byte): Integer;


implementation



function Base36ToInt(B36Num: string): Integer;
// =========================================
// Converts a Base-36 number of any length
// to a decimal number.
// -----------------------------------------
var
  Number_Length, Number_Position: word;
  i: Byte;
begin
  B36Num := UpperCase(B36Num);
  Number_Length := Length(B36Num);
  Number_Position := Number_Length;
  Result := 0;
  while Number_Position >= 1 do
  begin
    i := 1;
    while B36Num[Number_Position] <> Base36String[i] do
    begin
      inc(i);
      if i > B36 then
      begin
        Result := 0;
        exit;
      end;
    end;
    Result := Result + (i - 1) * Exp(B36, Number_Length - Number_Position);
    dec(Number_Position);
  end;
end;



// ---------------------------------------------------------------------------



function Exp(Base:Integer; Power:Byte): Integer;
// ==============================================
// A quickie exponential function where the power
// is exclusively a rounded non-negative number
// between (and including) 0 and 255!
// ----------------------------------------------
var
  i: byte;
begin
  Result := 1;
  for i := 1 to Power do Result := Result * Base;
end;



end.
