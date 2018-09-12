# afina
Simple caching server

Проект создан для курса по многопоточному программированию в Техносфере

# Telegram - obsolote
https://t.me/spheremt
В связи с героической смертью сервиса принимаем предложения на тему, где-бы создать группу для общения...

# Components
Сервер состоит из компонент, каждый в виде отдельной статической библиотеки:
- Allocator (include/afina/allocator/, src/allocator): менеджер памяти
- Storage (include/afina/Storage.h, src/storage): хранилище данных 
- Execute (include/afina/execute/, src/execute/): комманды, сервер создает экземпляры комманд на основе сообщений из сети и применяет их над заданным хранилищем
- Network (src/network/): сетевой слой, реализует подмножество memcached текстового протокола

# How to build
Для сборки нужен cmake >= 3.0.1, gcc > 4.9 и ядро 4.5+. Система сборки автоматически использует ccache если последний найден в системе:
```
[user@domain afina] mkdir build
[user@domain afina] cd build
[user@domain build] cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=y -DECM_ENABLE_SANITIZERS="memory,address" ..
[user@domain build] make
```

# Сервер:
```
[user@domain build] ./src/afina
```

Поддерживает следующий опции:
- --network <st_block, mt_block, non_block> какую использовать реализацию сети
  - *st_block*: все в одном треде
  - *mt_block*: 1 тред на каждое соединение (домашка)
  - *non_block*: многопоточный epoll (домашка)
- --storage <st_lru, mt_lru> какую реализацию хранилища использовать
  - *st_lru*: LRU без синхронизации (домашка)
  - *mt_lru*: LRU с глобальным локом (домашка)

Вот так можно отправить комманды:
```
echo -n -e "set foo 0 0 6\r\nfooval\r\n" | nc localhost 8080
```
обратите внимание на -e и -n

А вот тут подробнее про систему комманд: https://github.com/memcached/memcached/blob/master/doc/protocol.txt

# Tests
```
make runExecuteTests && ./test/execute/runExecuteTests - собрать и запустить тесты комманд
make runProtocolTests && ./test/protocol/runProtocolTests - собрать и запустить тесты парсера memcached протокола
make runStorageTests && ./test/storage/runStorageTests - собрать и запустить тесты хранилиза данных
```

# TODO
- benchmarks
- integration tests
