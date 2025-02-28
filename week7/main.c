#include <stdio.h>
#include "dshlib.h"

int main() {
    int rc = exec_local_cmd_loop();
    printf("cmd loop returned %d\n", rc);
    return rc;
}