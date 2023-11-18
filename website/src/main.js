import { Chart } from 'chart.js/auto';
import { DateTime } from 'luxon';
import 'chartjs-adapter-luxon';
import AirDatepicker from 'air-datepicker';
import 'air-datepicker/air-datepicker.css';
import localePL from 'air-datepicker/locale/pl';
import {createPopper, hide} from '@popperjs/core';

let datePicker = new AirDatepicker('#date-select', {
  locale: localePL,
  // container: '#scroll-container',
  visible: false,
  autoClose: false,
  range: true,
  multipleDatesSeparator: ' - ',
  onSelect({date}) {
    if(dateSelectElement.value != '')
    {
      
      if(datePicker.selectedDates != [])
      {
        showLoadingScreen();
        loadSensorData(sensorSelectElement.value, datePicker.selectedDates);
      }
    }   
  },
  position({$datepicker, $target, $pointer, done}) {
      let popper = createPopper($target, $datepicker, {
          placement: 'bottom',
          modifiers: [
              {
                  name: 'flip',
                  options: {
                      padding: {
                          top: 64
                      }
                  }
              },
              {
                  name: 'offset',
                  options: {
                      offset: [0, 20]
                  }
              },
              {
                  name: 'arrow',
                  options: {
                      element: $pointer
                  }
              }
          ]
      })
      
      /*
   Return function which will be called when `hide()` method is triggered,
   it must necessarily call the `done()` function
    to complete hiding process 
  */
      return function completeHide() {
          popper.destroy();
          done();
      }    
  }
})

const ctx = document.getElementById('humidityChart');

let data = [];

let chart = new Chart(ctx, {
  type: 'line',
  data: {
    datasets: [{
      label: 'Wilgotność gleby [%]',
      data: data,
      borderWidth: 1
    }]
  },
  options: {
    scales: {
      x: {
        type: 'time',
        time: {
            unit: 'minute',
            displayFormats: {
                minute: 'DD T'
            },
            tooltipFormat: 'DD T'
        },
        title: {
            display: true,
            text: 'Czas'
        }
      },
      y: {
        beginAtZero: true
      }
    }
  }
});

function showLoadingScreen()
{
  document.getElementById('connection-loading').classList.remove('visually-hidden');
  document.getElementById('chart-container').classList.add('visually-hidden');
}

function hideLoadingScreen()
{
  document.getElementById('connection-loading').classList.add('visually-hidden');
  document.getElementById('chart-container').classList.remove('visually-hidden');
}

// function updateHumidity(humidity)
// {
//     data.push({
//         x: new Date().getTime(),
//         y: humidity
//     })
//     chart.update();
// }

// if (!!window.EventSource) {
//     var source = new EventSource("/events");
  
//     source.addEventListener(
//       "open",
//       function (e) {
//         console.log("Events Connected");
//       },
//       false
//     );
//     source.addEventListener(
//       "error",
//       function (e) {
//         if (e.target.readyState != EventSource.OPEN) {
//           console.log("Events Disconnected");
//         }
//       },
//       false
//     );
  
//     source.addEventListener(
//       "message",
//       function (e) {
//         console.log("message", e.data);
//       },
//       false
//     );
  
//     source.addEventListener(
//       "new_readings",
//       function (e) {
//         var obj = JSON.parse(e.data);
//         updateHumidity(obj.humidity);
//       },
//       false
//     );
//   }

  function removeExtension(text) {
    return text.substring(0, text.lastIndexOf('.'))
  }

  async function fetchArray(URI)
  {
    const response = await fetch(URI);
    const textResponse = await response.text();
    let responseArray = textResponse.trim().split("\n"); // Trim po to żeby nie było pustych elementów
    return responseArray;
  }
  
  const dateSelectElement = document.getElementById("date-select");


  var datesArray = [];

  async function loadSensorData(deviceAddress, selectedDates) {
    data.length = 0; // Czyszczenie tablicy z danymi
    console.log("Ładowanie danych...");
    let promiseArray = [];
    let datesToFetch = [];
    let formattedStartDate = datePicker.formatDate(selectedDates[0], 'yyyy-MM-dd');
    if(selectedDates.length == 1) {
      datesToFetch[0] = formattedStartDate;
    } else {
      let formattedEndDate = datePicker.formatDate(selectedDates[1], 'yyyy-MM-dd');
      datesToFetch = datesArray.slice(datesArray.indexOf(formattedStartDate), datesArray.indexOf(formattedEndDate)+1);
    }
    datesToFetch.forEach((date) => {
      let requestURI = "managed-sensors?device=" + deviceAddress + "&date=" + date;
      promiseArray.push(fetchArray(requestURI))
    })
    Promise.all(promiseArray).then((response) => {
      response.forEach((dataArray, index) => {
        let formattedProcessedDate = datePicker.formatDate(datesToFetch[index], 'yyyy-MM-dd');
        dataArray.forEach(e => {
          let splitDataArray = e.split(" ");
          let xToDec;
          if(formattedProcessedDate != "2023-11-05") // Do usunięcia później, w tym dniu zapisałem dane w systemie dziesiętnym
          {
            xToDec = parseInt(splitDataArray[0], 16) * 1000; // Zamiana na system dziesiętny i milisekundy
          }
          else
          {
            xToDec = parseInt(splitDataArray[0], 10) * 1000;
          }
          
          data.push({
            x: xToDec,
            y: splitDataArray[1]
          });
        });
      })
      chart.update();
      hideLoadingScreen();
    })
  }

  async function loadDates(deviceAddress) {
    console.log("Ładowanie dat...");
    let requestURI = "managed-sensors?device=" + deviceAddress;
    datesArray = await fetchArray(requestURI);
    datesArray.forEach((el, index) => {
      datesArray[index] = removeExtension(el);
    });
    console.log(datesArray);
    dateSelectElement.classList.remove("placeholder");
    dateSelectElement.disabled = false;
    // datesArray.forEach(e => {
    //   const dateOptionElement = document.createElement("option");
    //   dateOptionElement.innerText = removeExtension(e);
    //   dateSelectElement.appendChild(dateOptionElement);
    // });
    let selectedDate = datesArray[datesArray.length-1];
    datePicker.clear({silent: true})
    datePicker.update({
      minDate: datesArray[0],
      maxDate: selectedDate,
    })
    datePicker.selectDate(selectedDate);
    //loadSensorData(deviceAddress, selectedDate);
  }

  const sensorSelectElement = document.getElementById("sensor-select");;

  async function loadDevices() {
    console.log("Ładowanie listy sensorów...");
    let devicesArray = await fetchArray("managed-sensors");
    console.log(devicesArray);
    sensorSelectElement.classList.remove("placeholder");
    sensorSelectElement.disabled = false;
    devicesArray.forEach(e => {
      const deviceOptionElement = document.createElement("option");
      deviceOptionElement.innerText = e;
      sensorSelectElement.appendChild(deviceOptionElement);
    });
    loadDates(devicesArray[0]);
  }

  sensorSelectElement.addEventListener("change", () => {
    dateSelectElement.classList.add("placeholder");
    dateSelectElement.disabled = true;
    dateSelectElement.innerHTML = "";
    showLoadingScreen();
    loadDates(sensorSelectElement.value);
  });

  // dateSelectElement.addEventListener("select", () => {
  //   showLoadingScreen();
  //   loadSensorData(sensorSelectElement.value, dateSelectElement.value);
  // })

  window.onload = (e) => {
    loadDevices().catch(error => {
      console.error("Nie można załadować listy urządzeń");
      console.error(error);
    });
  }