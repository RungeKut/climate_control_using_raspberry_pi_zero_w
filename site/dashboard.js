
var db_data = JSON.parse(document.getElementById('block').getAttribute('data-attr'));
var db_temperature = JSON.parse(document.getElementById("block").getAttribute('data-temperature'));
var db_humidity = JSON.parse(document.getElementById("block").getAttribute('data-humidity'));

(() => {
  // Graphs
  const ctx = document.getElementById('myChart')
  // eslint-disable-next-line no-unused-vars
  var temperatureData =
  {
    label: 'Температура, °C',
  //fill: false, //если нужна линия без заливки
    data: db_temperature,
    tension: 0, //Кривая Безье – натяжение линии
    backgroundColor: 'rgba(254,0,0,0.2)', //Цвет графика, секции или столбца
    borderColor: 'red',
    borderWidth: 1,
    pointBorderColor: 'transparent',
    pointBackgroundColor: 'transparent',
    pointRadius: 0,
    //pointHoverRadius: 10,
    //pointHitRadius: 30,
    //pointBorderWidth: 2,
    pointStyle: 'rectRounded',
  //cubicInterpolationMode: 'monotone'
  }
  var humidityData =
  {
    label: 'Влажность, %',
    data: db_humidity,
    tension: 0, //Кривая Безье – натяжение линии
    backgroundColor: 'rgba(0,0,254,0.2)', //Цвет графика, секции или столбца
    borderColor: 'blue', //Цвет рамки
  //hoverBackgroundColor: "rgba(255,99,132,0.4)", //Цвет графика, секции или столбца при наведении курсора
  //hoverBorderColor: "rgba(255,99,132,1)", //Цвет рамки при наведении курсора
    borderWidth: 1, //
    pointBorderColor: 'transparent',
    pointBackgroundColor: 'transparent',
    pointRadius: 0,
    pointStyle: 'rectRounded'
  }
  var chartOptions =
  {
  /*title: { //Заголовок диаграммы
      display: true, //Признак представления заголовка
      position: 'top', //Позиционирование заголовка возможные значения : top,left,bottom,right
      fontSize: 12, //Размер шрифта
      fontFamily: 'Helvetica', //Семейство шрифтов
      fontColor: '#666', //Цвет шрифта
      fontStyle: 'bold', //Стиль шрифта
      padding: 10, //Отступ сверху и снизу от заголовка в пикселях
      text: 'Граффик' //Титульная строка
    },*/
    scales: {
      yAxes: [{
        ticks: {
          beginAtZero: false // Ось Y начинать отсчет с нуля
        }
      }]
    },
    legend: { //Легенда диаграммы
      display: true, //Показать легенду
      position: 'top', //Позиционирование легенды возможные значения : top,left,bottom,right
      fullWidth: true,
    //onClick: //Функция обработки нажатия на метку легенды
    //onHover: //Функция обработки наведения курсора мыши на метку легенды
      reverse: false, //Порядок представления набора данных легенды
      labels: { //Конфигурация метки легенды
        boxWidth: 40, //Размер расцвечиваемого прямоугольника метки
        fontColor: 'black', //Цвет текста метки
        fontFamily: 'Calibri Light', //Семейство шрифтов меток легенды
        fontStyle: 'italic', //Стиль шрифта метки
        FontSize: 14, //Размер шрифта (пиксель)
        padding: 10 //Расстояние по вертикали между расцвеченными прямоугольниками
      }
    },
    tooltips: {
      enabled: false, //Признак подключения всплывающих подсказок
      custom:	null, //Подключение функции обработки
      mode:	'nearest', //Режим подключения; возможные значения [point, nearest, index, dataset, x, y]
      intersect: true, //true - представление подсказки при пересечении курсора с «графиком» false - постоянное представление подсказки
      position: 'average', //Позиционирование подсказки; возможные значения [average, nearest];
      callbacks: { //Функция представления подсказки (пример);
        labelColor: function(tooltipItem, chart) {
          return {
            borderColor: 'rgb(255, 255, 0)',
            backgroundColor: 'rgb(255, 0, 0)'
          }
        }
      },
    //itemSort: , //Функция сортировки элементов подсказки
    //filter: , //Функция фильтрации элементов подсказки
      backgroundColor: 'rgba(0,0,0,0.8)', //Цвет фона окна
      titleFontFamily: 'Helvetica Neue', //Шрифт заголовка подсказки
      titleFontSize: 12, //Размер шрифта заголовка подсказки
      titleFontStyle:	'bold', //Стиль шрифта заголовка подсказки
      titleFontColor:	'#fff', //Цвет заголовка подсказки
      titleSpacing:	2, //Расстояние сверху и снизу для каждой строки
      titleMarginBottom: 6, //Отступ снизу
      bodyFontFamily:	'Helvetica Neue', //Шрифт подсказки
      bodyFontSize:	12, //Размер шрифта
      bodyFontStyle: 'normal', //Стиль шрифта
      bodyFontColor: '#fff', //Цвет текста
      bodySpacing: 2, //Отступ сверху и снизу для каждой записи
      footerFontFamily: 'Helvetica Neue', //Шрифт нижнего колонтитула
      footerFontSize: 12, //Размер шрифта нижнего колонтитула
      footerFontStyle: 'bold', //Стиль шрифта нижнего колонтитула
      footerFontColor: '#fff', //Цвет текста нижнего колонтитула
      footerSpacing: 2, //Отступ сверху и снизу для каждой строки нижнего колонтитула
      footerMarginTop: 6, //Отступ сверху для нижнего колонтитула
      xPadding: 6, //Отступ слева/справа
      yPadding: 6, //Отступ сверху/снизу
      cornerRadius: 6, //Радиус угловых рамок
      borderColor: 'rgba(0,0,0,0)', //Цвет рамки
      borderWidth: 0 //Толщина рамки
    }
  }
  const myChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: db_data,
      datasets: [
        temperatureData,
        humidityData
      ],
    },
    options: chartOptions
    })
})()
