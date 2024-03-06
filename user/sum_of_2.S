#define size_buf = 1024
.data
    buffer:            .space 1024
    print_format:      .asciz "%d\n"
    read_format:       .asciz " "
    exception_sum:     .asciz "Something went wrong, please try again with correct arguments\n"


.text
.global main

main:
    la a0, buffer #в a0 ставим адрес буфера
    li a1, 512
    call gets #input
    beqz a0, print_error
    li a1,' '
    la a0,buffer # опять в a0 ставим адрес буфера
    call strchr # split(' ')
    li t0, 0
    sb t0, 0(a0) # вставляем нуль-треминатор в строку
    li t1, 1
    add t0, a0, t1 #теперь t0 указывает на число после пробела
    la a0,buffer #компируем в a0 первое число, идущее до нуль-терминатора
    call atoi #это число из строки конвертируем в int
    addi t1,a0,0 #сохраняем temp1= первое число
    addi a0,t0,0 #переносим адрес второго числа в a0
    call atoi
    addi t2,a0,0 #сохраняем temp2= второе число
    add t3,t2,t1  #собственно сумма
#переходим к выводу
    la a0, print_format
    addi a1, t3,0 #в а1 записываем сумму
    call printf
    li a7, 2
    li a0, 1
    ecall

print_error:
    la a0, exception_sum  // загрузить адрес строки ошибки в a0
    li a7, 4              // системный вызов для вывода строки в MIPS
    ecall                 // выполняем системный вызов
