#ifndef __GRAPH_OUT__
#define __GRAPH_OUT__

#include "glthread.h"
#include <string.h>
#include <stdio.h>
#include "net.h"
#include <stdlib.h>

#define NODE_NAME_SIZE 16
#define IF_NAME_SIZE 16
#define MAX_INTF_PER_NODE 10

// #define ANSI_COLOR_RED     "\x1b[31m"
// #define ANSI_COLOR_GREEN   "\x1b[32m"
// #define ANSI_COLOR_YELLOW  "\x1b[33m"
// #define ANSI_COLOR_BLUE    "\x1b[34m"
// #define ANSI_COLOR_MAGENTA "\x1b[35m"
// #define ANSI_COLOR_CYAN    "\x1b[36m"
// #define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct link_ link_t;
typedef struct node_ node_t;

typedef struct interface_{
    char if_name[IF_NAME_SIZE];
    struct node_ *att_node;
    struct link_ *link;
    intf_nw_props_t intf_nw_props;
} interface_t;

struct link_ {
    interface_t intf1;
    interface_t intf2;
    unsigned int cost;
};

typedef enum debug_status {
    DEBUG_ON ,
    DEBUG_OFF
}debug_status_t;
struct node_ {
    char node_name[NODE_NAME_SIZE];
    interface_t *intf[MAX_INTF_PER_NODE];
    glthread_t graph_out_glue;
    node_nw_prop_t node_nw_prop;
    int udp_sock_fd;
    unsigned int udp_port_number;
    debug_status_t debug_status;
};

GLTHREAD_TO_STRUCT(graph_out_glue_to_node, node_t, graph_out_glue);

typedef struct graph_out_ {
    char topolgy_name[32];
    glthread_t node_list;
} graph_out_t;

static inline node_t*
get_nbr_node(interface_t *interface){
    link_t* link = interface->link;

    if(&link->intf1 != interface)
        return link->intf1.att_node;
    return link->intf2.att_node;
};

static inline int
get_node_intf_available_slot(node_t* node){
    int index = 0;
    while( index < (MAX_INTF_PER_NODE - 1)){
        if(!node->intf[index])
            return index;
        index++;
     }
     return -1;
};

static inline interface_t*
get_node_if_by_name(node_t* node , char* if_name){
    for (int intf_index = 0 ; intf_index < (MAX_INTF_PER_NODE - 1); intf_index++){
        if(!node->intf[intf_index]) return NULL;
        if(strcmp(node->intf[intf_index]->if_name , if_name) == 0)
            return node->intf[intf_index];
    }
    return NULL;
}
static char* debug_status_str(debug_status_t status){
    switch(status){
        case DEBUG_ON :
            return "on";
        case DEBUG_OFF :
            return "off";
        default :
            return "unknown";
    }
}
static inline void
set_node_debug_status(node_t* node ,debug_status_t status){
    node->debug_status = status;
    printf("Info : %s - debug %s\n" , node->node_name , debug_status_str(status));
}


static inline node_t*
get_node_by_node_name(graph_out_t* topo , char* node_name){
    node_t *node;
    glthread_t *curr;

    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){

        node = graph_out_glue_to_node(curr);
        if(strncmp(node->node_name, node_name, strlen(node_name)) == 0)
            return node;
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    return NULL;
}
graph_out_t*
create_new_graph_out(char* topology_name);

node_t*
create_graph_out_node(graph_out_t* graph_out, char* node_name);

void insert_link_between_two_nodes(node_t* node1 , node_t* node2 ,char* from_if_name , char* to_if_name, unsigned int cost);

void dump_graph_out(graph_out_t* graph_out);
void dump_node(node_t* node);
void dump_interface(interface_t* interface);
void delete_graph_out(graph_out_t* graph_out);

#endif
