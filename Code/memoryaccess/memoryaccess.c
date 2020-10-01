#include <stdint.h>

//Messung von CPU-Zykeln
static inline uint64_t rdtsc() {
    uint64_t eax, edx;
    __asm__ volatile ("lfence");
    __asm__ volatile (  "rdtsc"                 //Read Time Stamp Counter bestimmt Anzahl der vergangenen CPU-Zykel und speichert in Register EDX:EAX
                    : "=a" (eax), "=d" (edx));  //speichert die unteren bits in Variable eax und die oberen in Variable edx
    __asm__ volatile ("lfence");
    uint64_t rdx = (edx<<32) | eax;             //verschiebt in edx gespeicherte Bits in oberen Bit-Bereich von rdx und hängt eax durch XOR an
    return rdx;
}

//Zugriff auf angegebene Speicheradresse
void access_memory(void* address){
    __asm__ volatile ("movq (%0), %%rax\n"      //Zugriff: legt Adresse (0.Parameter address) in Register rax
    :                                           //Keine Output-Operanden
    : "c" (address)                             //Input-Operand address in register rcx
    : "rax");                                   //rax zur Zwischenspeicherung
}

//flushen von Speicheradresse
inline void flush(void* address){
    __asm__ volatile ("clflush 0(%0)\n"
    :
    : "r" (address)                             //Input-Operand address in beliebiges General Purpose Register
    : "rax");
}

//Zeitmessung flush bei Cache Miss
uint64_t measuretime_flush_miss(void* address){
    flush(address);
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    uint64_t time_start = rdtsc();              //Startzeit bestimmen
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    flush(address);                             //Flushen der Speicheradresse
    __asm__ volatile ("mfence");
    uint64_t time_needed = rdtsc()-time_start;  //benötigte Zeit zum flushen berechnen
    return time_needed;
}

//Zeitmessung flush bei Cache Hit
uint64_t measuretime_flush_hit(void* address){
    access_memory(address);                     //auf Adresse Zugreifen -> in Cache laden
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    uint64_t time_start = rdtsc();              //Startzeit bestimmen
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    flush(address);                             //Flushen der Speicheradresse
    __asm__ volatile ("mfence");
    uint64_t time_needed = rdtsc()-time_start;  //benötigte Zeit zum flushen berechnen
    return time_needed;
}

//Zeitmessung flush
uint64_t measuretime_flush(void* address){
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    uint64_t time_start = rdtsc();              //Startzeit bestimmen
    __asm__ volatile ("mfence");
    __asm__ volatile ("lfence");
    flush(address);                             //Flushen der Speicheradresse
    __asm__ volatile ("mfence");
    uint64_t time_needed = rdtsc()-time_start;  //benötigte Zeit zum flushen berechnen
    return time_needed;
}
