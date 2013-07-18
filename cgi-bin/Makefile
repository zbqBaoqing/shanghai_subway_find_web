INCLUDES=-L/usr/include/mysql
LIBS=-L/usr/lib/mysql -lmysqlclient

server:server.o station_insert.o 
	gcc server.o station_insert.o $(LIBS) -o server -Wall
server.o:server.c station_insert.h
	gcc -c $(INCLUDES) $(LIBS) server.c -o server.o -Wall
station_insert.o:station_insert.c station_insert.h
	gcc -c $(INCLUDES) $(LIBS) station_insert.c -o station_insert.o -Wall
clean:
	-rm -f *.o
