#ifndef COMMANDS_H_
#define COMMANDS_H_

#define MAX_COMMANDS 10

class Command {
  private:
    String _name;
    int _priority;
  
  public:
    Command(String name, int priority): _name(name), _priority(priority) {}

    void initialize();
    void execute();
    bool finished();
    void stop();

    String getName() {
      return _name;
    }
    int getPriority() {
      return _priority;
    }
};

class Scheduler {
  private:
    Command scheduledCommands[MAX_COMMANDS];
    int scheduledPriorities[MAX_COMMANDS];
    int numScheduled;
  
  public:
    Scheduler();
    void schedule(Command command);
    Command getCurrentCommand();
};

#endif COMMANDS_H_