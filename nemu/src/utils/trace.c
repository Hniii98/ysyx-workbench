#include <trace.h>
#include <fcntl.h>
#include <unistd.h>


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


#ifdef CONFIG_MTRACE


bool mtrace_enable(vaddr_t addr){
    return MUXDEF(CONFIG_MTRACE, (addr >= CONFIG_MTRACE_START) && 
        (addr <= CONFIG_MTRACE_END), false);
}

void mwrite_trace(vaddr_t addr, int len, word_t data){
    if(mtrace_enable(addr)){
        printf("nemu: write %d bytes of 0x%x to memory at " FMT_PADDR "\n",
            len,
            data,
            addr);
    }
}

void mread_trace(vaddr_t addr, int len){
    if(mtrace_enable(addr)){
        printf("nemu: read %d bytes of memory at " FMT_PADDR "\n",
            len,
            addr);
    }
}

#endif

#ifdef CONFIG_FTRACE

    static FTraceData ftracedata;
    FTraceTab g_ftracetab;  

    void init_ftracedata(){
        memset(&ftracedata, 0, sizeof(FTraceData));
    }
    
    /* Get the .symtab and .strtab section by parsing elf file */
    void init_ftracetab(const char* elf_file){
        /* zero initial g_ftracetab */
        memset(&g_ftracetab, 0, sizeof(FTraceTab));
        /* elf header */
        Ehdr ehdr;
        /* section headers  */
        Shdr *shdr = NULL;
        /* read elf file and store .symtab and .strtab */
        int elf_fp;;
     
        elf_fp = open(elf_file, O_RDONLY);
        Assert(elf_fp >= 0, "Can not open given elf_file: %s", elf_file);
        

        /* initial ehdr and shdr */
        Assert(read(elf_fp, &ehdr, sizeof(Ehdr)) == sizeof(Ehdr), 
            "Error occurs when reading ehdr"); // read elf file header
        shdr = malloc(ehdr.e_shentsize * ehdr.e_shnum); 
        Assert(shdr != NULL, "Can not allocate %d bytes memory for section header table.",
            ehdr.e_shentsize * ehdr.e_shnum);
        lseek(elf_fp, ehdr.e_shoff, SEEK_SET); 

        /* read  section headers table */ 
        Assert(read(elf_fp, shdr, ehdr.e_shentsize * ehdr.e_shnum) == ehdr.e_shentsize * ehdr.e_shnum, 
            "Error occurs when reading shdr"); // read elf file header
        /* read .shstrtab section */
        Shdr *shstrtab_shdr = &shdr[ehdr.e_shstrndx];         
        char *shstrtab_data = malloc(shstrtab_shdr->sh_size); // get real string name of shdr
        lseek(elf_fp, shstrtab_shdr->sh_offset, SEEK_SET);
        
        Assert(read(elf_fp, shstrtab_data, shstrtab_shdr->sh_size) == shstrtab_shdr->sh_size, 
            "Error occurs when reading .shstrtab ");
        /* iterate to read section of .symtab and .strtab to g_ftracetab */
        for(int i = 0; i < ehdr.e_shnum; i++){
           const char *section_name = shstrtab_data + shdr[i].sh_name;// sh_name is a offset
           /* read .symtab section from elf file */
           if(strcmp(section_name, ".symtab") == 0){
                g_ftracetab.symtab = malloc(shdr[i].sh_size);
                Assert(g_ftracetab.symtab != NULL, "Can not allocate %d bytes memory for symtab pointer in g_ftracetab",
                    shdr[i].sh_size);
                lseek(elf_fp, shdr[i].sh_offset, SEEK_SET);
                Assert(read(elf_fp, g_ftracetab.symtab, shdr[i].sh_size) == shdr[i].sh_size, 
            "Error occurs when reading .symtab");
                /* record the number of entries */
                g_ftracetab.symcnt = (shdr[i].sh_size) / (shdr[i].sh_entsize);
           }
           /* read .strtab section from elf file */
           else if(strcmp(section_name, ".strtab") == 0){
                g_ftracetab.strtab = malloc(shdr[i].sh_size);
                Assert(g_ftracetab.strtab, "Can not allocate %d bytes memory for strtab pointer in ftracetab",
                    shdr[i].sh_size);
                lseek(elf_fp, shdr[i].sh_offset, SEEK_SET);
                
                Assert(read(elf_fp, g_ftracetab.strtab, shdr[i].sh_size) == shdr[i].sh_size, 
            "Error occurs when reading .strtab");
           } 
        }

        free(shdr);
        close(elf_fp);
    }

    void init_ftrace(const char* elf_file){
        init_ftracetab(elf_file);
        init_ftracedata();
    }


    void free_ftracetab(){
        free(g_ftracetab.strtab);
        free(g_ftracetab.symtab);
    }

    #define MAXLEN_FNAME 30

    static int get_function_name(vaddr_t target, char *s){
        if(s == NULL) return 0;

        int entcnt = g_ftracetab.symcnt;
        Sym *ptr_sym = g_ftracetab.symtab;
        char *ptr_str = g_ftracetab.strtab;

        for(int i = 0; i < entcnt; i++){
            if((ptr_sym[i].st_info == STT_FUNC) &&
                (target >= ptr_sym[i].st_value) &&
                (target < ptr_sym[i].st_value + ptr_sym[i].st_size)){

                char *ptr_funcname = ptr_str + ptr_sym[i].st_name;
                int string_len =  strnlen(ptr_funcname, MAXLEN_FNAME); //hard encode maxlen of function name to 128.
                strncpy(s, ptr_funcname, string_len+1);
                return string_len;
            }
        }
        /* not found target adress's function name, set name string to "???" */
        memset(s, '?', 3);
        return 0;
        
    }

    static void set_indent(int space_cnt, char *s){
        int loop = space_cnt;

        if(loop < 0) loop = 0;
        while(loop--){
            memset(s, ' ', 1);
            s += 1;
        }

    }


    void ftracedata_write_once(vaddr_t pc, int ft_type, vaddr_t target){
        word_t write_idx = ftracedata.wptr;
        Assert(write_idx < MAX_CALLDEPTH, "ft_logs write overflow! write index greate than %d\n", MAX_CALLDEPTH);
        int space_cnt = 1;
        word_t space_len = 1;

        char fname[MAXLEN_FNAME] = {0};
        char *p = ftracedata.ft_logs[write_idx];
        /* adding prefix pc address */
        p += snprintf(p, sizeof(ftracedata.ft_logs[write_idx]), FMT_WORD ":", pc);

        get_function_name(target, fname);
     
        if(ft_type == ft_ret){
            space_cnt -= 1;
            /* set indent to different line */
            set_indent(space_cnt, p);
            /* adding jump type */
            p += snprintf(p, 4, "ret");
            memset(p, ' ', space_len);
            p+= space_len;

            /* adding target name */
            p += snprintf(p, MAXLEN_FNAME+3, "[%s]", fname);
        }
        else if(ft_type == ft_call){
            space_cnt += 1;
           /* set indent to different line */
           set_indent(space_cnt, p);
           /* adding jump type */
           p += snprintf(p, 5, "call");
           memset(p, ' ', space_len);
           p += space_len;

           /* adding target name and  target address */
           p += snprintf(p, MAXLEN_FNAME+14, "[%s@" FMT_WORD "]", fname, target);
        }

        ftracedata.wptr = write_idx + 1;
    }

    void ftracedata_display(){
        int log_num = ftracedata.wptr;
        for(int i = 0; i < log_num; i++){
            Log("%s", ftracedata.ft_logs[i]);
        } 
    }



#endif
#endif

