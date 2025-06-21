@ECHO OFF
sox.exe "%~fs1" -n stat
set /p vol= "Volume: "
sox.exe -G -S "%~fs1" -e ms-adpcm "%~dps0_pre1.wav" vol %vol% trim 00:00:15 =00:00:25 fade t 0 00:00:10 00:00:01.5
sox.exe -G -S "%~fs1" -e ms-adpcm "%~dps0_pre2.wav" vol %vol% trim 00:00:30 =00:00:40 fade t 0 00:00:10 00:00:01.5
sox.exe -G -S "%~fs1" -e ms-adpcm "%~dps0_pre3.wav" vol %vol% trim 00:00:45 =00:00:55 fade t 0 00:00:10 00:00:01.5
sox.exe -G -S "%~fs1" -e ms-adpcm "%~dps0_pre4.wav" vol %vol% trim 00:01:00 =00:01:10 fade t 0 00:00:10 00:00:01.5
sox.exe -G -S "%~fs1" -e ms-adpcm "%~dps0_pre5.wav" vol %vol% trim 00:01:15 =00:01:25 fade t 0 00:00:10 00:00:01.5
