/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Example which allows to read from a file when it is ready. Commented code reveal the possibility to set reading from STDIN.
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <tevent.h>
#include <fcntl.h>

 struct computation_state {
     int local_var;
     int fd;
 };

 static void handler_read(struct tevent_context *ev, struct tevent_fd *fde,
			    uint16_t flags, void *private_data)
{
        struct computation_state *in = talloc_get_type(private_data, struct computation_state);

        printf("Reading a file\n");

        char txt[100] = {0};  // CLEAR array

        read(in->fd, txt, 100);  // read from file descriptor
        printf("------------------\n");
	    printf("%s\n", txt);
	    printf("------------------\n");
}


// ran after the socket is freed
static void handler_closed(struct tevent_context *ev, struct tevent_fd *fde,
			    int fd, void *private_data)
{
        struct computation_state *in = talloc_get_type(private_data, struct computation_state);
        in->local_var = 10;

        printf("File descriptor closed.\n");

}


int main (int argc, char **argv) {

    printf("INIT\n");

    struct tevent_context *event_ctx;
    TALLOC_CTX *mem_ctx;
    struct tevent_fd *fde;
    struct computation_state *in;
    int fd;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    // FILE* fp = fopen("data.txt","r");
    // fd = fileno(fp);    // get file descriptor

    fd = open("data.txt", O_RDONLY);

    in = talloc(mem_ctx, struct computation_state); //child
    in->local_var = 5;
    in->fd = fd;

    fde = tevent_add_fd(event_ctx, mem_ctx, fd, TEVENT_FD_READ, handler_read, in);
    if(fde == NULL) {
        printf("MEMORY ERROR\n");
    }
    tevent_fd_set_close_fn(fde, handler_closed);    // set a handler to be called when the context is freed!

    tevent_loop_once(event_ctx);

    close(fd);
    talloc_free(mem_ctx);   // handler_read will be call after this!

    printf("Quit\n");
    return EXIT_SUCCESS;
}

