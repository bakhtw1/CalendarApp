// Put all onload AJAX calls here, and event listeners

$(document).ready(function() {
    // On page-load AJAX Example

    updateFiles();
    $('[data-toggle="popover"]').popover({
        container: 'body'
    });
    $('[data-toggle="tooltip"]').tooltip();
    
    $('input[type="file"]').change(function(e){
        var fileName = e.target.files[0].name;
        $('#filename-label').html(fileName);
    });
    
    $('#clear-stat-log').click(function(e){
        $('#status-alerts').empty();
    });

    // $("#db-query-forms *").prop('disabled',true);
    // $('#div-database').html('<h1 align="center">Database Panel</h1>' +
    // '<form id="db-login-form" class="database-validation" novalidate>'+
    //     '<label for="db-username">Username</label>'+
    //     '<input type="text" class="form-control" id="db-username" name="uid" placeholder="username" value="" name="db-login" required>'+
    //     '<div class="invalid-feedback">'+
    //         'enter your goddamn username'+
    //     '</div>    '+
        
    //     '<label for="db-password">Password</label>'+
    //     '<input type="password" class="form-control" id="db-password" placeholder="Password" name="dbpass" required>'+
    //     '<div class="invalid-feedback">'+
    //         'enter your fucking password'+
    //     '</div>    '+

    //     '<button id="dblogin-button" type="button" class="btn btn-primary" style="margin-top: 5px;">Submit</button>'+
    // '</form>');

    $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                        '<strong>Success!</strong> Files On Server Uploaded Below!' +
                        '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                        '<span aria-hidden="true">&times;</span>'+
                        '</button>'+
                        '</div>');

        var forms = document.getElementsByClassName('needs-validation');
        var validation = Array.prototype.filter.call(forms, function(form) {
            form.addEventListener('submit', function(event) {
                if (form.checkValidity() === false) {                
                    event.preventDefault();
                    event.stopPropagation();
                }
                else {
                    var formObj = {};
                                
                    let today = new Date(),
                        year = today.getFullYear(), month = (today.getMonth()+1), day = today.getDate(),
                        hours = today.getHours(), minutes = today.getMinutes(), seconds = today.getSeconds();
                    
                        if(month < 10) month = '0' + month;
                        if(day < 10) day = '0' + day;
                        if(hours < 10) hours = '0' + hours;
                        if(minutes < 10) minutes = '0' + minutes;
                        if(seconds < 10) seconds = '0' + seconds;
                    
                        var cdate =  year.toString() + month.toString() + day.toString();
                        var ctime =  hours.toString() + minutes.toString() + seconds.toString();
                                        
                        formObj["cr-date"] = cdate;
                        formObj["cr-time"] = ctime;
                    
                        var inputs = $('#event-form').serializeArray();
                        $.each(inputs, function (i, input) {
                            formObj[input.name] = input.value;
                        });
                    
                        if(!formObj.hasOwnProperty('utc')){
                            formObj['utc'] = false;
                        }
                                    
                        formObj["st-date"] = formObj["st-date"].split('-').join('');
                        formObj["st-time"] = formObj["st-time"].split(':').join('') + '00';
                        formObj["url"] = $('#calViewSelDrpDwn').val();

                        console.log(formObj);
                        $.ajax({
                            type: 'get',
                            dataType: 'json',
                            async: false,
                            data: formObj,
                            url: '/updatecal',
                            success: function(data) {
                                $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                                    '<strong>Success!</strong> ' + data["url"] + ' Updated Succesfully! Status:' + data["status"] +
                                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                    '<span aria-hidden="true">&times;</span>'+
                                    '</button>'+
                                    '</div>');
                            },
                            fail: function (error) {
                                $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                            '<strong>Error!</strong>' + error.toString() + 
                                            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                            '<span aria-hidden="true">&times;</span>'+
                                            '</button>'+
                                            '</div>');
                            }
                        });
                }
                    
                form.classList.add('was-validated');
                    
            }, false);
        });
    
    var calendarstring = "";
    var eventsToCreate = [];

    $('#insert-event-cal').click(function(event){
        var singleEvent = {};
        
        let today = new Date(),
            year = today.getFullYear(), month = (today.getMonth()+1), day = today.getDate(),
            hours = today.getHours(), minutes = today.getMinutes(), seconds = today.getSeconds();
            
        if(month < 10) month = '0' + month;
        if(day < 10) day = '0' + day;
        if(hours < 10) hours = '0' + hours;
        if(minutes < 10) minutes = '0' + minutes;
        if(seconds < 10) seconds = '0' + seconds;

        var cdate =  year.toString() + month.toString() + day.toString();
        var ctime =  hours.toString() + minutes.toString() + seconds.toString();
            

        singleEvent["uid"] = $('#uid-input').val();
        singleEvent["dtstart"] = $('#st-date-input').val().split('-').join('') + 'T' + $('#st-time-input').val().split(':').join('') + '00';
        singleEvent["dtstamp"] = cdate + 'T' + ctime;
        singleEvent["summary"] = $('#summary-form-input').val().replace(/\n/g, " ");

        eventsToCreate.push(singleEvent);

        console.log(JSON.stringify(eventsToCreate));
    });

    $('#calendar-preview').click(function(event){
        calendarstring = 'BEGIN:VCALENDAR\r\n' + 'VERSION:' + $('#version-input').val() + '\r\n' + 'PRODID:' + $('#prodidinput').val() + '\r\n';

        for(var j = 0; j < eventsToCreate.length; j++){
            var eventsstr = 'BEGIN:VEVENT\r\n' +
                            'UID:' + eventsToCreate[j]["uid"] + '\r\n' + 
                            'DTSTART:' + eventsToCreate[j]["dtstart"] + '\r\n' +
                            'DTSTAMP:' + eventsToCreate[j]["dtstamp"] + '\r\n' +
                            'SUMMARY:' + eventsToCreate[j]["summary"] + '\r\n' +
                            'END:VEVENT\r\n';
            calendarstring += eventsstr;
        }
        calendarstring += 'END:VCALENDAR\r\n';
        console.log(JSON.stringify(calendarstring));
        $('#calendar-preview-area').val(calendarstring);

    });

    $('#submit-calendar').click(function(event){
        calendarstring = 'BEGIN:VCALENDAR\r\n' + 'VERSION:' + $('#version-input').val() + '\r\n' + 'PRODID:' + $('#prodidinput').val() + '\r\n';

        for(var j = 0; j < eventsToCreate.length; j++){
            var eventsstr = 'BEGIN:VEVENT\r\n' +
                            'UID:' + eventsToCreate[j]["uid"] + '\r\n' + 
                            'DTSTART:' + eventsToCreate[j]["dtstart"] + '\r\n' +
                            'DTSTAMP:' + eventsToCreate[j]["dtstamp"] + '\r\n' +
                            'SUMMARY:' + eventsToCreate[j]["summary"] + '\r\n' +
                            'END:VEVENT\r\n';
            calendarstring += eventsstr;
        }
        calendarstring += 'END:VCALENDAR\r\n';
        var fname = $('#filenameinput').val() + '.ics';
        console.log('Hello: ' + calendarstring);
        $('#create-calendar').modal('hide');
        
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: {"calstr":calendarstring,"url":fname},
            url: '/writecal',
            success: function(data){
                updateFiles();

                if(data["status"] === 'OK'){
                    $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                                    '<strong>Success!</strong> ' + fname + ' Created Succesfully! Status:' + data["status"] +
                                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                    '<span aria-hidden="true">&times;</span>'+
                                    '</button>'+
                                    '</div>');
                    
                }
                else{
                    $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                    '<strong>Error!</strong> Create Calender Unsuccessful!: ' + fname + ' - Error Code: '+ data["status"] + 
                                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                    '<span aria-hidden="true">&times;</span>'+
                                    '</button>' +
                                    '</div>');
                    
                }

                
            },
            fail: function(error){
                $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                            '<strong>Error!</strong>' + error.toString() + 
                                            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                            '<span aria-hidden="true">&times;</span>'+
                                            '</button>'+
                                            '</div>');
            }
        });

        $('#create-cal-form')[0].reset();
        calendarstring = "";
        eventsToCreate = [];

        return;

    });

    $('#close-cal-create').click(function(event){
        eventsToCreate = [];
        calenderstring = "";
    });

    $('#clear-cal-form').click(function(event){
        eventsToCreate = [];
        calenderstring = "";
    });

    var dbLoginInfo = {}
    var db_forms = document.getElementById('db-login-form');
    
    $('#dblogin-button').on('click',function(event){
        if(db_forms.checkValidity() === false) {
            event.preventDefault();
            event.stopPropagation();
        }
        else {
            dbLoginInfo.login = $('#db-username').val();
            dbLoginInfo.pass =  $('#db-password').val();
            dbLoginInfo.dbname = $('#db-name').val();

            $.ajax({
                type: 'get',
                dataType: 'json',
                data: dbLoginInfo,
                url: '/connectdb',
                success: function(data) {
                    console.log(data);
                    if(data.code == 'sup') {
                        $('#div-database').html('<h1 align="center">Authentication Success!</h1>');
                        $('#database-operations').css("display", "block");
                        getFilesFromDatabase(dbLoginInfo);
                    }
                    else {
                        alert("ERROR: " + data.code + "\n" + data.sqlMessage);
                    }
                },
                fail: function(error) {
                    console.log(error);
                }
            });
        }
        db_forms.classList.add('was-validated');
    }); 
    
    $('#db-upload-files').click(function (event){
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: dbLoginInfo,
            url: '/CalsToDb',
            success: function (data) {
                console.log(data);
                if(data.code === 'no_files'){
                    $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                    '<strong>Error!</strong>' + 'DataBase Upload Failes: No Files!' + 
                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                    '<span aria-hidden="true">&times;</span>'+
                    '</button>'+
                    '</div>');
                    $("#db-query-forms *").prop('disabled',true);
                }
                else if (data.code === 'OK') {
                    $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                    '<strong>Success!</strong> Files On Server Uploaded To DataBase!' +
                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                    '<span aria-hidden="true">&times;</span>'+
                    '</button>'+
                    '</div>');
                    $("#db-query-forms *").prop('disabled',false);
                    
                    // getFilesFromDatabase(dbLoginInfo);
                    $('#db-query-file').empty();
                    $('#db-query-file').append('<option value="0">All Files</option>')
                    
                    for(var i = 0; i < data["val"].length; i++){
                        $('#db-query-file').append('<option value="'+ data["val"][i]["file_name"] +'">' + data["val"][i]["file_name"] + '</option>')
                    }
                }
            },
            fail: function(error){
                console.log(error);
            }
        });
    });

    $('#db-clear').on('click', function(event){
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: dbLoginInfo,
            url: '/cleardb',
            success: function (data) {
                console.log(data);
                $("#db-query-forms *").prop('disabled',true);
                if(data["code"] === 'OK'){
                    $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                    '<strong>Success!</strong> ' + 'Database has ' + data["counts"]["num_file"] + ' files, ' + data["counts"]["num_events"] + ' events, and ' + data["counts"]["num_alarms"] + ' alarms' +
                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                    '<span aria-hidden="true">&times;</span>'+
                    '</button>'+
                    '</div>');
                }
            },
            fail: function(error){
                console.log(error);
            }
        });
    });

    $('#db-details').on('click', function(event){
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: dbLoginInfo,
            url: '/getdatabasedetails',
            success: function(data) {
                
                if(data["code"] === 'OK'){
                    $('#status-alerts').append('<div class="alert alert-success alert-dismissible fade show" role="alert">' +
                    '<strong>Success!</strong> ' + 'Database has ' + data["query"][0]["num_file"] + ' files, ' + data["query"][0]["num_events"] + ' events, and ' + data["query"][0]["num_alarms"] + ' alarms' +
                    '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                    '<span aria-hidden="true">&times;</span>'+
                    '</button>'+
                    '</div>');
                }
            },
            fail: function(error) {
                console.log(error);
            }
        });
    }); 

    var query_forms = document.getElementById('db-execute-querystr');
    $('#db-execute').on('click', function(event){
        if(query_forms.checkValidity() === false) {
            event.preventDefault();
            event.stopPropagation();
        }
        else {
            var 
                file = $('#db-query-file').val(),
                queryNum = $('#db-query-par').val();    
            var
                optionalVal = "",
                string = "";
            
            if(file == 0) {
                string = 'SELECT * FROM EVENT ';
            }
            else{
                string = 'SELECT * FROM EVENT WHERE EVENT.cal_file=(SELECT FILE.cal_id FROM FILE WHERE FILE.file_name="' + file + '") ';
            }

            if (queryNum == 1){
                optionalVal = 'ORDER BY EVENT.start_time';
            }
            else if (queryNum == 2){
                if(file == 0) {
                    optionalVal = 'WHERE CAST(start_time AS DATE) IN (SELECT CAST(start_time AS DATE) FROM EVENT GROUP BY CAST(start_time AS DATE) HAVING COUNT(*) > 1)';
                }
                else {
                    optionalVal = 'AND CAST(start_time AS DATE) IN (SELECT CAST(start_time AS DATE) FROM EVENT GROUP BY CAST(start_time AS DATE) HAVING COUNT(*) > 1)';
                }
            }
            else if(queryNum == 3) {
                if(file == 0) {
                    optionalVal = 'WHERE CAST(EVENT.start_time AS DATE)="' + $('#db-query-date').val() + '"';
                }
                else {
                    optionalVal = 'AND CAST(EVENT.start_time AS DATE)="' + $('#db-query-date').val() + '"';
                }
            }
            else if(queryNum == 4) {                
                if(file == 0) {
                    string = 'SELECT * FROM EVENT,ALARM ';
                    optionalVal = 'WHERE EVENT.event_id=ALARM.event';
                }
                else {
                    string = 'SELECT * FROM EVENT,ALARM WHERE EVENT.cal_file=(SELECT FILE.cal_id FROM FILE WHERE FILE.file_name="' + file + '") ';
                    optionalVal = 'AND EVENT.event_id=ALARM.event';
                }
            }
            else if(queryNum == 5) {
                if(file == 0) {
                    optionalVal = 'WHERE EVENT.location="' + $('#db-loc').val() + '"';
                }
                else {
                    optionalVal = 'AND EVENT.location="' + $('#db-loc').val() + '"';
                }
            }
            else if(queryNum == 6) {
                if(file == 0) {
                    optionalVal = 'WHERE EVENT.organizer="' + $('#db-org').val() + '"'; 
                }
                else {
                    optionalVal = 'AND EVENT.organizer="' + $('#db-org').val() + '"'; 
                }
            }

            $.ajax({
                type: 'get',
                dataType: 'json',
                data: {
                    'login': dbLoginInfo,
                    'query': string + optionalVal
                },
                url: '/getqueryresults',
                success: function(data){
                    // console.log(data);
                    if( data["code"] === 'OK' ) {

                        var keys = Object.keys(data["results"][0]);
                        var header = "";
                        var body = "";

                        for(var j = 0; j < keys.length; j++) {
                            header += '<th scope="col">'+keys[j]+'</th>'
                        }
                        
                        for(var i = 0; i < data["results"].length; i++) {
                            body += "<tr>";
                            for(var k = 0; k < keys.length; k++){
                                var entry = "";
                                if(data["results"][i][keys[k]] != null){
                                    entry = data["results"][i][keys[k]];
                                }
                                if(k === 0){
                                    body += "<th scope=\"row\">"+entry+"</th>";
                                }
                                else {
                                    body += "<td>"+entry+"</td>";
                                }
                                
                            }
                            body += "</tr>";
                        }
                        
                        $('#query-results-header').html(header);                        
                        $('#query-results-body').html(body);
                        $('#query-results-modal').modal('show');
                        
                    }
                },
                fail: function(error){
                    console.log(error);
                }
            }); 
        }
        query_forms.classList.add('was-validated');
    });

    $('#db-query-par').change(function(e) {
        
        if($('#db-query-par').val() == 3) {
            $('#db-querystr').html('<label for="db-query-date">Date: </label><input type="date" class="form-control" id="db-query-date" placeholder="Database Name" name="dbdate" required>');
        }
        else if($('#db-query-par').val() == 5) {
            $('#db-querystr').html('<label for="db-loc">Location: </label><input type="text" class="form-control" id="db-loc" placeholder="..." name="dbloc" required>');
        }
        else if($('#db-query-par').val() == 6) {
            $('#db-querystr').html('<label for="db-org">Organizer: </label><input type="text" class="form-control" id="db-org" placeholder="..." name="dborg" required>');
        }
        else {
            $('#db-querystr').empty();
        }

    });

});

