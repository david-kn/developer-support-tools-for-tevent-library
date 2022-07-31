/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Read file README to find out how to work with this test (together with usage of files send.c and receive.c)
 *
 * This program sets file descriptor events (+ signal event just for showing stats before killing the process by SIGINT).
 * After starting, program waits for incoming host name on port 32000 or 32001 or 32002 (use program compiled from 'send.c' to
 * send this data). This program asynchronously look for the result and then send the result to port 32100 (use program
 * 'receive.c' to receive this data).
 *
*/
#include <tevent.h>
#include <talloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

TALLOC_CTX *global_talloc_context = NULL;
struct tevent_context *root = NULL;
int succes = 0;
int failure = 0;
char last_query[50] = {0};

char *err_msg[] = {
    "OK",
    "Could not finish initialization",
    "Could not obtain address info",
    "Could not free address struct",
    "Problem with address query",
    "Problem with getting host address",
    "Problem with waking-up request",
    "Repeated request. Skipped.",
    "Problem with creating requst"
};

enum errors {
    EOK,
    EINIT,
    EADDR,      // could not obtain addrinfo
    EFREE,      // could not free addrinfo struct
    EQER,       // problem with address query
    EGET,       // problem with getting host address
    EREQ,       // problem with wake-up request
    EREP,       // repeated request
    EREQCR,     // could not create req
};

struct computation_state {
    int fd;
    struct sockaddr_in servaddr;
    TALLOC_CTX *ctx;
    struct tevent_queue *fronta;
    int port;
    int ret; // was it successul?
    bool result;
};

struct getname_state {
    struct tevent_req *req;
    int num;

};


struct gethostbyname_state { // gethostbyname_dns_state
    struct hostent *hp;
    char name[20];
    int status;
};

void err(char *msg) {

    fprintf(stderr, "> %s\n", msg);
}

static void handler(struct tevent_context *ev,
                    struct tevent_signal *se,
                    int signum,
                    int count,
                    void *siginfo,
                    void *private_data) {

        printf("Successfuly processed requests: %d\n", succes);
        printf("Failed or duplicated requests: %d\n", failure);
        talloc_free(private_data); // private_data == TALLOC_CTX *ctx
        exit(0);
}

static void gethostbyname_query(struct tevent_req *subreq) {

    printf("\t\tgethostbyname_query\n");

    struct tevent_req *req = tevent_req_callback_data(subreq,
                                                struct tevent_req);
    struct gethostbyname_state *state = tevent_req_data(req, struct gethostbyname_state);

    if (!tevent_wakeup_recv(subreq)) {
        tevent_req_error(req, EREQ);
        return;
    }
    talloc_free(subreq);

    printf("\t\t\tRequest for: %s\n", state->name);

    if(!strcmp(state->name, last_query)) {
        state->status = EREP;
        tevent_req_error(req, EREP);
        return;
    }

    state->hp = gethostbyname(state->name);
    if(state->hp == NULL) {
        tevent_req_error(req, EQER);
        return;
    }
    state->status = EOK;
    printf("\t\t\t--------\n");

     printf("\t\t\t%s = ", state->hp->h_name);
       unsigned int i=0;
       while ( state->hp -> h_addr_list[i] != NULL) {
          printf( "%s ", inet_ntoa( *( struct in_addr*)( state->hp -> h_addr_list[i])));
          i++;
       }
       printf("\n");

    printf("\t\t\t--------\n");


    tevent_req_done(req);
}

int getaddr_recv(struct tevent_req *req, int *status, struct tevent_req *new_req) {


    printf("\tgetaddr_recv\n");
    struct gethostbyname_state *current = tevent_req_data(req,
                                        struct gethostbyname_state);
    struct gethostbyname_state *new_state = tevent_req_data(new_req,
                                        struct gethostbyname_state);

    //printf("status%d:%d\n", status, *status);
    if (status) {
        *status = current->status;

        // exchange of pointers !
        strcpy(last_query, current->name);
        new_state->hp = current->hp;
        current->hp = NULL;
    }
    return EOK;
}

 static void getaddr_done(struct tevent_req *subreq)
{
        printf("\tgetaddr_done\n"); // subreq -> req - status
        struct tevent_req *req = tevent_req_callback_data(subreq, struct tevent_req);

        struct gethostbyname_state *state = tevent_req_data(req, struct gethostbyname_state);
        struct gethostbyname_state *state_old = tevent_req_data(subreq, struct gethostbyname_state);

        //req -- state <== subreq - state
        enum tevent_req_state ssstate;
        uint64_t err;
        if (tevent_req_is_error(subreq, &ssstate, &err)) {
            printf("Error: %s\n", err_msg[state_old->status]);
            tevent_req_error(req, EGET);
        } else {
            getaddr_recv(subreq, &state->status, req);
            tevent_req_done(req);
        }
/*
        if(gethostbyname_recv(subreq, &state->status, req) != EOK) {
            tevent_req_error(req, EGET);
        } else {
            tevent_req_done(req);
        }
*/
        talloc_free(subreq);
        return;
}


struct tevent_req *getaddr_send (TALLOC_CTX *mem_ctx, struct tevent_context *ev, char* host) {

    struct tevent_req *req, *subreq;
    struct getname_state *in;
    struct timeval zerotv = {0, 0};
    struct gethostbyname_state *state;

    printf("\tgetaddr_done\n");

    req = tevent_req_create(mem_ctx, &state, struct gethostbyname_state);
    if (req == NULL) {
        return NULL;
    }

    // internal data
    state->status = -1;
    strcpy (state->name, host);

