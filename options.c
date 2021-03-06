/*
 * options.c:
 *
 *
 */

#include "config.h"

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "iftop.h"
#include "options.h"
#include "cfgfile.h"
#include "integers.h"

#if !defined(HAVE_INET_ATON) && defined(HAVE_INET_PTON)
#   define inet_aton(a, b)  inet_pton(AF_INET, (a), (b))
#endif

options_t options;

//2017.01.21 isshe
//extern long history_delete_interval;
//extern long threshold;
extern int port_list[PORT_LIST_LEN]; 		//iftop.c defined

//2017.06.19 isshe
//options_args read from input file
int fargc = 1;
char *fargv[32];

//char optstr[] = "+i:f:nNF:G:lhpbBPm:c:s:tL:o:";
//char optstr[] = "+i:f:nNF:G:lhpbBPm:c:s:tL:o:SWZ:T:Y:X:qD:z:M:";
//char optstr[] = "+i:f:F:G:hpSWZ:T:Y:X:qD:z:M:";
char optstr[] = "+i:f:F:P:M:d:Z:t:T:B:hp";

/* Global options. */

/* Selecting an interface on which to listen: */

/* This is a list of interface name prefixes which are `bad' in the sense
 * that they don't refer to interfaces of external type on which we are
 * likely to want to listen. We also compare candidate interfaces to lo. */
static char *bad_interface_names[] = {
            "lo:",
            "lo",
            "stf",     /* pseudo-device 6to4 tunnel interface */
            "gif",     /* psuedo-device generic tunnel interface */
            "dummy",
            "vmnet",
            "wmaster", /* wmaster0 is an internal-use interface for mac80211, a Linux WiFi API. */
            NULL       /* last entry must be NULL */
        };

config_enumeration_type sort_enumeration[] = {
	{ "2s", OPTION_SORT_DIV1 },
	{ "10s", OPTION_SORT_DIV2 },
	{ "40s", OPTION_SORT_DIV3 },
	{ "source", OPTION_SORT_SRC },
	{ "destination", OPTION_SORT_DEST },
	{ NULL, -1 }
};

config_enumeration_type linedisplay_enumeration[] = {
	{ "two-line", OPTION_LINEDISPLAY_TWO_LINE },
	{ "one-line-both", OPTION_LINEDISPLAY_ONE_LINE_BOTH },
	{ "one-line-sent", OPTION_LINEDISPLAY_ONE_LINE_SENT },
	{ "one-line-received", OPTION_LINEDISPLAY_ONE_LINE_RECV },
	{ NULL, -1 }
};

config_enumeration_type showports_enumeration[] = {
	{ "off", OPTION_PORTS_OFF },
	{ "source-only", OPTION_PORTS_SRC },
	{ "destination-only", OPTION_PORTS_DEST },
	{ "on", OPTION_PORTS_ON },
	{ NULL, -1 }
};

static int is_bad_interface_name(char *i) {
    char **p;
    for (p = bad_interface_names; *p; ++p)
        if (strncmp(i, *p, strlen(*p)) == 0)
            return 1;
    return 0;
}

/* This finds the first interface which is up and is not the loopback
 * interface or one of the interface types listed in bad_interface_names. */
