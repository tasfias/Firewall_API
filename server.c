#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>

/* To be written. This file needs to be sumitted to canvas */

pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

struct list{
    char *elem;
    struct list *next;
};

struct list *requests = NULL;

char *getRequests(){
    struct list *current = requests;

    size_t len = 0;
    while(current != NULL){
        len += strlen(current->elem) + 1;
        current = current->next;
    }

    char *response = malloc(len + 1);
    if (!response){
        return NULL;
    }
    response[0] = '\0';

    current = requests;
    while(current != NULL){
        strcat(response, current->elem);
        strcat(response, "\n");
        current = current->next;
    }
    return response;
}

struct rule{
    char *elem;
    struct query *queries;
    struct rule *next;
};

struct rule *rules = NULL;

struct query{
    char *elem;
    struct query *next;
};

void addQuery(struct rule *r, char *connection){
    struct query *q = malloc(sizeof(struct query));

    q->elem = malloc(strlen(connection) + 1);
    strcpy(q->elem, connection);
    q->next = NULL;

    if(r->queries == NULL){
        r->queries = q;
    }else{
        struct query *tmp = r->queries;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = q;
    }
}

bool checkIp(char *ip){
    char copy[50];
    strcpy(copy, ip);

    char *token;
    char *saveptr;
    int nums = 0;
    
    token = strtok_r(copy, ".", &saveptr);

    while(token != NULL){
        for(int i = 0; token[i]; i++){
            if(!isdigit(token[i])){
                return false;
            }
        }

        int number = atoi(token);
        if(number < 0 || number > 255){
            return false;
        }

        nums++;
        token = strtok_r(NULL, ".", &saveptr);
    }
    return nums == 4;
}

int ipToNum(char *ip){
    char copy[50];
    strcpy(copy, ip);

    char *token;
    char *saveptr;

    int total = 0;
    token = strtok_r(copy, ".", &saveptr);

    while(token != NULL){
        total = total * 256 + atoi(token);
        token = strtok_r(NULL, ".", &saveptr);
    }
    return total;
}

bool checkRule(char *rule){
    char copy[200];
    strcpy(copy, rule);

    char *saveptr;

    char *request;
    char *ip;
    char *port;

    request = strtok_r(copy, " ", &saveptr);
    ip = strtok_r(NULL, " ", &saveptr);
    port = strtok_r(NULL, " ", &saveptr);

    if(request == NULL || ip == NULL || port == NULL){
        return false;
    }

    if(strchr(ip, '-') != NULL){
        char *ipSavePtr;
        char *ip1 = strtok_r(ip, "-", &ipSavePtr);
        char *ip2 = strtok_r(NULL, "-", &ipSavePtr);

        if(!checkIp(ip1) || !checkIp(ip2)){
            return false;
        }

        if(ipToNum(ip1) >= ipToNum(ip2)){
            return false;
        }
    } else{
        if(!checkIp(ip)){
            return false;
        }
    }

    if(strchr(port, '-') != NULL){
        char *portSavePtr;
        char *p1 = strtok_r(port, "-", &portSavePtr);
        char *p2 = strtok_r(NULL, "-", &portSavePtr);

        int numP1 = atoi(p1);
        int numP2 = atoi(p2);

        if(numP1 < 0 || numP1 > 65535 || numP2 < 0 || numP2 > 65535){
            return false;
        }

        if(numP1 > numP2){
            return false;
        }
    } else{
        for(int i = 0; port[i]; i++){
            if(!isdigit(port[i])){
                return false;
            }
        }

        int p = atoi(port);

        if(p < 0 || p > 65535){
            return false;
        }
    }
    return true;
}

bool checkConnection(char *connection, char *rule){
    char copyC[200];
    strcpy(copyC, connection);

    char *saveptrC;

    char *ipC;
    char *portC;

    strtok_r(copyC, " ", &saveptrC);
    ipC = strtok_r(NULL, " ", &saveptrC);
    portC = strtok_r(NULL, " ", &saveptrC);

    char copyR[200];
    strcpy(copyR, rule);

    char *saveptrR;

    char *ipR;
    char *portR;

    ipR = strtok_r(copyR, " ", &saveptrR);
    portR = strtok_r(NULL, " ", &saveptrR);

    if(strchr(ipR, '-') != NULL){
        char *ipRSavePtr;
        char *ipR1 = strtok_r(ipR, "-", &ipRSavePtr);
        char *ipR2 = strtok_r(NULL, "-", &ipRSavePtr);

        if((ipToNum(ipR1) > ipToNum(ipC)) || (ipToNum(ipC) > ipToNum(ipR2))){
            return false;
        }
    } else{
        if(strcmp(ipC, ipR) != 0){
            return false;
        }
    }

    if(strchr(portR, '-') != NULL){
        char *portRSavePtr;
        char *pR1 = strtok_r(portR, "-", &portRSavePtr);
        char *pR2 = strtok_r(NULL, "-", &portRSavePtr);

        int numPR1 = atoi(pR1);
        int numPR2 = atoi(pR2);

        int numPC = atoi(portC);

        if((numPR1 > numPC) || (numPC > numPR2)){
            return false;
        }
    } else{
        if(strcmp(portC, portR) != 0){
            return false;
        }
    }
    return true;
}

