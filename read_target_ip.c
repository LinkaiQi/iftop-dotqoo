#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "read_target_ip.h"

char **addr_list;
int n_addr_list = 16;
int n_addr = 0;


int read_ip_from_file() {
    FILE *fp;
    char buff[MAXBUFFLEN];

    addr_list = (char **) malloc(sizeof(char *) * n_addr_list);

    //char *addr_list = calloc(n, sizeof(char)*32)

    fp = fopen("file.txt", "r");

    if (fp == NULL) {
        //fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
        return 1;
    }

    while (fgets(buff, MAXBUFFLEN, (FILE*)fp) != NULL) {
        if (n_addr >= n_addr_list ) {
            addr_list = (char **) realloc(addr_list, n_addr_list * 2);
            n_addr_list = n_addr_list * 2;
        }

        addr_list[n_addr] = calloc(1, sizeof(char) * MAXBUFFLEN);
        strncpy(addr_list[n_addr], buff, MAXBUFFLEN);

        n_addr = n_addr + 1;
    }

    return 0;
}


void free_addr_list() {
    for (int i = 0; i < n_addr; i++) {
        free(addr_list[i]);
    }
    free(addr_list);
}
