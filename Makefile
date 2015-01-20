all: draw.o example bounce earth

clean:
	rm -f draw.o example bounce earth
	
fresh: clean all

# to add music:
# - delete or comment out -DDRAW_MUTE
# - add -lphonon to FLAGS
# - sudo apt-get install libphonon4 libphonon-dev vlc-plugin-fluidsynth
MUTE = -DDRAW_MUTE
WARN = -Wall -Wno-return-type -Wno-write-strings
INCL = -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I/usr/include/phonon
OFLAGS = $(INCL) $(WARN) -g -Wall -std=c++11 -fmax-errors=1 $(MUTE)
FLAGS = $(WARN) -g -lQtGui -lQtCore

# compile lib with g++ since clang++ has issues with std::{thread,chrono}
draw.o: draw.cpp
	moc-qt4 draw.cpp | g++ $(OFLAGS) -c -x c++ - -include draw.cpp -o draw.o
	
example: draw.o example.cpp
	g++ example.cpp draw.o $(FLAGS) -o example

bounce: draw.o bounce.cpp
	g++ bounce.cpp draw.o $(FLAGS) -o bounce

earth: draw.o earth.cpp
	g++ earth.cpp draw.o $(FLAGS) -o earth

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media
