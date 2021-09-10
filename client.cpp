/**
* This code implements Point-of-Sale Terminal using socket programming.
* Assignment-2
* Group-12
* Saurabh Jaiswal (204101050), Shaantanu Mishara (204101051), Shah Nihar
  Darshanbhai (204101052), Shivank Thapa(204101053)
* Date: 24/02/2021
*/
#include<bits/stdc++.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<unistd.h>
#include<ctype.h>

using namespace std;
#define MAXLINE 100
int sockfd; //client socket descriptor
int command_process(int sockfd) {
	char send_buff[MAXLINE],recv_buff[MAXLINE];
	cout<<endl<<"Enter the Request in Following Format :"<<endl<<"<request type> <item code> <quantity> "<<endl;
	while(1) {
		memset(send_buff,0,MAXLINE);
		memset(recv_buff,0,MAXLINE);

		fgets(send_buff,MAXLINE,stdin);
		send(sockfd,send_buff,MAXLINE,0);
		recv(sockfd,recv_buff,MAXLINE,0);
	
		if(recv_buff[0]=='0')
			printf("\n%s\n",recv_buff);

		if(recv_buff[0]=='1' || recv_buff[0]=='4') {
			printf("\n");
			fputs(recv_buff,stdout);
			close(sockfd);
			return 1;
		}			

		if(recv_buff[0]=='2') {
			printf("\n");
			fputs(recv_buff,stdout);
			cout<<endl<<"Enter the Request in Following Format :"<<endl<<"<request type> <item code> <quantity> "<<endl;
			continue;
		}
		
		if(recv_buff[0]=='3') {
			printf("Error encountered, Exiting..\n");
			close(sockfd);
			return -1;
		}	
	}	
}

void signal_handler(int sig) {
	char send_buff[MAXLINE];
	fputs("\nClient is closing",stdout);
	sprintf(send_buff,"SIGINT");	
	send(sockfd,send_buff,MAXLINE,0);
	close(sockfd);
	exit(0);
}
int main(int argc, char *argv[])
{
	int check;
	if(argc<2) {
		cout<<"Please Pass IP address of Server and Port No Appropriately to establish connection "<<endl;
		exit(0);	
	}
	int SERVER_PORT = stoi(argv[2]);
	struct sockaddr_in ServAddr;

	//creating the socket
	sockfd=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(sockfd<0) {
		perror("Cannot create socket!");
		return 0;
	}

	//initializing the server socket
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr(argv[1]); //using the imput IP 
	ServAddr.sin_port = htons(SERVER_PORT); //self defined server port

	if((connect(sockfd,(struct sockaddr *) &ServAddr,sizeof(ServAddr)))<0) {
		perror("Server is down!");
		return 0;
	}

	signal(SIGINT,signal_handler);
	cout << "Connection is now established.." << endl;

	while(1) {
		check=command_process(sockfd);
		if(check < 0) {
			printf("Closing the socket, Exiting..\n");
			close(sockfd);
			exit(0);
		}
		else if(check==1) {
			close(sockfd);
			exit(0);
		}
	}

	return 0;
}

