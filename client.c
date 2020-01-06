#include "client.h"

struct config config;



int main(void)
{
  char* temp = malloc(SIZE_LINE_MAX);
  config.control_fd = 0;
  config.debug = 0;
  config.passive = 0;
  config.portdata = PORTDATA;

  printf("FTP client\n");
  printf("type \"help\" for manual\n");

  do
  {
    printf("ftp> ");
    fgets(temp, SIZE_LINE_MAX, stdin);
    temp = strtok(temp," ");
    if( temp == NULL)
    {
      printf("Error Parsing, Exiting...");
      exit(0);
    }
    exec_command(temp);
  } while(strncmp(temp,"exit",4));

  free(temp);
  return 0;
}


void exec_command(char* choice)
{
  if     ( !strncmp( choice, "help\n",      SIZE_LINE_MAX))
    print_help();

  else if( !strncmp( choice, "show",        SIZE_LINE_MAX))
    exec_show(strtok(NULL,"\n"));

  else if( !strncmp( choice, "ren",        SIZE_LINE_MAX))
    exec_ren(strtok(NULL,"\n"));

  else if( !strncmp( choice, "del",        SIZE_LINE_MAX))
    exec_del(strtok(NULL,"\n"));

  else if( !strncmp( choice, "cd",        SIZE_LINE_MAX))
    exec_cd(strtok(NULL,"\n"));

  else if( !strncmp( choice, "mkd",        SIZE_LINE_MAX))
    exec_mkd(strtok(NULL,"\n"));

  else if( !strncmp( choice, "rmd",        SIZE_LINE_MAX))
    exec_rmd(strtok(NULL,"\n"));

  else if( !strncmp( choice, "open",        SIZE_LINE_MAX))
    exec_open(strtok(NULL,"\n"));

  else if( !strncmp( choice, "dir\n",       SIZE_LINE_MAX))
    exec_dir();

  else if( !strncmp( choice, "ciao\n",      SIZE_LINE_MAX))
    exec_ciao();

  else if( !strncmp( choice, "passiveon\n", SIZE_LINE_MAX))
    set_passive(1);

  else if( !strncmp( choice, "passiveoff\n",SIZE_LINE_MAX))
    set_passive(0);

  else if( !strncmp( choice, "debugon\n",   SIZE_LINE_MAX))
    set_debug(1);

  else if( !strncmp( choice, "debugoff\n",  SIZE_LINE_MAX))
    set_debug(0);

  else if( !strncmp( choice, "exit\n",      SIZE_LINE_MAX))
    printf("Exiting...\n");
}


void set_passive(int state)
{
  if(state)
  {
    config.passive = 1;
    printf("Switched to passive mode\n");
  }

  else
  {
    config.passive = 0;
    printf("Switched to active mode\n");
  }
}


void set_debug(int state)
{
  if(state)
  {
    config.debug = 1;
    printf("Debug mode activated\n");
  }

  else
  {
    config.debug = 0;
    printf("Debug mode deactivated\n");
  }
}


