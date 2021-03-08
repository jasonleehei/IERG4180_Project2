<h3 align="center">IERG4180_Project2: NetProbe Client-Server Edition </h3>

## About The Project
SID: 1155095162

Name: Lee Cheung Hei

## Getting Started
To get a local copy up and running follow these simple steps.

### Installation
1. Clone or Download the repo
   ```sh
   git clone https://github.com/jasonleehei/IERG4180_Project2
   ```

## Usage
  ```sh
    
    Simple client mode demo:

    Server:
       - Go to NetProbe/NetProbe-Server/
       - On terminal, type ./netprobe-server 


    TCP client:
        - Go to NetProbe/NetProbe-Client/

        For Linux:
            - On terminal, type ./netprobe-client -recv -proto tcp
            - On terminal, type ./netprobe-client -send -proto tcp

        For Window:
            - On terminal, type netprobe-client -recv -proto tcp
            - On terminal, type netprobe-client -send -proto tcp

    p.s.Due to this Server provide 1 pair 1 tcp clients connection, please first connect a recv first:) 


    UDP client:
        - Go to NetProbe/NetProbe-Client/

        For Linux:
            - On terminal, type ./netprobe-client -recv 
            - On terminal, type ./netprobe-client -send 

        For Window:
            - On terminal, type netprobe-client -recv 
            - On terminal, type netprobe-client -send 


    Please choose NetProbes' mode!
    SEND MODE: -send
    RECEIEVE MODE: -recv
    SERVER MODE: -server
    If[mode] = -send/-recv then the following are the supported parameters :
               < -stat yyy >         update statistics once every yyy ms. (Default = 500 ms)
               < -rhost hostname > send data to host specified by hostname. (Default 'localhost')
               < -rport portnum > send data to remote host at port number portnum. (Default '4180')
               < -proto tcp || udp > send data using TCP or UDP. (Default UDP)
               < -pktsize bsize > send message of bsize bytes. (Default 1000 bytes)
               < -pktrate txrate > send data at a data rate of txrate bytes per second,
                                   0 means as fast as possible. (Default 1000 bytes / second
               < -pktnum num > send or receive a total of num messages. (Default = infinite)
               < -sbufsize bsize > set the outgoing socket buffer size to bsize bytes.
               < -rbufsize bsize > set the incoming socket buffer size to bsize bytes.
    else if[mode] = -server then
               < -lhost hostname > hostname to bind to. (Default late binding)
               < -lport portnum > port number to bind to. (Default '4180')
               < -rbufsize bsize > set the incoming socket buffer size to bsize bytes.
               < -sbufsize bsize > set the outgoing socket buffer size to bsize bytes.
    end if.
   ```


