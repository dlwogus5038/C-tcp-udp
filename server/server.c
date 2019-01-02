#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>

int listenfd, new_connfd[11];
struct sockaddr_in addr;
int port_sockfd[11];
struct sockaddr_in port_addr[11];
int pasv_listenfd[11],pasv_connfd[11];
struct sockaddr_in pasv_addr[11];
int port_or_pasv[11];
int portFlag[11];
int pasvFlag[11];

char sentence[11][8192];
int p;
int len;
int pasv_port;
int stage[11];

int argPort;
char argPath[100];
char argPortCh[10];
char curPath[11][100];
int argIndex;
int apcIndex;
int apIndex;
int clientNum;

int client_table[11];

pthread_mutex_t mutex;

int Server_USER(int ID) {
	int connfd = new_connfd[ID];
	if(strncmp(sentence[ID],"USER anonymous",14) == 0 && (sentence[ID][14] == '\n' || sentence[ID][14] == '\r' || sentence[ID][14] == '\0')) {
		int n = write(connfd, "331 Guest login ok, send your password.\n", strlen("331 Guest login ok, send your password.\n"));
		if(n < 0) {
                    	printf("Error Login Sentence");
                    	close(connfd);
			return 1;
		}
		return 0;
	}
	else if(strncmp(sentence[ID],"USER",4) == 0) {
		int n = write(connfd, "530 Permission denied\n", strlen("530 Permission denied\n"));
		if(n < 0) {
			printf("Error Login Sentence");
			close(connfd);
			return 1;
		}
	}
	else if(strncmp(sentence[ID],"PASS",4) == 0) {
		int n = write(connfd, "503 Login with USER first.\n", strlen("503 Login with USER first.\n"));
		if(n < 0) {
			printf("Error Login Sentence");
			close(connfd);
			return 1;
		}
	}
	else {
		int n = write(connfd, "530 Please login with USER and PASS.\n", strlen("530 Please login with USER and PASS.\n"));
		if(n < 0) {
			printf("Error Login Sentence");
			close(connfd);
			return 1;
		}
	}
	return 2;
}

int Server_PASS(int ID) {
	int connfd = new_connfd[ID];

	if(strncmp(sentence[ID],"PASS ",5) == 0 && sentence[ID][5] != '\r' && sentence[ID][5] != '\n') {
		int n = write(connfd, "230 Login successful.\n", strlen("230 Login successful.\n"));
		if(n < 0) {
                    	printf("Error Login Sentence");
			return 1;
		}
		return 0;
	}
	else {
		int n = write(connfd, "530 Login incorrect.\n", strlen("530 Login incorrect.\n"));
		if(n < 0) {
			printf("Error Login Sentence");
			close(connfd);
			return 1;
		}
	}
	return 2;
}

int Server_QUIT(int ID) {
	int connfd = new_connfd[ID];
	int n = write(connfd, "221 Goodbye.\n", strlen("221 Goodbye.\n"));
	if(n < 0) {
		printf("Error Login Success Sentence");
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;
		return 1;
	}

	if (port_or_pasv[ID] == 1)
	{
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;
	}
	return 0;
}

int Server_SYST(int ID) {
	int connfd = new_connfd[ID];
	int n = write(connfd, "215 UNIX Type: L8\n", strlen("215 UNIX Type: L8\n"));
	if (n < 0) {
		printf("Error Send SYST Response");
		return 1;
 	}
	return 0;
}

int Server_TYPE(int ID) {
	int connfd = new_connfd[ID];
	if(strncmp(sentence[ID],"TYPE I",6) == 0 || strncmp(sentence[ID],"TYPE i",6) == 0)
	{
		int n = write(connfd, "200 Type set to I.\n", strlen("200 Type set to I.\n"));
		if (n < 0) {
			printf("Error Send TYPE Response");
			return 1;
 		}
	}
	else
	{
		int n = write(connfd, "500 Unrecognised TYPE command.\n", strlen("500 Unrecognised TYPE command.\n"));
		if (n < 0) {
			printf("Error Send TYPE Response");
			return 1;
 		}
	}
	return 0;
}

