/*
Name: Guguloth Lokesh
Roll no : 2401CS37
Course: CS2206
Declaration of Authorship:
I declare that this program is my own work. I have not copied
it from any other student or source except where explicitly
acknowledged. This work was completed as part of the course
requirements.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NO_OF_LABLES 600
#define MAX_LINE_LEN 600
#define MAX_LABLE_LEN 100

#define OPERAND_MIN -8388608
#define OPERAND_MAX 8388607

typedef struct {
    char mnemonic[10];
    int opcode;
    int has_operand;
} instruction;

typedef struct {
    char name[MAX_LABLE_LEN];
    int address;
} symbol;

/* instruction table */
instruction instruction_table[] = {
    {"ldc",0,1},
    {"adc",1,1},
    {"ldl",2,1},
    {"stl",3,1},
    {"ldnl",4,1},
    {"stnl",5,1},
    {"add",6,0},
    {"sub",7,0},
    {"shl",8,0},
    {"shr",9,0},
    {"adj",10,1},
    {"a2sp",11,0},
    {"sp2a",12,0},
    {"call",13,1},
    {"return",14,0},
    {"brz",15,1},
    {"brlz",16,1},
    {"br",17,1},
    {"HALT",18,0},
};

int instruction_count = 19;

symbol symbol_table[MAX_NO_OF_LABLES];
int symbol_cnt = 0;

int error_cnt = 0;
FILE *log_file = NULL; /*for errors*/
/*errors writing to a file*/
void report_error(char msg[], int line){
    if(log_file)
        fprintf(log_file,"Error at line %d : %s\n",line,msg);
    error_cnt++;
}

/*check if a instruction exists*/
int check_for_inst(char *name){
    int i;
    for(i=0;i<instruction_count;i++){
        if(strcmp(name,instruction_table[i].mnemonic)==0)
            return i;
    }
    return -1;
}

/*check if the label is present*/
int check_for_label(char *name){
    int i;
    for(i=0;i<symbol_cnt;i++){
        if(strcmp(name,symbol_table[i].name)==0)
            return symbol_table[i].address;
    }
    return -1;
}

/*check if label is valid*/
int valid_label(char *name){
    if(!isalpha(name[0]))
        return 0;
    int i;
    for(i=1;name[i]!='\0';i++){
        if(!isalnum(name[i]) && name[i]!='_')
            return 0;
    }
    return 1;
}

/*add  label(symbol) to symbol table*/
void add_symbol(char *name,int addr,int line){

    if(!valid_label(name)){
        report_error("Invalid label name",line);
        return;
    }

    if(symbol_cnt>=MAX_NO_OF_LABLES){
        report_error("Symbol table overflow",line);
        return;
    }

    if(check_for_label(name)!=-1){
        report_error("Duplicate label",line);
        return;
    }

    strcpy(symbol_table[symbol_cnt].name,name);
    symbol_table[symbol_cnt].address=addr;
    symbol_cnt++;
}

/*remove comments from line*/
void remove_cmt(char *line){
    char *p=strchr(line,';');
    if(p) *p='\0';
}
/*remove spaces from line*/
void remove_spaces(char *str){

    char *start=str;

    while(isspace(*start))
        start++;

    if(*start=='\0'){
        *str='\0';
        return;
    }

    if(start!=str)
        memmove(str,start,strlen(start)+1);

    char *end=str+strlen(str)-1;

    while(end>=str && isspace(*end)){
        *end='\0';
        end--;
    }
}

/*convert a string operand into integer value*/
int parse_number(char *str,int *result){

    char *endptr;
    long val=strtol(str,&endptr,0);
    if(endptr == str || *endptr != '\0')
        return 0; /* invalid number */

    *result=(int)val;
    return 1;
}

/* PASS 1 symbol table */
void print_symbol_table(){

    printf("\n---------------- Pass_1 ----------------\n\n");

    printf("%-10s %-10s\n","LABEL","ADDRESS");
    printf("----------------------\n");
    int i;
    for( i=0;i<symbol_cnt;i++){
        printf("%-10s %-10d\n",
        symbol_table[i].name,
        symbol_table[i].address);
    }

    printf("\n");
}

