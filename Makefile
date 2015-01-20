# to add music:
# - run 'make fresh music=y'
# - you might need to install some packages. on my machine this worked:
#      sudo apt-get install libphonon4 libphonon-dev vlc-plugin-fluidsynth

EXAMPLES = example bounce earth polygon
all: draw.o $(EXAMPLES)

clean:
	rm -f draw.o polygon.png $(EXAMPLES)
	
fresh: clean all

WARN = -Wall -Wno-return-type -Wno-write-strings
INCL = -I/usr/include/qt4/QtGui -I/usr/include/qt4
OFLAGS = $(INCL) $(WARN) -g -Wall -std=c++11 -fmax-errors=1
FLAGS = $(WARN) -g -lQtGui -lQtCore
ifdef music
INCL += -I/usr/include/phonon
OFLAGS += -DDRAW_UNMUTE
FLAGS += -lphonon
endif

# I had to compile lib with g++ since clang++ has issues with thread,chrono
draw.o: draw.cpp draw.h
	moc-qt4 draw.cpp | g++ $(OFLAGS) -c -x c++ - -include draw.cpp -o draw.o
	
example: draw.o example.cpp
	g++ example.cpp draw.o $(FLAGS) -o example

bounce: draw.o bounce.cpp
	g++ bounce.cpp draw.o $(FLAGS) -o bounce

earth: draw.o earth.cpp
	g++ earth.cpp draw.o $(FLAGS) -o earth
	
polygon: draw.o polygon.cpp
	g++ polygon.cpp draw.o $(FLAGS) -o polygon

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media
