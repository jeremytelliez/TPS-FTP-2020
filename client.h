#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SIZE_LINE_MAX 1024
#define MAX_FILE_PART_SIZE 1024
#define PORTDATA 34210
#define PORTCONT 21


void exec_command(char* choice);
void print_help();
void exec_show(char* param);
void exec_open(char* param);
void exec_ciao();
void exec_dir();

struct config{
  int debug;
  int control_fd;
  int passive;
};

#endif // CLIENT_H
