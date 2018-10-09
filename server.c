/*-------------------------------------------------------------------------------
 *File name : server.c
 *Author    : Steve Antony Xavier Kennedy


-------------------------------------------------------------------------------*/

// Header section
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdint.h>

#define BUFFERSIZE (2048)

int main(int argc, char *argv[])
{

	//creating the socket
	int server_socket, new_socket;
	char request[BUFFERSIZE]; // for holding values
	char method[10];
	char version[10];
	char url[100];
	
	FILE *fp;
	char content_type[10];
	char content_length[10];
	char response[BUFFERSIZE];

	if(argc<2)// passing ip address of server and port number as command line argument
	{
	        printf("\n Please provide port number\n");
	        exit(1);
        }
	
	socklen_t clilen;
	struct sockaddr_in server_address, to_address;
	server_socket = socket(AF_INET,SOCK_STREAM,0);// setting the server socket
	if(server_socket < 0)
	{

		perror("in socket");
		exit(EXIT_FAILURE);
	}
	memset(&server_address,0,sizeof(server_address));

	int true = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr	= INADDR_ANY;
	server_address.sin_port = htons(atoi(argv[1]));

	//bind the server socket with the remote client socket
	if(bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		printf("\nBinding failed\n");
	}
	listen(server_socket,3);
	printf("\nlistening.....\n");


	while(1)
	{
		clilen = sizeof(to_address);
		new_socket = accept(server_socket,(struct sockaddr*) &to_address, &clilen);
		if(new_socket<0)
		{
			printf("\nerror on accept\n");
		}
		printf("\nConnection established...\n");
	
		if(!fork())
		{
			close(server_socket);
			memset(request,0,BUFFERSIZE);
			memset(method,0,10);
			memset(url,0,20);
			memset(version,0,10);

			read(new_socket,request, BUFFERSIZE);
			printf("\n%s",request);

			sscanf(request,"%s%s%s",method, url, version);
			printf("\nRequest method - %s\n",method);
			printf("Request url       - %s\n",url);
			printf("HTTP Version     - %s\n",version);

			/* setting the path of url*/
			int len = strlen(url);
			char url_path[100];
			for(int i =0; i<len ; i++)
			{
				url_path[i] = url[i+1];
			}
			int new_length = strlen(url_path);
			url_path[new_length] = '\0';
			printf("URL : %s\n",url_path);

			if(((strcmp(method,"GET")==0))&&((strcmp(version,"HTTP/1.1")==0)||(strcmp(version,"HTTP/1.0")==0)))
			{
				/*Checks if it is valid extension*/
				if(strcmp(url,"/")==0)
				{
					printf("\nEntered home\n");
					fp = fopen("www/www/index.html","rb");
					if(fp)
						printf("\nFile opened");
					else
						printf("\nFile not present\n");
				}
				else
				{
					fp = fopen(url_path,"rb");// rectify this part
				}

			}
			if(fp!=NULL)
			{
				printf("\nFile Exists\n");
				fseek(fp,0,SEEK_END);
				int file_size = ftell(fp);
				fseek(fp,0,SEEK_SET);

				printf("\nFile size %d\n",file_size);
				/*Converting integer to string*/
				sprintf(content_length,"%d",file_size);
				
				printf("\nContent length = %s\n",content_length);

				if(strcmp(url,"/")==0)
				{
					strcpy(content_type,".html");
					printf("Content type %s\n",content_type);
				}
				else
				{
					char *pt = strstr(url,".");
					strcpy(content_type,pt);
					printf("\nContent type %s\n",content_type);
				}
				memset(response,0,sizeof(response));
				strcpy(response,"HTTP/1.1 200 Document Follows\r\nContent-Type: ");
				strcat(response,content_type);
				strcat(response,"\r\nContent-Length:");
				strcat(response,content_length);
				strcat(response,"\r\n\r\n");
				write(new_socket,response,strlen(response));
				printf("\nresponse %s\n",response);
				char *c = (char*)malloc(file_size);
				if(c)
				{
					fread(c,1,file_size, fp);
					printf("File content\n%s\n",c);
					write(new_socket,c,file_size);
					free(c);
				}
			}
	
			close(new_socket);
			printf("closing");
			exit(0);
		}
		close(new_socket);

	}
	close(new_socket);
	return 0;
}


