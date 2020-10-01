#ifndef SENDER_H
#define SENDER_H

void* s_synchronize(void*);
void* sender(void*, void*);
void* random_string();
int* ascii_to_binary(int);
int chat_send(int, char*,void*);

struct send_data{
    int strl;
    int* bitstring;
};

#endif