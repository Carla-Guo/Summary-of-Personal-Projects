# Building a Smart Stair Lighting System with ESP32-C6 and mmWave Radar

## The Starting Point: A YouTube Video

This project began after I came across a stair lighting video on YouTube.

The concept was simple but fascinating: two mmWave radars detect a person's movement direction, and the stair lights illuminate sequentially like flowing water, either from bottom to top or from top to bottom.

The original project was built around Home Assistant and included a complete automation setup and tutorial.

My first reaction was:

> "I want to build one too."

However, it didn't take long before I realized I didn't want to replicate the original design exactly.

---

# Why I Didn't Choose Home Assistant

The reason was fairly simple.

Although Home Assistant is an incredibly powerful platform, it doesn't really fit my environment.

I live in a relatively small apartment where devices are located close together, and I generally prefer lightweight standalone systems rather than maintaining a large smart-home ecosystem.

There was also a practical networking consideration.

I primarily rely on mobile internet and didn't want to maintain additional network infrastructure just for a stair lighting system.

At the same time, the two radar nodes still needed a way to communicate wirelessly.

That ultimately led me to ESP-NOW.

As it turns out, ESP-NOW is almost ideal for this kind of project:

- No router required
- No cloud dependency
- Low latency
- Direct peer-to-peer communication
- Lightweight enough for embedded systems

---

# System Architecture

The system consists of:

- A bottom radar node
- A top radar node
- ESP-NOW wireless communication
- An LED control node
- Addressable LED strips

The workflow is straightforward:

1. A radar detects movement or human presence
2. Presence information is transmitted via ESP-NOW
3. The system determines walking direction
4. The LED state machine executes the animation
5. The lights remain on briefly before gradually turning off

Unlike cloud-based smart-home solutions, everything runs locally.

This results in extremely low response latency while eliminating the need for Wi-Fi infrastructure.

---

# Hardware Selection

## Power System

Power delivery turned out to be one of the most important parts of the entire project.

Rather than powering the LED strip directly from the ESP32, I used an LED Driver Board as the power foundation.

The power system was designed around the following requirements:

- 12V input
- Stable 12V output for LED strips
- Stable 5V output for control electronics
- Independent LED power delivery

This allows the ESP32-C6 to focus entirely on logic processing and communication without handling LED current loads.

---

## Radar Sensor

Like the original project, I chose mmWave radar instead of a simpler PIR sensor.

My radar of choice was the LD2410C because it includes Bluetooth connectivity and an official mobile application.

Reasons for choosing the LD2410C include:

- Static human detection
- Better sensitivity adjustment
- Bluetooth support
- OTA firmware updates
- Official configuration application
- Greater flexibility for future experiments

Compared with PIR sensors, the LD2410C is clearly better suited for responsive indoor interaction systems.

---

# The First Prototype

The first prototype was intentionally simple.

At this stage, the only goal was:

> "Can a radar directly control an LED strip?"

The software architecture was minimal:

- Read radar status
- Determine whether someone is present
- Turn LEDs on or off

Everything was still written in a straightforward procedural style.

Despite its simplicity, this stage was extremely important because it validated:

- Radar communication
- LED control
- Human presence detection
- Static target detection

For the first time, the project felt real.

---

# Adding ESP-NOW Communication

The next step was establishing ESP-NOW communication between two ESP32-C6 nodes.

This was the point where the project evolved from a single-device prototype into a distributed system.

The lower radar node transmitted:

- Target status
- Distance
- Radar mode
- Motion-related data

The upper node then combined local and remote radar information to determine walking direction.

Initially, most of the challenges seemed manageable:

- Enum type conversions
- ESP32 Arduino Core API changes
- ESP-NOW callback updates
- Structure synchronization

Annoying, but solvable.

The real challenge came later.

---

# When the System Started Becoming Unstable

As more features were added, the software gradually became harder to manage.

The main loop slowly turned into a combination of:

- Radar polling
- Communication handling
- Animation logic
- Direction detection
- Timeout management
- Serial debugging
- State tracking

And, inevitably:

- `delay()`
- Nested `if` statements
- Repeated logic
- Blocking animations

started appearing everywhere.

The frustrating part was that every individual module worked perfectly on its own.

- The radar worked
- ESP-NOW worked
- LEDs worked

But once everything was running together, strange behavior appeared:

- Incorrect direction detection
- Random triggers
- Stuttering animations
- Lights refusing to turn off
- Timing conflicts

At first I blamed unstable hardware or ESP32 libraries.

Eventually I realized the real problem was something else entirely.