void exec_open(char* param)
{
  if(config.control_fd)
  {
    printf("Connection déjà existante !\n");
    return;
  }

  struct sockaddr_in serv_address;
  int client_fd;
  int read_char;
  char* buffer = malloc(SIZE_LINE_MAX); // buffer de reception de message
  char* response; // buffer d'envoi de message

  // Crée le file descriptor
  if( (client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      exit(EXIT_FAILURE);
  }

  //Assigne des options au file descriptor
  serv_address.sin_family = AF_INET;
  serv_address.sin_port = htons(PORTCONT);

  if( inet_pton(AF_INET, param, &serv_address.sin_addr) <= 0)
  {
      printf("\nInvalid address/ Address not supported \n");
      exit(EXIT_FAILURE);
  }

  //printf("Connection...\n");
  if( connect(client_fd, (struct sockaddr *)&serv_address, sizeof(serv_address)) < 0)
  {
      printf("\nConnection Failed \n");
      exit(EXIT_FAILURE);
  }



  read_char = read( client_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char]='\0';
  if( config.debug)
    printf("%s ", buffer );

  buffer = strtok(buffer," ");

  if(   buffer[0] == '2'
     && buffer[1] == '2'
     && buffer[2] == '0')
  {
    printf("Enter login and password \n");
    printf("Enter login : ");
    fgets( buffer, SIZE_LINE_MAX, stdin);

    /* response pour le login */
    response = malloc( strlen("USER") + strlen(buffer) + 1);
    response[0] = '\0';   // ensures the memory is an empty string
    strcat(response, "USER ");
    strcat(response, buffer);
    printf("--->%s", response);

    send(client_fd , response , strlen(response) , 0);
    free(response);

    read_char = read( client_fd , buffer, SIZE_LINE_MAX);
    buffer[read_char]='\0';
    if( config.debug)
      printf("%s ", buffer );

    if(buffer[0] == '3')
    {
      printf("Enter password : ");
      fgets(buffer, SIZE_LINE_MAX, stdin);

      /* response pour le mot de passe */
      response = malloc( strlen("PASS") + strlen(buffer) + 1);
      response[0] = '\0';   // ensures the memory is an empty string
      strcat(response, "PASS ");
      strcat(response, buffer);
      printf("--->%s", response);

      send(client_fd, response, strlen(response), 0);
      free(response);

      read_char = read( client_fd , buffer, SIZE_LINE_MAX);
      buffer[read_char] = '\0';
      if(config.debug)
        printf("%s ",buffer );

      if(   buffer[0] == '2'
         && buffer[1] == '3')
      {
        config.control_fd = client_fd; // Enregistre le fd du socket dans la configuration
        return;
      }
    }
    else
      printf("Erreur de login \n");
  }
  else
    printf("Erreur de connection au serveur FTP \n");
  return;
}


void exec_ciao()
{
  if( !config.control_fd)
  {
    printf("Not connected !\n");
    return;
  }
  char* buffer = malloc(SIZE_LINE_MAX);
  char* response = "QUIT\n";
  int read_char;
  printf("--->%s",response);

  send( config.control_fd, response , strlen(response), 0);

  read_char = read( config.control_fd, buffer, SIZE_LINE_MAX);
  buffer[read_char] = '\0';
  if( config.debug)
    printf("%s\n",buffer );

  config.control_fd = 0;
}


int set_serv_passive(char* cmd)
{
  struct sockaddr_in serv_address;
  int client_fd;
  int read_char;
  int port_data = 0;
  char* ip_data  = malloc(SIZE_LINE_MAX);
  char* buffer   = malloc(SIZE_LINE_MAX); // buffer de reception de message
  char* response = malloc(SIZE_LINE_MAX); // buffer d'envoi de message
  char** ip_data_part = malloc(4);

  // Crée le file descriptor
  if( (client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      return 0;
  }

  response[0] = '\0';
  strcat(response,"PASV\n");
  printf("--->%s",response);

  send(config.control_fd, response, strlen(response), 0);

  read_char = read(config.control_fd, buffer, SIZE_LINE_MAX);
  buffer[read_char+1] = '\0';

  if(config.debug)
    printf("%s\n",buffer );

  buffer = strtok(buffer, "(");

  for( int i = 3; i >= 0; i--)
  {
    ip_data_part[i] = malloc(IP_OCTET_CHAR);
    ip_data_part[i][0]='\0';
    strcat(ip_data_part[i],strtok(NULL,","));
  }

  sprintf(ip_data,"%s.%s.%s.%s",ip_data_part[3],ip_data_part[2],ip_data_part[1],ip_data_part[0]);
  port_data += (atoi(strtok(NULL,","))*256);
  port_data += (atoi(strtok(NULL,")")));

  //Assigne des options au file descriptor
  serv_address.sin_family = AF_INET;
  serv_address.sin_port = htons(port_data);


  if( inet_pton(AF_INET, ip_data, &serv_address.sin_addr) <= 0)
  {
      printf("\nInvalid address/ Address not supported \n");
      return;
  }

  //printf("Connection...\n");
  if( connect(client_fd, (struct sockaddr *)&serv_address, sizeof(serv_address)) < 0)
  {
      printf("\nConnection Failed \n");
      return 0;
  }

  response[0] = '\0';
  sprintf(response,"%s",cmd);
  printf("--->%s",response);
  send(config.control_fd, response, strlen(response), 0);

  return client_fd;
}


int set_serv_active(char* cmd)
{
  int read_char;
  int server_fd, temp_fd;
  int opt = 1;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char* response = malloc(SIZE_LINE_MAX);
  char* buffer = malloc(SIZE_LINE_MAX);

  // Creating socket file descriptor
  if( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      return 0;
  }

  // Forcefully attaching socket to the port 8080
  if( setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
      perror("setsockopt");
      return 0;
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(config.portdata);

  // Forcefully attaching socket to the port 8080
  if( bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
  {
      perror("bind failed");
      return 0;
  }

  if( listen(server_fd, 3) < 0)
  {
        perror("listen");
        return 0;
  }

  response[0] = '\0';
  strcat(response, "PORT 127,0,0,1,");
  char* temp = malloc(SIZE_LINE_MAX);
  sprintf(temp,"%d,",(config.portdata>>8)&0x00ff);
  strcat(response,temp);
  sprintf(temp,"%d\n",(config.portdata)&0x00ff);
  strcat(response,temp);
  printf("--->%s\n",response);
  send(config.control_fd , response , strlen(response) , 0 );

  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char] = '\0';
  if(config.debug)
    printf("%s\n",buffer );

  response[0] = '\0';
  strcat(response, cmd);
  send(config.control_fd, response, strlen(response), 0 );

  if( (temp_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
  {
      perror("accept");
      return 0;
  }
  config.portdata++;

  return temp_fd;
}


int set_serv(char* cmd)
{
  int temp_fd;
  char* response = malloc(SIZE_LINE_MAX);
  response[0] = '\0';

  if(config.passive)
    temp_fd = set_serv_passive(cmd);
  else
    temp_fd = set_serv_active(cmd);

  return temp_fd;
}


void exec_dir()
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);
  char filePart;
  int read_char;
  clock_t start, end;
  double cpu_time_used;
  int i = 0;
  int ftp_fd = set_serv("LIST\n");

  if(ftp_fd == 0)
    return;

  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char] = '\0';
  if(config.debug)
    printf("%s\n",buffer );

  start = clock();
  while( (read_char = read(ftp_fd , &filePart, 1)) == 1)
  {
    printf("%c",filePart);
    i++;
  }
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Transfered %d bytes in %.2f second(s)\n", i, cpu_time_used);

  read_char = read(config.control_fd , buffer, SIZE_LINE_MAX);
  buffer[read_char] = '\0';
  close(ftp_fd);
  if(config.debug)
    printf("%s\n",buffer );
}


void exec_show(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);
  char filePart;
  int read_char;
  clock_t start, end;
  double cpu_time_used;
  int i = 0;
  buffer[0] = '\0';
  strcat(buffer,"RETR ");
  strcat(buffer,param);
  strcat(buffer,"\n");

  int ftp_fd = set_serv(buffer);
  if(ftp_fd == 0)
    return;

  read_char = read( config.control_fd, buffer, SIZE_LINE_MAX);
  buffer[read_char]='\0';
  if(config.debug)
    printf("%s\n",buffer );

  start = clock();
  while( (read_char = read(ftp_fd , &filePart, 1)) == 1)
  {
    printf("%c",filePart);
    i++;
  }
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Transfered %d bytes in %.2f second(s)\n",i,cpu_time_used);

  read_char = read(config.control_fd, buffer, SIZE_LINE_MAX);
  buffer[read_char] = '\0';
  close(ftp_fd);
  if(config.debug)
    printf("%s\n",buffer );
}


void exec_ren(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);

  buffer[0] = '\0';
  strcat(buffer, "RNFR ");
  param = strtok(param, " ");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  buffer[0] = '\0';
  strcat(buffer, "RNTO ");
  param = strtok(NULL, "\n");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  /* reponse serveur */
  char* response = malloc(SIZE_LINE_MAX);
  int read_char;
  read_char = read( config.control_fd, response, SIZE_LINE_MAX);
  response[read_char]='\0';

  if(config.debug)
    printf("%s\n",response );
}


