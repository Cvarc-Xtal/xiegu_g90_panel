# xiegu_g90_panel

Это проект по замене штатной панели на трансивере xiegu-g90
Основан на дисплейном модуле со встроенным контроллером esp32s3 и наработках<br>
по реверс инжинирингу https://github.com/zeroping/xiegu-g90-headprotocol
<br><br>
<img src="1.png" alt="">
<img src="2.png" alt="">
https://aliexpress.ru/item/1005006729377800.html?sku_id=12000038112486569&spm=a2g2w.productlist.search_results.0.b16113b9wN5BTJ<br>
(обязательно с емкостным тачскрином)

Все управление через тачскрин и энкодер.<br>
Интерфейс выглядит приблизительно вот так: <br>
<img src="3.jpg" alt=""><br>
<br><br>
Все области тачскрина на экране функциональны.<br>
Схема панели в файле esp32s3-panel-xiegu-g90.pdf<br>
<br><br>
На данный момент реализованы все основные функции трансивера<br>
за исключением SPLIT и переключения vfoA/vfoB (за не надобностью).<br>

Также реализован (пока без тестирования) базовый функционал управления через CAT-интрефейс<br>
(прием/передача, смена частоты и режима (LSB/USB/CW/AM))

Краткая инструкция:<br>
Вращение энкодера изменяет текущие параметры:<br>
  - частоту, если не выведены на экран другие параметры
  - громкость, мощность, частоты скатов фильтров и др, если они выведены на экран.
  - нажатие на энкодер - включение трансивера
  - короткое нажатие на энкодер во время изменения параметров возвращает в режим перестройки частоты
  - длительное нажатие на энкодер во время работы - выключение трансивера

Области экрана для тачскрина (сверху вниз, слева направо):
  - S-метр - сохранение основных текущих параметров (восстанавливаются после включения)
  - Частота настройки - выбор диапазона
  - RIT - включение/выключение режима расстройки
  - Область текущих параметров - включение/выключение меню настроек и упрвления
  - Область спектра(без выведенных параметров) - левая половина уменьшает частоту настройки -24кГц
  - Область спектра(без выведенных параметров) - правая половина увеличивает частоту настройки +24кГц
  - Область спектра(с выведенными параметрами) - левая половина уменьшает показания
  - Область спектра(с выведенными параметрами) - правая половина увеличивает показания
  - Область "водопада" - левая/правая половина - увеличивает/уменьщает громкость на 1 ступень


Добавлена функция сканирования КСВ в пределах текущего диапазона.

Настройки Ардуино для сборки проекта:<br>
<img src="settings_arduino.jpg" alt=""><br>

Коротенькое видео:<br>


https://github.com/user-attachments/assets/96ce7fe5-5e5e-4ab0-87d7-d71a0492657b

