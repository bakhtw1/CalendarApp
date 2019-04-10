
/**
 * Author: Waqas Bakht
 * id: 0981571
 * email: wbakht@uoguelph.ca
 * course: CIS2750 
 * Prof: Denis Nikitenko
*/

#define _GNU_SOURCE
#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include "Helper.h"

/** Function to parse through the file, recursively returning the type of struct to return
* @pre file must be opened and the first thing in the file must be the begin vcalendar
* @post calendar object has been created and returned
* @return either a alarm, event or calendar
**/
void* parseCalendar(FILE **fp, char *type, ICalErrorCode *code){
    //line reading tools
    char* buffer = NULL;
    char* propname = NULL;
    char* propdesc = NULL;
    char* delim = ":;";

    //counters for the calendar
    int versionCount = 0;
    int prodidCount = 0;
    
    //counters for the event
    int dtstampcount = 0;
    int dtstartcount = 0;
    int uidcount = 0;

    //counter for the alarm
    int triggerCount = 0;
    int actionCount = 0;

    //bool vals to check
    bool endOfCalender = false;

    //set file pointers and initialize all types of structs we may return.
    FILE *file = *fp;

    Calendar *newCalendar = NULL;
    Event *newEvent = NULL;
    Alarm *newAlarm = NULL;


    if(file == NULL){
        *code = INV_FILE;
        return NULL;
    }

    // branching statements for each recursive type. 
    if( strcmp(type, cale) == 0 ){
        newCalendar = (Calendar*)malloc(sizeof(Calendar));
        newCalendar->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
        newCalendar->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    }
    else if( strcmp(type, vala) == 0 ){
        newAlarm = (Alarm*)malloc(sizeof(Alarm));
        newAlarm->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    }
    else if( strcmp(type, even) == 0 ){
        //need to create alarm helpers
        newEvent = (Event*)malloc(sizeof(Event));
        newEvent->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
        newEvent->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    }
    else{ //this will likely never occur
        *code = INV_CAL;
        return NULL;
    }

    while(!feof(file)){
        //buffer = (char*)malloc(80 * sizeof(char));
        //fgets(buffer, 80 * sizeof(char), file)
        
        size_t len = 0;
        size_t linecapp = 0;
        char* line = NULL;
        len = getline(&line, &linecapp, file);
        //printf("size_t %d:%d ", len, strlen(line)+1);
        //printf("line: %s\n", line);

        if(len > 0) { 

            buffer = (char*)malloc( (strlen(line) + 1) * sizeof(char));
            strcpy(buffer, line);
            free(line);

            if(buffer[strlen(buffer) - 2] != '\r' || buffer[strlen(buffer) - 1] != '\n'){
                free(buffer);
                *code = INV_FILE;
                if( strcmp(type, cale) == 0 ){ //if we are inside a calendar
                    deleteCalendar(newCalendar);
                    return NULL;
                }
                else if( strcmp(type, even) == 0){ //if we are inside an event
                    deleteEvent(newEvent);
                    return NULL;
                } 
                else if( strcmp(type, vala) == 0){ //if we are inside an alarm
                    deleteAlarm(newAlarm);
                    return NULL;
                }
            }
            
            char nextChar = '\0';
            int numLines = 1;
            while( buffer[strlen(buffer) - 2] == '\r' && buffer[strlen(buffer) - 1] == '\n' && (nextChar = fgetc(file)) != EOF && (nextChar == ' ' || nextChar == '\t') ){
                numLines++;
                //char* tmp = (char*)malloc(len * sizeof(char));
                // char* tmp = NULL;
                // char* tmp2 = NULL;
                //tmp = fgets(tmp, len*sizeof(char), file);
                size_t len2 = 0, lincapp2 = 0;
                len2 = getline(&line, &lincapp2, file);
                //tmp = (char*)malloc( (len2+1) * sizeof(char));
                //strcpy(tmp, line);
                //printf("tmp: %s\n", tmp2);
                buffer = realloc(buffer, (strlen(buffer) + len2 + 1) * sizeof(char));
                //printf("lalal %d, tmp:%s\n", len2 + strlen(buffer) + 1, tmp);
                strtok(buffer, "\r\n");
                strcat(buffer, line);
                //printf("buffer: %s\n", buffer);
                free(line);
            }
            ungetc(nextChar, file);

            if(buffer[0] == ';'){ //skip any comments
                free(buffer);
                continue;
            }

            // printf("%s", buffer);

            propname = strtok(buffer, delim);
            propdesc = strtok(NULL, "\r\n");

            if( strcmp(propname, "BEGIN") == 0 ) { //A BEGIN tag means a new structure...
                if( strcmp(propdesc, even) == 0 ) { //still in VCALENDAR but going inside a VEVENT
                    newEvent = parseCalendar(&file, even, code);
                    
                    if(newEvent == NULL) { //if the event returns null then delete the event and also return null
                        deleteCalendar(newCalendar);
                        free(buffer);
                        return NULL;
                    }
                    else { //otherwise add the event and move along
                        insertBack(newCalendar->events, newEvent);
                    }
                }
                else if( strcmp(propdesc, vala) == 0 ) { //still in VEVENT but going inside a VALARM
                    newAlarm = parseCalendar(&file, vala, code);
                    
                    if(newAlarm == NULL){ //if the alarm returns null then delete the event and also return null
                        deleteEvent(newEvent);
                        free(buffer);
                        return NULL;
                    }
                    else{ //otherwise add to the list and move along
                        insertBack(newEvent->alarms, newAlarm);
                    }
                }
                else { // if the BEGIN tag has a type which is not accounted for, set the error code and return NULL
                    free(buffer);
                    if( strcmp(type, cale) == 0 ){ //if we are inside a calendar
                        *code = INV_CAL;
                        deleteCalendar(newCalendar);
                        return NULL;
                    }
                    else if( strcmp(type, even) == 0){ //if we are inside an event
                        *code = INV_EVENT;
                        deleteEvent(newEvent);
                        return NULL;
                    } 
                    else if( strcmp(type, vala) == 0){ //if we are inside an alarm
                        *code = INV_ALARM;
                        deleteAlarm(newAlarm);
                        return NULL;
                    }
                }
            }
            else if( strcmp(propname, "END") == 0 && strcmp(propdesc, type) == 0 ) { //reached the end of a structure, and ensuring that the END type matches the BEGIN type
                if( strcmp(propdesc, even) == 0 ) { //reached the end of an event
                    free(buffer);
                    if(newEvent && uidcount == 1 && dtstampcount == 1 && dtstartcount == 1) {
                        return newEvent; 
                    }   
                    else { //this case will likely never happen
                        *code = INV_EVENT;
                        return NULL;
                    }
                }
                else if( strcmp(propdesc, vala) == 0 ) { //reached the end of an alarm
                    free(buffer);
                    if(newAlarm && triggerCount == 1 && actionCount == 1) {
                        return newAlarm;
                    }
                    else{ //this branch will likely never occur
                        *code = INV_ALARM;
                        return NULL;
                    }
                }
                else if( strcmp(propdesc, cale) == 0 ) { //if the end of the calendar has been reached, complete the parsing and return from function
                    if( strcmp(type, even) == 0 ){ //end of file reached but still in an alarm
                        *code = INV_EVENT;
                        free(buffer);
                        deleteEvent(newEvent);
                        return NULL;
                    }
                    else if( strcmp(type, vala) == 0 ){ //end of file reached but still in an event
                        *code = INV_ALARM;
                        deleteAlarm(newAlarm);
                        free(buffer);
                        return NULL;
                    }
                    else if( prodidCount == 0 || versionCount == 0 || newCalendar->events->length == 0 ) {
                        *code = INV_CAL;
                        free(buffer);
                        deleteCalendar(newCalendar);
                        return NULL;
                    }
                    endOfCalender = true;
                }
                else { // this will likely never happen
                    printf("Error: 95");
                }
            }
            else if( strcmp(propname, "END") == 0 && strcmp(propdesc, type) != 0 ){ //if an end tag does not match a the begin tag, what the fuck
                if( strcmp(type, cale) == 0 ){ //end tag mismatch while inside a calendar
                    deleteCalendar(newCalendar);
                    *code = INV_CAL;
                    return NULL;
                }
                else if( strcmp(type, even) == 0 ){ //end tag mismatch while inside an event
                    deleteEvent(newEvent);
                    *code = INV_EVENT;
                    return NULL;
                }
                else if( strcmp(type, vala) == 0 ){ //end tag mismatch while inside an alarm
                    deleteAlarm(newAlarm);
                    *code = INV_ALARM;
                    return NULL;
                }
            }
            else {
                if( strcmp(type, cale) == 0  && newCalendar != NULL ) { //property gathering inside a calendar

                    if( strcmp(propname, "PRODID") == 0 && prodidCount == 0){ //setting PRODID                        
                        if( propdesc == NULL || strcmp(propdesc, "") == 0){ //invalid PRODID value
                            *code = INV_PRODID;
                            free(buffer);
                            deleteCalendar(newCalendar);
                            return NULL;
                        }
                        else{
                            strcpy(newCalendar->prodID, propdesc);
                            prodidCount = 1;
                        }
                    }
                    else if( strcmp(propname, "VERSION") == 0 && versionCount == 0){ //setting version
                        if(propdesc == NULL || strcmp(propdesc, "") == 0){ //invalid VERSION value
                            *code = INV_VER;
                            free(buffer);
                            deleteCalendar(newCalendar);
                            return NULL;
                        }
                        else{
                            newCalendar->version = atof(propdesc);
                            versionCount = 1;
                        }
                    }
                    else if( strcmp(propname, "VERSION") == 0 && versionCount == 1){ //duplicate version 
                        *code = DUP_VER;
                        free(buffer);
                        deleteCalendar(newCalendar);
                        return NULL;
                    }
                    else if( strcmp(propname, "PRODID") == 0 && prodidCount == 1){ //duplicate prodids
                        *code = DUP_PRODID;
                        free(buffer);
                        deleteCalendar(newCalendar);
                        return NULL;
                    }
                    else if( propdesc != NULL && propname != NULL ){ //adding other calendar properties
                        int len = strlen(propdesc) + 1;
                        Property *newProperty = (Property*)malloc(sizeof(Property) + len);
                        strcpy(newProperty->propName, propname);
                        strcpy(newProperty->propDescr, propdesc);
                        insertBack(newCalendar->properties, newProperty);
                    }
                
                }
                else if( strcmp(type, even) == 0 && newEvent != NULL ) { //property gathering inside an event
                    
                    if( propdesc == NULL || strcmp(propdesc, "") == 0){
                        *code = INV_EVENT;
                        deleteEvent(newEvent);
                        free(buffer);
                        return NULL;
                    }

                    int len = strlen(propdesc) + 1;

                    if( strcmp(propname, "DTSTAMP") == 0 && dtstampcount == 0 ){ //getting the creation date
                        if( dateMatchesForm(propdesc) == false ){ //date-time format check
                            *code = INV_DT;
                            deleteEvent(newEvent);
                            free(buffer);
                            return NULL;
                        }
                        
                        char* ddate;
                        char* dtime;
                        
                        if(propdesc[strlen(propdesc)-1] == 'Z'){
                            newEvent->creationDateTime.UTC = true;
                        }
                        else{
                            newEvent->creationDateTime.UTC = false;
                        }

                        ddate = strtok(propdesc, "T");
                        dtime = strtok(NULL, "Z");
            
                        strcpy(newEvent->creationDateTime.date, ddate);
                        strcpy(newEvent->creationDateTime.time, dtime);
                        dtstampcount = 1;
                    }
                    else if( strcmp(propname, "DTSTART") == 0 && dtstartcount == 0 ){ //getting the start date
                        if( dateMatchesForm(propdesc) == false ){ //date-time format check
                            *code = INV_DT;
                            deleteEvent(newEvent);
                            free(buffer);
                            return NULL;
                        }

                        char* ddate;
                        char* dtime;
                        
                        if(propdesc[strlen(propdesc)-1] == 'Z'){
                            newEvent->startDateTime.UTC = true;
                        }
                        else{
                            newEvent->startDateTime.UTC = false;
                        }

                        ddate = strtok(propdesc, "T");
                        dtime = strtok(NULL, "Z");
                        
                        strcpy(newEvent->startDateTime.date, ddate);
                        strcpy(newEvent->startDateTime.time, dtime);
                        dtstartcount = 1;
                    }
                    else if( strcmp(propname, "UID") == 0 && uidcount == 0 ) { //getting the uid
                        strcpy(newEvent->UID, propdesc);
                        uidcount = 1;
                    }
                    else if( (strcmp(propname, "UID") == 0 && uidcount == 1) || (strcmp(propname, "DTSTART") == 0 && dtstartcount == 1) || (strcmp(propname, "DTSTAMP") == 0 && dtstampcount == 1) ) {
                        *code = INV_EVENT;
                        deleteEvent(newEvent);
                        free(buffer);
                        return NULL;
                    } 
                    else {
                        Property *newProperty = (Property*)malloc(sizeof(Property) + len);
                        strcpy(newProperty->propName, propname);
                        strcpy(newProperty->propDescr, propdesc);
                        insertBack(newEvent->properties, newProperty);
                    }
                    
                }
                else if( strcmp(type, vala) == 0 && newAlarm != NULL ){ //inside an alarm

                    if( propdesc == NULL || strcmp(propdesc, "") == 0){
                        *code = INV_ALARM;
                        deleteEvent(newAlarm);
                        free(buffer);
                        return NULL;
                    }

                    if( strcmp(propname, "TRIGGER") == 0 && triggerCount == 0 ){
                        int len = strlen(propdesc) + 1;
                        newAlarm->trigger = (char*)malloc(sizeof(char)*len);
                        strcpy(newAlarm->trigger, propdesc);
                        triggerCount = 1;
                    }
                    else if( strcmp(propname, "ACTION") == 0 && actionCount == 0 ){
                        strcpy(newAlarm->action, propdesc);
                        actionCount = 1;
                    }
                    else if( (strcmp(propname, "ACTION") == 0 || strcmp(propname, "TRIGGER") == 0) && (actionCount == 1 || triggerCount ==1) ){
                        *code = INV_ALARM;
                        deleteEvent(newAlarm);
                        free(buffer);
                        return NULL;
                    }
                    else{
                        int len = strlen(propdesc) + 1;
                        Property *newProperty = (Property*)malloc(sizeof(Property) + len);
                        strcpy(newProperty->propName, propname);
                        strcpy(newProperty->propDescr, propdesc);
                        insertBack(newAlarm->properties, newProperty);
                    }

                }
                else {
                    if( strcmp(type, cale) == 0 ){ //if we are inside a calendar
                        *code = INV_CAL;
                        deleteCalendar(newCalendar);
                        free(buffer);
                        return NULL;
                    }
                    else if( strcmp(type, even) == 0){ //if we are inside an event
                        *code = INV_EVENT;
                        deleteEvent(newEvent);
                        free(buffer);
                        return NULL;
                    } 
                    else if( strcmp(type, vala) == 0){ //if we are inside an alarm
                        *code = INV_ALARM;
                        deleteAlarm(newAlarm);
                        free(buffer);
                        return NULL;
                    }
                }
                
            }
            free(buffer);
        }
        
        //was here
    }

    //last check if calnder is good
    if(endOfCalender == false){
        *code = INV_CAL;
        deleteCalendar(newCalendar);
        return NULL;
    }

    //at this stage we need to return a calendar object
    return newCalendar;

}

