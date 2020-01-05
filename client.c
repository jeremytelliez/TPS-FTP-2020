#include "client.h"

struct config config;

int main(void) {
  char* temp = malloc(SIZE_LINE_MAX);
  config.control_fd = 0;
  config.debug = 0;
  config.passive = 0;
  printf("FTP client\n");
  printf("type \"help\" for manual :\n");

  do {
    printf("ftp>");
    fgets(temp,SIZE_LINE_MAX,stdin);
    temp = strtok(temp," ");
    if(temp == NULL){
      printf("Error Parsing, Exiting...");
      exit(0);
    }
    exec_command(temp);
  } while(strncmp(temp,"exit",4));
  free(temp);
  return 0;
}

void exec_command(char* choice){
  if(!strncmp(choice,"help\n",SIZE_LINE_MAX))
    print_help();
  else if(!strncmp(choice,"show",SIZE_LINE_MAX))
    exec_show(strtok(NULL,"\n"));
  else if(!strncmp(choice,"open",SIZE_LINE_MAX))
    exec_open(strtok(NULL,"\n"));
  else if(!strncmp(choice,"dir\n",SIZE_LINE_MAX))
    exec_dir();
  else if(!strncmp(choice,"ciao\n",SIZE_LINE_MAX))
    exec_ciao();
  else if(!strncmp(choice,"exit\n",SIZE_LINE_MAX))
    printf("Exiting...\n");
}

void exec_open(char* param){
  if(config.control_fd){
    printf("Connection déjà existante !\n");
    return;
  }
  struct sockaddr_in serv_address;
  int client_fd;
  char* buffer = malloc(SIZE_LINE_MAX); // buffer de reception de message
  char* response; // buffer d'envoi de message

  // Crée le file descriptor
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      exit(EXIT_FAILURE);
  }

  //Assigne des options au file descriptor
  serv_address.sin_family = AF_INET;
  serv_address.sin_port = htons(PORTCONT);

  puts(param);

  if(inet_pton(AF_INET, param, &serv_address.sin_addr)<=0)
  {
      printf("\nInvalid address/ Address not supported \n");
      exit(EXIT_FAILURE);
  }

  //printf("Connection...\n");
  if (connect(client_fd, (struct sockaddr *)&serv_address, sizeof(serv_address)) < 0)
  {
      printf("\nConnection Failed \n");
      exit(EXIT_FAILURE);
  }
  //printf("Connected\n");
  read( client_fd , buffer, SIZE_LINE_MAX);
  buffer = strtok(buffer," ");
  printf("%s ",buffer );
  if(buffer[0] == '2' && buffer[1] == '2' && buffer[2] == '0'){
    printf("Enter login and password \n");
    printf("Enter login : ");
    fgets(buffer,SIZE_LINE_MAX,stdin);

    /* response pour le login */
    response = malloc(strlen("USER")+strlen(buffer)+1);
    response[0] = '\0';   // ensures the memory is an empty string
    strcat(response,"USER ");
    strcat(response,buffer);
    printf("--->%s",response);
    send(client_fd , response , strlen(response) , 0 );
    free(response);

    read( client_fd , buffer, SIZE_LINE_MAX);
    printf("%s ",buffer );
    if(buffer[0] == '3'){
      printf("Enter password : ");
      fgets(buffer,SIZE_LINE_MAX,stdin);

      /* response pour le mot de passe */
      response = malloc(strlen("PASS")+strlen(buffer)+1);
      response[0] = '\0';   // ensures the memory is an empty string
      strcat(response,"PASS ");
      strcat(response,buffer);
      printf("--->%s",response);
      send(client_fd , response , strlen(response) , 0 );
      free(response);
      read( client_fd , buffer, SIZE_LINE_MAX);
      printf("%s ",buffer );

      if(buffer[0] == '2' && buffer[1]== '3'){
        // Enregistre le fd du socket dans la configuration
        config.control_fd = client_fd;
        return;
      }
    } else {
      printf("Erreur de login \n");
    }
  } else {
    printf("Erreur de connection au serveur FTP \n");
  }
  return;
}

void exec_ciao(){
  char* buffer = malloc(SIZE_LINE_MAX);
  char* response = "QUIT\n";
  int read_char;
  printf("--->%s",response);
  send(config.control_fd , response , strlen(response) , 0 );
  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char+1]='\0';
  printf("%s\n",buffer);
}

int set_serv(char* cmd){
  int read_char;
  int server_fd, temp_fd;
  int opt = 1;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char* response = malloc(SIZE_LINE_MAX);
  response[0] = '\0';
  char* buffer = malloc(SIZE_LINE_MAX);

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      exit(EXIT_FAILURE);
  }
  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
  {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORTDATA);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
  {
      perror("bind failed");
      exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0)
  {
        perror("listen");
        exit(EXIT_FAILURE);
  }

  strcat(response,"PORT 127,0,0,1,");
  char* temp = malloc(SIZE_LINE_MAX);
  sprintf(temp,"%d,",(PORTDATA>>8)&0x00ff);
  strcat(response,temp);
  sprintf(temp,"%d\n",(PORTDATA)&0x00ff);
  strcat(response,temp);
  printf("%s\n",response);
  send(config.control_fd , response , strlen(response) , 0 );


  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char+1]='\0';
  printf("%s\n",buffer);

  response[0]='\0';
  strcat(response, cmd);
  send(config.control_fd , response , strlen(response) , 0 );

  if ((temp_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
  {
      perror("accept");
      exit(EXIT_FAILURE);
  }
  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  printf("%s\n",buffer);

  return temp_fd;
}

void exec_dir(){

  if(!config.control_fd){
    printf("Not Connected\n");
    return;
  }
  char* buffer = malloc(SIZE_LINE_MAX);
  char* filePart = malloc(MAX_FILE_PART_SIZE);
  int read_char;
  int ftp_fd = set_serv("LIST\n");

  do {
    read_char = read(ftp_fd , filePart, MAX_FILE_PART_SIZE);
    filePart[read_char]='\0';
    printf("%s",filePart);
  } while(read_char >= MAX_FILE_PART_SIZE);

  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char+1]='\0';
  printf("%s\n",buffer);
}

void exec_show(char* param){
  if(!config.control_fd){
    printf("Not Connected\n");
    return;
  }
  char* buffer = malloc(SIZE_LINE_MAX);
  char* filePart = malloc(MAX_FILE_PART_SIZE);
  int read_char;
  buffer[0]='\0';
  strcat(buffer,"RETR ");
  strcat(buffer,param);
  strcat(buffer,"\n");
  int ftp_fd = set_serv(buffer);

  do {
    read_char = read(ftp_fd , filePart, MAX_FILE_PART_SIZE);
    filePart[read_char]='\0';
    printf("%s",filePart);
  } while(read_char >= MAX_FILE_PART_SIZE-1);

  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char+1]='\0';
  printf("%s\n",buffer);
}

void print_help(){
  printf("List of command :\n");
  printf("open <@IP>   : Connect to a FTP server at IP\n");
  printf("dir          : Show the list of file in current dir\n");
  printf("show <FILE>  : Show the FILE content\n");
  printf("ciao         : Disconnect from server\n");
  printf("exit         : Exit the program\n");
}
