#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "receiver.h"
#include "sender.h"
#include "rcvsnd.h"
#include "../version/version.h"
#include "../library/mylibrary.h"
#include "../memoryaccess/memoryaccess.h"
#include "chat.h"


PREPARE_SEND

VERSION


int main(int argc, char** argv) {
    printf("\e[92;3mChat beenden durch eintippen von 'quit' beim senden.\e[39;0m\n");

    write_version_to_file(argc, argv, "../version/chat-firstreceive_version.txt");          //Versionierung

    //Kommunikationsadressen vorbereiten
    struct address* addr1=malloc(sizeof(struct address));
    addr1->addr1=&func4;
    addr1->addr2=&func1;

    struct address* addr2=malloc(sizeof(struct address));
    addr2->addr1=&func3;
    addr2->addr2=&func2;

    //In Dauerschleife empfangen und senden bis einer 'quit' eintippt
    while(1) {
        int left=chat_receive(argc, argv, (void*) addr1,0);
        if(left==1)break;
        int goon=prepare_send((void*)addr2);
        if(goon==0)break;
    }

    free(addr1);
    free(addr2);
}