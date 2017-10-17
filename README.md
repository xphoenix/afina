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

# How to build Xcode
```
cmake -G Xcode -H. -B_build
```

# Сервер:
```
[user@domain build] ./src/afina
```

Поддерживает следующий опции:
- --network <uv, block> какую использовать реализацию сети
  - *uv*: демонстрационную на libuv
  - *block*: блокирующая (домашка)
- --storage <map_global> какую реализацию хранилища использовать
  - *map_global*: на основе std::map с глобальным локом (домашка)

Вот так можно отправить комманды:
```
echo -n -e "set foo 0 0 6\r\nfooval\r\n" | nc localhost 8080
```
обратите внимание на -e и -n

# Tests
```
make runAllocatorTests && ./test/allocator/runAllocatorTests - собрать и запустить тесты аллокатора
make runExecuteTests && ./test/execute/runExecuteTests - собрать и запустить тесты комманд
make runProtocolTests && ./test/protocol/runProtocolTests - собрать и запустить тесты парсера memcached протокола
make runNetworkTests && ./test/network/runNetworkTests - собрать и запустить тесты сетевой подсистемы
make runStorageTests && ./test/storage/runStorageTests - собрать и запустить тесты хранилиза данных
```
