EXEC=quest
OBJFILES=quest.o

#change these variables to suit your compiler/environment

CPP=g++
RM=rm -rf
#RM=erase
LIBS=`allegro-config --libs`
#LIBS=-lalleg

.SUFFIXES: .o.cpp

%.o: %.cpp
	${CPP} $< -c -o $@ -O2 -Wall

${EXEC}: ${OBJFILES}
	${CPP} ${OBJFILES} -o ${EXEC} -Wall -O2 ${LIBS}

clean:
	${RM} ${EXEC} ${OBJFILES}

dist:
	${RM} quest.tar.gz
	${RM} quest.zip
	tar -cf quest.tar quest-0.8
	gzip quest.tar
	zip -r quest quest-0.8