/** Function to verify that date time matches the form specified
 * @pre needs the date-time string passed to it
 * @post nothing is changed
 * @return boolean value for if the string conforms to the spec
**/
bool dateMatchesForm(char* date){
    if(date == NULL || strcmp(date, "") == 0){
        return false;
    }
    else if(strlen(date) != 16 && strlen(date) != 15){
        return false;
    }
    else if(date[8] != 'T'){
        return false;
    }
    else if(strlen(date) == 16 && date[strlen(date) - 1] != 'Z'){
        return false;
    }

    for(int i = 0; i < strlen(date) - 1; i++){
        if(i < 8 && (date[i] < '0' || date[i] > '9')){
            return false;
        }
        if((i > 8 && i < (strlen(date) - 1)) && (date[i] < '0' || date[i] > '9')){
            return false;
        }
    }

    return true; 
}

/** Function to verify that the filename has the correct extention
 * @pre string representing path to the calender file
 * @param:
 *  filename - a string to represent a path to calendar file
 * @return: false if filename is not in correct format, true otherwise
*/
bool isCalFile(char *filename){

    int len = 0;

    if(filename){
        len = strlen(filename);
        if(len > 4){
            if(filename[len-1] == 's' && filename[len-2] == 'c' && filename[len-3] == 'i' && filename[len-4] == '.'){
                return true;
            }
        }
    }

    return false;

}