static char *get_first_interface(void) {
    struct if_nameindex * nameindex;
    struct ifreq ifr;
    char *i = NULL;
    int j = 0;
    int s;
    /* Use if_nameindex(3) instead? */

    nameindex = if_nameindex();
    if(nameindex == NULL) {
        return NULL;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */

    while(nameindex[j].if_index != 0) {
        if (strcmp(nameindex[j].if_name, "lo") != 0 && !is_bad_interface_name(nameindex[j].if_name)) {
            strncpy(ifr.ifr_name, nameindex[j].if_name, sizeof(ifr.ifr_name));
            if ((s == -1) || (ioctl(s, SIOCGIFFLAGS, &ifr) == -1) || (ifr.ifr_flags & IFF_UP)) {
                i = xstrdup(nameindex[j].if_name);
                break;
            }
        }
        j++;
    }
    if_freenameindex(nameindex);
    return i;
}

void options_set_defaults() {
    char *s;
    int i;
    /* Should go through the list of interfaces, and find the first one which
     * is up and is not lo or dummy*. */
    options.interface = get_first_interface();
    if (!options.interface)
        options.interface = "eth0";

    options.filtercode = NULL;
    options.netfilter = 0;
    inet_aton("10.0.1.0", &options.netfilternet);
    inet_aton("255.255.255.0", &options.netfiltermask);
    options.netfilter6 = 0;
    inet_pton(AF_INET6, "fe80::", &options.netfilter6net);	/* Link-local */
    inet_pton(AF_INET6, "ffff::", &options.netfilter6mask);
    options.link_local = 0;
    options.dnsresolution = 1;
    options.portresolution = 1;
#ifdef NEED_PROMISCUOUS_FOR_OUTGOING
    options.promiscuous = 1;
    options.promiscuous_but_choosy = 1;
#else
    options.promiscuous = 0;
    options.promiscuous_but_choosy = 0;
#endif
    options.showbars = 1;
    options.showports = OPTION_PORTS_ON;
    options.aggregate_src = 0;
    options.aggregate_dest = 0;
    options.paused = 0;
    options.showhelp = 0;
    options.bandwidth_in_bytes = 1; 		//Ĭ�ϴ���-isshe
    options.sort = OPTION_SORT_DIV2;
    options.screenfilter = NULL;
    options.freezeorder = 0;
    options.linedisplay = OPTION_LINEDISPLAY_TWO_LINE;
    options.screen_offset = 0;
    options.show_totals = 1;	//isshe
    options.max_bandwidth = 0; /* auto */
    options.log_scale = 0;
    options.bar_interval = 1;
    options.timed_output = 0;
    options.no_curses = 0;
    options.num_lines = 10;

	//2017.1.17-isshe
	options.create_file = 0;
	options.save_file = 0;

	//2017.1.18-isshe
	options.write_communication_info = 0;
	options.write_interval_timeout = 0;

	//2017.1.21-isshe
	options.send_communication_info = 0;
	options.create_send_thread = 0;

	options.history_delete_interval = 0;	//delete history list interval
	options.threshold = 0; 					//
	options.write_interval = 2; 			//write file interval

    //2017.05.23-isshe
    //-----------------------------------------------------------
    //options.control_block_filter = 0;
        /* ignore IP addresses in 224.0.0/24 224.0.1/24
         * 127.0.0.1 255.255.255.255 range
         */
    options.duration = 0;

    //options.num_of_block_protocols = 0;
    //options.block_protocols = (int*)calloc(32, sizeof(int));
    for (i = 0; i < 32; i++) {
        options.block_protocols[i] = -1;
    }

    //2017.05.30-isshe
    options.send_interval = 0;
    options.send_last = time(NULL);
    //------------------------------------------------------------
    //2017.06.19-isshe
    options.mqtt_addr = (char*)malloc(sizeof(char)*64);
    strcpy(options.mqtt_addr, "tcp://test.mosquitto.org:1883");
    //------------------------------------------------------------

    /* Figure out the name for the config file */
    s = getenv("HOME");
    if(s != NULL) {
        int i = strlen(s) + 9 + 1;
        options.config_file = xmalloc(i);
        snprintf(options.config_file,i,"%s/.iftoprc",s);
    }
    else {
        options.config_file = xstrdup("iftoprc");
    }
    options.config_file_specified = 0;

}

/* usage:
 * Print usage information. */
static void usage(FILE *fp) {
    fprintf(fp,
"iftop: display bandwidth usage on an interface by host\n"
"\n"
"Synopsis: iftop -h | [-p] [-t seconds] [-T bytes]\n"
"                     [-d seconds] [-M mins]\n"
"\n"
"   iftop:\n"
"   -----------------------------------------------------------------------\n"
"   -h                  display this message\n"

"   -p                  run in promiscuous mode (show traffic between other\n"
"                       hosts on the same network segment)\n"
"   -i interface        listen on named interface\n"
"   -f filter code      use filter code to select packets to count\n"
"                       (default: none, but only IP packets are counted)\n"
"   -F net/mask         show traffic flows in/out of IPv4 network\n"

"\n"
"   dotqoo:\n"
"   -----------------------------------------------------------------------\n"
"   -P port,port...     port filter\n"
"   -M mins             time interval of send connection info over MQTT\n"
"   -d seconds          drop connection when no pkt send/recv within -d sec\n"
"                       after the connection has been created\n"
"   -Z ip_p,ip_p...     block the protocol types which follow by '-z'\n"
"                       protocol type number can find at\n"
"                       wikipedia.org: 'List_of_IP_protocol_numbers'\n\n"

"   -t seconds          clear connection-info interval\n"
"   -T threshold        drop the connection when its transmission flow\n"
"                       is less than threshold(bytes)\n"

"\n"
"iftop, modified by dotqoo from version: " PACKAGE_VERSION "\n"
"copyright (c) 2002 Paul Warren <pdw@ex-parrot.com> and contributors\n"
            );
}


int read_args_from_file(/* arguments */) {
    int i = 0;
    FILE *fp;
    char *pos, *arg, buff[255], args[255];
    char delim[] = " ";

    fp = fopen("./options_args.txt", "r");
    if (!fp) {
        printf("cannot find input 'options_args' file, running at default setting\n");
        return 1;
    }

    while (fgets(buff, 255, (FILE*)fp)) {
        if (buff[0] != '#' && buff[0] != '\n') {
            /* remove '\n' character */
            if ((pos=strchr(buff, '\n')) != NULL) {
                *pos = '\0';
            }

            strcpy(args+i, buff);
            i += strlen(buff);
            strcpy(args+i, " ");
            i++;
        }
    }

    // save args to global variable 'fargc' and 'fargv'
    if (((arg = strtok(args, delim)) != NULL)) {
        fargv[fargc] = (char*)malloc(sizeof(char)*64);
        strcpy(fargv[fargc], arg);
        fargc++;
    }
    while ((arg = strtok(NULL, delim)) != NULL) {
        fargv[fargc] = (char*)malloc(sizeof(char)*64);
        strcpy(fargv[fargc], arg);
        fargc++;
    }


    printf("size: %d\n", fargc);
    for (i = 0; i < fargc; i++) {
        printf("%s|", fargv[i]);
    }
    printf("\n");


    fclose(fp);
    return 0;
}


void options_read_args(int argc, char **argv) {
    int opt;

	//isshe
	int i = 0;
	char delim[] = ",";
	char *p = NULL;
    char *ip_p = NULL;

    opterr = 0;
    // 2017.05.30-isshe
    //--------------------------------------------------
    // disable dns-resolution and port-resolution
    config_set_string("dns-resolution","false");
    config_set_string("port-resolution","false");
    // and make sure always use text interface
    config_set_string("no-curses", "true");
    //--------------------------------------------------

    // 2017.06.19-isshe
    // if a input args file exist, apply the args file first
    if (!read_args_from_file()) {
        printf("-> input args read from 'options_args.txt'\n");
        argc = fargc;
        argv = fargv;
    }

    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'h':
                usage(stdout);
                exit(0);

            //case 'n':
            //    config_set_string("dns-resolution","false");
            //    break;

            //case 'N':
            //    config_set_string("port-resolution","false");
            //    break;

            case 'i':
                config_set_string("interface", optarg);
                break;

            case 'f':
                config_set_string("filter-code", optarg);
                break;

            case 'p':
                config_set_string("promiscuous", "true");
                break;

            case 'F':
                config_set_string("net-filter", optarg);
                break;

            case '?':
                fprintf(stderr, "iftop: unknown option -%c\n", optopt);
                usage(stderr);
                exit(1);

            case ':':
                fprintf(stderr, "iftop: option -%c requires an argument\n", optopt);
                usage(stderr);
                exit(1);

			//2017.01.21-isshe
			case 'P':
				for (i = 0; i < PORT_LIST_LEN; i++)
				{
					port_list[i] = -1;
				}

				port_list[0] = atoi(strtok(optarg,delim));
				printf("* Ignore ports: %d", port_list[0]);

				for(i = 1; (p = strtok(NULL, delim)) != NULL; i++)
				{
					port_list[i] = atoi(p);
					printf("%d", port_list[i]);
				}
                printf("\n");
				break;
            /*
			case 'W':
				options.write_communication_info = 1;
				break;

			case 'S':
				options.send_communication_info = 1;
				break;
            */

			case 't':
				options.history_delete_interval = atol(optarg);
				printf("* Clear connection-info interval = %ld secs\n", options.history_delete_interval);
				break;

			case 'T':
				options.threshold = atol(optarg);
				printf("* Threshold = %ld\n", options.threshold);
				break;

            /*
			case 'X':
				options.write_interval = atoi(optarg);
				printf("write_interval = %d\n", options.write_interval);
				options.write_communication_info = 1;
				break;
            */

            //2017.05.24-isshe
            //------------------------------------------------------------------
            case 'd':
                printf("* Pkt duration filter is on\n");
                printf("  connection lowest duration = %s secs\n", optarg);
                options.duration = atoi(optarg);
                break;

            case 'Z':
                options.block_protocols[0] = atoi(strtok(optarg, delim));
                //printf("ip_p: %d\n", options.block_protocols[0]);

                for(i = 1; (ip_p = strtok(NULL, delim)) != NULL; i++) {
					options.block_protocols[i] = atoi(ip_p);
					//printf("ip_p: %d\n", options.block_protocols[i]);
				}
                //printf("num_of_block_protocols: %d\n", options.num_of_block_protocols);
				break;

            //2017.05.30-isshe
            case 'M':
                printf("* Auto send data interval (MQTT) = %s mins\n", optarg);
                options.send_interval = atoi(optarg) * 60;
            //------------------------------------------------------------------

            case 'B':
                printf("* Broker address: %s\n",options.mqtt_addr);
                strcpy(options.mqtt_addr, optarg);

        }
    }


    if (optind != argc) {
        fprintf(stderr, "iftop: found arguments following options\n");
        fprintf(stderr, "*** some options have changed names since v0.9 ***\n");
        usage(stderr);
        exit(1);
    }
}

