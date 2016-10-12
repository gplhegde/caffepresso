connect arm hw
rst -processor
if { $::argc >0 } {
	 set elf [lindex $::argv 0]
} else {
	 set elf [lindex [glob *.elf] 0]
}
dow $elf
con
