//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define NUM_PAGES 4

// функция для проверки результатов доступа
int verify_access(const char* buf, int num_pages, const char* expected, int test_id)
{
    for (int i = 0; i < num_pages; ++i)
    {
        if (buf[i] != expected[i])
        {
            // если результат не совпадает с ожидаемым, выводим сообщение об ошибке
            fprintf(2, "TEST #%d FAILED:\n", test_id);
            fprintf(2, "RESULT:\t\t");
            for (int j = 0; j < num_pages; ++j)
                fprintf(2, "%d", buf[j]);
            fprintf(2, "\nEXPECTED:\t");
            for (int j = 0; j < num_pages; ++j)
                fprintf(2, "%d", expected[j]);
            fprintf(2, "\n");
            return 0;
        }
    }
    // если все результаты совпадают, тест считается успешным
    return 1;
}

// безопасный вызов pgaccess с отладочной информацией
int safe_pgaccess(void* addr, int num_pages, char* buf)
{
    // вызываем системный вызов pgaccess и проверяем его результат
    if (pgaccess(addr, num_pages, buf))
    {
        // если системный вызов завершился с ошибкой, выводим сообщение
        fprintf(2, "ERROR: sys_pgaccess() failed\n");
        return 1;
    }
    // успешное выполнение системного вызова
    return 0;
}

// общая функция тестирования с дополнительной отладочной информацией
int run_tests(char* test_array)
{
    int result = 0;  // переменная для хранения результата тестов
    char access_results[NUM_PAGES];  // массив для хранения результатов доступа

    // тест неинициализированного массива
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\0\0\0", 1))
        result = 1;
    else
        printf("TEST #1 PASSED\n");

    // заполнение массива нулями
    for (int i = 0; i < PGSIZE * NUM_PAGES; ++i)
        test_array[i] = 0;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\1\1\1\1", 2))
        result = 1;
    else
        printf("TEST #2 PASSED\n");

    // изменение значений в страницах #0 и #3
    test_array[PGSIZE * 0] += 1;
    test_array[PGSIZE * 3] += 1;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\1\0\0\1", 3))
        result = 1;
    else
        printf("TEST #3 PASSED\n");


    // изменение значений в страницах #1, #2 и #3
    test_array[PGSIZE * 1] += 1;
    test_array[PGSIZE * 2] += 1;
    test_array[PGSIZE * 3] += 1;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\1\1\1", 4))
        result = 1;
    else
        printf("TEST #4 PASSED\n");

    // изменение значений в странице #0
    test_array[0] += 1;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\1\0\0\0", 5))
        result = 1;
    else
        printf("TEST #5 PASSED\n");

    // проверка без изменений
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\0\0\0", 6))
        result = 1;
    else
        printf("TEST #6 PASSED\n");

    // изменение значений в странице #0
    test_array[0] += 1;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\1\0\0\0", 7))
        result = 1;
    else
        printf("TEST #7 PASSED\n");

    // изменение значений в страницах #2 и #3
    test_array[PGSIZE * 1] += 1;
    test_array[PGSIZE * 2] += 1;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\1\1\0", 8))
        result = 1;
    else
        printf("TEST #8 PASSED\n");

    // проверка без изменений
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\0\0\0", 9))
        result = 1;
    else
        printf("TEST #9 PASSED\n");

    // проверка без изменений снова
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\0\0\0", 10))
        result = 1;
    else
        printf("TEST #10 PASSED\n");

    // заполнение страницы #3 значениями
    for (int i = PGSIZE * 3; i < PGSIZE * 4; ++i) test_array[i] = 66;
    if (safe_pgaccess(test_array, NUM_PAGES, access_results))
        return 2;
    if (!verify_access(access_results, NUM_PAGES, "\0\0\0\1", 11))
        result = 1;
    else
        printf("TEST #11 PASSED\n");

    // возвращаем результат выполнения всех тестов
    return result;
}

char global_test_array[PGSIZE * NUM_PAGES];  // массив для тестирования на стеке

// тестирование pgaccess для массива, выделенного на стеке
int test_stack_array()
{
    return run_tests(global_test_array);
}

// тестирование pgaccess для массива, выделенного в куче
int test_heap_array()
{
    char* dynamic_array = malloc(PGSIZE * NUM_PAGES * sizeof(char));
    if (!dynamic_array)
    {
        // если malloc не удалось выделить память, выводим сообщение об ошибке
        fprintf(2, "malloc() failed\n");
        return 4;
    }
    int result = run_tests(dynamic_array);
    free(dynamic_array);  // освобождаем память, выделенную в куче
    return result;
}

int main()
{
    int overall_result = 0;
    overall_result |= test_stack_array();  // запускаем тесты для массива на стеке
    overall_result |= test_heap_array();   // запускаем тесты для массива в куче

    // выводим общий результат тестирования
    if (!overall_result)
        printf("All tests passed successfully.\n");
    else
        printf("Some tests failed.\n");

    exit(overall_result);
}

// Created by George Tsagol on 18.05.2024.
//