It was the architecture.

---

# Understanding the Real Problem

At some point, I noticed a pattern that existed in many of my previous projects.

As soon as multiple modules started interacting, the entire system became fragile.

The problem wasn't any individual feature.

The problem was that everything lived inside `loop()`.

Everything depended on everything else.

Long delays blocked communication.

Animation timing affected sensor polling.

State logic became tightly coupled.

The project had reached a point where adding new features was harder than implementing the features themselves.

That was when I decided to completely rethink the software architecture.

---

# Moving to an FSM Architecture

Instead of treating the project as one large program, I split it into several independent finite state machines.

The final architecture evolved into three major subsystems.

## Radar FSM

Responsible only for:

- Reading radar sensors
- Debouncing sensor data
- Producing stable presence snapshots

## Passage FSM

Responsible only for:

- Determining walking direction
- Tracking passage states
- Generating lighting events

## LED FSM

Responsible only for:

- Running animations
- Rendering LED effects
- Managing animation timing

Meanwhile, the `loop()` function itself became nothing more than a scheduler:

```cpp
void loop() {
    unsigned long now = millis();

    if (taskDue(now, lastRadarTaskAt, RADAR_POLL_MS)) {
        radarRunFsm(now);
    }

    if (taskDue(now, lastPassageTaskAt, PASSAGE_POLL_MS)) {
        passageRunFsm(now);
    }

    if (taskDue(now, lastLedTaskAt, LED_POLL_MS)) {
        ledRunFsm(now);
    }
}
```

This became the single biggest turning point of the project.

Suddenly the system was:

- Predictable
- Stable
- Easier to debug
- Easier to expand

Most importantly:

> The modules finally stopped interfering with each other.

---

# New Features That Appeared Naturally

Once the architecture became stable, many features that previously felt difficult suddenly became straightforward to implement.

## Entrance Preview Lighting

When someone approaches the stairs, the system no longer lights the entire staircase immediately.

Instead, the first 30 LEDs near the detected entrance illuminate first.

It creates a much more natural experience.

The staircase feels aware of your presence.

---

## Directional Flow Animation

Once both radar nodes confirm the walking direction, the LEDs flow accordingly:

- Bottom → Top when walking upstairs
- Top → Bottom when walking downstairs

This was the original visual effect that inspired the project.

---

## Stair Occupancy Hold

If someone stops on the staircase, the lights remain on.

The system does not force an immediate timeout simply because the animation has finished.

---

## Delayed Turn-Off

When the staircase becomes empty, the lights remain illuminated for a configurable period before turning off.

This creates a much smoother user experience.

---

## Radar Debouncing

Even mmWave radar sensors exhibit occasional fluctuations.

To improve reliability, I introduced a debouncing mechanism:

```cpp
updateDebouncer(...)
```

A presence state is only considered valid after remaining stable for a predefined period.

This eventually became one of the biggest contributors to overall system stability.

---

# Eventually, I Started Redesigning the Hardware

Once the software architecture stabilized, another issue became increasingly obvious:

At present, the system looks more like a makeshift lab setup than a finished project, and it looks a bit unsightly sitting on the stairs. 
Furthermore, anyone who has used XIAO knows that their buttons are simply too small. Every debugging session, firmware update, and hardware modification has become increasingly cumbersome.

So eventually I redesigned the entire system into a dedicated PCB. 

But at the beginning, I wasn't trying to create a completely new board. Instead, I started with an existing LED Driver Board design. My goal was simple:

> Reduce wiring and integrate everything into a single board.

This was actually my first custom PCB design. I made some practical improvements to the LED driver board：
- XIAO ESP32-C6 integration
- Radar interfaces
- Light sensor integration
- Expansion GPIO headers
- The Boot button is exposed externally instead of requiring access to the tiny button on the XIAO module
- Unused GPIOs are routed to edge headers for future expansion

At first glance, these changes seemed straightforward.

They weren't.


They weren't.

---

# Learning What Every Component Actually Does

One challenge I quickly discovered was that I didn't fully understand many parts of the original LED driver circuit.

Before modifying anything, I had to work backwards and learn why every component existed.

For each unfamiliar component, I found myself repeatedly asking:

* What does this part do?
* Why is it here?
* What would happen if I removed it?
* Does its position matter?

This process eventually grew into a personal hardware knowledge base where I documented PCB design concepts, layout practices, and common mistakes.

Instead of blindly copying circuits, I wanted to understand them.

Only after understanding the original design could I confidently begin modifying it.

---

# When the Schematic Was Finished... I Thought I Was Done

