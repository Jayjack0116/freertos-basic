#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"
#define base 1000000000
typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
int fibonacci(int );
void test_fibonacci_ov(int);
void fib_over_46(int);
int stio(char *);
int get_9_digits(int);
void _command(int, char **);
void new_command(int, char **);
void update_info(void *);


#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	MKCL(new, "Build new task"),
	MKCL(, ""),
};

void new_command(int n, char *argv[]){
	fio_printf(1,"\r\n");

	if(n == 2){
		switch(*argv[1]){
			case 'u':
				xTaskCreate(update_info,
	            (signed portCHAR *) "UPDATE",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);
	            break;
	         default:
	         	fio_printf(1,"Invalid command.\r\n");
	         	break;
		}
	}
	else
		fio_printf(1,"Please enter new [condition] \r\n");		
}

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        fio_printf(1, "%d\n", dir);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
	(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
    fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
    fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }

}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void test_command(int n, char *argv[]) {
    int handle;
    int error;

    fio_printf(1, "\r\n");
    
    if(n >= 3){
    	if(*argv[1] == 'f'){
    		if(*argv[2] != 0 ){//run fibonacci
    			int count = stio(argv[2]);
    			test_fibonacci_ov(count);
    		}
    	}
	}
	else{
		fio_printf(1, "Invalid command!\r\n");
	}

    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");

    handle = host_action(SYS_OPEN, "output/syslog", 8);
    if(handle == -1) {
        fio_printf(1, "Open file error!\n\r");
        return;
    }

    char *buffer = "Test host_write function which can write data to output/syslog\n";
    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
    if(error != 0) {
        fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
        host_action(SYS_CLOSE, handle);
        return;
    }

    host_action(SYS_CLOSE, handle);
}

int fibonacci(int x){
	int previous = -1;
  	int result = 1;
  	int i=0;
  	int sum=0;
  	for (i = 0; i <= x; i++) {
    	sum = result + previous;
	    previous = result;
	    result = sum;
	}
	return result;

}

void test_fibonacci_ov(int count){

	if( count <= 46 ){
		fio_printf(1, "The fibonacci at %d element is : %d\n\r ",count,fibonacci(count));
	}
	else //overflow
		fib_over_46(count);

}

void fib_over_46(int count){//up limit to 79 element
	int a = fibonacci(45);
	int b = fibonacci(46);
	int ha = a / base;//10-digit num
	int hb = b / base;//10-digit num
	int i,sum = 0;
	int ac = 0 ;
	int x = count -47;
	a = get_9_digits(a);
	b = get_9_digits(b);

	for(i = 0; i <= x ; i++){
		ac = ha + hb;
		ac += (a+b) / base;
		sum = get_9_digits(a+b);
		a = b;
		b = sum;
		ha = hb;
		hb = ac;
	}
	fio_printf(1, "The fibonacci at %d element is : %d%d\n\r ",count,ac,sum);
}

int stio(char *str){
	int i,result = 0; 
	int count = strlen(str);
	for(i = 0; i< count ; i++){
		result = result * 10 + (str[i] - '0');
	}
	return result;
}

int get_9_digits(int num){
	int x = num / base;
	return (num - x * base);
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

void update_info(void *para){
	while(1){
		//update information to host systeminfo
		int i;
		int handle;
    	int error;

		for ( i =0 ; i <999999999 ; i++ ) {}
		handle = host_action(SYS_SYSTEM, "mkdir -p output");
	    handle = host_action(SYS_SYSTEM, "touch output/syslog");

	    handle = host_action(SYS_OPEN, "output/syslog", 8);
	    if(handle == -1) {
	        fio_printf(1, "Open file error!\n\r");
	        return;
	    }

	    char *buffer = "Update!!!\n";
	    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
	    if(error != 0) {
	        fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
	        host_action(SYS_CLOSE, handle);
	        return;
	    }

	    host_action(SYS_CLOSE, handle);
	}
}
 


cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
