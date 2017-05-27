/* macros */
# define MAXBUFFLEN 32

/* function declaration*/
int read_ip_from_file(void);
void printList(void);
void free_addr_list(void);

struct addr_node {
    struct in_addr address;
    struct addr_node *next;
};
