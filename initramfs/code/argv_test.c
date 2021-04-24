#include "printf.h"
#include "uart.h"

int main(int argc, char **argv)
{
    init_printf(0, putc);
    while (1)
    {
        for (int i = 0; i < 100000000; i++)
        {
            if (i % 10000000 == 0)
            {
                printf("%d\n", argc);
                for (int j = 0; j < argc; j++)
                    printf("%s\n", argv[j]);
            }
        }
    }
}
