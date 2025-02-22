#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "./string.h"
#include "common.h"
#include "debug.h"

#define ctrl(x) (x & 0xf)

#if defined(CR_DEBUG_MODE)
static void show_input_buff(char const buff[static 1], size_t written)
{
    CR_DEBUG("read %zd characters: [", written);
    for (size_t i = 0; i < written; i++) {
        if (i)
            fprintf(stderr, " ");
        fprintf(stderr, "%.2x", buff[i]);
    }
    fprintf(stderr, "]\n");
    if (*buff == '\033')
        CR_DEBUG("Detected ascii sequence: [\\033%s]\n", &buff[1]);
}
#endif

static
size_t strcpy_printable(char *dest, char *src)
{
    size_t count = 0;

    for (; *src != '\0'; src++) {
        if (isprint(*src)) {
            *dest = *src;
            count++;
            dest++;
        }
    }
    return count;
}

static
bool append_null_terminator(buff_t *buff)
{
    if (!ensure_buff_capacity(buff))
        return false;
    buff->str[buff->count] = '\0';
    buff->count++;
    return true;
}

bool cr_getline(buff_t *buff)
{
    char read_buff[32] = "";
    ssize_t read_size = 0;

    if (!ensure_buff_capacity(buff))
        return false;
    while (*read_buff != '\n' && *read_buff != '\r') {
        memset(read_buff, '\0', sizeof read_buff);
        read_size = read(STDIN_FILENO, &read_buff, sizeof read_buff);
        CR_DEBUG("read count: %zd\n", read_size);
        if (read_size < 0)
            return false;
        if (read_size == 0) {
            buff->count = 0;
            return true;
        }
        if (*read_buff == ctrl('d'))
            return write(STDOUT_FILENO, SSTR_UNPACK("exit\n")), true;
        CR_DEBUG_CALL(show_input_buff, read_buff, read_size);
        if (str_printable(read_buff))
            write(STDOUT_FILENO, read_buff, read_size);
        if (!ensure_buff_capacity(buff))
            return false;
        buff->count += strcpy_printable(buff->str + buff->count, read_buff);
    }
    CR_DEBUG("buff count: %zu\n", buff->count);
    return append_null_terminator(buff);
}
