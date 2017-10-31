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

  if(bind(packet_socket, (struct sockaddr*)&ipaddr, sizeof(ipaddr))==-1){
    perror("bind");
  }
  
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
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);

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
  while(1){
    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);

    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);

    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
       continue;

    int i;
    char str[7];
    int ints[4];
    for(i = 28; i < 32; i++){
      ints[i-28] = (int)buf[i];
    }

    char macaddr[11];
    int len = 0;
    for(i = 0; i < 6; i++){//gets macaddr of last recvd
       len+=sprintf(macaddr+len,"%02X%s",recvaddr.sll_addr[i],i < 5 ? ":":"");
    }

    sprintf(str, "%d.%d.%d.%d", ints[0], ints[1], ints[2], ints[3]);
    //start processing all others
   

 
    if(strcmp(str, "10.0.0.1") == 0){//packet from r1
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H1: %s", macaddr);
      sendto(send_socketH3, buf, 1500, 0, (struct sockaddr*)&h3addr, sizeof(h3addr)); 
      sendto(send_socketH4, buf, 1500, 0, (struct sockaddr*)&h4addr, sizeof(h4addr)); 
      sendto(send_socketH5, buf, 1500, 0, (struct sockaddr*)&h5addr, sizeof(h5addr)); 
    }
    if(strcmp(str, "10.0.0.2") == 0){//packet from r2
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H1: %s", macaddr);
      sendto(send_socketH1, buf, 1500, 0, (struct sockaddr*)&h1addr, sizeof(h1addr)); 
      sendto(send_socketH2, buf, 1500, 0, (struct sockaddr*)&h2addr, sizeof(h2addr)); 
    }
    if(strcmp(str, "10.1.0.3") == 0){//packet from h1
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H1: %s", macaddr);
      sendto(send_socketH2, buf, 1500, 0, (struct sockaddr*)&h2addr, sizeof(h2addr)); 
      sendto(send_socketR2, buf, 1500, 0, (struct sockaddr*)&r2addr, sizeof(r2addr)); 
    }
    if(strcmp(str, "10.1.1.5") == 0){//packet from h2
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H2: %s", macaddr);
      sendto(send_socketH1, buf, 1500, 0, (struct sockaddr*)&h1addr, sizeof(h1addr)); 
      sendto(send_socketR2, buf, 1500, 0, (struct sockaddr*)&r2addr, sizeof(r2addr)); 
    }
    if(strcmp(str, "10.3.0.32") == 0){//packet from h3
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H3: %s", macaddr);
      sendto(send_socketH4, buf, 1500, 0, (struct sockaddr*)&h4addr, sizeof(h4addr)); 
      sendto(send_socketH5, buf, 1500, 0, (struct sockaddr*)&h5addr, sizeof(h5addr)); 
      sendto(send_socketR1, buf, 1500, 0, (struct sockaddr*)&r1addr, sizeof(r1addr)); 
    }
    if(strcmp(str, "10.3.1.201") == 0){//packet from h4
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H2: %s", macaddr);
      sendto(send_socketH3, buf, 1500, 0, (struct sockaddr*)&h3addr, sizeof(h3addr)); 
      sendto(send_socketH5, buf, 1500, 0, (struct sockaddr*)&h5addr, sizeof(h5addr)); 
      sendto(send_socketR1, buf, 1500, 0, (struct sockaddr*)&r1addr, sizeof(r1addr)); 
    }
    if(strcmp(str, "10.3.4.54") == 0){//packet from h5
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("macaddress H2: %s", macaddr);
      sendto(send_socketH3, buf, 1500, 0, (struct sockaddr*)&h3addr, sizeof(h3addr)); 
      sendto(send_socketH4, buf, 1500, 0, (struct sockaddr*)&h4addr, sizeof(h4addr)); 
      sendto(send_socketR1, buf, 1500, 0, (struct sockaddr*)&r1addr, sizeof(r1addr)); 
    }

    
    
  }
  //exit
  return 0;
}