$('#submit-file').click(function(e) {
    console.log('Hello');
});

$('#calViewSelDrpDwn').change(function (e) { 
    
    var filename = $('#calViewSelDrpDwn').val();
    console.log(filename);

    if(filename === 'empty'){
        console.log("Empty");
        return;
    } 

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/parsecal?var1=19',
        data: 'fname=' + filename,
        success: function(data) {

            var version = data["version"];
            var prodid = data["prodID"];
            var eventNum = data["numEvents"];
            var propNum = data["numProps"];
            var events = data["events"];

            $('#event-details-list').empty();
            $('#insert-event-button').empty();
            
            $('#versionLabel').html("Version: " + version);
            $('#prodidLabel').html("Product ID: " + prodid);
            $('#eventsNumLabel').html("No. of Events: " + eventNum);
            $('#propsNumLabel').html("No. of Properties: " + propNum);
            $('#calEventsTable').find('tbody tr').remove();

            for(var i = 0; i < events.length; i++){

                var
                    day = events[i]["startDT"]["date"].substring(0,4), 
                    month = events[i]["startDT"]["date"].substring(4,6),
                    year = events[i]["startDT"]["date"].substring(6,8),
                    date = day +'/'+ month +'/'+ year;
                var 
                    hours = events[i]["startDT"]["time"].substring(0,2),
                    minutes = events[i]["startDT"]["time"].substring(2,4),
                    seconds = events[i]["startDT"]["time"].substring(4,6),
                    time = hours+':'+minutes+':'+seconds;
                    
                if(events[i]["startDT"]["isUTC"] === true){
                    time = time + ' (UTC)';
                }

                var props = "", alarms = "";

                
                for(var n = 0; n < events[i]["props"].length; n++)
                {
                    props += events[i]["props"][n] + '<br>';
                }
                // console.log(props);
                
                for(var n = 0; n < events[i]["alarms"].length; n++){
                    alarms += '<br>Alarm '+n+'<br>'+'TRIGGER:' + events[i]["alarms"][n]["trigger"] + '<br>' +
                            'ACTION:' + events[i]["alarms"][n]["action"] + '<br>' + 
                            'Properties:<br>';
                    for(var m = 0; m < events[i]["alarms"][n]["props"].length; m++){
                        alarms += events[i]["alarms"][n]["props"][m] + '<br>';
                    }
                }
                
                $('#event-details-list').append('<div class="alert scrollable-area alert-success alert-dismissible fade show" role="alert">' +
                '<h2>Event: ' + (i+1) + '</h2><br>' +'<h4>Properties: <br></h4>' + props + '<br>' + '<h4>Alarms:</h4> <br>' + alarms + '<br>' +
                '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                '<span aria-hidden="true">&times;</span>'+
                '</button>'+
                '</div>');

                $('#calEventsTable')
                    .find('tbody')
                        .append('<tr><th scope="row">'+(i+1)+'</th>' +
                                '<td>'+ date +'</td>' +
                                '<td>'+ time +'</td>' +
                                '<td>'+events[i]["summary"]+'</td>' +
                                '<td><button id="event'+i+'prop" type="button" class="btn btn-info papap" data-toggle="popover" title="elele" data-content="helpme">'+events[i]["numProps"]+'</button></td>' +
                                '<td><button id="event'+i+'alarm" type="button" class="btn btn-info papap" data-toggle="popover" title="lololo" data-content="helpme">'+events[i]["numAlarms"]+'</button></td>/tr>');
            
            }

            $('#events-summary-title').html('Events Summary: ' + filename);
            $("#insert-event-button").append('<button type="button" class="btn btn-block btn-primary" style="margin-top: 5px;" data-toggle="modal" data-target="#event-details">Event Summary</button>')

            $('#insert-event-button').append('<button id="add-event-form" type="button" class="btn btn-block btn-secondary" data-toggle="modal" data-target="#exampleModal">Add Event</button>');

        },
        fail: function(error){
            $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                            '<strong>Error!</strong>' + error.toString() + 
                                            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                            '<span aria-hidden="true">&times;</span>'+
                                            '</button>'+
                                            '</div>');
        }
    });

});

