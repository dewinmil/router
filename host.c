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

    int n = recvfrom(sockfd, buf, 1500, 0, (struct sockaddr*)&recvaddr, &len);
    if(n != -1){
      fprintf(stderr, "got something: %s\n", buf);
    }
    //close(packet_socket);
  }

  


}
