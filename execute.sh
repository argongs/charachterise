#!/bin/bash
#This shell script combines the invocation of 'cmake' 'make' and executable binary of C++ file into a single place. This allows execution of the requested C++ program with just a single shell command.
if [ $# -eq 0 ]; then 
	echo "[error] You need to provide a C++ program file and a image file(jpg/jpeg/png) for execution"
	echo "[info.] usage : bash execute.sh file_to_execute.cpp image.jpeg"
else
	##Obtain the file name for execution
	cpp_source="$1"
	img_source="$2"
	exec_binary="exec_binary"
	##Check if the given cpp source file and image file really exist
	if [ -f "$cpp_source" ] && [ -f "$img_source" ]; then
		echo "[info.] Compiling $cpp_source ..."
		
		cmake -D CPP_SOURCE=$cpp_source -D EXEC_BINARY=$exec_binary .
		make
		
		echo "[info.] Finished compilation procedure."
		echo "[info.] Performing execution .."
		
		./$exec_binary $img_source
		
		echo "[info.] Cleaning up the mess.."
		make clean
		
		echo "[info.] Done"
	else
		echo "[error] $cpp_source and $img_source do not exist."
	fi
fi
