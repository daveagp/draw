# to add audio:
# 1. you need to install some packages. on my machine this worked:
#  sudo apt-get install phonon-backend-gstreamer libphonon4 libphonon-dev
# 2. run 'make fresh audio=y' and use audio=y every time you run make
# 3. you might need to install more packages to play .mid files

EXAMPLES = example bounce earth polygon sierpinski htree nestedcircles
all: draw.o $(EXAMPLES)

clean:
	rm -f draw.o polygon.png $(EXAMPLES)
	
fresh: clean all

WARN = -Wall -Wno-return-type -Wno-write-strings
INCL = -I/usr/include/qt4/QtGui -I/usr/include/qt4
OFLAGS = $(INCL) $(WARN) -g -Wall -std=c++11 -fmax-errors=1
FLAGS = $(WARN) -g -lQtGui -lQtCore
ifdef audio
INCL += -I/usr/include/phonon
OFLAGS += -DDRAW_UNMUTE
FLAGS += -lphonon
endif

# have to compile lib with g++ since clang++ has issues with thread,chrono
draw.o: draw.cpp draw.h
	moc-qt4 draw.cpp | g++ $(OFLAGS) -c -x c++ - -include draw.cpp -o draw.o

$(EXAMPLES): %: draw.o %.cpp
	g++ $@.cpp draw.o $(FLAGS) -o $@

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media README
