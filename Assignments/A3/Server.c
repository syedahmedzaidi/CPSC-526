#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <openssl/sha.h>
#include "myHeader.h"
#define SOCKET_ERROR 0xFFFFFFFF
#define INVALID_SOCKET 0xFFFFFFFF

int main(int argc, char *argv[])
{

  int port;
  char key[1024];
  char key2[1024];
  int bytes,character;
  char mensagem[2],*message,*welcome,*termPointer;
  char buffer[1024] = "";
  char sendMessage[1024] = "";
  unsigned char iv[1024]="";
  char cpUsing[1024] = "";
  char cmr[1024] = "";
  unsigned char *initialVector;
  unsigned char *shared = "sharedSecret";
  unsigned char* keyProtocol;
  int cipher = 0; // 0 for none, 1 for aes128, 2 for aes256
  FILE *command;
  struct sockaddr_in server;
  time_t mytime;
	mytime = time(NULL);

  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);

  if( argc > 3 )
  {
    printf("\nToo many arguments supplied. Please supply a port number and a key if you wish to do so.\n");
    return 0;
  }

  else if( argc == 3 )
  {
  	port = atoi(argv[1]);
    if(strlen(key) < 32){
      strcpy(key, argv[2]);
    }else{
      strcpy(key, argv[2]);
      key[32] = '\0';
    }
    int i=0;
    for(i =0; i < 16; i++)
    {
      key2[i] = key[i];
    }
    key2[16] = '\0';
  	printf("\nUsing port %i\n",port); //msg to see which port is used
    printf("\nAES128 Secret key: %s\n",key2); //msg to see which port is used
  	printf("\nAES256 Secret key: %s\n",key); //msg to see which port is used
  }

  else if (argc == 2)
  {
  	port = atoi(argv[1]);
  	gen_random(key, 32);
    int i=0;
    for(i =0; i < 16; i++)
    {
      key2[i] = key[i];
    }
    key2[16] = '\0';
    printf("\nUsing port: %i\n",port); //msg to see which port is used
    printf("\nAES128 Secret key: %s\n",key2); //msg to see which key is generated
  	printf("\nAES256 Secret key: %s\n",key); //msg to see which key is generated
  }

	int con;

	if((con = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) //if there are any problems with the connection socket
	{
		perror("\nFail to create socket");
		exit(1);
	}
	bzero(&server, sizeof(server));//place zero-valued bytes in the area pointed to
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;  //Possible reason for 0.0.0.0
	server.sin_port = htons(port);
	bzero(&(server.sin_zero), 8);//place zero-valued bytes in the area pointed to

	if(bind(con,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)//assigns the address specified by addr to the socket descriptor sockfd
	{
		perror("\nFailed to bind");
		exit(1);
	}
	listen(con , 1);//listen to the socket
	if (con < 0) {
	      perror("\nERROR on accept");
	      exit(1);
	}
	//Connection established, enter infinite while loop to take commands from client
	while(1)
	{
    int clientCon;
		while((clientCon = accept(con,0,0))==SOCKET_ERROR); //accept the connection
		printf("\nNew Client Connected !!!\n");
		bytes = recv(clientCon,buffer,1024,0);
		if(bytes == -1) break;
		buffer[bytes] = '\0';
    //Initial protocol
    //Using no cipher
    if(strncmp(buffer, "Use none", 8)==0){
      cipher = 0;
      strcat(cpUsing , "none");
    }
    //Using AES128-CBC
    if((strncmp(buffer, "Use aes128", 10)==0))
    {
      cipher = 1;
      int i=0;
      char initVector[1024] = "";
      for(i =0; i < strlen(buffer); i++)
      {
        cmr[i] = buffer[4+i];
      }
      char *x;
      //last occurence
      x = strrchr(cmr, ' ');
      int p = 0;
      for(p=0; p < strlen(cmr); p++)
      {
        initVector[p] = cmr[x-cmr+1+p];
      }
      initVector[p] = '\0';	//strlen(fileName)
      strcpy(iv, initVector);
      iv[strlen(iv)] = '\0';
      strcat(cpUsing , "AES128");
      unsigned char temp[SHA_DIGEST_LENGTH];
      char buf[SHA_DIGEST_LENGTH*2];
      memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
      memset(temp, 0x0, SHA_DIGEST_LENGTH);
      SHA1(shared, strlen(shared), temp);
      for (i=0; i < SHA_DIGEST_LENGTH; i++) {
          sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
      }
      keyProtocol = buf;
      printf("%s\n", buf);
      printf("%s\n", keyProtocol);
    }
    //Using AES256-CBC
    if((strncmp(buffer, "Use aes256", 10)==0))
    {
      cipher = 2;
      int i=0;
      char initVector[1024] = "";
      for(i =0; i < strlen(buffer); i++)
      {
        cmr[i] = buffer[4+i];
      }
      char *x;
      //last occurence
      x = strrchr(cmr, ' ');
      int p = 0;
      for(p=0; p < strlen(cmr); p++)
      {
        initVector[p] = cmr[x-cmr+1+p];
      }
      initVector[p] = '\0';	//strlen(fileName)
      strcpy(iv, initVector);
      iv[strlen(iv)] = '\0';
      strcat(cpUsing , "AES256");
      unsigned char temp[SHA_DIGEST_LENGTH];
      char buf[SHA_DIGEST_LENGTH*2];
      memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
      memset(temp, 0x0, SHA_DIGEST_LENGTH);
      SHA1(shared, strlen(shared), temp);
      for (i=0; i < SHA_DIGEST_LENGTH; i++) {
          sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
      }
      keyProtocol = buf;
      printf("%s\n", buf);
      printf("%s\n", keyProtocol);
    }
    printf("\nClient: %s crypto: %s iv: %s\n", inet_ntoa(server.sin_addr), cpUsing, iv);//gotta fix 	this?????? 0.0.0.0
    memset(&buffer,0,1024);
    memset(&cpUsing, 0, 1024);
    unsigned char *encryptedProtocol;
    if(cipher != 0)
    {
      if(cipher == 1){
        if(strlen(key2) < 16){
          while(strlen(key2) < 16){
            strcat(key2, key2);
          }
        }
        key2[16] = '\0';
        int protocol_len = encrypt128(keyProtocol, strlen(keyProtocol), key2, iv, encryptedProtocol);
        if( send(clientCon, encryptedProtocol , protocol_len , 0) < 0)
        {
          puts("\nSend failed");
          return 1;
        }
      }
      if(cipher == 2){
        if(strlen(key) < 32){
          while(strlen(key) < 32){
            strcat(key, key);
          }
        }
        key[32] = '\0';
        int protocol_len = encrypt256(keyProtocol, strlen(keyProtocol), key, iv, encryptedProtocol);
        if( send(clientCon, encryptedProtocol , protocol_len , 0) < 0)
        {
          puts("\nSend failed");
          return 1;
        }
      }
    }

    bytes = recv(clientCon,buffer,1024,0);
		if(bytes == -1) break;
		//memset(&buffer,0,1024);
    //No cipher (encryption) to be used
    if((cipher == 0) && (strlen(buffer) > 0))
    {
      //Write to a file the contents the server has recieved.
      if((strncmp(buffer, "write ",6) == 0))
      {
        char messageRecieved[1024]="";
        char message[1024] = "";
        char fileName[1024]= "";
        int i=0;
        for(i =0; i < strlen(buffer); i++)
        {
          messageRecieved[i] = buffer[6+i];
        }
        char *s;
        //last occurence
        s = strrchr(messageRecieved, ' ');
        strncpy(message, messageRecieved, s-messageRecieved);
        message[strlen(message)] = '\0';
        int j = 0;
        for(j=0; j < strlen(messageRecieved); j++){
          fileName[j] = messageRecieved[s-messageRecieved+1+j];
        }
        fileName[j] = '\0';	//strlen(fileName)
        printf("\nCommand: Write %s\n", fileName);
        FILE *fp;
        fp= fopen(fileName, "wb");
        char *base64Buffer;
        base64Buffer = base64Encode(message, strlen(message));
        fprintf(fp, "%s", base64Buffer);
        fclose(fp);
        printf("\nDone\n");
        char reply[10] = "Ok\n";
        termPointer = reply;
        send(clientCon,termPointer,strlen(termPointer),0);
      }// end write to file
      // Read the contents of file and send them to client
      else if((strncmp(buffer, "read ",5) == 0))
      {
        char messageRecieved[1024]="";
        char fileName[1024]= "";
        int i=0;
        for(i =0; i < strlen(buffer); i++)
        {
          messageRecieved[i] = buffer[5+1+i];
        }
        FILE *fp;
        if((fp = fopen(messageRecieved, "rb")) == NULL){
          strcat(sendMessage, "File does not exist.\n");
          termPointer = sendMessage;
          send(con,termPointer,strlen(sendMessage),0);
          memset(&sendMessage, 0, 1024);
          memset(&buffer, 0, 1024);
        }
        fp= fopen(messageRecieved, "rb");
        while(!feof(fp))
        {
          fgets(sendMessage, 1024 , fp);
        }
        fclose(fp);
        printf("\nDone\n");
        termPointer = sendMessage;
        send(clientCon,termPointer,strlen(termPointer),0);
      }// end read from file
      //reset memory
      memset(&buffer,0,1024);
    }//end no cipher (encryption) to be used clause
    //AES128 to be used as encryption algorithm
    else if((cipher == 1) && (strlen(buffer) > 0))
    {
      //stretch key if not 16 characters long
      if(strlen(key2) < 16){
        while(strlen(key2) < 16){
          strcat(key2, key2);
        }
      }
      key2[16] = '\0';
      unsigned char decryptedtext[1024] = "";
      unsigned char ciphertext[1024] = "";
      //decrypt the incoming contents in the buffer
      int decryptedtext_len = decrypt128(buffer, strlen(buffer), key2, iv, decryptedtext);
      decryptedtext[decryptedtext_len] = '\0';
      memset(&buffer,0,1024);
      //Write to a file the contents the server has recieved.
      if((strncmp(decryptedtext, "write ",6) == 0))
      {
        char messageRecieved[1024]="";
        char message[1024] = "";
        char fileName[1024]= "";
        int i=0;
        for(i =0; i < strlen(decryptedtext); i++)
        {
          messageRecieved[i] = decryptedtext[6+i];
        }
        char *s;
        //last occurence
        s = strrchr(messageRecieved, ' ');
        strncpy(message, messageRecieved, s-messageRecieved);
        message[strlen(message)] = '\0';
        int j = 0;
        for(j=0; j < strlen(messageRecieved); j++)
        {
          fileName[j] = messageRecieved[s-messageRecieved+1+j];
        }
        fileName[j] = '\0';	//strlen(fileName)
        printf("\nCommand: Write %s\n", fileName);
        FILE *fp;
        fp= fopen(fileName, "wb+");
        char *base64Buffer;
        base64Buffer = base64Encode(message, strlen(message));
        fprintf(fp, "%s", base64Buffer);
        //fwrite(base64Buffer,sizeof(base64Buffer),1,fp);
        fclose(fp);
        printf("\nDone\n");
        memset(&fileName, 0, 1024);
        char reply[10] = "Ok\n";
        int ciphertext_len = encrypt128(reply, strlen(reply), key2, iv, ciphertext);
        termPointer = ciphertext;
        send(clientCon,termPointer, ciphertext_len,0);
        memset(&decryptedtext,0,1024);
        memset(&ciphertext,0,1024);
      }// end write to file
      //read from file
      else if((strncmp(decryptedtext, "read ",5) == 0))
      {
        char messageRecieved[1024]="";
        char fileName[1024]= "";
        int i=0;
        for(i =0; i < decryptedtext_len; i++)
        {
          messageRecieved[i] = decryptedtext[5+1+i];
        }
        FILE *fp;
        fp = fopen(messageRecieved, "rb+");
        if(fp == NULL){
          strcat(sendMessage, "File does not exist.\n");
          int ciphertext_len = encrypt128(sendMessage, strlen(sendMessage), key2, iv, ciphertext);
          termPointer = ciphertext;
          send(con,termPointer,ciphertext_len,0);
          memset(&sendMessage, 0, 1024);
          memset(&decryptedtext,0,1024);
          memset(&ciphertext,0,1024);
        }
        else
        {
          while(!feof(fp))
          {
            fgets(sendMessage, 1024 , fp);
          }
          fclose(fp);
          printf("\nDone\n");
          int ciphertext_len = encrypt128(sendMessage, strlen(sendMessage), key2, iv, ciphertext);
          termPointer = ciphertext;
          send(clientCon,termPointer,ciphertext_len,0);
          memset(&sendMessage, 0, 1024);
          memset(&decryptedtext,0,1024);
          memset(&ciphertext,0,1024);
        }
      }// end read from file
        memset(&buffer,0,1024);
    }//end of AES128-CBC
    //AES256 to be used as encryption algorithm
		else if((cipher == 2) && (strlen(buffer) > 0))
    {
      //stretch key if not 32 characters long
      if(strlen(key) < 32){
        while(strlen(key) < 32){
          strcat(key, key);
        }
      }
      key[32] = '\0';
			unsigned char decryptedtext[1024] = "";
			unsigned char ciphertext[1024] = "";
      //decrypt the incoming contents in the buffer
      int decryptedtext_len = decrypt256(buffer, strlen(buffer), key, iv, decryptedtext);
  		decryptedtext[decryptedtext_len] = '\0';
  		memset(&buffer,0,1024);
      //Write to a file the contents the server has recieved.
  		if((strncmp(decryptedtext, "write ",6) == 0))
      {
  			char messageRecieved[1024]="";
  			char message[1024] = "";
  			char fileName[1024]= "";
  			int i=0;
  			for(i =0; i < strlen(decryptedtext); i++)
        {
  				messageRecieved[i] = decryptedtext[6+i];
  			}
  			char *s;
  			//last occurence
  			s = strrchr(messageRecieved, ' ');
  			strncpy(message, messageRecieved, s-messageRecieved);
        message[strlen(message)] = '\0';
        int j = 0;
  			for(j=0; j < strlen(messageRecieved); j++)
        {
  				fileName[j] = messageRecieved[s-messageRecieved+1+j];
  			}
  			fileName[j] = '\0';	//strlen(fileName)
  			printf("\nCommand: Write %s\n", fileName);
  			FILE *fp;
  			fp= fopen(fileName, "wb");
        fprintf(fp, "%s", message);
  			fclose(fp);
  			printf("\nDone\n");
        memset(&fileName, 0, 1024);
  			char reply[10] = "Ok\n";
  			int ciphertext_len = encrypt256(reply, strlen(reply), key, iv, ciphertext);
  			termPointer = ciphertext;
  			send(clientCon,termPointer, ciphertext_len,0);
  			memset(&decryptedtext,0,1024);
  			memset(&ciphertext,0,1024);
  		}// end write to file
      //read from file
  		else if((strncmp(decryptedtext, "read ",5) == 0))
      {
        //if(decryptedtext[strlen (decryptedtext)-1] == '\n') decryptedtext[strlen (decryptedtext)-1] = '\0';
  			char messageRecieved[1024]="";
  			char fileName[1024]= "";
  			int i=0;
  			for(i =0; i < decryptedtext_len; i++)
        {
  				messageRecieved[i] = decryptedtext[5+1+i];
  			}
  			FILE *fp;
  			fp= fopen(messageRecieved, "rb");
  			while(!feof(fp))
        {
  				fgets(sendMessage, 1024 , fp);
  			}
  			fclose(fp);
  			printf("\nDone\n");
  			int ciphertext_len = encrypt256(sendMessage, strlen(sendMessage), key, iv, ciphertext);
  			termPointer = ciphertext;
  			send(clientCon,termPointer,ciphertext_len,0);
  			memset(&decryptedtext,0,1024);
  			memset(&ciphertext,0,1024);
  		}// end read from file
  			memset(&buffer,0,1024);
		}/*end of aes256 cipher to be used*/
    /* Clean up */
    EVP_cleanup();
    ERR_free_strings();
    close(clientCon);
	}//end while
  close(con);
  return 0;
}
