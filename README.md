# JSON-language

<a href="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml"><img src="https://github.com/MikePuzanov/JSON-language/actions/workflows/cmake.yml/badge.svg?branch=master" alt="Build Status"></a>

Репозиторий предоставляет сервер для записи и получение различной информации в виде JSON, а также библиотеку для интеграции с сервером и локального взаимодействия с информацией в виде JSON.

## Cервер
При начале боевой работы очистите `galaxy.json` и настройте `serverConfig.json`. 
В директории json-server есть файл для конфигурации `serverConfig.json`, в котором можно настроить `ip` и `port` для сервера.
В директории json-server есть файл `galaxy.json`, в который сохраняются данный, которые лежат на сервере. В данный момент сохранение поддерживаются в следущем виде:

Windows:
- При закрытии терменила, в котором запущем сервис с помощью комбинации `Ctrl+Break`
- При полном закрытие окна терминал, с работающим сервисом
- Раз в 5 минут на сервере запускается job-a, которая записывает данный из галактика в `galaxy.json`

Linux:
- При полном закрытие окна терминал, с работающим сервисом
- Раз в 5 минут на сервере запускается job-a, которая записывает данный из галактика в `galaxy.json`

**Внимание:
Запуск тестов у сервера имеет побочные эффекты, данные в `galaxy.json` пропадут.**

### Linux

#### Запуск (В данном запуске используются данные из файла `serverConfig.json`) 

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки: `mkdir build && cd build`
5. Соберите проект: `cmake .. && make`
6. Запустите сервер из директории json-server: `./build/json-server`
   
Дополнительно:

6.1 Также можно запустить сервер из директории json-server: `./build/json-server ip port`, где надо указать `ip` и `port` вручную в терминале.

#### Тестирование

1. Убедитесь, что вы создали директорию для сборки проекта: `mkdir build && cd build`
2. Убедитесь, что вы уже собрали проект: `cmake .. && make`
3. Запустите проект  из директории json-server: `./build/json-server`
4. Откройте еще одно окно терминала
5. Перейдите в /tests : `cd json-server/build/json-server/tests`
6. Запустите тесты: `./tests`

### Windows

