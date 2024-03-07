# Инструкция по запуску практической работы

### Установка зависимостей
Для удобства и воспроизводимости установки зависимостей проект завернут в nix flake,
поэтому требуется сперва поставить этот пакетный менеджер себе в систему.

Пример установки на Ubuntu
```apt install nix```

Если для используемого диструбутива он не опакетирован, то можно воспользоваться следующим скриптом,
который ещё в процессе работы ещё и запросит root права, чтобы создать директорию `/nix`
(согласен, выглядит как жесть, но что поделать?)
```sh <(curl -L https://nixos.org/nix/install) --daemon```

Если не хочется ставить `nix`, то для сборки данной лабораторной должно хватить
`cmake` и библиотеки `boost`. Но в таком случае есть большая вероятность, что `cmake` не найдёт `boost`.

### Сборка и запуск
* Собрать и запустить сервер:
```
nix --experimental-features 'nix-command flakes' run '.#server' -- server_port concurrency_level
```
* Собрать и запустить клиент:
```
nix --experimental-features 'nix-command flakes' run '.#client' -- server_host server_port filename
```
* Собрать и запустить сервер(без `nix`):
```
cmake -B build && cmake --build build && ./build/server/server server_port concurrency_level
```
* Собрать и запустить клиент(без `nix`):
```
cmake -B build && cmake --build build && ./build/client/client server_host server_port filename
```

#### Post Scriptum
(при возникновении трудностей связаться со мной можно через <https://t.me/khaser1>).

