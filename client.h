#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SIZE_LINE_MAX 1024
#define MAX_FILE_PART_SIZE 1024
#define IP_OCTET_CHAR 4
#define PORTDATA 34211
#define PORTCONT 21


void exec_command(char* choice);
void print_help();

void exec_show(char* param);
void exec_open(char* param);

int set_serv_passive(char* cmd);
int set_serv_active(char* cmd);
int set_serv(char* cmd);

void set_debug(int state);
void set_passive(int state);

void exec_ciao();
void exec_dir();

void exec_get(char* param);
void exec_send(char* param);

void exec_ren(char* param);
void exec_del(char* param);

void exec_cd(char* param);
void exec_mkd(char* param);
void exec_rmd(char* param);

struct config
{
  int debug;
  int control_fd;
  int passive;
  int portdata;
};

#endif // CLIENT_H
