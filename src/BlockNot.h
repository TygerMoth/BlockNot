/**
 * BlockNot is a simple and easy to use Arduino class for the implementation
 * of non-blocking timers, as it is far better to use non-blocking timers
 * with a micro-controller since they allow you to trigger code at defined
 * durations of time, without stopping the execution of your main loop.
 *
 * Written by - Michael Sims
 * Full documentation can be found at: https://github.com/EasyG0ing1/BlockNot
 *
 * See LICENSE file for acceptable use conditions - this is open source
 * and there are no restrictions on its usage, I simply ask for some acknowledgment
 * if it is used in your project.
 */
#ifndef BlockNot_h
#define BlockNot_h

#include <Arduino.h>

#pragma once

/**
 * Macros - their usage and significance is described in README.md
 */

enum BlockNotUnit : uint8_t {
    mic_cTime, mil_cTime, sec_cTime, min_cTime
};
enum BlockNotGlobal : uint8_t {
    yes, no
};
enum BlockNotState : uint8_t {
    running, stopped
};

#define WITH_RESET true
#define NO_RESET   false
#define ALL        true
#define MICROSECONDS            BlockNotUnit::mic_cTime
#define MILLISECONDS            BlockNotUnit::mil_cTime
#define SECONDS                 BlockNotUnit::sec_cTime
#define MINUTES                 BlockNotUnit::min_cTime
#define NO_GLOBAL_RESET         BlockNotGlobal::no
#define GLOBAL_RESET            BlockNotGlobal::yes
#define RUNNING                 BlockNotState::running
#define STOPPED                 BlockNotState::stopped

#define ELAPSED                     getTimeSinceLastReset()
#define REMAINING                   getTimeUntilTrigger()
#define DURATION                    getDuration()
#define GET_UNITS                   getUnits()
#define GET_START_TIME              getStartTime()

/**
 * Deprecated macros. Each aliases to a tiny [[deprecated]]-attributed wrapper method
 * (defined on BlockNot below) instead of calling the warning directly here, so that the
 * compiler only warns when a sketch actually uses one of these macros - not on every
 * translation unit that merely includes this header. [[deprecated]] is standard C++14 and
 * understood identically by GCC, Clang and MSVC, so no compiler-specific branching is needed.
 */
#define DONE                        deprecatedDone()
#define TIME_PASSED                 deprecatedTimePassed()
#define TIME_SINCE_RESET            deprecatedTimeSinceReset()
#define TIME_TILL_TRIGGER           deprecatedTimeTillTrigger()
#define TRIGGERED_ON_MARK           deprecatedTriggeredOnMark()
#define NOT_DONE                    deprecatedNotDone()
#define ISSTARTED                   deprecatedIsStarted()
#define TRIGGERED_ON_DURATION_ALL   deprecatedTriggeredOnDurationAll()
#define TRIGGERED_ALL               deprecatedTriggeredAll()
#define START_RESET                 deprecatedStartReset()

#define TRIGGERED                   triggered()
#define LAST_TRIGGER_DURATION       lastTriggerDuration()
#define HAS_TRIGGERED               triggered(NO_RESET)
#define TRIGGER_NEXT                triggerNext()
#define TRIGGERED_ON_DURATION(...)  triggeredOnDuration(__VA_ARGS__)
#define NOT_TRIGGERED               notTriggered()
#define FIRST_TRIGGER               firstTrigger()
#define RESET                       reset()
#define RESET_TIMERS                resetAllTimers()
#define START(...)                  start(__VA_ARGS__)
#define STOP                        stop()
#define ISRUNNING                   isRunning()
#define ISSTOPPED                   isStopped()
#define TOGGLE                      toggle()

class BlockNot {
    friend void resetAllTimers(unsigned long newStartTime);

public:
    /**
     * Constructors
     */
    BlockNot();

    explicit BlockNot(unsigned long milliseconds);

    BlockNot(unsigned long milliseconds, BlockNotState state);

    BlockNot(unsigned long time, BlockNotUnit units);

