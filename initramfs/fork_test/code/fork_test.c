#include "printf.h"
#include "uart.h"
#include "syscall.h"

// void main(int argc, char **argv)
// {
//     init_printf(0, putc);
//     // while (1)
//     int counter = 4;
//     while (counter--)
//     {
//         for (int i = 0; i < 10000000; i++)
//         {
//             if (i % 2000000 == 0)
//             {
//                 printf("[child] pid: %d %d\n", getpid(), i);
//                 printf("%d ", argc);
//                 for (int j = 0; j < argc; j++)
//                     printf("%s ", argv[j]);
//                 printf("\n");
//             }
//         }
//     }
//     exit();
// }

// void delay(unsigned int count)
// {
//     while (count--)
//         ;
// }

void main(void) 
{
    init_printf(0, putc);
    printf("Fork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    // child
    if ((ret = fork()) == 0) 
    {
        printf("pid: %d, cnt: %d, ptr: %x\n", getpid(), cnt, &cnt);
        ++cnt;
        fork();
        while (cnt < 5) 
        {
            printf("pid: %d, cnt: %d, ptr: %x\n", getpid(), cnt, &cnt);
            // delay(1000000);
            ++cnt;
        }
        exit();
    }
    else 
    {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
        exit();
    }
}