void exec_del(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);

  buffer[0] = '\0';
  strcat(buffer, "RNFR ");
  param = strtok(param, " ");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  buffer[0] = '\0';
  strcat(buffer, "RNTO ");
  param = strtok(NULL, "\n");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  /* reponse serveur */
  char* response = malloc(SIZE_LINE_MAX);
  int read_char;
  read_char = read( config.control_fd, response, SIZE_LINE_MAX);
  response[read_char]='\0';

  if(config.debug)
    printf("%s\n",response );
}


void exec_cd(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);

  if(   param[0] == '.'
     && param[1] == '.') // cd ..
  {
    buffer[0] = '\0';
    strcat(buffer, "CDUP\n");

    if(config.debug)
      printf("%s\n",buffer );

    send(config.control_fd, buffer, strlen(buffer), 0);
  }

  else    // cd <rep>
  {
    buffer[0] = '\0';
    strcat(buffer, "CWD ");

    param = strtok(param, " ");
    strcat(buffer, param);
    strcat(buffer, "\n");

    if(config.debug)
      printf("%s\n",buffer );

    send(config.control_fd, buffer, strlen(buffer), 0);
  }

  /* reponse serveur */
  char* response = malloc(SIZE_LINE_MAX);
  int read_char;
  read_char = read( config.control_fd, response, SIZE_LINE_MAX);
  response[read_char]='\0';

  if(config.debug)
    printf("%s\n",response );

}


