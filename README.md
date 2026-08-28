*This project has been created as part of the 42 curriculum by pzavada*

# Codexion

## Description
**Codexion** is a concurrent programming project based on a modified variation of the classic Dining Philosophers problem. 

In this simulation, $N$ **coders** sit around a circular table with $N$ shared **dongles** placed between each adjacent pair. To perform a compilation cycle, a coder requires access to **two adjacent dongles** (left and right). A complete cycle consists of three sequential states:
1. **Compiling**: The coder holds both dongles and compiles their project.
2. **Debugging**: Dongles are released, and the coder spends time debugging.
3. **Refactoring**: The coder refactors their code before attempting to acquire dongles for the next compilation.

If a coder fails to start compiling within a specified `burnout_time` since their last compilation started (or since simulation start), they **burn out** and the simulation terminates. Additionally, dongles enter a **cooldown period** immediately after release, during which no coder may acquire them.

### Key Learning Areas
- **POSIX Threads (`pthreads`)**: Creation, management, and lifecycle of concurrent execution threads.
- **Thread Synchronization**: Mutex locks (`pthread_mutex_t`) and condition variables (`pthread_cond_t`) for thread coordination.
- **Deadlock Prevention**: Designing asymmetric resource acquisition strategies to break circular wait conditions.
- **Precise Timing & Monitoring**: Implementing a real-time monitor thread to track coder health and enforce precise burnout timeouts.
- **Resource Cooldowns & Scheduling**: Managing shared resource availability with timers using `pthread_cond_timedwait`.
- **Log Serialization & Race Condition Prevention**: Safeguard against data races and prevent corrupted output via output mutexes.

---

## Blocking Cases & Synchronization Handled

### 1. Deadlock Prevention
Deadlocks are prevented by enforcing an **asymmetric resource acquisition order**:
- **Odd ID Coders**: Always attempt to acquire their **left dongle first**, then their **right dongle**.
- **Even ID Coders**: Always attempt to acquire their **right dongle first**, then their **left dongle**.

By breaking the symmetry across adjacent coders around the circular table, circular wait conditions are eliminated, guaranteeing that deadlocks cannot occur.

### 2. Dongle Cooldown Handling
After a coder finishes compiling and releases their dongles, each dongle enters a mandatory **cooldown period**.
- Cooldown timing is handled using **`pthread_cond_timedwait`**.
- Instead of CPU-heavy busy-waiting (spinning) or imprecise polling, waiting threads block efficiently on a condition variable until the calculated timestamp when the cooldown expires.

### 3. Precise Burnout Detection & Signal Propagation
- A dedicated **monitor thread** continuously tracks coder state, checking if the elapsed time since a coder's last compilation exceeds `burnout_time`.
- Upon detecting a burnout, the monitor locks global simulation state, sets the stop flag, logs the event, and broadcasts a signal (`pthread_cond_broadcast`) to unblock all waiting threads.
- Coders check the stop flag at multiple execution checkpoints (before entering dongle lines, before taking dongles, and during sleep intervals) to guarantee immediate and clean shutdown across the entire simulation.

### 4. Log Serialization
- All state transition messages are serialized through a centralized printing function guarded by a dedicated **stdout mutex** (`print_lock`).
- This guarantees strictly sequential, ungarbled, chronologically ordered log entries and prevents race conditions or post-termination log emissions.

---

## Thread Synchronization Mechanisms

All standard POSIX thread synchronization primitives allowed in the subject have been utilized to ensure thread safety and eliminate data races:

1. **`pthread_mutex_t` (Mutual Exclusion Locks)**
   - Used to protect shared memory locations and data structures, preventing multiple threads from reading or writing concurrently to the same resource at the same time.
   - **Example (`print_lock` Serialization)**: 
     To ensure clean, ungarbled, and readable log output without interleaved character streams from concurrent threads, all console output is synchronized using a dedicated `print_lock` mutex:
     ```c
     void mut_print(t_coder *c, char *str)
     {
         unsigned long long time;

         pthread_mutex_lock(&c->init->print_lock);
         if (c->init->stop)
         {
             pthread_mutex_unlock(&c->init->print_lock);
             return ;
         }
         time = get_time_ms() - c->init->start_t;
         printf("%llu %i %s\n", time, c->id, str);
         pthread_mutex_unlock(&c->init->print_lock);
     }
     ```

2. **`pthread_cond_t` (Condition Variables)**
   - Used to suspend thread execution until a specific condition evaluates to true (such as simulation startup synchronization or dongle availability queue signaling).
   - Allows threads to sleep efficiently until signaled by another thread (`pthread_cond_signal` / `pthread_cond_broadcast`) instead of continuously polling.

3. **`pthread_cond_timedwait` (Timed Condition Wait)**
   - Used to block a thread until a condition is signaled **or** a specified absolute system timestamp is reached.
   - Particularly utilized for **dongle cooldowns**, enabling coders to wait efficiently for the exact duration of a dongle's cooldown timer before acquiring it.

---

## Instructions

### Compilation Options (Makefile)
The project includes a standard `Makefile` compiled with `cc` using the `-Wall -Wextra -Werror -pthread` flags.

Available rules:
- `make` or `make all`: Compiles the source files and generates the `./codexion` executable.
- `make clean`: Removes compiled object files (`.o`).
- `make fclean`: Removes compiled object files and the `./codexion` executable.
- `make re`: Performs a full re-compilation (`fclean` followed by `all`).

### Command Line Arguments
The executable expects **8 required command-line arguments**:

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

| Argument | Description |
| :--- | :--- |
| `number_of_coders` | Total number of coders (and dongles) in the simulation. |
| `time_to_burnout` | Maximum time (in ms) a coder can go without starting a compilation before burning out. |
| `time_to_compile` | Time (in ms) a coder spends compiling (holding both dongles). |
| `time_to_debug` | Time (in ms) a coder spends debugging after compilation. |
| `time_to_refactor` | Time (in ms) a coder spends refactoring before attempting to compile again. |
| `number_of_compiles_required` | Number of successful compilation cycles each coder must complete to finish. |
| `dongle_cooldown` | Mandatory cooldown duration (in ms) after a dongle is released before it can be taken again. |
| `scheduler` | Queue scheduling algorithm to use: `fifo` (First In, First Out) or `edf` (Earliest Deadline First). |

### Example Execution
```bash
# Compile the project
make

# Run simulation with 4 coders, 800ms burnout, 200ms compile, 200ms debug, 200ms refactor, 5 compiles each, 100ms dongle cooldown, using FIFO queue
./codexion 4 800 200 200 200 5 100 fifo
```

---

## Resources

- **AI Assistance**: Artificial intelligence (ChatGPT & Gemini) was utilized during development for bug fixing, optimizing concurrent state management, and deepening theoretical understanding of multithreading mechanics.
- **Online Testers**: Custom multithreading test suites and online evaluation tools were used to verify timestamp accuracy, deadlock freedom, and output formatting.
  - *https://codexion-visualizer.sacha-dev.me/*
- **YouTube Tutorials & Concurrency Guides**: Educational videos covering POSIX threads, mutex locks, condition variables, and classic concurrency problems:
  - *https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2*
