//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXPATH 128

char* path1 = "/dir1/dir2/links/file1", *path2 = "/dir1/file2", *path3 = "/dir1/dir2/links/sub1/sub2/file3";
char* links_to_file1[] = {"/dir1/dir2/links/link1", "/dir1/dir2/links/link2", "/dir1/dir2/links/link3", "/dir1/dir2/links/newlink1", "/dir1/dir2/links/newlink2"},
    *local_links[] = {"oldlink", "newoldlink", "newoldlink2"},*bad_links[] = {"recursive", "loop2", "loop3", "loop1", "nextlink", "currentlink", "uplink", "downlink"};
char* file_system_error = ">>> Ошибка: ошибка файловой системы <<<\n";

// создание файла с данными и проверкой на ошибки
//void create_file(const char *path, const char *content) {
void create_file(char* name, char data)
{
    int fd = open(name, O_CREATE | O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
    if (write(fd, &data, 1) - 1 < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
    close(fd);
}

// создание директории с проверкой на ошибки
void create_dir(char* name)
{
    if (mkdir(name) < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
}

// смена текущей директории с проверкой на ошибки
void change_dir(char* name)
{
    if (chdir(name) < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
}

// создание символической ссылки с проверкой на ошибки
void create_symlink(char* target, char* linkpath)
{
    if (symlink(target, linkpath) < 0)
    {
        fprintf(2, ">>> Ошибка: ошибка при создании символической ссылки <<<\n");
        exit(1);
    }
}

// создание вложенных директорий
void create_nested_dirs()
{
    create_dir("dir1");
    create_dir("dir1/dir2");
    create_dir("dir1/dir2/links");
    create_dir("dir1/dir2/links/sub1");
    create_dir("dir1/dir2/links/sub1/sub2");
}

// проверка корректности символических ссылок
void verify_link(char* target, char* link, int valid)
{
    int fd = open(link, O_RDONLY);
    if (fd == -1 && !valid) return; // если файл не существует и он не должен существовать, то выходим из функции
    if (fd < 0)
    {
        fprintf(2, ">>> Ошибка: не могу открыть корректный файл <<<\n");
        exit(1);
    }
    if (valid - 1 < 0)
    {
        fprintf(2, ">>> Ошибка: открытие некорректного файла <<<\n");
        exit(1);
    }
    char read_result;
    int res = read(fd, &read_result, 1);
    if (res - 1 < 0)
    {
        fprintf(2, ">>> Ошибка: не могу прочитать символическую ссылку <<<\n");
        exit(1);
    }
    close(fd);
    fd = open(target, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
    char expected;
    res = read(fd, &expected, 1);
    if (res - 1 < 0)
    {
        fprintf(2, file_system_error);
        exit(1);
    }
    close(fd);
    if (read_result != expected)
    {
        fprintf(2, ">>> Ссылка на неверный файл -> ");
        printf("Ожидалось: %c, Получено: %c\n", expected, read_result);
        exit(3);
    }
}

void test_symlinks()
{
    create_nested_dirs();
    create_file(path1, 'a'), create_file(path2, 'b'), create_file(path3, 'c');
    create_symlink(path1, "/dir1/dir2/links/link1"); // создание символической ссылки на path1
    create_symlink("/dir1/dir2/links/link1", "/dir1/dir2/links/link2"); // создание символических ссылок
    create_symlink("/dir1/dir2/links/link2", "/dir1/dir2/links/link3");
    create_symlink("/dir1/dir2/links/oldlink", "/dir1/dir2/links/newlink1");
    create_symlink("/dir1/dir2/links/newlink1", "/dir1/dir2/links/newlink2");
    change_dir("dir1/dir2/links");
    create_symlink("./file1", "oldlink"); // создание символической ссылки на file1 в текущей директории
    create_symlink("oldlink", "newoldlink"); // создание символической ссылки на oldlink
    create_symlink("newoldlink", "newoldlink2"); // создание символической ссылки на newoldlink
    create_symlink("../../file2", "otherlink"); // создание символической ссылки на file2 двумя уровнями выше
    create_symlink("sub1/sub2/file3", "finalfile"); // создание символической ссылки на file3 внутри sub1/sub2
    create_symlink("recursive", "recursive"); // создание рекурсивной символической ссылки на саму себя
    create_symlink("loop1", "loop2"), create_symlink("loop2", "loop3"), create_symlink("loop3", "loop1"); // создание символических ссылок для зацикливания
    create_symlink("nonexistent", "nextlink"); // создание символической ссылки на несуществующий файл
    create_symlink("dummy", "currentlink"); // создание символической ссылки на фиктивный файл
    create_symlink("../../file1", "uplink"); // создание символической ссылки на file1 двумя уровнями выше
    create_symlink("sub1/sub2/file1", "downlink"); // создание символической ссылки на file1 внутри sub1/sub2
    verify_link("../../file2", "otherlink", 1); // проверка корректности символической ссылки на file2
    verify_link("sub1/sub2/file3", "finalfile", 1); // проверка корректности символической ссылки на file3
    change_dir("../../..");
}


int main()
{
    test_symlinks();
    for (int i = 0; i < 5; ++i) verify_link(path1, links_to_file1[i], 1); // проверка корректности символических ссылок на path1
    change_dir("dir1/dir2/links");
    for (int i = 0; i < 3; ++i) verify_link("file1", local_links[i], 1); // проверка корректности символических ссылок на file1
    for (int i = 0; i < 8; ++i) verify_link("", bad_links[i], 0); // проверка некорректных символических ссылок
    char* directories[] = {"/", "/dir1", "/dir1/dir2", "/dir1/dir2/links", "/dir1/dir2/links/sub1", "/dir1/dir2/links/sub1/sub2"};
    char* ls_commands[] = {"/ls", "../ls", "../../ls", "../../../ls", "../../../../ls", "../../../../../ls"};
    for (int i = 0; i < 6; ++i)
    {
        int pid = fork(); // форк процесса для выполнения команды ls
        if (pid < 0)
        {
            fprintf(2, ">>> Ошибка: ошибка форка <<<\n");
            exit(1);
        }
        if (!pid)
        {
            printf("\n\n#############################\n");
            printf("Текущая директория: %s\n", directories[i]);
            change_dir(directories[i]); // смена директории на указанную
            char* argv[] = {0};
            exec(ls_commands[i], argv); // выполнение команды ls в указанной директории
            fprintf(2, "ошибка exec\n");
            exit(1);
        }
        else
        {
            int status;
            wait(&status); // ожидание завершения дочернего процесса
        }
    }
    exit(0);
}
// Created by George Tsagol on 14.05.2024.
// symtest