/* options_config_get_string:
 * Gets a value from the config, sets *value to a copy of the value, if
 * found.  Leaves the option unchanged otherwise. */
int options_config_get_string(const char *name, char** value) {
    char *s;
    s = config_get_string(name);
    if(s != NULL) {
        *value = xstrdup(s);
        return 1;
    }
    return 0;
}

int options_config_get_bool(const char *name, int* value) {
    if(config_get_string(name)) {
        *value = config_get_bool(name);
        return 1;
    }
    return 0;
}

int options_config_get_int(const char *name, int* value) {
    if(config_get_string(name)) {
        config_get_int(name, value);
        return 1;
    }
    return 0;
}

int options_config_get_enum(char *name, config_enumeration_type* enumeration, int *result) {
    int i;
    if(config_get_string(name)) {
        if(config_get_enum(name, enumeration, &i)) {
            *result = i;
            return 1;
        }
    }
    return 0;
}

int options_config_get_promiscuous() {
    if(config_get_string("promiscuous")) {
        options.promiscuous = config_get_bool("promiscuous");
        if(options.promiscuous) {
            /* User has explicitly requested promiscuous mode, so don't be
             * choosy */
            options.promiscuous_but_choosy = 0;
        }
        return 1;
    }
    return 0;
}

int options_config_get_bw_rate(char *directive, long long* result) {
    char* units;
    long long mult = 1;
    long long value;
    char *s;
    s = config_get_string(directive);
    if(s) {
        units = s + strspn(s, "0123456789");
        if(strlen(units) > 1) {
            fprintf(stderr, "Invalid units in value: %s\n", s);
            return 0;
        }
        if(strlen(units) == 1) {
            if(*units == 'k' || *units == 'K') {
                mult = 1024;
            }
            else if(*units == 'm' || *units == 'M') {
                mult = 1024 * 1024;
            }
            else if(*units == 'g' || *units == 'G') {
                mult = 1024 * 1024 * 1024;
            }
            else if(*units == 'b' || *units == 'B') {
                /* bits => mult = 1 */
            }
            else {
                fprintf(stderr, "Invalid units in value: %s\n", s);
                return 0;
            }
        }
        *units = '\0';
        if(sscanf(s, "%lld", &value) != 1) {
            fprintf(stderr, "Error reading rate: %s\n", s);
        }
        options.max_bandwidth = value * mult;
        return 1;
    }
    return 0;
}

