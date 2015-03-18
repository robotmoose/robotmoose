/**
 This Arduino header defines "action", which runs code at known intervals.
 
 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#ifndef __action__H
#define __action__H

#include <Arduino.h>

/**
 One I/O device, which takes a loop() call repeatedly.
 You do want to inherit from this class to do your own stuff.
*/
class action {
public:
  action() { next=NULL; }
  virtual ~action() { /* does nothing */ }
  
  /// Do your functionality in loop here.
  virtual void loop() =0;
  
  // Don't mess with this, which is owned by the action_list
  action *next;
};

/**
 A set of I/O devices, which all loop together.
*/
class action_list {
  action *head;
public:
  action_list() { head=NULL; }
  ~action_list() { /* arguably, delete head and friends here */ }
  
  /// Add this device to our list
  void add(action *dev) {
    dev->next=head;
    head=dev;
  }
  
  /// Run the loop function on each device
  void loop() {
    for (action *cur=head;cur!=0;cur=cur->next)
    {
      cur->loop();
    }
  }
};


/**
 Calling timer functions repeatedly is slow,
 so this global stores the current time, in milliseconds,
 the last time you called loop()
*/
extern long action_time_ms; 

extern action_list actions_always; // run every loop()
extern action_list actions_1ms; // run at most every 1 ms
extern action_list actions_10ms; // every 10ms
extern action_list actions_100ms; // every 100ms
extern action_list actions_1s; // every 1 second
extern action_list actions_10s; // every 10 seconds
extern action_list actions_100s; // every 100 seconds
extern action_list actions_1000s; // every 1000 seconds

extern void action_setup();
extern void action_loop();

#endif

