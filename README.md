# JSON-language

<a href="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml"><img src="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml/badge.svg?branch=master" alt="Build Status"></a>


## Cервер

### Linux

#### Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки: `mkdir build && cd build`
5. Соберите проект: `cmake .. && make`
6. Запустите сервер: `./json-server`

#### Тестирование

1. Убедитесь, что вы создали директорию для сборки проекта: `mkdir build && cd build`
2. Убедитесь, что вы уже собрали проект: `cmake .. && make`
3. Запустите проект: `./json-server`
4. Откройте еще одно окно терминала
5. Перейдите в /tests : `cd json-server/build/json-server/tests`
6. Запустите тесты: `./tests`

### Windows

#### Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки и перейдите в нее: `mkdir build`, `cd build`
5. Соберите проект: `cmake ..`, `cmake --build .`
6. Запустите сервер: `./Debug/json-server.exe`

#### Тестирование

1. Убедитесь, что вы создали директорию для сборки проекта  и перешли в нее: `mkdir build`,`cd build`
2. Убедитесь, что вы уже собрали проект:: `cmake ..`, `cmake --build .`
3. Запустите проект: `.Debug/json-server.exe`
4. Откройте еще одно окно терминала
5. Перейдите в /tests : `cd json-server/build/json-server/tests`
6. Запустите тесты: `./Debug/tests.exe`

## Использование

### Добавление JSON-записи

Чтобы добавить новую запись, отправьте POST-запрос на эндпоинт `/add` с JSON-телом, содержащим набор команд куда мы хотим добавить и что хотим добавить. Пример:

```bash
curl -X POST -H "Content-Type: application/json" -d '[["one"], [1, "zs", {"v":"ret","hl":1}]]' http://your-server-address/add
```

Ожидаемый успешный ответ:`{ "status": "success" }`

### Получение JSON-записи

Чтобы получить значение по ключу, отправьте POST-запрос на эндпоинт `/get` с JSON-телом, содержащим набор команд до поля или массива, которое хотим получить. Пример:

```bash
curl -X POST -H "Content-Type: application/json" -d '["one", 2]' http://your-server-address/get
```
Если будем опираться на пример с добавление, то мы хотим получим "zs"

### Postman
Также для отправки запросов можете использовать удобную утилиту Postman.
