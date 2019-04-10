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


ICalErrorCode createCalendar(char* fileName, Calendar** obj) {

    FILE* fp = NULL;
    char* buffer = NULL;
    ICalErrorCode code = OK; 

    if(fileName == NULL || strcmp(fileName, "") == 0 || isCalFile(fileName) == false){
        return INV_FILE;
    }
    
    fp = fopen(fileName, "r");

    if (fp == NULL) {
        return INV_FILE;
    }
    else {
        buffer = (char*)malloc(80 * sizeof(char));
        if( fgets(buffer, 80*sizeof(char), fp) ) {

            char nextChar = '\0';
            int numLines = 1;
            while( buffer[strlen(buffer) - 2] == '\r' && buffer[strlen(buffer) - 1] == '\n' && (nextChar = fgetc(fp)) != EOF && (nextChar == ' ' || nextChar == '\t') ){
                numLines++;
                char* line = NULL;
                size_t len2 = 0, lincapp2 = 0;
                len2 = getline(&line, &lincapp2, fp);
                buffer = realloc(buffer, (strlen(buffer) + len2 + 1) * sizeof(char));
                strtok(buffer, "\r\n");
                strcat(buffer, line);
                free(line);
            }
            ungetc(nextChar, fp);

            strtok(buffer, "\r\n");
            if(strcmp(buffer, "BEGIN:VCALENDAR") == 0) {
                *obj = parseCalendar(&fp, "VCALENDAR", &code);
            }
            else{
                free(buffer);
                return INV_CAL;
            }
        }
        else{
            free(buffer);
            return INV_FILE;
        }
        free(buffer);
    }

    fclose(fp);

    return code;

}

void deleteCalendar(Calendar *obj) {
    if(obj != NULL) {
        if(obj->events)
        {
            freeList(obj->events);
        }
        if(obj->properties)
        {
            freeList(obj->properties);
        }
        free(obj);
    }
}

char* printError(ICalErrorCode err){

    char* buffer = NULL;

    switch (err)
    {
        case OK:
            buffer = (char*)malloc(3 * sizeof(char));
            strcpy(buffer, "OK");       
            break;
        case INV_FILE:
            buffer = (char*)malloc(9 * sizeof(char));
            strcpy(buffer, "INV_FILE");
            break;
        case INV_CAL:
            buffer = (char*)malloc(8 * sizeof(char));
            strcpy(buffer, "INV_CAL");
            break;
        case INV_VER:
            buffer = (char*)malloc(8 * sizeof(char));
            strcpy(buffer, "INV_VER");
            break;
        case DUP_VER:
            buffer = (char*)malloc(8 * sizeof(char));
            strcpy(buffer, "DUP_VER");
            break;
        case INV_PRODID:
            buffer = (char*)malloc(11 * sizeof(char));
            strcpy(buffer, "INV_PRODID");
            break;
        case DUP_PRODID:
            buffer = (char*)malloc(11 * sizeof(char));
            strcpy(buffer, "DUP_PRODID");
            break;
        case INV_EVENT:
            buffer = (char*)malloc(10 * sizeof(char));
            strcpy(buffer, "INV_EVENT");
            break;
        case INV_DT:
            buffer = (char*)malloc(7 * sizeof(char));
            strcpy(buffer, "INV_DT");
            break;
        case INV_ALARM:
            buffer = (char*)malloc(10 * sizeof(char));
            strcpy(buffer, "INV_ALARM");
            break;
        case WRITE_ERROR:
            buffer = (char*)malloc(12 * sizeof(char));
            strcpy(buffer, "WRITE_ERROR");
            break;
        case OTHER_ERROR:
            buffer = (char*)malloc(12 * sizeof(char));
            strcpy(buffer, "OTHER_ERROR");
        break;
    
        default:

            break;
    }

    return buffer;
}


/** This is a sad function **/
char* printCalendar(const Calendar* obj) {
    char* buffer;

    if(obj == NULL){
        return NULL;
    }

    int len = strlen(obj->prodID) + sizeof(float) + 24;
    buffer = (char*)malloc( len * sizeof(char));

    sprintf(buffer, "PRODID: %s\nVERSION: %f\n", obj->prodID, obj->version);

    return buffer;
}


