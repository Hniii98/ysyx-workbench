#include <trace.h>



#ifndef CONFIG_TARGET_AM
#ifdef CONFIG_IRINGTRACE
/* Instruction ring buffer */
    static IRingBuf iringbuf;

    void init_iringbuf(){
        memset(&iringbuf, 0, sizeof(IRingBuf));
    }


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
        /* don't need file name / line num / function name, so use this two instead of Log() */
        printf("---- instructions ring buffer ----\n");
        log_write("---- instructions ring buffer (capacity: %d defined by RBUFSIZE in trace.h)----\n", RBUFSIZE);

        for(size_t i = 0; i < RBUFSIZE; i++){
            /* empty string, break loop */
            if(strlen(iringbuf.ringbuf[i]) == 0) break;

            if((i + 1) % RBUFSIZE == iringbuf.wptr){
                printf(ANSI_FMT("  ==> %s", ANSI_FG_RED) " \n", iringbuf.ringbuf[i]);
                log_write(ANSI_FMT("  ==> %s", ANSI_FG_RED) " \n", iringbuf.ringbuf[i]);
            }
            else{
                printf("%s \n", iringbuf.ringbuf[i]);
                log_write("%s \n", iringbuf.ringbuf[i]);
            }
        }
    }
#endif



#ifdef CONFIG_MTRACE


bool mtrace_enable(paddr_t addr){
    return MUXDEF(CONFIG_MTRACE, (addr >= CONFIG_MTRACE_START) && 
        (addr <= CONFIG_MTRACE_END), false);
}

void mwrite_trace(paddr_t addr, int len, word_t data){
    /* shell can perfectly display ANSI color */
    printf(ANSI_FMT("\tnemu: write lower %d bytes of { 0x%08x } to memory at " FMT_PADDR, ANSI_FG_WHITE) "\n",
        len,
        data,
        addr); 

    /* txt file ANSI format affect readability */
    log_write("\tnemu: write lower %d bytes of { 0x%08x } to memory at " FMT_PADDR "\n",
        len,
        data,
        addr); 
}

void mread_trace(paddr_t addr, int len){

    printf(ANSI_FMT("\tnemu: read %d bytes of memory at " FMT_PADDR, ANSI_FG_WHITE) "\n",
        len,
        addr);

    log_write("\tnemu: read %d bytes of memory at " FMT_PADDR "\n",
        len,
        addr);
}

#endif

