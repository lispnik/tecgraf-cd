#include <stdio.h>
#include "include/cd.h"

int main() {
    printf("CD Version: %s\n", cdVersion());
    printf("CD Version Date: %s\n", cdVersionDate());
    printf("CD Version Number: %d\n", cdVersionNumber());
    return 0;
}