/*
 * ui_common.c
 *
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "addr_hash.h"
#include "serv_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"
#include "options.h"

#include "ui_common.h"

/* 2, 10 and 40 seconds */
int history_divs[HISTORY_DIVISIONS] = {1, 5, 20};

#define UNIT_DIVISIONS 4
char* unit_bits[UNIT_DIVISIONS] =  { "b", "Kb", "Mb", "Gb"};
char* unit_bytes[UNIT_DIVISIONS] =  { "B", "KB", "MB", "GB"};

extern hash_type* history;
extern int history_pos;
extern int history_len;

/*
 * Compare two screen lines based on bandwidth.  Start comparing from the
 * specified column
 */
// int screen_line_bandwidth_compare(host_pair_line* aa, host_pair_line* bb, int start_div)


/*
 * Compare two screen lines based on hostname / IP.  Fall over to compare by
 * bandwidth.
 */
// int screen_line_host_compare(void* a, void* b, host_pair_line* aa, host_pair_line* bb)



/*
 * Compare two screen lines based on the sorting options selected.
 */
// int screen_line_compare(void* a, void* b)



/*
 * Format a data size in human-readable format
 */
void readable_size(float n, char* buf, int bsize, int ksize, int bytes) {

    int i = 0;
    float size = 1;

    /* Convert to bits? */
    if(bytes == 0) {
      n *= 8;
    }

    while(1) {
      if(n < size * 1000 || i >= UNIT_DIVISIONS - 1) {
        snprintf(buf, bsize, " %4.0f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
      i++;
      size *= ksize;
      if(n < size * 10) {
        snprintf(buf, bsize, " %4.2f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
      else if(n < size * 100) {
        snprintf(buf, bsize, " %4.1f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
  }
}

int history_length(const int d) {
    if (history_len < history_divs[d])
        return history_len * RESOLUTION;
    else
        return history_divs[d] * RESOLUTION;
}


// void screen_list_init()


// void screen_list_clear()


// void calculate_totals()


// void make_screen_list()


// void screen_hash_clear()


/* print all history record in the history hash table */
void print_all_history() {
    hash_node_type* n = NULL;
    int counter = 0;
    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        counter = counter + 1;

        char host1[HOSTNAME_LENGTH], host2[HOSTNAME_LENGTH];
        addr_pair* addr_info;
        history_type* stat;
        /*
        typedef struct node_tag {
            struct node_tag *next;    // next node
            void* key;                // key
            void* rec;                // user data
        } hash_node_type;
        */

        stat = (history_type*)n->rec;
        /*
        typedef struct {
            long recv[HISTORY_LENGTH];
            long sent[HISTORY_LENGTH];
            double long total_sent;
            double long total_recv;
            int last_write;
        } history_type;
        */
        addr_info = (addr_pair*)n->key;

        /*
        typedef struct {
            int af;
            unsigned short int protocol;
            unsigned short int src_port;
            union {
                struct in_addr src;
                struct in6_addr src6;
            };
            unsigned short int dst_port;
            union {
                struct in_addr dst;
                struct in6_addr dst6;
            };
        } addr_pair;
        */

        sprint_host(host1, AF_INET, &(addr_info->src),
            addr_info->src_port, addr_info->protocol, 40, options.aggregate_src);
        sprint_host(host2, AF_INET, &(addr_info->dst),
            addr_info->dst_port, addr_info->protocol, 40, options.aggregate_dest);
        printf("%s <->  %s %d %llu\n", host1, host2, addr_info->protocol, stat->total_sent+stat->total_recv);

    }
    printf("counter = %d\n", counter);
    printf("---------------------------------------------------------\n");
}


// void analyse_data();


void sprint_host(char * line, int af, struct in_addr* addr, unsigned int port, unsigned int protocol, int L, int unspecified_as_star) {
    char hostname[HOSTNAME_LENGTH];
    char service[HOSTNAME_LENGTH];
    char* s_name;
    union {
        char **ch_pp;
        void **void_pp;
    } u_s_name = { &s_name };

    ip_service skey;
    int left;

    if(IN6_IS_ADDR_UNSPECIFIED(addr) && unspecified_as_star) {
        sprintf(hostname, " * ");
    }
    else {
        //if (options.dnsresolution)
        //    resolve(af, addr, hostname, L);
        //else
        inet_ntop(af, addr, hostname, sizeof(hostname));
    }
    left = strlen(hostname);

    if(port != 0) {
      skey.port = port;
      skey.protocol = protocol;
      if(options.portresolution && hash_find(service_hash, &skey, u_s_name.void_pp) == HASH_STATUS_OK) {
        snprintf(service, HOSTNAME_LENGTH, ":%s", s_name);
      }
      else {
        snprintf(service, HOSTNAME_LENGTH, ":%d", port);
      }
    }
    else {
      service[0] = '\0';
    }

    /* If we're showing IPv6 addresses with a port number, put them in square
     * brackets. */
    if(port == 0 || af == AF_INET || L < 2) {
      sprintf(line, "%-*s", L, hostname);
    }
    else {
      sprintf(line, "[%-.*s]", L-2, hostname);
      left += 2;
    }
    if(left > (L - strlen(service))) {
        left = L - strlen(service);
        if(left < 0) {
           left = 0;
        }
    }
    sprintf(line + left, "%-*s", L-left, service);
}
