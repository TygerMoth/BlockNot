# Changelog
All notable changes to this project will be documented in this file.

The format is based on **Keep a Changelog**, and this project adheres to **Semantic Versioning**.

---

## [3.1.0] – 2026-07-10
### Fixed
- Several internal timer state variables (offsets, speed-comp settings, first-trigger response, stopped-return
  value, etc.) were left uninitialized by every constructor, causing undefined/garbage behavior including
  immediate spurious triggers and unpredictable `delay()` calls. They now default deterministically
  (0/false), matching documented defaults.
- `takeTime()` could leave a timer's duration negative instead of clamping to 0 when taking more time than
  remained in the duration.
- `start(NO_RESET)` (resuming a stopped timer) discarded time that had already elapsed before `stop()` was
  called, instead of preserving it as documented.
- `getStartTime(BlockNotUnit units)` always misinterpreted the stored start time as microseconds regardless
  of the timer's actual base unit, producing wrong results for millisecond/second/minute timers.
- `setMicrosOffset()` did not adjust `startTime` to compensate, unlike `setMillisOffset()`, causing an
  instantaneous jump in elapsed time when called on a running timer.
- `getUnits()` had no case for a `MINUTES`-based timer and fell through to returning `"Microseconds"` for
  one - found while auditing README.md against the code for accuracy. Now returns `"Minutes"`.
- `resetAllTimers()`/`RESET_TIMERS` no longer guaranteed that every timer got the exact same startTime. The
  zero-argument form re-read `millis()`/`micros()` independently inside each timer's own `reset()` call as
  the loop walked the timer list, instead of capturing the clock once and sharing it. (Git history shows the
  original implementation's default argument was `= millis()`, evaluated once per call - a later refactor
  changed the default to `0` and lost that guarantee, most likely by accident, since it directly contradicts
  the documented purpose of the feature.) In practice millis()-based timers usually landed on the same
  millisecond anyway since the loop is fast, but it wasn't architecturally guaranteed, and `MICROSECONDS`
  timers mixed into the same reset were meaningfully more likely to end up out of sync. `resetAllTimers()`
  now captures `millis()` and `micros()` once per call and hands the appropriate one (plus that timer's own
  offset) to each timer via a new private `resetToCapturedTime()`, restoring the original guarantee. The
  explicit-timestamp form (`resetAllTimers(someValue)`) is unaffected - it already applied one caller-supplied
  value verbatim to every timer.
- Deprecated macros (`DONE`, `TIME_PASSED`, `TIME_SINCE_RESET`, `TIME_TILL_TRIGGER`, `TRIGGERED_ON_MARK`,
  `NOT_DONE`, `ISSTARTED`, `TRIGGERED_ON_DURATION_ALL`, `TRIGGERED_ALL`, `START_RESET`) triggered a compiler
  warning on GCC/Clang toolchains (including every PlatformIO board target) simply from including
  `BlockNot.h`, regardless of whether a sketch actually used any of them. Deprecation warnings now only
  fire at the exact call site of a deprecated macro, matching the behavior already present for MSVC. Also
  removed a stray duplicate `#define TIME_PASSED` inside the class body that silently overrode the intended
  definition.

### Changed
- Replaced the internal `cTime` double-backed representation (used for `duration`/`stopTime` and every
  unit conversion) with plain integer arithmetic. `cTime` did a floating-point multiply on every read,
  including on the `hasTriggered()` hot path called every `loop()` iteration; it's gone now, along with all
  floating-point usage in the class. This is an internal representation change with no public signature
  changes - `BlockNot::cTime` itself is removed, but it was never referenced in README.md, the examples, or
  keywords.txt.
  - As a side effect, unit conversions (`convert()`, `getStartTime(units)`, etc.) are now exact instead of
    occasionally off by one due to `double` round-trip rounding error (e.g. `convert(1000001, MICROSECONDS,
    MICROSECONDS)` previously returned `1000000`; it now correctly returns `1000001`). This can change the
    exact return value for existing sketches in these rounding-error edge cases, always toward the
    mathematically correct result.
  - Measured impact: `sizeof(BlockNot)` drops from 56 to 44 bytes on AVR (8-bit) and from 96 to 52 bytes on
    32-bit targets (ESP32/RP2040). A single-timer sketch on an Uno drops from 246 to 234 bytes RAM and 3164
    to 2590 bytes flash.