#### Запуск (В данном запуске используются данные из файла `serverConfig.json`)

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd json-server`
4. Создайте директорию для сборки и перейдите в нее: `mkdir build`, `cd build`
5. Соберите проект: `cmake ..`, `cmake --build .`
6. Запустите сервер из директории json-server: `./build/Debug/json-server.exe`

Дополнительно:

6.1 Также можно запустить сервер из директории json-server: `./build/Debug/json-server.exe`, где надо указать `ip` и `port` вручную в терминале.

#### Тестирование

1. Убедитесь, что вы создали директорию для сборки проекта  и перешли в нее: `mkdir build`,`cd build`
2. Убедитесь, что вы уже собрали проект:: `cmake ..`, `cmake --build .`
3. Запустите проект из директории json-server: `./buildDebug/json-server.exe`
4. Откройте еще одно окно терминала
5. Перейдите в /tests : `cd json-server/build/json-server/tests`
6. Запустите тесты: `./Debug/tests.exe`

## Использование сервера

### Добавление JSON-записи

Чтобы добавить новую запись, отправьте POST-запрос на эндпоинт `/add` с JSON-телом, содержащим набор команд куда хотите добавить и что хотите добавить. 

Тело запроса:

`
[path_in_galaxy, new_value]
`

path_in_galaxy - это массив [], в котором описывается последовательность полей, хранящихся в `galaxy.json`, по которым будет происходить поиск по вложенности всего объекта галактики и последним элементом должно быть названия ключа, которое будет изменено или добавлено.

new_value - это значение, которое будет присвоено ключу (последнему элементу в массиве path_in_galaxy). new_value может быть массивом [], объектом {}, числом или строкой.

Пример:

```bash
curl -X POST -H "Content-Type: application/json" -d '[["one"], [1, "zs", {"v":"ret","hl":1}]]' http://127.0.0.1:4000/add
```

В данном примере видно, что `path_in_galaxy = ["one"]`, значит будет добавлен или изменен ключ "one". Новым значением ключа "one" будет `new_value = [1, "zs", {"v":"ret","hl":1}][1, "zs", {"v":"ret","hl":1}]`.

В итоге в галактике окажется, следующая запись:

```
{
  "one" : [1, "zs", {"v":"ret","hl":1}]
}
```

Ожидаемый успешный ответ:`{ "status": "success" }`

### Получение JSON-записи

Чтобы получить значение по ключу, отправьте POST-запрос на эндпоинт `/get` с JSON-телом, содержащим набор команд до поля или массива, которое хотите получить.

Тело запроса:

`
[path_in_galaxy]
`

path_in_galaxy - это массив [], в котором описывается последовательность полей, хранящихся в `galaxy.json`, по которым будет происходить поиск по вложенности всего объекта галактики и последним элементом должно быть названия ключа, которое хотите получить.

Пример:

```bash
curl -X POST -H "Content-Type: application/json" -d '["one", 2]' http://127.0.0.1:4000/get
```

В прошлом примере добавили запись 

```
{
  "one" : [1, "zs", {"v":"ret","hl":1}]
}
```

Как можно заметить ключу `"one"` соответсвует массив `[1, "zs", {"v":"ret","hl":1}]`. Значит для того, чтобы получить конкретный элемент из этого массива, нужно указать индекс интерисующего нас элемента. В примере указан индекс = 2.

В итоге получаем от сервера ответ: `{"v":"ret","hl":1}`

### Как очистить галактик с помощью JSON-запроса

Чтобы очистить галактик, отправьте POST-запрос на эндпоинт `/add`. 

POST-запрос на эндпоинт `/add`:

```bash
curl -X POST -H "Content-Type: application/json" -d '[[], null]' http://127.0.0.1:4000/add
```


### Postman
Также для отправки запросов можно использовать удобную утилиту Postman.

## Библиотека

Библиотека позволяет сохранять данные локально или отправлять их на нужный url, если он указан в последовательности. Библиотека работает с такими же форматами данными как и сервер ([Пример](https://github.com/MikePuzanov/JSON-language?tab=readme-ov-file#использование)).
Единственное отличие заключается в том, что если мы хотим взаимодействовать с сервером, в массиве `path_in_galaxy` первым элементом должен быть адрес сервера.

**Внимание:
Запуск тестов у библиотеки имеет побочные эффекты, потому что в них присутствует взаимодействием с сервером, который мы запускаем для [тестирования](https://github.com/MikePuzanov/JSON-language?tab=readme-ov-file#linux). Из-за этого ваши данные, которые храняться в galaxy.json пропадут.**

### Linux

#### Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++.
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd library`
4. Создайте директорию для сборки: `mkdir build && cd build`
5. Соберите проект: `cmake .. && make`

#### Тестирование

1. Запустите cервер, смотрите [раздел Сервер](https://github.com/MikePuzanov/JSON-language?tab=readme-ov-file#linux).
2. Откройте еще одно окно терминала
3. Убедитесь, что вы создали директорию для сборки проекта и перешли в нее, находясь в library: `mkdir build && cd build`
4. Убедитесь, что вы уже собрали проект: `cmake .. && make`
5. Перейдите в /tests : `cd library/build/library/tests`
6. Запустите тесты: `./tests`

### Windows

#### Запуск

1. Убедитесь, что у вас установлен CMake и компилятор C++. 
2. Клонируйте репозиторий
3. Перейдите в директорию с проектом: `cd library`
4. Создайте директорию для сборки и перейдите в нее: `mkdir build`, `cd build`
5. Соберите проект: `cmake ..`, `cmake --build .`

#### Тестирование

1. Запустите cервер, смотрите [раздел Сервер](https://github.com/MikePuzanov/JSON-language?tab=readme-ov-file#windows).
2. Откройте еще одно окно терминала
3. Убедитесь, что вы создали директорию для сборки проекта и перешли в нее, находясь в library: `mkdir build`,`cd build`
4. Убедитесь, что вы уже собрали проект:: `cmake ..`, `cmake --build .`
5. Перейдите в /tests : `cd library/build/library/tests`
6. Запустите тесты: `./Debug/tests.exe`
