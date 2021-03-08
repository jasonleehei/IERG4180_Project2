#undef UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>   //strlen
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <pthread.h> //for threading , link with lpthread

#define TRUE   1
#define FALSE  0


/* DEFAULT define of server */
#define DEFAULT_BIND_HOSTNAME "IN_ADDR_ANY"
#define DEFAULT_BIND_PORT 4180

int SERVER (int argc, char *argv[]);

void *connection_handler (void *);

static struct option server_setting[] =
        {
                {"lhost", required_argument,    0, 1},
                {"lport", required_argument,    0, 2},
                {"sbufsize", required_argument, 0, 3},
                {"rbufsize", required_argument, 0, 4},
        };

struct client_info {
    int client_no; // client_no

    /*recv form client*/
    char *type[3];
    char *proto[2];
    int pktsize;
    int pktrate;
    int pktnum;
    int sock;
    int udp_sock;
    char *address;
    int port;

    /*for client of send*/
    int send_status;//0 for not connected, 1 for connected, 2 for closed
    char *send_buf;
    int conn_sock;

    /*for client of recv*/
    int status; //0 for not connected, 1 for connected, 2 for closed
    int conn_no;
};

int main (int argc, char *argv[]) {
    SERVER(argc, argv);
    return (0);

}


int SERVER (int argc, char *argv[]) {

    /* parameters of server */
    char *lhost = DEFAULT_BIND_HOSTNAME;
    int lport = DEFAULT_BIND_PORT;
    int rbufsize = 65536;
    int sbufsize = 65536;
    int parameters;


    /* getopt in server */
    while ((parameters = getopt_long_only(argc, argv, "", server_setting, 0)) != -1) {
        switch (parameters) {
            case 1:
                lhost = optarg;
                break;
            case 2:
                lport = atoi(optarg);
                break;
            case 3:
                rbufsize = atoi(optarg);
                break;
            case 4:
                sbufsize = atoi(optarg);
            default:
                printf("Please choose NetProbes' mode!\n"
                       "SEND MODE: -send\n"
                       "RECEIEVE MODE: -recv\n"
                       "SERVER MODE: -server\n"
                       "If[mode] = -send/-recv then the following are the supported parameters :\n"
                       "           < -stat yyy >         update statistics once every yyy ms. (Default = 500 ms)\n"
                       "           < -rhost hostname > send data to host specified by hostname. (Default 'localhost')\n"
                       "           < -rport portnum > send data to remote host at port number portnum. (Default '4180')\n"
                       "           < -proto tcp || udp > send data using TCP or UDP. (Default UDP)\n"
                       "           < -pktsize bsize > send message of bsize bytes. (Default 1000 bytes)\n"
                       "           < -pktrate txrate > send data at a data rate of txrate bytes per second,\n"
                       "                               0 means as fast as possible. (Default 1000 bytes / second\n"
                       "           < -pktnum num > send or receive a total of num messages. (Default = infinite)\n"
                       "           < -sbufsize bsize > set the outgoing socket buffer size to bsize bytes.\n"
                       "           < -rbufsize bsize > set the incoming socket buffer size to bsize bytes.\n"
                       "else if[mode] = -server then\n"
                       "           < -lhost hostname > hostname to bind to. (Default late binding)\n"
                       "           < -lport portnum > port number to bind to. (Default '4180')\n"
                       "           < -rbufsize bsize > set the incoming socket buffer size to bsize bytes.\n"
                       "           < -sbufsize bsize > set the outgoing socket buffer size to bsize bytes.\n"
                       "end if."
                );
                break;
        }
    }


    printf("NetProbe Configurations:\n");
    printf("-lhost = %s\n-lport = %d\n-sbufsize:%d\n-rbufsize:%d\n", lhost, lport, sbufsize, rbufsize);

    int opt = TRUE, client_no = 1;
    int master_socket, udp_master_socket, addrlen, new_socket, udp_new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    int udp_count=0;

    char *server_recv[2000];
    int recv_size;
    char buffer[rbufsize];
    char *buf = (char *) calloc(sizeof(char), 65536);

    //set of socket descriptors
    fd_set readfds;

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) { client_socket[i] = 0; }

    //create a master tcp socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    //create a master udp socket
    if ((udp_master_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(udp_master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(lport);

    //bind tcp
    if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //bind udp
    if (bind(udp_master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //Accept and incoming connection
    puts("Listening to incoming connection request ...\n");
    addrlen = sizeof(address);


    while (TRUE) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        FD_SET(udp_master_socket, &readfds);

        max_sd = master_socket + 1;

        //add child sockets to set
        for (i = 0; i < max_clients; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            //highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        struct client_info client[20]; //for 20 client access once
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) {
            while ((new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen))) {

                //Receive a reply from the server
                if ((recv_size = recv(new_socket, server_recv, 2000, 0)) == 1) {
                    puts("recv failed");
                } else {
                    int count = 0;
                    server_recv[recv_size] = '\0';
                    char *ptr = strtok((char *) server_recv, ",");
                    while (ptr != NULL) {
                        if (count == 0) {
                            sprintf((char *) client[client_no].type, "%s", ptr);
                            //client[client_no].type = ptr;
                            count++;
                        } else if (count == 1) {
                            sprintf((char *) client[client_no].proto, "%s", ptr);
                            //client[client_no].proto = ptr;
                            count++;
                        } else if (count == 2) {
                            client[client_no].pktsize = atoi(ptr);
                            count++;
                        } else if (count == 3) {
                            client[client_no].pktrate = atoi(ptr);
                            count++;
                        } else if (count == 4) {
                            client[client_no].pktnum = atoi(ptr);
                            count++;
                        }
                        ptr = strtok(NULL, ",");

                    }

                    client[client_no].address = inet_ntoa(address.sin_addr);
                    client[client_no].port = ntohs(address.sin_port);
                    client[client_no].send_status = 0;
                    client[client_no].status = 0; //status 0 means client is not connected another client yet
                    client[client_no].client_no = client_no;
                }
                printf("Connected to client %d: %s port %d, MODE:%s, Proto:%s, %d Bps\n", client[client_no].client_no,
                       inet_ntoa(address.sin_addr),
                       ntohs(address.sin_port), client[client_no].type, client[client_no].proto,
                       client[client_no].pktrate);


                //assign send client to recv client
                int confirm = FALSE;
                if (strcmp((const char *) client[client_no].type, "send") == 0) {
                    int counter = 1;
                    //printf("\nclient no: %d ,%s ,%d\n", client[counter].client_no, client[counter].type, client[counter].status);
                    if (strcmp((const char *) client[client_no].proto, "tcp") == 0) {

                        for (counter = 1; counter < client_no; counter++) {
                            //check the recv client status, 0 for not connected
                            if (strcmp((const char *) client[counter].type, "recv") == 0 &&
                                client[counter].status == 0) {
                                client[client_no].send_status = 1;
                                client[counter].status = 1;
                                client[client_no].conn_sock = client[counter].sock;
                                client[client_no].conn_no = client[counter].client_no;
                                client[counter].conn_no = client[client_no].client_no;
                                confirm = TRUE;
                                break;
                            }
                        }
                    }

                    if (strcmp((const char *) client[client_no].proto, "udp") == 0) {
                        udp_new_socket = accept(udp_master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);

                        client[client_no].udp_sock = udp_new_socket;
                        pthread_t sniffer_thread;
                        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) &client[client_no])) {
                            perror("could not create thread");
                            return 1;
                        }
                        //close(new_socket);
                    }
                } else {
                    if (strcmp((const char *) client[client_no].proto, "udp") == 0) {

                        client[client_no].udp_sock = udp_master_socket;
                        pthread_t sniffer_thread;
                        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) &client[client_no])) {
                            perror("could not create thread");
                            return 1;
                        }

                    }
                    confirm = TRUE;
                } //if it is recv connection


                //if confirm = TRUE, udp connection || available tcp recv connection || recv connection
                if (confirm == TRUE && strcmp((const char *) client[client_no].proto, "tcp") == 0) {
                    pthread_t sniffer_thread;
                    client[client_no].sock = new_socket;
                    if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) &client[client_no])) {
                        perror("could not create thread");
                        return 1;
                    }
                } else if (strcmp((const char *) client[client_no].proto, "tcp") == 0) {
                    printf("[!Error] No avaliable TCP client for client %d connection.\n", client[client_no].client_no);
                    close(new_socket);
                }

                client_no++;

            }
        }

    }

    return 0;
}


