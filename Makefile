compiler = g++
flags = -g -Wall -std=c++11
compile = $(compiler) $(flags)

Hashtable: HashtableTests.cpp Hashtable.h
	$(compile) $< -o $@

.PHONY: clean
clean:
	rm -rf *.o Hashtable
