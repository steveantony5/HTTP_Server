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

#define HEADER (500)
#define LISTEN_MAX 10
#define BUFFERSIZE (2048)
/*
void error(char *string)
{
	printf("%s\n",string);
}
*/

int main(int argc, char *argv[])
{

	//creating the socket
	int server_socket, new_socket;
	/*For request header*/
	char request[HEADER];
	char method[10];
	char version[10];
	char url[100];
	
	FILE *fp;
	char content_type[100];
	char content_length[40];

	/*For response header*/
	char response[HEADER];

	/*For setting socket timeout in Pipelining*/
	struct timeval tv;

	if(argc<2)// passing port number as command line argument
	{
	        perror("Please provide port number");
	        exit(1);
        }
	
	socklen_t clilen;
	struct sockaddr_in server_address, to_address;
	server_socket = socket(AF_INET,SOCK_STREAM,0);// setting the server socket
	if(server_socket < 0)
	{

		perror("error on socket creation");
		//exit(1);
	}
	memset(&server_address,0,sizeof(server_address));

	/*Setting socket so as to reuse the port number*/
	int true = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr	= INADDR_ANY;
	server_address.sin_port = htons(atoi(argv[1]));

	//bind the server socket with the remote client socket
	if(bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		perror("Binding failed");
	}


	if(listen(server_socket,LISTEN_MAX) < 0)
	{
		perror("Error on listen");
	}
	else
		printf("\nlistening.....\n");


	while(1)
	{
		new_socket = 0;
		clilen = sizeof(to_address);
		new_socket = accept(server_socket,(struct sockaddr*) &to_address, &clilen);
		if(new_socket<0)
		{
			printf("\nerror on accept\n");
		}
		else
		{
			printf("\nConnection established...\n");
		}
	
		/*Creating child processes*/
		if(!fork())
		{
start:
			memset(request,0,HEADER);
			memset(method,0,10);
			memset(url,0,20);
			memset(version,0,10);

			/*Receiving the request from client*/
			recv(new_socket,request, HEADER,0);
			printf("\n%s",request);
			
			/*Pipelining*/
			char *alive = strstr(request,"Connection: keep-alive");
			if(alive!=NULL)
			{
				tv.tv_sec = 10;
				printf("\nConnection : keep-alive present in request\n");
				setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
			}
			else
			{
				tv.tv_sec = 0;
			        setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
			}

			/*If no request received, close the active socket*/
			if(strlen(request) == 0)
				break;

			/*separating the parameters from request string*/
		 	sscanf(request,"%s%s%s",method, url, version);
			printf("\nRequest method - %s\n",method);
			printf("Request url       - %s\n",url);
			printf("HTTP Version     - %s\n",version);


			/* setting the path of url*/
			int len = strlen(url);
			char url_path[100];
			memset(url_path,0,100);

			/*Setting the default file to display*/
			if(strcmp(url,"/")==0)
			{
				strcpy(url_path,"www/www/index.html");
				goto A;
			}

			/*Setting the url path for the file to open*/
			for(int i =0; i<len ; i++)
			{
				url_path[i] = url[i+1];
			}
			int new_length = strlen(url_path);
			url_path[new_length] = '\0';
			printf("URL : %s\n",url_path);
			
			/*Checks if the folder exists*/
A:			fp = NULL;			
			fp = fopen(url_path,"rb");
			int file_check = 0;
			if(fp!=NULL)
			{
				/*Checks if the file exists*/
				if(fseek(fp,0,SEEK_END) < 0)
					perror("error on seek");
				
				file_check= ftell(fp);
				fseek(fp,0,SEEK_SET);
			}

			/*GET Request*/
			/*Checks the request type, file existance, HTTP version number*/
			if(((strcmp(method,"GET")==0))&&((strcmp(version,"HTTP/1.1")==0)||(strcmp(version,"HTTP/1.0")==0)) && (fp!=NULL)&&(file_check>=0))
			{
				printf("\nFile Exists\n");
					
				/*Calculating the size of requested file*/
				fseek(fp,0,SEEK_END);
				int file_size = 0;
				file_size= ftell(fp);
				fseek(fp,0,SEEK_SET);

				printf("\nFile size %d\n",file_size);
				/*Converting file size from integer to string*/
				sprintf(content_length,"%d",file_size);
				
				printf("\nContent length = %s\n",content_length);

				/*storing the content type of the requested file*/
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
			
				/*Forming the response header*/	
				memset(response,0,sizeof(response));

				if(strcmp(version,"HTTP/1.1")==0)
				{
					strcpy(response,"HTTP/1.1 200 Document Follows\r\nContent-Type: ");
				}
				else if(strcmp(version,"HTTP/1.0")==0)
				{
				        strcpy(response,"HTTP/1.0 200 Document Follows\r\nContent-Type: ");
				}
				strcat(response,content_type);
				strcat(response,"\r\nContent-Length:");
				strcat(response,content_length);
				strcat(response,"\r\nConnection:");
				if(alive!=NULL)
				{
					strcat(response,"keep-alive");
				}
				else
				{
					strcat(response,"close");
				}
				strcat(response,"\r\n\r\n");

				/*Sending the response header*/
				write(new_socket,response,strlen(response));
				printf("\nresponse %s\n",response);

				/*Reading the requested file*/
				char *file = (char*)malloc(file_size);
				if(file)
				{
					fread(file,1,file_size, fp);
					//printf("File content\n%s\n",file);
					/*Sending the requested file*/
					write(new_socket,file,file_size);
					free(file);
					printf("\n*********************************\n");
				}

			}
			/*POST Request*/
			/*Checks request method, file existance, HTTP version*/
			else if(((strcmp(method,"POST")==0))&&((strcmp(version,"HTTP/1.1")==0)||(strcmp(version,"HTTP/1.0")==0)) && (fp!=NULL)&&(file_check>=0))
			{
				char postdata[400];
				char postmessage[400];
				int32_t length_postdata = 0;
				int32_t length_total = 0;
				int32_t file_size = 0;
				char return_length[10];
				printf("File Exists\n");
				char *pointer = strstr(request,"Content-Length:");
				sscanf(pointer,"%*s%d",&length_postdata);
				/*Storing the data to be posted*/
				while(1)
				{
					pointer++;
					if(*pointer == '\n')
					{
						pointer = pointer+2;
						strcpy(postdata,pointer);
						break;
					}
				}
				printf("Postdata %s",postdata);

				/*Data to be posted in html*/
				sprintf(postmessage,"<html><body><pre><h1>%s<h1></pre>",postdata);

				/*Calculating the requested file size*/
				fseek(fp,0,SEEK_END);
				file_size= ftell(fp);
				fseek(fp,0,SEEK_SET);
                                printf("\nFile size %d\n",file_size);
				length_total = file_size+strlen(postmessage);		
				/*Converting integer to string*/
                                sprintf(return_length,"%d",length_total);
                                printf("\nreturn length = %s\n",return_length);
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
                                 if(strcmp(version,"HTTP/1.1")==0)
                                 {
                                        strcpy(response,"HTTP/1.1 200 Document Follows\r\nContent-Type: ");
                                 }
                                 else if(strcmp(version,"HTTP/1.0")==0)
                                 {
                                        strcpy(response,"HTTP/1.0 200 Document Follows\r\nContent-Type: ");
                                 }
                                 strcat(response,content_type);
                                 strcat(response,"\r\nContent-Length:");
                                 strcat(response,return_length);
		                 strcat(response,"\r\nConnection:");
				 if(alive!=NULL)
			     	 {
				        strcat(response,"keep-alive");
				 }
				 else
				 {
				        strcat(response,"close");
				 }
				 strcat(response,"\r\n\r\n");
				 write(new_socket,response,strlen(response));
				 printf("Response\n%s",response);
				 char *c = (char*)malloc(file_size);
				 if(c)
				 {
					 fread(c,1,file_size, fp);
					 printf("File content\n%s%s\n",postmessage,c);
                                         write(new_socket,postmessage,strlen(postmessage));
					 write(new_socket,c,file_size);
					 free(c);
					 printf("\n*********************************\n");
				}

			}

			else
			{
				printf("\nError on request\n");
				memset(response,0,sizeof(response));
				if(strcmp(version,"HTTP/1.1")==0)
				        strcpy(response,"HTTP/1.1 500 Internal Server Error\r\nContent-Type: ");
				else if(strcmp(version,"HTTP/1.0")==0)
					strcpy(response,"HTTP/1.0 500 Internal Server Error\r\nContent-Type: ");
				else
					strcpy(response,"500 Internal Server Error\r\nContent-Type: ");

				strcat(response,".html\r\n");
				char message[100];
				sprintf(message,"<html><body><h1> 500 Internal Server Error: %s </h1></body></html>",url);
				strcat(response,"Content-Length: ");
				int size_error = sizeof(message);
				/*Converting integer to string*/
				char size_error_str[8];
				sprintf(size_error_str,"%d",size_error);

				strcat(response,size_error_str);
				strcat(response,"\r\nConnection:");
				if(alive!=NULL)
				{
				        strcat(response,"keep-alive");
				}
				else
				{
				        strcat(response,"close");
				}
				strcat(response,"\r\n\r\n");
				printf("\nResponse Header: \n%s",response);
				write(new_socket,response,strlen(response));

				write(new_socket,message,strlen(message));
				printf("\n*************************************\n");

			}
			fclose(fp);

			if(alive!=NULL)
				goto start;
			else
			{
				close(new_socket);
				printf("Closing socket\n");
			}
		
			printf("\nEnd of fork\n");			
			exit(0);
				
		}
		close(new_socket);
		printf("\nClosing while\n");


	}
	close(server_socket);
	return 0;
}