    BlockNot(unsigned long time, BlockNotUnit units, BlockNotState state);

    BlockNot(unsigned long milliseconds, BlockNotGlobal globalReset);

    BlockNot(unsigned long milliseconds, BlockNotState state, BlockNotGlobal globalReset);

    BlockNot(unsigned long time, BlockNotUnit units, BlockNotGlobal globalReset);

    BlockNot(unsigned long time, BlockNotUnit units, BlockNotState state, BlockNotGlobal globalReset);

    BlockNot(unsigned long milliseconds, unsigned long stoppedReturnValue);

    BlockNot(unsigned long milliseconds, unsigned long stoppedReturnValue, BlockNotState state);

    BlockNot(unsigned long time, unsigned long stoppedReturnValue, BlockNotUnit units);

    BlockNot(unsigned long time, unsigned long stoppedReturnValue, BlockNotUnit units, BlockNotState state);

    BlockNot(unsigned long milliseconds, unsigned long stoppedReturnValue, BlockNotGlobal globalReset);

    BlockNot(unsigned long milliseconds, unsigned long stoppedReturnValue, BlockNotGlobal globalReset,
             BlockNotState state);

    BlockNot(unsigned long time, unsigned long stoppedReturnValue, BlockNotUnit units, BlockNotGlobal globalReset);

    BlockNot(unsigned long time, unsigned long stoppedReturnValue, BlockNotUnit units, BlockNotGlobal globalReset,
             BlockNotState state);

    /**
     * Public Methods
     */

    void setDuration(unsigned long time, bool resetOption = WITH_RESET);

    void setDuration(unsigned long time, BlockNotUnit units, bool resetOption = WITH_RESET);

    void addTime(unsigned long time, bool resetOption = NO_RESET);

    void takeTime(unsigned long time, bool resetOption = NO_RESET);

    bool triggered(bool resetOption = true);

    bool triggeredOnDuration(bool allMissed = false);

    bool notTriggered();

    bool firstTrigger();

    void triggerNext();

    void setFirstTriggerResponse(bool response);

    unsigned long getNextTriggerTime() const;

    unsigned long getTimeUntilTrigger() const;

    unsigned long getStartTime() const;

    unsigned long getStartTime(BlockNotUnit units) const;

    unsigned long getDuration() const;

    unsigned long lastTriggerDuration() const;

    String getUnits() const;

    unsigned long getTimeSinceLastReset() const;

    void setStoppedReturnValue(unsigned long stoppedReturnValue);

    void start(bool resetOption = NO_RESET);

    void stop();

    bool isRunning() const;

    bool isStopped() const;

    void toggle();

    unsigned long convert(unsigned long value, BlockNotUnit units) const;

    void switchTo(BlockNotUnit units);

    void reset(unsigned long newStartTime = 0);

    void setMillisOffset(unsigned long offset = 0);

    void setMicrosOffset(unsigned long offset = 0);

    void speedComp(unsigned long time);

    void disableSpeedComp();

    unsigned long getMillis() const;

    BlockNotUnit getBaseUnits() const;

    /**
     * Deprecated macro shims - see the macro definitions above. Not intended to be called
     * directly; use the replacement named in each deprecation message instead.
     */
    [[deprecated("DONE is deprecated and will be removed in a future release. Use TRIGGERED instead.")]]
    bool deprecatedDone() { return triggered(); }

    [[deprecated("TIME_PASSED is deprecated and will be removed in a future release. Use ELAPSED instead.")]]
    unsigned long deprecatedTimePassed() { return getTimeSinceLastReset(); }

    [[deprecated("TIME_SINCE_RESET is deprecated and will be removed in a future release. Use ELAPSED instead.")]]
    unsigned long deprecatedTimeSinceReset() { return getTimeSinceLastReset(); }

    [[deprecated("TIME_TILL_TRIGGER is deprecated and will be removed in a future release. Use REMAINING instead.")]]
    unsigned long deprecatedTimeTillTrigger() { return getTimeUntilTrigger(); }

