//modules
var level = require("level");
const lineReader = require("line-reader");

// // Create or open the underlying LevelDB store
var db = level("./skill26_mydb", { valueEncoding: "json" });

// Function to stream from database
function readDB(arg) {
  db.createReadStream()
    .on("data", function (data) {
      console.log(data.key, "=", data.value);
      // Parsed the data into a structure but don't have to ...
      var dataIn = { [data.key]: data.value };
      //console.log(db.get(data.key, data.value));
      // Stream data to client
      //   io.emit("message", dataIn);
    })
    .on("error", function (err) {
      console.log("Oh my!", err);
    })
    .on("close", function () {
      console.log("Stream closed");
    })
    .on("end", function () {
      console.log("Stream ended");
    });
}
// Read from smoke.txt and put data into a data structure
//variable that holds data
var count = 0;
lineReader.eachLine("smoke.txt", function (line) {
  if (count != 0) {
    // var date = Date.now();
    linestr = String(line).split("\t");

    //console.log(linestr);
    var value = [
      {
        Time: parseInt(linestr[0]),
        ID: parseInt(linestr[1]),
        Smoke: parseInt(linestr[2]),
        Temp: parseFloat(linestr[3]),
      },
    ];

    // Put structure into database based on key == date, and value
    db.put([count], value, function (err) {
      if (err) return console.log("Ooops!", err); // some kind of I/O error
    });
  }

  count++;
});

var delayInMilliseconds = 2000;
setTimeout(function () { }, delayInMilliseconds);

// query from database - sensor ids, smoke, temperature
console.log( db.get(1, function (err, value) { //change # for desired query
  if (err) {
    console.error("null");
  }
  else {
    console.log("Query 1: ", value);
  }
}));


//readDB();

// console.log(db.get(1));
//console.log(db.get(value));
//level.open('skill26_mydb');
//console.log(Level.get(linestr));
