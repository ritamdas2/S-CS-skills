const csv = require('csv-parser');
const fs = require('fs');

const express = require('express')
const app = express()
var path = require('path');
const port = 8080

/* Sends HTML file from server to browser */
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname + '/stocks.html'));
})

/* Specifies $.get("/data") in HTML file */
app.get('/data', (req, res) => {

	var data = [];	// Array to hold CSV data

	fs.createReadStream('/data')	// create readStream from fs(?)
	.pipe(csv())						// Pipe data to csv object (inside server object)

	.on('data', (row) => {
		data.push(row);		// Appends each row to data array
	})

	.on('end', () => {
		res.send(data);		// Send CSV data to HTML page when done
	});
})

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})