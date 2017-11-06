#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct{
  struct sockaddr_ll sourceMac;
  struct sockaddr_ll targetMac;
  struct sockaddr_in sourceIp;
  struct sockaddr_in targetIp;
} arpHeader;



struct sockaddr_in createIpAddr(char str[20]){
  struct sockaddr_in ipaddr;
  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = inet_addr(str);
  return ipaddr;
}




int main(){
  int send_socketR1 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketR2 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH1 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH2 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH3 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH4 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH5 = socket(AF_INET, SOCK_DGRAM, 0);
   
  struct sockaddr_ll r1mac, r2mac;
  struct sockaddr_in ipaddr, r1addr, r2addr, h1addr, h2addr, h3addr, h4addr, h5addr;
  int packet_socket, send_socket, e;
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;

  r1addr = createIpAddr("10.0.0.1");
  r2addr = createIpAddr("10.0.0.2");
  h1addr = createIpAddr("10.1.0.3");
  h2addr = createIpAddr("10.1.1.5");
  h3addr = createIpAddr("10.3.0.32");
  h4addr = createIpAddr("10.3.1.201");
  h5addr = createIpAddr("10.3.4.54");
  
  
  
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp;
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);
      //create a packet socket on interface r?-eth1
      //if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
      if(!strncmp(&(tmp->ifa_name[0]),"r1-eth0",7)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
   
        struct sockaddr_ll *s = (struct sockaddr_ll*) tmp->ifa_addr;
        int i;
        for(i = 0; i < 6; i++){  
          r1mac.sll_addr[i] = s->sll_addr[i];
        }
	packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket<0){
	  perror("socket");
	  return 2;
	}
        
	//if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	//  perror("bind");
	//}
      }
    }
  }
  //free the interface list when we don't need it anymore
  //freeifaddrs(ifaddr);

  printf("Ready to recieve now\n");
  char *macaddr = malloc(11 * sizeof(char));
  char *wholeBuf = malloc(83 * sizeof(char));
  while(1){
    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);

    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
    
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
       continue;

    char str[7];
    sprintf(str, "%d.%d.%d.%d", buf[38], buf[39], buf[40], buf[41]);//target ip
    char str2[8];
    sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
    char str3[4];
    sprintf(str3, "%02X", buf[21]);//op flag
    
    int i;
    int len = 0;
    for(i = 0; i < 6; i++){//gets macaddr of last recvd
       len+=sprintf(macaddr+len,"%02X%s",recvaddr.sll_addr[i],i < 5 ? ":":"");
    }
    len = 0;
    for(i = 0; i < 42; i++){//gets macaddr of last recvd
      len+=sprintf(wholeBuf+len,"%02X%s", buf[i],i < 41 ? ":":"");
    }
    //start processing all others
    
    char sendbuf[1500];
    
    fprintf(stderr, "str: %s\n", str);
    if(strcmp(str, "10.1.0.1") == 0){//packet from h1
      if(strcmp(str2, "0806") == 0){//arp
        if(strcmp(str3, "01") == 0){//arp request
          fprintf(stderr, "is an arp packet\n");
          int ints2[4]; 
          char temp[4];
          int op = 2;
          memcpy(&buf[21], &op, 1);
          memcpy(temp, &buf[38], 4);
          memcpy(&buf[32], &buf[6], 6);//source mac cpy into target mac
          memcpy(&buf[38], &buf[28], 4);//source ip cpy into target ip
          memcpy(&buf[28], temp, 4);//target ip cpy into source ip
          memcpy(&buf[22], r1mac.sll_addr, 6);
          memcpy(buf, &buf[6], 6);//set eth header source
          memcpy(&buf[6], recvaddr.sll_addr, 6);//set eth header dest 
          sendto(send_socketH1, buf, 1500, 0, (struct sockaddr*)&h1addr, sizeof(h1addr));
        }
      }else{
        fprintf(stderr, "not an arp packet\n");
      }

    }
    
  }
  free(macaddr);
  free(wholeBuf);
  //exit
  return 0;
}
