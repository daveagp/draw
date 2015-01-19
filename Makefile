all: example

clean:
	rm -f draw.o example

# compile lib with g++ since clang++ has issues with std::{thread,chrono}
draw.o:	
	moc-qt4 draw.cpp | g++ -g -c -std=c++11 -I/usr/include/qt4/QtGui -I/usr/include/qt4 -x c++ - -include draw.cpp -o draw.o

example: draw.o
	g++ -g example.cpp draw.o -lQtGui -lQtCore -o example

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip draw.zip draw.cpp draw.h draw_qo.h Makefile example.cpp
