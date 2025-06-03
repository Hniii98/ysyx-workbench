#include <trace.h>


#ifndef CONFIG_TARGET_AM
#ifdef CONFIG_IRINGTRACE
/* Instruction ring buffer */
    IRingBuf iringbuf = {0};


    void iringbuf_write_once(uint32_t inst, vaddr_t pc){
        size_t widx = iringbuf.wptr;
        char *p = iringbuf.ringbuf[widx];
        p += snprintf(p, sizeof(iringbuf.ringbuf[widx]), FMT_WORD ":", pc);
        int ilen = sizeof(word_t);
        int i;
        uint8_t *byte = (uint8_t*)&inst;
        for(i = ilen - 1; i >=0 ; i--){
            p += snprintf(p, 4, " %02x", byte[i]); // little endian
        }
        memset(p, ' ', 1);
        p += 1;

        void disassemble(char *str, int size, uint64_t pc, uint8_t* code, int byte);
        disassemble(p, iringbuf.ringbuf[widx] + sizeof(iringbuf.ringbuf[widx]) - p,
            pc, (uint8_t*)&inst, ilen);
        
        /* wrap around */
        iringbuf.wptr = (widx + 1) % RBUFSIZE;
    }

    void iringbuf_display(){
        printf("Below is %d instructions near error occurs\n", RBUFSIZE);
        for(size_t i = 0; i < RBUFSIZE; i++){
            /* empty string, break loop */
            if(strlen(iringbuf.ringbuf[i]) == 0) break;

            if((i + 1) % RBUFSIZE == iringbuf.wptr){
                printf("  ==> %s \n", iringbuf.ringbuf[i]);
            }
            else{
                printf("%s \n", iringbuf.ringbuf[i]);
            }
        }
    }

    
    
#endif
#endif

