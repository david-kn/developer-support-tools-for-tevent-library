/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Simple example which just shows the most basic task - allocate memory and initialize tevent context.
 *
 */

#include <stdio.h>
#include <tevent.h>

int main(int argc, char **argv)
{

    printf("INIT\n");

    struct tevent_context *event_ctx;
    TALLOC_CTX *mem_ctx;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    talloc_free(mem_ctx); // inotify_handler_closed will be call after this!

    printf("Quit\n");
    return EXIT_SUCCESS;
}
