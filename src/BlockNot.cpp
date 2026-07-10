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

#include <BlockNot.h>

/**
 * Global Variables
 */

BlockNot *BlockNot::firstTimer   = nullptr;
BlockNot *BlockNot::currentTimer = nullptr;
BlockNotGlobal BlockNot::global  = GLOBAL_RESET;

/**
 * Constructors
 */

BlockNot::BlockNot() {
    baseUnits  = MILLISECONDS;
    timerState = RUNNING;
    global     = (global == NO_GLOBAL_RESET) ? NO_GLOBAL_RESET : GLOBAL_RESET;
    if (global == GLOBAL_RESET) addToTimerList();
}

BlockNot::BlockNot(const unsigned long milliseconds) {
    init(milliseconds, MILLISECONDS, RUNNING, false, 0, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long milliseconds, const BlockNotState state) {
    init(milliseconds, MILLISECONDS, state, false, 0, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long time, const BlockNotUnit units = MILLISECONDS) {
    init(time, units, RUNNING, false, 0, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long time, const BlockNotUnit units, const BlockNotState state) {
    init(time, units, state, false, 0, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long milliseconds, const BlockNotGlobal globalReset) {
    init(milliseconds, MILLISECONDS, RUNNING, false, 0, true, globalReset);
}

BlockNot::BlockNot(const unsigned long milliseconds, const BlockNotState state, const BlockNotGlobal globalReset) {
    init(milliseconds, MILLISECONDS, state, false, 0, true, globalReset);
}

BlockNot::BlockNot(const unsigned long time, const BlockNotUnit units, const BlockNotGlobal globalReset) {
    init(time, units, RUNNING, false, 0, true, globalReset);
}

BlockNot::BlockNot(const unsigned long time, const BlockNotUnit units, const BlockNotState state, const BlockNotGlobal globalReset) {
    init(time, units, state, false, 0, true, globalReset);
}

BlockNot::BlockNot(const unsigned long milliseconds, const unsigned long stoppedReturnValue) {
    init(milliseconds, MILLISECONDS, RUNNING, true, stoppedReturnValue, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long milliseconds, const unsigned long stoppedReturnValue, const BlockNotState state) {
    init(milliseconds, MILLISECONDS, state, true, stoppedReturnValue, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long time, const unsigned long stoppedReturnValue, const BlockNotUnit units) {
    init(time, units, RUNNING, true, stoppedReturnValue, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long time, const unsigned long stoppedReturnValue, const BlockNotUnit units, const BlockNotState state) {
    init(time, units, state, true, stoppedReturnValue, false, GLOBAL_RESET);
}

BlockNot::BlockNot(const unsigned long milliseconds, const unsigned long stoppedReturnValue, const BlockNotGlobal globalReset) {
    init(milliseconds, MILLISECONDS, RUNNING, true, stoppedReturnValue, true, globalReset);
}

BlockNot::BlockNot(const unsigned long milliseconds, const unsigned long stoppedReturnValue, const BlockNotGlobal globalReset, const BlockNotState state) {
    init(milliseconds, MILLISECONDS, state, true, stoppedReturnValue, true, globalReset);
}

BlockNot::BlockNot(const unsigned long time, const unsigned long stoppedReturnValue, const BlockNotUnit units, const BlockNotGlobal globalReset) {
    init(time, units, RUNNING, true, stoppedReturnValue, true, globalReset);
}

BlockNot::BlockNot(const unsigned long time, const unsigned long stoppedReturnValue, const BlockNotUnit units, const BlockNotGlobal globalReset, const BlockNotState state) {
    init(time, units, state, true, stoppedReturnValue, true, globalReset);
}

void BlockNot::init(const unsigned long time, const BlockNotUnit units, const BlockNotState state,
                    const bool hasStoppedReturnValue, const unsigned long stoppedReturnValue,
                    const bool hasGlobalParam, const BlockNotGlobal globalReset) {
    baseUnits  = units;
    timerState = state;
    global     = hasGlobalParam ? globalReset : ((global == NO_GLOBAL_RESET) ? NO_GLOBAL_RESET : GLOBAL_RESET);
    if (timerState == STOPPED) stop();
    initDuration(time);
    if (hasStoppedReturnValue) timerStoppedReturnValue = stoppedReturnValue;
    reset();
    if (global == GLOBAL_RESET) addToTimerList();
}

/**
 * Public Methods
 */

void BlockNot::setDuration(const unsigned long time, const bool resetOption) {
    initDuration(time);
    if (resetOption) reset();
}

void BlockNot::setDuration(const unsigned long time, BlockNotUnit inUnits, const bool resetOption) {
    initDuration(time, inUnits);
    if (resetOption) reset();
}

void BlockNot::addTime(const unsigned long time, const bool resetOption) {
    unsigned long newDuration = duration + time;
    if (newDuration < duration) newDuration = 0xFFFFFFFFUL;
    duration = newDuration;
    if (resetOption) reset();
}

void BlockNot::takeTime(const unsigned long time, const bool resetOption) {
    long newDuration = static_cast<long>(duration) - static_cast<long>(time);
    if (newDuration < 0) newDuration = 0L;
    duration = static_cast<unsigned long>(newDuration);
    if (resetOption) reset();
}

bool BlockNot::triggered(const bool resetOption) {
    const bool triggered = hasTriggered();
    if (resetOption && triggered) {
        reset();
    }
    return timerState == RUNNING && triggered;
}

bool BlockNot::triggeredOnDuration(const bool allMissed) {
    const bool triggered = hasTriggered();
    if (triggered) {
        const unsigned long missedDurations = timeSinceReset() / duration;
        totalMissedDurations                += (allMissed ? missedDurations : 0);
        reset(startTime + (missedDurations * duration));
    }
    if (totalMissedDurations > 0 && allMissed) {
        totalMissedDurations--;
        return true;
    }
    return triggered;
}

bool BlockNot::notTriggered() {
    return timerState == RUNNING && !hasTriggered();
}

bool BlockNot::firstTrigger() {
    if (onceTriggered) {
        return firstTriggerResponse;
    }
    if (hasTriggered()) {
        onceTriggered = true;
        return timerState == RUNNING;
    }
    return false;
}

void BlockNot::triggerNext() {
    triggerOnNext = true;
}

void BlockNot::setFirstTriggerResponse(const bool response) {
    firstTriggerResponse = response;
}

unsigned long BlockNot::getNextTriggerTime() const {
    if (triggerOnNext) {
        // Matches the pre-existing cTime-backed behavior: writing micros() then millis()
        // into the same shared value meant only the millis() write ever survived.
        return convertValue(millis(), MILLISECONDS, baseUnits);
    }
    return convertUnits(startTime + duration);
}

unsigned long BlockNot::getTimeUntilTrigger() const {
    return timeTillTrigger();
}

unsigned long BlockNot::getStartTime() const {
    return convertUnits(startTime);
}

unsigned long BlockNot::getStartTime(const BlockNotUnit units) const {
    return convertValue(startTime, nativeUnit(), units);
}

unsigned long BlockNot::getDuration() const {
    return timerState == RUNNING ? convertUnits(duration) : timerStoppedReturnValue;
}

unsigned long BlockNot::lastTriggerDuration() const {
    return lastDuration;
}

String BlockNot::getUnits() const {
    switch (baseUnits) {
        case SECONDS:      return "Seconds";
        case MILLISECONDS: return "Milliseconds";
        case MINUTES:      return "Minutes";
        default:           return "Microseconds";
    }
}

unsigned long BlockNot::getTimeSinceLastReset() const {
    return (timerState == RUNNING) ? convertUnits(timeSinceReset()) : timerStoppedReturnValue;
}

void BlockNot::setStoppedReturnValue(const unsigned long stoppedReturnValue) {
    timerStoppedReturnValue = stoppedReturnValue;
}

void BlockNot::start(const bool resetOption) {
    if (resetOption)
        reset();
    else {
        switch (baseUnits) {
            case MICROSECONDS: {
                startTime += micros() - stopTime;
                break;
            }
            default: {
                startTime += millis() - stopTime;
                break;
            }
        }
    }
    timerState = RUNNING;
}

void BlockNot::stop() {
    timerState = STOPPED;
    switch (baseUnits) {
        case MICROSECONDS: {
            stopTime = micros();
            break;
        }
        default: {
            stopTime = millis();
            break;
        }
    }
}

bool BlockNot::isRunning() const { return timerState == RUNNING; }

bool BlockNot::isStopped() const { return timerState == STOPPED; }

void BlockNot::toggle() {
    if (timerState == RUNNING)
        timerState = STOPPED;
    else
        timerState = RUNNING;
}

unsigned long BlockNot::convert(const unsigned long value, const BlockNotUnit units) const {
    return convertValue(value, baseUnits, units);
}

void BlockNot::switchTo(const BlockNotUnit units) { baseUnits = units; }

void BlockNot::reset(const unsigned long newStartTime) {
    unsigned long finalStartTime = newStartTime;
    if (finalStartTime == 0) {
        switch (baseUnits) {
            case MICROSECONDS: {
                finalStartTime = micros() + microsOffset;
                break;
            }
            default: {
                finalStartTime = millis() + millisOffset;
                if (speedCompensation)
                    delay(compTime);
                break;
            }
        }
    }
    resetTimer(finalStartTime);
}

void BlockNot::resetToCapturedTime(const unsigned long capturedMillis, const unsigned long capturedMicros) {
    unsigned long finalStartTime;
    switch (baseUnits) {
        case MICROSECONDS: {
            finalStartTime = capturedMicros + microsOffset;
            break;
        }
        default: {
            finalStartTime = capturedMillis + millisOffset;
            if (speedCompensation)
                delay(compTime);
            break;
        }
    }
    resetTimer(finalStartTime);
}

void BlockNot::setMillisOffset(const unsigned long offset) {
    long delta   = offset - millisOffset;
    startTime    = startTime + delta;
    millisOffset = offset;
}

void BlockNot::setMicrosOffset(const unsigned long offset) {
    long delta   = offset - microsOffset;
    startTime    = startTime + delta;
    microsOffset = offset;
}

void BlockNot::speedComp(const unsigned long time) {
    speedCompensation = true;
    compTime          = time;
}

void BlockNot::disableSpeedComp() {
    speedCompensation = false;
}

unsigned long BlockNot::getMillis() const {
    return millis() + millisOffset;
}

BlockNotUnit BlockNot::getBaseUnits() const {
    return baseUnits;
}

/**
 * Private Methods
 */

void BlockNot::initDuration(const unsigned long time) {
    duration = convertValue(time, baseUnits, nativeUnit());
}

void BlockNot::initDuration(const unsigned long time, const BlockNotUnit inUnits) {
    duration = convertValue(time, inUnits, nativeUnit());
}

void BlockNot::resetTimer(const unsigned long newStartTime) {
    startTime     = newStartTime;
    triggerOnNext = false;
    onceTriggered = false;
}

unsigned long BlockNot::timeSinceReset() const {
    switch (baseUnits) {
        case MICROSECONDS:
            return microsOffset + micros() - startTime;
        default:
            return millisOffset + millis() - startTime;
    }
}

bool BlockNot::hasTriggered() {
    if (triggerOnNext) {
        triggerOnNext = false;
        return true;
    }
    const unsigned long sinceReset = timeSinceReset();
    const bool triggered           = sinceReset >= duration;
    if (triggered)
        lastDuration = sinceReset;
    return triggered;
}

unsigned long BlockNot::timeTillTrigger() const {
    unsigned long tillTrigger = 0L;
    if (!triggerOnNext) {
        const unsigned long sinceReset      = timeSinceReset();
        const unsigned long remainingNative = (sinceReset < duration) ? (duration - sinceReset) : 0UL;
        tillTrigger                         = (timerState == RUNNING) ? convertUnits(remainingNative) : timerStoppedReturnValue;
    }
    return tillTrigger;
}

BlockNotUnit BlockNot::nativeUnit() const {
    return (baseUnits == MICROSECONDS) ? MICROSECONDS : MILLISECONDS;
}

unsigned long BlockNot::convertValue(const unsigned long value, const BlockNotUnit fromUnits, const BlockNotUnit toUnits) {
    if (fromUnits == toUnits) return value;
    switch (fromUnits) {
        case MICROSECONDS:
            switch (toUnits) {
                case MILLISECONDS: return value / 1000UL;
                case SECONDS: return value / 1000000UL;
                default: return value / 60000000UL; // MINUTES
            }
        case MILLISECONDS:
            switch (toUnits) {
                case MICROSECONDS: return value * 1000UL;
                case SECONDS: return value / 1000UL;
                default: return value / 60000UL; // MINUTES
            }
        case SECONDS:
            switch (toUnits) {
                case MICROSECONDS: return value * 1000000UL;
                case MILLISECONDS: return value * 1000UL;
                default: return value / 60UL; // MINUTES
            }
        default: // MINUTES
            switch (toUnits) {
                case MICROSECONDS: return value * 60000000UL;
                case MILLISECONDS: return value * 60000UL;
                default: return value * 60UL; // SECONDS
            }
    }
}

unsigned long BlockNot::convertUnits(const unsigned long nativeValue) const {
    return convertValue(nativeValue, nativeUnit(), baseUnits);
}

void BlockNot::addToTimerList() {
    if (firstTimer == nullptr) {
        firstTimer = currentTimer = this;
    }
    else {
        currentTimer->nextTimer = this;
        currentTimer            = this;
    }
    this->nextTimer = nullptr;
}

/**
 * Global Methods affecting all instantiations of the BlockNot class
 */

void resetAllTimers(const unsigned long newStartTime) {
    BlockNot *current = BlockNot::firstTimer;
    if (newStartTime != 0) {
        while (current != nullptr) {
            current->reset(newStartTime);
            current = current->nextTimer;
        }
        return;
    }
    const unsigned long capturedMillis = millis();
    const unsigned long capturedMicros = micros();
    while (current != nullptr) {
        current->resetToCapturedTime(capturedMillis, capturedMicros);
        current = current->nextTimer;
    }
}
