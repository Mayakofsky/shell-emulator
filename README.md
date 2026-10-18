# Эмулятор UNIX-подобной оболочки

## 1. Описание проекта и этап

Проект для дисциплины «Конфигурационное управление», вариант №7. Этап 5
добавляет команды `rm` и `mv`, завершая набор файловых операций над VFS.
Этап 4 сделал команды `ls` и `cd` рабочими поверх VFS и добавил команды
`wc`, `find` и `who`. Этап 3 добавил виртуальную файловую систему (VFS),
загружаемую из ZIP-архива целиком в память, и служебные команды `vfs-load`,
`vfs-tree`, `vfs-read`. Этап 2 расширил REPL настройкой через CLI и XML,
JSON Lines-логированием и стартовым скриптом.
Сохраняются приглашение из данных ОС, разбор командной строки, раскрытие
переменных окружения и команда `exit`.

Текущий каталог оболочки (`cd`) — это путь внутри VFS, а приглашение
по-прежнему строится из каталога реальной ОС. VFS доступна только через
команды; на диск архив не распаковывается.

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
- `src/shell/Parser.hpp/.cpp`: преобразование токенов в имя команды и
  аргументы,
  обработка пустого ввода и синтаксических ошибок.
- `src/shell/Shell.hpp/.cpp`: REPL, связь приглашения, парсера и реестра команд.
- `src/commands/CommandResult.hpp`: результат выполнения команды, включая
  stdout, stderr и параметры завершения.
- `src/commands/Command.hpp`: интерфейс расширяемой команды.
- `src/commands/CommandContext.hpp`: контекст выполнения команды с доступом
  к VFS и текущему каталогу.
- `src/commands/CommandRegistry.hpp/.cpp`: реестр команд и обработка неизвестных
  имён.
- `src/commands/LsCommand.hpp/.cpp`: вывод имён каталога VFS.
- `src/commands/CdCommand.hpp/.cpp`: смена текущего каталога VFS.
- `src/commands/WcCommand.hpp/.cpp`: подсчёт строк, слов и байт файлов VFS.
- `src/commands/FindCommand.hpp/.cpp`: обход поддерева VFS с фильтрами
  `-name` и `-type`.
- `src/commands/WhoCommand.hpp/.cpp`: печать имени пользователя и хоста.
- `src/commands/RmCommand.hpp/.cpp`: удаление файлов и каталогов VFS.
- `src/commands/MvCommand.hpp/.cpp`: переименование и перемещение узлов VFS.
- `src/commands/ExitCommand.hpp/.cpp`: проверка аргумента и завершение REPL.
- `src/commands/VfsLoadCommand.hpp/.cpp`: загрузка VFS из ZIP-архива.
- `src/commands/VfsTreeCommand.hpp/.cpp`: печать поддерева VFS.
- `src/commands/VfsReadCommand.hpp/.cpp`: чтение файла VFS.
- `src/vfs/VfsNode.hpp`: узел дерева VFS.
- `src/vfs/Vfs.hpp/.cpp`: дерево VFS в памяти, построение из записей,
  `resolve`, `resolveFrom`, `listPaths`, `removeEntry` и `moveEntry`.
- `src/vfs/ZipReader.hpp/.cpp`: чтение ZIP-архива в память без распаковки на
  диск (STORE и DEFLATE).
- `src/vfs/Inflate.hpp/.cpp`: собственная распаковка DEFLATE (RFC 1951).
- `src/vfs/Base64.hpp/.cpp`: кодирование и строгое декодирование base64.
- `src/config/Configuration.hpp`: объединённые параметры запуска.
- `src/config/ArgsParser.hpp/.cpp`: разбор параметров командной строки.
- `src/config/ConfigFileReader.hpp/.cpp`: минимальный XML-парсер конфигурации.
- `src/config/ConfigLoader.hpp/.cpp`: чтение XML и объединение с CLI по правилу
  «командная строка важнее файла».
