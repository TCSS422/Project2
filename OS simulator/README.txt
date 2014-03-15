Peter Pentescu, Dawn Rocks,
Mike Baxter, & Maya Osbourne
TCSS 422 - Winter 2014
14 March 2014


(1) How to Use the Simulator

The program can be invoked from command-line using no arguments. The user is instead prompted to enter the parameters once the program begins executing. The user is first prompted to enter the number of processes to run; keyboard processes, I/O bound processes, producer-consumer processes, and computation processes. The number of each type of processes entered can range from 1 to the desired number of processes. After the numbers of processes to run are specified, the user is then prompted to select a scheduling algorithm. Upon entering the scheduling choice and pressing enter, the simulation begins. The simulation displays messages indicating which process is running, when service requests are generated, when interrupts occur, when context switches occur, and the state of each process on each context switch. Dots (“.”) are printed for every virtual instruction executed. The keyboard device generates interrupts whenever the user types a key. Typing ‘q’ will quit the program.

(2) Comparing Combinations of Processes and Devices Under Different Scheduling Policies

Round-Robin

This algorithm is very efficient for producer-consumer processes when ordered correctly (consumer runs immediately after producer), but it can lock the system into a worst-case state when run the other way around (consumer runs first, then has to wait for the round-robin process to go all the way through before the producer comes back up). In a real system with more complex producer-consumer behavior, finding an optimum round-robin ordering could be very difficult.

Priority

For the simulation we gave I/O bound processes the highest priority, followed by producer-consumer memory management, then compute background processes. This results in fast user response, but some processes (especially low priority calculators) may almost never get the chance to run.

Lottery

This algorithm works OK for almost everything, but doesn't work exceptionally well for anything. It can take a very long time to respond to keyboard input when there are a large number of processes, because their "lottery tickets" don't always come up quickly.
