#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>




struct sockaddr_in createIpAddr(char str[20]){
  struct sockaddr_in ipaddr;
  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = inet_addr(str);
  return ipaddr;
}




int main(){
  int send_socket1, send_socket2;
  int send_socketH3, send_socketH4, send_socketH5;
  int send_socketH1 = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socketH2 = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in ipaddr, r1addr, r2addr, h1addr, h2addr, h3addr, h4addr, h5addr;
  int packet_socket, send_socket, e;
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;

  h1addr = createIpAddr("10.1.0.3");
  h2addr = createIpAddr("10.1.1.5");

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
    int ints[4];
    int ints2[4];
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
      continue;
    int i;
    char str[7];
    char str2[7];
    for(i = 28; i < 32; i++){
      ints[i-28] = (int)buf[i];
    }
    
    for(i = 38; i < 42; i++){
      ints2[i-38] = (int)buf[i];
    }
    char macaddr[11];
    int len = 0;
    for(i = 0; i < 6; i++){//gets macaddr of last recvd
       len+=sprintf(macaddr+len,"%02X%s",recvaddr.sll_addr[i],i < 5 ? ":":"");
    }



    sprintf(str, "%d.%d.%d.%d", ints[0], ints[1], ints[2], ints[3]);
    sprintf(str2, "%d.%d.%d.%d", ints2[0], ints2[1], ints2[2], ints2[3]);
    //start processing all others
    
    if(strcmp(str, "10.1.0.3") == 0){//packet from h1
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("on line %s\n\n", str2); 
      sendto(send_socketH2, buf, 1500, 0, (struct sockaddr*)&h2addr, sizeof(h2addr)); 
    }
    if(strcmp(str, "10.1.1.5") == 0){//packet from h2
      printf("Got a %d byte packet from ip: %s\n", n, str);
      printf("on line %s\n\n", str2);
      sendto(send_socketH1, buf, 1500, 0, (struct sockaddr*)&h1addr, sizeof(h1addr)); 
    }


    
    
  }
  //exit
  return 0;
}
