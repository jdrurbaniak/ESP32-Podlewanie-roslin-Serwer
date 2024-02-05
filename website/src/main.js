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
        loadSensorData(devicesArray[sensorSelectElement.selectedIndex], datePicker.selectedDates);
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
  document.getElementById('settings-button').disabled = true;
}

function hideLoadingScreen()
{
  document.getElementById('connection-loading').classList.add('visually-hidden');
  document.getElementById('chart-container').classList.remove('visually-hidden');
}



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

const sensorSelectElement = document.getElementById("sensor-select");;
const dateSelectElement = document.getElementById("date-select");
const minimumMoistureLevelInputElement = document.getElementById('minimumMoistureLevelInput');
const maximumMoistureLevelInputElement = document.getElementById('maximumMoistureLevelInput');
const deviceNameInputElement = document.getElementById('deviceNameInput');
var datesArray = [];
var devicesArray = [];

async function loadSettingsData(deviceAddress) {
  document.getElementById("macAddressInput").value = deviceAddress;
  let URI = "/sensor-settings/" + deviceAddress;
  document.getElementById('settings-button').disabled = false;
  const response = await fetch(URI);
  const jsonResponse = await response.json();
  if(jsonResponse["deviceName"] != "unnamed-device") {
    sensorSelectElement.children[sensorSelectElement.selectedIndex].innerText = jsonResponse["deviceName"]
    deviceNameInputElement.value = jsonResponse["deviceName"];
  } else {
    deviceNameInputElement.value = jsonResponse["deviceName"] = "";
  }
  minimumMoistureLevelInputElement.value = jsonResponse["minimumMoistureLevel"];
  maximumMoistureLevelInputElement.value = jsonResponse["maximumMoistureLevel"];
}

async function loadSensorData(deviceAddress, selectedDates) {
  data.length = 0;
  let promiseArray = datesToFetch = [];
  let formattedStartDate = datePicker.formatDate(selectedDates[0], 'yyyy-MM-dd');
  if(selectedDates.length == 1) {
    datesToFetch[0] = formattedStartDate;
  } else {
    let formattedEndDate = datePicker.formatDate(selectedDates[1], 'yyyy-MM-dd');
    datesToFetch = datesArray.slice(datesArray.indexOf(formattedStartDate), datesArray.indexOf(formattedEndDate)+1);
  }
  datesToFetch.forEach((date) => {
    let requestURI = "sensor-data/" + deviceAddress + "&date=" + date;
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
    loadSettingsData(deviceAddress);
  })
}

async function loadDates(deviceAddress) {
  let requestURI = "sensor-data/" + deviceAddress;
  datesArray = await fetchArray(requestURI);
  datesArray.forEach((el, index) => {
    datesArray[index] = removeExtension(el);
  });
  dateSelectElement.classList.remove("placeholder");
  dateSelectElement.disabled = false;
  let selectedDate = datesArray[datesArray.length-1];
  datePicker.clear({silent: true})
  datePicker.update({
    minDate: datesArray[0],
    maxDate: selectedDate,
  })
  datePicker.selectDate(selectedDate);
}

async function loadDevices() {
  devicesArray = await fetchArray("sensor-data");
  devicesArray.forEach(element => {
    const deviceOptionElement = document.createElement("option");
    deviceOptionElement.innerText = element;
    sensorSelectElement.appendChild(deviceOptionElement);
  });
  sensorSelectElement.classList.remove("placeholder");
  sensorSelectElement.disabled = false;
  loadDates(devicesArray[0]);
}

sensorSelectElement.addEventListener("change", () => {
  dateSelectElement.classList.add("placeholder");
  dateSelectElement.disabled = true;
  dateSelectElement.innerHTML = "";
  showLoadingScreen();
  loadDates(devicesArray[sensorSelectElement.selectedIndex]);
});

window.onload = (e) => {
  loadDevices().catch(error => {
    console.error("Nie można załadować listy urządzeń");
    console.error(error);
  });
}