main: client server

client:
	g++ client.cpp -o client.o
	
server:
	g++ server.cpp -o server.o	

zip:
	\rm *.zip
	zip pa2.zip client.cpp server.cpp makefile

clean:
	\rm *.o received.txt