- `src/log/JsonLogger.hpp/.cpp`: построчная запись JSON-объектов с UTC-временем.
- `src/script/ScriptRunner.hpp/.cpp`: последовательное выполнение стартового
  скрипта с отображением приглашения и команды.
- `tests/`: собственный мини-фреймворк и тесты лексера, парсера, приглашения,
  команд, команд VFS, файловых команд этапов 4–5, конфигурации, логгера,
  стартового скрипта и ограничений репозитория.
- `run.sh` и `run.bat`: сборка, запуск приложения и запуск тестов.
- `scripts/`: демонстрационные `.bat` и `.sh` сценарии, XML-конфигурация и
  стартовый скрипт.

Парсер раскрывает переменные реального окружения ОС.
Одинарные кавычки сохраняют `$NAME` буквально; в двойных кавычках переменные
раскрываются. Неизвестная переменная заменяется пустой строкой.
Ошибки разбора и команд выводятся в stderr, после чего REPL продолжает работу.

## 3. Параметры командной строки

Поддерживаются формы `--name value` и `--name=value`:

```text
--vfs <путь>       физическое расположение VFS
--log <путь>       JSON Lines-файл журнала
--script <путь>    стартовый скрипт
--config <путь>    XML-файл конфигурации
```

Неизвестный параметр и отсутствующее значение завершают запуск с кодом 1.
Сообщение имеет вид `shell: argument error: ...` и выводится в stderr.

## 4. XML-конфигурация и приоритеты

Пример файла:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<config>
  <vfs>/path/to/vfs</vfs>
  <log>/path/to/log.jsonl</log>
  <script>/path/to/startup.txt</script>
</config>
```

Все элементы необязательны. Неизвестные элементы игнорируются, пробелы по
краям значения удаляются. Значения CLI имеют приоритет над XML.
Ошибка чтения или разбора XML выводится как `shell: config error: ...` и
возвращает код 1.

## 5. Отладочный вывод

До выполнения стартового скрипта и REPL приложение печатает:

```text
configuration:
  config = <значение или (unset)>
  vfs    = <значение или (unset)>
  log    = <значение или (unset)>
  script = <значение или (unset)>
