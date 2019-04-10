/**
 * Author: Waqas Bakht
 * id: 0981571
 * email: wbakht@uoguelph.ca
 * course: CIS2750 
 * Prof: Denis Nikitenko
*/

#ifndef HELPER_H
#define HELPER_H

#define _GNU_SOURCE
#include <math.h>
#include <errno.h>
#include "CalendarParser.h"
#include "LinkedListAPI.h"

//some repititious strings were birthed into macros!
#define cale "VCALENDAR"
#define vala "VALARM"
#define even "VEVENT"

/** Function to parse through the file, recursively returning the type of struct to return
 * @pre file must be opened and the first thing in the file must be the begin vcalendar
 * @post calendar object has been created and returned
 * @return either a alarm, event or calendar
*/
void* parseCalendar(FILE **fp, char *type, ICalErrorCode *code);

/** Function to verify that date time matches the form specified
 * @pre needs the date-time string passed to it
 * @post nothing is changed
 * @return boolean value for if the string conforms to the spec
*/
bool dateMatchesForm(char* date);

/** Function to verify that the filename has the correct extention
 * @pre string representing path to the calender file
 * @param:
 *  filename - a string to represent a path to calendar file
 * @return: false if filename is not in correct format, true otherwise
*/
bool isCalFile(char *filename);

/** Function to verify that the Properties in a Calendar are 
 *  in conformance with RFC5545
 * @pre: An initialized calendar with a list of properties
 * @post: Nothing is changed
 * @param: list - list of CALENDAR Properties
 * @return: true or false, depending on whether the list of 
            properties is in conformace with RFC5545
*/
bool chkCalProps(List* list);


/** Function to check the events of a calendar
 * ...
 * ...
 * ...
*/
ICalErrorCode chkCalEvents(List* list);

/** Function to verify whether an Event in a Calendar object
    is in conformance with the RFC5545 spec
 * @pre: An Event with initialized components
 * @post: Nothing is Changed
 * @param: event - the Event to be verified
 * @return: Either INV_EVENT or INV_ALARM depending on 
            if the problem is in the Alarms list or 
            the Events components
*/
ICalErrorCode isValidEvent(const Event* event);

/** Function to check whether a DateTime Struct is properly 
    initialized and set
 * @pre: A DateTime struct initialized by an Event Struct
 * @post: Nothing is changed
 * @param: dt - The DateTime struct in question
 * @return: true/false - depending on if the DateTime struct
            has been properly initialized/set 
*/
bool chkDT(DateTime dt);

/** Function to check if an Events properties is in conformance
    with the VEVENT spec outline in RFC5545
 * @pre: A list of event properties
 * @post: Nothing is changed
 * @param: list - list of event properties
 * @return: true/false - depending on if the above is true
*/
bool chkEvtProps(List* list);

/** Function to check if a property is valid according to 
    the valid event properties described in RFC5545 
 * @pre: An event property
 * @post: Nothing is changed
 * @param: prop - property in question
 * @return: true/false
*/
bool isValidEventProp(Property* prop);

/** A function to check if the Alarms component of An Event 
    is in conformance with RFC5545
 * @pre: The list of Alarms from an event struct
 * @post: Nothing is changed
 * @param: alarms list of an event
 * @return: true/false
*/
bool chkAlarms(List* alarms);

/** Function to check if the list of properties in 
    an alarm is in conformance with the RFC5545 spec
 * @pre: Need an Alarm with an list of properties
 * @post: Nothing is changed
 * @param: list of alarm properties
 * @return: true/false
*/
bool chkAlarmProps(List* alrProps);

/** Function to check if a single alarm property is in 
    conformance with the spec outlined in RFC5545 specifications
 * @pre: An initialized alarm property
 * @post: Nothing is changed
 * @param: A single alarm property
 * @return: true/false
*/
bool isValidAlarmProp(Property* prop);

/** Helper function to get the number of
    times a property occurs in a list of properties
 * @pre: A list of properties, the use of this function 
         is determined by the caller
 * @post: Nothing is changed
 * @param: props - list of properties
           string - Property Name of property you want to count
 * @return: num of time the property occurs in the list 
*/
int numOccur(List* props, char* string);

/** Function to check if a property has
    been properly initialized 
 * @pre: A property 
 * @post: Nothing is changed
 * @param: the property in question
 * @return: true/false
*/
bool isValidProperty(const Property* prop);


/** Function to return part of a string specified by caller
 * @pre: Non NULL string
 * @post: Nothing is changed
 * @param: string - the string to be chopped up
 *         start - the start index of substring
 *         end - the end index of substring
 * @return: a substring 
*/
char* subString(char* string, int start, int end);

#endif