    // creation of wrapper - causes triggering of callback 'gethostbyname_query'
    subreq = tevent_wakeup_send(req, ev, zerotv);
    if (subreq == NULL) {
        fprintf(stderr, "Failed to create subrequest - wrapper\n");
        talloc_free(req);
        return NULL;
    }
    tevent_req_set_callback(subreq, gethostbyname_query, req);

    return req;
}


static void forward_recv(struct tevent_req *req, bool sent) {

    printf("forward_recv\n");

    struct gethostbyname_state *state = tevent_req_data(req, struct gethostbyname_state);

    if(sent) {
        succes +=1;

    } else {
        failure += 1;
    }
    return;
}

static void forward_done(struct tevent_req *req) {

    printf("forward_done\n");

    struct gethostbyname_state *state = tevent_req_data(req, struct gethostbyname_state);

    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    char sendline[500] = {0};
    bool sent = false;
    int i = 1;
    int len = 0;
    enum tevent_req_state ssstate;
    uint64_t err;

    if (tevent_req_is_error(req, &ssstate, &err)) {
        return;
    }



    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(32100);

    strcpy(sendline, inet_ntoa( *( struct in_addr*)( state->hp -> h_addr_list[0])));
    while ( state->hp -> h_addr_list[i] != NULL) {
        len = strlen(sendline);
        if(len >= 483) {
            break;
        }
        strcpy(sendline + len, ",");
        strcpy(sendline + len + 1, inet_ntoa( *( struct in_addr*)( state->hp -> h_addr_list[i])));
        i++;
    }

    if(sendto(sockfd, sendline ,strlen(sendline), 0, (struct sockaddr *)&servaddr,sizeof(servaddr)) != -1) {
        sent = true;
    }
    forward_recv(req, sent);

    talloc_free(req);
}


struct tevent_req *forward_send (TALLOC_CTX *mem_ctx, struct tevent_context *ev, struct computation_state *in, char *host){
    struct tevent_req *req;
    struct tevent_req *subreq;
    int ret;
    struct getname_state *in_ret;
    struct gethostbyname_state *state;
    printf("________________________________________\n");
    printf("forward_send\n");

    req = tevent_req_create(in, &state, struct gethostbyname_state);
    if(req == NULL) {
        tevent_req_error(req, EREQCR);
        return req; // or NULL
    }
    strcpy (state->name, host);
    state->status = -1;


    // nesting
    subreq = getaddr_send(mem_ctx, ev, state->name);
    if (tevent_req_nomem(subreq, req)) {
        printf("nomem\n");
         return tevent_req_post(req, ev); //forward_send req
     }
    tevent_req_set_callback(subreq, getaddr_done, req);

    return req;
}


 static void socket_read(struct tevent_context *ev, struct tevent_fd *fde, uint16_t flags, void *private_data) {

    // read incoming data
    struct computation_state *in = talloc_get_type(private_data, struct computation_state);
    int n;
    char *host;
    char recvline[1000];
    struct sockaddr_in cliaddr;
    socklen_t len;
    struct tevent_req *req;

    len = sizeof(in->servaddr);
          //n = recvfrom(in->fd,recvline,10000,0,NULL,NULL);
    n = recvfrom(in->fd,recvline, 1000,0,(struct sockaddr *)&(in->servaddr),&len);
    recvline[n] = 0;

    //  done reading
    host = talloc_strndup(in, recvline, strlen(recvline)-1);
    req = forward_send(in->ctx, ev, in, host);
    if (req == NULL) {
        err("req NULL");
        return;
    }
    tevent_req_set_callback(req, forward_done, &in->result);
}


int main (int argc, char **argv) {
    printf("INIT..\n");

    struct tevent_context *ev = NULL;
    struct tevent_queue *fronta = NULL;
    TALLOC_CTX *ctx = NULL;
    struct tevent_signal *sig;

    int sockfd[3];
    int cnt = 0;
    int port = 32000;
    struct tevent_fd* fde[3];
    struct computation_state *in;
    struct sockaddr_in servaddr;
    char name[] = "127.0.0.1";


    ctx = talloc_new(NULL);
    if (ctx == NULL) {
        err("talloc");
        return EXIT_FAILURE;
    }

    ev = tevent_context_init(ctx);tevent_loop_wait(ev);
    if (ev == NULL) {
        err("tevent_context");
        return EXIT_FAILURE;
    }
    root = ev;

    //prepare three ports for listening (32000, 32001, 32002)
    for(cnt = 0; cnt < 3; cnt++) {
        sockfd[cnt] = socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd[cnt] < 0) {
            err("socket");
            return EXIT_FAILURE;
        }

        // prepare connection
        port = 32000 + cnt;
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr=inet_addr(name);
        servaddr.sin_port = htons(port);
        bind(sockfd[cnt],(struct sockaddr *)&servaddr,sizeof(servaddr));

        // store information into internal structure
        in = talloc(ctx, struct computation_state); //child
        in->fd = sockfd[cnt];
        in->port = port;
        in->servaddr = servaddr;
        in->ctx = ctx;
        in->fronta = fronta;

        // create file descriptor event
        fde[cnt] = tevent_add_fd(ev, ctx, sockfd[cnt], TEVENT_FD_READ, socket_read, in);
        if(fde[cnt] == NULL) {
            err("add_fd");
            return EXIT_FAILURE;
        }
    }

    // create signal event
    sig = tevent_add_signal(ev, ctx, SIGINT, 0, handler, ctx);
    if(sig == NULL) {
        fprintf(stderr, "FAILED to create signal event (still continue...)\n");
    }

    // lasts for ever
    tevent_loop_wait(ev);

    talloc_free(ctx);
    printf("EXIT!\n");

    return EXIT_SUCCESS;
}