int Server_PORT(int ID) {
	int connfd = new_connfd[ID];
	if(strncmp(sentence[ID],"PORT ",5) != 0 || (strlen(sentence[ID]) > 5 && (sentence[ID][5] == '\n' || sentence[ID][5] == '\r')))
	{
		int m = write(connfd, "500 Illegal PORT command.\n", strlen("500 Illegal PORT command.\n"));
		if (m < 0) {
			printf("Error Send PORT Response");
			return 1;
 		}
		return 2;
	}

	int i = 5;
	int j = 0;
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

	for(k=0;k<6;k++){
		while(sentence[ID][i] != ',' && sentence[ID][i] != '\0' && sentence[ID][i] != '\n' && sentence[ID][i] != '\r'){
			if(j>3){
				ErrorNum = 1;
				break;
			}

			if(sentence[ID][i] >= '0' && sentence[ID][i] <= '9'){
				checkAddr[j] = sentence[ID][i];
			}
			else{
				ErrorNum = 1;
				break;
			}

			i++;
			j++;
		}
		if(sentence[ID][i+1] == ',')
			ErrorNum = 1;
		if(k != 5 && (sentence[ID][i] == '\n' || sentence[ID][i] == '\0' || sentence[ID][i] == '\r'))
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
	if(ErrorNum == 1 || k != 6 || (sentence[ID][i] != '\0' && sentence[ID][i] != '\n')){
		int m = write(connfd, "500 Illegal PORT command.\n", strlen("500 Illegal PORT command.\n"));
		if (m < 0) {
			printf("Error Send PORT Response");
			return 1;
 		}
		return 2;
	}

	/*create new transport connection*/

	for(k=0;k<strlen(New_Addr);k++) {
		if(New_Addr[k] == ',') {
			New_Addr[k] = '.';
		}
	}
				

	if(port_or_pasv[ID] == 1)
	{
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
	}

	if ((port_sockfd[ID] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&port_addr[ID], 0, sizeof(port_addr[ID]));
	port_addr[ID].sin_family = AF_INET;
	port_addr[ID].sin_port = htons(port_Num);
	if (inet_pton(AF_INET, New_Addr, &port_addr[ID].sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
				
	portFlag[ID] = 1;
	port_or_pasv[ID] = 1;

	int m = write(connfd, "200 PORT command successful\n", strlen("200 PORT command successful\n"));
	if (m < 0) {
		printf("Error Send PORT Response");
		return 1;
 	}
	return 0;		
}

int Server_PASV(int ID) {
	int connfd = new_connfd[ID];
	int m = 0;
	if(port_or_pasv[ID] == 1) {
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
	}

	if ((pasv_listenfd[ID] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&pasv_addr[ID], 0, sizeof(pasv_addr[ID]));
	pasv_addr[ID].sin_family = AF_INET;
	pasv_addr[ID].sin_port = htons(pasv_port);
	pasv_addr[ID].sin_addr.s_addr = htonl(INADDR_ANY);

	while(bind(pasv_listenfd[ID], (struct sockaddr*)&pasv_addr[ID], sizeof(pasv_addr[ID])) == -1) {
		pasv_port++;
		memset(&pasv_addr[ID], 0, sizeof(pasv_addr[ID]));
		pasv_addr[ID].sin_family = AF_INET;
		pasv_addr[ID].sin_port = htons(pasv_port);
		pasv_addr[ID].sin_addr.s_addr = htonl(INADDR_ANY);
	}

	if (listen(pasv_listenfd[ID], 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	struct sockaddr_in sock_addr;
	int sokcAddrLen = sizeof(sock_addr);
	getsockname(connfd,(struct sockaddr *)&sock_addr,&sokcAddrLen);

	char hostIP[100];
	strcpy(hostIP,inet_ntoa(sock_addr.sin_addr));

	int p1 = pasv_port / 256;
	int p2 = pasv_port % 256;
	char p1char[4];
	char p2char[4];
	char temp[4] = "0";
	int i=0;

	pasv_port++;
	if(pasv_port == 65536) {
		pasv_port = 20000;
	}

	for(int k=0;k<strlen(hostIP);k++) {
		if(hostIP[k] == '.') {
			hostIP[k] = ',';
		}
	}

	while(1) {
		if(p1 == 0) {
			break;
		}

		temp[i] = p1%10 + '0';
		p1 = p1/10;

		i++;
	}
	temp[i] = '\0';
	i=0;
	p1char[strlen(temp)] = '\0';
	while(temp[i] != '\0') {
		p1char[strlen(temp)-i-1] = temp[i];
		i++;
	}
				
	i = 0;

	while(1) {
		if(p2 == 0) {
			break;
		}

		temp[i] = p2%10 + '0';
		p2 = p2/10;

		i++;
	}
	temp[i] = '\0';
	i=0;
	p2char[strlen(temp)] = '\0';
	while(temp[i] != '\0') {
		p2char[strlen(temp)-i-1] = temp[i];
		i++;
	}

	port_or_pasv[ID] = 1;
	pasvFlag[ID] = 1;

	char PASV_Re[100] = "227 Entering Passive Mode (";
	int PARlen = 0;
	int k = 0;

	strcat(PASV_Re,hostIP);
	strcat(PASV_Re,",");

	PARlen = strlen(PASV_Re);
	while(1) {
		if(p1char[k] == '\0') {
			break;
		}
		PASV_Re[PARlen] = p1char[k];
		PARlen++;
		k++;
	}
	PASV_Re[PARlen] = ',';
	PARlen++;
	k = 0;
	while(1) {
		if(p2char[k] == '\0') {
			break;
		}
		PASV_Re[PARlen] = p2char[k];
		PARlen++;
		k++;
	}
	PASV_Re[PARlen] = ')';
	PARlen++;
	PASV_Re[PARlen] = '\n';
	PARlen++;
	PASV_Re[PARlen] = '\0';


	m = write(connfd,PASV_Re,strlen(PASV_Re));
	if (m < 0) {
		printf("Error Send PASV Response");
		return 1;
 	}
	return 0;
}

int Server_CWD(int ID) {
	int connfd = new_connfd[ID];
	if(strcmp(sentence[ID],"CWD \n") == 0 || strcmp(sentence[ID],"CWD \r") == 0 || strncmp(sentence[ID],"CWD ",4) != 0) {
					
		int m = write(connfd,"550 Failed to change directory.\n",strlen("550 Failed to change directory.\n"));
		if (m < 0) {
			printf("Error Send CWD Response");
			close(connfd);
			return 1;
	 	}
		return 0;
	}

	char CWDpath[200];
	int CWDindex = 0;
	char CWDpara[200];
	int k = 4;

	memset(CWDpath,0,200);
	memset(CWDpara,0,200);
				
	while(sentence[ID][k] != '\n' && sentence[ID][k] != '\r') {
		CWDpara[k-4] = sentence[ID][k];
		k++;
	}
	CWDpara[k] = '\0';

	if(CWDpara[0] == '/') {
		strcpy(CWDpath,argPath);
	}
	else {
		strcpy(CWDpath,curPath[ID]);
		strcat(CWDpath,"/");
	}
	CWDindex = strlen(CWDpath);

	strcat(CWDpath,CWDpara);

	if(access(CWDpath,0) == 0) {
		strcpy(curPath[ID],CWDpath);
		int m = write(connfd,"250 Okay\n",strlen("250 Okay\n"));
		if (m < 0) {
			printf("Error Send CWD Response");
			close(connfd);
			return 1;
	 	}
	}
	else {
		char ErrorMessage[200] = "550 ";
		strcat(ErrorMessage,CWDpara);
		strcat(ErrorMessage,": No such file or directory\n");
		int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
		if (m < 0) {
			printf("Error Send CWD Response");
			close(connfd);
			return 1;
	 	}
	}
	return 0;
}

int Server_MKD(int ID) {
	int connfd = new_connfd[ID];
	if(strcmp(sentence[ID],"MKD \n") == 0 || strcmp(sentence[ID],"MKD \r") == 0 || strncmp(sentence[ID],"MKD ",4) != 0) {
					
		int m = write(connfd,"550 Create directory operation failed.\n",strlen("550 Create directory operation failed.\n"));
		if (m < 0) {
			printf("Error Send MKD Response");
			close(connfd);
			return 1;
	 	}
		return 0;
	}

	char MKDpath[200];
	int MKDindex = 0;
	char MKDpara[200];
	int k = 4;

	memset(MKDpath,0,200);
	memset(MKDpara,0,200);
				
	while(sentence[ID][k] != '\n' && sentence[ID][k] != '\r') {
		MKDpara[k-4] = sentence[ID][k];
		k++;
	}
	MKDpara[k] = '\0';

	if(MKDpara[0] == '/') {
		strcpy(MKDpath,argPath);
	}
	else {
		strcpy(MKDpath,curPath[ID]);
		strcat(MKDpath,"/");
	}
	MKDindex = strlen(MKDpath);

	strcat(MKDpath,MKDpara);

	char tempPathCh[200];
	char tempDirCh[200];

	memset(tempPathCh,0,200);
	memset(tempDirCh,0,200);

	k=0;
	int t = 0;
	while(MKDpath[k] != '\0') {
		if(MKDpath[k] == '/') {
			strcat(tempPathCh,tempDirCh);
			strcat(tempPathCh,"/");
			memset(tempDirCh,0,200);
			t = 0;
			k++;
			continue;
		}
		else {
			tempDirCh[t] = MKDpath[k];
			t++;
			k++;
		}
	}
	tempPathCh[strlen(tempPathCh)-1] = '\0';

	if(access(tempPathCh,0) == 0) {
		strcat(tempPathCh,"/");
		strcat(tempPathCh,tempDirCh);
		int nResult = mkdir(tempPathCh,0777);
		if(nResult == 0) {
			char SuccessMessage[200] = "257 ";
			if(MKDpara[0] == '/') {
				strcat(SuccessMessage,"\"");
			}
			else {
				strcat(SuccessMessage,"\"/");
			}
			strcat(SuccessMessage, MKDpara);
			strcat(SuccessMessage,"\" created\n");
			int m = write(connfd,SuccessMessage,strlen(SuccessMessage));
			if (m < 0) {
				printf("Error Send MKD Response");
				close(connfd);
				return 1;
		 	}
		}
		else {
			char ErrorMessage[200] = "550 ";
			strcat(ErrorMessage,MKDpara);
			if(errno == EEXIST) {
				strcat(ErrorMessage,": FIle exists\n");
			}
			else {
				strcat(ErrorMessage,": No such file or directory\n");
			}
			int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
			if (m < 0) {
				printf("Error Send MKD Response");
				close(connfd);
				return 1;
		 	}
		}
	}
	else {
		char ErrorMessage[200] = "550 ";
		strcat(ErrorMessage,MKDpara);
		strcat(ErrorMessage,": No such file or directory\n");
		int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
		if (m < 0) {
			printf("Error Send CWD Response");
			close(connfd);
			return 1;
	 	}
	}
	return 0;
}

int Server_RMD(int ID) {
	int connfd = new_connfd[ID];
	if(strcmp(sentence[ID],"RMD \n") == 0 || strcmp(sentence[ID],"RMD \r") == 0 || strncmp(sentence[ID],"RMD ",4) != 0) {		
		int m = write(connfd,"550 Remove directory operation failed.\n",strlen("550 Remove directory operation failed.\n"));
		if (m < 0) {
			printf("Error Send RMD Response");
			close(connfd);
			return 1;
	 	}
		return 0;
	}

	char RMDpath[200];
	int RMDindex = 0;
	char RMDpara[200];
	int k = 4;

	memset(RMDpath,0,200);
	memset(RMDpara,0,200);
				
	while(sentence[ID][k] != '\n' && sentence[ID][k] != '\r') {
		RMDpara[k-4] = sentence[ID][k];
		k++;
	}
	RMDpara[k] = '\0';

	if(RMDpara[0] == '/') {
		strcpy(RMDpath,argPath);
	}
	else {
		strcpy(RMDpath,curPath[ID]);
		strcat(RMDpath,"/");
	}
	RMDindex = strlen(RMDpath);

	strcat(RMDpath,RMDpara);

	char tempPathCh[200];
	char tempDirCh[200];

	memset(tempPathCh,0,200);
	memset(tempDirCh,0,200);

	k=0;
	int t = 0;
	while(RMDpath[k] != '\0') {
		if(RMDpath[k] == '/') {
			strcat(tempPathCh,tempDirCh);
			strcat(tempPathCh,"/");
			memset(tempDirCh,0,200);
			t = 0;
			k++;
			continue;
		}
		else {
			tempDirCh[t] = RMDpath[k];
			t++;
			k++;
		}
	}
	tempPathCh[strlen(tempPathCh)-1] = '\0';

	if(access(tempPathCh,0) == 0) {
		strcat(tempPathCh,"/");
		strcat(tempPathCh,tempDirCh);
		int nResult = rmdir(tempPathCh);
		if(nResult == 0) {
			int m = write(connfd,"250 Okay\n",strlen("250 Okay\n"));
			if (m < 0) {
				printf("Error Send RMD Response");
				close(connfd);
				return 1;
		 	}
		}
		else {
			char ErrorMessage[200] = "550 ";
			strcat(ErrorMessage,RMDpara);
			if(errno == ENOTEMPTY) {
				strcat(ErrorMessage,": Directory not empty\n");
			}
			else {
				strcat(ErrorMessage,": No such file or directory\n");
			}
			int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
			if (m < 0) {
				printf("Error Send RMD Response");
				close(connfd);
				return 1;
		 	}
		}
	}
	else {
		char ErrorMessage[200] = "550 ";
		strcat(ErrorMessage,RMDpara);
		strcat(ErrorMessage,": No such file or directory\n");
		int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
		if (m < 0) {
			printf("Error Send CWD Response");
			close(connfd);
			return 1;
	 	}
	}
	return 0;

}

int Server_RETR(int ID) {
	int connfd = new_connfd[ID];
	if(strncmp(sentence[ID],"RETR ",5) != 0 || strcmp(sentence[ID],"RETR \n") == 0 || strcmp(sentence[ID],"RETR \r") == 0) {
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;
		int w = write(connfd, "550 Failed to open file.\n", strlen("550 Failed to open file.\n"));
		if(w < 0) {
                    	printf("Error Send RETR Response");
                    	close(connfd);
			return 0;
		}
					
		return 0;
	}

	char RETRpath[200];
	memset(RETRpath,0,200);

	if(sentence[ID][5] == '/') {
		strcpy(RETRpath,argPath);
	}
	else {
		strcpy(RETRpath,curPath[ID]);
		strcat(RETRpath,"/");
	}

	char Filename[100];
	int i;
	for(i=0;i<100;i++) {
		if(sentence[ID][i+5] == '\n' || sentence[ID][i+5] == '\r')
			break;
		Filename[i] = sentence[ID][i+5];
	}
	Filename[i] = '\0';

	strcat(RETRpath,Filename);
					
	FILE *fin;
	fin = fopen(RETRpath,"rb");

	int n;
	if(fin == NULL) {
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;
		n = write(connfd, "550 file-does-not-exist\n", strlen("550 file-does-not-exist\n"));
		if(n < 0) {
                    	printf("Error Send RETR Response 1");
                    	if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			return 1;
                }
		return 0;
	}
	if( portFlag[ID] == 1 ) {
		if (connect(port_sockfd[ID], (struct sockaddr*)&port_addr[ID], sizeof(port_addr[ID])) < 0) {
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}	
	}
	else if( pasvFlag[ID] == 1 ) {
		if ((pasv_connfd[ID] = accept(pasv_listenfd[ID], NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
	}

	char RETR_Response[100] = "150 Opening BINARY mode data connection for ";
	int RR_len = strlen(RETR_Response);
	for(i=RR_len;i<RR_len+strlen(Filename);i++) {
		RETR_Response[i] = Filename[i-RR_len];
	}
	RETR_Response[i] = '\n';
	i++;
	RETR_Response[i] = '\0';

	n = write(connfd, RETR_Response, strlen(RETR_Response));
	if(n < 0) {
		printf("Error Send RETR Response 2");
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_connfd[ID]);
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		return 1;
	}

	int val;
	unsigned char byte[10];

	while(1) {
		val = fgetc(fin);
						
		if(val == EOF) {
			sleep(1);
			break;
		}

		byte[0] = (unsigned char)val;
		if(portFlag[ID] == 1) {
			n = write(port_sockfd[ID], byte, 1);
		}
		else if(pasvFlag[ID] == 1) {
			n = write(pasv_connfd[ID], byte, 1);
		}
		if(n < 0) {
			printf("Error Send RETR Response 3");
                    	if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			return 1;
		}
	}


	n = write(connfd, "226 Transfer complete.\n", strlen("226 Transfer complete.\n"));
	if(n < 0) {
		printf("Error Send RETR Response 3");
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_connfd[ID]);
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		return 1;
	}

	fclose(fin);
	if(portFlag[ID] == 1) {
		close(port_sockfd[ID]);
		portFlag[ID] = 0;
	}
	else if(pasvFlag[ID] == 1) {
		close(pasv_connfd[ID]);
		close(pasv_listenfd[ID]);
		pasvFlag[ID] = 0;
	}
	port_or_pasv[ID] = 0;
	return 0;
}

int Server_STOR(int ID) {
	int connfd = new_connfd[ID];
	if(strncmp(sentence[ID],"STOR ",5) != 0 || strcmp(sentence[ID],"STOR \n") == 0 || strcmp(sentence[ID],"STOR \r") == 0) {
		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;

		int w = write(connfd, "550 Failed to download file.\n", strlen("550 Failed to download file.\n"));
		if(w < 0) {
                    	printf("Error Send STOR Response");
                    	close(connfd);
			return 1;
                }
		return 0;
	}

	if(portFlag[ID] == 1) {
		if (connect(port_sockfd[ID], (struct sockaddr*)&port_addr[ID], sizeof(port_addr[ID])) < 0) {
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
	}


	char Filename[100];
	int i = 0;
	int n = 0;

	while(sentence[ID][i] != ' ') {
			i++;
	}
	i++;
	while(1) {
		if(sentence[ID][i] == '/') {
			memset(Filename,0,100);
			n = 0;
			i++;
			continue;
		}
		else if(sentence[ID][i] != '\n' && sentence[ID][i] != '\r') {
			Filename[n] = sentence[ID][i];
		}
		else {
			break;
		}
		n++;
		i++;
	}
	Filename[n] = '\0';

	char storPath[200];
	memset(storPath,0,200);
	strcpy(storPath,curPath[ID]);
	if(sentence[ID][5] != '/') {
		strcat(storPath,"/");
	}
	strcat(storPath,Filename);

	FILE *fout;
	fout = fopen(storPath,"wb");
	if(fout == NULL) {
		printf("Can't Create New File");
		n = write(connfd, "550 Can't Create New File\n", strlen("550 Can't Create New File\n"));
		if(n < 0) {
			printf("Error Read STOR Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}

		if(portFlag[ID] == 1) {
			close(port_sockfd[ID]);
			portFlag[ID] = 0;
		}
		else if(pasvFlag[ID] == 1) {
			close(pasv_listenfd[ID]);
			pasvFlag[ID] = 0;
		}
		port_or_pasv[ID] = 0;
		return 0;
	}

	n = write(connfd, "150 Ok to send data.\n", strlen("150 Ok to send data.\n"));
	if(n < 0) {
		printf("Error Send STOR Response");
                close(connfd);
		return 0;
        }

	if(pasvFlag[ID] == 1) {
		if ((pasv_connfd[ID] = accept(pasv_listenfd[ID], NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
	}

				
	char unsigned cd[10];
	int t;
	while(1)
	{
		for(t = 0;t<10;t++)
			cd[t] = '\0';
		if(portFlag[ID] == 1){
			n = read(port_sockfd[ID],cd,1);
		}
		else if(pasvFlag[ID] == 1) {
			n = read(pasv_connfd[ID],cd,1);
		}
		if(n < 0) {
		        printf("Error Read BINARY MODE File");
		        if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
		else if(n == 0 && strlen(cd) == 0) {
			break;
		}
		else {
			fputc(cd[0],fout);
		}	
	}
				
	n = write(connfd, "226 Transfer complete.\n", strlen("226 Transfer complete.\n"));
	if(n < 0) {
        	printf("Error Send STOR Response");
        	close(connfd);
		return 0;
        }

	fclose(fout);
	if(portFlag[ID] == 1) {
		close(port_sockfd[ID]);
		portFlag[ID] = 0;
	}
	else if(pasvFlag[ID] == 1) {
		close(pasv_connfd[ID]);
		close(pasv_listenfd[ID]);
		pasvFlag[ID] = 0;
	}
	port_or_pasv[ID] = 0;
	return 0;
}

int Server_LIST(int ID) {
	int connfd = new_connfd[ID];
	int m = 0;
	if(strcmp(sentence[ID],"LIST\n") == 0 || strcmp(sentence[ID],"LIST\r") == 0) {
				
	}
	else if(strcmp(sentence[ID],"LIST \n") == 0 || strcmp(sentence[ID],"LIST \r") == 0) {
		memset(sentence[ID],0,8192);
		strcpy(sentence[ID],"LIST\n");
	}
	else if(strcmp(sentence[ID],"LIST \n") == 0 || strcmp(sentence[ID],"LIST \r") == 0 || strncmp(sentence[ID],"LIST ",5) != 0) {

		m = write(connfd,"550 Invalid LIST Command\n",strlen("550 Invalid LIST Command\n"));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
	 	}
		return 0;
	}
				
	char LISTpath[200];
	int LISTindex = 0;
	char LISTpara[200];
	int k = 5;

	memset(LISTpath,0,200);
	memset(LISTpara,0,200);

	if(strcmp(sentence[ID],"LIST\n") == 0 || strcmp(sentence[ID],"LIST\r") == 0) {
		strcpy(LISTpath,curPath[ID]);
	}
	else {
		while(sentence[ID][k] != '\n' && sentence[ID][k] != '\r') {
			LISTpara[k-5] = sentence[ID][k];
			k++;
		}
		LISTpara[k] = '\0';

		if(LISTpara[0] == '/') {
			strcpy(LISTpath,argPath);
		}
		else {
			strcpy(LISTpath,curPath[ID]);
			strcat(LISTpath,"/");
		}
		LISTindex = strlen(LISTpath);
	}
	
	if(portFlag[ID] == 1) {
		if (connect(port_sockfd[ID], (struct sockaddr*)&port_addr[ID], sizeof(port_addr[ID])) < 0) {
			printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
	}
	else if(pasvFlag[ID] == 1) {
		if ((pasv_connfd[ID] = accept(pasv_listenfd[ID], NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
	}

	strcat(LISTpath,LISTpara);

	struct stat fileBuf;
	stat(LISTpath,&fileBuf);

	if(fileBuf.st_mode == 33204) {
		char perChar[11];
		memset(perChar,'-',11);
		
		if(fileBuf.st_mode & S_IFDIR) perChar[0] = 'd';
		if(fileBuf.st_mode & S_IRUSR) perChar[1] = 'r';
		if(fileBuf.st_mode & S_IWUSR) perChar[2] = 'w';
		if(fileBuf.st_mode & S_IXUSR) perChar[3] = 'x';
		if(fileBuf.st_mode & S_IRGRP) perChar[4] = 'r';
		if(fileBuf.st_mode & S_IWGRP) perChar[5] = 'w';
		if(fileBuf.st_mode & S_IXGRP) perChar[6] = 'x';
		if(fileBuf.st_mode & S_IROTH) perChar[7] = 'r';
		if(fileBuf.st_mode & S_IWOTH) perChar[8] = 'w';
		if(fileBuf.st_mode & S_IXOTH) perChar[9] = 'x';
		perChar[10] = '\0';

		char fileAttr[300];
		memset(fileAttr,0,300);
		strcat(fileAttr,perChar);
		strcat(fileAttr,"  ");
		
		char tempChar[100];

		memset(tempChar,0,100);
		snprintf(tempChar,99,"%ld",fileBuf.st_nlink);
		strcat(fileAttr,tempChar);
		strcat(fileAttr,"  ");

		memset(tempChar,0,100);
		snprintf(tempChar,99,"%d",fileBuf.st_uid);
		strcat(fileAttr,tempChar);
		strcat(fileAttr,"  ");

		memset(tempChar,0,100);
		snprintf(tempChar,99,"%d",fileBuf.st_gid);
		strcat(fileAttr,tempChar);
		strcat(fileAttr,"  ");

		memset(tempChar,0,100);
		snprintf(tempChar,99,"%ld",fileBuf.st_blksize);
		strcat(fileAttr,tempChar);
		strcat(fileAttr,"  ");

		strcat(fileAttr,ctime(&fileBuf.st_ctime));
		if(fileAttr[strlen(fileAttr)-1] == '\n'){
			fileAttr[strlen(fileAttr)-1] = '\0';
		} 
		strcat(fileAttr,"  ");

		

		char tempFileName[100];
		int temp = 0;
		int tempIndex = 0;
		memset(tempFileName,0,100);

		while(LISTpath[temp] != '\0'){
			if(LISTpath[temp] == '/') {
				memset(tempFileName,0,100);
				temp++;
				tempIndex = 0;
			}
			else {
				tempFileName[tempIndex] = LISTpath[temp];
				tempIndex++;
				temp++;
			}
		}
		tempFileName[tempIndex] = '\n';
		tempFileName[tempIndex + 1] = '\0';

		strcat(fileAttr,tempFileName);

		m = write(connfd,"150 Here comes the directory listing.\n",strlen("150 Here comes the directory listing.\n"));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}

		if(portFlag[ID] == 1){
			m = write(port_sockfd[ID],fileAttr,strlen(fileAttr));
		}
		else if(pasvFlag[ID] == 1) {
			m = write(pasv_connfd[ID],fileAttr,strlen(fileAttr));
		}
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
	 	}

		sleep(1);

		m = write(connfd,"226 Directory send OK.\n",strlen("226 Directory send OK.\n"));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
		
	}
	else if(access(LISTpath,0) == 0) {
		int depth = 1;
		char filename[256][256];
		int len = 0;
		DIR *d; 
		struct dirent *file; 
		struct stat sb;   
					   
		if(!(d = opendir(LISTpath))) {
			char ErrorMessage[200] = "550 ";
			strcat(ErrorMessage,LISTpara);
			strcat(ErrorMessage,": No such file or directory\n");
			int m = write(connfd,ErrorMessage,strlen(ErrorMessage));
			if (m < 0) {
				printf("Error Send LIST Response");
				if(portFlag[ID] == 1) {
					close(port_sockfd[ID]);
					portFlag[ID] = 0;
				}
				else if(pasvFlag[ID] == 1) {
					close(pasv_connfd[ID]);
					close(pasv_listenfd[ID]);
					pasvFlag[ID] = 0;
				}
				port_or_pasv[ID] = 0;
				return 1;
		 	}
			return 0;
		}
		m = write(connfd,"150 Here comes the directory listing.\n",strlen("150 Here comes the directory listing.\n"));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}

		while((file = readdir(d)) != NULL) {
			if(strncmp(file->d_name, ".", 1) == 0)
				continue;
			strcpy(filename[len++], file->d_name); 
					
		}
		closedir(d);

		char fileList[1024*1024];
		memset(fileList,0,1024*1024);
			
		char perChar[11];
		char fileAttr[300];
		char tempChar[100];
		char tempPath[200];

		for(int k=0;k<len;k++) {

			memset(tempPath,0,200);
			strcat(tempPath,LISTpath);
			strcat(tempPath,"/");
			strcat(tempPath,filename[k]);

			stat(tempPath,&fileBuf);

			memset(perChar,'-',11);
		
			if(fileBuf.st_mode & S_IFDIR) perChar[0] = 'd';
			if(fileBuf.st_mode & S_IRUSR) perChar[1] = 'r';
			if(fileBuf.st_mode & S_IWUSR) perChar[2] = 'w';
			if(fileBuf.st_mode & S_IXUSR) perChar[3] = 'x';
			if(fileBuf.st_mode & S_IRGRP) perChar[4] = 'r';
			if(fileBuf.st_mode & S_IWGRP) perChar[5] = 'w';
			if(fileBuf.st_mode & S_IXGRP) perChar[6] = 'x';
			if(fileBuf.st_mode & S_IROTH) perChar[7] = 'r';
			if(fileBuf.st_mode & S_IWOTH) perChar[8] = 'w';
			if(fileBuf.st_mode & S_IXOTH) perChar[9] = 'x';
			perChar[10] = '\0';

			memset(fileAttr,0,300);
			strcat(fileAttr,perChar);
			strcat(fileAttr,"  ");

			memset(tempChar,0,100);
			snprintf(tempChar,99,"%ld",fileBuf.st_nlink);
			strcat(fileAttr,tempChar);
			strcat(fileAttr,"  ");

			memset(tempChar,0,100);
			snprintf(tempChar,99,"%d",fileBuf.st_uid);
			strcat(fileAttr,tempChar);
			strcat(fileAttr,"  ");

			memset(tempChar,0,100);
			snprintf(tempChar,99,"%d",fileBuf.st_gid);
			strcat(fileAttr,tempChar);
			strcat(fileAttr,"  ");

			memset(tempChar,0,100);
			snprintf(tempChar,99,"%ld",fileBuf.st_blksize);
			strcat(fileAttr,tempChar);
			strcat(fileAttr,"  ");

			strcat(fileAttr,ctime(&fileBuf.st_ctime));
			if(fileAttr[strlen(fileAttr)-1] == '\n'){
				fileAttr[strlen(fileAttr)-1] = '\0';
			} 
			strcat(fileAttr,"  ");


			strcat(fileAttr,filename[k]);
			strcat(fileAttr,"\n");


			strcat(fileList,fileAttr);
		}
		if(portFlag[ID] == 1){
			m = write(port_sockfd[ID],fileList,strlen(fileList));
		}
		else if(pasvFlag[ID] == 1) {
			m = write(pasv_connfd[ID],fileList,strlen(fileList));
		}
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
	 	}

		sleep(1);

		m = write(connfd,"226 Directory send OK.\n",strlen("226 Directory send OK.\n"));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
		}
	}
	else {
		char ErrorMessage[200] = "550 ";
		strcat(ErrorMessage,LISTpara);
		strcat(ErrorMessage,": No such file or directory\n");
		m = write(connfd,ErrorMessage,strlen(ErrorMessage));
		if (m < 0) {
			printf("Error Send LIST Response");
			if(portFlag[ID] == 1) {
				close(port_sockfd[ID]);
				portFlag[ID] = 0;
			}
			else if(pasvFlag[ID] == 1) {
				close(pasv_connfd[ID]);
				close(pasv_listenfd[ID]);
				pasvFlag[ID] = 0;
			}
			port_or_pasv[ID] = 0;
			return 1;
	 	}
	}

	if(portFlag[ID] == 1) {
		close(port_sockfd[ID]);
		portFlag[ID] = 0;
	}
	else if(pasvFlag[ID] == 1) {
		close(pasv_connfd[ID]);
		close(pasv_listenfd[ID]);
		pasvFlag[ID] = 0;
	}
	port_or_pasv[ID] = 0;
	return 0;
}

void * Server_MULTI(void * data) {

	int ID = *(int *)data;
	int connfd = new_connfd[ID];

	/* Read Command */
	int checkResult = 0;
		
	while(1) {
		memset(sentence[ID],0,8192);
		int n = read(connfd, sentence[ID], 8192);
		if (n < 0) {
			break;
		}
		if(strlen(sentence[ID]) >= 4) {
			for(int k=0;k<4;k++){
				if(sentence[ID][k] >= 'a' && sentence[ID][k] <= 'z') {
					sentence[ID][k] -= 32;
				}
			}
		}
		else if(strlen(sentence[ID]) == 3) {
			for(int k=0;k<3;k++){
				if(sentence[ID][k] >= 'a' && sentence[ID][k] <= 'z') {
					sentence[ID][k] -= 32;
				}
			}
		}

		printf("ID : %d >> %s",ID,sentence[ID]);


		if(strcmp(sentence[ID],"QUIT\n") == 0 || strcmp(sentence[ID],"ABOR\n") == 0
		|| strcmp(sentence[ID],"QUIT\r") == 0 || strcmp(sentence[ID],"ABOR\r") == 0) {
			Server_QUIT(ID);
			break;
		}

		if(stage[ID] == 0) {
			checkResult = Server_USER(ID);
			if(checkResult == 1) {
				break;
			}
			else if(checkResult == 2) {
				continue;
			}
			else {
				stage[ID]++;
				continue;
			}
		}
		else if(stage[ID] == 1) {
			checkResult = Server_PASS(ID);
			if(checkResult == 1) {
				break;
			}
			else if(checkResult == 2) {
				continue;
			}
			else {
				stage[ID]++;
				continue;
			}
		}
		else if(stage[ID] == 2) { 
			if(strncmp(sentence[ID],"SYST",4) == 0) {
				checkResult = Server_SYST(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"TYPE",4) == 0) {
				checkResult = Server_TYPE(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"PORT",4) == 0) {
				checkResult = Server_PORT(ID);
				if(checkResult == 1) {
					break;
				}
				else if(checkResult == 2) {
					continue;
				}
				else {
					stage[ID]++;
					continue;
				}
			}
			else if(strncmp(sentence[ID],"PASV",4) == 0) {
				checkResult = Server_PASV(ID);
				if(checkResult == 1) {
					break;
				}
				else if(checkResult == 2) {
					continue;
				}
				else {
					stage[ID]++;
					continue;
				}
			}
			else if(strncmp(sentence[ID],"CWD",3) == 0) {
				checkResult = Server_CWD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"MKD",3) == 0) {
				checkResult = Server_MKD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"RMD",3) == 0) {
				checkResult = Server_RMD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"RETR",4) == 0 || strncmp(sentence[ID],"STOR",4) == 0 
			|| strncmp(sentence[ID],"LIST",4) == 0){
				int m = write(connfd,"425 No TCP Connection Was Established\n",strlen("425 No TCP Connection Was Established\n"));
				if (m < 0) {
					printf("Error Send RETR/STOR Response");
					break;
	 			}
				continue;
			}
			else {
				int m = write(connfd,"500 Unknown command. 1\n",strlen("500 Unknown command. 1\n"));
				if (m < 0) {
					printf("Error Send RETR/STOR Response");
					break;
	 			}
				continue;
			}
		}
		else if(stage[ID] == 3) {
			if(strncmp(sentence[ID],"PORT",4) == 0) {
				checkResult = Server_PORT(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"PASV",4) == 0) {
				checkResult = Server_PASV(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"RETR",4) == 0) {
				checkResult = Server_RETR(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					stage[ID]--;
					continue;
				}
			}
			else if(strncmp(sentence[ID],"STOR",4) == 0) {
				checkResult = Server_STOR(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					stage[ID]--;
					continue;
				}
			}
			else if(strncmp(sentence[ID],"LIST",4) == 0) {
				checkResult = Server_LIST(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					stage[ID]--;
					continue;
				}
			}
			else if(strncmp(sentence[ID],"SYST",4) == 0) {
				checkResult = Server_SYST(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"TYPE",4) == 0) {
				checkResult = Server_TYPE(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"CWD",3) == 0) {
				checkResult = Server_CWD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"MKD",3) == 0) {
				checkResult = Server_MKD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else if(strncmp(sentence[ID],"RMD",3) == 0) {
				checkResult = Server_RMD(ID);
				if(checkResult == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else {
				int m = write(connfd,"500 Unknown command. 2\n",strlen("500 Unknown command. 2\n"));
				if (m < 0) {
					printf("Error Send RETR/STOR Response");
					break;
	 			}
				continue;
			}
		}
	}
	stage[ID] = 0;
	pthread_mutex_lock(&mutex);
	clientNum--;
	client_table[ID] = 0;
	pthread_mutex_unlock(&mutex);
	close(connfd);
}

int main(int argc, char **argv) {
	int thr_id;
	int pth_ID[11];
	int client_st = 0;

	pthread_t p_thread[11];
	pasv_port = 20000;
	argPort = 0;
	argIndex = 0;
	apcIndex = 0;
	apIndex = 0;
	clientNum = 0;
	pthread_mutex_init(&mutex,NULL);

	for(int k=0;k<11;k++) {
		port_or_pasv[k] = 0;
		portFlag[k] = 0;
		pasvFlag[k] = 0;
		client_table[k] = 0;
		stage[k] = 0;;
	}

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	if(argc == 5) {
		if(strcmp(argv[1],"-port") == 0 && strcmp(argv[3],"-root") == 0) {
			while(argv[2][argIndex] >= '0' && argv[2][argIndex] <= '9') {
				argPortCh[apcIndex] = argv[2][argIndex];
				argIndex++;
				apcIndex++;
			}
			argPortCh[apcIndex] = '\0';
			argPort = atoi(argPortCh);

			argIndex = 0;
			while(argv[4][argIndex] != '\0') {
				argPath[apIndex] = argv[4][argIndex];
				apIndex++;
				argIndex++;
			}
			argPath[apIndex] = '\0';
		}
		else { 
			argPort = 21;
			strcpy(argPath,"/tmp");
		}		
	}
	else {
		argPort = 21;
		strcpy(argPath,"/tmp");
	}

	for(int k=0;k<11;k++) {
		strcpy(curPath[k],argPath);
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(argPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	while (1) {

		if ((client_st = accept(listenfd, (struct sockaddr *)&client_addr, &len)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
		else{
			int n = write(client_st, "220 FTP server ready\n", strlen("220 FTP server ready\n"));
			if(n < 0) {
				printf("Error Write Initial Message");
				close(client_st);
				return 1;
			}
		}

		if(clientNum == 10) {
			for(int k = 1;k<11;k++) {
				if(client_table[k] == 1) {
					close(client_st);
					continue;
				}
			}
			close(listenfd);
			return 0;
		}

		pthread_mutex_lock(&mutex);
		clientNum++;
		client_table[clientNum] = 1;
		pthread_mutex_unlock(&mutex);

		printf("ClientNum : %d\n",clientNum);

		new_connfd[clientNum] = client_st;
		printf("accept %s\n",inet_ntoa(client_addr.sin_addr));
		
		
		pth_ID[clientNum] = clientNum;
		thr_id = pthread_create(&p_thread[clientNum],NULL,Server_MULTI,(void *)&pth_ID[clientNum]);
		if(thr_id < 0) {
			perror("thread create error: ");
			close(listenfd);
			return 1;
		}

		pthread_detach(p_thread[clientNum]);
		
	}

	close(listenfd);
	return 0;
}		