bool chkCalProps(List* list){

    int calScaleCount = 0;
    int methodCount = 0;
    ListIterator propIter;

    if(list == NULL){
        return false;
    }

    if(list->length > 0){
        propIter = createIterator(list);
        for(int i = 0; i < list->length; i++){
            Property* tmp = (Property*)nextElement(&propIter);
            if(tmp){
                if( isValidProperty(tmp) == false ){
                    return false;
                }
                else if( strcmp(tmp->propName, "METHOD") == 0 ){
                    methodCount++;
                }
                else if( strcmp(tmp->propName, "CALSCALE") == 0 ){
                    calScaleCount++;
                }
                else{
                    return false;
                }
            } 
        }
        if(calScaleCount > 1 || methodCount > 1){
            return false;
        }
    }
    
    return true;
}

ICalErrorCode chkCalEvents(List* list){

    ICalErrorCode code;
    ListIterator eventIter;

    if(list && list->length > 0){
        eventIter = createIterator(list);
        for(int i = 0; i < list->length; i++){
            Event* tmp = (Event*)nextElement(&eventIter);
            code = isValidEvent(tmp);
            if(code != OK){
                return code;
            }
        }
    }
    else{
        return INV_CAL;
    }

    return OK;
}

ICalErrorCode isValidEvent(const Event* event){
    
    if(event->UID == NULL || event->UID[0] == '\0'){
        // printf("1: ");
        return INV_EVENT;
    }
    else if(chkDT(event->creationDateTime) == false || chkDT(event->startDateTime) == false){
        // printf("2: ");
        return INV_EVENT;
    }
    else if(event->properties == NULL || event->alarms == NULL){
        // printf("3: ");
        return INV_EVENT;
    }
    else if(chkEvtProps(event->properties) == false){
        // printf("4: ");
        return INV_EVENT;
    }
    else if(chkAlarms(event->alarms) == false){
        // printf("5: ");
        return INV_ALARM;
    }


    return OK;

}

