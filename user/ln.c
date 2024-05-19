#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // Проверка корректного количества аргументов
    if (argc == 4)
    {
        if (!strcmp(argv[1], "-h"))
        { // проверка флага -h для жесткой ссылки
            if (link(argv[2], argv[3]) < 0)
            { // попытка ее создания
                fprintf(2, "link %s %s: failed\n", argv[2], argv[3]);
                exit(0);
            }
        } else if (!strcmp(argv[1], "-s"))
            { // проверка флага -s для символической ссылки
            if (symlink(argv[2], argv[3]) < 0)
            { // попытка ее создания
                fprintf(2, "symlink %s %s: failed\n", argv[2], argv[3]);
                exit(0);
            }
        }
    }
    else
    {
        fprintf(2, "Usage: ln [-h/-s] old new\n");
        exit(1);
    }
}

