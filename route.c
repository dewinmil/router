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
  //stor mac address from arp reply's - blank is compared to check empty
  char *h1arp = calloc(6, sizeof(char));
  char *h2arp = calloc(6, sizeof(char));
  char *h3arp = calloc(6, sizeof(char));
  char *h4arp = calloc(6, sizeof(char));
  char *h5arp = calloc(6, sizeof(char));
  char *r1arp = calloc(6, sizeof(char));
  char *r2arp = calloc(6, sizeof(char));
  char *blank = calloc(6, sizeof(char));

  //variables
  struct sockaddr_ll r1mac, r2mac;
  struct sockaddr_in ipaddr, r1addr, r2addr, h1addr, h2addr, h3addr, h4addr, h5addr;
  int packet_socket, r1_socket, r2_socket, eth1_socket, eth2_socket, eth3_socket, send_socket, e;
  send_socket = socket(AF_INET, SOCK_DGRAM, 0);

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(5555);
  ipaddr.sin_addr.s_addr = INADDR_ANY;

  h4addr = createIpAddr("10.3.1.201");
  h5addr = createIpAddr("10.3.4.54");
  
  
  packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

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
	r2_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket<0){
	  perror("socket");
	  return 2;
	}
        
	if(bind(r2_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
      if(!strncmp(&(tmp->ifa_name[0]),"r2-eth0",7)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
   
        struct sockaddr_ll *s = (struct sockaddr_ll*) tmp->ifa_addr;
        int i;
        for(i = 0; i < 6; i++){  
          r2mac.sll_addr[i] = s->sll_addr[i];
        }
	r1_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket<0){
	  perror("socket");
	  return 2;
	}
        
	if(bind(r1_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
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


  
  //beginning of loop
  while(1){
    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);

    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
    
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
       continue;

    
    int i;
    int len = 0;
    
    char sendbuf[1500];
    

    if(n == 42){//if size of arp packet
      char str[7];
      sprintf(str, "%d.%d.%d.%d", buf[38], buf[39], buf[40], buf[41]);//target ip
      char str2[8];
      sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
      char str3[4];
      sprintf(str3, "%02X", buf[21]);//op flag
      if(strcmp(str2, "0806") == 0){//is an arp packet
        if(strcmp(str3, "01") == 0){//is an arp request
          fprintf(stderr, "is an arp packet\n\n");
          int ints2[4]; 
          char temp[4];
          int op = 2;
        
          memcpy(temp, &buf[38], 4);//copy target ip address
          memcpy(&buf[21], &op, 1);//set op to arp reply
          memcpy(&buf[32], &buf[6], 6);//source mac cpy into target mac
          memcpy(&buf[38], &buf[28], 4);//source ip cpy into target ip
          memcpy(&buf[28], temp, 4);//target ip cpy into source ip

          
          char *routerAddress = malloc(11 * sizeof(char));
          memcpy(routerAddress, &buf, 6);//get this routers mac address
         
          memcpy(&buf[22], r1mac.sll_addr, 6);
          memcpy(&buf[6], r1mac.sll_addr, 6);//set eth header source
          memcpy(buf, recvaddr.sll_addr, 6);//set eth header dest 
            
          fprintf(stderr, "str: %s\n", str);

          //if router 1
          if(strcmp(str, "10.1.0.1") == 0){//packet from h1
            send(eth1_socket, buf, 42, 0);
          }
          else if(strcmp(str, "10.1.1.1") == 0){//packet from h2
            send(eth2_socket, buf, 42, 0);
          }
          else if(strcmp(str, "10.0.0.1") == 0){//packet from r2
            send(r2_socket, buf, 42, 0);
          }
          else{//must be router 2

            memcpy(&buf[22], r2mac.sll_addr, 6);
            memcpy(&buf[6], r2mac.sll_addr, 6);//set eth header source
           
            fprintf(stderr, "str: %s\n", str);
            if(strcmp(str, "10.3.0.1") == 0){//packet from h3
              send(eth1_socket, buf, 42, 0);
            }
            else if(strcmp(str, "10.3.1.1") == 0){//packet from h4
              send(eth2_socket, buf, 42, 0);
            }
            else if(strcmp(str, "10.3.4.1") == 0){//packet from h5
              send(eth3_socket, buf, 42, 0);
            }
            else if(strcmp(str, "10.0.0.2") == 0){//packet from r1
              send(r1_socket, buf, 42, 0);
            }
          }
          free(routerAddress);
        }else if(strcmp(str3, "02") == 0){//got an arp reply
          char recvMacAddr[11];
          sprintf(str, "%d.%d.%d.%d", buf[28], buf[29], buf[30], buf[31]);//target ip
          fprintf(stderr, "TARGET IP =============== %s\n", str);

          //save / store mac address from arp reply
          if(strcmp(str, "10.0.0.1") == 0){//r1
            memcpy(r1arp, &buf[6], 6);
          }
          if(strcmp(str, "10.0.0.2") == 0){//r2
            memcpy(r2arp, &buf[6], 6);
          }
          if(strcmp(str, "10.1.0.3") == 0){//h1
            memcpy(h1arp, &buf[6], 6);
          }
          if(strcmp(str, "10.1.1.5") == 0){//h2
            memcpy(h2arp, &buf[6], 6);
          }
          if(strcmp(str, "10.3.0.32") == 0){//h3
            memcpy(h3arp, &buf[6], 6);
          }
          if(strcmp(str, "10.3.1.-55") == 0){//h4 - -55 due to storing as int
            memcpy(h4arp, &buf[6], 6);
          }
          if(strcmp(str, "10.3.4.54") == 0){//h5
            memcpy(h5arp, &buf[6], 6);
          }


 
          //sprintf(recvMacAddr, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
          sprintf(recvMacAddr, "%02x:%02x:%02x:%02x:%02x:%02x", buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]);
          fprintf(stderr, "Received Mac Address: %s\n", recvMacAddr); 
          
        }
      }
      else{ 
        fprintf(stderr, "not an arp packet\n"); 
      }
    }else{//packet with ip header (the one's I deal with)
    
      
      
      char str[7];
      sprintf(str, "%d.%d.%d.%d", buf[30], buf[31], buf[32], buf[33]);//target ip
      char str2[8];
      sprintf(str2, "%02X%02X", buf[12], buf[13]);//type
      char str3[4];
      sprintf(str3, "%02X", buf[21]);//op flag
      
       
      fprintf(stderr, "str: %s\n", str);
      if(strcmp(str, "10.0.0.1") == 0 || strcmp(str, "10.0.0.2") == 0){//dest is a router
        
        //for ethernet header
        char tempMac[6];
        memcpy(tempMac, buf, 6);//cpy target mac
        memcpy(buf, &buf[6], 6);//source mac -> target mac
        memcpy(&buf[6], tempMac, 6);//target mac -> source mac


        char ipheader[20];
        
        uint16_t checksum;
        uint16_t clear = 0; 
        uint16_t sum;

        memcpy(&checksum, &buf[24], 2); //store checksum 
        memcpy(&buf[24], &clear, 2); //clear checksum from ip header
        memcpy(ipheader, &buf[14], 20); //copy ip header
        sum = createCheckSum(ipheader, 20); //create new checksum for ip header
        
        if(sum == checksum){

          //for ip header
          char temp[4];
          memcpy(temp, &buf[26], 4);//source ip -> tmp
          memcpy(&buf[26], &buf[30], 4);//target ip -> source ip
          memcpy(&buf[30], temp, 4);//source ip -> target ip

          uint8_t *ttl = &buf[22]; 
          *ttl = *ttl - 1;
          memcpy(&buf[22], ttl, 1);//replace old ttl

          memcpy(ipheader, &buf[14], 20);//cpy ipheader
          checksum = createCheckSum(ipheader, 20);//create newchecksum 
          memcpy(&buf[24], &checksum, 2);//insert checksum into ipheader

     
          sprintf(str3, "%02X", buf[23]);//protocol flag flag

          int checkval = 0;
          char icmpheader[64];
          if(strcmp(str3, "01") == 0){
          //for icmp header
          
            memcpy(&checksum, &buf[36], 2);//cpy checksum 
            memcpy(&buf[36], &clear, 2);//clear checksum
            memcpy(icmpheader, &buf[34], 64);//cpy icmp header 
            sum = createCheckSum(icmpheader, 64);//create icmp header checksum
            if(sum != checksum){
              checkval = 1;
            }
          }
          if(checkval == 0){
            memcpy(&buf[34], &clear, 1);
              
            if(strcmp(str3, "01") == 0){
              memcpy(icmpheader, &buf[34], 64);//cpy icmp buffer
              checksum = createCheckSum(icmpheader, 64);//create newchecksum 
              memcpy(&buf[36], &checksum, 2);//insert checksum into icmpheader
            }
            if(strcmp(str, "10.0.0.1") == 0){
              sprintf(str, "%d.%d.%d.%d", buf[30], buf[31], buf[32], buf[33]);//target ip
              if(strcmp(str, "10.1.0.3") == 0){// reply to h1
                send(eth1_socket, buf, n, 0);
              }
              else if(strcmp(str, "10.1.1.5") == 0){// reply to h2
                send(eth2_socket, buf, n, 0);
              }
              else if(strcmp(str, "10.0.0.2") == 0){// reply to r2
                send(r2_socket, buf, n, 0);
              }
            }
            else if(strcmp(str, "10.0.0.2") == 0){
              sprintf(str, "%d.%d.%d.%d", buf[30], buf[31], buf[32], buf[33]);//target ip
              if(strcmp(str, "10.3.0.32") == 0){// reply to h3
                send(eth1_socket, buf, n, 0);
              }
              else if(strcmp(str, "10.3.1.201") == 0){// reply to h4
                send(eth2_socket, buf, n, 0);
              }
              else if(strcmp(str, "10.3.4.54") == 0){// reply to h5
                send(eth3_socket, buf, n, 0);
              }
              else if(strcmp(str, "10.0.0.1") == 0){// reply to r1
                send(r1_socket, buf, n, 0);
              }
            }
            
          }          
        }
      }else{//not destined for router - need to foreward packet
        
        sprintf(str, "%d.%d.%d", buf[30], buf[31], buf[32]);//target ip
      
        char *routerAddress = malloc(11 * sizeof(char));
        memcpy(routerAddress, &buf, 6);//get this routers mac address
      
        int conditional = 1;

        char forewardBuf[1500];
        memcpy(forewardBuf, buf, 1500);
        
        //for ethernet header
        char tempMac[6];
        memcpy(tempMac, forewardBuf, 6);//cpy target mac
        memcpy(&forewardBuf[6], tempMac, 6);//target mac -> source mac
      
        //augment ttl
        uint8_t *ttl = &forewardBuf[22]; 
        *ttl = *ttl - 1;
        memcpy(&forewardBuf[22], ttl, 1);//replace old ttl
        
        if(ttl <= 0){
          //drop packet
        }
        uint16_t clear = 0; 
        memcpy(&forewardBuf[24], &clear, 2); 
        
        char ipheader[20];
        memcpy(ipheader, &forewardBuf[14], 20);
       
        uint16_t checkSum;
        checkSum = createCheckSum(ipheader, 20);
        memcpy(&forewardBuf[24], &checkSum, 2);

        fprintf(stderr, "CURRENT RELEVENT STRING STR: %s\n", str);
        if(strcmp(routerAddress, r1mac.sll_addr)==0){//we are router 1
          if(strcmp(str, "10.1.0") == 0){//foreward to h1
            if(strcmp(h1arp, blank) != 0){
              memcpy(forewardBuf, h1arp, 6);//create target mac address
              send(eth1_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
          else if(strcmp(str, "10.1.1") == 0){//foreward to h2 
            if(strcmp(h2arp, blank) != 0){
              memcpy(forewardBuf, h2arp, 6);//create target mac address
              send(eth2_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
          else if(strcmp(str, "10.3.0") == 0 || strcmp(str, "10.3.1") == 0 || strcmp(str, "10.3.4") == 0){//foreward to r2 
            if(strcmp(r2arp, blank) != 0){
              fprintf(stderr, "r2arp: %s\n", r2arp);
              memcpy(forewardBuf, r2arp, 6);//create target mac address
              send(r2_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
        }
        else if(strcmp(routerAddress, r2mac.sll_addr)==0){//we are router 2
          if(strcmp(str, "10.3.0") == 0){//foreward to h3
            if(strcmp(h3arp, blank) != 0){
              memcpy(forewardBuf, h3arp, 6);//create target mac address
              send(eth1_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
          else if(strcmp(str, "10.3.1") == 0){//foreward to h4
            if(strcmp(h4arp, blank) != 0){
              memcpy(forewardBuf, h4arp, 6);//create target mac address
              send(eth2_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
          else if(strcmp(str, "10.3.4") == 0){//foreward to h5 
            if(strcmp(h5arp, blank) != 0){
              memcpy(forewardBuf, h5arp, 6);//create target mac address
              send(eth3_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
          else if(strcmp(str, "10.1.0") == 0 || strcmp(str, "10.1.1") == 0){//foreward to r1 
            if(strcmp(r1arp, blank) != 0){
              memcpy(forewardBuf, r1arp, 6);//create target mac address
              send(r1_socket, forewardBuf, n, 0);
              conditional = 0;
            }
          }
        }
        
        
        if(conditional == 1){//we have not recieved an arp reply / mac address unknown so send arp request





          char arpRequest[42];
          fprintf(stderr, "str-----: %s\n", str);

          //eth header
          uint16_t allOnes = 65535;
          memcpy(arpRequest, &allOnes, 2); 
          memcpy(&arpRequest[2], &allOnes, 2); 
          memcpy(&arpRequest[4], &allOnes, 2);//all F's broacast mac
          memcpy(&arpRequest[6], &buf, 6);//set source mac address
          
          //arp header
          uint16_t arpType = 1544;//hex 608 - inserts backwards as 806 (arp)
          memcpy(&arpRequest[12], &arpType, 2);//set ethernet type 0806 - arp
          uint16_t one = 256;//hex 01 00  flip to 10 00 flip to 00 01
          memcpy(&arpRequest[14], &one, 2);//set hardware type to 1
          uint16_t eightHundred = 8;//hex 00 08 - flips to 00 80 flips to 08 00
          memcpy(&arpRequest[16], &eightHundred, 2);//set hardware type to 1
          uint32_t sizeAndOp = 16778246;//hex 01 00 04 06 flips to 10 00 40 60 flips to 06 04 00 01
                                        //- 06-hwsize 04-protocol size 00 01-opcode
          memcpy(&arpRequest[18], &sizeAndOp, 4);//set sizes and op
          memcpy(&arpRequest[22], &buf, 6);//sender mac address(router)
          uint16_t allZeros = 0;
          memcpy(&arpRequest[32], &allZeros, 2);//target mac address - all  0's
          memcpy(&arpRequest[34], &allZeros, 2);//target mac address - all  0's
          memcpy(&arpRequest[36], &allZeros, 2);//target mac address - all  0's
          memcpy(&arpRequest[38], &buf[30], 4);//target ip address



          uint32_t r1Ip = 16777226;//hex 01 00 00 0a flips to 10 00 00 a0
                                 //flips to 0a 00 00 01 - r1 ip - 10.0.0.1
          uint32_t r2Ip = 33554442;//hex 02 00 00 0a flips to 20 00 00 a0
                                 //flips to 0a 00 00 02 - r2 ip - 10.0.0.2

        
        
                
       
        
        
          if(strcmp(routerAddress, r1mac.sll_addr)==0){//we are router 1
            memcpy(&arpRequest[28], &r1Ip, 4);//sender ip address
            if(strcmp(str, "10.1.0") == 0){//foreward to h1
              send(eth1_socket, arpRequest, 42, 0);
            }
            if(strcmp(str, "10.1.1") == 0){//foreward to h2
              send(eth2_socket, arpRequest, 42, 0);
        
            }
            if(strcmp(str, "10.3.0") == 0 || strcmp(str, "10.3.1") == 0 || strcmp(str, "10.3.4") == 0){//foreward to r2 
              memcpy(&arpRequest[38], &r2Ip, 4);//target ip address
              send(r2_socket, arpRequest, 42, 0);
            }
          }
          else if(strcmp(routerAddress, r2mac.sll_addr)==0){//we are router 2
            memcpy(&arpRequest[28], &r2Ip, 4);//sender ip address
            if(strcmp(str, "10.3.0") == 0){//foreward to h3
              send(eth1_socket, arpRequest, 42, 0);
            }
            if(strcmp(str, "10.3.1") == 0){//foreward to h4
              send(eth2_socket, arpRequest, 42, 0);
        
            }
            if(strcmp(str, "10.3.4") == 0){//foreward to h5 
              send(eth3_socket, arpRequest, 42, 0);
            }
            if(strcmp(str, "10.1.0") == 0 || strcmp(str, "10.1.1") == 0){//foreward to r1 
              memcpy(&arpRequest[38], &r1Ip, 4);//target ip address
              send(r1_socket, arpRequest, 42, 0);
            }
        
          }
        }
        free(routerAddress);
        //sprintf(str, "%d.%d.%d", arpRequest[30], arpRequest[31], arpRequest[32]);//target ip

      } 
    }



      /*
      len = 0;
      for(i = 0; i < n; i++){//gets whole buffer
        len+=sprintf(icmpBuf+len,"%02X%s", buf[i],i < (n-1) ? ":":"");
      }
      */
    //}prob for size 42 - placement if I was wrong on this
    
  }
  free(macaddr);
  free(wholeBuf);
  //exit
  return 0;
}
