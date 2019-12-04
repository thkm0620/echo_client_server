#include <stdio.h> // for perror
#include <string.h> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <vector>
#include <map>
#include <thread>
#include <mutex>
using namespace std;

int check; // -b : 1
map<int,int> mp;  // <childfd, 1 or 0 >
vector<thread> vc;
mutex mu;

void print_rcv(int childfd)
{
	if(check){
		mu.lock();
		mp[childfd]=1;
		mu.unlock();
	}
	while(true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];
		ssize_t received = recv(childfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			if(check){
				mu.lock();
				mp[childfd]=0;
				mu.unlock();
			}
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);

		if(check){
			for(auto it=mp.begin(); it!=mp.end(); it++){
				if(it->second){
					ssize_t sent = send(it->first, buf, strlen(buf), 0);
					if (sent == 0) {
						perror("send failed");
						mu.lock();
						it->second=0;
						mu.unlock();
					break;
					}
				}
			}
			
 		}
		else{
			ssize_t sent = send(childfd, buf, strlen(buf), 0);
			if (sent == 0) {
				perror("send failed");
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc==3) check=1;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &optval , sizeof(int));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("bind failed");
		return -1;
	}

	res = listen(sockfd, 2);
	if (res == -1) {
		perror("listen failed");
		return -1;
	}

	while (true) {
		struct sockaddr_in addr;
		socklen_t clientlen = sizeof(sockaddr);
		int childfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &clientlen);
		if (childfd < 0) {
			perror("ERROR on accept");
			break;
		}
		printf("connected\n");
		vc.push_back(thread(print_rcv,childfd));
	}
	for(int i=0; i<vc.size(); i++) vc[i].detach();

	close(sockfd);
}