/* PASS 1 */
void pass1(FILE *fp){

    char line[MAX_LINE_LEN];
    int pc=0;
    int line_no=0;

    while(fgets(line,MAX_LINE_LEN,fp)){

        line_no++;

        remove_cmt(line);
        remove_spaces(line);

        if(strlen(line)==0)
            continue;

        char label[MAX_LABLE_LEN]="";

        if(strchr(line,':')){
            int i=0;
            while(line[i]!=':' && line[i]!='\0'){
                label[i]=line[i];
                i++;
            }
            label[i]='\0';

            remove_spaces(label);

            i++;
            while(line[i]==' '||line[i]=='\t')
                i++;

            char inst[50], operand[50], extra[50];
            int count = sscanf(line+i,"%s %s %s",inst,operand,extra);

            /*  HANDLE SET*/
            if(count >= 1 && strcmp(inst,"SET")==0){

                if(strlen(label)==0){
                    report_error("SET must have label",line_no);
                    continue;
                }

                if(count < 2){
                    report_error("Missing operand for SET",line_no);
                    continue;
                }

                if(count > 2){
                    report_error("Extra operand for SET",line_no);
                    continue;
                }

                int val;
                if(!parse_number(operand,&val)){
                    report_error("Invalid SET value",line_no);
                    continue;
                }

                if(val < OPERAND_MIN || val > OPERAND_MAX){
                    report_error("SET value out of range",line_no);
                    continue;
                }

                add_symbol(label,val,line_no);
                continue;   /* we do not increment the pc as it is set instruction */
            }

            /*  NORMAL LABEL  */
            add_symbol(label,pc,line_no);

            if(line[i] != '\0'){
                char temp_inst[50];
                sscanf(line+i,"%s",temp_inst);
                if(strcmp(temp_inst,"SET") != 0){
                    pc++; 
                }
            }
        }
        else{
            char inst[50];
            if(sscanf(line,"%s",inst)==1){
                if(strcmp(inst,"SET")!=0){
                    pc++;
                }
            }
        }
    }
}




