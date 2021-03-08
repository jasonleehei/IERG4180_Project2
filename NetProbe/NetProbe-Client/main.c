#undef UNICODE
/*
* File         : NetProbe-Client
* System       : WIN32/Linux
* Project      : IERG4180 project 2
* 
*/

#include "stdafx.h"
#pragma comment(lib, "Ws2_32.lib")


static struct option mode_options[] =
{   
    {"send", no_argument, 0, 1},
    {"recv", no_argument, 0, 2},
};

static struct option send_recv_setting[] =
{
    {"stat", required_argument, 0, 1},
    {"rhost", required_argument, 0, 2},
    {"rport", required_argument, 0, 3},
    {"proto", required_argument, 0, 4},
    {"pktsize", required_argument, 0, 5},
    {"pktrate", required_argument, 0, 6},
    {"pktnum", required_argument, 0, 7},
    {"sbufsize", required_argument, 0, 8},
    {"rbufsize", required_argument, 0 ,9},
};


int main(int argc, char* argv[]) {


    /* Select Mode */
    int MODE = getopt_long_only(argc, argv, "", mode_options, 0);
    
    if (MODE != -1)
    {
        switch (MODE)
        {
            case 1:
                SEND(argc, argv);
                break;
            case 2:
                RECV(argc, argv);
                break;
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
                    "end if.");
                break;
        }
    }
    return(0);

}