/*
 * Read the net filter option.
 */
int options_config_get_net_filter() {
    char* s;
    s = config_get_string("net-filter");
    if(s) {
        char* mask;

        options.netfilter = 0;

        mask = strchr(s, '/');
        if (mask == NULL) {
            fprintf(stderr, "Could not parse net/mask: %s\n", s);
            return 0;
        }
        *mask = '\0';
        mask++;
        if (inet_aton(s, &options.netfilternet) == 0) {
            fprintf(stderr, "Invalid network address: %s\n", s);
            return 0;
        }
        /* Accept a netmask like /24 or /255.255.255.0. */
        if (mask[strspn(mask, "0123456789")] == '\0') {
            /* Whole string is numeric */
            int n;
            n = atoi(mask);
            if (n > 32) {
                fprintf(stderr, "Invalid netmask length: %s\n", mask);
            }
            else {
                if(n == 32) {
                  /* This needs to be special cased, although I don't fully
                   * understand why -pdw
                   */
                  options.netfiltermask.s_addr = htonl(0xffffffffl);
                }
                else {
                  u_int32_t mm = 0xffffffffl;
                  mm >>= n;
                  options.netfiltermask.s_addr = htonl(~mm);
                }
            }
            options.netfilter = 1;
        }
        else {
            if (inet_aton(mask, &options.netfiltermask) != 0)
                options.netfilter = 1;
            else {
                fprintf(stderr, "Invalid netmask: %s\n", s);
                return 0;
            }
        }
        options.netfilternet.s_addr = options.netfilternet.s_addr & options.netfiltermask.s_addr;
        return 1;
    }
    return 0;
}

