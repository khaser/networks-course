Учебная реализация простейшего FTP клиента на C socket.
Используется двухканальный пассивный (т.е. второе соединение также принимает сервер) вариант протокола FTP.

Для тестирования использовался следующий сервер:
FTP URL: ftp.dlptest.com or ftp://ftp.dlptest.com/
FTP User: dlpuser
Password: rNrKYTX9g7z3RgJRmxWuGHbeu
Server ip: 44.241.66.173

Компиляция и запуск:
cmake -B build && cmake --build build && ./build/ftp_client 44.241.66.173 testdir

Функционал по смене текущей директории работает лишь для команды rls!
