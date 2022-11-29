/* globals Chart:false, feather:false */

(() => {
  'use strict'

  feather.replace({ 'aria-hidden': 'true' })

  // Graphs
  const ctx = document.getElementById('myChart')
  var temperatureData =
  {
    labels: [
      $inputArray[0][0],
      'Monday',
      'Tuesday',
      'Wednesday',
      'Thursday',
      'Friday',
      'Saturday'
    ],
    datasets: [{
      labels: "Температура",
      data: [
        15339,
        21345,
        18483,
        24003,
        23489,
        24092,
        1034
      ],
      lineTension: 0,
      fill: false,
      backgroundColor: 'transparent',
      borderColor: '#007bff',
      borderWidth: 4,
      cubicInterpolationMode: 'monotone',
      pointBackgroundColor: '#007bff'
    }]
  }
  var chartOptions =
  {
    scales: {
      yAxes: [{
        ticks: {
          beginAtZero: false
        }
      }]
    },
    legend: {
      display: false
    }
  }
  // eslint-disable-next-line no-unused-vars
  const myChart = new Chart(ctx, {
    type: 'line',
    data: temperatureData,
    options: chartOptions
    })
})()
