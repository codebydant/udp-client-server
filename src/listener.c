// UDP reference: https://jweinst1.medium.com/how-to-use-udp-sockets-on-windows-29e7e60679fe
// UDP reference: https://www.binarytides.com/udp-socket-programming-in-winsock/

//#include <arpa/inet.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
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


int main(int argc,char * argv[]){

  printf("\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("***   LISTENER PROGRAM    ***\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

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
  SOCKET serverSocket = INVALID_SOCKET;
  serverSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  if (serverSocket == INVALID_SOCKET)
  {
    printf("Could not create socket : %d",WSAGetLastError());
    waitInputKey();
  }

  printf("done.\n");

  printf("[DEBUG] Binding socket...");

  struct sockaddr_in serverAddr;

  // The HSS should be bound to port 12778
  // short port = 27015;
  short port = 12777;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if(bind(serverSocket,(SOCKADDR *)& serverAddr, sizeof(serverAddr)))
  {
    printf("bind failed with error %d\n",WSAGetLastError());
    waitInputKey();

  }

  printf("done.\n");

  // printf("Listening data...\n");

  int bytes_received;
  char serverBuf[1025];
  int serverBufLen = 1024;

  struct sockaddr_in SenderAddr;
  int SenderAddrSize = sizeof(SenderAddr);

  //keep listening for data
	while(1)
	{
    printf("\n");
    printf("----------------\n");
		printf("[DEBUG] Waiting for data...");
		fflush(stdout);

    //clear the buffer by filling null, it might have previously received data
		memset(serverBuf,'\0', serverBufLen);

    bytes_received = recvfrom(serverSocket,serverBuf,serverBufLen,0,(SOCKADDR *)& SenderAddr,&SenderAddrSize);
    if (bytes_received == SOCKET_ERROR)
    {
      printf("[DEBUG] recvfrom failed with error %d\n", WSAGetLastError());
      WSACleanup();
      waitInputKey();
    }

    printf("done.\n");

    //print details of the client/peer and the data received
    printf("[DEBUG] Received datagrams from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    printf("[DEBUG] data: %s\n" , serverBuf);

    // serverBuf[bytes_received] = '\0';
    //printf("datagrams received.\n");

    printf("[DEBUG] Sending response to client...");
    // char sendBuf[] = {'h','e','l','l','o','\0'};
    char sendBuf[] = {'O','K','\0'};
    int sendBufLen = (int)(sizeof(sendBuf)-1);
    int sendResult = sendto(serverSocket,sendBuf,sendBufLen,0,(SOCKADDR *) & SenderAddr,SenderAddrSize);

    if (sendResult == SOCKET_ERROR)
    {
      printf("[DEBUG] Sending back response got an error: %d\n", WSAGetLastError());
      WSACleanup();
      waitInputKey();
    }

    printf("done.\n");
    printf("----------------\n");

    printf("\n");
    printf("LISTENER: Process finished.\n");
    Sleep(1000);

  }

  closesocket(serverSocket);
	WSACleanup();
  waitInputKey();

  return 0;

}
