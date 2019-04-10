'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

app.get('/escheresque_ste.png', function(req, res){
  res.sendFile(path.join(__dirname+'/public/escheresque_ste.png'));
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.filename;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    // res.send('Success Story');
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 


//char* calUpdateNewEvent(char* fileToUpdatePath, char* startDate, char* startTime, char* createDate, char* createTime, char* uid, bool isUTC, char* summary)
let parserLib = ffi.Library('./libcal', {
  'getCalJSON': ['string', ['string']],
  'calUpdateNewEvent': ['string', ['string', 'string','string', 'string', 'string', 'string', 'bool', 'string',]],
  'valCreateClientCal': ['string', ['string', 'string']]
});

function getLocOrg(array){
  
  var locOrgObj = {
    'loc' : "NULL",
    'org' : "NULL"
  };

  for(var i = 0; i < array.length; i++){

    var propname, propdesc;
    propname = array[i].split(':')[0];
    propdesc = array[i].split(':').slice(1).join(':');

    if(propname === 'LOCATION') locOrgObj.loc = '\"' + propdesc + '\"';
    if(propname === 'ORGANIZER') locOrgObj.org = '\"' + propdesc + '\"';

  }

  return locOrgObj;

}

function convertDT(dtObject){
  var dt = dtObject.date;
  var tm = dtObject.time;

  dt = dtObject.date.substring(0,4) + '-' + dtObject.date.substring(4,6) + '-' + dtObject.date.substring(6,8);
  tm = dtObject.time.substring(0,2) + ':' + dtObject.time.substring(2,4) + ':' + dtObject.time.substring(4,6);

  return '\"'  + dt + 'T' + tm + '\"';
}

//Sample endpoint
app.get('/someendpoint', function(req , res){
  
  fs.readdir(path.join(__dirname+'/uploads'), function(err, items) {
    // directoryList = JSON.parse(items);
    var calsOnServer = [];
    for(var i = 0; i < items.length; i++){
      let calStr = parserLib.getCalJSON('uploads/' + items[i]);
      // console.log('Calstr: ' + calStr);
      var calJson = {};
      try{
        calJson = JSON.parse(calStr);
        calJson.errCode = "OK";
      }catch(SyntaxError){
        calJson.errCode = calStr;
      }
      
      calJson.url = items[i];  
      calsOnServer.push(calJson);
    }
    
    res.send(calsOnServer);
  });
  
});

app.get('/updatecal', function(req, res) {
  console.log("Here: "+JSON.stringify(req.query));
  var eventToAdd = req.query;
  var calEvents = parserLib.calUpdateNewEvent('uploads/'+eventToAdd["url"], 
                                         eventToAdd["st-date"], 
                                         eventToAdd["st-time"], 
                                         eventToAdd["cr-date"], 
                                         eventToAdd["cr-time"], 
                                         eventToAdd["uid"], 
                                         eventToAdd["utc"], 
                                         eventToAdd["summary"]);

  console.log(calEvents);                                    

  res.send({
    url: eventToAdd["url"],
    status: calEvents
  });
});

app.get('/parsecal', function(req, res) {
  // console.log('id: ' + req.query.id + ' fname: ' + req.query.fname);
  var filename = req.query.fname;
  let calDetails = parserLib.getCalJSON('uploads/'+filename);
  var calJSON;
  try {
    calJSON = JSON.parse(calDetails);  
    calJSON.errCode = "OK";
  } catch (error) {
    calJSON.errCode = calDetails;
  }
  
  res.send(calJSON);
});

app.get('/writecal', function(req, res){
  // console.log(JSON.stringify(req.query));

  fs.exists('uploads/'+req.query.url, (exists) => {
    if(exists === false){
      var calstatus = parserLib.valCreateClientCal('uploads/'+req.query.url, req.query.calstr);
      if(calstatus !== 'OK'){
        fs.unlink('uploads/'+req.query.url, (err) => {
          if (err) throw err;
          console.log('path/file.txt was deleted');
        });
      }
      res.send({
        status: calstatus
      });
    }
    else{
      res.send({
        status: "file-exists"
      });
    }
  });
});


app.get('/connectdb', function(req, res){
  console.log( JSON.stringify(req.query));
  
  var usr = req.query.login;
  var pss = req.query.pass;
  var dbname = req.query.dbname;

  var connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : usr,
    password : pss,
    database : dbname
  });

  connection.connect();

  let 
    initFileTB = 'CREATE TABLE IF NOT EXISTS FILE (' +
                  'cal_id INT AUTO_INCREMENT PRIMARY KEY, ' +
                  'file_name VARCHAR(60) NOT NULL, ' +
                  'version INT NOT NULL, ' +
                  'prod_id VARCHAR(256) NOT NULL);',
    
    initEventTB = 'CREATE TABLE IF NOT EXISTS EVENT (' +
                  'event_id INT AUTO_INCREMENT PRIMARY KEY, ' +
                  'summary VARCHAR(1024), ' +
                  'start_time DATETIME NOT NULL, ' +
                  'location VARCHAR(60), ' + 
                  'organizer VARCHAR(256), ' +
                  'cal_file INT NOT NULL,' + 
                  'FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE);',
    
    initAlarmTB = 'CREATE TABLE IF NOT EXISTS ALARM (' +
                  'alarm_id INT AUTO_INCREMENT PRIMARY KEY, ' +
                  'action VARCHAR(256) NOT NULL, ' +
                  '`trigger` VARCHAR(256) NOT NULL, ' +
                  'event INT NOT NULL, ' +
                  'FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE);';

  connection.query(initFileTB, function(err, rows, fields) {
    if(err){
      console.log('SOMETHING IS WRONGIES\n' + err);
      res.send(err);
    }
    else{
      connection.query(initEventTB, function(err, rows, fields){
        if(err) {
          console.log('SOMETHING IS WRONGIES\n' + err);
          res.send(err);
        }
        else {
          connection.query(initAlarmTB, function(err, rows, fields){
            if(err){
              console.log('SOMETHING IS WRONGIES\n' + err);
              res.send(err);
            } 
            else{
              res.send({
                'code':'sup'
              });
            }
            connection.end();
          });
        }
      });
    }
  });

});

