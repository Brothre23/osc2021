#include "printf.h"
#include "string.h"
#include "cpio.h"
#include "printf.h"
#include "sysregs.h"
#include "mm.h"

int hex2int(char *hex)
{
    int value = 0;
    for (int i = 0; i < 8; i++)
    {
        // get current character then increment
        char byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9')
            byte = byte - '0';
        else if (byte >= 'A' && byte <= 'F')
            byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        value = (value << 4) | byte;
    }
    return value;
}

int round2four(int origin, int option)
{
    int answer = 0;

    switch (option)
    {
    case 1:
        if ((origin + 6) % 4 > 0)
            answer = ((origin + 6) / 4 + 1) * 4 - 6;
        else
            answer = origin;
        break;

    case 2:
        if (origin % 4 > 0)
            answer = (origin / 4 + 1) * 4;
        else
            answer = origin;
        break;

    default:
        break;
    }

    return answer;
}

void cpio_read(char **address, char *target, int count)
{
    while (count--)
    {
        *target = **address;
        (*address)++;
        target++;
    }
}

int cpio_parse_header(char **ramfs, char *file_name, char **file_content)
{
    struct cpio_newc_header header;
    int file_size = 0, name_size = 0;

    cpio_read(ramfs, header.c_magic, 6);
    (*ramfs) += 48;
    cpio_read(ramfs, header.c_filesize, 8);
    (*ramfs) += 32;
    cpio_read(ramfs, header.c_namesize, 8);
    (*ramfs) += 8;

    name_size = round2four(hex2int(header.c_namesize), 1);
    file_size = round2four(hex2int(header.c_filesize), 2);

    cpio_read(ramfs, file_name, name_size);
    *file_content = *ramfs;
    (*ramfs) += file_size;

    file_name[name_size] = '\0';

    return file_size;
}

void cpio_ls()
{
    char file_name[32];
    char *file_content;

    char *ramfs = (char *)0x8000000;

    while (1)
    {
        strset(file_name, '0', 32);
        cpio_parse_header(&ramfs, file_name, &file_content);

        if ((strcmp(file_name, "TRAILER!!!") == 0))
            break;

        printf(file_name);
        printf("\n");
    }
}

void cpio_find_file(char file_name_to_find[])
{
    char file_name[32];
    char *file_content;

    char *ramfs = (char *)0x8000000;
    int found = 0;

    while (1)
    {
        strset(file_name, '0', 32);
        cpio_parse_header(&ramfs, file_name, &file_content);

        if ((strcmp(file_name, file_name_to_find) == 0))
        {
            found = 1;
            break;
        }
        else if ((strcmp(file_name, "TRAILER!!!") == 0))
            break;
    }

    if (found)
        printf("%s", file_content);
    else
        printf("FILE NOT FOUND!");

    printf("\n");
}

void *cpio_run_program(char program_name[])
{
    char *ramfs = (char *)0x8000000;
    char file_name[32], *file_content;

    int file_size;

    while (1)
    {
        strset(file_name, '0', 32);
        file_size = cpio_parse_header(&ramfs, file_name, &file_content);

        if (strcmp(file_name, program_name) == 0)
            break;
        else if ((strcmp(file_name, "TRAILER!!!") == 0))
            return 0;
    }

    char *program_start = NULL;
    if (strcmp(program_name, "argv_test.img") == 0)
        program_start = (char *)0x10A0000;
    else if (strcmp(program_name, "fork_test.img") == 0)
        program_start = (char *)0x10B0000;
    else if (strcmp(program_name, "vfs_test.img") == 0)
        program_start = (char *)0x10C0000;

    for (int i = 0; i < file_size; i++)
        *(program_start + i) = *(file_content + i);

    return (void *)program_start;
}
