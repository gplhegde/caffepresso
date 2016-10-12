##select_file:
#Find all files matching the specified glob.
#Inputs:
#filetype - textual description of file type
#fileglob - glob to be searched for (using "find -name $fileglob")
#interactive - prompt when multiple files found
#
#Returns:
#"" if no matching files found
#file name if one matching file found
#newest file name if multiple files found and non-interactive
#selected file (may be "" if none selected) if multiple found and interactive
proc select_file {filetype fileglob interactive} {
    set sel_files [exec find -name $fileglob]
    set num_sel_files [llength $sel_files]

    set done false
    if {$num_sel_files < 1} {
	puts "No $filetype found"
	return 1
    } elseif {$num_sel_files == 1} {
	puts -nonewline "Found 1 $filetype:"
	puts $sel_files
	return $sel_files
    } else {
	puts "Found $num_sel_files $filetype:"
	puts $sel_files
	set newest_file [lindex $sel_files 0]
	set newest_time 0
	foreach sel_file $sel_files {
	    if {!$interactive} {
		set sel_time [file mtime $sel_file]
		if {$sel_time > $newest_time} {
		    set newest_time $sel_time
		    set newest_file $sel_file
		}
	    } else {
		set inp "?"
		while {$inp != "y" && $inp != "n" && $inp != "Y" && $inp != "N"} {
		    puts -nonewline "Use $sel_file (y/n)? "
		    flush stdout
		    set inp [gets stdin]
		}
		if {$inp == "y" || $inp == "Y"} {
		    return $sel_file
		}
	    }
	}
	if {!$interactive} {
	    return $newest_file
	}
    }
    return ""
}


#default options
set interactive false

#Parse command line arguments, currently if non-zero then run interactive
if { $::argc > 0 } {
    puts "Arguments passed ($::argv), running interactively."
    set interactive true
} else {
    puts "No arguments passed, running in non-interactive mode."
}

#Connect and reset
connect arm hw
stop
rst -srst

#find bitstream and program
set bitstream [select_file "bitstream" "*.bit" $interactive]
if {$bitstream == ""} {
    puts "Bitstream not programmed"
    exit 1
} else {
    puts "Programming $bitstream"
    fpga -f $bitstream
}

#find ps7_init.tcl and source
set ps7_tcl "bsp/ps7_init.tcl"
if {$ps7_tcl == ""} {
    puts "PS7 not initialized"
    exit 2
} else {
    puts "Initializing PS7 from $ps7_tcl"
    source $ps7_tcl
}

#Initialize processing system
ps7_init
ps7_post_config
con
