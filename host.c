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
    
    char str2[7];
    int ints[4];
    for(i = 0; i < 4; i++){
      ints[i] = (int)buf[i];
    }
    sprintf(str2, "%d.%d.%d.%d", ints[0], ints[1], ints[2], ints[3]);
    
    char str3[7];
    for(i = 38; i < 42; i++){
      ints[i-38] = (int)buf[i];
    }
    sprintf(str3, "%d.%d.%d.%d", ints[0], ints[1], ints[2], ints[3]);

    if(n != -1){
      //char* ip = inet_ntoa(arp.sourceIp.sin_addr);
      //fprintf(stderr, "got something: %s\n", ip);
      unsigned char str[6];
      int i;
      memcpy(str, &buf[4], 6);
      fprintf(stderr, "Target Mac%02X::%02X::%02X::%02X::%02X::%02X\n", str[0], str[1], str[2], str[3], str[4], str[5]); 
      fprintf(stderr, "TargetV Ip: %s\n", str2);
    }
    //close(packet_socket);
  }

  


}