/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre
    Calendar object exists, and is not NULL.
    fileName is not NULL, has the correct extension
 *@post Calendar has not been modified in any way, and a file representing the
       Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when traversing the Calendar
 *@param
    obj - a pointer to a Calendar struct
 	fileName - the name of the output file
 **/
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){
    
    FILE *fp = NULL;
    // ListIterator eventIterator;
    // ListIterator alarmIterator;
    // ListIterator propIterator;
    // Event* tmp = NULL;
    char* events = NULL;
    char* props = NULL;

    if(isCalFile(fileName) == false){
        return WRITE_ERROR;
    }
    if(obj == NULL){
        return WRITE_ERROR;
    }
    if(obj->events == NULL || obj->properties == NULL){
        return INV_CAL;
    }
    
    fp = fopen(fileName, "w");

    if(fp == NULL){
        return WRITE_ERROR;
    }

    events = toString(obj->events);
    props = toString(obj->properties);

    if(fprintf(fp, "BEGIN:VCALENDAR\r\nVERSION:%0.0f\r\nPRODID:%s\r\n%s%sEND:VCALENDAR\r\n", obj->version, obj->prodID, props, events) < 0){
        free(events);
        free(props);
        fclose(fp); 
        return WRITE_ERROR;   
    }

    free(events);
    free(props);
    fclose(fp);

    return OK;    
}


/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not NULL
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode validateCalendar(const Calendar* obj){
    
    ICalErrorCode code;

    if(obj == NULL){
        code = INV_CAL;
    }
    else
    {
        if( obj->version < 2.0 ){
            code = INV_CAL;
        }
        else if( obj->prodID == NULL || strcmp(obj->prodID, "") == 0 ){
            code = INV_CAL;
        }
        else if( obj->events == NULL || obj->properties == NULL ){
            code = INV_CAL;
        }
        else if( obj->events->length == 0){
            code = INV_CAL;
        }
        else if( chkCalProps(obj->properties) == false ){
            code = INV_CAL;
        }
        else{
            code = chkCalEvents(obj->events);
        }
    }

    return code;
}

/** Function to converting a DateTime into a JSON string
 *@pre N/A
 *@post DateTime has not been modified in any way
 *@return A string in JSON format
 *@param prop - a DateTime struct
 **/
char* dtToJSON(DateTime prop){
    char* string = NULL;
    
    if(chkDT(prop) == false){
        return NULL;
    }
    else{
        string = (char*)malloc( (42 + strlen(prop.date) + strlen(prop.time)) * sizeof(char));
    }

    //{"date":"date val","time":"time val","isUTC":utcVal}
    sprintf(string, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}", prop.date, prop.time, prop.UTC ? "true" : "false");

    return string;
}

/** Function to converting an Event into a JSON string
 *@pre Event is not NULL
 *@post Event has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to an Event struct
 **/
char* eventToJSON(const Event* event){
    char* string = NULL;
    char* dtString = NULL;
    char* summaryStr = NULL;
    char* propStr = NULL;
    char* alarmStr = NULL;
    int numEventProps = 0;
    ListIterator evPropIterator;

    if(event == NULL){
        string = (char*)malloc(3*sizeof(char));
        strcpy(string, "{}");
    }
    else{
        numEventProps = 3 + event->properties->length;
        if( (dtString = dtToJSON(event->startDateTime)) == NULL) return NULL;

        if(numOccur(event->properties, "SUMMARY") == 1){
            evPropIterator = createIterator(event->properties);
            Property* tmp = NULL;
            for(int i = 0; i < event->properties->length; i++){
                tmp = (Property*)nextElement(&evPropIterator);
                if(strcmp(tmp->propName, "SUMMARY") == 0){
                    summaryStr = (char*)malloc( (strlen(tmp->propDescr)+1) * sizeof(char) );
                    strcpy(summaryStr, tmp->propDescr);
                    break;
                }
            }
        }
        else{
            summaryStr = (char*)malloc(1 + sizeof(char));
            strcpy(summaryStr, "");
        }

        propStr = propListToJSON(event->properties);
        alarmStr = alarmListToJSON(event->alarms);
        string = (char*)malloc( (90 + strlen(dtString) + strlen(summaryStr) + strlen(propStr) + strlen(alarmStr)) * sizeof(char) );
        //{"startDT":DTval,"numProps":propVal,"numAlarms":almVal,"summary":"sumVal"}

        sprintf(string, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"props\":%s,\"alarms\":%s}", dtString, numEventProps, event->alarms->length, summaryStr, propStr, alarmStr);

        free(dtString);
        free(summaryStr);
        free(propStr);
        free(alarmStr);

    }

    return string;
}

