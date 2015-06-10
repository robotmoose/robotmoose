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
      delete cur;
    }
    head=0;
  }
  
  /// Assign this as our new sole action,
  ///  removing all other actions.
  void set(action *a) {
    reset(); add(a);
  }
};


/**
 This global stores the current time, in milliseconds,
 the last time you called action_loop().
 (Here because calling timer functions repeatedly is slow.)
*/
extern long action_time_ms; 

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

