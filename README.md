# Проектирование веб-сервиса
Автор: Будкин Григорий<br>
Преподаватели: Бодин Антон, Быков Александр

## Содержание
1. [Тема и целевая аудитория](#introduction)
2. [Расчёт нагрузки](#load-calculation)
    1. [Продуктовые метрики](#product-metrics)
    2. [Технические метрики](#tech-metrics)
3. [Логическая схема БД](#logic-db)
4. [Физическая схема БД](#phys-db)
5. [Технологии](#techs)
6. [Схема проекта](#scheme)
7. [Список серверов](#servers)
8. [Список литературы](#literature)


## Тема и целевая аудитория <a name="introduction"></a>
В качестве темы выбран сервис Dropbox -  файловый хостинг компании Dropbox Inc., включающий персональное облачное хранилище, синхронизацию файлов и программу-клиент.<br>
Для реализации MVP предлагается следующий функционал: 
* Загрузка файлов любого формата в персональное хранилище
* Скачивание файлов из хранилища
* Просмотр загруженных файлов через веб-интерфейс
* Предоставление доступа на просмотр файлов третьим лицам

Для рассмотрения целевой аудитории в качестве основы будут взяты данные о сервисе Dropbox. Согласно источнику [1], общее количество зарегистрированных пользователей сервиса во всём мире превышает число в 700 миллионов. При этом:
* более 75% пользователей - вне США;
* следовательно в США находится около 25% пользователей, т.е. 700 * 10^6 * 0.25 = 175 * 10^6 = 175 миллионов;
* в Японии более 10 миллионов пользователей.

## Расчёт нагрузки <a name="load-calculation"></a>
В данном разделе будут приведены основные продуктовые и технические метрики, необходимые для расчёта общей нагрузки на сервис.

### Продуктовые метрики <a name="product-metrics"></a>
Dropbox не раскрывает количество активных пользователей, однако согласно анализу в месяц совершается около 172 миллионов посещений [2]. В среднем ежедневно около 6 миллионов посещений. Пусть 50% из них происходит с запросом авторизации, тогда ежедневно выполняется 3 миллиона запросов на авторизацию.<br>
Размер хранилища, доступный при бесплатном использовании, равен 2 Гб. Удвоим размер хранилища для обеспечения нужд новых и платных пользователей: 700 * 10^6 * 2 * 2 = 2800 * 10^6 Гб ~ 2.73 * 10^6 Тб.<br>
*Таблица 1. Среднее количество действий пользователей [1].*
| Тип запроса | Среднее количество (млн/день) |
| --- | ----------- |
| Загрузка файлов | 345 |
| Просмотр файлов | 1200 |
| Предоставление доступа к файлу | 2.4 |
| Авторизация | 3 |
| Просмотр списка файлов | 6 |

### Технические метрики <a name="tech-metrics"></a>
Так как размер пользовательских данных достаточно мал относительно размера файловых хранилищ, то будем считать, что достаточно 2.75 * 10^6 Тб памяти.<br>
Обычно пользовательские файлы представляют собой изображения и текстовые документы. Будем считать, что их средний размер - 2Мб. Исходя из данных в таблице 1 рассчитаем объём входящего и исходящего трафика и RPS, учитывая, что в пике средняя нагрузка увеличена в 2 раза.<br>
*Таблица 2. Входящий трафик.*
|Тип запроса|Объём 1 запроса (Кб)|Суточный объём (Гб/сутки)| Суточный объём в пике |
| --- | ----------- | --- | --- |
|Авторизация|1|2.86|560 Кбит/с|
|Предоставление доступа к файлу|1|2.29|448 Кбит/с|
|Загрузка файлов|2048|673828|124.8 Гбит/с|

*Таблица 3. Исходящий трафик.*
|Тип запроса|Объём 1 запроса (Кб)|Суточный объём (Гб/сутки)| Суточный объём в пике |
| --- | ----------- | --- | --- |
|Просмотр списка файлов|512|2929.7|0.54 Гбит/с|
|Просмотр файлов|2048|2343749.6|434 Гбит/с|

*Таблица 4. Количество RPS по типам запросов.*
| Тип запроса | RPS |
| --- | ----------- |
| Загрузка файлов | 8000 |
| Просмотр файлов | 28000 |
| Предоставление доступа к файлу | 56 |
| Авторизация | 70 |
| Просмотр списка файлов | 140 |

## Логическая схема БД <a name="logic-db"></a>
![Database ER diagram (crow's foot)](https://user-images.githubusercontent.com/40579712/146390804-4ff28e56-ca58-4676-bd5b-7577aac3675b.png)

## Физическая схема БД <a name="phys-db"></a>
**Пользователи (таблица Users)**<br>
4 * 64 байт + 4 байт = 260 байт - 1 запись.<br>
Предположим, что в таблице будет располагаться 1 миллиард записей: 260 байт * 10^9 / (1024^3) ~ 242 Гб.

**Подписки (таблица Subscriptions)**<br>
4 + 4 + 8 + 8 = 24 байт - 1 запись.<br>
Кол-во платных пользователей Dropbox ~ 15 миллионов. Предположим, что в таблице около 100 миллионов записей: 24 байт * 10^8 / (1024^3) ~ 2.24 Гб.

**Документы (таблица Documents)**<br>
4 + 4 + 8 + 64 + 128 байт = 208 байт - 1 запись.<br>
Кол-во загруженных документов в Dropbox ~ 400 миллиардов[1]. 208 байт * 4 * 10^11 / (1024^4) ~ 75.7 Тб.

**Доступ к документам (таблицы Access и Roles)**<br>
Кол-во ролей при доступе к документам ограничено, поэтому размером таблицы Roles можно пренебречь.<br>
Не для всех документов предоставляется совместный доступ. Предположим, что половина документов над половиной всех документов работают в среднем по 3 человека. Тогда общее кол-во записей: 4 * 10^11 / 2 + 4 * 10^11 / 2 * 3 = 8 * 10^11.<br>
4 + 4 + 4 + 2 = 14 байт - 1 запись. 14 байт * 8 * 10^11 / (1024^4) ~ 10.2 Тб.

**Версии документов (таблица Versions)**<br>
Для сокращения кол-ва памяти предполагается хранить непосредственную разницу в 2 версиях документа. Предположим, что в среднем меняется по 1 Кб текста только в половине документов, а версий документа в среднем 5. Тогда общее кол-во записей: 4 * 10^11 / 2 * 5 = 10^12.<br>
4 + 4 + 4 + 4 + 8 + 1024 + 1024 = 2072 байт - 1 запись. 2072 байт * 10^12 / (1024^4) ~ 1884.5 Тб.

Так как максимальный размер таблицы одной из наиболее популярных реляционных СУБД PostgreSQL составляет 32 Тб, то рационально будет использовать шардирование БД по разным регионам.

## Технологии <a name="techs"></a>
**Backend**<br>
Для написания будет использован язык программирования Go, который набирает популярность в качестве инструмента создания высоконагруженных веб-приложений. Для повышения возможностей разработки и сопровождения кода должны быть использованы принципы чистой архитектуры с разделением на микросервисы. В первую очередь следует выделить микросервисы: пользователей (авторизации), документов и версий документов. Также можно добавить микросервис, отвечающий за синхронизацию документов для разных клиентов. 

**Database**<br>
В качестве СУБД выбрана PostgreSQL, обладающая хорошей функциональностью и поддержкой, позволяя персистентно хранить данные при высоких нагрузках. Для повышения отказоустойчивости помимо шардирования по регионам будет использована репликация баз для разделение операций на чтение (которых значительно больше) и запись.

**Load balancing**<br>
Для балансировки нагрузки между бэкендами и отдачи статических файлов будет использован веб-сервер Nginx, давно зарекомендовавший себя как эффективное средство при обработке большого кол-ва соединений.

**Frontend**<br>
Для написания клиентской части может быть использован любой подходящий фреймворк. Самые популярные из них - это React, Vue, Angular. Также следует организовать соединений с бэкендом по WebSocket для того, чтобы изменения документа с сервера передавались без дополнительных запросов к API.

**Authorization**<br>
Для авторизации без сохранения состояния можно использовать JWT, что позволит отказаться от хранения сессионных токенов на бэкенде.

## Схема проекта <a name="scheme"></a>


## Список серверов <a name="servers"></a>


## Список литературы <a name="literature"></a>
1. https://expandedramblings.com/index.php/dropbox-statistics/
2. https://www.similarweb.com/ru/website/dropbox.com/#overview
3. https://medium.com/codex/design-dropbox-e82b61e5b197
