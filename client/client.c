#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int sockfd;
struct sockaddr_in addr;
int port_listenfd,port_connfd;
struct sockaddr_in port_addr;
int pasv_sockfd;
struct sockaddr_in pasv_addr;
char sentence[8192];
int port_or_pasv = 0;
int len;
int p;
int portFlag = 0;
int pasvFlag = 0;
int checkReturn = 0;

int argPort = 0;
char argIP[100];
char argPortCh[10];
char curPath[100];
int argIndex = 0;
int apcIndex = 0;
int apIndex = 0;

int Client_PORT(void) {
	int j;
	int temp = 0;
	int m;
	int i = 5;
	int k = 0;
	int c = 0;
	int NAindex = 0;
	int NPindex = 0;
	int ErrorNum = 0;
	int n = 0;
	char checkAddr[8192];
	char New_Addr[8192];
	char New_Port[8192];
	int port_Num = 0;

	if(strncmp(sentence,"PORT ",5) != 0 || (strlen(sentence) > 5 && sentence[5] == '\n'))
	{
		m = write(sockfd,sentence,strlen(sentence));
		if (m < 0) {
			printf("Error Send PORT Response");
			return 1;
 		}

		memset(sentence,0,8192);
		m = read(sockfd, sentence, 8192);
		if (m < 0) {
			printf("Error Read PORT Response");
			return 1;
 		}
		printf("FROM SERVER: %s\n",sentence);
		return 0;
	}
	/* create new transport connection */

	for(k=0;k<6;k++){
		while(sentence[i] != ',' && sentence[i] != '\0' && sentence[i] != '\n' && sentence[i] != '\r'){
			if(j>3){
				ErrorNum = 1;
				break;
			}

			if(sentence[i] >= '0' && sentence[i] <= '9'){
				checkAddr[j] = sentence[i];
			}
			else{
				ErrorNum = 1;
				break;
			}

			i++;
			j++;
		}
		if(sentence[i+1] == ',')
			ErrorNum = 1;
		if(k != 5 && (sentence[i] == '\n' || sentence[i] == '\0' || sentence[i] == '\r'))
			ErrorNum = 1;
		if(ErrorNum == 1){
			break;
		}

		checkAddr[j] == '\0';
		n = atoi(checkAddr);

		if(n < 0 || n > 255){
			ErrorNum = 1;
			break;
		}

		for(c=0;c<j;c++){
			if(k < 4){
				New_Addr[NAindex] = checkAddr[c];
				NAindex++;
			}
			else {
				New_Port[NPindex] = checkAddr[c];
				NPindex++;
			}
						
		}
				
		if(k < 3){
			New_Addr[NAindex] = ',';
			NAindex++;
		}
		else if(k == 3){
			New_Addr[NAindex] = '\0';
		}
		else if(k == 4){
			New_Port[NPindex] = ',';
			port_Num = n * 256;
			NPindex++;
		}
		else {
			New_Port[NPindex] = '\0';
			port_Num += n;
		}

		for(c=0;c<8192;c++) {
			checkAddr[c] = '\0';
		}

		j=0;
		i++;
	}
	if(ErrorNum == 1 || k != 6 || sentence[i] != '\0'){
		m = write(sockfd,sentence,strlen(sentence));
		if (m < 0) {
			printf("Error Send PORT Response");
			return 1;
 		}

		memset(sentence,0,8192);
		int m = read(sockfd, sentence, 8192);
		if (m < 0) {
			printf("Error Send PORT Response");
			return 1;
 		}

		printf("FROM SERVER: %s\n",sentence);
		return 0;
	}

	if(port_or_pasv == 1) {
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
	}

	if ((port_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}


	memset(&port_addr, 0, sizeof(port_addr));
	port_addr.sin_family = AF_INET;
	port_addr.sin_port = htons(port_Num);
	port_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(port_listenfd, (struct sockaddr*)&port_addr, sizeof(port_addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		close(port_listenfd);
		return 1;
	}

	if (listen(port_listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		close(port_listenfd);
		return 1;
	}

	n = write(sockfd, sentence, strlen(sentence));
	if (n < 0) {
		printf("Error Login");
		return 1;
 	}

	memset(sentence,0,8192);
	n = read(sockfd, sentence, 8192);
	if (n < 0) {
		printf("Error Login");
		return 1;
 	}

	printf("FROM SERVER: %s\n",sentence);

	if(strncmp(sentence,"200",3) != 0) {
		close(port_listenfd);
		portFlag = 0;
		port_or_pasv = 0;
		return 0;
	}

	portFlag = 1;
	port_or_pasv = 1;
	return 0;
}

int Client_PASV(void) {
	char New_Addr[100];
	char New_Port[100];
	int j = 0;
	int dotCount = 0;
	int NAIndex = 0;
	int NPIndex = 0;
	int newPort = 0;
	int temp = 0;
	int k = 0;
	int m = 0;

	if(port_or_pasv == 1) {
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
	}

	while(sentence[j] != '(') {
		j++;
	}
	j++;

	while(1) {
		if(sentence[j] == ',') {
			dotCount++;
			if(dotCount == 4) {
				j++;
				break;
			}
			New_Addr[NAIndex] = '.';
		}
		else {
			New_Addr[NAIndex] = sentence[j];
		}

		j++;
		NAIndex++;
				
	}
	New_Addr[NAIndex] = '\0';

	while(sentence[j] != ',') {
		New_Port[NPIndex] = sentence[j];
		NPIndex++;
		j++;
	}
	j++;
	New_Port[NPIndex] = '\0';
	temp = atoi(New_Port);
	temp *= 256;
	newPort = temp;

	NPIndex = 0;
			
	for(k = 0;k<100;k++) {
		New_Port[k] = '\0';
	}

	while(sentence[j] != ')') {
		New_Port[NPIndex] = sentence[j];
		NPIndex++;
		j++;
	}
	New_Port[NPIndex] = '\0';
	temp = atoi(New_Port);
	newPort += temp;

	if ((pasv_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&pasv_addr, 0, sizeof(pasv_addr));
	pasv_addr.sin_family = AF_INET;
	pasv_addr.sin_port = htons(newPort);
	if (inet_pton(AF_INET, New_Addr, &pasv_addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		close(pasv_sockfd);
		return 1;
	}

	port_or_pasv = 1;
	pasvFlag = 1;
	return 0;
}

int Client_LIST(void) {
	int i = 0;
	int n = 0;
	int t;
	unsigned char cd[8192];

	n = write(sockfd, sentence, strlen(sentence));
	if (n < 0) {
		printf("Error Login");
		return 1;
 	}

	memset(sentence,0,8192);
	n = read(sockfd,sentence,8192);
	if(n < 0) {
		printf("Error Read BINARY MODE File");
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		port_or_pasv = 0;
		return 1;
	}

	printf("FROM SERVER: %s\n",sentence);

	if(strncmp(sentence,"150",3) != 0) {
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		port_or_pasv = 0;
		return 0;
	}

	if(portFlag == 1) {
		if ((port_connfd = accept(port_listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			port_or_pasv = 0;
			return 1;
		}
	}
				
	while(1)
	{
		memset(cd,0,8192);
		if(portFlag == 1){
			n = read(port_connfd,cd,1);
		}
		else if(pasvFlag == 1) {
			n = read(pasv_sockfd,cd,1);
		}
		if(n < 0) {
                    	printf("Error Read BINARY MODE File");
                    	if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1) {
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			port_or_pasv = 0;
			return 1;
		}
		else if(n == 0 && strlen(cd) == 0) {
			break;
		}
		else {
			printf("%c",cd[0]);
		}
	}
	

	memset(sentence,0,8192);
	n = read(sockfd,sentence,8192);
	if(n < 0) {
		printf("Error Read BINARY MODE File");
		if(portFlag == 1) {
			close(port_connfd);
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		port_or_pasv = 0;
		return 1;
	}

	printf("FROM SERVER: %s\n",sentence);

	if(portFlag == 1) {
		close(port_connfd);
		close(port_listenfd);
		portFlag = 0;
	}
	else if(pasvFlag == 1) {
		close(pasv_sockfd);
		pasvFlag = 0;
	}
	port_or_pasv = 0;
	return 0;
}

int Client_RETR(void) {
	char Filename[100];
	int i = 0;
	int n = 0;
	int t;
	unsigned char cd[10];

	while(1) {
		if(sentence[i] == 'f' && sentence[i+1] == 'o' && sentence[i+2] == 'r'){
			i += 4;
			break;
		}
		else {
			i++;
		}
	}
	while(1) {
		if(sentence[i] == '\0') {
			break;
		}
		else if(sentence[i] == '/') {
			memset(Filename,0,100);
			n = 0;
			i++;
			continue;
		}
		else if(sentence[i] != ' ') {
			Filename[n] = sentence[i];
		}
		else {
			break;
		}
		n++;
		i++;
	}
	Filename[n] = '\0';

	if(portFlag == 1) {
		if ((port_connfd = accept(port_listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			port_or_pasv = 0;
			return 1;
		}
	}

	FILE *fout;
	fout = fopen(Filename,"wb");
	if(fout == NULL)
	{
		printf("file does not exist");
		if(portFlag == 1) {
			close(port_connfd);
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
			}
		port_or_pasv = 0;
		return 1;
	}

	while(1)
	{
		memset(cd,0,10);
		if(portFlag == 1){
			n = read(port_connfd,cd,1);
		}
		else if(pasvFlag == 1) {
			n = read(pasv_sockfd,cd,1);
		}
		if(n < 0) {
                    	printf("Error Read BINARY MODE File");
                    	if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1) {
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			port_or_pasv = 0;
			return 1;
		}
		else if(n == 0 && strlen(cd) == 0) {
			break;
		}
		else {
			fputc(cd[0],fout);
		}
	}
	

	memset(sentence,0,8192);
	n = read(sockfd,sentence,8192);
	if(n < 0) {
		printf("Error Read BINARY MODE File");
		if(portFlag == 1) {
			close(port_connfd);
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		port_or_pasv = 0;
		return 1;
	}

	printf("FROM SERVER: %s\n",sentence);

	fclose(fout);
	if(portFlag == 1) {
		close(port_connfd);
		close(port_listenfd);
		portFlag = 0;
	}
	else if(pasvFlag == 1) {
		close(pasv_sockfd);
		pasvFlag = 0;
	}
	port_or_pasv = 0;
	return 0;
}

int Client_STOR(void) {

	char Filename[100];
	int i;
	for(i=0;i<100;i++)
	{
		if(sentence[i+5] == '\n')
			break;
		Filename[i] = sentence[i+5];
	}
	Filename[i] = '\0';
					
	FILE *fin;
	fin = fopen(Filename,"rb");

	if(fin == NULL)
	{
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		port_or_pasv = 0;
		printf("550 file-does-not-exist\n");
		return 0;
	}

	int n = write(sockfd, sentence, len);
	if (n < 0) {
		printf("Error Login");
		return 1;
 	}

	if( pasvFlag == 1 ) {
		if (connect(pasv_sockfd, (struct sockaddr*)&pasv_addr, sizeof(pasv_addr)) < 0) {
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}	
	}

	memset(sentence,0,8192);
	n = read(sockfd, sentence, 8192);
	if (n < 0) {
		printf("Error Read Response");
		return 1;
	}
	else if(n == 0) {
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		return 0;
	}

	printf("FROM SERVER: %s\n",sentence);

	if(strncmp(sentence,"150",3) != 0) {
		if(portFlag == 1) {
			close(port_listenfd);
			portFlag = 0;
		}
		else if(pasvFlag == 1) {
			close(pasv_sockfd);
			pasvFlag = 0;
		}
		return 0;
	}

	if( portFlag == 1 ) {
		if ((port_connfd = accept(port_listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			port_or_pasv = 0;
			return 1;
		}
	}

	int val;
	unsigned char byte[10];

	while(1)
	{
		val = fgetc(fin);
						
		if(val == EOF) {
			sleep(1);
			if(portFlag == 1) {
				close(port_connfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1) {
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			break;
		}

		byte[0] = (unsigned char)val;
		if(portFlag == 1) {
			n = write(port_connfd, byte, 1);
		}
		else if(pasvFlag == 1) {
			n = write(pasv_sockfd, byte, 1);
		}
		if(n < 0) {
		printf("Error Send STOR Response 3");
                    	if(portFlag == 1) {
				close(port_connfd);
				close(port_listenfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1) {
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			return 1;
                  }
	}

	for(i=0;i<8192;i++)
		sentence[i] = '\0';
	n = read(sockfd, sentence, 8192);
	if (n < 0) {
		printf("Error Read Response");
			return 1;
	}

	printf("FROM SERVER: %s\n",sentence);

	fclose(fin);
	if(portFlag == 1) {
		close(port_listenfd);
		portFlag = 0;
	}
	else if(pasvFlag == 1) {
		pasvFlag = 0;
	}
	port_or_pasv = 0;
	return 0;
}



int main(int argc, char **argv) {

	int retrFlag = 0;
	int storFlag = 0;
	int n;

	if(argc == 5) {
		if(strcmp(argv[1],"-ip") == 0 && strcmp(argv[3],"-port") == 0) {
			while(argv[4][argIndex] >= '0' && argv[4][argIndex] <= '9') {
				argPortCh[apcIndex] = argv[4][argIndex];
				argIndex++;
				apcIndex++;
			}
			argPortCh[apcIndex] = '\0';
			argPort = atoi(argPortCh);

			argIndex = 0;
			while(argv[2][argIndex] != '\0') {
				argIP[apIndex] = argv[2][argIndex];
				apIndex++;
				argIndex++;
			}
			argIP[apIndex] = '\0';
		}
		else { 
			argPort = 21;
			strcpy(argIP,"127.0.0.1");
		}		
	}
	else {
		argPort = 21;
		strcpy(argIP,"127.0.0.1");
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(argPort);
	if (inet_pton(AF_INET, argIP, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(sentence,0,8192);
        n = read(sockfd, sentence, 8192);
	if (n < 0) {
		printf("Error Read Initial Message");
		return 1;
	}
	else if(n == 0) {
		close(sockfd);
		return 0;
	}
        else{
        	printf("FROM SERVER: %s\n", sentence);
	}

	while(1) {
		fgets(sentence, 4096, stdin);
		len = strlen(sentence);

		if(strncmp(sentence,"RETR",4) == 0 || strncmp(sentence,"retr",4) == 0 && port_or_pasv == 1) {
			retrFlag = 1;
			if(pasvFlag == 1) {
				if (connect(pasv_sockfd, (struct sockaddr*)&pasv_addr, sizeof(pasv_addr)) < 0) {
					printf("Error connect(): %s(%d)\n", strerror(errno), errno);
					close(pasv_sockfd);
					return 1;
				}
			}
		}
		else if(strncmp(sentence,"STOR",4) == 0 || strncmp(sentence,"stor",4) == 0 && port_or_pasv == 1) {
			if(Client_STOR() == 1) {
				close(sockfd);
				return 1;
			}
			storFlag = 0;
			continue;
		}
		else if(strncmp(sentence,"PORT",4) == 0 || strncmp(sentence,"port",4) == 0) {
			if(Client_PORT() == 1) {
				close(sockfd);
				return 1;
			}
			continue;
		}
		else if(strncmp(sentence,"LIST",4) == 0 || strncmp(sentence,"list",4) == 0) {
			if(pasvFlag == 1) {
				if (connect(pasv_sockfd, (struct sockaddr*)&pasv_addr, sizeof(pasv_addr)) < 0) {
					printf("Error connect(): %s(%d)\n", strerror(errno), errno);
					close(pasv_sockfd);
					return 1;
				}
			}
			if(Client_LIST() == 1) {
				close(sockfd);
				return 1;
			}
			continue;
		}

		int n = write(sockfd, sentence, len);
		if (n < 0) {
			printf("Error Login");
			return 1;
 		}

		memset(sentence,0,8192);
		n = read(sockfd, sentence, 8192);
		if (n < 0) {
			printf("Error Read Login Message");
			return 1;
		}

		printf("FROM SERVER: %s\n", sentence);



		/*command*/
		if(strncmp(sentence,"221",3) == 0) {
			if(port_or_pasv == 1) {
				if(portFlag == 1) {
					close(port_listenfd);
					portFlag = 0;
				}
				else if(pasvFlag == 1){
					close(pasv_sockfd);
					pasvFlag = 0;
				}
				port_or_pasv = 0;
			}
			close(sockfd);
			return 0;
		}
		else if(strncmp(sentence,"227",3) == 0) {
			if(Client_PASV() == 1) {
				close(sockfd);
				return 1;
			}
		}
		else if(strncmp(sentence,"150",3) == 0) {
			if(retrFlag == 1) {
				if(Client_RETR() == 1) {
					close(sockfd);
					return 1;
				}
				retrFlag = 0;
			}
		}
		else if(strncmp(sentence,"550",3) == 0 || strncmp(sentence,"450",3) == 0
		|| strncmp(sentence,"452",3) == 0 || strncmp(sentence,"553",3) == 0) {
			if(portFlag == 1) {
				close(port_listenfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1){
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			port_or_pasv = 0;
			retrFlag = 0;
			storFlag = 0;
		}
		else if(strncmp(sentence,"200",3) != 0 || strncmp(sentence,"215",3) != 0 ||
			strncmp(sentence,"331",3) != 0 || strncmp(sentence,"230",3) != 0 ||
			strncmp(sentence,"220",3) != 0 || strncmp(sentence,"257",3) != 0 ||
			strncmp(sentence,"250",3) != 0) {
			if(portFlag == 1) {
				close(port_listenfd);
				portFlag = 0;
			}
			else if(pasvFlag == 1){
				close(pasv_sockfd);
				pasvFlag = 0;
			}
			port_or_pasv = 0;
		}
	}
	
	close(sockfd);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////////////