    [[deprecated("TRIGGERED_ON_MARK is deprecated and will be removed in a future release. Use TRIGGERED_ON_DURATION instead.")]]
    bool deprecatedTriggeredOnMark() { return triggeredOnDuration(); }

    [[deprecated("NOT_DONE is deprecated and will be removed in a future release. Use NOT_TRIGGERED instead.")]]
    bool deprecatedNotDone() { return notTriggered(); }

    [[deprecated("ISSTARTED is deprecated and will be removed in a future release. Use ISRUNNING instead.")]]
    bool deprecatedIsStarted() const { return isRunning(); }

    [[deprecated("TRIGGERED_ON_DURATION_ALL is deprecated and will be removed in a future release. Use TRIGGERED_ON_DURATION(ALL) instead.")]]
    bool deprecatedTriggeredOnDurationAll() { return triggeredOnDuration(ALL); }

    [[deprecated("TRIGGERED_ALL is deprecated and will be removed in a future release. Use TRIGGERED_ON_DURATION(ALL) instead.")]]
    bool deprecatedTriggeredAll() { return triggeredOnDuration(ALL); }

    [[deprecated("START_RESET is deprecated and will be removed in a future release. Use START(WITH_RESET) instead.")]]
    void deprecatedStartReset() { start(WITH_RESET); }

    static BlockNot *firstTimer;
    static BlockNot *currentTimer;
    BlockNot *nextTimer;

private:
    /**
     * Private Variables and Methods
     */
    /**
     * 4-byte fields are grouped together, and the six 1-byte fields (four bools plus the two
     * fixed-width enums below) are grouped together after them. This keeps struct padding to a
     * single trailing gap instead of one padding gap per 1-byte field interleaved among 4-byte
     * fields - on 32-bit targets where enums aren't packed to 1 byte by default (e.g. ESP32/RP2040),
     * interleaving would otherwise silently erase the RAM savings from giving BlockNotUnit/
     * BlockNotState an explicit uint8_t underlying type.
     */
    unsigned long startTime = 0;
    unsigned long millisOffset = 0;
    unsigned long microsOffset = 0;
    unsigned long timerStoppedReturnValue = 0;
    unsigned long lastDuration = 0;
    unsigned long compTime = 0;
    unsigned long duration = 0;
    unsigned long stopTime = 0;
    int totalMissedDurations = 0;

    static BlockNotGlobal global;
    bool onceTriggered = false;
    bool triggerOnNext = false;
    bool firstTriggerResponse = false;
    bool speedCompensation = false;
    BlockNotUnit baseUnits;
    BlockNotState timerState;

    void init(unsigned long time, BlockNotUnit units, BlockNotState state,
              bool hasStoppedReturnValue, unsigned long stoppedReturnValue,
              bool hasGlobalParam, BlockNotGlobal globalReset);

    void resetTimer(unsigned long newStartTime);

    void resetToCapturedTime(unsigned long capturedMillis, unsigned long capturedMicros);

    void initDuration(unsigned long time);

    void initDuration(unsigned long time, BlockNotUnit desiredUnits);

    unsigned long timeSinceReset() const;

    bool hasTriggered();

    void addToTimerList();

    unsigned long timeTillTrigger() const;

    /**
     * duration/startTime/stopTime are always stored in "native units" - the same unit
     * that drives the timer's own clock (microseconds for a MICROSECONDS timer, milliseconds
     * for everything else, matching the micros()/millis() split used throughout). nativeUnit()
     * and convertValue() replace the old cTime double-backed representation with plain integer
     * arithmetic - every ratio between MICROSECONDS/MILLISECONDS/SECONDS/MINUTES is an exact
     * integer, so no floating point is needed anywhere in the class.
     */
    BlockNotUnit nativeUnit() const;

    static unsigned long convertValue(unsigned long value, BlockNotUnit fromUnits, BlockNotUnit toUnits);

    unsigned long convertUnits(unsigned long nativeValue) const;
};

/**
 * Global methods affecting all instances of the BlockNot class.
 */
void resetAllTimers(unsigned long newStartTime = 0);

#endif