/** Function to converting an Event list into a JSON string
 *@pre Event list is not NULL
 *@post Event list has not been modified in any way
 *@return A string in JSON format
 *@param eventList - a pointer to an Event list
 **/
char* eventListToJSON(const List* eventList){
    char* string = NULL;
    char* tmpStr = NULL;
    ListIterator eventIterator;

    if(eventList == NULL){
        string = (char*)malloc(3*sizeof(char));
        strcpy(string, "[]");
        return string;
    }

    string = malloc(3 * sizeof(char));
    strcpy(string, "[");

    eventIterator = createIterator((List*)eventList);
    for(int i = 0; i < eventList->length; i++){
        Event* tmp = (Event*)nextElement(&eventIterator);
        tmpStr = eventToJSON(tmp);
        string = realloc(string, (strlen(string) + strlen(tmpStr) + 3) * sizeof(char));
        strcat(string, tmpStr);
        
        if(i < eventList->length - 1) strcat(string, ",");
        free(tmpStr);
    }

    strcat(string, "]");

    return string;
}

/** Function to converting a Calendar into a JSON string
 *@pre Calendar is not NULL
 *@post Calendar has not been modified in any way
 *@return A string in JSON format
 *@param cal - a pointer to a Calendar struct
 **/
char* calendarToJSON(const Calendar* cal){
    char* string = NULL;
    int numProps = 0;

    if(cal == NULL){
        string = (char*)malloc(3 * sizeof(char));
        strcpy(string, "{}");
    }
    else{
        numProps = cal->properties->length + 2;
        string = (char*)malloc( (70 + strlen(cal->prodID)) * sizeof(char) );        
        
        //{"version":verVal,"prodID":"prodIDVal","numProps":propVal,"numEvents":evtVal}
        sprintf(string, "{\"version\":%0.0f,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",cal->version, cal->prodID, numProps, cal->events->length);

    }

    return string;
}

/** Function to converting a JSON string into a Calendar struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Calendar struct
 *@param str - a pointer to a string
 **/
Calendar* JSONtoCalendar(const char* str){
    //string format to parse
    //{"version":verVal,"prodID":"prodIDVal"}
    
    char* stringToParse = NULL;
    char* colon = ":";
    char* comma = ",";
    char* version = NULL;  
    char* verVal = NULL;  
    char* prodID = NULL;  
    char* prodIDVal = NULL;

    Calendar* tmp = NULL;

    if(str == NULL){
        return NULL;
    }

    stringToParse = (char*)malloc( (strlen(str) + 1) * sizeof(char) );
    strcpy(stringToParse, str);

    version = strtok(stringToParse, colon);
    verVal = strtok(NULL, comma);
    prodID = strtok(NULL, colon);
    prodIDVal = strtok(NULL, comma);

    if(version != NULL && verVal != NULL && prodID != NULL && prodIDVal != NULL){
        if(strcmp(version, "{\"version\"") == 0 && strcmp(prodID, "\"prodID\"") == 0){
            tmp = (Calendar*)malloc(sizeof(Calendar));
            tmp->events = initializeList(printEvent, deleteEvent, compareEvents);
            tmp->properties = initializeList(printProperty, deleteProperty, compareProperties);

            errno = 0;
            tmp->version = strtol(verVal, NULL, 10);
        
            char* token = strtok(prodIDVal, "\"");
            strcpy(tmp->prodID, token);
        }
    }

    free(stringToParse);

    return tmp;
}

