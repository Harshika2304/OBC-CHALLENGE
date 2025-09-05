# OBC-CHALLENGE
This challenge requires us to debug C++ source files, integrate it into cFS, and fault injection for CubeSat using 42 Simulator.

## ***APPROACH***: 
First, i got access to the bugged C++ course files, debugged them, and ran it on VS code to make sure that it runs without errors without changing the logic of the code. Next, I made sure the corrected C++ files were included in the cFS build system by adding the build instructions and registering the app to run when the system starts. Then, I built the system inside the Docker container, and ran the simulation to make sure everything was working fine. Then the next step was to let the python script read the telemetry data, which is provided by cubesat software so that it could be monitored.

## ***RESOURCES***:
Chat GPT: For debugging C++ files  https://chatgpt.com/c/68ba107b-0e68-8321-8846-9bfeaca13339
Perplexity AI: steps for clarifying the cloning process
Python telemetry scripting articles — for understanding telemetry data flow and processing
NASA Core Flight System GitHub repository (https://github.com/nasa/cFS) — for codebase and build infrastructure





## ***1) DEBUGGING C++ SOURCE FILES:***

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
a) I added my debugged C++ files into the cFS project by first making a (CMakeLists.txt file) that had all my source files stored. Then, I updated the main build file (targets.cmake) to include my app folder(my_apps) so the build system knows to compile it, I also changed the startup scripts to make sure my app runs when the simulation starts.

b)Then, I built the  cFS system inside a docker container that I set up to share my local source files, then after building, I ran the simulator inside that container, and then I checked docker desktop to make sure taht the container was running and using resources, so at least the sim was active.

The code that I used in windows CMD :

docker pull hk2989441/adcs_sim:test2
docker stop adcs_sim
docker rm adcs_sim
docker run -it --cap-add=sys_nice --cap-add=ipc_lock --cap-add=SYS_RESOURCE \
--ulimit rtprio=99 --ulimit memlock=-1 --ulimit nice=-20 \
--sysctl fs.mqueue.msg_max=256 --sysctl fs.mqueue.msgsize_max=65536 \
--user root --name adcs_sim -v C:/Users/benny/cFS:/home/cFS \
hk2989441/adcs_sim:test2

# Enter container terminal
docker exec -it adcs_sim /bin/bash

# Inside container
cd /home/cFS
make distclean
make SIMULATION=native prep
make
cd build/exe/cpu1
./core-cpu1

<img width="1222" height="665" alt="image" src="https://github.com/user-attachments/assets/f9d9b37c-c616-467d-bb33-e8de63929202" />

<img width="963" height="630" alt="image" src="https://github.com/user-attachments/assets/435ba498-630b-42fb-b55d-01ca813a5236" />

## ***FAILURES AND FIXES***
Issue: (make) wasn’t working on my windows CMD because the tools it was based on linux only.

Fix: I switched to using a docker container that had linux and all the build tools ready.

Issue: My debugged files didn’t show up inside the container at first.

Fix: I fixed that by setting up docker volume mounts to share my windows source folder into the container, so the container sees my files.

Issue: I wasn’t quite sure how to connect the telemetry output from my C++ code with the Python script.

Fix: I spent time understanding how telemetry data flows and how the Python code reads it and it was mostly about matching file locations.


Since i didn't have time to complete the project, i would have emphasized completely integrating the telemetry Python code with the C++ program to receive live data exchanged between them. My strategy would be to first specify the data structure format that the C++ program outputs so that the python code can read it without complications. I would test this exchange step by step, beginning with basic file outputs or socket connections, and then progress to a live connection.

The greatest blockers I encountered were time and some confusion regarding how precisely the telemetry data must be passed from the C++ simulation to the Python script. Additionally, it took some time to set up the build environment within docker, which cut into how much time I could have devoted to the telemetry integration. I would prioritize specifying the format of telemetry data upfront and design small tests to test end to end data transfer prior to constructing the complete system.