bool chkDT(DateTime dt){
    
    if(dt.date == NULL || dt.time == NULL){
        return false;
    }
    else if(dt.date[0] == '\0' || dt.time[0] == '\0'){
        return false;
    }
    else if( strlen(dt.date) != 8 || strlen(dt.time) != 6 ){
        return false;
    }

    return true;
}

bool chkEvtProps(List* list){

    ListIterator propsIterator;
    Property* tmp;

   char* optionalOnce[17] = {
        "CLASS","CREATED","DESCRIPTION","GEO","LAST-MODIFIED",
        "LOCATION","ORGANIZER","PRIORITY","SEQUENCE","STATUS",
        "SUMMARY","TRANSP","URL","RECURRENCE-ID","RRULE","DTEND","DURATION"
    };

    propsIterator = createIterator(list);

    for(int i = 0; i < list->length; i++){
        tmp = nextElement(&propsIterator);
        // printf("Prop %d: %s\n", i, tmp->propName);
        if(isValidEventProp(tmp) == false){
            return false;
        }
    }

    for(int j = 0; j < 17; j++){
        if(numOccur(list, optionalOnce[j]) > 1){
            return false;
        }
    }


    if(numOccur(list, "DTEND") == 1 && numOccur(list, "DURATION") == 1){
        // printf("580\n");
        return false;
    }

    return true;

}

