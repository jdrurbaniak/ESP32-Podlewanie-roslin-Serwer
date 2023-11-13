const ctx = document.getElementById('humidityChart');

let data = [];

chart = new Chart(ctx, {
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
    responseArray = textResponse.trim().split("\n"); // Trim po to żeby nie było pustych elementów
    return responseArray;
  }
  
  const dateSelectElement = document.getElementById("date-select");

  async function loadSensorData(deviceAddress, date) {
    data.length = 0; // Czyszczenie tablicy z danymi
    console.log("Ładowanie danych...");
    requestURI = "managed-sensors?device=" + deviceAddress + "&date=" + date;
    dataArray = await fetchArray(requestURI);
    dataArray.forEach(e => {
      splitDataArray = e.split(" ");
      if(date != "2023-11-05") // Do usunięcia później, w tym dniu zapisałem dane w systemie dziesiętnym
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
    chart.update();
    document.getElementById('connection-loading').classList.add('visually-hidden');
    document.getElementById('chart-container').classList.remove('visually-hidden');
  }

  async function loadDates(deviceAddress) {
    console.log("Ładowanie dat...");
    requestURI = "managed-sensors?device=" + deviceAddress;
    datesArray = await fetchArray(requestURI);
    console.log(datesArray);
    dateSelectElement.classList.remove("placeholder");
    dateSelectElement.disabled = false;
    datesArray.forEach(e => {
      const dateOptionElement = document.createElement("option");
      dateOptionElement.innerText = removeExtension(e);
      dateSelectElement.appendChild(dateOptionElement);
    });
    loadSensorData(deviceAddress, removeExtension(datesArray[0]));
  }

  const sensorSelectElement = document.getElementById("sensor-select");;

  async function loadDevices() {
    console.log("Ładowanie listy sensorów...");
    devicesArray = await fetchArray("managed-sensors");
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
    document.getElementById('connection-loading').classList.remove('visually-hidden');
    document.getElementById('chart-container').classList.add('visually-hidden');
    loadDates(sensorSelectElement.value);
  });

  dateSelectElement.addEventListener("change", () => {
    document.getElementById('connection-loading').classList.remove('visually-hidden');
    document.getElementById('chart-container').classList.add('visually-hidden');
    loadSensorData(sensorSelectElement.value, dateSelectElement.value);
  })

  window.onload = (e) => {
    loadDevices().catch(error => {
      console.error("Nie można załadować listy urządzeń");
      console.error(error);
    });
  }