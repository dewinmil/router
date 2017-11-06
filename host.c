#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>


typedef struct{
  struct sockaddr_ll sourceMac;
  struct sockaddr_ll targetMac;
  struct sockaddr_in sourceIp;
  struct sockaddr_in targetIp;

} arpHeader;


int main(){
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

  char macaddr1[16];
  struct ifaddrs *tmp;
  struct sockaddr_in ipaddr, recvaddr;
  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;
  
  bind(sockfd, (struct sockaddr*)&ipaddr, sizeof(ipaddr));
  listen(sockfd, 10);
   
  char buf[1500];
     
  arpHeader arp;
  int len = sizeof(recvaddr);    
  int socket = accept(sockfd, (struct sockaddr*)&recvaddr, &len);
  
  while(1){
    int i;
    char macaddr[11];
    int len = 0;
    //for(i = 0; i < 6; i++){//gets macaddr of last recvd
    //   len+=sprintf(macaddr+len,"%02X%s",recvaddr.sll_addr[i],i < 5 ? ":":"");
    //}

  
   
    
    int n = recvfrom(sockfd, buf, 1500, 0, (struct sockaddr*)&recvaddr, &len);
    
    char str[7];
    sprintf(str, "%d.%d.%d.%d", buf[38], buf[39], buf[40], buf[41]);//dest ip
    char str2[8];
    sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
    char str3[4];
    sprintf(str3, "%02X", buf[21]);//op flag


    if(strcmp(str, "10.1.0.3") == 0){//got pack from router
      fprintf(stderr, "got pack from router\n");
      if(strcmp(str2, "0806") == 0){//got an arp packet
        fprintf(stderr, "got an arp pack\n");
        if(strcmp(str3, "02") == 0){//got arp reply
          fprintf(stderr, "got an arp reply\n");
          
        }
      }
      
    }
    //close(packet_socket);
  }

  


}