app.get('/CalsToDb', function(req, res){
  fs.readdir(path.join(__dirname+'/uploads'), function(err, items) {

    var connection = mysql.createConnection({
      host     : 'dursley.socs.uoguelph.ca',
      user     : req.query.login,
      password : req.query.pass,
      database : req.query.dbname
    });

    var calsOnServer = [];
    for(var i = 0; i < items.length; i++) {
      let calStr = parserLib.getCalJSON('uploads/' + items[i]);
      // console.log('Calstr: ' + calStr);
      var calJson = {};
      try {
        calJson = JSON.parse(calStr);
        calJson.errCode = "OK";
        calJson.url = items[i];  
        calsOnServer.push(calJson);
      } catch(SyntaxError){
        calJson.errCode = calStr;
      }
    }
    
    if(calsOnServer.length == 0) {
      res.send({
        'code': 'no_files'
      });
    }
    else {

      var 
        calNum = 0,
        eventNum = 0,
        alarmNum = 0;

      for(var i = 0; i < calsOnServer.length; i++, calNum++) { //calendars
        var sqlCommandFile = 'REPLACE INTO FILE VALUES(' + (i+1) + ',\"' + calsOnServer[i]["url"] + '\",' + calsOnServer[i]["version"] + ',\"' + calsOnServer[i]["prodID"] + '\");'; 
        connection.query(sqlCommandFile, function (err, rows, fields){
          if(err){
            console.log(err);
          }
        });

        for(var j = 0; j < calsOnServer[i]["events"].length; j++, eventNum++) { //events
          var 
            dtsql = convertDT(calsOnServer[i]["events"][j]["startDT"]), 
            orgLoc = getLocOrg(calsOnServer[i]["events"][j]["props"]);
          
          var sqlCommandEvent = 'REPLACE INTO EVENT VALUES(' + (eventNum+1) + ',\"' + calsOnServer[i]["events"][j]["summary"] + '\",' + dtsql + ',' + orgLoc.loc + ',' + orgLoc.org + ',' + (i+1) +');';
          
          connection.query(sqlCommandEvent, function (err, row, fields) { 
            if(err){
              console.log(err);
            }
          });
          
          for(var k = 0; k < calsOnServer[i]["events"][j]["alarms"].length; k++, alarmNum++) { //alarms
            var sqlCommandAlarm = 'REPLACE INTO ALARM VALUES(' + (alarmNum+1) + ',' + '\"' + calsOnServer[i]["events"][j]["alarms"][k]["action"] + '\"' + ',' + '\"' + calsOnServer[i]["events"][j]["alarms"][k]["trigger"] + '\"' + ',' + (eventNum+1) + ');';
  
            connection.query(sqlCommandAlarm, function(err, row, fields) {
              if(err){
                console.log(err);
              }
            });            
          }
        }
      }

      connection.query('SELECT file_name FROM FILE;', function(err, row, fields){
        if(err) {
          res.send(err);
        }
        else {
          res.send({
            'code':"OK",
            'val':row
          });
        }
      });
      
      connection.end();
    }
  
  });
});

