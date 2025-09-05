# OBC-CHALLENGE
This challenge requires us to debug C++ source files, integrate it into cFS, and fault injection for CubeSat using 42 Simulator.


## ***1) DEBUGGING C++ SOURCE FILES:***

Since i have not been taught C++ before, I have used the help of AI to debug the source files- but i made sure to understand the logic behind fixing the bugs.

### **FIRST FILE (ADCSController.cpp.):**

1)Manual new/delete for pid_controllers_
→ This risks memory leaks and exceptions. Used std::array<std::unique_ptr<PIDController>, 3> instead.

2)Hardcoded dt = 0.1f
→ Better to pass dt as a parameter (or store control frequency).

3)Redundant memset/memcpy
→ Can use std::fill / std::copy for type safety.

4)safeMode and constructor duplication
→ Simplify with std::fill_n.

5)Destructor deleting controllers
→ Not needed if we use smart pointers.

### SECOND FILE(adcs_controller.h):

1)Added #include <cstring> so that memset/memcpy in .cpp compile.

2)No std::array / smart pointers →  kept the original structure written.

3)Compatible with previous adcs_controller.cpp.

### THRD FILE-(microcontroller.cpp)
1)Added a null check before calling adcs_controller_->computeControl(...) so we never dereference a null pointer.

2)Fixed the health-monitoring bug where uptime_seconds_ could increment at control_cycles_ == 0. Now it increments only after the first 10 cycles.

3)Kept use of memset, new/delete, and the same control/fault flow — no structural redesigns, no smart pointers, no API changes.

4)Left includes, logging, and semantics unchanged except for the two small fixes above.[Uploading microcontroller.h…]()



### FOURTH FILE-(microcontroller.h)
1)Changed Microcontroller::SensorData → just SensorData

2)Changed Microcontroller::ActuatorCommands → just ActuatorCommands
(since they are top-level structs, not nested in the class)


And similarly, analysing the bugs and debugging for the rest of the files.All the debugged files are attached.

## 2) ***INTEGRATION :***




