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
//                 printf("[CHILD] pid: %d %d\n", getpid(), i);
//                 printf("%d ", argc);
//                 for (int j = 0; j < argc; j++)
//                     printf("%s ", argv[j]);
//                 printf("\n");
//             }
//         }
//     }
//     exit();
// }

void main(int argc, char **argv) 
{
    init_printf(0, putc);
    printf("Argv Test, pid %d\n", getpid());
    for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
    // exit();
    const char *fork_argv[] = {"fork_test", 0};
    exec("fork_test.img", fork_argv);
}
