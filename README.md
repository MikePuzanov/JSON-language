# JSON-language

<a href="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml"><img src="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml/badge.svg?branch=master" alt="Build Status"></a>


# Cервер

## Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки: `mkdir build && cd build`
5. Соберите проект: `cmake .. && make`
6. Запустите сервер: `./json-server`

## Тестирование

1. Убедитесь, что вы создали директорию для сборки проекта: `mkdir build && cd build`
2. Убедитесь, что вы уже собрали проект: `cmake .. && make`
3. Запустите проект: `./json-server`
4. Откройте еще одно терминала
5. Перейдите в /tests : `cd json-server/build/json-server/tests`
6. Запустите тесты: `./tests`
