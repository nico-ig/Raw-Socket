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

// include local
// include local
#include "conexao.h"
#include "crc8.cpp"
#include "crc8.h"
#include "frame.h"
#include "macros.h"

using namespace std;

#define HEX "%02x"

int main(int argc, char *argv[]) {
  gen_crc8_table();

  conexao server("lo");
  string dataReceive;
  int receive = 0;
  string dataSend = "Hello World";
  cout << "server" << server.get_socket() << endl;
  int receiveds = 0;
  while (true) {
    // server.send_data(dataSend, 12, 0x01);
    receive = server.receive_frames(receiveds);
    receiveds++;
    // cout << "receive: " << receive << endl;
    if (receive == FIMT) {
      cout << "------------------ FIMT ------------------\n";
      server.reconstroi_arquivo("recebe_Arquivo");
      break;
    }
    // reconstroi arquivo
  }

  return 0;
}
