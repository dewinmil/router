#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>


//ethernet header
//eth dst = broadcast
//eth src = requests mac
//eth type = 0x806 arp

typedef struct{
  struct sockaddr_ll sourceMac;
  struct sockaddr_ll targetMac;
  struct sockaddr_in sourceIp;
  struct sockaddr_in targetIp; 
  
} arpHeader;

typedef struct{

} icmpHeader;

struct sockaddr_in createIpAddr(char str[20]){
  struct sockaddr_in ipaddr;
  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = inet_addr(str);
  return ipaddr;
}



int main(){
  int send_socket = socket(AF_INET, SOCK_STREAM, 0);
 
  int packet_socket1, packet_socket2;
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp;
  struct sockaddr_in *sa;
  char *addr1, *addr2;
  struct sockaddr_ll macaddr1, macaddr2;
  struct sockaddr_in ipaddr1, ipaddr2;
  ipaddr1 = createIpAddr("10.1.0.3");
  ipaddr2 = createIpAddr("10.1.1.5");

  //int e = connect(send_socket, (struct sockaddr *)&ipaddr1, sizeof(ipaddr1));
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);
      //create a packet socket on interface r?-eth1


      //eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
	printf("Creating Socket eth1 on interface %s\n",tmp->ifa_name);
	packet_socket1 = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket1<0){
	  perror("socket");
	  return 2;
	}
        struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
        int i;
        //int len = 0;
        //for(i = 0; i < 6; i++)
        //   len+=sprintf(macaddr1+len,"%02X%s",s->sll_addr[i],i < 5 ? ":":"");
        //fprintf(stderr, "eth1 macaddr: %s\n", macaddr1);
        for(i = 0; i < 6; i++){
          macaddr1.sll_addr[i] = s->sll_addr[i];
        }

        if(bind(packet_socket1,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }

      //eth2
      if(!strncmp(&(tmp->ifa_name[3]),"eth2",4)){
	printf("Creating Socket eth2 on interface %s\n",tmp->ifa_name);
	packet_socket2 = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket2<0){
	  perror("socket");
	  return 2;
	}

        struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
        int i;
	//IMPORTANT NOTE tmp->ifa_addr->sll_addr = physical / mac address (of sender)
        for(i = 0; i < 6; i++){
          macaddr2.sll_addr[i] = s->sll_addr[i];
        }
	if(bind(packet_socket2,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
    }
  }
  //free the interface list when we don't need it anymore
  //freeifaddrs(ifaddr);

  //loop and recieve packets. We are only looking at one interface,
  //for the project you will probably want to look at more (to do so,
  //a good way is to have one socket per interface and use select to
  //see which ones have data)
  printf("Ready to recieve now\n");
  while(1){
    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);
    int sockaddrlen=sizeof(struct sockaddr);

    arpHeader arp;

    int n = recvfrom(packet_socket1, buf, 1500, MSG_DONTWAIT,(struct sockaddr*)&recvaddr, &recvaddrlen);
    int n2 = recvfrom(packet_socket2, buf, 1500, MSG_DONTWAIT,(struct sockaddr*)&recvaddr, &recvaddrlen);

    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
      continue;
    //start processing all others
    if(n != -1){
      printf("Got a %d byte packet from eth1\n", n);
    }//CURRENT IMPORTANT NOTES - macaddrs + recvaddr(should be ip)
    if(n2 != -1){
      printf("Got a %d byte packet from eth2\n", n2);
      int e = connect(send_socket, (struct sockaddr *)&ipaddr1, sizeof(ipaddr1));
      arp.sourceMac = macaddr2;
      arp.sourceIp = ipaddr2;
      send(send_socket, &arp, sizeof(arpHeader), 0);
      
    }
    
    
    //what else to do is up to you, you can send packets with send,
    //just like we used for TCP sockets (or you can use sendto, but it
    //is not necessary, since the headers, including all addresses,
    //need to be in the buffer you are sending)
    
  }
  //exit
  return 0;
}