- `convertValue()` (the unit-conversion routine backing `convert()`, `getDuration()`, `ELAPSED`, `REMAINING`,
  etc.) no longer promotes to `unsigned long long` to do the multiply/divide. It's replaced by direct
  32-bit arithmetic per unit pair, which is exact across the full practical range of a `millis()`/`micros()`-
  driven duration and avoids pulling AVR's 64-bit `libgcc` division/multiplication routines into the binary
  of any sketch that calls a getter - previously a cost paid even by sketches whose timer never actually
  exercised a non-identity conversion at runtime, since the compiler can't prove that statically.
- `timeSinceReset()` - read on every `hasTriggered()`/`triggered()` call, i.e. every `loop()` iteration for
  every active timer - no longer unconditionally computes `millisOffset + millis()` before discarding it for
  `MICROSECONDS`-based timers; that branch is now only evaluated on the path that uses it.
- The 17 constructor overloads shared a large amount of duplicated body logic (the sticky/explicit global
  reset ternary, `stop()`, `initDuration()`, `reset()`, `addToTimerList()`), all inlined into each overload.
  They now delegate to a single private `init()` helper, cutting duplicate code generation across the
  overload set; no behavior or call-site change.
- `BlockNotUnit`, `BlockNotState`, and `BlockNotGlobal` now declare an explicit `uint8_t` underlying type,
  and the private member fields were reordered to group all of the 1-byte fields (the four `bool`s plus
  `baseUnits`/`timerState`) together at the end of the class. AVR's default `-fshort-enums` already packed
  these enums to 1 byte, but 32-bit targets that don't set that flag (e.g. stock ESP32/RP2040 toolchains)
  were storing them as 4-byte `int`s. The reorder matters as much as the type change: giving the enums a
  1-byte type but leaving them interleaved among 4-byte fields (as they were originally declared) would
  have the compiler pad each one right back out to 4 bytes for alignment, silently erasing the saving.
  Grouped together, they share a single trailing padding gap instead. No public enum value changes.
- Removed two private methods, `hasNotTriggered()` and `remaining()`, that were fully implemented but never
  called from anywhere in the class - dead code left over from an earlier refactor.
- `triggeredOnDuration()` computed `timeSinceReset()` up to three times per call (once inside `hasTriggered()`,
  once directly, and once more inside the now-removed `getDurationTriggerStartTime()` helper) even though
  the second and third reads were always computing the same value. It's now read once and reused, cutting a
  redundant `millis()`/`micros()` read and division on every duration-catch-up trigger.

### Removed
- **BREAKING:** `getHelp()` (both overloads), added in 2.4.0, has been removed entirely - not deprecated.
  It printed a ~30-line macro reference table via `Serial.println()`, which cost real flash (and, until this
  release, RAM too - see the now-moot `F()` fix that briefly lived in this same Unreleased section). That's a
  disproportionate amount of storage for a one-off debugging aid on memory-constrained targets, so instead of
  carrying it forward under deprecation, it's gone. Any sketch calling `myTimer.getHelp(...)` will now fail
  to compile; the macro reference table itself still lives in README.md. This is why this release is a major
  version bump.
- Unused private members `newStartTimeMillis` and `newStartTimeMicros` (dead code, never read or written).
- `BlockNot::cTime` (public nested class) and its `milli_t`/`micro_t`/`minutes_t` helper classes, replaced
  by the integer-based representation described above.
- Dead private methods `hasNotTriggered()` and `remaining()` (see above).


## [2.4.0] – 2025-XX-XX
### Added
- `triggerNext()` method and `TRIGGER_NEXT` macro.
- Ability to define timers in **MINUTES**.
- `getHelp()` method.

### Changed
- Restructured internal code for efficiency.
- Rewrote the ResetAll example for clarity.

---

## [2.3.0] – 2025-XX-XX
### Added
- `speedComp()` feature for preventing rapid unintended triggers on high-speed MCUs (e.g., Raspberry Pi Pico).

---

## [2.2.0] – 2025-XX-XX
### Added
- `setFirstTriggerResponse(bool)` method to control behavior of `firstTrigger()`.

