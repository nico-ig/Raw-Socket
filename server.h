// Receive all the data
#ifndef _SERVER_
#define _SERVER_

#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
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

#include <arpa/inet.h>

// include local
#include "conexao.h"
#include "crc8.h"
#include "frame.h"
#include <mutex>
std::mutex mtx;

using namespace std;

class server {

private:
  int soquete;
  vector<frame *> framesMidia;
  conexao *local;
  conexao *target;

public:
  server(conexao *local, conexao *target);
  // ~server();
  void run();
};

void server::run() {
  // lock the thread with mutex
  int i;
  while (true) {
    frame fReceive;
    cout << "Recebendo frame ---------------\n";

    /*-- listening local ip and wainting for messages --*/
    fReceive = *local->receive_frame(true);
    fReceive.imprime(DEC);

    /*-- sending ack to target  --*/
    frame *ack = new frame();
    ack->set_tipo(ACK);
    ack->set_dado(fReceive.get_dado());
    ack->set_seq(fReceive.get_seq());
    cout << "Enviando ACK ---------------\n";
  sendack:
    int ackSent = 0;
    ackSent = target->send_frame(ack);
    cout << "ACK enviado: " << ackSent << endl;
    // reconstroi arquivoY
  }
}

server::server(conexao *localConnection, conexao *targetConnection) {
  local = localConnection;
  target = targetConnection;
}

#endif
