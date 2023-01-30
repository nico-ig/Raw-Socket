#ifndef _CONEXAO_
#define _CONEXAO_

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
#include "crc8.h"
#include "frame.h"

using namespace std;

#define BYTE "%02x"

class conexao {
private:
  // ------ Dados ------ //

  int soquete;
  vector<frame> framesSending, framesReceiving;
  frame f1, f2; // frame envio e recebimento
  char buffer[sizeof(frame)];
  char bufferSend[sizeof(frame)] = "Hello World ";
  int sequence = -1; // sequencia do último frame recebido
  struct sockaddr_ll endereco;

  // ----- Funçoes ------ //

  int ConexaoRawSocket(char *device);
  void send_frame(char *buffer, int size);    // envia um pacote
  void receive_frame(char *buffer, int size); // recebe um pacote
  void close_connection();                    // fecha a conexao
public:
  // ------ Construtor ------ //
  conexao();
  conexao(char *device);

  // ------ Funcoes ------ //
  // int get_socket();                               // retorna o socket
  // int send_data(char *buffer, int size, UC tipo); // envia dados
  // int receive_data(char *buffer, int size);       // recebe dados
  // void close_conexao();                           // fecha a conexao

  int get_socket() { return soquete; }

  int send_data(string data, int size, UC tipo) {
    int byteSend;
    if (data.size() > BITPOW(6) - 1) {
      // Vector of frames
      vector<frame> frames;
      for (size_t i = 0; i < data.size(); i += BITPOW(6) - 1) {
        frame f;
        f.set_tipo(tipo);
        f.set_seq(i);
        f.set_dado(data.substr(i, BITPOW(6) - 1));
        frames.push_back(f);
      }
      for (size_t i = 0; i < frames.size(); i++) {
        // frames[i].imprime();
        memcpy(buffer, &frames[i], sizeof(frame));
        byteSend = send(soquete, buffer, sizeof(frame), 0);
        if (byteSend < 0) {
          printf("Erro no sendto %d\n", byteSend);
          exit(-1);
        }
      }
    } else {
      f1.set_tipo(0x10);
      f1.set_seq(0x01);
      f1.set_dado(data);
      char bufferSend[1024];
      memcpy(bufferSend, &f1, sizeof(frame));
      cout << "--------------------------------------------\n";
      cout << "Enviando frame: " << bufferSend << "\n";
      cout << "Frame: -----------------------------------------\n";
      f1.imprime(DEC);

      byteSend = send(soquete, bufferSend, 1024 - 1, 0);
      if (byteSend < 0) {
        printf("Erro no sendto %d\n", byteSend);
        exit(-1);
      }
    }
    return byteSend;
  };

  int receive_frames(int index) {
    char buffer[sizeof(frame)];
    memset(buffer, 0, sizeof(buffer));

    int byteRecv;
    byteRecv = recv(soquete, buffer, sizeof(frame), 0);
    if (byteRecv <= 0) {
      printf("Erro no recvfrom %d\n", byteRecv);
      exit(-1);
    }
    frame f;
    memcpy(&f, buffer, sizeof(frame));
    if (index <= 5) {
      cout << "--------------------------------------------\n";
      cout << "Recebido Frame: " << index << "\n" << buffer << "\n";
      cout << "Frame:--------------------------------------------\n";
      f.imprime(DEC);
      cout << "framesReceiving.size(): " << framesReceiving.size() << "\n";
    }
    
    if (f.get_seq() == sequence)
      return f.get_tipo();

    sequence = f.get_seq();
    framesReceiving.push_back(f);
    return f.get_tipo();
  };

  int reconstroi_arquivo(string nomeArquivo) {
    cout << "------------Reconstroi arquivo------------------\n";
    cout << "Tamanho do vetor: " << framesReceiving.size() << "\n";
    string fileData;
    for (size_t i = 0; i < framesReceiving.size() - 1; i++) {
      fileData += string(framesReceiving[i].get_dado(), 63);
    }
    cout << "Tamanho do arquivo: " << fileData.size() << "\n";
    ofstream file;
    file.open("./" + nomeArquivo, ios::binary);
    file << fileData;
    file.close();

    framesReceiving.clear();

    return 0;
  };

  int send_frames(vector<frame> frames) {
    int byteSend;
    char buffer[sizeof(frame)];
    cout << "Tamanho do vetor: " << frames.size() << "\n";
    int i = 0;
    for (i = 0; i < frames.size(); i++) {
      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, &frames[i], sizeof(frame));

      frame f;
      memcpy(&f, buffer, sizeof(frame));
      byteSend = send(soquete, buffer, sizeof(frame), 0);
      if (byteSend < 0) {
        printf("Erro no sendto %d\n", byteSend);
      }
      if (i <= 5) {
        cout << "--------------------------------------------\n";
        cout << "Enviando frame: " << i << "\n" << buffer << "\n";
        cout << "Frame: -----------------------------------------\n";
        f.imprime(DEC);
      }
    }
    cout << "--------------------------------------------\n";
    cout << "Enviado " << i << " frames\n";
    // send end of file
    frame f;
    f.set_tipo(FIMT);
    f.set_seq(0);
    f.set_dado("FIM");
    memcpy(buffer, &f, sizeof(frame));
    cout << "--------------------------------------------\n";
    cout << "Enviando FIM: " << buffer << "\n";
    cout << "Frame: -----------------------------------------\n";
    f.imprime(DEC);

    byteSend = send(soquete, buffer, sizeof(frame), 0);
    if (byteSend < 0) {
      printf("Erro no sendto FIMT %d\n", byteSend);
      exit(-1);
    }

