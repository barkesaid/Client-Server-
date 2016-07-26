#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include <pthread.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX 1024


struct clientdata {
	unsigned ip;
	int port;
	int id;
};

void sendMessageToClient();
void sendMessageToServer(int sock, char *message);
void decodeClientData(char *);
void *startListen(void *port);

struct clientdata clients[72];
int noOfClients;



int main(int argc, char *argv[])
{
	pthread_t thread1;
	int sockfd,portno,n, i, choice;
	struct sockaddr_in serv_addr,my_addr;
	struct hostent *server;
	char message[255]; char *path = "clients.txt"; 
	FILE *fp; int k;char buffer[MAX];

	if (argc <3){
		fprintf(stderr , "usage %s hostname port \n",argv[0]);
		return 0;
	}

	portno=atoi(argv[2]); 
	int myport;
	printf("Enter client port number\n");
	scanf("%d",&myport );

	//create a socket point
	puts("[log] Open socket");
	sockfd = socket (AF_INET,SOCK_STREAM,0);


	if(sockfd<0){
		perror("Error opening the socket");
		return 1;
	}

	puts("[log] Bind socket");
	bzero((char *) &my_addr, sizeof( ));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = myport;

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	  error("setsockopt(SO_REUSEADDR) failed");

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
	 perror("Error binding");
	 exit(1);
  }

	puts("[log] Connect to server");
	server=gethostbyname(argv[1]);

	if(server == NULL) {
		fprintf(stderr, "error ,no such host");
		return 0;
	}

	bzero((char *) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port =htons(portno);

	//connect to server
	if(connect (sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
		perror ("Error connecting");
		return 1;
	}

	//read from the file of server
	puts("[log] Read client list from server");
	bzero(buffer,256);
	n = read(sockfd, buffer, 255);
	if(n<0){
		perror("Error reading from socket");
		return 1;
	}

	puts("[log] Write client list to file");
	fp = fopen(path, "w");
	puts(buffer); //print to screen
	fprintf(fp, "%s", buffer); //write to file
	fclose(fp);
	puts("[log] Writen client list to file"); //successfully written to file


	decodeClientData(path);
	
	close(sockfd);
	sleep(2);
printf("1. Listen for connections\n"
	   "2. Connect to a peer\n");
		 printf("choice: ");
		 scanf("%d", &choice);
		 if (choice==1) {
			 //start listening thread
 				int iret1 = pthread_create(&thread1,NULL, startListen,(void*)myport);
 				 if(iret1)
 					 {
 							 perror("Failure creating listener thread");
 							 exit(EXIT_FAILURE);
 					 }
 					 else {
 			 			printf("Socket listening\n");
 			 		}
					//keep listening
					for(;;){

					}
		 }
		 else if(choice ==2){
			 int chosenport; 
			//list the available clients to connect to
			 for(i = 0; i < noOfClients; i++){
			 	printf("Client %d %d %d %d\n", i + 1, clients[i].ip, clients[i].port, clients[i].id);
			}
			
			 printf("Enter Port to connect : ");
			 scanf("%d", &chosenport);
			 int newsockfd = socket (AF_INET,SOCK_STREAM,0);

			 sendToClient(newsockfd,chosenport);
		 }


	return 0;
}


void decodeClientData(char *path)
{
	int port, id, ip, i, j;
	char *str, *buffer;
	FILE *fp = fopen(path, "rb+");
	noOfClients = 0;

	buffer = malloc(sizeof(char)*254);
	str = malloc(sizeof(char)*254);

	i = 0;
	while((fgets(str, 250, fp)) != NULL){
	
		j = 0;
		while((buffer = strtok(str, "\t")) != NULL){ //break  string into tokens
			str = NULL;
			if(j == 0)
				clients[i].ip = atoi(buffer);
			else if(j == 1)
				clients[i].port = atoi(buffer);
			else if(j == 2)
				clients[i].id = atoi(buffer);
			j++;
			
		}
		str = realloc(str, sizeof(char)*254);
		i++;
	
	}
	noOfClients = i;

	fclose(fp);
}


void sendMessageToServer(int sock, char *message)
{
	int n;

	n = write(sock, message, strlen(message));
	if(n<0){
		perror("[Error] Writing to socket");
		_exit(1);
	}
}

void *startListen(void *port){
  int sockfd;
  int newsockfd,pid;
  struct sockaddr_in cli_addr,my_addr;
  socklen_t clilen;

	sockfd = socket (AF_INET,SOCK_STREAM,0);
	if(sockfd<0){
		perror("Error opening the socket");
		return 1;
	}
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		error("setsockopt(SO_REUSEADDR) failed");

	bzero((char *) &my_addr, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = port;

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
	 perror("Error binding");
	 exit(1);
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  while(1){
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    printf("Connected to port %d n",cli_addr.sin_port);

    if (newsockfd < 0) {
       perror("ERROR on accept");
       exit(1);
    }
    /* Create child process */
    pid = fork();
    if (pid < 0) {
       perror("ERROR on fork");
       exit(1);
    }
    if (pid == 0) {
       /* This is the client process  */
       close(sockfd);   
       onReceive(newsockfd);// if child process created  successfully 
       exit(0);
    }
    else {
       close(newsockfd);
    }

  }
}
void onReceive(int sockfd){
	int n; 
	//buffers
	char buffer[256],buffer2[256];
	bzero(buffer,256);
	bzero(buffer2,256);

	while(1){
	n = read(sockfd, buffer2, 255);
	if (n < 0) {
		 perror("Error reading from peer");
		 exit(1);
	}
	//display what was receiived
	printf("Received : %s\n",buffer2 );
//write/reply to what was received 
	fgets(buffer,255,stdin);
 	n = write(sockfd,buffer,255);

 if (n < 0) {
		perror("Error writing to socket");
		exit(1);
 }
 }
}

void sendToClient(int sockfd,int port){
	struct sockaddr_in peer_addr;
  struct hostent *server;
  server = gethostbyname("127.0.0.1");
  // pthread_t peerthread1, peerthread2;

  bzero((char *) &peer_addr, sizeof(peer_addr));
  peer_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,(char *)&peer_addr.sin_addr.s_addr, server->h_length);
  peer_addr.sin_port = port;
  printf("Connecting to peer on Port... %d\n",peer_addr.sin_port );

  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
     perror("Error connecting");
     exit(1);
	 }
	 char buffer[256],buffer2[256];
	 bzero(buffer,256);
	 bzero(buffer2,256);
	 int n;
	 puts("Enter a message : ");

	 while(1){
		 fgets(buffer,255,stdin);
	 n = write(sockfd,buffer,255);
	 if (n < 0) {
		 perror("Error writing to socket");
		 exit(1);
	 }
	 n = read(sockfd, buffer2, 255);
	 if (n < 0) {
			perror("Error reading from peer");
			exit(1);
	 }
	 printf("Received : %s\n",buffer2 );

 }
}
