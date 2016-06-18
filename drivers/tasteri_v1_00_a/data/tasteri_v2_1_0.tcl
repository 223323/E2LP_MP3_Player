##############################################################################
## Filename:          C:\materija\ra248-2013\E2LP_MP3_Player/drivers/tasteri_v1_00_a/data/tasteri_v2_1_0.tcl
## Description:       Microprocess Driver Command (tcl)
## Date:              Sat Jun 18 12:32:38 2016 (by Create and Import Peripheral Wizard)
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "tasteri" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" 
}
