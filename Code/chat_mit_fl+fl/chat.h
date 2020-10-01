#ifndef CHAT_H
#define CHAT_H

#define PREPARE_SEND \
int prepare_send(void* addr){\
char string[500]={'\0'}; \
printf("Was soll gesendet werden?\n");\
fgets(string, sizeof(string), stdin);\
char* delim="\n";\
if(strcmp(&string[0], delim)==0) \
    chat_send(0,string, addr);\
else{\
    int i=0;\
    while(string[i]!='\n'){         \
    i++;\
}\
string[i]='\0';\
chat_send(1,string, addr);\
if(strcmp(string, "quit")==0)\
    return 0;\
}\
return 1;\
}                    \

#define VERSION \
void write_version_to_file(int argc, char** argv, char* filename){\
    FILE* fd;\
    fd=fopen(filename, "w");\
    fprintf(fd, "Versionsinformationen zu cachebasiertem Fl+Fl Seitenkanal Chat:\n");\
    fprintf(fd, "Programmaufruf:\t");\
    for(int i=0; i<argc;i++){\
        fprintf(fd,"%s ", argv[i]);\
    }\
    fprintf(fd,"%s\n", get_version());\
    fprintf(fd, "%s\n", COMMENT_VERSION);\
    fprintf(fd, "%s\n", MEMACCES_VERSION);\
    fclose(fd);\
    return;\
}

#endif
