all: echo_client echo_server

echo_client:
	g++ -o echo_client echo_client.cpp -pthread

echo_server:
	g++ -o echo_server echo_server.cpp -pthread

clean:
	rm -f echo_client *.o
	rm -f echo_server *.o
