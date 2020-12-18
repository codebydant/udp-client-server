// UDP reference: https://jweinst1.medium.com/how-to-use-udp-sockets-on-windows-29e7e60679fe
// UDP reference: https://www.binarytides.com/udp-socket-programming-in-winsock/

//#include <arpa/inet.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#ifdef __WIN32__
#include <winsock2.h>
#include <Windows.h>
#elif __WIN64__
#include <winsock2.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

int waitInputKey(){
  // Wait for an input to finish execution
  char ch;
  scanf("%c",&ch);
  return 1;
}

// https://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
//! Byte swap unsigned int (big-endian)
uint32_t swap_uint32( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}

//! Byte swap unsigned short
uint16_t swap_uint16( uint16_t val )
{
    return (val << 8) | (val >> 8 );
}

// https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc,char * argv[]){

  printf("\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("***   PUBLISHER PROGRAM   ***\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

  if (argc < 2){
    printf("Usage %s <simulator path>",argv[0]);
    printf("\n");
    exit(0);
  }

  char * filename = argv[1];
  WSADATA wsa;

  printf("[DEBUG] Initialising Winsock...");

  int res = WSAStartup(MAKEWORD(2,2),&wsa);
  if (res != NO_ERROR)
  {
    printf("Failed. Error code : %d",WSAGetLastError());
    waitInputKey();
  }

  printf("done.\n");

  /***
  Socket family: AF_INET (means IPv4)
  Socket type: SOCK_DGRAM (type of packet can receive)
  Socket protocol: IPPROTO_UDP (type of socket)
  ***/
  printf("[DEBUG] Initialising socket...");
  SOCKET sendSocket = INVALID_SOCKET;
  sendSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  if (sendSocket == INVALID_SOCKET)
  {
    printf("Could not create socket : %d",WSAGetLastError());
    waitInputKey();
  }

  printf("done.\n");

	// Create a UDP datagram socket
  printf("[DEBUG] Binding socket...");
  int bytes_received;
  char SendBuf[1024];
  int SendBufLen = (int)(sizeof(SendBuf)-1);
  const char* toSend = "danielpapi";
  strcpy(SendBuf,toSend);

  struct sockaddr_in ClientAddr;
  int clientAddrSize = (int)sizeof(ClientAddr);

  // The message bus router is bound to port 12777
  // short port = 27015;
  short port = 12777;
  const char* local_host = "127.0.0.1";

  ClientAddr.sin_family = AF_INET;
  ClientAddr.sin_port = htons(port);
  ClientAddr.sin_addr.s_addr = inet_addr(local_host);

  printf("done.\n");

  FILE *fp;
  uint16_t buf;
  // uint32_t buf;

  fp = fopen(filename, "r");
  if (fp == NULL){
    printf("Could not open file %s",filename);
    exit(0);
  }

  printf("[DEBUG] reading simulator.txt file...\n");
  int cont = 0;

  /* Seek to the beginning of the file */
  fseek(fp, 0, SEEK_SET);

  //start communication
	// while(1)
	// {
  while (fread(&buf, sizeof(buf), 1, fp) == 1)
  {

      // uint32_t BE_int = swap_uint32(buf);
      uint16_t raw_reading = swap_uint16(buf);
      printf("[DEBUG] simulator data = 0x%04x\n", raw_reading);

      char str_[80];
      int str_len = sprintf(str_, "%d", raw_reading);

      /*
      Keep in mind that laser altimeter readings are generated as unsigned 16-bit
      integers and must be scaled to determine the true height in meters. The laser
      altimiter has a max range of 1000 meters and will produces readings from 0 to
      65535 (UINT16_MAX).
      */
      // int raw_reading_int = atoi(str_);
      float raw_reading_int = atof(str_);
      float height_meters = (raw_reading_int / 65535)*1000;
      printf("[DEBUG] height %.5f meters\n",height_meters);

      float height_centi = height_meters * 1000;

      char height_measurement[50];
      sprintf(height_measurement, "%f", height_centi);

      char header[] = "HEIGHT ";
      char* res = concat(header,height_measurement);

      if (height_meters == 0.0){
        char header[] = "ENGINE_CUTOFF ";
        res = concat(header,height_measurement);
        printf("[DEBUG] ENGINE_CUTOFF\n");
      }

      const char* toSend = res;
      // const char* toSend = "holaa";
      strcpy(SendBuf,toSend);

      // uint8_t MSB_byte  = (uint8_t) (BE_int & 0xFF00);
      // uint8_t LSB_byte  = (uint8_t) (BE_int & 0x00FF);
      // printf("x8 = 0x%02x\n", MSB_byte);
      // printf("x8 = 0x%02x\n", LSB_byte);

    //   if (cont == 5){
    //     break;
    //   }else{
    //     cont += 1;
    //   }
    // }

    // printf("finished.\n");

    printf("[DEBUG] Starting communication...\n");
    printf("\n");
    printf("----------------------\n");
    printf("[DEBUG] Sending a datagram to the server...wait 3 seconds..." );
    Sleep(3000);

    int clientResult = sendto(sendSocket,SendBuf,SendBufLen,0,(SOCKADDR *) & ClientAddr,clientAddrSize);

    if (clientResult == SOCKET_ERROR)
    {
      printf("[DEBUG] Sending back response got an error: %d\n", WSAGetLastError());
      WSACleanup();
      waitInputKey();
    }

    printf("done.\n");
    printf("[DEBUG] data sent: %s\n",SendBuf);
    printf("[DEBUG] Getting response from server...");

    //receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(SendBuf,'\0', SendBufLen);

    //try to receive some data, this is a blocking call
    bytes_received = recvfrom(sendSocket,SendBuf,SendBufLen,0,(SOCKADDR *)& ClientAddr,&clientAddrSize);
    if (bytes_received == SOCKET_ERROR)
    {
      printf("[DEBUG] recvfrom failed with error %d\n", WSAGetLastError());
      WSACleanup();
      waitInputKey();
      break;
    }

    printf("done.\n");
    //print details of the client/peer and the data received
    printf("[DEBUG] Received packet from %s:%d\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
    printf("[DEBUG] response from server: %s\n" , SendBuf);
    printf("----------------------\n");
    printf("\n");

    //clear the buffer by filling null, it might have previously received data
		memset(SendBuf,'\0', SendBufLen);

		// puts(SendBuf);
    // puts("Sending a datagram to the listener...\n");
    printf("PUBLISHER: Process finished.\n");
    Sleep(1000);
    printf("\n");
    // break;

  }

  fclose(fp);
  closesocket(sendSocket);
	WSACleanup();
  waitInputKey();

  return 0;

}
