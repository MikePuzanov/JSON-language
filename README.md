# JSON-language

## Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий: `git clone https://github.com/MikePuzanov/json-server.git`
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки: `mkdir build && cd build`
5. Соберите проект: `cmake .. && make`
6. Запустите сервер: `./json-server`

## Тестирование

1. Перейдите в директорию tests в корне проекта.
2. Создайте директорию для сборки тестов: `mkdir build && cd build`
3. Соберите тесты: `cmake .. && make`
4. Перейдите в ./tests : `cd /tests`
5. Запустите тесты: `./tests`