---

## [2.1.5] – 2025-XX-XX
### Added
- Advanced Auto Flashers example.
- Updated README documentation.

---

## [2.1.4] – 2025-XX-XX
### Changed
- Minor internal updates (per PR #19).

---

## [2.1.3] – 2025-XX-XX
### Changed
- `firstTrigger()` no longer performs calculations after triggering, improving efficiency of repeated calls.

---

## [2.1.2] – 2025-XX-XX
### Fixed
- Bug fix.

---

## [2.1.1] – 2025-XX-XX
### Changed
- Renamed public enums to avoid name conflicts with other libraries.

---

## [2.1.0] – 2025-XX-XX
### Added
- Added **Last Trigger Duration** support.

---

## [2.0.7] – 2025-XX-XX
### Changed
- Updated variable declarations to improve thread-safety (see Thread Safety notes).

---

## [2.0.6] – 2025-XX-XX
### Added
- Constructors allowing creation of timers in a **STOPPED** state.
- `setDuration(unsigned long time, Unit inUnits)` to change duration using alternative units without modifying base units.
- Ability to start a timer *while resetting it*, modifying start/stop behavior to act more like a stopwatch.

---

## [2.0.5] – 2025-XX-XX
### Changed
- Adjusted handling of millis offset.

### Added
- `setMicrosOffset(unsigned long)` for testing micros() rollover.

---

## [2.0.3] – 2025-XX-XX
### Added
- Undocumented methods to aid in testing millis() rollover.
- Example sketch demonstrating rollover behavior.

---

## [2.0.0] – 2025-XX-XX
### Added
- Major upgrade enabling timers based on **seconds**, **milliseconds**, and **microseconds**.
- Significant internal redesign to support microsecond precision.

---

## [1.8.5] – 2024-XX-XX
### Added
- `TRIGGERED_ALL` and `TRIGGERED_ON_MARK` macros.
- Millis() Rollover section added to documentation.

### Changed
- Rewrote the triggeredOnDuration documentation and simplified related graphs.

---

## [1.8.4] – 2024-XX-XX
### Changed
- Renamed `STARTED`, `RUNNING`, and `STOPPED` to `ISSTARTED`, `ISRUNNING`, and `ISSTOPPED` to avoid conflicts.

---

## [1.8.3] – 2024-XX-XX
### Fixed
- Minor bug fixes.

---

## [1.8.2] – 2024-XX-XX
### Changed
- Global Reset option is now default again.
- Updated README to reflect internal public API adjustments.

---

## [1.8.1] – 2024-XX-XX
### Added
- `timeTillTrigger()` method and `TIME_TILL_TRIGGER` macro.

### Removed
- Removed unnecessary millis() rollover code.

### Changed
- Cleaned up redundant macros.
- Renamed internal methods for consistency.

---

## [1.8.0] – 2024-XX-XX
### Changed
- Major restructuring into separate header and code files.
- Fixed ODR (One Definition Rule) violation.

---

## [1.7.4] – 2024-XX-XX
### Fixed
- Corrected `triggeredOnDuration` calculations; now accounts for rollover.

---

## [1.7.3] – 2024-XX-XX
### Changed
- Now compatible with millis rollover at ~49 days.

---

## [1.7.2] – 2024-XX-XX
### Added
- `start()` and `stop()` methods.
- `START` and `STOP` macros.

### Changed
- Timers can now be modified while stopped.

---

## [1.7.1] – 2024-XX-XX
### Changed
- Minor efficiency improvements.

---

## [1.7.0] – 2024-XX-XX
### Added
- New `resetAllTimers()` (`RESET_TIMERS`) with bug fixes.
- `triggeredOnDuration()` method and corresponding macro.

### Fixed
- Corrected accumulated drift caused by earlier resetAllTimers behavior.

---

## [1.6.7] – 2024-XX-XX
### Fixed
- Bug preventing compilation when declaring a timer with duration alone.

---

## [1.6.6] – 2024-XX-XX
### Added
- `resetAllTimers()` / `RESET_TIMERS` contributed by @bizprof.

---

## [1.6.5] – 2024-XX-XX
### Added
- Added **SECONDS Mode**.
