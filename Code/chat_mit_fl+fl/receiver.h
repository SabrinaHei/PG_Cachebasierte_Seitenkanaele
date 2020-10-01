#ifndef RECEIVER_H
#define RECEIVER_H

void* r_synchronize(void*);
void* receiver(int, void*);
void check(int*, int);
char binary_to_ascii(int*);
int chat_receive(int, char**,  void*, int);

#endif
