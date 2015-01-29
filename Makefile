
programs: read.o prompt.o
		cc -o program read.o prompt.o

read.o: read_command.cpp rshell.h
		cc -c rshell.cpp

prompt.o: type_prompt.cpp rshell.h
		cc -c type_prompt.cpp
