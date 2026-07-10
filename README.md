# BlockNot Arduino Library

This library enables you to create non-blocking timers using simple, common sense terms which simplifies the reading and
writing of your code. It offers, among several things, convenient timer functionality, but most of all ... it gets you
away from blocking methods - like delay() - as a means of managing events in your code.

**Non-Blocking is the proper way to implement timing events in Arduino code and BlockNot makes it easy!**

### *** If you are updating to version 3.1.0, READ THIS: `getHelp()` has been removed entirely (not deprecated) to save flash/RAM on memory-constrained boards. If your code calls `myTimer.getHelp(...)`, remove that call - the same macro reference table it used to print is documented in the [Macros](#macros) section below. See [CHANGELOG.md](CHANGELOG.md) for details.

### *** If you are updating to this new version (2.4.0), [READ THIS](#deprecated-macros) so that your code doesn't stop working.

### *** If you are noticing unwanted rapid succession triggering on high speed microcontrollers, [READ THIS](#triggering-too-fast-with-high-speed-microcontrollers)

# Table of Contents

<!-- TOC -->

* [Quick Start](#quick-start)
* [Theory behind BlockNot](#theory-behind-blocknot)
* [How To Use BlockNot](#how-to-use-blocknot)
    * [The Trigger](#the-trigger)
        * [One Time Trigger](#one-time-trigger)
        * [Trigger Next](#trigger-next)
        * [Last Trigger Duration](#last-trigger-duration)
        * [Triggered OnDuration](#triggered-onduration)
            * [Default Behavior](#default-behavior)
            * [OnDuration(ALL)](#ondurationall)
    * [The Reset](#the-reset)
        * [Global Reset](#global-reset)
    * [Time Unit Options](#time-unit-options)
        * [Default](#default)
        * [Other Units](#other-units)
            * [BlockNot Assumptions](#blocknot-assumptions)
            * [Microseconds](#microseconds)
        * [Converting Units](#converting-units)
        * [Changing Duration](#changing-duration)
        * [Switching Base Units](#switching-base-units)
    * [Start / Stop](#start--stop)
        * [Return Values on Stopped Timers](#return-values-on-stopped-timers)
    * [Summary](#summary)
* [Examples](#examples)
    * [Advanced Auto Flashers](#advanced-auto-flashers)
    * [BlockNot Blink](#blocknot-blink)
    * [BlockNot Blink Party](#blocknot-blink-party)
    * [Millis Rollover Test](#millis-rollover-test)
    * [Button Debounce](#button-debounce)
    * [Duration Trigger](#duration-trigger)
    * [On With Off Timers](#on-with-off-timers)
    * [Reset All](#reset-all)
    * [Timers Rules](#timers-rules)
* [Library](#library)
    * [Methods](#methods)
    * [Macros](#macros)
    * [Constants](#constants)
* [Discussion](#discussion)
    * [Memory](#memory)
    * [Rollover](#rollover)
    * [Thread Safety](#thread-safety)
    * [Triggering Too Fast With High Speed Microcontrollers](#triggering-too-fast-with-high-speed-microcontrollers)
    * [Deprecated Macros](#deprecated-macros)
* [Version Update Notes](#version-update-notes)
* [Suggestions](#suggestions)

<!-- TOC -->

# Quick Start

Here is an example of BlockNot's easiest and most common usage:

First, you create the timer:

```C++ 
#include <BlockNot.h>   
BlockNot helloTimer(1300); //In Milliseconds    
```

**OR optionally**

```C++ 
#include <BlockNot.h>   
BlockNot helloTimer(15, SECONDS); //Whole Seconds timer    
BlockNot helloTimer(120000, MICROSECONDS); //Microseconds timer    
```

Then, you just test it to see if it triggered.

```C++
   if (helloTimer.TRIGGERED) {  
      Serial.println("Hello World!"); 
   } 
 ``` 

Every time the TRIGGERED call returns true, the timer is reset and it won't trigger
again until the duration time has elapsed (all behaviors can be changed based on your needs).

That is all you need to start using BlockNot. Keep reading to learn about other features of the library.

# Theory behind BlockNot

This is a traditional non-blocking timer:

```C++  
long someDuration = 1300;  
long startTime = millis();  
if (millis() - startTime >= someDuration) {  
        //Code to run after someDuration has passed.
 }  
```  

This does the same thing, only with much simpler code!

```C++  
if (myTimer.TRIGGERED) {  
        //Code to run after timer has triggered.
 }  
```  

The idea behind BlockNot is very simple. You create the timer, setting its duration when you declare it, then check on
the timer in your looping code to see if it TRIGGERED. Or, you can check for other information such as how long until it
will trigger, or how much time has passed since it last triggered, or you can ask it what the current duration is, which
might be useful in scenarios where you change the duration based on dynamic criteria.

For example, if you wanted to see if the timer's duration has come to pass, but you don't want to reset the timer, you
can use this method:

```C++  
if (myTimer.triggered(NO_RESET)) {}  
```  

OR, you can do it like this:

```C++  
if (myTimer.HAS_TRIGGERED) {}  
```  

They both do the same thing, but in terms of readability, the second example is the obvious choice. BlockNot has several
easy to understand commands that make it very 'user-friendly' and make your code much more readable.

Here is a simple graph showing you how BlockNot timers work. What's important here is to realize that your code never
stops executing while the timer is passing time.

![](./img/visual.png)

# How To Use BlockNot

## The Trigger

BlockNot is all about the trigger event. When runners line up to start a race, it is a traditional practice
for someone to stand next to the line and hold a gun in the air and pull the trigger when the race starts.
That is the idea behind the TRIGGERED event in BlockNot. If your timer is set, for example, to 1300 milliseconds,
it will return true when you call the TRIGGERED event on or after 1300 milliseconds have passed ... there are
exceptions, however, as you will see, which can be useful.

```C++  
if (voltageReadTimer.TRIGGERED) {  
    readVoltage();
}  
```   

### One Time Trigger

I have personally found it quite handy in some scenarios to be able to get a boolean true response after the
timer has triggered, but only once, so that the code which executes after getting a true response only executes
once, and when the test comes up again in the loop, a response of false will be given until the timer has been
manually reset. The false response can be changed to true if you desire, by running this method:

```C++
myTimer.setFirstTriggerResponse(true);
```

This kind of trigger is called FIRST_TRIGGER and you use it like this:

```C++  
if (myTimer.FIRST_TRIGGER) { my code }  
```  

That method will return true ONLY ONE TIME after the timer's duration has passed, but subsequent calls to
that method will return false **until you manually reset the timer** like this:

```C++  
myTimer.RESET;  
```  

Why would you need to do that? There are countless scenarios where that would be immediately useful.
I have used it with stepper motor projects where I want an idle stepper motor to be completely cut off
from any voltage when it has been idle for a certain length of time ... let's say 25 seconds.

So first, we define the timer

```C++  
BlockNot stepperSleepTimer (25, SECONDS);  
```  

Then, we reset the timer every time we use the motor:

```C++  
stepperSleepTimer.RESET;  
```  

Then, in your loop, you would put something like this:

```C++  
if (stepperSleepTimer.FIRST_TRIGGER) {  
     sleepStepper();
 }  
```  

So that if the stepper hasn't moved in the last 25 seconds, it will be put to sleep and that sleep routine
won't execute over and over again each time the loop encounters the check. Yet when the stepper is engaged again,
the sleep timer is reset, and when it becomes idle again for 25 seconds, it is put to sleep. This helps efficiency
in your program, and it conserves valuable CPU time.

### Trigger Next

This allows you to signal the timer to trigger the next time it is checked, regardless of whether or not the duration
time has passed.

I have found situations where I am using more than one timer to accomplish some purpose, where I need a dependent
timer to trigger the next time it is checked depending on the situation. For example: when using a WiFi module in a
project, I often use a timer to periodically check to make sure WiFi is connected. If not, then I reset the module so
that it makes a clean effort to re-connect to the access point. But let's say that I have another method that will do
something like sync a local Real Time Clock with an Internet Network Time Protocol (NTP) server, but I only want to
run this method once the WiFi has connected. Let's also say that I prefer to re-run the time sync method every few
days to mitigate time drift in the local Real Time Clock. So I have a timer called `resyncTimeTimer`, but its duration
is set to execute every three days.

In that case, I want `resyncTimeTimer` to trigger the next time it is checked once the WiFi module has connected to the
local access point, so my code would look something like this:

```c++
#include <BlockNot.h>

enum UpDown {
    UP,
    DOWN
};

UpDown wifiState = DOWN;

void getWifiState() {
    wifiState = WiFi.status() == WL_CONNECTED ? UP : DOWN;
}

void connectWifi() {
    BlockNot wifiConnectTimeout(25, SECONDS);
    Serial.println("Attempting to connect to Wifi...");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    getWifiState();
    // This while loop will wait for the WiFi to connect for 25 seconds
    // If the Wifi connects OR 25 seconds pass, the loop exits.
    while (wifiState == DOWN and !wifiConnectTimeout.TRIGGERED) {
        getWifiState();   
    }
}

void loop() {
    static BlockNot checkWifiTimer(60, SECONDS);
    static BlockNot resyncTimeTimer(4320, MINUTES); // 72 hours (60 * 72)

    if (checkWifiTimer.TRIGGERED) {
        getWifiState();
        if (wifiState == DOWN) {
            connectWifi();
            resyncTimeTimer.TRIGGER_NEXT;
        }
    }
    
    if (resyncTimeTimer.TRIGGERED and wifiState == UP) {
        syncRTCTime();
    }
}
```

Using the TRIGGER_NEXT macro on the re-sync timer lets me use the code that the timer holds in the situation where I
need that code to run immediately instead of waiting for the timer to trigger. After it runs, the timer will not
trigger again until its duration has passed (three days in this example).

Code like this works especially well on a micro controller like the Pi Pico, where it can be run in a loop in a separate
thread in the second core of the CPU, allowing your main loop to keep running even while this code is holding things up
waiting for the wifi to connect.

### Last Trigger Duration

There can be times when you are collecting information about events using an interrupt pin, and it becomes necessary to
know how much time passed in the last data gathering interval.

```cpp
myTimer.LAST_TRIGGER_DURATION
```

will give you the amount of time that had actually elapsed the last time the timer triggered. This exists because it's
possible to check for the trigger after the timer's duration has already passed, meaning more time actually elapsed
than the duration you set - `lastTriggerDuration()` is how you find out exactly how much.

### Triggered OnDuration

This topic is a little tricky to comprehend (myself included as I
wrote this method), but I have done my best to explain it as simply as possible.

There might be times when it becomes necessary to respect a timer's trigger in the context of
its **duration**, so that when a timer's trigger is checked, it then resets its ```startTime``` to a time that is
relative to its duration rather than simply resetting to the current value of ```micros()``` or ```millis()```.

```triggeredOnDuration()``` does this with two optional results.

##### Default Behavior

Let's use a timer that is set to a duration of 500ms as our example.

You test it for TRIGGER by using either the macro or the method

```CPP
myTimer.TRIGGERED_ON_DURATION
myTimer.triggeredOnDuration()
```

BlockNot views the timer as having rigid trigger marks that happen exactly 500ms apart. So
if you check the timer - say 700ms after you start it, you will get a TRUE response, and
it will trigger again in 300ms instead of 500ms, because BlockNot will set the startTime
back to 500ms and not the current time of 700.

So let's say we are well into the passage of time, and you test for trigger at time index 2830 ...
BlockNot will return ```TRUE``` and then set the startTime back to 2500, so it will trigger again at 3000.

The idea is that you can have a timer that will provide the ability to run code on a consistent pulse, where each pulse
happens exactly at every interval of time based on the timer's duration, and you can have your code execute
as close to those 'pulse marks' as possible.

#### OnDuration(ALL)

```triggeredOnDuration(ALL)``` works exactly as ```triggeredOnDuration()``` EXCEPT that BlockNot
will continue to return a ```TRUE``` result until every missed mark is accounted for.

You can test for TRIGGERED using this macro or the method

```CPP
myTimer.TRIGGERED_ON_DURATION(ALL)
myTimer.triggeredOnDuration(ALL)
```

Continuing with our 500ms duration timer, if you test it at time index 1250 then again at
time index 1300, you will get a TRUE response for both tests, since you missed the first
mark at 500, but then tested after the second mark at 1000. If you tested again before
1500, you would get a ```FALSE``` response.

These graphs show you what will happen as time advances and you test for TRIGGERED
using this method.

A green Test means you tested and got TRUE while red means you got FALSE.

This is how the standard TRIGGERED event responds:

![](./img/cmpTRIGGERED.png)

This visualises how ```TRIGGERED_ON_DURATION``` works.

![](./img/cmpONMARK.png)

Notice how you can get a TRUE response immediately before and immediately after a trigger
event. BlockNot is giving you a TRUE response for the trigger that happened at time index
2500, then it gives a TRUE response for the trigger that happened at time index 3000. But
when you test again before 3500, you will get a FALSE response.

Here is how ```TRIGGERED_ON_DURATION(ALL)``` works.

![](./img/cmpONDURATION.png)

Notice you got FOUR consecutive TRUE results in a row. This is because you missed three triggers,
then you tested twice - once at a trigger point, then once again - and the 5th time you tested,
your test happened before the next trigger, and all of the missed trigger events had been accounted
for, so you get a FALSE response.

A possible use case for the ALL trigger test would be when using a timer as a sync counter of sorts. To explain
that, I am going to use an extreme example that should illustrate the point... Let's say that you have a project
that must automatically water plants at least six times every hour. It doesn't matter if the plants are watered every
10 minutes, or if they are watered once, then 15 minutes later, then again 5 minutes later etc., as long as they are
watered six times every hour.

We would define this timer as follows:

```CPP
BlockNot waterTimer = BlockNot(600, SECONDS); //10 minute duration
```

Let's also assume that your code is so busy doing other things that it might not
be able to check the trigger on that timer - possibly even after two full durations have passed.
When you check the trigger using ```TRIGGERED_ON_DURATION(ALL)```, that will cause BlockNot to continue giving
you a TRUE response until each missed trigger has been provided to you.

See the example sketch called **DurationTrigger** to see this method in action.

## The Reset

Resetting a timer is critical to performing repeated events at the right intervals. However, there may be times when you
don't want this behavior.

Resetting a timer once it has triggered is the default behavior of BlockNot.

```C++  
if (myTimer.TRIGGERED) { my code }  
```  

Using TRIGGERED, the timer automatically resets, whereas if you do this:

```C++  
if (myTimer.HAS_TRIGGERED) { my code }   
```  

The startTime **does not reset**, and that test will always come back true every time it is executed in your
code, as long as the timer's duration has passed. The exception, of course, would be using the FIRST_TRIGGER
method.

### Global Reset

Sometimes, having the ability to reset all of your timers at the exact same time is handy. There are situations,
for example, when you need things to happen in a specific timed order and to do so repeatedly. This is possible
by creating your timers, then simply calling one method that resets them all.

You can reset all of your timers by simply calling either the method or the macro:

```C++
resetAllTimers();
RESET_TIMERS;
```

When you call this, it captures a single ```millis()``` reading and a single ```micros()``` reading up front, then loops
through every instantiated timer and resets each one's startTime using whichever of those two captured values matches
that timer's own currently assigned base unit. So a peppered mix of timers, some MILLISECONDS, some MICROSECONDS, some
SECONDS, etc., will each be reset using the correct clock for their own base unit, and every timer sharing a clock
(all your MILLISECONDS/SECONDS/MINUTES timers, and separately all your MICROSECONDS timers) ends up with the exact
same startTime - not just approximately the same, but the identical value, since the clock is only read once per call
rather than re-read per timer.

It should be noted that even if you have your project divided into multiple code files, ```resetAllTimers()```
will reset all timers across all of your code ... it is global to your entire project.

## Time Unit Options

### Default

When you declare your timers without specifying the units, they will default to millisecond timers,
because milliseconds are the most commonly used time units in Arduino programming.

### Other Units

You can declare a timer to operate within any time unit you desire, as long as the time unit you desire is one of
MICROSECONDS, MILLISECONDS, SECONDS, or MINUTES.

For example, when you only need **second** or **minute** precision, you can declare your timer as a SECONDS or MINUTES
timer. It can be much easier in terms of writing and reading your code if you don't need milli or micro second
precision. It's much simpler to use 23 than 23000 when you only need to know about 23 seconds.

You instantiate your timers with other units like this:

```C++
BlockNot myTimer(5, SECONDS);
BlockNot myTimer(3, MINUTES);
BlockNot myTimer(14000, MICROSECONDS);
```

Under the hood, BlockNot calculates SECONDS, MILLISECONDS, and MINUTES timers using the millis() method, whereas
MICROSECOND timers use the micros() method.

#### BlockNot Assumptions

When a timer is declared as a SECONDS, MILLISECONDS, MINUTES, or MICROSECONDS timer, the unit you choose is referred
to as the timer's **base units**.

You MUST interact with your timer in the base units you declared it as, or in the base units you switch it to
(discussed below).

When you read values from your timer, by default, you will always get back a value that is in the
base units of the timer.

For example, these

````C++
BlockNot myTimer(50000, MICROSECONDS);

myTimer.GET_START_TIME;
myTimer.getStartTime();
````

will always return a value in MICROSECONDS.

And

```C++
BlockNot myTimer(50, SECONDS);

myTimer.GET_START_TIME;
myTimer.getStartTime();
```

will always return a value in SECONDS.

It needs to be noted that in a SECONDS or MINUTES timer, the numbers that are returned when seeking a value
will ALWAYS be rounded DOWN. So if, for example, you have a SECONDS timer, and you request the value
for the number of seconds remaining until the next trigger, and BlockNot calculates that value to be
8700 milliseconds, you will get 8 SECONDS back in response.

This shouldn't be a problem, because if you need fractional second accuracy, then use a millisecond timer.

#### Microseconds

MICROSECONDS are almost always used in situations when you need to reference time durations that
are faster than a millisecond. Therefore, NEVER use a MICROSECONDS timer when you need to evaluate
time in durations longer than an hour, because the Arduino rolls the micros() counter over after roughly
70 minutes (read the discussion on rollover below), and BlockNot has no way of knowing how often that counter
rolls over. It can and will calculate durations accurately when a rollover happens in between TRIGGERED
events, but when more than one of these events happens in between TRIGGERED events, BlockNot will not
be able to know about those rollovers and your results will be inaccurate.

Realistically, you should never use a MICROSECONDS timer if your event durations are always longer
than one million microseconds, and certainly you should never use a MICROSECONDS timer for durations
longer than an hour. **If you need to track intervals of time that are longer than an hour,
USE SECONDS, MINUTES, OR MILLISECONDS!**

### Converting Units

Because program storage space is extremely valuable with microcontrollers, I decided to offer the
convert method as opposed to writing methods and macros for every option available where values
of interest might be needed. The convert method will convert from the units that your timer is
declared in, to whichever DESIRED units are passed into the method.

The convert method uses this structure

```C++
unsigned long desiredValue = myTimer.convert(valueOfInterest, UNIT_DESIRED);
```

- The number returned will always be an unsigned long.

- ```valueOfInterest``` MUST be an unsigned long, or a long that is not negative
- ```UNIT_DESIRED``` MUST be one of ```MICROSECONDS```, ```MILLISECONDS```, ```SECONDS```, or ```MINUTES```

For example, let's say that we have declared a timer as a MILLISECONDS timer, but we are interested
in knowing how many SECONDS or MICROSECONDS remain until the timer triggers again. We can get those
values in different units like this:

```C++
value = myTimer.convert(myTimer.REMAINING, SECONDS);
value = myTimer.convert(myTimer.REMAINING, MICROSECONDS);
```

The ```convert()``` method can be used to convert ANY value into whichever units you need, but
realize that the value you pass into the method will be assumed to be in the timer's base units.

Any of these methods can be passed into the convert() method to obtain their values in whichever
unit you desire (desired units in this example were chosen randomly).

```C++
myTimer.convert(myTimer.getTimeUntilTrigger(), SECONDS)
myTimer.convert(myTimer.getNextTriggerTime(), MILLISECONDS)
myTimer.convert(myTimer.getStartTime(), SECONDS)
myTimer.convert(myTimer.getDuration(), MICROSECONDS)
myTimer.convert(myTimer.getTimeSinceLastReset(), MILLISECONDS)
```

Here is what each of these methods provides:

- ```getTimeUntilTrigger()``` returns a value that is relative to the timer's duration. So if the duration is set to
  1350ms, and it has been 500ms since it last triggered, the returned value will be 850ms. It returns 0 once the
  duration has fully elapsed (it never goes negative).
- ```getNextTriggerTime()``` returns a value that is relative to the microcontroller's internal micros() or millis()
  value - it is the timer's current startTime PLUS the timer's duration, i.e. the raw clock reading at which the timer
  is scheduled to trigger next.
- ```getStartTime()``` returns the value of the CPU's micros() or millis() method that was recorded as the timer's
  current startTime
- ```getDuration()``` simply returns the duration that is currently set in the timer (the duration you assign when you
  create the timer, or the one that you changed it to after the fact)
- ```getTimeSinceLastReset()``` returns a value that represents how much time has elapsed since the timer was last
  reset. It does not consider trigger events, only the current startTime.

### Changing Duration

When you need to change a timer's duration, use the ```setDuration(time)``` method. BlockNot assumes that the number
you pass into the argument will be in the same units as the current base unit of the timer. However, if you wish to
change the duration by passing in a value that is in different units, you can use ```setDuration(time, Unit)``` and
BlockNot will convert that number into whatever its current base unit is.

For example, if you have a timer that is declared as a MILLISECONDS timer

```C++
BlockNot myTimer(2500);
```

And you want to change the duration to three seconds, you could do it in two ways:

```c++
myTimer.setDuration(3, SECONDS);
//OR
myTimer.switchTo(SECONDS);
myTimer.setDuration(3);
```

Using the first option will not change the base unit of the timer, so a MILLISECOND timer will remain as a MILLISECOND
timer even though you changed the duration to 3 SECONDS.

Both forms also accept an optional trailing boolean if you don't want the duration change to reset the timer -
```setDuration(3, SECONDS, NO_RESET)``` or ```setDuration(3, NO_RESET)```.

### Switching Base Units

If you need to switch the timer's base units, you can do so like this:

````C++
myTimer.switchTo(MICROSECONDS);
myTimer.switchTo(MILLISECONDS);
myTimer.switchTo(SECONDS);
myTimer.switchTo(MINUTES);
````

Once you have changed the base units, values returned from methods will be returned
in the new base unit, and values given to the timer will be assumed to be in the new base unit
you switched it to.

**Switching between MILLISECONDS, SECONDS, and MINUTES is fully safe** - internally they all share the same
millisecond-based clock, so `switchTo()` between any pair of them is equivalent to having declared the timer in the new
unit to begin with.

**Switching to or from MICROSECONDS is currently not safe.** MICROSECONDS timers are driven internally by a separate
clock (`micros()`) from the other three units (`millis()`). `switchTo()` changes which unit the timer reports values
in, but it does not rescale the timer's already-recorded startTime/duration to the new clock, so crossing that boundary
on an existing timer will leave it comparing values from the wrong clock and produce nonsensical trigger behavior. If
you need a MICROSECONDS timer, declare it as one from the start rather than switching an existing timer into or out of
MICROSECONDS.

## Start / Stop

You can stop a timer, then start it again as needed.

By default, the start() method DOES NOT reset a timer. Calling the method like this is like starting
a stop watch after it has been stopped. The stop watch does not reset the start time to 00:00, but
rather it merely pauses the timer until you start it again. In like manner, calling stop() then start()
merely pauses the timer, so that any time that passes between a stop() and a start() gets subtracted
out ... and the timer's startTime is changed so that the time that passed is added to the startTime, so that
you can pick up right where you left off.

You can override this behavior and have the timer RESET when you start it, by using either of these
options:

```C++
myTimer.START(WITH_RESET);
myTimer.start(WITH_RESET);
```

**Note:** `toggle()` (see below) is not the same as calling `stop()` then `start()`. `toggle()` only flips the timer's
running/stopped flag - it does not capture or compensate for the time that passes while stopped the way the
`stop()`/`start()` pair does. If you toggle a timer off for longer than its remaining duration and then toggle it back
on, it will report as triggered immediately, because its internal clock kept advancing the whole time it was "off." If
you need the pause-and-resume behavior described above, use `stop()`/`start()` rather than `toggle()`.

When a timer is in a stopped state, any call to the timer that would return a boolean value will ALWAYS
return false. And when you query the timer where a numeric value is supposed to be returned,
some methods will return a ZERO by default (although you can change what number they return, as long as the number
you set is a positive number - BlockNot does not ever deal with negative numbers, since time in our universe always
moves forward). Others simply keep returning the timer's real underlying value regardless of whether it's running or
stopped - see the breakdown below.

I've used STOP and START when stepping motors, where the delay between steps is defined in a
MICROSECONDS timer (where the duration is constantly changing based on the value of RPMs), but
when the RPMs are set to 0, then I simply STOP the timer and stepping will not occur. When RPMs
are above 0, then I START the timer and stepping resumes.

These methods will ALWAYS return false when a timer is stopped (for macro calls see the Macro
section of this document):

* **triggered()**
* **notTriggered()**
* **firstTrigger()**

These methods will return a ZERO by default when a timer is stopped (or whichever value you set as the return, via
`setStoppedReturnValue()`):

* **getDuration()**
* **getTimeUntilTrigger()**
* **getTimeSinceLastReset()**

Every other getter (`getStartTime()`, `getStartTime(units)`, `getNextTriggerTime()`, `getMillis()`, `getBaseUnits()`,
`lastTriggerDuration()`, `convert()`) is unaffected by the running/stopped state - it always reports the timer's real
underlying value either way.

### Return Values on Stopped Timers

You can declare the return value when you create the timer (see the constructors in the .h file), OR,
once you create your timer, you simply set the value using this method:

```C++
setStoppedReturnValue(8675309);
```

What matters in this situation is that the default return value for any stopped timer is always ZERO unless you change
it, and the number you assign, once again, CANNOT BE NEGATIVE.

You can start and stop a timer using these methods / macros.

```C++  
myTimer.START;  
myTimer.STOP;  
```  

And you can find out if the timer is running or not using either of these calls:

```C++  
if (myTimer.ISRUNNING) { my code; }  
if (myTimer.ISSTOPPED) { my code; }  
```  

You can also flip the state of the timer (if stopped, it will start; if started, it will stop):

```C++  
myTimer.TOGGLE;  
```  

Why would you want to just change the state with one line of code? Perhaps you have a toggle button that will toggle a
timer to be started or stopped ... you can assign the one command to the button and everything is handled.

```C++
#define BUTTON_PRESSED digitalRead(BUTTON) == LOW

pinMode(BUTTON, INPUT_PULLUP);

if (BUTTON_PRESSED) {  
   myTimer.TOGGLE;
 }  
```  

## Summary

Well, that's BlockNot in a nutshell.

Simple, right?

BlockNot is a library intended to make the employment of non-blocking timers easy,
intuitive, natural, and obvious. It can be engaged with simple single word macros
or by calling the methods directly.

There are more methods that allow you to affect change on your timers after instantiation, and also methods to get info
about your timers. You can change the duration of an existing timer in different ways, you can reset the timer, or
you can even find out how much time is left before the trigger event occurs, or find out how much time has passed since
the timer last triggered.

# Examples

There are currently nine examples in the library.

### Advanced Auto Flashers

This sketch was one I wrote recently that was for a friend who wanted to put large LEDs on the back of his 5th wheel so
that people behind him received much better feedback depending on whether he hits his brakes, uses his turn signals, or
uses the hazard lights. It's a fairly good example of using BlockNot timers to achieve compartmentalized functions in
code that runs continuously and never stops. The code was written for a Raspberry Pi Pico.

### BlockNot Blink

This sketch does the same thing as the famous blink sketch, only it does it with BlockNot elegance and style.

### BlockNot Blink Party

If you have a nano or an uno or equivalent laying around, and four LEDs and some resistors, connect them to pins 9 - 12
and run this sketch. You will immediately see the benefit of non-blocking timers. You could never write a sketch that
could do the same thing using the delay() command. It would be impossible.

**Non-Blocking MATTERS!**

### Millis Rollover Test

This sketch was added to demonstrate that BlockNot can and does properly calculate
timer durations even when millis() rolls over. The sketch has comments at the top that
fully explain what it does, and how you can adjust the time until millis() rolls using the
terminal. See [the discussion](#rollover) further down on millis() and micros() rollover.

### Button Debounce

Learn how to debounce a button without using delay()

### Duration Trigger

Read the section above to get an idea of what TRIGGERED_ON_DURATION does, then load this example up and play around
with it. You can pause the loop from the Serial Terminal Monitor by typing in p and hitting enter. Then, if you wait
for several durations to pass and then un-pause the loop, you will see how BlockNot handles that feature.

### On With Off Timers

This example shows you how to use on and off timers to control anything that you need
to have on for a certain length of time and also off for a certain length of time.

The example specifically blinks two LEDs such that they will always be in sync every
6 seconds ... by this pattern:

### Reset All

This sketch shows how all BlockNot timers defined in your sketch can be reset with a
single line of code, rather than having to call reset() for each and every one
separately. This comes in handy when all timers need to be reset at once, e.g. after
the system clock has been adjusted from an external source (NTP or RTC, for example).

### Timers Rules

This sketch has SIX timers created and running at the same time. There are various
things happening at the trigger event of each timer. The expected behavior is explained
in the output Strings to Serial. Read them, then let it run for a minute or so, then stop
your Serial monitor and look at the output. You should be able to look at the number of
milliseconds that is given in each output, and compare the differences with the
expected behavior, and see that everything runs as it is expected to run.

For example, when LiteTimer triggers, you should soon after that see the output from
stopAfterThreeTimer. When you look at the number of milliseconds in each of their
outputs, you can see that indeed it does trigger three seconds after being reset,
but then it does not re-trigger until after it is reset again.

- Thanks to [@SteveRMann](https://github.com/SteveRMann) for kick-starting this example and working with me on
  fine-tuning it.

#### These examples barely scratch the surface of what you can accomplish with BlockNot.

# Library

## Methods

Below you will find the name of each method in the library and any arguments that it accepts. Below that list, you will
find the names of the macros that are connected to each method, along with the arguments that a given macro may or may
not pass to the method. The macros are key to making your code simple.

**For any method call that resets a timer by default, the resetting behavior can be overridden by passing `NO_RESET`
into the method's argument. The exception to this is the `triggeredOnDuration()` method, which exists because of the
way it resets your timer - overriding reset would make the method useless.**

* **setDuration(time, resetOption = WITH_RESET)** - Overrides the current timer duration and sets it to a new value,
  assumed to be in the timer's own base unit. This also resets the timer. If you need the timer to NOT reset, pass
  arguments like this: `(newDuration, NO_RESET)`.
* **setDuration(time, units, resetOption = WITH_RESET)** - Same as above, but the value you pass is assumed to be in
  `units` and gets converted into the timer's base unit before being stored.
* **addTime(time, resetOption = NO_RESET)** - Adds the time you pass into the argument to the current duration value.
  This does NOT reset the timer by default. To also reset the timer, call the method like this:
  **addTime(newTime, WITH_RESET)**.
* **takeTime(time, resetOption = NO_RESET)** - The opposite effect of addTime(); subtracts from the duration, clamped
  at 0 (it will never go negative). Same deal if you want to also reset the timer.
* **triggered(resetOption = true)** - Returns true if the duration time has passed. Also resets the timer to the
  current ```micros()``` or ```millis()``` (override by passing NO_RESET as an argument).
* **triggeredOnDuration(allMissed = false)** - See the section above entitled **Triggered OnDuration** for a complete
  discussion.
* **notTriggered()** - Returns true if the trigger event has not happened yet.
* **firstTrigger()** - Returns true only once, and only after the timer has triggered - can be modified with
  setFirstTriggerResponse(bool).
* **triggerNext()** - Signals the timer to report as triggered the very next time it's checked, regardless of whether
  its duration has actually elapsed. See the **Trigger Next** section above.
* **setFirstTriggerResponse(bool)** - Changes what `firstTrigger()` returns on the calls *after* its one true response,
  from the default `false` to whatever you pass in. See **One Time Trigger** above.
* **getNextTriggerTime()** - Returns an unsigned long that is the raw micros()/millis() clock reading at which the
  timer is scheduled to trigger next (its startTime plus its duration), converted to the timer's base unit.
* **getTimeUntilTrigger()** - Returns an unsigned long with the amount of time remaining until the trigger event
  happens, in the timer's base units. Returns 0 once the duration has fully elapsed.
* **getStartTime()** - Returns an unsigned long: the value of ```micros()``` or ```millis()``` that was recorded at the
  last reset of the timer, converted to the timer's currently assigned base unit.
* **getStartTime(units)** - Same as above, but converts directly to `units` instead of the timer's own base unit.
* **getDuration()** - Returns an unsigned long: the duration that is currently set in the timer (or the stopped-return
  value if the timer is stopped).
* **lastTriggerDuration()** - Returns an unsigned long: how much time had actually elapsed the last time the timer
  triggered. See **Last Trigger Duration** above.
* **getUnits()** - Returns a String naming the assigned base units of the timer: "Seconds", "Minutes",
  "Milliseconds", or "Microseconds".
* **getTimeSinceLastReset()** - Returns an unsigned long indicating how much time has passed since the timer was last
  reset or instantiated (or the stopped-return value if the timer is stopped). Response will be in the base units of
  the timer.
* **setStoppedReturnValue(value)** - Lets you set the value returned for those methods that return numbers, when the
  timer is stopped.
* **start(resetOption = NO_RESET)** - starts the timer (timers are started by default when you create them). Does NOT
  reset the timer by default; pass `WITH_RESET` to also reset it. See **Start / Stop** above.
* **stop()** - stops the timer.
* **isRunning()** - returns true if the timer is currently running.
* **isStopped()** - returns true if the timer is stopped.
* **toggle()** - Flips the running/stopped state, so that you only need to call this one method - like in a push
  button toggle situation. Note this is not equivalent to `stop()`/`start()` - see the caution note under
  **Start / Stop** above.
* **convert(value, units)** - Converts `value` (assumed to be in the timer's own base units) into `units` and returns
  the result. See **Converting Units** above.
* **switchTo(units)** - Change the timer from whichever base unit it currently is over to MICROSECONDS, MILLISECONDS,
  SECONDS, or MINUTES. See the caution about MICROSECONDS in **Switching Base Units** above.
* **reset(newStartTime = 0)** - Sets the start time of the timer to the current micros() or millis(), depending on its
  currently assigned base unit. You can optionally pass an explicit raw clock value instead of letting it capture the
  current one - this is how `resetAllTimers()` and `triggeredOnDuration()` reset timers internally, and you can use it
  directly too, e.g. to align a timer to a timestamp obtained from an RTC or NTP sync.
* **setMillisOffset(offset = 0)** - Adds a fixed offset to every millis()-based reading this timer makes (affects
  MILLISECONDS/SECONDS/MINUTES timers only). Changing the offset immediately adjusts startTime to compensate, so it
  takes effect without causing a jump in the timer's reported elapsed time.
* **setMicrosOffset(offset = 0)** - Same as above, but for MICROSECONDS timers.
* **speedComp(time)** - Enables a compensating delay (in milliseconds) on every reset, to work around spurious rapid
  re-triggers on very fast microcontrollers. See **Triggering Too Fast With High Speed Microcontrollers** below.
* **disableSpeedComp()** - Disables the delay enabled by `speedComp()`.
* **getMillis()** - Returns `millis()` plus whatever offset was set via `setMillisOffset()`. A general-purpose utility,
  independent of any particular timer's trigger state.
* **getBaseUnits()** - Returns the timer's currently assigned base unit (`MICROSECONDS`, `MILLISECONDS`, `SECONDS`, or
  `MINUTES`).
* **resetAllTimers()** - loops through all timers that you created and resets each one's startTime to a freshly
  captured ```micros()``` or ```millis()```, depending on that timer's own currently assigned base unit. See
  **Global Reset** above and the **Memory** section below for further discussion.

## Macros

Here are the macro terms and the methods that they call, along with any arguments they pass into the method:

| **Macro**                     | **Method**                |
|--------------------------------|---------------------------|
| **ELAPSED**                    | getTimeSinceLastReset()   |
| **REMAINING**                  | getTimeUntilTrigger()     |
| **DURATION**                   | getDuration()             |
| **GET_UNITS**                  | getUnits()                |
| **GET_START_TIME**             | getStartTime()            |
| **TRIGGERED**                  | triggered()               |
| **LAST_TRIGGER_DURATION**      | lastTriggerDuration()     |
| **HAS_TRIGGERED**              | triggered(NO_RESET)       |
| **TRIGGER_NEXT**                | triggerNext()             |
| **TRIGGERED_ON_DURATION**       | triggeredOnDuration()     |
| **TRIGGERED_ON_DURATION(ALL)**  | triggeredOnDuration(ALL)  |
| **NOT_TRIGGERED**              | notTriggered()            |
| **FIRST_TRIGGER**              | firstTrigger()            |
| **RESET**                      | reset()                   |
| **RESET_TIMERS**               | resetAllTimers()          |
| **START**                      | start()                   |
| **START(WITH_RESET)**          | start(WITH_RESET)         |
| **STOP**                       | stop()                    |
| **ISRUNNING**                  | isRunning()               |
| **ISSTOPPED**                  | isStopped()               |
| **TOGGLE**                     | toggle()                  |

## Constants

* **WITH_RESET** - boolean true
* **NO_RESET**   - boolean false
* **ALL**        - boolean true
* **MICROSECONDS**
* **MILLISECONDS**
* **SECONDS**
* **MINUTES**
* **NO_GLOBAL_RESET**
* **GLOBAL_RESET**
* **RUNNING**
* **STOPPED** (Pass this into a constructor to create a timer in a STOPPED state)

If you can think of MACRO names that would make the reading and writing of your code more
natural, and you think it would be a benefit to BlockNot, PLEASE either submit a pull
request or shoot me an email so that we can all work together to make this library the
best that it can possibly be.

Also, you can, of course, create your own macros within your code. So, for example, let's
say that you wanted a macro that overrides the default reset behavior in the setDuration()
method, which by default will change the duration of the timer to your new value and will
also reset the timer. But let's say you want to change the duration WITHOUT resetting the
timer, and you wanted that to be done with a word that makes more sense to you.

```C++  
#define QUICK_CHANGE(value) myTimer.setDuration(value, false)  
  
QUICK_CHANGE(3200);  
```  

The only difference here is that you cannot make a macro that applies universally to all
of your timers. You would need to make one macro for each timer you have created. This is
why it is better to submit a pull request or contact me with your ideas, so that all of us
who use BlockNot can benefit through continual improvement of the library.

# Discussion

## Memory

I have compiled BlockNot in a variety of scenarios. The only difference between each specific scenario is that I
would use traditional methods of implementing non-blocking timers, vs using BlockNot. In some scenarios, BlockNot
would cause the sketch to compile using less memory, and in some scenarios, it would use a little more memory.
Obviously your situation will be different depending on the size of your project, other libraries used, etc.

If you're really struggling for memory space, try creating your timers using the manual method just to see if it
makes a difference or not vs using BlockNot.

In the interest of squeezing as much program space as possible, I have added the ability to disable BlockNot's
global reset option, because it, by default, maintains a linked list of all instantiations of BlockNot, which consumes
a little extra memory. If you don't need that feature and are hurting for memory space, you can disable
it by passing NO_GLOBAL_RESET as the last argument into your first timer (you only need to pass that argument ONCE
and it will remain disabled for all timers created after that, as long as none of them explicitly pass GLOBAL_RESET).

Examples of how to disable the feature:

````C++
BlockNot myTimer(3800, NO_GLOBAL_RESET);
BlockNot myTimer(15, SECONDS, NO_GLOBAL_RESET);
````

OR, you can optionally define some timers that are affected by the global reset method, then issue the NO_GLOBAL_RESET
argument into the next timer you create, and that timer, and every timer created after it, will not be included in the
global reset option.

````C++
BlockNot timer1(1350);
BlockNot timer2(5, SECONDS);
BlockNot timer3(2670, NO_GLOBAL_RESET);
BlockNot timer4(3460); //not included in global reset
````

## Rollover

I've been contacted by a few people who have expressed concern about possible problems in timing when the
Arduino ```millis()``` or ```micros()``` counter rolls over (millis() at approximately 50 days and micros() at around
70 minutes) after power up.

First and foremost, **DON'T USE MICROSECOND TIMERS WHEN YOU CAN USE MILLISECONDS INSTEAD**

The whole issue about rollover **is not a concern at all**, because
of the way that BlockNot uses ```micros()``` and ```millis()```, your timers will still calculate properly even if the
```micros()``` or ```millis()``` value rolls over during the duration of a timer. I've tested BlockNot using simulated
values to artificially create a rollover scenario, and I can tell you that it indeed works properly through a
rollover.

The reason it works has to do with the way CPUs handle binary numbers, where there is no possibility of the number
being negative, and [this article](https://techexplorations.com/guides/arduino/programming/millis-rollover/) can
explain it in detail if you're interested.

I have added an example sketch called ```MillisRolloverTest.ino``` that will demonstrate how well
BlockNot calculates timer durations even through millis() rollovers, by artificially inflating the
value of millis() and calculating the time difference between trigger events. There is more
discussion in that sketch.

## Thread Safety

With the introduction of cost-effective multi-core microcontrollers, more and more people will be
writing code where they take advantage of having more than one core in the CPU. Currently, the
way that most microcontrollers implement the use of another core is by adding a separate code thread,
where the code for that core runs in a different thread.

One of the major problems with multi-threading applications is when code from different threads
tries to change the value of a shared variable simultaneously.

Where BlockNot is concerned, that could be an issue if you make references to a single timer from
different threads, because, for example, when you check for TRIGGERED and the return value is true,
BlockNot updates the timer's startTime so that TRIGGERED will only return
true again after the next duration has passed.

**BlockNot's internal state is not declared `volatile` and the class makes no locking or atomicity guarantees, so it is
not thread-safe on its own.** If you need to access or engage a timer from two different threads, what is most
important is that only one thread makes any calls to the timer that cause changes to its internal state. This
includes the TRIGGERED method. There is a way to accomplish the modification of a timer from two different threads, by
utilizing a variable where only one thread writes to it and the other thread reads from it.

Consider this example. Specifically, look at the ```adjustTimer()``` method and the ```core1Entry``` method, which is
what is executing in the other core - or is what is running on the other thread - however you want to
look at it (six of one, half-dozen of the other).

What is important to see here is that one thread is making changes to ```timerDelay```, while the
other thread is taking that value and passing it into the timer's ```setDuration``` method. That same
thread is also making calls to TRIGGERED, which leaves only that thread as the thread causing changes
to the timer's internal state.

```C++
#include <BlockNot.h>

BlockNot stepperTimer(1, MICROSECONDS);
unsigned long timerDelay = 0;


void stepStepper() {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(3);
    digitalWrite(STEP, LOW);
    delayMicroseconds(2);
}


[[noreturn]] void core1Entry() {
    static unsigned long lastTimerDelay = 0;
    while (true) {
        if(lastTimerDelay != timerDelay) {
            stepperTimer.setDuration(timerDelay, NO_RESET);
            lastTimerDelay = timerDelay;
        }
        if (stepperTimer.TRIGGERED)
            stepStepper();
    }
}

void adjustTimer() {
    long potValue = analogRead(POT_PIN);
    timerDelay = map(potValue, 0, 1024, 5000, 25);
}

void setup() {
    multicore_launch_core1(core1Entry);
}

void loop() {
    adjustTimer();
}
```

Even though BlockNot is not "thread-safe," you can still use it in multi-threaded environments if you
simply make sure that only one thread will ever be causing changes to happen in the timer itself.

## Triggering Too Fast With High Speed Microcontrollers

If you're noticing that some timers seem to trigger immediately after a trigger or a reset, and you're running
on a high speed microcontroller like a Pi Pico, you can enable a feature called `speedComp()` and pass in an
amount of time for a delay during each reset. When I noticed the problem, I had a switch statement in my loop
that would run each time the timer triggered, and I had three cases in the switch. In this instance, each case
that hit would set the next trigger to run the next case. Each case displayed something different on an OLED
screen. However, I was noticing that the information was not being shown in the right order - instead I would see a
quick flash of something, then it would go to the next case after the one that was supposed to be next.

What fixed it for me was adding a 5ms delay after each triggering of the timer, so I added this feature, and you
can use it like this:

```c++
myTimer.speedComp(5);
```

Put that in your setup() code, as it only needs to be executed one time. That will automatically implement a
delay (of 5 milliseconds in this example) every time the timer is reset. And the timer is automatically reset
every time it triggers by default.

If you need to disable this feature:

```c++
myTimer.disableSpeedComp();
```

## Deprecated Macros

A handful of older macro names have been deprecated in favor of clearer, more standardized replacements. If your
sketch uses one of them, the compiler will emit a `[[deprecated]]` warning at that exact line, naming the
replacement macro to switch to - so there's no need to memorize the list here. See [CHANGELOG.md](CHANGELOG.md) for
the full deprecation history, including which release each old macro is scheduled to be removed in.

# Version Update Notes

## Changelog

For the full version history, see the [CHANGELOG.md](CHANGELOG.md) file.

# Suggestions

I welcome any and all suggestions for changes or improvements. You can either open an issue, or code the change yourself
and create a pull request. This library is for all of us, and making it the best it can be is important!

You can also email me<BR>[sims.mike@gmail.com](mailto:sims.mike@gmail.com)

Thank you for your interest in BlockNot. I hope you find it as invaluable in your projects as I have in mine.
