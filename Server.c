#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

// details about a client
struct clientdata {
	unsigned int ip;
	int port;
	int id;
};


int main(int argc, char *argv[])
{

	int sockfd; int newsockfd; int portno; int clilen;
	int n; 
	struct sockaddr_in serv_addr,cli_addr; int pid;
	struct clientdata client; int i; 
	FILE *fp;
	char *path ="connectedclients.txt", *buffer;

	//creating the socket
	puts("[log] Creating socket");
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd<0){
		perror("Error opening the socket");
		return 1;
	}

	//iniatialize the socket structure
	bzero((char *) &serv_addr,sizeof(serv_addr));
	portno=5700;

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY; //bind to all interfaces not localhost alone
	serv_addr.sin_port=htons(portno);

	//binding
	if(bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0){
		perror("Error on binding ");
		return 1;
	}

	//put client to listening state
	puts("[log] Put client to listening state");
	listen(sockfd,5);
	clilen=sizeof(cli_addr);

	while(1) {
		//accept connection from client
		puts("[log] Accept connection from client"); 
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
		if(newsockfd <0){
			perror("Error on accept ");
			return 1;
		}
		puts("[log] Accepted connection from client");
	
		//if the connection was accepted then 
		//get the client infomation
		client.port =cli_addr.sin_port; //port number
		client.ip =cli_addr.sin_addr.s_addr; 
		client.id = rand()%10; //an id
	

		//write the connected client to file
		puts("[log] write details about client to file");
		fp = fopen(path, "a+");
		fprintf(fp,"%d\t%d\t%d\n",client.port,client.ip,client.id);
		fclose(fp);
		//end writing client details to file
	
		//send list of other connected clients to new connected client
		//read the whole file into a string
		fp =fopen(path, "rb+");
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp); //current file position
		fseek(fp, 0, SEEK_SET);
		

		buffer = malloc(sizeof(char)*512);
		fread(buffer, fsize, 1, fp);
		fclose(fp);
	
		buffer[fsize] = '\0'; //set EOF
		puts("[log] Read file into buffer");
		

		// send the file
		puts("[log] Send list of clients to client");
		n = write(newsockfd, buffer, fsize);
		if(n < 0)
		{
			perror("Error sending client list to client");
			return 1;
		}
		puts(buffer);
		// end read and send client list to newly connected client.
		
		//create child process
		pid = fork();
		if(pid<0){
			perror("Error in fork");
			return 1;
		}

		if(pid == 0){
			//This is the child process
			close(sockfd);
						
			bzero(buffer, sizeof(buffer));
			puts("[log] Read message from client");
			n = read(newsockfd, buffer, 255);
			if(n<0){
				perror("Error reading from socket");
				return 1;
			}

			printf("Message: %s\n", buffer);
		
		}else{
			close(newsockfd);
		}

	}
	return 0;
}
