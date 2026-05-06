const express = require('express');
const fs = require('fs');
const app = express();

app.use(express.json());

const CSV_FILE = '/sdcard/Download/weather_log.csv';

if (!fs.existsSync(CSV_FILE)) {
  fs.writeFileSync(CSV_FILE, 'Timestamp,Temperature(°C),Humidity(%),Light\n');
  console.log("✅ CSV File Created");
}

app.post('/data', (req, res) => {
  const { temperature, humidity, light } = req.body;
  const timestamp = new Date().toLocaleString('en-IN');
  
  fs.appendFileSync(CSV_FILE, `${timestamp},${temperature},${humidity},${light}\n`);
  
  console.log(`📊 Logged: ${temperature}°C | ${humidity}% | Light=${light}`);
  res.json({ status: 'success' });
});

app.listen(3000, () => {
  console.log("🚀 Server Running on Port 3000");
  console.log("📁 Log saved to: /sdcard/Download/weather_log.csv");
});