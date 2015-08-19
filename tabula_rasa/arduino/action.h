/**
 This Arduino header defines "action", which runs code at known intervals.
 
 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#ifndef __ACTION__H
#define __ACTION__H

#include <Arduino.h>

/**
 One I/O device, which takes a loop() call repeatedly.
 You do want to inherit from this class to do your own stuff.
*/
class action {
public:
  action() { next=NULL; }
  virtual ~action() { /* does nothing */ }
  
  /// Do your functionality here--this will execute every time.
  virtual void loop() =0;
  
  // Don't mess with this, which is owned by the action_list
  action *next;
};

/**
 A set of actions, which all run loop at roughly the same time.
*/
class action_list {
  action *head;
public:
  action_list() { head=NULL; }
  ~action_list() { reset(); }
  
  /// Add this new action to our list.
  ///  Your action MUST be allocated using new!
  void add(action *a) {
    a->next=head;
    head=a;
  }
  
  /// Run the loop function on each action
  void loop() {
    for (action *cur=head;cur!=0;cur=cur->next)
    {
      cur->loop();
    }
  }
  
  /// Remove all existing actions from our list
  void reset() {
    action *cur=head;
    while (cur!=0)
    {
      action *doomed=cur;
      cur=cur->next;
      delete doomed;
    }
    head=0;
  }
};

/**
  This is a time, in milliseconds.
  Note that it wraps around every 65 seconds on an Arduino--
  this is to force you to fix timer wraparound bugs!
*/
typedef unsigned int milli_t;

/**
 This global stores the current time, in milliseconds,
 the last time you called action_loop().
 (Here because calling timer functions repeatedly is slow.)
*/
extern milli_t action_time_ms; 

/**
  This bool goes true when we're receiving commands from the PC,
  and is false otherwise.  It's handy for putting the Roomba into safe mode when the backend shuts down.
*/
extern bool pc_connected;

/** 
  These are scheduling rungs, used to avoid inefficient duplicate polling.
  
  You add your action to the appropriate rung, like
      actions_10ms.add(new my_crazy_device(A3));
*/
enum {action_lists_count=6};
extern action_list action_lists[action_lists_count];

#define actions_always action_lists[0] // run every loop()
#define actions_1ms action_lists[1] // run at most every 1 ms
#define actions_10ms action_lists[2] // every 10ms
#define actions_100ms action_lists[3] // every 100ms
#define actions_1s action_lists[4] // every 1 second
#define actions_10s action_lists[5] // every 10 seconds

extern void action_setup();
extern void action_loop();

#endif