/*
 * Read the net filter IPv6 option.
 */
int options_config_get_net_filter6() {
    char* s;
    int j;

    s = config_get_string("net-filter6");
    if(s) {
        char* mask;

        options.netfilter6 = 0;

        mask = strchr(s, '/');
        if (mask == NULL) {
            fprintf(stderr, "Could not parse IPv6 net/prefix: %s\n", s);
            return 0;
        }
        *mask = '\0';
        mask++;
        if (inet_pton(AF_INET6, s, &options.netfilter6net) == 0) {
            fprintf(stderr, "Invalid IPv6 network address: %s\n", s);
            return 0;
        }
        /* Accept prefix lengths and address expressions. */
        if (mask[strspn(mask, "0123456789")] == '\0') {
            /* Whole string is numeric */
            unsigned int n;

            n = atoi(mask);
            if (n > 128 || n < 1) {
                fprintf(stderr, "Invalid IPv6 prefix length: %s\n", mask);
            }
            else {
                int bl, rem;
                const uint8_t mm = 0xff;
                uint8_t part = mm;

                bl = n / 8;
                rem = n % 8;
                part <<= 8 - rem;
                for (j=0; j < bl; ++j)
                    options.netfilter6mask.s6_addr[j] = mm;

                if (rem > 0)
                    options.netfilter6mask.s6_addr[bl] = part;
                options.netfilter6 = 1;
              }
        }
        else {
            if (inet_pton(AF_INET6, mask, &options.netfilter6mask) != 0)
                options.netfilter6 = 1;
            else {
                fprintf(stderr, "Invalid IPv6 netmask: %s\n", s);
                return 0;
            }
        }
        /* Prepare any comparison by masking the provided filtered net. */
        for (j=0; j < 16; ++j)
            options.netfilter6net.s6_addr[j] &= options.netfilter6mask.s6_addr[j];

        return 1;
    }
    return 0;
}

void options_make() {
    options_config_get_string("interface", &options.interface);
    options_config_get_bool("dns-resolution", &options.dnsresolution);
    options_config_get_bool("port-resolution", &options.portresolution);
    options_config_get_string("filter-code", &options.filtercode);
    options_config_get_bool("show-bars", &options.showbars);
    options_config_get_promiscuous();
    options_config_get_bool("hide-source", &options.aggregate_src);
    options_config_get_bool("hide-destination", &options.aggregate_dest);
    options_config_get_bool("use-bytes", &options.bandwidth_in_bytes);
    options_config_get_enum("sort", sort_enumeration, (int*)&options.sort);
    options_config_get_enum("line-display", linedisplay_enumeration, (int*)&options.linedisplay);
    options_config_get_bool("show-totals", &options.show_totals);
    options_config_get_bool("log-scale", &options.log_scale);
    options_config_get_bw_rate("max-bandwidth", &options.max_bandwidth);
    options_config_get_enum("port-display", showports_enumeration, (int*)&options.showports);
    options_config_get_string("screen-filter", &options.screenfilter);
    options_config_get_bool("link-local", &options.link_local);
    options_config_get_int("timed-output", &options.timed_output);
    options_config_get_bool("no-curses", &options.no_curses);
    options_config_get_int("num-lines", &options.num_lines);
    options_config_get_net_filter();
    options_config_get_net_filter6();
};
