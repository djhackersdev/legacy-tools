unit DXAC1;
// =========================================================
// DXAC1.pas
// Created 2007.04.03
// Updated 2007.04.05 - Fixed 'no end-of-song' event bug.
// Updated 2007.04.06 - Corrected time of 'song stop' event.
//
// Converts data currently loaded in DB_Main arrays
// to the .1 format.
// ---------------------------------------------------------

interface

uses
  SysUtils, Classes, DB_Main;

type T1Event = packed record
       Event_Time: Cardinal;       // Time of the event, in (Seconds*FrameRate)
       Event_Type: Byte;           // What kind of event.
       Data1:      Byte;           // Use depends on the event's type.
       Data2:      Word;           // Use depends on the event's type.
     end;

type T1HeaderEntry = packed record
       NoteChart_Offset: Cardinal; // Byte location of note chart.
       NoteChart_Size:   Cardinal; // Byte size of note chart.
     end;

type T1Header = packed record
       Entry: array[1..12] of T1HeaderEntry;
     end;

const
  EndSequenceTime: Cardinal = 2130706432; // This value (0, 0, 0, 127) marks the
                                          // end of the note data sequence

  FrameRate: Single = 59.941; // Try 60 if this value gives undesired results.

  JudgementEvents: array[1..48] of byte = (0, 0, 0, 0, 8,  0, 240, 0,  // These
                                           0, 0, 0, 0, 8,  1, 248, 0,  // are
                                           0, 0, 0, 0, 8,  2, 255, 0,  // from
                                           0, 0, 0, 0, 8,  3, 3,   0,  // 5.1.1
                                           0, 0, 0, 0, 8,  4, 10,  0,  //
                                           0, 0, 0, 0, 8,  5, 18,  0); //

var
  DX1Stream: TMemoryStream;


function  Save1(Path_File: String): Boolean;
function  Create1Chart: Boolean;

implementation



// ----------------------------------------------------------------------------



function Save1(Path_File: String): Boolean;
begin
  Create1Chart;
  DX1Stream.SaveToFile(Path_File);
  DX1Stream.Free;
  Result := True;
end;



// ----------------------------------------------------------------------------



function Create1Chart: Boolean;
// ==================================
// Generates a .1 note chart from the
// currently loaded song, and keeps
// the data stream in memory.
// ----------------------------------
var
  i,  LastEventTime: Integer;
  Event:             T1Event;
  Header:            T1Header;
  TempStream:        TMemoryStream;

