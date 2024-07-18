//_
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// простая функция для вывода сообщения об ошибке и завершения программы с указанным кодом ошибки
void error_exit(const char *msg, int code)
{
    fprintf(2, "Error: %s\n", msg);
    exit(code);
}

void child_part(int *parent_to_child_channel, int *child_to_parent_channel)
{
    if (close(parent_to_child_channel[1]) < 0) error_exit("child unable to close write fd of parent to child channel", -6); // закрываем канал записи от родителя к ребенку, проверка на ошибку
    if (close(child_to_parent_channel[0]) < 0) error_exit("child unable to close read fd of child to parent channel", -7); // закрываем канал чтения от ребенка к родителю, проверка на ошибку
    int pid = getpid(); // получаем pid текущего процесса
    if (pid < 0) error_exit("impossible to get pid", -4); // проверка на ошибку получения pid
    int read_status;
    char symbol;
    while ((read_status = read(parent_to_child_channel[0], &symbol, 1)) == 1) // читаем символы из канала от родителя к ребенку
    {
        printf("PID: %d received '%c'\n", pid, symbol); // выводим полученный символ
        if (write(child_to_parent_channel[1], &symbol, 1) != 1) error_exit("child unable to write symbol", -8); // отправляем символ обратно родителю, проверка на ошибку
    }
    if (read_status != 0) error_exit("child unable to read symbol", -9); // проверка на ошибку чтения
    if (close(child_to_parent_channel[1]) < 0) error_exit("child unable to close write fd of child to parent channel", -11); // закрываем канал записи от ребенка к родителю, проверка на ошибку
    if (close(parent_to_child_channel[0]) < 0) error_exit("child unable to close read fd of parent to child channel", -10); // закрываем канал чтения от родителя к ребенку, проверка на ошибку
    exit(0);
}

void elder_part(int *parent_to_child_channel, int *child_to_parent_channel, const char *string_to_send)
{
    if (close(parent_to_child_channel[0]) < 0) error_exit("parent unable to close read fd of parent to child channel", -17); // закрываем канал чтения от родителя к ребенку, проверка на ошибку
    if (close(child_to_parent_channel[1]) < 0) error_exit("parent unable to close write fd of child to parent channel", -18); // закрываем канал записи от ребенка к родителю, проверка на ошибку
    int pid = getpid(); // получаем pid текущего процесса
    if (pid < 0) error_exit("impossible to get pid", -4); // проверка на ошибку получения pid
    int read_status;
    char symbol;
    int len = strlen(string_to_send); // получаем длину строки для отправки
    if (write(parent_to_child_channel[1], string_to_send, len) != len) error_exit("parent unable to write symbol", -15); // отправляем строку ребенку, проверка на ошибку
    if (close(parent_to_child_channel[1]) < 0) error_exit("parent unable to close write fd of parent to child channel", -13); // закрываем канал записи от родителя к ребенку, проверка на ошибку
    while ((read_status = read(child_to_parent_channel[0], &symbol, 1)) == 1) printf("PID: %d received '%c'\n", pid, symbol); // читаем символы от ребенка и выводим их
    if (read_status != 0) error_exit("parent unable to read symbol", -16); // проверка на ошибку чтения
    if (close(child_to_parent_channel[0]) < 0) error_exit("parent unable to close read fd of child to parent channel", -14); // закрываем канал чтения от ребенка к родителю, проверка на ошибку
    if (wait(0) < 0) error_exit("parent unable to wait child execution", -12); // ждем завершения выполнения ребенка, проверка на ошибку
    exit(0);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) error_exit("number of arguments must be 1", -1); // проверяем количество аргументов командной строки
    int parent_to_child_channel[2];
    int child_to_parent_channel[2];
    if (pipe(parent_to_child_channel) < 0) error_exit("impossible to create pipe from parent to child", -2); // создаем канал от родителя к ребенку, проверка на ошибку
    if (pipe(child_to_parent_channel) < 0) error_exit("impossible to create pipe from child to parent", -5); // создаем канал от ребенка к родителю, проверка на ошибку
    int fork_status = fork(); // создаем дочерний процесс
    if (fork_status < 0) error_exit("impossible to create a child process", -3); // проверка на ошибку создания процесса
    else if (!fork_status) child_part(parent_to_child_channel, child_to_parent_channel); // выполняем действия для дочернего процесса
    elder_part(parent_to_child_channel, child_to_parent_channel, argv[1]); // выполняем действия для родительского процесса
    exit(0);
}
// Created by George Tsagol on 17.07.2024.
//
