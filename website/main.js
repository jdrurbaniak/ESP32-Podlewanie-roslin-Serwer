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

ViteExpress.listen(app, port, () =>
  console.log(`Server is listening on port ${port}...`)
);
