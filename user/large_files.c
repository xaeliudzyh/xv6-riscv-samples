//
#include "../kernel/types.h"
#include "user.h"
#include "../kernel/fcntl.h"

#define MULTIPLIER 1103515245 //множитель для LCG
#define MODULUS 2147483648 //модуль для LCG (2^31)
#define BUFFER_SIZE 4096 //размер буфера для операций ввода-вывода в блоках
#define MAX_FILE_SIZE ((11 + 256 + 256 * 256) * 1024 / sizeof(uint))
//макс размер файла, поддерживаемый файловой системой


int verify_data(uint size, uint seed, const char* filename)
{
    uint* buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    if (!buffer)
    {
        fprintf(2, "Memory allocation error\n");
        return 1;
    }
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "File open error\n");
        return 2;
    }
    //uint index = 0;
    for (uint i = 0; i < size / BUFFER_SIZE; i++)
    {
        if (read(fd, buffer, sizeof(uint) * BUFFER_SIZE) != sizeof(uint) * BUFFER_SIZE)
        {
            fprintf(2, "File read error\n");
            close(fd);
            return 3;
        }
        for (int j = 0; j < BUFFER_SIZE; j++)
        {
            if (buffer[j] != seed)
            {
                fprintf(2, "Data mismatch at position %d: expected %d, got %d\n", i * BUFFER_SIZE + j, seed, buffer[j]);
                close(fd);
                return 4;
            }
            seed = (MULTIPLIER * seed + 12345) % MODULUS;
        }
    }

    if (size % BUFFER_SIZE)
    {
        int remaining = size % BUFFER_SIZE;
        if (read(fd, buffer, sizeof(uint) * remaining) != sizeof(uint) * remaining)
        {
            fprintf(2, "File read error\n");
            close(fd);
            return 3;
        }
        for (int j = 0; j < remaining; j++)
        {
            if (buffer[j] != seed)
            {
                fprintf(2, "Data mismatch at position %d: expected %d, got %d\n", size - remaining + j, seed, buffer[j]);
                close(fd);
                return 4;
            }
            seed = (MULTIPLIER * seed + 12345) % MODULUS;
        }
    }
    close(fd);
    free(buffer);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3)
    {
        fprintf(2, "Usage: test_largefile <size> <seed>\n");
        exit(1);
    }
    uint size = atoi(argv[1]) / sizeof(uint), seed = atoi(argv[2]);
    if (size <= 0 || size > MAX_FILE_SIZE)
    {
        fprintf(2, "Invalid file size\n");
        exit(1);
    }
    const char* filename = "largefile";
    uint seed2=seed, size2=size, index = 0;
    uint* buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    if (!buffer)
    {
        fprintf(2, "Memory allocation error\n");
        exit(1);
    }
    int fd = open(filename, O_CREATE | O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "File creation error\n");
        exit(2);
    }
    for (uint i = 0; i < size; i++)
    {
        buffer[index++] = seed;
        seed = (MULTIPLIER * seed + 12345) % MODULUS;
        if (index == BUFFER_SIZE || i == size - 1)
        {
            int to_write = (i == size - 1) ? index : BUFFER_SIZE;
            if (write(fd, buffer, sizeof(uint) * to_write) != sizeof(uint) * to_write)
            {
                fprintf(2, "File write error\n");
                close(fd);
                exit(3);
            }
            index = 0;
        }
    }
    close(fd);
    free(buffer);
    int status = verify_data(size2, seed2, filename);
    if (!status) printf("File verification successful\n");
    else exit(status);
    exit(0);
}

// Created by George Tsagol on 06.05.2024.
//
