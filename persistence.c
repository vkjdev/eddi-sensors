#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#include <persistence.h>


#define PORT_NUMBER 3661

int socketFile, newSocketFile;
int rc;
char setString[256];

void error(const char *msg){
  perror(msg);
  exit(1);
}



void persistenceInitialize(){
  struct sockaddr_un serverAddress;

  // make socket file
  socketFile = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socketFile < 0) {
    error("ERROR opening socket");
  }

  // make server address
  bzero((char *) &serverAddress, sizeof(serverAddress));
  serverAddress.sun_family = AF_UNIX;
  strncpy(serverAddress.sun_path, "./data/sensors.sock", sizeof(serverAddress.sun_path) - 1);

  // bind to socket
  if( bind(socketFile, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // listen and bind once the client contacts
  if( listen(socketFile, 5) == -1 ){
    error("ERROR on listen");
  }

  newSocketFile = accept(socketFile, NULL, NULL);
  if( newSocketFile < 0 ){
    error("ERROR on accept");
  }

  // set nosigpip option so the process isn't terminated by sigpipe
  int set = 1;
  setsockopt(newSocketFile, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
}

ssize_t persistSenseSet(struct SenseSet *set){
  sprintf(setString, "%lu|%f|%f|%d|%d|%d\n",
      set->timestamp, set->qOut, set->qDump,
      set->ppmOut, set->ppmIn, set->ppmRec );
  return write(newSocketFile, setString, strlen(setString));
}

void persistenceCleanup(){
  close(newSocketFile);
  close(socketFile);
}