/** Function to converting a JSON string into an Event struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Event struct
 *@param str - a pointer to a string
 **/
Event* JSONtoEvent(const char* str){
    //string format to parse
    //{"UID":"value"}

    char* string = NULL;
    Event* tmp = NULL; 
    char* token = NULL;   
    char* uid = NULL;
    char* value = NULL;

    if(str == NULL){
        return NULL;
    }

    string = (char*)malloc((strlen(str) + 1)*sizeof(char));
    strcpy(string, str);

    token = strtok(string, "{}");
    printf("token: %s\n", token);

    uid = strtok(token, ":");
    value = strtok(NULL, ":");
    value = strtok(value, "\"");

    

    if(uid != NULL && value != NULL && strcmp(uid, "\"UID\"") == 0){
        tmp = (Event*)malloc(sizeof(Event));
        tmp->properties = initializeList(printProperty, deleteProperty, compareProperties);
        tmp->alarms = initializeList(printAlarm, deleteAlarm, compareProperties);
        strcpy(tmp->UID, value);
        // printf("%s %s\n", uid, value);
    }

    free(string);

    return tmp;
}

/** Function to adding an Event struct to an ixisting Calendar struct
 *@pre arguments are not NULL
 *@post The new event has been added to the calendar's events list
 *@return N/A
 *@param cal - a Calendar struct
 *@param toBeAdded - an Event struct
 **/
void addEvent(Calendar* cal, Event* toBeAdded){
    if(cal != NULL && toBeAdded != NULL){
        if(cal->events != NULL) insertBack(cal->events, toBeAdded);
    }
}

void deleteEvent(void* toBeDeleted){
    Event* tmpEvent = NULL;

    if(toBeDeleted == NULL) return;

    tmpEvent = (Event*)toBeDeleted;

    if(tmpEvent->properties){
        freeList(tmpEvent->properties);
    }
    if(tmpEvent->alarms){
        freeList(tmpEvent->alarms);
    }
    free(tmpEvent);
}

int compareEvents(const void* first, const void* second){
    return 0;
}

char* printEvent(void* toBePrinted){
    int len;
    char* buffer = NULL;
    char* propStr = NULL;
    char* alarStr = NULL;
    char* dtStamp = NULL;
    char* dtStart = NULL;
    Event* tmpEvent = NULL;
    
    if(toBePrinted == NULL) return NULL;

    tmpEvent = (Event*)toBePrinted;

    propStr = toString(tmpEvent->properties);
    alarStr = toString(tmpEvent->alarms);
    dtStamp = printDate(&(tmpEvent->creationDateTime));
    dtStart = printDate(&(tmpEvent->startDateTime));

    len = strlen(tmpEvent->UID) + strlen(dtStamp) + strlen(dtStart) + strlen(propStr) + strlen(alarStr) + 60;
    buffer = (char*)malloc(len * sizeof(char));

    sprintf(buffer, "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s\r\n%s%sEND:VEVENT\r\n", tmpEvent->UID, dtStamp, dtStart, propStr, alarStr);

    free(propStr);
    free(alarStr);
    free(dtStamp);
    free(dtStart);

    return buffer;
}

void deleteAlarm(void* toBeDeleted){
    Alarm *tmp = NULL;

    if(toBeDeleted == NULL) return;

    tmp = (Alarm*)toBeDeleted;

    if(tmp->trigger) free(tmp->trigger);
    if(tmp->properties) freeList(tmp->properties);
    free(tmp);
}

int compareAlarms(const void* first, const void* second){
    return 0;
}

char* printAlarm(void* toBePrinted){
    int len;
    char* buffer = NULL;
    Alarm* tmp = NULL;
    char* propstr = NULL;

    if(toBePrinted == NULL) return NULL;

    tmp = (Alarm*)toBePrinted;
    propstr = toString(tmp->properties);

    len = strlen(tmp->action) + strlen(tmp->trigger) + strlen(propstr) + 59;
    buffer = (char*)malloc(len*sizeof(char));

    sprintf(buffer, "BEGIN:VALARM\r\nTRIGGER:%s\r\nACTION:%s\r\n%sEND:VALARM\r\n", tmp->trigger, tmp->action, propstr);

    free(propstr);

    return buffer;
}

