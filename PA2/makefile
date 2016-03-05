main: packet client server

packet:
	g++ -c packet.cpp -o packet.o

client:
	g++ packet.o client.cpp -o client.o
	
server:
	g++ packet.o server.cpp -o server.o

zip:
	\rm *.zip
	zip pa2.zip client.cpp server.cpp makefile

clean:
	\rm *.o