/* SEND MODE */
int SEND(int argc, char* argv[])
{
    /* Paramters */
    int stat = DEFAULT_UPDATE;
    char* rhost = DEFAULT_SERVER;
    int rport = DEFAULT_REMOTE_PORT_NUMBER;
    char* proto = DEFAULT_PROTO;
    int pktsize = DEFAULT_BSIZE;
    int pktrate = DEFAULT_TXRATE;
    int pktnum = DEFAULT_TOTAL_NUM_MESSAGES;
    int sbufsize = 65536;
    int parameters;

    /* getopt of parameters in mode */
    while ((parameters = getopt_long_only(argc, argv, "", send_recv_setting, 0)) != -1)
    {
        switch (parameters)
        {
        case 1:
            stat = atoi(optarg);
            break;
        case 2:
            rhost = optarg;
            break;
        case 3:
            rport = atoi(optarg);
            break;
        case 4:
            proto = optarg;
            break;
        case 5:
            pktsize = atoi(optarg);
            break;
        case 6:
            pktrate = atoi(optarg);
            break;
        case 7:
            pktnum = atoi(optarg);
            break;
        case 8:
            sbufsize = atoi(optarg);
            break;
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
    printf("Mode:SEND Protocol:%s\n", proto);
    printf("-stat = %d\n-pktsize = %d\n-pktrate = %d\n-pktnum = %d\n-sbufsize = %d\n", stat, pktsize, pktrate, pktnum, sbufsize);

    /* Statistics Display Setting */
    clock_t current_clock, previous_clock = clock();
    double  cum_time_cost = 0, coum_bytes_send = 0;
    double total_time = 1;
    unsigned long numSent = 0;


    SOCKET s, udp_s;
    struct sockaddr_in server;
    char *send_pmessage;

#ifdef WIN32
    /* Initialise winsock */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());  exit(EXIT_FAILURE);
    }
#endif // WIN32

    /* Connect to the server and send the netprobe parameters*/
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    if ((udp_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    memset(send_pmessage, 0, sizeof(int));
    server.sin_addr.s_addr = inet_addr(DEFAULT_SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(rport);

    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        puts("connect error"); return 1;
    }

    sprintf(send_pmessage, "%s,%s,%d,%d,%d","send", proto, pktsize, pktrate, pktnum);

    //Send the parameter data
    if (send(s, send_pmessage, strlen(send_pmessage), 0) < 0)
    {
        puts("Send failed"); return 1;
    }
    printf("Connect to the server sucessfully\n");
    /* End of connecting to the server*/

    /***** SEND by UDP *****/
    if (strcmp(proto, "udp") == 0)
    {
        closesocket(s);

        /*
        if (connect(udp_s, (struct sockaddr*)&server, sizeof(server)) < 0)
        { puts("connect error"); return 1; }
        */

        char* buf;
        if (sbufsize == 0) { buf = (char*)calloc(sizeof(char), BUFLEN); }
        else { buf = (char*)calloc(sizeof(char), sbufsize); }
        char message[sizeof(int)];
        double total_sent_bit = 0;
        double average = 0;
        double jitter = 0;

        memset(message, 0, sizeof(int));
        memset(buf, 0, sizeof(int));

        //Create a socket
        if ((udp_s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        { printf("Could not create socket : %d", WSAGetLastError()); exit(EXIT_FAILURE); }


        while (pktnum == 0 || (pktnum != 0 && (int)numSent <= pktnum))
        {
            int bytes_sent = 0;
            *((unsigned long*)message) = htonl(numSent);
            //sendto(udp_s, message, sizeof(int), 0, (struct sockaddr*)&server, sizeof(server));
            
            current_clock = clock();
            cum_time_cost += (double)(current_clock - previous_clock) / CLOCKS_PER_SEC;
            average = (double)(average * (double)numSent + previous_clock - current_clock) / ((double)numSent + 1);
            jitter = (double)(jitter * (double)numSent + abs(previous_clock - current_clock - average)) / ((double)numSent + 1);

            //print status
            if (cum_time_cost > stat) {
                total_time += cum_time_cost;
                printf("\rElapsed:%dms | Rate:%.2fkbps | Jitter:%.2fms ", (int)total_time, (total_sent_bit/1000.0/total_time), jitter);
                cum_time_cost = 0;
                previous_clock = clock();
            }

            //control sending pktrate 
            if (total_sent_bit / total_time < pktrate * 8.0)
            {

                //send the package
                while (bytes_sent < pktsize)
                {

                    int sent_bit = sendto(udp_s, buf, pktsize, 0, (struct sockaddr*)&server, sizeof(server));

                    if (sent_bit > 0) { bytes_sent = bytes_sent + sent_bit; }
                    else
                    {
                        if (WSAGetLastError() == 10054) { printf("\nDisconnect from the server"); }
                        else { printf("sendto() failed with error code : %d", WSAGetLastError()); }
                        exit(EXIT_FAILURE);
                    }
                    total_sent_bit += bytes_sent;
                }
                numSent++;
                //printf("numSent %d vs pktnum %d\n", numSent, pktnum);
            }
        }

        closesocket(s);
#ifdef WIN32
        WSACleanup();
#endif
        return 0;
    }

    /***** SEND by TCP *****/
    else if (strcmp(proto, "tcp") == 0)
    {

        char* buf;
        if (sbufsize == 0) { buf = (char*)calloc(sizeof(char), BUFLEN); }
        else { buf = (char*)calloc(sizeof(char), sbufsize); }
        double total_sent_bit = 1;
        double average = 0;
        double jitter = 0;

        while (pktnum == 0 || (pktnum != 0 && (int)numSent <= pktnum))
        {
            int bytes_sent = 0;

            current_clock = clock();
            cum_time_cost += (double)(current_clock - previous_clock) / CLOCKS_PER_SEC;
            average = (double)(average * (double)numSent + previous_clock - current_clock) / ((double)numSent + 1);
            jitter = (double)(jitter * (double)numSent + abs(previous_clock - current_clock - average)) / ((double)numSent + 1);
            
            //print status
            if (cum_time_cost > stat) {
                total_time += cum_time_cost;
                printf("\rElapsed:%dms | Rate:%.2fkbps | Jitter:%.2fms ", (int)(total_time), (total_sent_bit / 1000.0 / total_time), jitter);
                cum_time_cost = 0;
                previous_clock = clock();
            }

            //control sending pktrate 
            if (total_sent_bit/total_time < pktrate*8.0) {


                //send the package
                while (bytes_sent < pktsize) {

                    int sent_bit = send(s, buf + bytes_sent, pktsize, 0);
                    if (sent_bit > 0) { bytes_sent = bytes_sent + sent_bit; }
                    else
                    {
                        if (WSAGetLastError() == 10054 || WSAGetLastError() == 10053) { printf("\nDisconnect from the server"); }
                        else { printf("sendto() failed with error code : %d", WSAGetLastError()); }
                        return -1;
                    }
                    total_sent_bit += bytes_sent;
                }
                numSent++;
            }
        }

        closesocket(s);
#ifdef WIN32
        WSACleanup();
#endif
        return 0;
    }

    return 0;
}

/* RECV MODE */
int RECV(int argc, char* argv[])
{

    /* parameters of recv */
    int stat = DEFAULT_UPDATE;
    char* rhost = DEFAULT_SERVER;
    int rport = DEFAULT_REMOTE_PORT_NUMBER;
    char* proto = DEFAULT_PROTO;
    int pktsize = DEFAULT_BSIZE;
    int pktrate = DEFAULT_TXRATE;
    int pktnum = DEFAULT_TOTAL_NUM_MESSAGES;
    int rbufsize = 65536;
    int parameters;

    double total_recv_bit = 1;
    double average = 0;
    double jitter = 0;
    float loss_ratio = 0;
    int stat_recv = 0;

    u_long packRecv = 0;

    /* getopt in recv */
    while ((parameters = getopt_long_only(argc, argv, "", send_recv_setting, 0)) != -1)
    {
        switch (parameters)
        {
        case 1:
            stat = atoi(optarg);
            break;
        case 2:
            rhost = optarg;
            break;
        case 3:
            rport = atoi(optarg);
            break;
        case 4:
            proto = optarg;
            break;
        case 5:
            pktsize = atoi(optarg);
            break;
        case 6:
            pktrate = atoi(optarg);
            break;
        case 7:
            pktnum = atoi(optarg);
            break;
        case 8:
            rbufsize = atoi(optarg);
            break;
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
    printf("Mode:RECV Protocol:%s\n", proto);
    printf("-stat = %d\n-pktsize = %d\n-rbufsize = %d\n", stat, pktsize, rbufsize);
    if (rbufsize == 0) { printf("Default recv buffer size = 65536 bytes\n"); }
    else { printf("Custom recv buffer size = %d\n", rbufsize); }

    /* For Statistics Display */
    clock_t current_clock, previous_clock = clock();
    double  cum_time_cost = 0, coum_bytes_send = 0;
    double total_time = 1;
    
    /* Initialise winsock */
    SOCKET s, udp_s;
    struct sockaddr_in server;
    char *recv_pmessage;

#ifdef WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());  exit(EXIT_FAILURE);
    }

#endif // WIN32

    /* Connect to the server and send the netprobe parameters*/
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }


    memset(recv_pmessage, 0, sizeof(int));
    server.sin_addr.s_addr = inet_addr(DEFAULT_SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(rport);

    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        puts("connect error"); return 1;
    }

    sprintf(recv_pmessage, "%s,%s,%d,%d,%d", "recv", proto, pktsize, pktrate, pktnum);

    //Send the parameter data
    if (send(s, recv_pmessage, strlen(recv_pmessage), 0) < 0)
    {
        puts("Send failed"); return 1;
    }
    printf("Connect to the server sucessfully\n");
    /* End of connecting to the server*/


    /***** RECV by UDP *****/
    if (strcmp(proto, "udp") == 0)
    {

        /* for udp connection */
        if ((udp_s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            printf("Could not create socket : %d", WSAGetLastError());
        }

        int slen = sizeof(server);
        char* buf;
        if (rbufsize == 0) { buf = (char*)calloc(sizeof(char), BUFLEN); }
        else { buf = (char*)calloc(sizeof(char), rbufsize); }

        memset(buf, 'a', sizeof(char));
        printf("Waiting for incoming datagrames ...\n");


        //keep listening for data
        while (1)
        {


            int bytes_recv = 0;
            
            current_clock = clock();
            cum_time_cost += (double)(current_clock - previous_clock) / CLOCKS_PER_SEC;
            average = (double)(average * (double)packRecv + previous_clock - current_clock) / ((double)packRecv + 1);
            jitter = (double)(jitter * (double)packRecv + abs(previous_clock - current_clock - average)) / ((double)packRecv + 1);
            
            //print message
            if (cum_time_cost > stat) {
                total_time += cum_time_cost;
                printf("\rElapsed:%dms | Pkts:%d | Lost:%.4f%% | Rate:%.2fkbps | Jitter:%.2fms ", (int)total_time, stat_recv/stat, loss_ratio, (total_recv_bit / 1000.0 / total_time), jitter);
                cum_time_cost = 0;
                stat_recv = 0;
                previous_clock = clock();
            }
            sendto(udp_s, buf, pktsize, 0, (struct sockaddr*)&server, sizeof(server));

            recvfrom(udp_s, buf, pktsize, 0, (struct sockaddr*)&server, &slen);
            loss_ratio = (((float)ntohl(*((unsigned long*)(buf))) - (float)packRecv) / (float)ntohl(*((unsigned long*)(buf)))) * 100.00;

            if (total_recv_bit / total_time < pktrate * 8.0 ) {

                
                while (bytes_recv < pktsize)
                {
                    sendto(udp_s, buf, 10, 0, (struct sockaddr*)&server, sizeof(server));
                    int recv_len = recvfrom(udp_s, buf, pktsize, 0, (struct sockaddr*)&server, &slen);
                    //loss_ratio = (((float)ntohl(*((unsigned long*)(buf))) - (float)packRecv) / (float)ntohl(*((unsigned long*)(buf)))) * 100.00;
                    //printf("%f\n", ((float)ntohl(*((unsigned long*)(buf)))));
                    
                    

                    if (recv_len == SOCKET_ERROR)
                    {
                        printf("recvfrom() failed with error code : %d", WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }

                    else if (recv_len > 0) {
                        bytes_recv += recv_len;
                        total_recv_bit += recv_len;
                    }
                }
                packRecv++;
                stat_recv++;
            }

        }

        closesocket(s);
        
        
#ifdef WIN32
        WSACleanup();
#endif

        return 0;
    }


    /***** RECV by TCP *****/
    else if (strcmp(proto, "TCP") == 0 || strcmp(proto, "tcp") == 0)
    {

        char* buf;
        if (rbufsize == 0) { buf = (char*)calloc(sizeof(char), BUFLEN); }
        else { buf = (char*)calloc(sizeof(char), rbufsize); }
        
        listen(s, 3);

        printf("waiting for incoming connection by other client from the server\n");


        while (1)
        {
            int bytes_recv = 0;

            current_clock = clock();
            cum_time_cost += (double)(current_clock - previous_clock) / CLOCKS_PER_SEC;
            average = (double)(average * (double)packRecv + previous_clock - current_clock) / ((double)packRecv + 1);
            jitter = (double)(jitter * (double)packRecv + abs(previous_clock - current_clock - average)) / ((double)packRecv + 1);

            //print message
            if (cum_time_cost > stat) {
                total_time += cum_time_cost;
                printf("\rElapsed:%dms | Pkts:%d | Rate:%.2fkbps | Jitter:%.2fms ", (int) (total_time), stat_recv/stat, (total_recv_bit / 1000 / total_time), jitter);
                cum_time_cost = 0;
                stat_recv = 0;
                previous_clock = clock();
            }

            if ( total_recv_bit / total_time < pktrate * 8.0 ) {
                while (bytes_recv < pktsize) 
                {
                    int recv_len = recv(s, buf, pktsize, MSG_WAITALL);
                    if (recv_len <= 0)
                    {
                        printf("\nDisconnected from the server."); 
                        exit(EXIT_FAILURE);
                    }
                    else if (recv_len > 0) {
                        bytes_recv += recv_len;
                    }
                    total_recv_bit += bytes_recv;
                }
                packRecv++;
                stat_recv++;
            }

        }

        closesocket(s);
#ifdef WIN32
        WSACleanup();
#endif

        return 0;
    }

    return 0;
}



