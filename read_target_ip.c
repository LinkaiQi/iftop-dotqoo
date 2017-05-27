#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "read_target_ip.h"

//char **addr_list;
//int n_addr_list = 16;
//int n_addr = 0;

struct addr_node *head = NULL;

void insert(char* addr) {
    //create a link
    struct addr_node *link = (struct addr_node*) malloc(sizeof(struct addr_node));
    inet_aton(addr, &link->address); // store IP addredd
    //point it to old first node
    link->next = head;
    //point first to new first node
    head = link;
}

//display the list
void printList() {
   struct addr_node *ptr = head;

   //start from the beginning
   while(ptr != NULL) {
      printf("%u ", ptr->address.s_addr);
      ptr = ptr->next;
   }

   printf("\n");
}

void free_addr_list() {
    struct addr_node *ptr = head;
    struct addr_node *next_ptr;

    if (ptr == NULL) {
        return;
    }

    //start from the beginning
    while(ptr != NULL) {
        next_ptr = ptr->next;
        free(ptr);
        ptr = next_ptr;
    }
}


int read_ip_from_file() {
    FILE *fp;
    char buff[MAXBUFFLEN];

    //addr_list = (char **) malloc(sizeof(char *) * n_addr_list);

    //char *addr_list = calloc(n, sizeof(char)*32)

    fp = fopen("file.txt", "r");

    if (fp == NULL) {
        //fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
        return 1;
    }

    while (fgets(buff, MAXBUFFLEN, (FILE*)fp) != NULL) {
        insert(buff);
    }
    fclose(fp);
    return 0;
}

/*
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
    fclose(fp);
    return 0;
}
*/
//inet_aton("10.0.0.1", &antelope.sin_addr); // store IP in antelope

/*
void free_addr_list() {
    for (int i = 0; i < n_addr; i++) {
        free(addr_list[i]);
    }
    free(addr_list);
}
*/
