#HEADERS = program.h headers.h

default: event-app-test

event-app-test.o: event-app-test.cpp $(HEADERS)
	g++ -c event-app-test.cpp -o event-app-test.o

event-app-test: event-app-test.o
	g++ event-app-test.o -o event-app-test

clean:
	 -rm -f event-app-test.o
	 -rm -f event-app-test
