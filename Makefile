
programs: rshell.o type_prompt.o
		g++ -o program rshell.o type_prompt.o

rshell.o: rshell.cpp rshell.h
		g++ -c rshell.cpp

type_prompt.o: type_prompt.cpp rshell.h
		g++ -c type_prompt.cpp