function updateFiles() {
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            $('#file-log').find('tbody').empty();
            $('#calViewSelDrpDwn').empty();
            $('#calViewSelDrpDwn').append('<option value="empty">Select Calendar</option>');
            // $('#database-operations').empty();

            if(data.length === 0){ 
                $('#file-log').find('tbody').append( '<tr><th scope="row"><td colspan="5">No Legumes Here</td></tr>' );
            }
            else {
                for(var i = 0; i < data.length; i++){

                    if(data[i]["errCode"] === 'OK'){
                        var link = '<a href="/uploads/'+ data[i]["url"] +'">'+ data[i]["url"] +'</a>';
                        //<option value="4">text3</option>
                        var btnLink = '<option value="'+ data[i]["url"] +'">'+ data[i]["url"] +'</option>';
                        $('#file-log').find('tbody').append( '<tr><th scope="row">' + link + '</th><td>'+ data[i]["version"]+'</td><td>'+data[i]["prodID"]+'</td><td>'+data[i]["numEvents"]+'</td><td>'+data[i]["numProps"]+'</td></tr>' );
                        $('#calViewSelDrpDwn').append(btnLink);
                    }
                    else{
                        
                        $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                            '<strong>Error!</strong> Invalid File: ' + data[i]["url"] + ' - Error Code: '+ data[i]["errCode"] + 
                                            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                            '<span aria-hidden="true">&times;</span>'+
                                            '</button>'+
                                            '</div>');
                    }

                }
                // $('#database-operations').append('<button class="btn btn-primary" id="db-upload-files">Upload Files To DataBase</button>');
            }
            
            //We write the object to the console to show that the request was successful
            
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#status-alerts').append('<div class="alert alert-danger alert-dismissible fade show" role="alert">' +
                                            '<strong>Error!</strong>' + error.toString() + 
                                            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                                            '<span aria-hidden="true">&times;</span>'+
                                            '</button>'+
                                            '</div>');
         
        }
    });
}

function getFilesFromDatabase(loginDetails){
    $.ajax({
        type: 'get',
        dataType: 'json',
        data: loginDetails,
        url: '/getfilesfromdb',
        success: function(data){
            $('#db-query-file').empty();
            $('#db-query-file').append('<option value="0">All Files</option>')
            for(var i = 0; i < data["val"].length; i++){
                $('#db-query-file').append('<option value="'+ data["val"][i]["file_name"] +'">' + data["val"][i]["file_name"] + '</option>')
            }
        },
        fail: function(error){
            console.log(error);
        }
    });
}