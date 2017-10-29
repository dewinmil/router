#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>



typedef struct{
  struct sockaddr_ll sourceMac;
  struct sockaddr_ll targetMac;
  struct sockaddr_in sourceIp;
  struct sockaddr_in targetIp;

} arpHeader;




int main(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  char macaddr1[16];
  struct ifaddrs *tmp;
  struct sockaddr_in ipaddr, recvaddr;
  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;
  
  
  char buf[1500];
  //struct sockaddr_ll recvaddr;
  int recvaddrlen=sizeof(struct sockaddr_ll);
     
  arpHeader arp;
  
  bind(sockfd, (struct sockaddr*)&ipaddr, sizeof(ipaddr));
  listen(sockfd, 10);
    
  while(1){
    
    int len = sizeof(recvaddr);
    int packet_socket = accept(sockfd, (struct sockaddr*)&recvaddr, &len);


    int n = recv(packet_socket, &arp, sizeof(arpHeader), 0);
    fprintf(stderr, "got something: %d\n", n);

  }

  


}
