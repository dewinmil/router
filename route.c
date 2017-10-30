#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>

int main(){
  int packet_socket, send_socket;
  struct sockaddr_in ipaddr, broadcastaddr;
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;

  broadcastaddr.sin_family = AF_INET;
  broadcastaddr.sin_port = htons(5555);
  broadcastaddr.sin_addr.s_addr = inet_addr("1.1.1.1");

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
  freeifaddrs(ifaddr);

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
    printf("Got a %d byte packet from ip: %s\n", n, str);
    printf("on line %s\n", str2);
    
    if(strcmp(str2, "10.1.0.1") == 0){//packet from h1
      fprintf(stderr, "from h1\n");
    }
    if(strcmp(str2, "10.1.1.1") == 0){//packet from h2
      fprintf(stderr, "from h2\n");
    }

    int n2 = send(send_socket, &str, 8, 0);

    
    
  }
  //exit
  return 0;
}