```

## 6. JSON-лог

При заданном `--log` файл открывается с перезаписью.
Каждая непустая строка из стартового скрипта и интерактивного ввода
записывается отдельным JSON-объектом.
В объекте есть UTC-время секундной точности, команда, массив аргументов и
статус `ok`, `unknown`, `error`, `syntax-error` или `exit`.
Синтаксическая ошибка записывается с `"command":null`.

## 7. Стартовый скрипт

Стартовый скрипт читается построчно. Перед каждой непустой строкой
печатаются приглашение и исходная команда.
Ошибочные команды выводят ошибку и не останавливают выполнение следующих
строк. Команда `exit` завершает весь процесс с указанным кодом.
После нормального завершения скрипта запускается интерактивный REPL.

## 8. Сборка и тестирование

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

## 9. Скрипты в `scripts/`

- `test_cli_args.bat` и `test_cli_args.sh` используют все четыре CLI-параметра.
- `test_config_only.bat` и `test_config_only.sh` запускают XML-конфигурацию.
- `test_priority.bat` и `test_priority.sh` проверяют приоритет CLI над XML.
- `test_all.bat` и `test_all.sh` используют конфигурацию, лог, VFS и стартовый
  скрипт одновременно.
- `make_vfs.bat` и `make_vfs.sh` собирают ZIP-архивы VFS в `data/vfs/` из
  заготовок `data/vfs_samples/`.
- `test_vfs_minimal.*`, `test_vfs_several.*`, `test_vfs_nested.*` и
  `test_vfs_errors.*` проверяют варианты VFS и обработку ошибок.
- `test_commands.*` запускает эмулятор с архивом `nested.zip` и стартовым
  скриптом `startup_commands.txt`, демонстрируя `ls`, `cd`, `wc`, `find` и
  `who`.
- `test_extra.*` запускает эмулятор с архивом `several.zip` и стартовым
  скриптом `startup_extra.txt`, демонстрируя `rm` и `mv`.
- `config.xml` содержит пример конфигурации.
- `startup_demo.txt` демонстрирует успешные команды, кавычки и пропуск
  ошибок.
- `startup_vfs.txt` демонстрирует команды этапов 1–3, включая VFS.
- `startup_commands.txt` демонстрирует файловые команды этапа 4.
- `startup_extra.txt` демонстрирует удаление и перемещение этапа 5.

Все сценарии после запуска печатают созданный лог-файл.

## 10. Виртуальная файловая система (этап 3)

VFS живёт целиком в памяти. Архив читается побайтово, распаковывается в
буфер и превращается в дерево; на диск ничего не пишется, кроме самих
ZIP-файлов, которые создают скрипты `make_vfs`.

- Источник VFS — ZIP-архив. Поддерживаются метод сжатия 0 (STORE) и 8
  (DEFLATE, распаковывается собственной реализацией по RFC 1951).
- Имена читаются в UTF-8; при флаге `0x800` кодировка CP437 не применяется.
- Каталоги могут быть заданы явными записями с завершающим `/` или выведены
  из путей файлов. При конфликте побеждает последняя запись.
- Ошибки: не ZIP, отсутствие EOCD, битые заголовки, неподдерживаемый метод
  сжатия, несовпадение размеров.

### 10.1. Служебные команды VFS

- `vfs-load <path>` — загружает или заменяет VFS из архива. Успех:
  `vfs: loaded <N> entries from <path>`. Ошибка: `shell: vfs error: <detail>`,
  прежнее дерево сохраняется.
- `vfs-tree [path]` — печатает полные пути поддерева по одному на строку,
  отсортированные лексикографически, каталоги с завершающим `/`. Без
  аргумента берётся текущий каталог. Ошибка: `vfs-tree: no such path: <path>`.
- `vfs-read <path>` — содержимое файла с переводом строки. Текст выводится как
  есть; бинарный файл (есть байт `0x00` или байт `>= 0x80`) выводится строкой
  `base64:<encoded>`. Ошибки: `vfs-read: no such file: <path>` и
  `vfs-read: is a directory: <path>`.

### 10.2. Загрузка при старте

Если `--vfs` задан, после блока `configuration:` архив загружается. Успех —
строка `vfs: loaded <N> entries from <path>`. Ошибка — `shell: vfs error:
<detail>` и код возврата 1. Без `--vfs` VFS остаётся пустой и сообщение не
печатается.

### 10.3. Требования к содержимому архива

Архив — обычный ZIP: локальные заголовки, central directory и записи EOCD.
Используйте небольшие текстовые файлы и каталоги. Примеры лежат в
`data/vfs_samples/`: `minimal` (один `.gitkeep`), `several` (несколько
файлов и каталог `docs`) и `nested` (три уровня `a/b/c` и `a/inner.txt`).

### 10.4. Скрипты реальной ОС

- `scripts/make_vfs.bat` и `scripts/make_vfs.sh` собирают архивы в
  `data/vfs/` из каталогов-заготовок (`Compress-Archive` на Windows, `zip`
  или `python3 -m zipfile` на POSIX). `*.zip` в git не коммитятся.
- `test_vfs_minimal.*`, `test_vfs_several.*`, `test_vfs_nested.*` запускают
  эмулятор с `--vfs` на соответствующий архив и стартовым скриптом
  `startup_vfs.txt`.
- `test_vfs_errors.*` проверяет обработку ошибок: несуществующий архив и
  битый архив завершаются с кодом 1, ошибки команд в скрипте пропускаются.
- `startup_vfs.txt` демонстрирует успешные и ошибочные вызовы `vfs-*`,
  переменные окружения и пропуск ошибочных строк.

## 11. Команды этапа 4: ls, cd, wc, find, who

Все команды работают с VFS. Относительные пути вычисляются от текущего
каталога оболочки, который меняет `cd`.

- `ls [path]` — имена детей каталога по одному на строку, отсортированные
  лексикографически, без завершающего `/`. Без аргумента берётся текущий
  каталог. Ошибки: `ls: no such path: <path>`, `ls: not a directory: <path>`,
  `ls: too many arguments`.
- `cd [path]` — смена текущего каталога. Без аргумента переходит в `/`.
  Ошибки: `cd: no such directory: <path>`, `cd: not a directory: <path>`,
  `cd: too many arguments`.
- `wc [-l] [-w] [-c] <file>...` — для каждого файла печатает
  `lines words bytes path`; при нескольких файлах добавляется строка
  `lines words bytes total`. Флаги `-l`, `-w`, `-c` (можно вместе)
  оставляют только выбранные счётчики; без флагов печатаются все три.
  Словом считается непрерывная последовательность непробельных байтов.
  Ошибки: `wc: missing file operand`, `wc: no such file: <path>`,
  `wc: is a directory: <path>`, `wc: invalid option: -<x>`.
- `find [path] [-name <glob>] [-type f|d]` — обходит поддерево и печатает
  полные пути, отсортированные лексикографически, каталоги с завершающим `/`.
  Без аргумента обходит текущий каталог. Маска `-name` понимает `*` и `?`;
  `-type` ограничивает файлами (`f`) или каталогами (`d`). Ошибки:
  `find: no such path: <path>`, `find: unknown predicate: <arg>`,
  `find: missing argument for -name`,
  `find: invalid argument for -type: <value>`.
- `who` — печатает `username hostname`. Ошибка: `who: too many arguments`.

## 12. Команды этапа 5: rm, mv

- `rm [-r] [-f] <path>...` — удаляет узлы VFS. Каталог удаляется только с
  флагом `-r` (или `-R`); без него выводится `rm: is a directory: <path>`.
  Флаг `-f` подавляет ошибку отсутствующего пути. Корень `/` удалить нельзя.
  Ошибки: `rm: missing operand`, `rm: invalid option: -<x>`,
  `rm: no such file: <path>`, `rm: is a directory: <path>`,
  `rm: cannot remove root: /`.
- `mv <source> <destination>` — переименовывает узел или перемещает его.
  Если `<destination>` — существующий каталог, источник перемещается внутрь
  него с прежним именем. Существующий файл-цель перезаписывается. Каталог
  нельзя переместить внутрь себя. Ошибки: `mv: missing file operand`,
  `mv: missing destination file operand after '<source>'`,
  `mv: too many arguments`, `mv: no such file: <source>`,
  `mv: invalid target: <destination>`, `mv: cannot move root: /`,
  `mv: cannot move a directory into itself: <source>`.

## 13. Примеры запуска

Только CLI:

```sh
./build/shell-emulator --vfs ./data/vfs --log ./run.jsonl \
    --script ./scripts/startup_demo.txt --config ./scripts/config.xml
```

Только XML:

```sh
./build/shell-emulator --config ./scripts/config.xml
```

## 14. Пример REPL

Пример предполагает, что `HOME` равен `/home/alice`, а VFS содержит `/data`,
`/data/one.txt` и `/notes.txt`:

```text
alice@workstation:~$ ls
data
notes.txt
alice@workstation:~$ cd data
alice@workstation:~$ ls
one.txt
alice@workstation:~$ wc -l one.txt
2 one.txt
alice@workstation:~$ find / -name "*.txt"
/data/one.txt
/notes.txt
alice@workstation:~$ who
alice workstation
alice@workstation:~$ unknown-command
shell: unknown command: unknown-command
alice@workstation:~$ exit 0
```