Like many beginners, I initially believed that finishing the schematic meant the hard work was over.

Then I opened the PCB layout editor.

And immediately realized I was wrong.

The schematic looked neat and organized.

The PCB looked like chaos.

Connections crossed everywhere.

Components seemed impossible to place.

Nothing fit the way I imagined.

My first attempt was to use the automatic router.

That lasted about five minutes.

The software technically connected everything, but the result looked terrible.

Some edge connectors ended up near the center of the board.

Power traces wandered across the PCB.

Signal paths took bizarre routes.

The board was electrically connected, but it wasn't a design I would actually want to manufacture.

Eventually I deleted most of the routing and started over.

One trace at a time.

---

# Every Component Has Its Own Personality

One lesson that surprised me was that PCB design isn't simply about connecting pins together.

Different components have very different layout requirements.

A schematic tells you what should connect.

A PCB layout determines whether those connections actually work well.

For example:

### Input Protection Components

The fuse, reverse-polarity protection diode, and TVS diode need to be placed as close as possible to the 12V power input.

Their job is to stop electrical problems before those problems enter the rest of the board.

If they are placed too far away, they become significantly less effective.

### Switching Power Components

The inductor used by the 5V switching regulator behaves very differently.

It carries relatively large currents and generates electromagnetic noise.

Because of that:

* Traces should be short and wide
* Nearby sensitive circuitry should be avoided
* The surrounding area should remain relatively clear

Ignoring these recommendations can introduce instability and noise throughout the board.

### Sensitive Control Circuits

Meanwhile, small capacitors and feedback components used for voltage regulation have their own requirements.

These parts should remain close to the regulator IC.

Long traces can introduce noise and reduce regulation performance.

The more I learned, the more I realized that PCB layout is almost a form of physical engineering logic.

The schematic defines relationships.

The layout defines behavior.

---

# Designing for Heat

Another challenge was thermal management.

LED systems can draw significant current.

Even when everything works electrically, excessive heat can become a long-term reliability problem.

For this part of the design, I borrowed several techniques from the original LED Driver Board.

Components carrying large currents were grouped together.

Power nets were merged into large copper pours instead of relying entirely on thin traces.

Large copper areas were used to distribute current more effectively and spread heat across the PCB.

I also added numerous thermal vias connecting copper regions between layers.

These vias help transfer heat into larger copper areas and improve overall cooling performance.

It's not the kind of thing that immediately stands out when looking at the finished board.

But it has a major impact on reliability.

---

# From Modules to a Platform

By the time the PCB was complete, the project had changed significantly. At this point, it no longer feels like a simple stair light. It feels more like a compact local sensing and lighting control platform.

Ironically, the goal at the beginning was simply to make a staircase light.

But the hardware journey ended up teaching me almost as much as the software side of the project.

---

# Future Development and Community Feedback

One interesting aspect of this project is that the controller board itself is already built upon a previous design iteration.

The power section originated from an earlier LED Driver Board that provided stable 12V input and dedicated 12V/5V outputs for LEDs and control electronics. For this project, the design was expanded to integrate the XIAO ESP32-C6, radar interfaces, external boot controls, and additional expansion connectors into a single platform.

Now that both the software architecture and PCB prototype are functioning reliably, I'd like to continue exploring how far this idea can evolve as a compact standalone sensing and lighting platform.

Several future directions are currently being considered.

## Ambient-Light Adaptive Brightness

The LD2410C provides light-sensing information that could be used to automatically adjust LED brightness based on environmental conditions.

For example:

- Brighter during daytime
- Dimmer at night
- Reduced power consumption in dark environments

---

## Position Tracking Using Radar Distance Gates

The two radar nodes can do more than simply detect occupancy.

By leveraging distance gate information from both ends of the staircase, it may be possible to estimate a person's position on the stairs.

Instead of illuminating the entire staircase, the lighting could dynamically follow the user as they move.

---

## Web-Based Configuration

Future firmware versions may include a built-in web interface that allows users to configure the system without modifying code.

Potential settings include:

- Stair length (LED count)
- Animation speed
- Radar distance gate sensitivity
- Hold duration
- Brightness limits
- OTA firmware updates

This would make adapting the system to different stair installations significantly easier.

---

# What Would You Add?

At this point, I'm curious to see where the community would take this project next.

If you were building a radar-based stair lighting system, what features would you add?

Would you focus on:

- Smarter lighting effects?
- More accurate position tracking?
- Energy efficiency?
- Multi-floor installations?
- Something completely different?

I'd love to hear new ideas and see where this project could go next.
