#include<stdio.h> //printf
#include<stdlib.h>
#include<string.h>    //strlen
#include<fcntl.h> // for open
#include<unistd.h> // for close
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<openssl/conf.h>
#include<openssl/evp.h>
#include<openssl/err.h>
#include<openssl/sha.h>
#include"myHeader.h"

int main (int argc , char *argv[])
{
    int sock, numofBytes;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000], printReply[2000], clearMessage[2000];
    char constructedMessage[5000];
    char command[50], filename[1000], port[65535], host[1000], cipher[1000], key[1000], hostAndPort[66355];
    unsigned char iv[1024] = "";
    unsigned char *shared = "sharedSecret";
    unsigned char* decryptSecret;
    gen_random(iv, 16);
    unsigned char* calculatedHash;

    /* Initialise the library */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    unsigned char temp[SHA_DIGEST_LENGTH];
    char buf[SHA_DIGEST_LENGTH*2];
    memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
    memset(temp, 0x0, SHA_DIGEST_LENGTH);
    SHA1(shared, strlen(shared), temp);
    int i=0;
    for (i=0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
    }
    calculatedHash = buf;
    printf("Buffer: %s\n", buf);
    printf("Hash: %s\n", calculatedHash);

    char *s;
    s = strchr(argv[3], ':');

    if((argc > 4) && (argv[3] != NULL) && (s!=NULL))
    {

  		strncpy(host, argv[3], s-argv[3]);
  		int i =0;
  		strcpy(hostAndPort, argv[3]);
  		for(i = 0; i < strlen(argv[3]); i++)
  		{
  			port[i] = hostAndPort[s-argv[3]+1+i];
  		}
    }

    if( argc > 6 ) {
     	printf("\n[+] Too many arguments supplied. Please follow this pattern: $ Client command filename hostname:port cipher [key]\n");
  		return 0;
    }

    else if(argc < 5)
    {
  		printf("\nNot enough arguments supplied. Please follow this pattern: $ Client command filename hostname:port cipher [key]\n");
  		return 0;
    }

    else if((argc == 6) && ((strcmp(argv[1], "write") == 0) || (strcmp(argv[1], "read") == 0 )) && ((strcmp(argv[4],"aes128") == 0) || (strcmp(argv[4],"aes256") == 0))) //with key included and cipher with write or read command
    {
  		strcpy(command, argv[1]);
  		strcpy(filename, argv[2]);
  		strcpy(cipher, argv[4]);
  		strcpy(key, argv[5]);
      if(key[strlen (key)-1] == '\n') key[strlen (key)-1] = '\0';
    }

    else if((argc == 6) && !((strcmp(argv[1], "write") == 0) || (strcmp(argv[1], "read") == 0 ))) //with key included and cipher but incorrect command
    {
  		printf("\nIncorrect Command\n");
  		printf("\nCorrect commands are: read or write.\n");
  		return 0;
    }

    else if(!(argc == 6) && ((strcmp(argv[1], "write") == 0) || (strcmp(argv[1], "read") == 0 )) && ((strcmp(argv[4],"aes128") == 0) || (strcmp(argv[4],"aes256") == 0))) //without key but with cipher write or read command
    {
  		printf("\nPlease provide a key\n");
  		return 0;
    }

    else if((argc ==  5) && (strcmp(argv[4],"none") == 0)) //must have localhost:port and cipher == none
    {
  		strcpy(command, argv[1]);
  		strcpy(filename, argv[2]);
  		strcpy(cipher, argv[4]);
    }

    else if((argc ==  6) && (strcmp(argv[4],"none") == 0)) //must have localhost:port and cipher == none but key is also provided
    {
  		strcpy(command, argv[1]);
  		strcpy(filename, argv[2]);
  		strcpy(cipher, argv[4]);
  		strcpy(key, argv[5]);
      if(key[strlen (key)-1] == '\n') key[strlen (key)-1] = '\0';
    }

    else{
  		printf("\nSomething went horribly wrong.");
  		exit(0);
    }

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1)
    {
        printf("\nCould not create socket");
    }
    //puts("Socket created");

    server.sin_addr.s_addr = inet_addr(host);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("\nConnect failed. Error.");
        return 1;
    }

    //send a message abour cipher and iv to be used in clear
    strcat(clearMessage, "Use ");
    strcat(clearMessage, cipher);
    strcat(clearMessage, " ");
    strcat(clearMessage, iv);

    //send
    if(send(sock, clearMessage, strlen(clearMessage) , 0) < 0)
    {
  		puts("\nSend failed");
  		return 1;
    }

    if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
    {
      puts("\nrecv failed");
      exit(0);
    }
    if(strcmp(cipher, "aes128") == 0){
      if(strlen(key) < 16){
        while(strlen(key) < 16){
          strcat(key, key);
        }
      }
      key[16] = '\0';
      int decryptedtext1_len = decrypt128(server_reply, numofBytes, key, iv,decryptSecret);
      if(strcmp(decryptSecret, calculatedHash) != 0){
        puts("\nWrong Key");
        exit(0);
      }

    }
    if(strcmp(cipher, "aes256") == 0){
      if(strlen(key) < 32){
        while(strlen(key) < 32){
          strcat(key, key);
        }
      }
      key[32] = '\0';
      int decryptedtext1_len = decrypt256(server_reply, numofBytes, key, iv,decryptSecret);
      if(strcmp(decryptSecret, calculatedHash) != 0){
        puts("\nWrong Key");
        exit(0);
      }

    }
    //distinguish between these keys and put in one big if condition.
	//clear server response
    memset(&server_reply, 0, sizeof server_reply);

    if((strcmp(command, "write") == 0) && (strcmp(cipher, "none") == 0))
    {
      memset(constructedMessage,0, sizeof constructedMessage);
      memset(message,0, sizeof message);
      printf("\nEnter message: ");
      fgets(message, sizeof message, stdin);
      strcat(constructedMessage, "write ");
      strcat(constructedMessage, message);
      if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
      strcat(constructedMessage, " ");
      strcat(constructedMessage, filename);
        //Send some data
      if( send(sock , constructedMessage , strlen(constructedMessage) , 0) < 0)
      {
        puts("\nSend failed");
        return 1;
      }
      //Receive a reply from the server
      if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
      {
        puts("\nrecv failed");
        exit(0);
      }
      server_reply[numofBytes] = '\0';
      unsigned char *binaryBuffer;
      size_t fileLength = base64Decode((char*)server_reply, strlen(server_reply), &binaryBuffer);
      memset(&printReply, 0, sizeof printReply);
      strncpy(printReply, server_reply, sizeof server_reply);
      //strncpy(printReply, binaryBuffer, sizeof binaryBuffer);
      //memcpy(printReply, &binaryBuffer, strlen(binaryBuffer));
      printReply[strlen(printReply)] = '\0';
    }

    else if((strcmp(command, "read") == 0) && (strcmp(cipher, "none") == 0))
    {
      memset(constructedMessage,0, sizeof constructedMessage);
      strcat(constructedMessage, "read ");
      if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
      strcat(constructedMessage, " ");
      strcat(constructedMessage, filename);
      //Send some data
      if( send(sock , constructedMessage , strlen(constructedMessage) , 0) < 0)
      {
        puts("\nSend failed");
        return 1;
      }
      //Receive a reply from the server
      if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
      {
        puts("\nrecv failed");
        exit(0);
      }
      server_reply[numofBytes] = '\0';
      memset(&printReply, 0, sizeof printReply);
      strncpy(printReply, server_reply, sizeof server_reply);
      printReply[strlen(printReply)] = '\0';
    }

    else if((strcmp(command, "write") == 0) && (strcmp(cipher, "aes128") == 0))
    {
      //stretch key if not 32 characters long
      if(strlen(key) < 16){
        while(strlen(key) < 16){
          strcat(key, key);
        }
      }
      key[16] = '\0';
      //unsigned char *key = (unsigned char *)"0123456789012345";   //<------fixed key being used for the time being
      /* A 128 bit IV */
  		memset(constructedMessage,0, sizeof constructedMessage);
  		memset(message,0, sizeof message);
  		printf("\nEnter message: ");
  		fgets(message, sizeof message, stdin);
      if(message[strlen (message)-1] == '\n') message[strlen (message)-1] = '\0';
  		strcat(constructedMessage, "write ");
  		strcat(constructedMessage, message);
  		//if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
  		strcat(constructedMessage, " ");
  		strcat(constructedMessage, filename);
  		unsigned char ciphertext[1024];
  		int ciphertext_len = encrypt128(constructedMessage, strlen ((char *)constructedMessage), key, iv, ciphertext);
  		//Send some data
  		if(send(sock , ciphertext , strlen(ciphertext) , 0) < 0)
  		{
  			puts("\nSend failed");
  			return 1;
  		}
  		//Receive a reply from the server
  		if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
  		{
  			puts("\nrecv failed");
  			exit(0);
  		}
  		server_reply[numofBytes] = '\0';
  		unsigned char decryptedtext[1024] = "";
  		int decryptedtext_len = decrypt128(server_reply, numofBytes, key, iv,decryptedtext);
  		memset(&printReply, 0, sizeof printReply);
  		strncpy(printReply, decryptedtext, decryptedtext_len);
      printReply[strlen(printReply)] = '\0';
    }

    else if((strcmp(command, "read") == 0) && (strcmp(cipher, "aes128") == 0))
    {
      //stretch key if not 32 characters long
      if(strlen(key) < 16){
        while(strlen(key) < 16){
          strcat(key, key);
        }
      }
      key[16] = '\0';
      printf("iv: %s\n", iv);
      printf("Key: %s\n",key);
      //unsigned char *key = (unsigned char *)"0123456789012345"; //<------fixed key being used for the time being
      /* A 128 bit IV */
  		memset(constructedMessage,0, sizeof constructedMessage);
  		strcat(constructedMessage, "read ");
  		//if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
  		strcat(constructedMessage, " ");
  		strcat(constructedMessage, filename);
  		unsigned char ciphertext[1024];
  		int ciphertext_len = encrypt128(constructedMessage, strlen ((char *)constructedMessage), key, iv, ciphertext);
  		//Send some data
  		if( send(sock , ciphertext , ciphertext_len , 0) < 0)
  		{
  			puts("\nSend failed");
  			return 1;
  		}
  		//Receive a reply from the server
  		if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
  		{
  			puts("\nrecv failed");
  			exit(0);
  		}
  		server_reply[numofBytes] = '\0';
  		unsigned char decryptedtext[1024];
      int decryptedtext_len = decrypt128(server_reply, numofBytes, key, iv,decryptedtext);
  		memset(&printReply, 0, sizeof printReply);
  		strncpy(printReply, decryptedtext, decryptedtext_len);
      printReply[strlen(printReply)] = '\0';
    }

    else if((strcmp(command, "write") == 0) && (strcmp(cipher, "aes256") == 0))
    {
      //unsigned char *key = (unsigned char *)"01234567890123456789012345678901";  //<------fixed key being used for the time being
      /* A 128 bit IV */
  		memset(constructedMessage,0, sizeof constructedMessage);
  		memset(message,0, sizeof message);
  		printf("\nEnter message: ");
  		fgets(message, sizeof message, stdin);
  		strcat(constructedMessage, "write ");
  		strcat(constructedMessage, message);
  		if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
  		strcat(constructedMessage, " ");
  		strcat(constructedMessage, filename);
  		unsigned char ciphertext[1024];
  		int ciphertext_len = encrypt256(constructedMessage, strlen ((char *)constructedMessage), key, iv, ciphertext);
      //Send some data
  		if(send(sock , ciphertext , ciphertext_len , 0) < 0)
  		{
  			puts("\nSend failed");
  			return 1;
  		}
  		//Receive a reply from the server
  		if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
  		{
  			puts("\nrecv failed");
  			exit(0);
  		}
  		server_reply[numofBytes] = '\0';
  		unsigned char decryptedtext[1024] = "";
  		int decryptedtext_len = decrypt256(server_reply, numofBytes, key, iv,decryptedtext);
  		memset(&printReply, 0, sizeof printReply);
  		strncpy(printReply, decryptedtext, decryptedtext_len);
      printReply[strlen(printReply)] = '\0';
    }

    else if((strcmp(command, "read") == 0) && (strcmp(cipher, "aes256") == 0))
    {
      //unsigned char *key = (unsigned char *)"01234567890123456789012345678901";  //<------fixed key being used for the time being
      /* A 128 bit IV */
  		memset(constructedMessage,0, sizeof constructedMessage);
  		strcat(constructedMessage, "read ");
  		if(constructedMessage[strlen (constructedMessage)-1] == '\n') constructedMessage[strlen (constructedMessage)-1] = '\0';
  		strcat(constructedMessage, " ");
  		strcat(constructedMessage, filename);
  		unsigned char ciphertext[1024];
  		int ciphertext_len = encrypt256(constructedMessage, strlen ((char *)constructedMessage), key, iv, ciphertext);
  		//Send some data
  		if( send(sock , ciphertext , ciphertext_len , 0) < 0)
  		{
  			puts("\nSend failed");
  			return 1;
  		}
  		//Receive a reply from the server
  		if( (numofBytes = recv(sock , server_reply , 5000 , 0)) == -1)
  		{
  			puts("\nrecv failed");
  			exit(0);
  		}
  		server_reply[numofBytes] = '\0';
  		unsigned char decryptedtext[1024] = "";
  		int decryptedtext_len = decrypt256(server_reply, numofBytes, key, iv,decryptedtext);
  		memset(&printReply, 0, sizeof printReply);
  		strncpy(printReply, decryptedtext, decryptedtext_len);
      printReply[strlen(printReply)] = '\0';
    }


	printf("%s\n", printReply);
	memset(&printReply,0, sizeof printReply);
	memset(&server_reply,0, sizeof server_reply);
	close(sock);
  /* Clean up */
  EVP_cleanup();
  ERR_free_strings();
	return 0;
}