bool isValidProperty(const Property* prop){
    if(prop == NULL){
        return false;
    }
    else{
        if(prop->propName == NULL || prop->propDescr == NULL){
            return false;
        }
        else if(prop->propName[0] == '\0' || prop->propDescr[0] == '\0'){
            return false;
        }
    }
    return true;
}

int numOccur(List* props, char* string){

    int numOfOccur = 0;
    Property* tmp;
    ListIterator propsIterator;

    propsIterator = createIterator(props);

    for(int i = 0; i < props->length; i++){
        tmp = (Property*)nextElement(&propsIterator);
        if(strcmp(string, tmp->propName) == 0){
            numOfOccur++;
        }
    }
    
    return numOfOccur;

}

bool isValidEventProp(Property* prop){
    const char* validEventProps[26] = {
        "CLASS","CREATED","DESCRIPTION","GEO","LAST-MODIFIED","LOCATION",
        "ORGANIZER","PRIORITY","SEQUENCE","STATUS","SUMMARY","TRANSP","URL",
        "RECURRENCE-ID","RRULE","DTEND","DURATION","ATTACH","ATTENDEE",
        "CATEGORIES","COMMENT","CONTACT","EXDATE","RELATED-TO","RESOURCES","RDATE"
    };

    if(isValidProperty(prop) == false){
        return false;
    }

    for(int i = 0; i < 26; i++){
        if(strcmp(prop->propName, validEventProps[i]) == 0){
            return true;
        }
    }

    return false;

}

