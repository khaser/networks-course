# Инструкция по запуску практической работы

### Сборка и запуск
Для сборки:
```
make build
```

Для запуска сервиса по адресу `localhost:8080`:
```
make run
```

### Необходимое окружение:
По идее должно хватать следующего:
* Свежий `gcc`, тк. в Makefile прописано `--std=c++20`
* Любая версия `make`

### Используемые мною версии:

```
$ gcc --version
gcc (GCC) 12.3.0
Copyright (C) 2022 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

```
$ make --version
GNU Make 4.4.1
Built for x86_64-pc-linux-gnu
Copyright (C) 1988-2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
```

При желании можно полностью воспроизвести моё окружение, если Вы знакомы с `nix`.
В директории приложены файлы `flake.nix` и `flake.lock` жёстко фиксирующие версии
всех пакетов используемых мною для сборки и разработки.
Однако данный `flake.nix` содержит мою конфигурацию `vim`,
подгружающегося с приватного репозитория, её нужно удалить из этого файла.
(при возникновении трудностей связаться со мной можно через <https://t.me/khaser1>).
