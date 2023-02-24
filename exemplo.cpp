#include <bits/stdc++.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

// include local
#include "conexao.h"
#include "crc8.h"
#include "frame.h"
#include "macros.h"
#include "server.h"
#include "client.h"

using namespace std;

#define HEX "%02x"

typedef enum { CLIENT, SERVER } STATUS_E;

int get_status( char *argv )
{
  if ( !strcmp( argv, "client") ) { return CLIENT; } 
  if ( !strcmp( argv, "server") ) { return SERVER; } 
  else                           { return -1; }
}

int main(int argc, char *argv[]) {

  gen_crc8_table();
  conexao socket((char *)"lo");

  int status = get_status(argv[1]);
  switch ( status )
  {
    case CLIENT:
    {
      client cliente(&socket);
      cliente.run();
    }
      break;
    
    case SERVER:
    {
      server servidor(&socket);
      servidor.run();
    }
      break;

    default:
      cout << "Comando invalido\n";
      break;
  }   



//  thread clientSend(&client::run, &cliente);

//  server servidor(&local, &target);
//  thread serverReceive(&server::run, &servidor);

//  int receive = 0;
//  while (true) {
//    receive++;
//  }

// serverReceive.join();
//  clientSend.join();

  return 0;
}

