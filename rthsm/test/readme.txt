This is a fictious state machine used to test the state machine
engine.

Below is an ascii representation of a UML2.0 state machine diagram for
this state machine. Please note the global state is not represented.
Default initial states have their name written in UPPERCASE.


State machine variables:
  iteration = 0
  process = 99
  malfunction = 123
  counter = 2


Some actions were too large for the ascii diagram and have bee
reported here:

(1) counter++; if ((counter % 7) == 0) push(EV_ACQUIRE) else push(EV_DATA)

(2) if ((iteration == 2) && (process > 5)) push(EV_ERROR) else push(EV_PROCESSING)

(3) if (process > 10) push(EV_PROCESSED)


 +---------------------------------------------------------------------+
 |                                                                     |
 | DeviceOn                                                            |
 |                                                                     |
 | exit / iteration++                                                  |
 |                                                                     |
 |  +--------------------------------------------+                     |
 |  |                                            |                     |
 |  | ACTIVE                                     |                     |
 |  |                       EV_ACQUIRE /         |                     |
 |  |   +----------------+  process = 0          |    +------------+   |
 |  |   |                |__________             |    |            |   |
 |  |   | READING        |          \            |    | Saving     |   |
 |  |   |                |          |            |    |            |   |
 |  |   | entry /        |          V            |    |            |   |
 |  |   | push(EV_DATA)  |     +-------------+   |    |            |   |
 |  |   |                |     |             |   |    +------------+   |
 |  |   |                |     | Processing  |   |          ^          |
 |  |   | EV_DATA / (1)  |     |             |   |          |          |
 |  |   |    ____        |     | entry / (2) |--------------+          |
 |  |   |   /    \       |     | exit / (3)  |   |  EV_PROCESSED       |
 |  |   |   |    |       |     +-------------+   |                     |
 |  |   |   |    V       |        |    ^         |                     |
 |  |   +----------------+        |    |         |                     |
 |  |                 ^           \____/         |                     |
 |  |                 |                          |   EV_ERROR          |
 |  |                 |       EV_PROCESSING /    |_____                |
 |  |                 |       process++          |     \               |
 |  |      EV_RECOVER |                          |     |               |
 |  |                 |                          |     |               |
 |  |                 +-----------------------+  |     |               |
 |  |                                         |  |     |               |
 |  +-----------------------------------------|--+     |               |
 |                                            |        |               |
 |                                            |        |               |
 +--------------------------------------------|--------|---------------+
   |            ^                             |        |      
   | EV_SAVED   | EV_NEXT [iteration < 5]     |        |      
   V            |                             |        |      
 +----------------+                  +--------|--------|----------+
 |                |                  |        |        |          |
 | STARTING       |                  | Error  |        |          |
 |                |                  |        |        V          |
 +----------------+                  |    +--------------+        |
        |                            |    |              |        |
        |                            |    | MALFUNCTION  |        |
        |                            |    |              |        |
        | EV_NEXT [iteration >= 5]   |    +--------------+        |
        |                            |      |                     |
        |                            |      | EV_NEXT             |
        |                            |      | [malfunction != 0]  |
        |                            |      | / push(EV_RECOVER)  |
        |                            |      |                     |
        |                            |      V                     |
        |                            +----------------------------+
        V                               |
 +--------------+                       |
 |              |        EV_DEAD        |
 | Finished     |<----------------------+
 |              |
 +--------------+

