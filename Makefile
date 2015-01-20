all: draw.o example bounce earth

clean:
	rm -f draw.o example bounce earth
	
fresh: clean all

# compile lib with g++ since clang++ has issues with std::{thread,chrono}
draw.o: draw.cpp
	moc-qt4 draw.cpp | g++ -g -c -std=c++11 -fmax-errors=1 -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I/usr/include/phonon -x c++ - -include draw.cpp -o draw.o
	
example: draw.o example.cpp
	g++ -g example.cpp draw.o -lQtGui -lQtCore -lphonon -o example

bounce: draw.o bounce.cpp
	g++ -g bounce.cpp draw.o -lQtGui -lQtCore -lphonon -o bounce

earth: draw.o earth.cpp
	g++ -g -Wno-write-strings earth.cpp draw.o -lQtGui -lQtCore -lQtMultimediaKit -lphonon -o earth

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media
