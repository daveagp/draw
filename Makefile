all: demo

clean:
	rm -f drawmoc.cpp draw.o example

# compile lib with g++ since clang++ has issues with std::{thread,chrono}
lib:	
	moc-qt4 drawmoc.h -o drawmoc.cpp
	g++ -g -c -std=c++11 -I/usr/include/qt4/QtGui -I/usr/include/qt4 -include drawmoc.cpp draw.cpp -o draw.o

example: lib
	g++ example.cpp draw.o -lQtGui -o example
