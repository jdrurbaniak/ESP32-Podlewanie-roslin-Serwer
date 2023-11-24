const fs = require('fs')
const path = require('path');
const express = require('express')
const ViteExpress = require("vite-express");
const app = express()
const port = 3000

function dirToString(dirName) {
  let dirArray = fs.readdirSync(dirName)
  let result = "";
  dirArray.forEach((el) => {
    result += `${el}\n`;
  });
  return result;
}

//app.use(express.static(`data/www`))

app.get('/managed-sensors', (req, res) => {
  let device = req.query.device;
  let date = req.query.date;
  console.log(`device: ${device}`);
  console.log(`date: ${date}`);
  if(device != undefined) {
    if(date != undefined) {
      let fileName = path.join(__dirname, `../data/sensordata/${device}/data/${date}.txt`)
      res.sendFile(fileName)
    } else {
      res.send(dirToString(`../data/sensordata/${device}/data`))
    }
  }
  else
  {
    res.send(dirToString('../data/sensordata'));
  }
})

app.use(express.json());

app.post('/update-device-settings', (req, res) => {
  console.log(req.body);
  res.sendStatus(200);
});

app.get('/get-device-settings', (req, res) => {
  let device = req.query.device;
  console.log(`device: ${device}`);
  if(device != undefined) {
      let fileName = path.join(__dirname, `../data/defaults/sensorConfig.json`)
      res.sendFile(fileName)
  }
})


ViteExpress.listen(app, port, () =>
  console.log(`Server is listening on port ${port}...`)
);
