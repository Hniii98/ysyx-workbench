#include <trace.h>
#include <common.h>


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
        int elf_fp;
     
        elf_fp = open(elf_file, O_RDONLY);
        read(elf_fp, &ehdr, sizeof(Ehdr));

        shdr = malloc(ehdr.e_shentsize * ehdr.e_shnum); 
        assert(shdr);
        lseek(elf_fp, ehdr.e_shoff, SEEK_SET); 
        read(elf_fp, shdr, ehdr.e_shentsize * ehdr.e_shnum);
        
        Shdr *shstrtab_shdr = &shdr[ehdr.e_shstrndx];         
        char *shstrtab_data = malloc(shstrtab_shdr->sh_size); // get real string name of shdr
        lseek(elf_fp, shstrtab_shdr->sh_offset, SEEK_SET);
        read(elf_fp, shstrtab_data, shstrtab_shdr->sh_size);
        assert(shstrtab_data);

        /* iterate to read section of .symtab and use its sh_link to read .strtab */
        for(int i = 0; i < ehdr.e_shnum; i++){
           const char *section_name = shstrtab_data + shdr[i].sh_name;// sh_name is a offset
           
           /* read .symtab section from elf file */
           if(strcmp(section_name, ".symtab") == 0){
                ftracetab.symtab = malloc(shdr[i].sh_size);
                assert(ftracetab.symtab);
                lseek(elf_fp, shdr[i].sh_offset, SEEK_SET);
                read(elf_fp, ftracetab.symtab, shdr[i].sh_size);
              
           
                /* record the number of entries */
                ftracetab.symcnt = (shdr[i].sh_size) / (shdr[i].sh_entsize);

                int strtab_idx = shdr[i].sh_link;
                Shdr *strtab_shdr = &shdr[strtab_idx];
                ftracetab.strtab = malloc(strtab_shdr->sh_size);
                lseek(elf_fp, strtab_shdr->sh_offset, SEEK_SET);
                read(elf_fp, ftracetab.strtab, strtab_shdr->sh_size);
                assert(ftracetab.strtab);
           }
        }

        free(shdr);
        close(elf_fp);
    }

    void init_ftrace(const char* elf_file){
        assert(elf_file);
        init_ftracetab(elf_file);
        init_ftracedata();
    }


    void free_ftracetab(){
        free(ftracetab.strtab);
        free(ftracetab.symtab);
    }


    static int get_function_name(uint32_t target, char *s){
        if(s == NULL) return 0;

        int entcnt = ftracetab.symcnt;
        Sym *ptr_sym = ftracetab.symtab;
        char *ptr_str = ftracetab.strtab;

        for(int i = 0; i < entcnt; i++){
            if((ST_TYPE(ptr_sym[i].st_info) == STT_FUNC) &&
                (target >= ptr_sym[i].st_value) &&
                (target < ptr_sym[i].st_value + ptr_sym[i].st_size)){

                char *ptr_funcname = ptr_str + ptr_sym[i].st_name;
                int string_len =  strnlen(ptr_funcname, 128); //hard encode maxlen of function name to 128.
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



  void ftracedata_write_once(uint32_t pc, int ft_type, uint32_t target){
    uint32_t write_idx = ftracedata.wptr;

    char fname[128] = {0};
    char *p = ftracedata.ftrace_logs[write_idx];

    /* adding prefix*/
    p += snprintf(p, sizeof(ftracedata.ftrace_logs[write_idx]), FMT_WORD ":", pc);

    get_function_name(target, fname);

    /* decide intdetn depth */
    int current_indent = indent_level;
    if (ft_type == ft_ret) current_indent--;  // ret inst need to left indetn a step
    if (current_indent < 0) current_indent = 0;

    /* adding indent */
    p += set_indent(current_indent, p);

    if (ft_type == ft_ret) {
        p += snprintf(p, 4, "ret");
        *p++ = ' ';
        p += snprintf(p, 128, "[%s]", fname);
        indent_level--;  // after a ret, indet move left
        if (indent_level < 0) indent_level = 0;
    } 
    else if (ft_type == ft_call) {
        p += snprintf(p, 5, "call");
        *p++ = ' ';
        p += snprintf(p, 128, "[%s@" FMT_WORD "]", fname, target);
        indent_level++;  // after a call, indent move right 
    }

    /* circle buffer, write around */
    ftracedata.rptr = ftracedata.wptr;
    ftracedata.wptr = (write_idx + 1) % MAX_CALLDEPTH;
}



    void ftracedata_display_once(){
        size_t index = ftracedata.rptr;
        printf("%s\n", ftracedata.ftrace_logs[index]);
    }

    void write_uncondjump_trace(uint32_t pc, uint8_t rd, uint32_t target){
    /*  standard calling convention: 
        uses 'x1', as well as rd = 1, as return address reg.
        uses 'x5', as well as rd = 5, as an alternate link register. 
        when we do not need back to this funciton , rd = 0, jump will
        be a return. */
    if(rd == 0){
        /* rd = 0, this jump is a return .*/
        ftracedata_write_once(pc, ft_ret, target);
    }
    else{
        /* else, this jump is a funcion call. */
        ftracedata_write_once(pc, ft_call, target);
    }
}

    // void ftracedata_display(){
    //     int log_num = ftracedata.wptr;
    //     printf("---- function name trace ----\n");
    //     printf("---- function name trace ----(capacity: %d defined by MAX_CALLDEPTH" \
    //         " in trace.h)----\n", MAX_CALLDEPTH); 
            
    //     for(int i = 0; i < log_num; i++){
    //         printf("%s\n", ftracedata.ftrace_logs[i]);
    //         printf("%s\n", ftracedata.ftrace_logs[i]);
    //     } 
    // }


#endif

