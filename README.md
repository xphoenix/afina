# afina
Simple caching server

Проект создан для курса по многопоточному программированию в Техносфере

# Telegram
https://t.me/spheremt

# Components
Сервер состоит из компонент, каждый в виде отдельной статической библиотеки:
- Allocator (include/afina/allocator/, src/allocator): менеджер памяти
- Storage (include/afina/Storage.h, src/storage): хранилище данных 
- Execute (include/afina/execute/, src/execute/): комманды, сервер создает экземпляры комманд на основе сообщений из сети и применяет их над заданным хранилищем
- Network (src/network/): сетевой слой, реализует подмножество memcached текстового протокола

# How to build
Для сборки нужен cmake >= 3.0.1 и gcc, так же система сборки использует ccache если последний найден в системе.
```
[user@domain afina] mkdir build
[user@domain afina] cd build
[user@domain build] cmake -DCMAKE_BUILD_TYPE=Debug ..
[user@domain build] make
```

После этого можно запустить само приложение и/или тесты:
```
[user@domain build] ./src/afina
```

# Tests
```
make runAllocatorTests && ./test/allocator/runAllocatorTests - собрать и запустить тесты аллокатора
make runExecuteTests && ./test/execute/runExecuteTests - собрать и запустить тесты комманд
make runNetworkTests && ./test/network/runNetworkTests - собрать и запустить тесты сетевой подсистемы
make runStorageTests && ./test/storage/runStorageTests - собрать и запустить тесты хранилиза данных
```
