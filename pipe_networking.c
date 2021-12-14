#include "pipe_networking.h"

#define server_printf(args...) fprintf(stderr, "[SERVER] " args)
#define client_printf(args...) fprintf(stderr, "[CLIENT] " args)

/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
    int from_client = 0;

    char private_pipe[BUFFER_SIZE] = {};
    int bytes_read;

    mkfifo(WKP, 0666);
    server_printf("Created WKP\n");
    from_client = open(WKP, O_RDONLY);
    server_printf("Opened WKP. Waiting for input\n");

    bytes_read = read(from_client, private_pipe, BUFFER_SIZE);
    server_printf("Recieved %d bytes of input from WKP, closing\n", bytes_read);

    remove(WKP);
    server_printf("Removed %s\n", WKP);

    *to_client = open(private_pipe, O_WRONLY);
    server_printf("Opened Pipe [%s]\n", private_pipe);

    write(*to_client, ACK, sizeof(ACK));
    server_printf("Sent ACK\n");

    return from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
    int from_server = 0;
    char private_pipe[BUFFER_SIZE] = {}, ack[HANDSHAKE_BUFFER_SIZE];
    sprintf(private_pipe, "%d", getpid());

    *to_server = open(WKP, O_WRONLY);
    client_printf("Connected to WKP\n");
  
    mkfifo(private_pipe, 0666);
    client_printf("Created Private Pipe [%s]\n", private_pipe);

    write(*to_server, private_pipe, BUFFER_SIZE);
    client_printf("Wrote Data to WKP\n");

    from_server = open(private_pipe, O_RDONLY);
    client_printf("Opened Private Pipe [%s]\n", private_pipe);

    remove(private_pipe);
    client_printf("Removed %s\n", private_pipe);

    client_printf("Waiting for ACK from Server...\n");
    read(from_server, ack, HANDSHAKE_BUFFER_SIZE);
    client_printf("Recieved ACK [%s]\n", ack);

    write(*to_server, ACK, sizeof(ACK));
    client_printf("Sent ACK\n");

    return from_server;
}