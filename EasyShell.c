#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>

char *divide(char *s, const char *delim, char **save_ptr) {  
    char *token;  
  
    if (s == NULL) s = *save_ptr;  
  
    /* Scan leading delimiters.  */  
    s += strspn(s, delim);  
    if (*s == '\0')   
        return NULL;  
  
    /* Find the end of the token.  */  
    token = s;  //保存当前的待分解串的指针
    s = strpbrk(token, delim);  //在token中找分界符
    if (s == NULL)  
        /* This token finishes the string.  */  
        *save_ptr = strchr(token, '\0');  
    else {  
        /* Terminate the token and make *SAVE_PTR point past it.  */  
        *s = '\0';  //截断
        *save_ptr = s + 1;  
    }  
  
    return token;  
}  

int main() {
    int pid;
    int rtn; /*子进程的返回数值*/
    int exec_errorno;
    char command[256];
    char *p;


    while (1) {
        /* 从终端读取要执行的命令 */
        printf("MyShell@Bock:");
        command[0] = '\0';
        p = fgets(command, 256, stdin);
        if (p == NULL) {
            perror("Error in fgets()");
            exit(-1);
        }

        // Delete the last char (new line) in the string returned by fgets()
        command[strlen(command) - 1] = '\0';

        int count = 0;
        char *buf = command;
        char *arg[20];
        char *inner_ptr = NULL;
        while ((arg[count] = divide(buf, " ", &inner_ptr)) != NULL) 
        {
	    count++;
	    buf = NULL;
        }
        arg[count] = NULL;

        // Quit if user inputs "quit"
        if (!strcmp(arg[0], "quit")) {
            break;
        }

        // Create a child process to execute command
        pid = fork();
        if (pid < 0) {
            perror("Failed while calling fork");
            exit(-1);
        } 
        else if (pid == 0) {

            //考虑重定向;在字符串数组中找重定向标志
            int i = 0;
            int flag = 0;
            for (; arg[i] != NULL; ++i )
            {
                if (strcmp(">", arg[i]) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            arg[i] = NULL;
            if(flag)
            {
                if (arg[i+1] == NULL)
                {
                    printf("command error\n");
                    exit(1);
                }
                close(fileno(stdout)); 
                int out_fd = open(arg[i+1], O_CREAT|O_RDWR, 0666);
                dup2(out_fd, fileno(stdout));
            }

            i = 0;
            flag = 0;
            for (; arg[i] != NULL; ++i )
            {
                if (strcmp("<", arg[i]) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            arg[i] = NULL;
            if(flag)
            {
                if (arg[i+1] == NULL)
                {
                    printf("command error\n");
                    exit(1);
                }
                close(fileno(stdin)); 
                int in_fd = open(arg[i+1], O_CREAT |O_RDONLY, 0666);
                dup2(in_fd, fileno(stdin));
            }

            /* 子进程执行此命令 */
            exec_errorno = execvp(arg[0], arg);
            /* 如果exec函数返回，表明没有正常执行命令 */ 
            /* 只有在这种情况下，才会执行下面的打印错误信息*/
            perror(command);

            exit(exec_errorno);
        }
        else {
            /* 父进程， 等待子进程结束，并打印子进程的返回值 */
            wait(&rtn);
            printf("\nValue returned from child process, rtn = %d\n", rtn);
            printf("WEXITSTATUS(rtn) = %d\n", WEXITSTATUS(rtn));
        }
    }

    return 0;
}