void deleteProperty(void* toBeDeleted) {
    Property *tmpProp;

    if (toBeDeleted == NULL){
		return;
	}

    tmpProp = (Property*)toBeDeleted;

    free(tmpProp);
}

int compareProperties(const void* first, const void* second){
    Property *tmpProp1, *tmpProp2;

    if (first == NULL || second == NULL){
		return 0;
	}

    tmpProp1 = (Property*)first;
    tmpProp2 = (Property*)second;

    return strcmp((char*)tmpProp1->propName, (char*)tmpProp2->propName);
}

char* printProperty(void* toBePrinted) {
    Property *tmpProp;
    char *tmpStr;
    int length;

    if(toBePrinted == NULL)
    {
        return NULL;
    }

    tmpProp = (Property*)toBePrinted;

    length = strlen(tmpProp->propDescr) + strlen(tmpProp->propName) + 4;
    tmpStr = (char*)malloc(sizeof(char) * length);

    sprintf(tmpStr, "%s:%s\r\n", tmpProp->propName, tmpProp->propDescr);

    return tmpStr;
}

void deleteDate(void* toBeDeleted){

}

int compareDates(const void* first, const void* second){
    return 0;
}

char* printDate(void* toBePrinted){
    char* buffer;
    DateTime* tmp = NULL;

    if(toBePrinted == NULL) return NULL;
    tmp = (DateTime*)toBePrinted;

    int len = strlen(tmp->date) + strlen(tmp->time) + 7;
    buffer = (char*)malloc(len * sizeof(char));

    if(tmp->UTC){
        sprintf(buffer, "%sT%sZ", tmp->date, tmp->time);
    }
    else{
        sprintf(buffer, "%sT%s", tmp->date, tmp->time);
    }

    return buffer;
}
 
char* propToJSON(void* toBePrinted) {
    Property *tmpProp;
    char *tmpStr;
    int length;

    if(toBePrinted == NULL)
    {
        return NULL;
    }

    tmpProp = (Property*)toBePrinted;

    length = strlen(tmpProp->propDescr) + strlen(tmpProp->propName) + 4;
    tmpStr = (char*)malloc(sizeof(char) * length);

    sprintf(tmpStr, "\"%s:%s\"", tmpProp->propName, tmpProp->propDescr);

    return tmpStr;
}


char* propListToJSON(List* props){
    
    char* string = NULL;
    ListIterator propIter;

    if(props == NULL){
        string = (char*)malloc(3*sizeof(char));
        strcpy(string, "[]");
        return string;
    }
    
    string = (char*)malloc(3*sizeof(char));
    strcpy(string, "[");

    propIter = createIterator((List*)props);
    for(int i = 0; i < props->length; i++){
        Property* tmp = (Property*)nextElement(&propIter);
        char* tmpStr = propToJSON(tmp);
        string = realloc(string, (strlen(string) + strlen(tmpStr)+3) * sizeof(char));
        strcat(string, tmpStr);

        if(i < props->length - 1) strcat(string, ",");
        free(tmpStr);
    }
    strcat(string, "]");

    return string;
}

char* alrmToJSON(Alarm* alarm){
    
    char* string = NULL;
    char* propstr = NULL;
    int len;

    if(alarm == NULL){
        string = (char*)malloc(3* sizeof(char));
        strcpy(string, "{}");
        return string;
    } 

    propstr = propListToJSON(alarm->properties);

    len = strlen(alarm->action) + strlen(alarm->trigger) + strlen(propstr) + 59;
    string = (char*)malloc(len*sizeof(char));

    sprintf(string, "{\"trigger\":\"%s\",\"action\":\"%s\",\"props\":%s}", alarm->trigger, alarm->action, propstr);

    free(propstr);

    return string;
}

