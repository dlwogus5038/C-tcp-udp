#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>

#define MAXBUF 1024*1024

int count = 0;
char * pBufAddr[51];

void uppercase(char *p) {
  for ( ; *p; ++p) *p = toupper(*p);
}

void echo(int sd) {
    char bufin[MAXBUF];
    int lenCount = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    struct sockaddr_in remote;

    /* need to know how big address struct is, len must be set before the
       call to recvfrom!!! */
    socklen_t len = sizeof(remote);

    while (1) {
      /* read a datagram from the socket (put result in bufin) */
      int n = recvfrom(sd, bufin, MAXBUF, 0, (struct sockaddr *) &remote, &len);
      if (n < 0) {
        perror("Error receiving data");
      } else {
//===============================================================================================
        /* 0-50 */
        if(strcmp(bufin,"start_send") == 0)
        {
          for(i=0;i<MAXBUF;i++)
            bufin[i] = '\0';
          char numChar[10];

          if(count/10 == 0) {
            numChar[0] = '0' + count;
            numChar[1] = '\0';
          }
          else {
            numChar[0] = '0' + count/10;
            numChar[1] = '0' + count%10;
            numChar[2] = '\0';
          }

          int send_count = sendto(sd, numChar, strlen(numChar), 0, (struct sockaddr *)&remote, len);
          if (send_count < 0) {
            perror("Problem sending data");
            exit(1);
          }
          if (send_count != strlen(numChar)) {
            printf("Sendto sent %d bytes\n", send_count);
          }
          for(i=0;i<=50;i++){
                for(k=0;k<MAXBUF;k++)
                  bufin[k] = '\0';
                int num_0_50 = recvfrom(sd, bufin, MAXBUF, 0, (struct sockaddr *) &remote, &len);
                if (num_0_50 < 0) {
                  perror("Error receiving data");
                }
                if(i < count){
                  int send_buf = sendto(sd, pBufAddr[i], strlen(pBufAddr[i]), 0, (struct sockaddr *)&remote, len);
                  if (send_buf < 0) {
                    perror("Problem sending data");
                    exit(1);
                  }
                  if (send_buf != strlen(pBufAddr[i])) {
                    printf("Sendto sent %d bytes\n", send_buf);
                  }
                }
            }
          continue;
        }

//===============================================================================================
        i = count;
        for(;i != 0;){
            i = i / 10;
            j++;
        }
        
        lenCount = j;
        i = count;
        
        char* result = (char*)malloc(sizeof(char)*MAXBUF); 
        if(i == 0){
            result[0] = '0';
            result[1] = ' ';
            result[2] = '\0';
            lenCount = 1;
        }
        else {
            result[j] = ' ';
            result[j+1] = '\0';
            for (; j > 0 ; j--) {
                result[j - 1] = '0' + i%10;
                i = i/10;
            }
        }
        
        strcat(result,bufin);

        if(count <= 50){
          pBufAddr[count] = result;
        }
        count++;
        //uppercase(bufin);
        /* Got something, just send it back */
        sendto(sd, result, strlen(result), 0, (struct sockaddr *)&remote, len);
      }
    }
}

/* server main routine */

int main() {
  int ld;
  struct sockaddr_in skaddr;
  socklen_t length;

  /* create a socket
     IP protocol family (PF_INET)
     UDP protocol (SOCK_DGRAM)
  */

  if ((ld = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* establish our address
     address family is AF_INET
     our IP address is INADDR_ANY (any of our IP addresses)
     the port number is 9876
  */

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(9876);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr)) < 0) {
    printf("Problem binding\n");
    exit(0);
  }

  /* find out what port we were assigned and print it out */

  length = sizeof(skaddr);
  if (getsockname(ld, (struct sockaddr *) &skaddr, &length) < 0) {
    printf("Error getsockname\n");
    exit(1);
  }

  /* Go echo every datagram we get */
  echo(ld);
  return 0;
}
