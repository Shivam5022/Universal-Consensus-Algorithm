class Queue:
    def __init__(self):
        self.queue = []

    def enqueue(self, value):
        self.queue.append(value)

    def dequeue(self):
        if len(self.queue) > 0:
            return self.queue.pop(0)
        else:
            return "EMPTY"

class Stack:
    def __init__(self):
        self.stack = []

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        if len(self.stack) > 0:
            return self.stack.pop()
        else:
            return "EMPTY"

def print_red(text):
    print("\033[91m" + text + "\033[0m")

def print_green(text):
    print("\033[92m" + text + "\033[0m")

def check_queue(log):
    queue = Queue()
    for line in log:
        if "PUSH" in line:
            value = int(line.split()[-1])
            queue.enqueue(value)
        elif "POP" in line:
            result = queue.dequeue()
            isemp = False
            if "EMPTY" in line:
                isemp = True
            if result == "EMPTY":
                if isemp == False:
                    print_red("ERROR: Trying to dequeue from an empty queue.")
                    return False
            else:
                value = int(line.split()[-1])
                if value != result:
                    print_red(f"ERROR: Expected {result} but got {value}")
                    return False
    print_green("Queue property is maintained.")
    return True

def check_stack(log):
    stack = Stack()
    for line in log:
        if "PUSH" in line:
            value = int(line.split()[-1])
            stack.PUSH(value)
        elif "POP" in line:
            result = stack.POP()
            isemp = False
            if "EMPTY" in line:
                isemp = True
            if result == "EMPTY":
                if isemp == False:
                    print_red("ERROR: Trying to POP from an empty stack.")
                    return False
            else:
                value = int(line.split()[-1])
                if value != result:
                    print_red(f"ERROR: Expected {result} but got {value}")
                    return False
    print_green("Stack property is maintained.")
    return True

def read_execution_log(filename):
    with open(filename, 'r') as file:
        return file.readlines()

def main():
    log = read_execution_log("execution_log.txt")
    queue_log = []
    stack_log = []
    current_log = None

    for line in log:
        if "QUEUE" in line:
            current_log = queue_log
        elif "STACK" in line:
            current_log = stack_log
        else:
            current_log.append(line)
    
    print("RUNNING TESTER SCRIPT:\n")
    print("Testing Queue Log:")
    check_queue(queue_log)
    print("\nTesting Stack Log:")
    check_stack(stack_log)

if __name__ == "__main__":
    main()
