# 🕷 SpiderOS

**SpiderOS** — это любительская 32-битная операционная система для архитектуры **x86**, написанная на **C и Assembly**, загружаемая через **GRUB (Multiboot)**.  
Проект создаётся в учебных и исследовательских целях, с упором на низкоуровневую архитектуру ОС.

---

## 🚀 Возможности

На текущий момент в SpiderOS реализовано:

### 🧱 Загрузка и архитектура
- Загрузка через **GRUB2 (Multiboot)**
- 32-битный **Protected Mode**
- Собственный **linker script**
- Чистая freestanding-сборка (`-ffreestanding`)

### ⚙️ Процессор и прерывания
- **GDT** (Global Descriptor Table)
- **IDT** (Interrupt Descriptor Table)
- **ISR / IRQ** обработчики
- Поддержка аппаратных прерываний (PIT, Keyboard)

### ⏱ Таймер и ввод
- **PIT timer** (IRQ0)
- **PS/2 клавиатура** (IRQ1)
- Обработка scancode’ов

### 🖥 Видео и терминал
- **VGA text mode (0xB8000)**
- Цветной вывод текста
- Абстракция терминала (TTY)
- Прокрутка экрана

### 🧠 Память
- Чтение **Multiboot memory map**
- **Physical Memory Manager (PMM)** — bitmap
- **Paging** (виртуальная память)
- **Kernel heap** (`malloc` / `free`)

### 🧪 Kernel Shell
- Простейшая командная оболочка
- Ввод с клавиатуры
- Базовые команды (help, echo и др.)

### 🪵 Отладка
- Kernel logging (INFO / WARN / ERROR)
- `panic()` с остановкой CPU

---

## 📁 Структура проекта

SpiderOS/
├── kernel/
│ ├── gdt.* # GDT и protected mode
│ ├── idt.* # IDT
│ ├── isr.* # ISR и IRQ
│ ├── timer.* # PIT таймер
│ ├── kbd.* # Клавиатура
│ ├── vga.* # VGA text mode
│ ├── tty.* # Терминал
│ ├── memory.* # Memory map
│ ├── pmm.* # Physical Memory Manager
│ ├── paging.* # Paging
│ ├── heap.* # Kernel heap
│ ├── shell.* # Kernel shell
│ ├── panic.* # Panic handler
│ ├── log.* # Logging
│ ├── multiboot.* # Multiboot header
│ └── kernel.c # kernel_main
├── linker.ld # Linker script
├── grub.cfg # GRUB config
├── Makefile # Build system
└── README.md
---

## 🛠 Сборка и запуск

### Зависимости
- `i686-elf-gcc`
- `i686-elf-binutils`
- `nasm`
- `grub-pc-bin`
- `xorriso`
- `qemu-system-i386`

### Сборка
```bash
make

### Запуск в QEMU
make run
