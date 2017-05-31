/*
 * options.h:
 *
 */

#ifndef __OPTIONS_H_ /* include guard */
#define __OPTIONS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef enum {
  OPTION_PORTS_OFF,
  OPTION_PORTS_SRC,
  OPTION_PORTS_DEST,
  OPTION_PORTS_ON
} option_port_t;

typedef enum {
  OPTION_SORT_DIV1,
  OPTION_SORT_DIV2,
  OPTION_SORT_DIV3,
  OPTION_SORT_SRC,
  OPTION_SORT_DEST
} option_sort_t;

typedef enum {
  OPTION_LINEDISPLAY_TWO_LINE,
  OPTION_LINEDISPLAY_ONE_LINE_BOTH,
  OPTION_LINEDISPLAY_ONE_LINE_RECV,
  OPTION_LINEDISPLAY_ONE_LINE_SENT
} option_linedisplay_t;

/*
 * This structure has to be defined in the same order as the config
 * directives in cfgfile.c.  Clearly this is EBW.
 */
typedef struct {
    /* interface on which to listen */
    char *interface;

    int dnsresolution;
    int portresolution;
    /* pcap filter code */
    char *filtercode;

    int showbars;
    option_port_t showports;

    int promiscuous;
    int promiscuous_but_choosy;
    int aggregate_src;
    int aggregate_dest;
    int paused;
    int showhelp;
    int timed_output;
    int no_curses;
    int num_lines;
    int bandwidth_in_bytes;
    option_sort_t sort;

    int bar_interval;

    char* screenfilter;
    int freezeorder;

    int screen_offset;

    option_linedisplay_t linedisplay;

    int show_totals;

    long long max_bandwidth;
    int log_scale;

    /* Cross network filter */
    int netfilter;
    struct in_addr netfilternet;
    struct in_addr netfiltermask;

    int netfilter6;
    struct in6_addr netfilter6net;
    struct in6_addr netfilter6mask;

    /* Account for link-local traffic. */
    int link_local;

    char *config_file;
    int config_file_specified;

	//2017.01.17-isshe
	int create_file; 		//�ȴ����ļ�����д�ļ�.
	int save_file;

	int write_communication_info;
	int write_interval_timeout;

	int send_communication_info;
	int create_send_thread;

	long history_delete_interval;	//delete history list interval
	long threshold; 				//
	int write_interval; 			//write file interval

    //2017.05.23-isshe
    //--------------------------------------------------------------------------
    int control_block_filter;       /* ignore IP addresses in 224.0.0/24 224.0.1/24
                                     * 127.0.0.1 255.255.255.255 range
                                     */
    int duration;

    int num_of_block_protocols;
    int *block_protocols;

    //2017.05.30-isshe
    int send_interval;
    time_t send_last;
    //--------------------------------------------------------------------------

} options_t;


void options_set_defaults();
void options_read(int argc, char **argv);
void options_read_args(int argc, char **argv);
void options_make();

#endif /* __OPTIONS_H_ */
