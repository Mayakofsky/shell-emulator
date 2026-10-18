# Эмулятор UNIX-подобной оболочки

## 1. Описание проекта и этап

Проект для дисциплины «Конфигурационное управление», вариант №7. Этап 1 —
минимальный REPL на C++23: приглашение из данных ОС, разбор командной строки,
раскрытие переменных окружения и выполнение команд-заглушек `ls`, `cd` и `exit`.

Файловая система и конфигурация приложения на этом этапе не реализованы.
Команда `cd` проверяет аргументы, но не меняет текущий каталог.

## 2. Функции, классы и файлы

- `src/main.cpp` настраивает завершение по Ctrl+C и запускает REPL.
- `src/shell/Environment.hpp/.cpp`: чтение переменных окружения, имени
  пользователя, имени хоста и текущего каталога.
- `src/shell/PromptBuilder.hpp/.cpp`: формирование приглашения
  `user@host:cwd$ `, подстановка `~` для домашнего каталога и нормализация
  разделителей пути.
- `src/shell/Token.hpp`: структура лексического токена.
- `src/shell/Lexer.hpp/.cpp`: разбиение строки на токены, кавычки, escape-
  последовательности и раскрытие `$NAME` и `${NAME}`.
- `src/shell/Parser.hpp/.cpp`: преобразование токенов в имя команды и аргументы,
  обработка пустого ввода и синтаксических ошибок.
- `src/shell/Shell.hpp/.cpp`: REPL, связь приглашения, парсера и реестра команд.
- `src/commands/CommandResult.hpp`: результат выполнения команды, включая
  stdout, stderr и параметры завершения.
- `src/commands/Command.hpp`: интерфейс расширяемой команды.
- `src/commands/CommandRegistry.hpp/.cpp`: реестр команд и обработка неизвестных
  имён.
- `src/commands/LsCommand.hpp/.cpp` и `CdCommand.hpp/.cpp`: команды-заглушки.
- `src/commands/ExitCommand.hpp/.cpp`: проверка аргумента и завершение REPL.
- `tests/`: собственный мини-фреймворк и тесты лексера, парсера, приглашения,
  команд и ограничений репозитория.
- `run.sh` и `run.bat`: сборка, запуск приложения и запуск тестов.

Парсер раскрывает переменные реального окружения ОС. Одинарные кавычки
сохраняют `$NAME` буквально; в двойных кавычках переменные раскрываются.
Неизвестная переменная заменяется пустой строкой. Ошибки разбора и команд
выводятся в stderr, после чего REPL продолжает работу.

## 3. Сборка и тестирование

Из каталога `shell-emulator`:

```text
run.bat
run.bat test
```

Для POSIX:

```sh
./run.sh
./run.sh test
```

Вручную через CMake:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## 4. Пример REPL

Пример предполагает, что `HOME` равен `/home/alice`:

```text
alice@workstation:~$ ls "$HOME/My Folder"
ls called with args: [/home/alice/My Folder]
alice@workstation:~$ cd 'folder with spaces'
cd called with args: [folder with spaces]
alice@workstation:~$ unknown-command
shell: unknown command: unknown-command
alice@workstation:~$ exit 0
```
