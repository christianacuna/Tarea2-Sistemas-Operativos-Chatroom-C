#include <stdio.h>
#include "debugger.h"

static void my_log(char *s)
{
    printf("[+] %s\n", s);
}
static void my_error(char *s)
{
    printf("[-] Error: %s", s);
}
namespace_struct const console = {my_log, my_error};