/*
 * This will handle connection for each client
 * */
void *connection_handler (void *client) {
    struct client_info *this_client = client;
    //Get the socket descriptor
    int read_size;
    int r_read_size;
    char client_buf[65536];

    memset(client_buf, 0, sizeof(int));

    /* handle client in send mode */
    if ((strcmp((const char *) this_client->type, "send")) == 0) {
        char *buf = (char *) calloc(sizeof(char), (unsigned long) 65536);
        //clients' proto = tcp
        if ((strcmp((const char *) this_client->proto, "tcp")) == 0) {
            while (1) {

                int bytes_sent = 0;
                while (bytes_sent < this_client->pktrate) {
                    r_read_size = recv(this_client->sock, buf, this_client->pktsize, 0);
                    read_size = send(this_client->conn_sock, buf, this_client->pktsize, 0);
                    bytes_sent += read_size;

                    if (read_size <= 0 || r_read_size <=0) {
                        printf("[Disconnection] Client %d & Client %d disconnected\n", this_client->client_no,
                               this_client->conn_no);
                        close(this_client->sock);
                        close(this_client->conn_sock);
                        return 0;
                    }

                }

            }
        }

        //clients' proto = udp
        else if ((strcmp((const char *) this_client->proto, "udp")) == 0) {

            struct sockaddr_in from;
            unsigned int addrlen = sizeof(from);
            int total_send = 0;
            int bytes_sent = 0;
            while (1) {

                bytes_sent = 0;
                if (bytes_sent < this_client->pktrate) {
                    read_size = recvfrom(this_client->udp_sock, buf, this_client->pktsize, 0, (struct sockaddr *) &from,
                                         &addrlen);
                    sendto(this_client->udp_sock, client_buf, this_client->pktsize, 0,(struct sockaddr*)&from, sizeof(from));
                    bytes_sent += read_size;
                    total_send += bytes_sent;
                }
                if(read_size <= 0)
                {
                    return 0;
                }
            }

        }

        return 0;
    }

    /* handle client in recv mode */
    if (strcmp((const char *) this_client->type, "recv") == 0) {
        char *buf = (char *) calloc(sizeof(char), 65536);
        int count = 0;
        //Accept and incoming connection


        double time;
        clock_t ctime, ptime = clock();
        if (strcmp((const char *) this_client->proto, "tcp") == 0) {
            printf("[Pending] Client %d Waiting for incoming connections...\n", this_client->client_no);
        }
        while (1) {

            if (strcmp((const char *) this_client->proto, "tcp") == 0) {
                if (this_client->status == 1) {
                    this_client->status = 2;
                    printf("[Connection] Client %d Connected Client %d.\n", this_client->conn_no,
                           this_client->client_no);

                }
                time = time + ctime - ptime;

                if (time / CLOCKS_PER_SEC > 10 && this_client->status == 0) {
                    printf("\nClient %d timeout, disconnected.", this_client->client_no);
                    this_client->status = 2;
                    close(this_client->sock);
                    return 0;
                }
                ptime = clock();
            }

            if (strcmp((const char *) this_client->proto, "udp") == 0) {

                struct sockaddr_in from;
                unsigned int addrlen = sizeof(from);
                char message[sizeof(unsigned long)];
                int numSent = 0;
                int totalSent = 0;
                clock_t c_clock, p_clock;
                double t_time;

                memset(message, 0, sizeof(unsigned long));
                memset(buf, -100, sizeof(int));

                recvfrom(this_client->udp_sock, buf, this_client->pktsize, 0, (struct sockaddr *) &from, &addrlen);
                while(this_client->pktnum == 0 || (this_client->pktnum != 0 && (int)numSent <= this_client->pktnum)) {
                    c_clock = clock();
                    int bytes_sent=0;
                    *((unsigned long*)message) = htonl(numSent);
                    sendto(this_client->udp_sock, message, this_client->pktsize, 0, (struct sockaddr *) &from, addrlen);
                    if(totalSent/t_time < this_client->pktrate * 8.0) {

                        while (bytes_sent < this_client->pktsize) {

                            bytes_sent = sendto(this_client->udp_sock, buf, this_client->pktsize, 0,
                                                (struct sockaddr *) &from, addrlen);
                            int check = recvfrom(this_client->udp_sock, buf, this_client->pktsize, 0, (struct sockaddr *) &from, &addrlen);
                            totalSent += bytes_sent;
                            if(check <= 0){return 0;}
                        }
                        numSent++;
                    }
                    p_clock = clock();
                    t_time += (double) (c_clock - p_clock) / CLOCKS_PER_SEC;

                }
            }
        }

    }


    return 0;
}
