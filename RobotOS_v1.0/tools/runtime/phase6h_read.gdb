# phase6h_read.gdb
# GDB batch script: Phase 6H counter read via OpenOCD GDB server.
#
# Used automatically by capture_phase6h_runtime.ps1 when RTT dump fails.
# Can also be run manually (see below).
#
# REQUIRES: OpenOCD running as GDB server on localhost:3333
#   openocd -s "C:\Program Files\OpenOCD\share\openocd\scripts" \
#           -f "D:\Robot_OS\zephyr\boards\arm\stm32f411e_disco\support\openocd.cfg"
#
# MANUAL USAGE:
#   arm-zephyr-eabi-gdb.exe --batch -x phase6h_read.gdb D:\Robot_OS\build\zephyr\zephyr.elf
#
# OUTPUT FORMAT:
#   $1 = 8    (s_timer_attempted_count)
#   $2 = 8    (s_timer_ok_count)
#   $3 = 0    (s_timer_full_count)
#   $4 = 0    (s_timer_invalid_count)
#   $5 = 0    (s_timer_other_error_count)
#   $6 = 8    (s_timer_handled_count)
#   $7 = 0    (s_timer_unexpected_count)
#   $8 = 1    (s_timer_final_logged)
#   0xe000ed28 <SCB_CFSR>:  0x00000000
#   0xe000ed2c <SCB_HFSR>:  0x00000000
#
# PASS CRITERIA:
#   $1=8 $2=8 $3=0 $4=0 $5=0 $6=8 $7=0 $8=1 CFSR=0 HFSR=0

target remote localhost:3333
monitor reset run
monitor after 8000
monitor halt
print (unsigned int)s_timer_attempted_count
print (unsigned int)s_timer_ok_count
print (unsigned int)s_timer_full_count
print (unsigned int)s_timer_invalid_count
print (unsigned int)s_timer_other_error_count
print (unsigned int)s_timer_handled_count
print (unsigned int)s_timer_unexpected_count
print (int)s_timer_final_logged
x/1xw 0xE000ED28
x/1xw 0xE000ED2C
monitor reset run
quit