bool chkAlarms(List* alarms){

    ListIterator alarmIterator;
    Alarm* tmp;

    alarmIterator = createIterator(alarms);
    for(int i = 0; i < alarms->length; i++){
        tmp = (Alarm*)nextElement(&alarmIterator);
        if(tmp->action == NULL || tmp->trigger == NULL){
            return false;
        }
        else if(tmp->action[0] == '\0'){
            return false;
        }
        else if(tmp->properties == NULL){
            return false;
        }
        else if(chkAlarmProps(tmp->properties) == false){
            return false;
        }
        // printf("ALARM: %s\n", tmp->trigger);
    }

    return true;

}

bool chkAlarmProps(List* alrProps){

    ListIterator alPropIterator;
    Property* prop;

    // int triggerNum = 0;
    // int actionNum = 0;
    int durationNum = 0;
    int repeatNum = 0;
    int attachNum = 0;

    alPropIterator = createIterator(alrProps);

    for(int i = 0; i < alrProps->length; i++){
        prop = (Property*)nextElement(&alPropIterator);
        if(isValidProperty(prop) == false){
            return false;
        }
        else if(isValidAlarmProp(prop) == false){
            return false;
        }
    }

    durationNum = numOccur(alrProps, "DURATION");
    repeatNum = numOccur(alrProps, "REPEAT");
    attachNum = numOccur(alrProps, "ATTACH");

    // printf("%d %d %d %d %d\n", triggerNum, actionNum, durationNum, repeatNum, attachNum);

    if(attachNum > 1 || durationNum > 1 || repeatNum > 1){
        return false;
    }
    else if(durationNum != repeatNum){
        return false;
    }

    return true;

}

bool isValidAlarmProp(Property* prop){
    char* alarmProps[3] = {
        "REPEAT","DURATION","ATTACH"
    };

    for(int i = 0; i < 3; i++){
        if(strcmp(prop->propName, alarmProps[i]) == 0){
            return true;
        }
    }

    return false;

}


char* subString(char* string, int start, int end){

    char *str = NULL;

    if(start == end){
        return NULL;
    }
    else if(start > end){
        return NULL;
    }
    else if(start < 0 || end < 0){
        return NULL;
    }
    else if(string == NULL){
        return NULL;
    }

    str = (char*)malloc( (end - start + 2) * sizeof(char) );
    for(int i = 0, j = start; j < end; i++){
        str[i] = string[j];
    }

    return str;

}