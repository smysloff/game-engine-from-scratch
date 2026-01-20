```c
// startup.s - минимальный ассемблерный стартап
.global _start
_start:
    // Инициализация .bss
    // Настройка стека
    // Вызов main
    call main

    // Выход через syscall
    mov $60, %rax   // SYS_exit
    xor %rdi, %rdi  // код 0
    syscall

// main.c - без единого include стандартной библиотеки
#include "core/types.h"
#include "core/util.h"

// Syscall write
static usize sys_write(i32 fd, const void* buf, usize count) {
    usize ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(1), "D"(fd), "S"(buf), "d"(count)
        : "rcx", "r11", "memory"
    );
    return ret;
}

i64 main(void) {
    const char* msg = "Hello without libc!\n";
    sys_write(1, msg, string_length(msg));
    return 0;
}
```