    return byteSend;
  }
  int send_file(string location) {
    cout << "Location: " << location.c_str() << "\n";
    ifstream file;
    file.open(location.c_str(), ios::binary);

    if (!file) {
      cout << "Erro ao abrir o arquivo1\n";
      ofstream myfile;
      myfile.open("example.txt");
      myfile << "Writing this to a file.\n";
      myfile.close();
      return 0;
      // return -1;
    } else {
      cout << "Arquivo aberto com sucesso\n";
    }

    char *buffer = new char[63];
    vector<vector<char>> fileBuffer;
    int byteRead = 0;
    // file.read(buffer, 63);
    while (file.read(buffer, 63) || file.gcount()) {
      char chunk[63];
      memcpy(chunk, buffer, 63);
      fileBuffer.push_back(vector<char>(chunk, chunk + sizeof(chunk)));
      byteRead += 63;
    }

    // create frames from file chunks
    for (size_t i = 0; i < fileBuffer.size(); i++) {
      f1.set_tipo(MIDIA);
      f1.set_seq(i);
      f1.set_dado(string(fileBuffer[i].begin(), fileBuffer[i].end()));
      framesSending.push_back(f1);
    }
    // reconstroi arquivo local
    //  reconstruct file
    string fileData;
    for (size_t i = 0; i < fileBuffer.size(); i++) {
      // cout << "------- Chunk - " << i << ":\n" << fileBuffer[i] << "\n";
      fileData += string(fileBuffer[i].begin(), fileBuffer[i].end());
    }
    // create file
    ofstream file2;
    file2.open("fileLocal", ios::binary);
    file2 << fileData;
    file2.close();

    // fwrite(fileData.c_str(), 1, fileData.size(), file2);
    // fclose(file2);
    cout << "Arquivo enviado com sucesso\n";
    cout << "Tamanho do arquivo: " << fileData.length() << "\n";
    cout << "Tamanho frames: " << framesSending.size() << "\n";
    // reconstruct with framesSending

    string Data;
    for (size_t i = 0; i < framesSending.size(); i++) {
      string Example = string(fileBuffer[i].begin(), fileBuffer[i].end());
      // cout << "Example: " << Example << "\n";
      Data += string(framesSending[i].get_dado(), 63);
    }

    // create file
    ofstream file3;
    file3.open("file_frames", ios::binary);
    file3 << Data;
    file3.close();

    // reconstroi_arquivo("LocalFunction");
    return send_frames(framesSending);

    // fileBuffer.push_back(buffer);

    return 1;

    // return send_data(data, size, tipo);
  };

  int send_message(string data, int size, UC tipo) {
    switch (tipo) {
      {
      case MIDIA:
        return send_file(data);
        break;

      default:
        break;
      }
    };
  };

  int receive_data(string buffer, int size) {
    int byteRecv;
    frame f_recebido;
    char *buffer2 = new char[size];
    byteRecv = recv(soquete, buffer2, size, 0);
    if (byteRecv < 0) {
      printf("Erro no recvfrom %d\n", byteRecv);
      exit(-1);
    }
    // cout << "--------------------------------------------\n";
    // cout << "Recebendo frame: " << buffer2 << "\n"
    //  << "Size: " << byteRecv << "--" << sizeof(frame) << "\n";
    memcpy(&f_recebido, buffer2, sizeof(frame));
    framesReceiving.push_back(f_recebido);
    // cout << "Frame: -----------------------------------------\n";
    // f_recebido.imprime(DEC);
    // cout << "--------------------------------------------\n";
    // cout << "frame.dado: " << f_recebido.get_dado() << "\n";

    return byteRecv;
  }
};

/* --------------------- PRIVATE --------------------- */

int conexao::ConexaoRawSocket(char *device) {
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq)); /*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }

  memset(&endereco, 0, sizeof(endereco)); /*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }

  memset(&mr, 0, sizeof(mr)); /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) ==
      -1) {
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
};

/* --------------------- PUBLIC --------------------- */

conexao::conexao(char *device) { soquete = ConexaoRawSocket(device); }

// conexao::sendFrame(char *buffer, int size) {
//   int byteSend;
//   byteSend = send(soquete, buffer, size, 0);
//   if (byteSend < 0) {
//     printf("Erro no sendto %d\n", byteSend);
//     exit(-1);
//   }
// }g

// int main(int argc, char *argv[]) {
//   int soquete;
//   char buffer[1024];
//   char bufferSend[1024] = "Hello World ";
//   // void *__restrict__ __buffer2;
//   // struct sockaddr_ll endereco;
//   // socklen_t tamanho = sizeof(endereco);

//   soquete = ConexaoRawSocket("lo"); /*cria socket*/

//   int byteRead, byteSend;
//   int counter = 0;
//   while (counter++ < 5) {
//     strcat(bufferSend, to_string(counter).c_str());
//     byteSend = send(soquete, bufferSend, 1024 - 1, 0);
//     byteRead = recv(soquete, buffer, 1024 - 1, 0);

//     // send data to socket

//     if (byteRead < 0) {
//       printf("Erro no recvfrom %d\n", byteRead);
//       exit(-1);
//     }
//     buffer[byteRead] = 0;
//     // c++ print every byte in buffer

//     printf("recebido %d Bytes: -> %s \n", byteRead, buffer);
//     for (int i = 0; i < byteRead; i++) {
//       if ((unsigned int)buffer[i] > 0 && (unsigned int)buffer[i] < 126) {
//         printf("%02x", (unsigned int)buffer[i]);

//         /* code */
//       }
//     }
//     printf("\n");
//     printf("-------------------\n");
//   }
//   return 0;
// };

#endif