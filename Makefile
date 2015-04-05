# to add audio:
# 1. you need to install some packages. on my machine this worked:
#    sudo apt-get install phonon-backend-gstreamer libphonon-dev
# 2. run 'make fresh audio=y' and use audio=y every time you run make
# 3. see if ./earth works
# 4. if you get error about .mid files, more packages needed. I used:
#    sudo apt-get install ubuntu-restricted-extras vlc-plugin-fluidsynth
#    (use tab and space to navigate the weird dialogs)
#    or just don't use .mid files

CXX = clang++
MOCQT4 = moc-qt4
# tries later to check if it's the course VM, and changes CXX to 'compile'

EXAMPLES = pulse bounce earth polygon iterpoints htree nestedcircles

all: draw.o $(EXAMPLES)

clean:
	rm -f draw.o polygon.png bull $(EXAMPLES)

fresh: clean all

INCL = -I/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui
OFLAGS = $(INCL) -Wall -Wno-unreachable-code -Wno-return-type
FLAGS = -g -Wall -lQtCore -lQtGui

ifdef audio
OFLAGS += -DDRAW_UNMUTE -I/usr/include/phonon
FLAGS += -lphonon
endif

ifdef stub
FLAGS =
endif

# this is for CS 103 at USC specifically.
# bash doesn't expand aliases. patch student machine. prefer 'compile'.
ifeq ("$(wildcard /bin/compile)","")
ifneq ("$(wildcard ~/.compile.py)","")
PATCH = sudo cp ~/.compile.py /bin/compile; sudo chmod a+x /bin/compile
CXX = compile
endif
else
CXX = compile
endif

draw.o: draw.cpp draw.h
	$(PATCH)
	$(MOCQT4) draw.cpp | $(CXX) $(OFLAGS) -c -x c++ - -include draw.cpp -o draw.o

%: %.cpp draw.o draw.h
	$(CXX) $@.cpp draw.o $(FLAGS) -o $@

# prepare a zip that people not using git can use
zip:
	rm -f draw.zip
	zip -r draw.zip *.cpp draw.h Makefile media README

stub:
	$(CXX) -c drawstub.cpp -o draw.o
