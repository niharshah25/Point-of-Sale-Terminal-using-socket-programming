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
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<pthread.h>

#define MAXLINE 100
#define backlog 5
using namespace std;
void* handler(void *conns);
int records; //keeps track of number of records in the database
int get_index(int);//searches and returns item_code supplied in the database and returns accordingly
vector<string> vec;
// listensd and connsd are file descriptor associated with file
int listensd,connsd;

// For appropriately handling database This function reads the database and store the values in the vector.
void database_new(string filename) {
	ifstream file;
	file.open(filename);
	string k; // To temporary hold the value of the database field
	int count = 0; 
	while(file >> k) {
		if(count == 0)
			records=stoi(k);		
		else
			vec.push_back(k);
		count++;
	}
	return;
}
// For handling CTRL + C signal Appropriately
void signal_handler(int sig) {
	char msg[MAXLINE];
	close(listensd);
	fputs("\nServer is Closing.",stdout);
	sprintf(msg,"4 : Server is Closed\n");
	send(connsd,msg,MAXLINE,0);
	exit(0);
}

int main(int argc, char *argv[]) {
	socklen_t Clilen; //Clilen is the length of the client socket, used as a value-result argument
	struct sockaddr_in ServAddr, CliAddr;
	if(argc<2) {
		cout<<"Please Pass Ip address and Port Number Appropriately in arguement respectively"<<endl;
		exit(0);
	}
	// Converting Port No to integer
	int SERVER_PORT = atoi(argv[2]);
	//creating the socket
	listensd=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(listensd<0) {
		perror("Cannot create socket!");
		return 0;
	}

	//initializing the server socket
	ServAddr.sin_family=PF_INET;
	//Converting the input ip appropriately 
	ServAddr.sin_addr.s_addr = inet_addr(argv[1]);
	//Host to network byte order 
	ServAddr.sin_port = htons(SERVER_PORT); 

	//binding socket
	if(bind(listensd,(struct sockaddr *) &ServAddr, sizeof(ServAddr))<0) {
		perror("Cannot bind port!");
		return 0;
	}
	
	//defining number of clients that can connect through SERVER_PORT , Backlog defined as 5
	listen(listensd,backlog);

	//Storing Value from Database into Vector for Servicing Request of Client
	database_new("database.txt");

	signal(SIGINT,signal_handler);
	// Handling Multiple Clients simultaneously
	while(1) {
		Clilen=sizeof(CliAddr);
		if((connsd=accept(listensd,(struct sockaddr *)&CliAddr,&Clilen))<0) {
			perror("Cannot establish connection!");
			return 0;
		}
		// Creating Thread for each Client  
		pthread_t tchild;
		int *psocket=new int;
		*psocket=connsd;
		pthread_create(&tchild,NULL,handler,psocket);
	}
}

int get_index(int item_code) {
	for(int i = 0; i < records*3; i += 3) 
		if(item_code==stoi(vec[i]))
			return i;
	return -1;
}

void* handler(void* conns) {
	int len, token_ctr, request_type, quantity, item_code, index;
	double total = 0.0;
	char buffer[MAXLINE], msg[MAXLINE], *token;
	int connsd = *((int*)conns);
	free(conns);
	printf("Processing request of Thread_id : %ld\n",(pthread_self()));
	while(1) {
		len=token_ctr=index=quantity=0;

		memset(msg,0,MAXLINE); //clears contents of msg

		len=recv(connsd,buffer,MAXLINE,0);
		if(len<0) {
			sprintf(msg,"3 : Encountered error receiving command. closing..\n");
			send(connsd,msg,MAXLINE,0);
			close(connsd);
			exit(0);
		}

		if(strcmp(buffer,"SIGINT")==0) {
			printf("\nChild process terminated abnormally..\n");
			close(connsd);
			return NULL;
		}

		token=strtok(buffer," ");
		request_type=stoi(token);
		int flag=0;
		while(token!=NULL) {
			token=strtok(NULL," ");
			if((token==NULL && token_ctr==0) || (token==NULL && token_ctr==1)) {
				if(request_type==1) {
					sprintf(msg,"1 : Closing the request, Total cost = %f",total);
					send(connsd,msg,MAXLINE,0);
					cout << "Closing the Thread with id : "<<pthread_self()<<endl;
					close(connsd);
					return NULL;
				}
				else {
					sprintf(msg,"2 : Fromat is incorrect, Please resend packet in correct format.\n");
					send(connsd,msg,MAXLINE,0);
					flag=1;
					break;
				}
			}
			if(token_ctr==0)
				item_code=stoi(token);
			if(token_ctr==1)
				quantity=stoi(token);
			token_ctr++;
		}
		if(flag==1)
			continue;

		if(token_ctr<2) {
			sprintf(msg,"2 : Protocol error, discarding packet. Please send the request again.\n");
			send(connsd,msg,MAXLINE,0);
		}
		else {
			if(request_type==0) {
				index=get_index(item_code);
				if(index>=0) {
					total=total+(stof(vec[index+2])*quantity);
					char * arra = new char[100];
					for(int i=0;i<vec[index+1].length();i++)
						arra[i]=vec[index+1][i];
					sprintf(msg,"0 : Price = %f\tItem name: %s\tCurrent Total Cost :%f",stof(vec[index+2]),arra,total);
					send(connsd,msg,MAXLINE,0);
				}
				else {
					sprintf(msg,"2 : UPC code %d not found, Please resend the request!\n",item_code);
					send(connsd,msg,MAXLINE,0);
				}
			}
			else if(request_type==1) {
				sprintf(msg,"1 : Total cost = %f",total);
				send(connsd,msg,MAXLINE,0);
				cout<<"Closing the Thread with id : "<<pthread_self()<<endl;
				close(connsd);
				return NULL;
			}
			else {
				sprintf(msg,"2 : Protocol error, discarding packet. Please resend the request!\n");
				send(connsd,msg,MAXLINE,0);
			}
		}
	}
}