void exec_mkd(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);

  buffer[0] = '\0';
  strcat(buffer, "MKD ");

  param = strtok(param, " ");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  /* reponse serveur */
  char* response = malloc(SIZE_LINE_MAX);
  int read_char;
  read_char = read( config.control_fd, response, SIZE_LINE_MAX);
  response[read_char]='\0';

  if(config.debug)
    printf("%s\n",response );
}


void exec_rmd(char* param)
{
  if( !config.control_fd)
  {
    printf("Not Connected\n");
    return;
  }

  char* buffer = malloc(SIZE_LINE_MAX);

  buffer[0] = '\0';
  strcat(buffer, "RMD ");

  param = strtok(param, " ");
  strcat(buffer, param);
  strcat(buffer, "\n");

  if(config.debug)
    printf("%s\n",buffer );

  send(config.control_fd, buffer, strlen(buffer), 0);

  /* reponse serveur */
  char* response = malloc(SIZE_LINE_MAX);
  int read_char;
  read_char = read( config.control_fd, response, SIZE_LINE_MAX);
  response[read_char]='\0';

  if(config.debug)
    printf("%s\n",response );
}


void print_help()
{
  printf("List of command :\n");
  printf("open <@IP>           : Connect to a FTP server at IP\n");
  printf("dir                  : Show the list of file in current dir\n");
  printf("debug(on|off)        : Activate or deactivate the printing of server response\n");
  printf("passive(on|off)      : Activate or deactivate passive mode of FTP\n");
  printf("show <FILE>          : Show the FILE content\n");
  printf("ren <file1> <file2>  : Rename file\n");
  printf("del <file>           : Delete file\n");
  printf("cd <rep>             : move to distant directory\n");
  printf("cd ..                : move to parent directory\n");
  printf("mkd <rep>            : create distant directory\n");
  printf("rmd <rep>            : delete distant directory\n");
  //printf("get <FILE>     : Show the FILE content\n");
  //printf("send <FILE>     : Show the FILE content\n");
  printf("ciao                 : Disconnect from server\n");
  printf("exit                 : Exit the program\n");
}
