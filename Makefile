# to add audio:
# 1. you need to install some packages. on my machine this worked:
#  sudo apt-get install phonon-backend-gstreamer libphonon-dev
# 2. run 'make fresh audio=y' and use audio=y every time you run make
# 3. see if ./earth works
# 4. if you get error about .mid files, more packages needed. I used:
#  sudo apt-get install ubuntu-restricted-extras vlc-plugin-fluidsynth
#    (use tab and space to navigate the weird dialogs)

EXAMPLES = example bounce earth polygon sierpinski htree nestedcircles
CXX = g++ # compile lib with g++; clang++ has issues with thread, chrono
MOCQT4 = moc-qt4

all: draw.o $(EXAMPLES)

clean:
	rm -f draw.o polygon.png bull $(EXAMPLES)
	
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

draw.o: draw.cpp draw.h
	$(MOCQT4) draw.cpp | $(CXX) $(OFLAGS) -c -x c++ - -include draw.cpp -o draw.o
	
%: %.cpp draw.o draw.h
	$(CXX) $@.cpp draw.o $(FLAGS) -o $@
	
# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media README