app.get('/cleardb', function(req, res){
  var connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : req.query.login,
    password : req.query.pass,
    database : req.query.dbname
  });

  connection.connect();

  connection.query('DELETE FROM ALARM;', function(err, row, fields){
    if(err) {
      console.log(err);
    }
    else {
      connection.query('DELETE FROM EVENT;', function(err, row, fields){
        if(err) {
          console.log(err);
        }
        else {
          connection.query('DELETE FROM FILE;', function(err, row, fields){
            if(err) {
              console.log(err);
            }
            else {
              console.log("Success!");
            }
          });
        }
      });
    }
  });

  var queryStr = 'SELECT ' +
                    '(SELECT COUNT(*) FROM FILE) AS num_file, '+
                    '(SELECT COUNT(*) FROM EVENT) AS num_events, ' +
                    '(SELECT COUNT(*) FROM ALARM) AS num_alarms;';

  connection.query(queryStr, function(err, rows, fields){
    if(err){
      res.send(err);
    }
    else{
      res.send({
        'code': "OK",
        'counts':rows[0]
      });
    }
    // connection.end();
  });


});

app.get('/getdatabasedetails', function(req, res){
  
  var connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : req.query.login,
    password : req.query.pass,
    database : req.query.dbname
  });

  connection.connect();
  var queryStr = 'SELECT ' +
                    '(SELECT COUNT(*) FROM FILE) AS num_file, '+
                    '(SELECT COUNT(*) FROM EVENT) AS num_events, ' +
                    '(SELECT COUNT(*) FROM ALARM) AS num_alarms;';

  connection.query(queryStr, function(err, row, fields){
    if(err){
      res.send(err);
    }
    else{
      res.send({
        'code': 'OK',
        'query': row
      });
    }
    connection.end();
  });

});

app.get('/getfilesfromdb', function(req, res){

  var connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : req.query.login,
    password : req.query.pass,
    database : req.query.dbname
  });

  connection.connect();
  var queryStr = 'SELECT file_name FROM FILE;';

  connection.query(queryStr, function(err, row, fields){
    if(err) {
      res.send(err);
    }
    else {
      res.send({
        'code':'OK',
        'val':row
      })
    }
    connection.end();
  });

});

app.get('/getqueryresults', function(req, res){

  var connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : req.query.login.login,
    password : req.query.login.pass,
    database : req.query.login.dbname
  });

  connection.connect();

  connection.query(req.query.query, function(err, row, fields){
    if(err) {
      res.send(err);
    }
    else {
      res.send({
        'code':'OK',
        'results':row
      });
    }
    connection.end();
  });

});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
