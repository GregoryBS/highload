# Проектирование веб-сервиса
Автор: Будкин Григорий<br>
Преподаватели: Бодин Антон, Быков Александр

## Содержание
1. [Тема и целевая аудитория](#introduction)
2. [Расчёт нагрузки](#load-calculation)
    1. [Продуктовые метрики](#product-metrics)
    2. [Технические метрики](#tech-metrics)
3. [Список литературы](#literature)


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
Dropbox не раскрывает количество активных пользователей, однако согласно анализу в месяц совершается около 172 миллионов посещений [2]. Также было объявлено, о том, что количество пользователей, приобретших подписку на дополнительное хранилище и функционал, в 2021 году достигло 15 миллионов [1].<br>
Размер хранилища, доступный при бесплатном использовании, равен 2 Гб, следовательно для обеспечения нужд пользователей нужно минимум: 700 * 10^6 * 2 = 1400 * 10^6 Гб ~ 1.37 * 10^6 Тб.<br>
*Таблица 1. Среднее количество действий пользователей [1].*
| Тип запроса | Среднее количество (млн/день) |
| --- | ----------- |
| Загрузка файлов | 345 |
| Просмотр файлов | 1200 |
| Предоставление доступа к файлу | 2.4 |

### Технические метрики <a name="tech-metrics"></a>
Так как размер пользовательских данных достаточно мал относительно размера файловых хранилищ, то будем считать, что достаточно 1.4 * 10^6 Тб памяти.<br>
Будем считать, что средний размер файлов, которые загружаются и просматриваются ежедневно, равен 100 Кб [3]. Исходя из данных в таблице 1 ежедневно поступает (1200 + 345) * 10^6 запросов на просмотр и загрузку файлов. Следовательно суммарный суточный объём сетевого трафика равен: 100 Кб * 1545 * 10^6 = 1545 * 10^8 Кб/сутки ~ 147342.7 Гб/сутки ~ 13.6 Гбит/с.<br>
*Таблица 2. Количество RPS по типам запросов.*
| Тип запроса | RPS |
| --- | ----------- |
| Загрузка файлов | 4000 |
| Просмотр файлов | 14000 |
| Предоставление доступа к файлу | 28 |

## Список литературы <a name="literature"></a>
1. https://expandedramblings.com/index.php/dropbox-statistics/
2. https://www.similarweb.com/ru/website/dropbox.com/#overview
3. https://medium.com/codex/design-dropbox-e82b61e5b197