begin

  // =================================
  // Create a temporary data stream to
  // hold the converted note info.
  //
  // Also prepare the note chart's
  // "header".
  // ---------------------------------
  TempStream := TMemoryStream.Create;
  TempStream.SetSize(Event_Count shl 5);                // Allow for 2x space
                                                        // just to be safe.

  Event.Event_Time := 0;                                // Add 'Total Notes'
  Event.Event_Type := 16;                               // event marker
  Event.Data1 := 0;                                     //
  Event.Data2 := TotalNotes;                            //
  TempStream.WriteBuffer(Event, 8);                     //

  Event.Data1 := 1;                                     // The 'Bad' event?
  Event.Data2 := 0;                                     //
  TempStream.WriteBuffer(Event, 8);                     //
                                                        //

  Event.Event_Type := 4;                                // Prepare and write the
  Event.Data1      := 100;                              // song's starting tempo
  Event.Data2      := Trunc(Tempo * 100);               // event.
  TempStream.WriteBuffer(Event, 8);                     //

  Event.Event_Type := 5;                                // The 'Measure Size'
  Event.Data1      := 4;                                // event. Not sure if
  Event.Data2      := 4;                                // this is even used
  TempStream.WriteBuffer(Event, 8);                     // by anything.

  TempStream.WriteBuffer(JudgementEvents, 48);          // Default timing
                                                        // settings.

  // =================================
  // Now go through the Event database
  // and convert to .1 events
  // ---------------------------------
  LastEventTime := 0;
  for i := 0 to Event_Count - 1 do
  begin
    FillChar(Event, 8, 0);                               // Reset event variable
    Event.Event_Time := Trunc(Note_Time[i] * FrameRate);

    case Note_Column[i] of
      1: begin // --------------------------------------------------------------
           Event.Event_Type := 7;                             // BG Autoplay
           Event.Data2      := Note_Sound[i];                 //
         end;
      8: begin
           Event.Event_Type := 4;                             // Tempo Change
           if Note_Position[i] <= 655.35 then                 //
           begin                                              // Tempo values
             Event.Data1 := 100;                              // 655.35 BPM and
             Event.Data2 := Trunc(Note_Position[i] * 100);    // smaller will
           end                                                // use the special
           else                                               // '100' divisor.
           begin                                              //
             Event.Data1 := 1;                                // Higher values
             Event.Data2 := Trunc(Note_Position[i]);          // will be divided
           end;                                               // by 1.
         end;
      10: Event.Event_Type := 12;                             // Measure Line
      11..15: begin // ---------------------------------------------------------
                Event.Event_Type := 0;                        // Player 1
                Event.Data1      := Note_Column[i] - 11;      // Keys 1-5
              end;
      16: begin
            Event.Event_Type := 0;                            // Player 1
            Event.Data1      := 7;                            // Scratch Note
          end;
      18..19: begin
                Event.Event_Type := 0;                        // Player 1
                Event.Data1      := Note_Column[i] - 13;      // Keys 6-7
              end;
      21..25: begin // ---------------------------------------------------------
                Event.Event_Type := 1;                        // Player 2
                Event.Data1      := Note_Column[i] - 21;      // Keys 1-5
              end;
      26: begin
            Event.Event_Type := 1;                            // Player 2
            Event.Data1      := 7;                            // Scratch Note
          end;
      28..29: begin
                Event.Event_Type := 1;                        // Player 2
                Event.Data1      := Note_Column[i] - 23;      // Keys 6-7
              end;
      31..35: begin // ---------------------------------------------------------
                Event.Event_Type := 2;                        // Player 1 Sound
                Event.Data1      := Note_Column[i] - 31;      // Change,Keys 1-5
                Event.Data2      := Note_Sound[i];            //
              end;
      36: begin
            Event.Event_Type := 2;                            // Player 1 Sound
            Event.Data1      := 7;                            // Change,Scratch
            Event.Data2      := Note_Sound[i];                //
          end;
      38..39: begin
                Event.Event_Type := 2;                        // Player 1 Sound
                Event.Data1      := Note_Column[i] - 33;      // Change,Keys 6-7
                Event.Data2      := Note_Sound[i];            //
              end;
      41..45: begin // ---------------------------------------------------------
                Event.Event_Type := 3;                        // Player 2 Sound
                Event.Data1      := Note_Column[i] - 41;      // Change,Keys 1-5
                Event.Data2      := Note_Sound[i];            //
              end;
      46: begin
            Event.Event_Type := 3;                            // Player 2 Sound
            Event.Data1      := 7;                            // Change,Scratch
            Event.Data2      := Note_Sound[i];                //
          end;
      48..49: begin
                Event.Event_Type := 3;                        // Player 2 Sound
                Event.Data1      := Note_Column[i] - 43;      // Change,Keys 6-7
                Event.Data2      := Note_Sound[i];            //
              end; // ----------------------------------------------------------
      else Event.Event_Type := 255;                           // BGA? Skip it!
    end;

    if Event.Event_Type <> 255 then
    begin
      LastEventTime := Event.Event_Time;
      TempStream.WriteBuffer(Event, 8);

      if (Event.Event_Type = 12) and      // Note chart data streams in 2-player
         (Players > 1) then               // .1 files require a seperate measure
      begin                               // divider line event for each player.
        Event.Data1 := 1;                 // Absolutely no idea why that is, but
        TempStream.WriteBuffer(Event, 8); // this code portion will add it in.
      end;

    end;

  end;


  // ============================
  // Done with all of that? Yes!
  // Finalize the note chart now.
  // ----------------------------
  Event.Event_Time := LastEventTime + 60;    // Create the "End Song" event.
  Event.Event_Type := 6;                     // For now, this event will
  Event.Data1 := 0;                          // always be placed 1 second
  Event.Data2 := 0;                          // after the last event.
  TempStream.WriteBuffer(Event, 8);          //

  if Players > 1 then
  begin
    Event.Data1 := 1;                        // Write a second end-of-song event
    TempStream.WriteBuffer(Event, 8);        // for player 2 if needed.
  end;


  FillChar(Event, 8, 0);               // Create the "0 0 0 127 0 0 0 0"
  Event.Event_Time := EndSequenceTime; // "End of Sequence" event marker.
  TempStream.WriteBuffer(Event, 8);    //

  // =============================
  // Create the actual data stream
  // and the .1 header.
  // -----------------------------
  TempStream.SetSize(TempStream.Position);
  TempStream.Seek(0, soFromBeginning);
  FillChar(Header, 96, 0);              // Erase any existing old info in this

  Header.Entry[1].NoteChart_Offset := 96;              // Create the entry
  Header.Entry[1].NoteChart_Size   := TempStream.Size; // to the chart's data.

  for i := 2 to 4 do                    // Copy entry #1 to all other in-use
    Header.Entry[i] := Header.Entry[1]; // entry positions for player 1.
                                        // (7,L7,A7,Beg)

  for i := 7 to 9 do                    // And again for all of player 2's
    Header.Entry[i] := Header.Entry[1]; // note charts. (14,L14,A14)


  // =====================================================================
  // A little information about why the above is doing that...
  //
  // .1 headers are always 96 bytes long, so the first existing note chart
  // can have an offset value of 96.
  //
  // And since this early version of Save1 supports only 1 BMS at a time,
  // this will use that BMS note chart for all supported difficulties.
  //
  // Also, since all difficulties share the same chart, each offset in the
  // .1 header can point to the same byte location. No idea if that will
  // actually work on the official implementation, but let's try it!
  // ---------------------------------------------------------------------


  // ======================================
  // Finally, create the actual data stream
  // to use for saving the .1 file.
  // --------------------------------------
  DX1Stream := TMemoryStream.Create;
  DX1Stream.WriteBuffer(Header, 96);
  DX1Stream.CopyFrom(TempStream, TempStream.Size);
  TempStream.Free;

  Result := True;

end;



end.
