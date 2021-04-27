#include "printf.h"
#include "uart.h"
#include "syscall.h"

int main(int argc, char **argv)
{
    init_printf(0, putc);
    // while (1)
    int counter = 4;
    while (counter--)
    {
        for (int i = 0; i < 10000000; i++)
        {
            if (i % 2000000 == 0)
            {
                printf("[child] pid: %d %d\n", getpid(), i);
                printf("%d ", argc);
                for (int j = 0; j < argc; j++)
                    printf("%s ", argv[j]);
                printf("\n");
            }
        }
    }
    exit();
}