bool deleteRule(char *rule){
    struct rule *current = rules;
    struct rule *prev = NULL;

    while(current != NULL){
        if(strcmp(current->elem, rule) == 0){
            if(prev == NULL){
                rules = current->next;
            }else{
                prev->next = current->next;
            }

            if(current->queries != NULL){
                struct query *q = current->queries;
                while(q != NULL){
                    struct query *tmp = q;
                    q = q->next;
                    free(tmp->elem);
                    free(tmp);
                }
            }
            free(current->elem);
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

char *getRules(){
    struct rule *current = rules;

    size_t len = 0;
    while(current != NULL){
        len += strlen("Rule: ") + strlen(current->elem) + 1;
        struct query *q = current->queries;
        while(q != NULL){
            len += strlen("Query: ") + strlen(q->elem) + 1;
            q = q->next;
        }
        current = current->next;
    }

    char *response = malloc(len);
    if(!response){
        return NULL;
    }
    response[0] = '\0';

    current = rules;
    while(current != NULL){
        strcat(response, "\n");
        strcat(response, "Rule: ");
        strcat(response, current->elem);
        strcat(response, "\n");
        if(current->queries != NULL){
            struct query *q = current->queries;
            while(q != NULL){
                strcat(response, "Query: ");
                strcat(response, q->elem);
                strcat(response, "\n");
                q = q->next;
            }
        }
        current = current->next;
    }
    return response;
}

void deleteRules(){
    pthread_rwlock_wrlock(&lock);
    struct rule *current = rules;

    while(current != NULL){
        struct rule *next = current->next;
        deleteRule(current->elem);
        current = next;
    }
    pthread_rwlock_unlock(&lock);
}

extern char *processRequest (char *);

char *processRequest (char *request) {
    char *response = malloc(1000);
    if(!response){
        return NULL;
    }
    response[0] = '\0';

    pthread_rwlock_wrlock(&lock);
    struct list *nodeR = malloc(sizeof(struct list));
    nodeR->elem = malloc(strlen(request) + 1);
    strcpy(nodeR->elem, request);
    nodeR->next = NULL;

    if(requests == NULL){
        requests = nodeR;
    } else{
        struct list *tmp = requests;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = nodeR;
    }
    pthread_rwlock_unlock(&lock);

    switch(request[0]){
        case 'R':
            free(response);
            pthread_rwlock_rdlock(&lock);
            response = getRequests();
            pthread_rwlock_unlock(&lock);
            break;

        case 'A':
            if(checkRule(request)){
                char *rule = request + 2;
                struct rule *nodeA = malloc(sizeof(struct rule));
                nodeA->elem = malloc(strlen(rule) + 1);
                strcpy(nodeA->elem, rule);
                nodeA->queries = NULL;
                nodeA->next = NULL;

                pthread_rwlock_wrlock(&lock);
                if(rules == NULL){
                    rules = nodeA;
                } else{
                    struct rule *tmp = rules;
                    while(tmp->next != NULL){
                        tmp = tmp->next;
                    }
                    tmp->next = nodeA;
                }
                pthread_rwlock_unlock(&lock);
                strcpy(response, "Rule added");
            } else{
                strcpy(response, "Invalid rule");
            }
            break;

        case 'C':
            if(!checkRule(request)){
                strcpy(response, "Illegal IP address or port specified");
            } else{
                struct rule *tmp = rules;
                char *connection = request + 2;
                pthread_rwlock_rdlock(&lock);
                while(tmp != NULL){
                    if(checkConnection(request, tmp->elem)){
                        pthread_rwlock_unlock(&lock);
                        pthread_rwlock_wrlock(&lock);
                        addQuery(tmp, connection);
                        pthread_rwlock_unlock(&lock);
                        strcpy(response, "Connection accepted");
                        return response;
                    }
                    tmp = tmp->next;
                }
                pthread_rwlock_unlock(&lock);
                strcpy(response, "Connection rejected");
            }
            break;

        case 'D':
            char *r = request + 2;
            if(!checkRule(request)){
                strcpy(response, "Invalid rule");
            } else if(!deleteRule(r)){
                strcpy(response, "Rule not found");
            } else{
                strcpy(response, "Rule deleted");
            }
            break;

        case 'L':
            free(response);
            pthread_rwlock_rdlock(&lock);
            response = getRules();
            pthread_rwlock_unlock(&lock);
            break;

        case 'F':
            pthread_rwlock_wrlock(&lock);
            deleteRules();

            struct list *current = requests;
            while(current != NULL){
                struct list *next = current->next;
                free(current->elem);
                free(current);
                current = next;
            }
            requests = NULL;

            pthread_rwlock_unlock(&lock);
            strcpy(response, "All rules deleted");
            break;

        default:
            strcpy(response, "Illegal request");
    }
    return response;
}