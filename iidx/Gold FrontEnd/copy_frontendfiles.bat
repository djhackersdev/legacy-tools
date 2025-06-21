@echo off
mkdir FrontendFiles\Announcers
mkdir FrontendFiles\BGMs
mkdir FrontendFiles\Keys

copy data\sd_data\sys_sd\coin* FrontendFiles\BGMs\
copy data\sd_data\sys_sd\sys_op.2dx FrontendFiles\Announcers\
copy data\sd_data\sys_sd\sys_skin_hoshino.2dx FrontendFiles\Announcers\
copy data\graph\sys\items\* FrontendFiles\Turntables\Default\
copy data\graph\sys\gameparts\* FrontendFiles\Towels\Default\