/* PASS 2 */
void pass2(FILE *fp, FILE *obj, FILE *lst)
{
    char line[MAX_LINE_LEN];

    int pc = 0;
    int line_no = 0;
    int printed = 0;

    while(fgets(line, MAX_LINE_LEN, fp)){

        line_no++;

        char original[MAX_LINE_LEN];

        remove_cmt(line);

        strcpy(original,line);
        original[strcspn(original, "\n")] = '\0';

        remove_spaces(line);

        if(strlen(line)==0)
            continue;

        char label[MAX_LABLE_LEN]="";
        char inst[100];
        char operand[100];

        char *ptr=line;

        /* remove label */
        if(strchr(ptr,':')){
            sscanf(ptr,"%[^:]:",label);
            ptr=strchr(ptr,':')+1;
            remove_spaces(ptr);
        }

        if(sscanf(ptr,"%s",inst)!=1)
            continue;

        /* handle SET */
        if(strcmp(inst,"SET")==0){
            continue;
        }


        /* handle data */
        if(strcmp(inst,"data")==0){

            int val=0;
            char extra[50];

            int count = sscanf(ptr,"%s %s %s",inst,operand,extra);

            if(count < 2){
                report_error("Missing operand for data",line_no);
                pc++;
                continue;
            }

            if(count > 2){
                report_error("Extra operand for data",line_no);
                pc++;
                continue;
            }

            if(!parse_number(operand,&val)){
                report_error("Invalid data value",line_no);
                pc++;
                continue;
            }

            int machinecode = val;

            fwrite(&machinecode,sizeof(int),1,obj);


            char label_print[50]="";

            for(int i=0;i<symbol_cnt;i++){
                if(symbol_table[i].address==pc){
                    strcpy(label_print,symbol_table[i].name);
                    break;
                }
            }


            if(printed==0){

                printf("---------------- Pass_2 ----------------\n\n");

                printf("------------------------------------------------------\n");
                printf("%-10s %-10s %-15s %s\n",
                "ADDRESS","LABEL","MACHINE CODE","SOURCE");
                printf("------------------------------------------------------\n");


                fprintf(lst,"------------------------------------------------------\n");
                fprintf(lst,"%-10s %-10s %-15s %s\n",
                "ADDRESS","LABEL","MACHINE CODE","SOURCE");
                fprintf(lst,"------------------------------------------------------\n");
            }


            fprintf(lst,"%08X %-10s %08X %s\n",
            pc,label_print,machinecode,original);

            printf("%08X %-10s %08X %s\n",
            pc,label_print,machinecode,original);


            printed++;
            pc++;
            continue;
        }


        /* normal instruction */

        int idx=check_for_inst(inst);

        if(idx==-1){
            report_error("Unknown instruction",line_no);
            pc++;
            continue;
        }


        int operand_val=0;
        char extra[100];

        int count=sscanf(ptr,"%s %s %s",inst,operand,extra);


        if(instruction_table[idx].has_operand){

            if(count<2){
                report_error("Missing operand",line_no);
                pc++;
                continue;
            }


            if(count>2){
                report_error("Extra input",line_no);
                pc++;
                continue;
            }


            int l=check_for_label(operand);


            if(l!=-1){

                if(strcmp(inst,"br")==0 ||
                   strcmp(inst,"brz")==0 ||
                   strcmp(inst,"brlz")==0 ||
                   strcmp(inst,"call")==0)

                    operand_val=l-(pc+1);

                else
                    operand_val=l;

            }
            else{

                if(isalpha(operand[0])){
                    report_error("No such label",line_no);
                    pc++;
                    continue;
                }


                if(!parse_number(operand,&operand_val)){
                    report_error("Invalid operand",line_no);
                    pc++;
                    continue;
                }


                if(operand_val < OPERAND_MIN ||
                   operand_val > OPERAND_MAX){

                    report_error("Operand exceeds 24-bit range",line_no);
                    pc++;
                    continue;
                }
            }

        }
        else{

            if(count>1){
                report_error("Unexpected operand",line_no);
                pc++;
                continue;
            }
        }


        int machinecode =
        ((operand_val&0xFFFFFF)<<8) |
        (instruction_table[idx].opcode);


        fwrite(&machinecode,sizeof(int),1,obj);


        char label_print[50]="";


        for(int i=0;i<symbol_cnt;i++){

            if(symbol_table[i].address==pc){

                strcpy(label_print,symbol_table[i].name);
                break;
            }
        }


        if(printed==0){

            printf("---------------- Pass_2 ----------------\n\n");

            printf("------------------------------------------------------\n");
            printf("%-10s %-10s %-15s %s\n",
            "ADDRESS","LABEL","MACHINE CODE","SOURCE");
            printf("------------------------------------------------------\n");


            fprintf(lst,"------------------------------------------------------\n");
            fprintf(lst,"%-10s %-10s %-15s %s\n",
            "ADDRESS","LABEL","MACHINE CODE","SOURCE");
            fprintf(lst,"------------------------------------------------------\n");
        }


        fprintf(lst,"%08X %-10s %08X %s\n",
        pc,label_print,machinecode,original);


        printf("%08X %-10s %08X %s\n",
        pc,label_print,machinecode,original);


        printed++;
        pc++;
    }


    if(printed==0){

        printf("\nNo valid instructions. PASS 2 not displayed.\n");

    }
}

/* main */
int main(int argc,char *argv[]){

    if(argc<2){
        printf("file is not provided\n");
        return 0;
    }

    FILE *fp=fopen(argv[1],"r");

    if(!fp){
        printf("Cannot open file\n");
        return 0;
    }

    char basename[200];
    char objname[200];
    char lstname[200];
    char logname[200];
    

    strcpy(basename,argv[1]);

    char *dot=strrchr(basename,'.');
    if(dot) *dot='\0';

    sprintf(objname,"%s.obj",basename);
    sprintf(lstname,"%s.lst",basename);
    sprintf(logname,"%s.log",basename);
    
    
    log_file=fopen(logname,"w");
    FILE *obj=fopen(objname,"wb");
    FILE *lst=fopen(lstname,"w");

    pass1(fp);
    print_symbol_table();

    rewind(fp);

    pass2(fp,obj,lst);

    fclose(fp);
    fclose(obj);
    fclose(lst);
    fclose(log_file);
    

    if(error_cnt>0)
        printf("\nProgram finished with %d errors\n",error_cnt);
    else
        printf("\nProgram finished with zero errors\n");

    return 0;
}