#ifndef CONFIG_TARGET_AM
#ifdef CONFIG_FTRACE

    static FTraceData ftracedata;
    static FTraceTab ftracetab;  

    void init_ftracedata(){
        memset(&ftracedata, 0, sizeof(FTraceData));
    }
    
    /* Get the .symtab and .strtab section by parsing elf file */
    void init_ftracetab(const char* elf_file){
      
        memset(&ftracetab, 0, sizeof(FTraceTab));
      
        Ehdr ehdr;
        Shdr *shdr = NULL;
        int elf_fp;;
     
        elf_fp = open(elf_file, O_RDONLY);
        Assert(elf_fp >= 0, "Can not open given elf_file: %s", elf_file);
        Assert(read(elf_fp, &ehdr, sizeof(Ehdr)) == sizeof(Ehdr), 
            "Error occurs when reading ehdr"); // read elf file header

        shdr = malloc(ehdr.e_shentsize * ehdr.e_shnum); 
        Assert(shdr != NULL, "Can not allocate %d bytes memory for section header table.",
            ehdr.e_shentsize * ehdr.e_shnum);
        lseek(elf_fp, ehdr.e_shoff, SEEK_SET); 
        Assert(read(elf_fp, shdr, ehdr.e_shentsize * ehdr.e_shnum) == ehdr.e_shentsize * ehdr.e_shnum, 
            "Error occurs when reading shdr"); // read elf file header
       
        Shdr *shstrtab_shdr = &shdr[ehdr.e_shstrndx];         
        char *shstrtab_data = malloc(shstrtab_shdr->sh_size); // get real string name of shdr
        lseek(elf_fp, shstrtab_shdr->sh_offset, SEEK_SET);
        Assert(read(elf_fp, shstrtab_data, shstrtab_shdr->sh_size) == shstrtab_shdr->sh_size, 
            "Error occurs when reading .shstrtab ");

        /* iterate to read section of .symtab and use its sh_link to read .strtab */
        for(int i = 0; i < ehdr.e_shnum; i++){
           const char *section_name = shstrtab_data + shdr[i].sh_name;// sh_name is a offset
           
           /* read .symtab section from elf file */
           if(strcmp(section_name, ".symtab") == 0){
                ftracetab.symtab = malloc(shdr[i].sh_size);
                Assert(ftracetab.symtab != NULL, "Can not allocate %d bytes memory for symtab pointer in ftracetab",
                    shdr[i].sh_size);
                lseek(elf_fp, shdr[i].sh_offset, SEEK_SET);
                Assert(read(elf_fp, ftracetab.symtab, shdr[i].sh_size) == shdr[i].sh_size, 
            "Error occurs when reading .symtab");
                /* record the number of entries */
                ftracetab.symcnt = (shdr[i].sh_size) / (shdr[i].sh_entsize);

                int strtab_idx = shdr[i].sh_link;
                Shdr *strtab_shdr = &shdr[strtab_idx];
                ftracetab.strtab = malloc(strtab_shdr->sh_size);
                lseek(elf_fp, strtab_shdr->sh_offset, SEEK_SET);
                Assert(read(elf_fp, ftracetab.strtab, strtab_shdr->sh_size) == strtab_shdr->sh_size, 
            "Error occurs when reading .strtab");       
                break;
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
        free(ftracetab.strtab);
        free(ftracetab.symtab);
    }

    #define MAXLEN_FNAME 30

    static int get_function_name(vaddr_t target, char *s){
        if(s == NULL) return 0;

        int entcnt = ftracetab.symcnt;
        Sym *ptr_sym = ftracetab.symtab;
        char *ptr_str = ftracetab.strtab;

        for(int i = 0; i < entcnt; i++){
            if((ST_TYPE(ptr_sym[i].st_info) == STT_FUNC) &&
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

    static int indent_level = 0;

   static int set_indent(int space_cnt, char *s) {
        int written = 0;
        if (space_cnt < 0) space_cnt = 0;

        for (int i = 0; i < space_cnt * 2; i++) {
            *s++ = ' ';
            written++;
        }
        return written;
    }



  void ftracedata_write_once(vaddr_t pc, int ft_type, vaddr_t target){
    word_t write_idx = ftracedata.wptr;

    Assert(write_idx < MAX_CALLDEPTH, "ft_logs write overflow! write index greater than %d\n", MAX_CALLDEPTH);

    char fname[MAXLEN_FNAME] = {0};
    char *p = ftracedata.ft_logs[write_idx];

    // 打印地址前缀
    p += snprintf(p, sizeof(ftracedata.ft_logs[write_idx]), FMT_WORD ":", pc);

    get_function_name(target, fname);

    // 决定缩进深度，但先不要修改 indent_level
    int current_indent = indent_level;
    if (ft_type == ft_ret) current_indent--;  // ret 要少一级缩进
    if (current_indent < 0) current_indent = 0;

    // 添加缩进空格
    p += set_indent(current_indent, p);

    if (ft_type == ft_ret) {
        p += snprintf(p, 4, "ret");
        *p++ = ' ';
        p += snprintf(p, MAXLEN_FNAME+3, "[%s]", fname);
        indent_level--;  // 回到上一层
        if (indent_level < 0) indent_level = 0;
    } 
    else if (ft_type == ft_call) {
        p += snprintf(p, 5, "call");
        *p++ = ' ';
        p += snprintf(p, MAXLEN_FNAME+14, "[%s@" FMT_WORD "]", fname, target);
        indent_level++;  // 进入新一层
    }

    ftracedata.wptr = write_idx + 1;
}


    void ftracedata_display(){
        int log_num = ftracedata.wptr;
        printf("---- function name trace ----\n");
        log_write("---- function name trace ----(capacity: %d defined by MAX_CALLDEPTH" \
            " in trace.h)----\n", MAX_CALLDEPTH); 
            
        for(int i = 0; i < log_num; i++){
            printf("%s\n", ftracedata.ft_logs[i]);
            log_write("%s\n", ftracedata.ft_logs[i]);
        } 
    }


#endif
#endif

#endif


