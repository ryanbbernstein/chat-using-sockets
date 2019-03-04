/*
** chat.c -- a multiuser chat server/client
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <sys/time.h>

#include "InputHandler.h"

// define file descriptor for stdin
#define STDIN 0

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    setlinebuf(stdout);
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    char* port = argv[1];
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

    struct sockaddr_in * ipAddress;
    char* ip;

	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}
        ipAddress = (struct sockaddr_in *)p->ai_addr;

		break;
	}

    ip = inet_ntoa(ipAddress->sin_addr);

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);
    FD_SET(fileno(stdin), &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    char stdinBuf[200];

    int bPrompt = 1;
    // main loop
    for(;;) {
        if (bPrompt) {
            bPrompt = 0;
            printf(">> ");
            fflush( stdout );
        }
        

        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }


        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == fileno(stdin)) {
                    bPrompt = 1;
                    if (fgets(stdinBuf, sizeof stdinBuf, stdin)) {
                        if (ferror(stdin)) {
                            perror("stdin");
                            exit(1);
                        }
                        size_t len = strlen(stdinBuf);
                        if( stdinBuf[len-1] == '\n' )
                            stdinBuf[len-1] = 0;
                        
                        if (strlen(stdinBuf) != 0) {
                        	char* parsedArgs[MAXLIST];
                            int command = processString(stdinBuf, parsedArgs);

                            switch (command)
                            {
                                case 1:
                                    // exit
                                    printf("\nGoodbye!\n");
                                    exit(0);
                                    break;
                                case 2:
                                    // help
                                    ShowHelp();
                                    break;
                                case 3:
                                    // myip
                                    printf("\nLocal IP address: %s\n", ip);
                                    break;
                                case 4:
                                    // myport
                                    printf("\nListening on port: %s\n", port);
                                    break;
                                case 5:
                                    // connect
                                    
                                    break;
                                case 6:
                                    // list
                                    
                                    break;
                                case 7:
                                    // terminate
                                    
                                    break;
                                case 8:
                                    // send
                                    
                                    break;
                                default:
                                    break;
                            }
                        }


                        
                    }
                } else
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}



