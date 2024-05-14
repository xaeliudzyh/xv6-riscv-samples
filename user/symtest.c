//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXPATH 128

void create_file(const char *path, const char *content) {
    int fd = open(path, O_CREATE | O_RDWR);
    if (fd < 0) {
        fprintf(2, "failed to create file: %s\n", path);
        exit(1);
    }
    write(fd, content, strlen(content));
    close(fd);
}

void create_symlink(const char *target, const char *linkpath) {
    if (symlink(target, linkpath) < 0) {
        fprintf(2, "failed to create symlink: %s -> %s\n", linkpath, target);
        exit(1);
    } else {
        printf("created symlink: %s -> %s\n", linkpath, target);
    }
}

void print_dir(const char *path) {
    printf("directory listing for: %s\n", path);

    int pid = fork();
    if (pid == 0) {
        char *argv[] = { "ls", (char *)path, 0 };
        exec("ls", argv);
        exit(0);
    } else if (pid > 0) {
        wait(0);
    } else {
        fprintf(2, "fork failed\n");
        exit(1);
    }
}

void safe_mkdir(const char *path) {
    if (mkdir(path) < 0) {
        fprintf(2, "failed to create directory: %s\n", path);
        exit(1);
    } else {
        printf("created directory: %s\n", path);
    }
}

void print_symlink_target(const char *path) {
    char buf[MAXPATH];
    int n = readlink(path, buf);  // исправлена сигнатура для xv6
    if (n < 0) {
        fprintf(2, "failed to read symlink: %s\n", path);
        return;
    }
    buf[n] = '\0';
    printf("%s --> %s\n", path, buf);
}

void test_symlinks() {
    // создание файлов и каталогов
    safe_mkdir("a");
    safe_mkdir("b");
    safe_mkdir("a/dir1");
    safe_mkdir("a/dir2");
    safe_mkdir("a/dir1/dir3");
    safe_mkdir("a/dir1/dir3/dir4");

    create_file("a/file1", "content1");
    create_file("a/dir1/file2", "content2");
    create_file("a/dir1/dir3/file3", "content3");

    // создание символических ссылок для тестов
    create_symlink("/a/file1", "b/symlink1");  // абсолютная ссылка на файл
    create_symlink("../file1", "a/dir1/symlink2");  // относительная ссылка на файл в том же каталоге
    create_symlink("../../file1", "a/dir1/dir3/symlink3");  // относительная ссылка на файл на 2 уровня выше
    create_symlink("../dir1/file2", "a/dir1/dir3/symlink4");  // относительная ссылка на файл на 1 уровень выше

    // рекурсивные ссылки
    create_symlink("/b/symlink1", "a/dir1/dir3/symlink5");  // абсолютная ссылка на абсолютную символическую ссылку
    create_symlink("../dir1/symlink2", "a/dir1/dir3/symlink6");  // относительная ссылка на относительную символическую ссылку

    // самоссылающаяся ссылка (бесконечная рекурсия)
    create_symlink("selflink", "b/selflink");

    // косвенная самоссылающаяся ссылка
    create_symlink("indirect1", "b/indirect2");
    create_symlink("indirect2", "b/indirect1");

    // сломанные ссылки
    create_symlink("/nonexistent", "b/broken1");
    create_symlink("../nonexistent", "a/dir1/broken2");
    create_symlink("../../nonexistent", "a/dir1/dir3/broken3");

    // вывод структуры каталогов
    print_dir("a");
    print_symlink_target("a/dir1/symlink2");
    print_symlink_target("a/dir1/dir3/symlink3");
    print_symlink_target("a/dir1/dir3/symlink4");

    print_dir("a/dir1");
    print_symlink_target("a/dir1/dir3/symlink5");
    print_symlink_target("a/dir1/dir3/symlink6");

    print_dir("a/dir1/dir3");
    print_symlink_target("b/symlink1");
    print_symlink_target("b/selflink");
    print_symlink_target("b/indirect1");
    print_symlink_target("b/indirect2");
    print_symlink_target("b/broken1");

    print_dir("b");
}

int main(int argc, char *argv[]) {
    test_symlinks();
    exit(0);
}

// Created by George Tsagol on 14.05.2024.
// symtest
