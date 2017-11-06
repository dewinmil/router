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
#include <netinet/ip.h>

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


//taken from 
//http://www.microhowto.info/howto/calculate_an_internet_protocol_checksum_in_c.html
uint16_t createCheckSum(char* data, size_t length) {
    // Cast the data pointer to one that can be indexed.

    // Initialise the accumulator.
    uint32_t acc=0xffff;

    // Handle complete 16-bit blocks.
    for (size_t i=0;i+1<length;i+=2) {
        uint16_t word;
        memcpy(&word,data+i,2);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Handle any partial block at the end of the data.
    if (length&1) {
        uint16_t word=0;
        memcpy(&word,data+length-1,1);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Return the checksum in network byte order.
    return htons(~acc);
}

int main(){
   
  struct sockaddr_ll r1mac, r2mac;
  struct sockaddr_in ipaddr, r1addr, r2addr, h1addr, h2addr, h3addr, h4addr, h5addr;
  int packet_socket, eth1_socket, eth2_socket, eth3_socket, send_socket, e;
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;

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
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
   
	eth1_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth1_socket<0){
	  perror("socket");
	  return 2;
	}
        
	if(bind(eth1_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
      if(!strncmp(&(tmp->ifa_name[3]),"eth2",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
   
	eth2_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth2_socket<0){
	  perror("socket");
	  return 2;
	}
        
	if(bind(eth2_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
      if(!strncmp(&(tmp->ifa_name[3]),"eth3",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
   
	eth3_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth3_socket<0){
	  perror("socket");
	  return 2;
	}
        
	if(bind(eth3_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
    }
  }
  //free the interface list when we don't need it anymore
  //freeifaddrs(ifaddr);

  printf("Ready to recieve now\n");
  char *macaddr = malloc(11 * sizeof(char));
  char *wholeBuf = malloc(83 * sizeof(char));
  char *icmpBuf = malloc(196 * sizeof(char));
  while(1){
    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);

    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
    
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
       continue;

    
    int i;
    int len = 0;
    /*
    for(i = 0; i < 42; i++){//gets whole buffer
      len+=sprintf(wholeBuf+len,"%02X%s", buf[i],i < 41 ? ":":"");
    }
    */
    //start processing all others
    
    char sendbuf[1500];
    
    //fprintf(stderr, "str: %s\n", str);

    if(n == 42){
      char str[7];
      sprintf(str, "%d.%d.%d.%d", buf[38], buf[39], buf[40], buf[41]);//target ip
      char str2[8];
      sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
      char str3[4];
      sprintf(str3, "%02X", buf[21]);//op flag
      if(strcmp(str, "10.1.0.1") == 0){//packet from h1
        if(strcmp(str2, "0806") == 0){//arp
          if(strcmp(str3, "01") == 0){//arp request
            fprintf(stderr, "is an arp packet\n\n");
            int ints2[4]; 
            char temp[4];
            int op = 2;
          
            memcpy(temp, &buf[38], 4);
            memcpy(&buf[21], &op, 1);//set op - may want to set buf[0] to int 0 as op is 2 bytes
            memcpy(&buf[32], &buf[6], 6);//source mac cpy into target mac
            memcpy(&buf[38], &buf[28], 4);//source ip cpy into target ip
            memcpy(&buf[28], temp, 4);//target ip cpy into source ip
            memcpy(&buf[22], r1mac.sll_addr, 6);
            memcpy(&buf[6], r1mac.sll_addr, 6);//set eth header source
            memcpy(buf, recvaddr.sll_addr, 6);//set eth header dest 
            
            /*
            len = 0;
            for(i = 0; i < 42; i++){//gets whole buffer
              len+=sprintf(wholeBuf+len,"%02X%s", buf[i],i < 41 ? ":":"");
            }
            */
            send(eth1_socket, buf, 42, 0);
          }
        }else{
          fprintf(stderr, "not an arp packet\n");
        }
    
      }
    }else if (n == 98){
      fprintf(stderr, "is an icmp packet \n\n");
      
      char str[7];
      sprintf(str, "%d.%d.%d.%d", buf[30], buf[31], buf[32], buf[33]);//target ip
      char str2[8];
      sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
      char str3[4];
      sprintf(str3, "%02X", buf[21]);//op flag
      
       
      fprintf(stderr, "str: %s\n", str);
      if(strcmp(str, "10.0.0.1") == 0){//dest is r1
        
        fprintf(stderr, "made it\n");
        
        //struct ip *ipheader;
        char ipheader[20];
        
        uint16_t checksum;
        uint16_t clear = 0; 
        uint16_t sum;

        memcpy(&checksum, &buf[24], 2); 
        memcpy(&buf[24], &clear, 2); 
        memcpy(ipheader, &buf[14], 20); 
        sum = createCheckSum(ipheader, 20);
        
        if(sum == checksum){
          fprintf(stderr, "sum: %d\n", sum);
          fprintf(stderr, "checksum: %d\n", checksum);

          //for ip header
          char temp[4];
          memcpy(temp, &buf[26], 4);//source ip -> tmp
          memcpy(&buf[26], &buf[30], 4);//target ip -> source ip
          memcpy(&buf[30], temp, 4);//source ip -> target ip

          uint8_t *ttl = &buf[22]; 
          *ttl = *ttl - 1;
          memcpy(&buf[22], ttl, 1);//replace old ttl

          memcpy(ipheader, &buf[14], 20); 
          checksum = createCheckSum(ipheader, 20);//create newchecksum 
          memcpy(&buf[24], &checksum, 2);//insert checksum into ipheader

          //for icmp header
          

          

        }
  
         
        
      }



      /*
      len = 0;
      for(i = 0; i < n; i++){//gets whole buffer
        len+=sprintf(icmpBuf+len,"%02X%s", buf[i],i < (n-1) ? ":":"");
      }
      */
    }
    
  }
  free(macaddr);
  free(wholeBuf);
  //exit
  return 0;
}
