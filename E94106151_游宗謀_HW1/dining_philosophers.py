import threading
import time
import random

# Define the number of philosophers and chopstick
num_philosophers = 5
num_chopsticks = num_philosophers

# Define semaphores for the chopsticks and the mutex
chopsticks = [threading.Semaphore(1) for i in range(num_chopsticks)]
mutex = threading.Semaphore(1)

# Define the philosopher thread function
def philosopher(index):
    # while True:
    for i in range(2):
        print(f"Philosopher {index} is thinking...")
        time.sleep(random.randint(1, 5))
        
        mutex.acquire()
        
        left_chopstick_index = index
        right_chopstick_index = (index + 1) % num_chopsticks
        
        chopsticks[left_chopstick_index].acquire()
        chopsticks[right_chopstick_index].acquire()
        
        mutex.release()
        
        print(f"Philosopher {index} is eating...")
        time.sleep(random.randint(1, 5))
        
        chopsticks[left_chopstick_index].release()
        chopsticks[right_chopstick_index].release()

# Create a thread for each philosopher
philosopher_threads = []
for i in range(num_philosophers):
    philosopher_threads.append(threading.Thread(target=philosopher, args=(i,)))
    
# Start the philosopher threads
for thread in philosopher_threads:
    thread.start()
    
# Wait for the philosopher threads to complete
for thread in philosopher_threads:
    thread.join()
