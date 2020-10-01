#include <stdint.h>
#ifndef MINE_MEMORYACCESS_H
#define MINE_MEMORYACCESS_H

#define MEMACCES_VERSION "Datei Memoryaccess: \tMit inline Funktionen"      //Versions Kommentar

//Messung von CPU-Zykeln
static inline uint64_t rdtsc();

//Zugriff auf angegebene Speicheradresse
void access_memory(void* address);

//flushen von Speicheradresse
inline void flush(void* address){
    __asm__ volatile ("clflush 0(%0)\n"
    :
    : "r" (address)     //Input-Operand address in beliebiges General Purpose Register
    : "rax");
}


//Zeitmessung flush bei Cache Miss
uint64_t measuretime_flush_miss(void* address);

//Zeitmessung flush bei Cache Hit
uint64_t measuretime_flush_hit(void* address);

//Zeitmessung flush
uint64_t measuretime_flush(void* address);


#endif
