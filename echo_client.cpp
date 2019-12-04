#include <stdio.h> // for perror
#include <string.h> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <thread>
using std::thread;

struct sockaddr_in addr;
int sockfd;

void print_rcv(){
	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];

		ssize_t received = recv(sockfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);
	}
}

int main(int argc, char* argv[]) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2])); // port
	addr.sin_addr.s_addr = inet_addr(argv[1]); // host
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("connect failed");
		return -1;
	}
	printf("connected\n");
	thread t(print_rcv);

	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];

		scanf("%s", buf);
		if (strcmp(buf, "quit") == 0) break;

		ssize_t sent = send(sockfd, buf, strlen(buf), 0);
		if (sent == 0) {
			perror("send failed");
			break;
		}
	}
	t.detach();
	close(sockfd);
}