char* alarmListToJSON(List* alarms){
    char* string = NULL;
    ListIterator propIter;

    if(alarms == NULL){
        string = (char*)malloc(3*sizeof(char));
        strcpy(string, "[]");
        return string;
    }
    
    string = (char*)malloc(3*sizeof(char));
    strcpy(string, "[");

    propIter = createIterator((List*)alarms);
    for(int i = 0; i < alarms->length; i++){
        Alarm* tmp = (Alarm*)nextElement(&propIter);
        char* tmpStr = alrmToJSON(tmp);
        string = realloc(string, (strlen(string) + strlen(tmpStr)+3) * sizeof(char));
        strcat(string, tmpStr);

        if(i < alarms->length - 1) strcat(string, ",");
        free(tmpStr);
    }
    strcat(string, "]");

    return string;
}

char* getCalJSON(char* pathToFile){
    char* string = NULL;
    char* eventsStr = NULL;
    // char* propsStr = NULL;
    int len = 0;
    int numProps = 0;
    Calendar* cal = NULL;
    ICalErrorCode errCode = OK; 
    
    errCode = createCalendar(pathToFile, &cal);
    
    if(errCode != OK) {
        string = printError(errCode);
    }
    else {
        eventsStr = eventListToJSON(cal->events);
        len = strlen(eventsStr) + strlen(cal->prodID) + 80;
        numProps = cal->properties->length + 2;
        string = (char*)malloc( len * sizeof(char) );        
        //{"version":verVal,"prodID":"prodIDVal","numProps":propVal,"numEvents":evtVal}
        sprintf(string, "{\"version\":%0.0f,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d,\"events\":%s}",cal->version, cal->prodID, numProps, cal->events->length, eventsStr);
        // printf("%s\n", string);
    }

    if(eventsStr) free(eventsStr);
    deleteCalendar(cal);
    return string;
}

char* calUpdateNewEvent(char* fileToUpdatePath, char* startDate, char* startTime, char* createDate, char* createTime, char* uid, bool isUTC, char* summary){
    
    ICalErrorCode readErrCode = INV_FILE;
    ICalErrorCode writeErrCode = INV_FILE;
    Calendar* cal = NULL;
    Event* eventToAdd = NULL;

    readErrCode = createCalendar(fileToUpdatePath, &cal);
    if( cal != NULL && readErrCode == OK ){
        
        eventToAdd = (Event*)malloc(sizeof(Event));
        eventToAdd->properties = initializeList(printProperty, deleteProperty , compareProperties);
        eventToAdd->alarms = initializeList(printAlarm, deleteAlarm, compareAlarms);
        strcpy(eventToAdd->UID, uid);
        strcpy(eventToAdd->startDateTime.date, startDate);
        strcpy(eventToAdd->startDateTime.time, startTime);
        eventToAdd->startDateTime.UTC = isUTC;
        strcpy(eventToAdd->creationDateTime.date, createDate);
        strcpy(eventToAdd->creationDateTime.time, createTime);
        eventToAdd->creationDateTime.UTC = isUTC;
        
        
        if(strlen(summary) > 0){
            Property* tmp = (Property*)malloc( sizeof(Property) + (sizeof(char)*(strlen(summary)+1)) );
            strcpy(tmp->propName, "SUMMARY");
            strcpy(tmp->propDescr, summary);
            insertBack(eventToAdd->properties, tmp);
        }

        insertBack(cal->events, eventToAdd);

        writeErrCode = writeCalendar(fileToUpdatePath, cal);
        
        deleteCalendar(cal);
        return printError(writeErrCode);

    }

    return printError(readErrCode);

}

char* valCreateClientCal(char* filename, char* calstr){

    char* status = NULL;
    ICalErrorCode createErr = OK;
    Calendar* cal = NULL;
    FILE* fp = NULL;

    fp = fopen(filename, "w");
    fprintf(fp, calstr);
    fclose(fp);

    createErr = createCalendar(filename, &cal);
    if(cal != NULL && createErr == OK){
        status = printError(validateCalendar(cal));
    }
    else{
        status = printError(createErr);
    }
    deleteCalendar(cal);

    return status;
}