#include "commands.h"

void Scheduler::schedule(Command command) {
  int priority = command.getPriority();
  auto pos = commands.begin();
  
  switch (priority) {
    case IMMEDIATE:
      commands.insert(std::next(pos, numImmediate), command);
      numImmediate++;
      break;
    case AUTO:
      commands.insert(std::next(pos, numImmediate+numAuto), command);
      numAuto++;
      break;
    case TELEOP:
    default:
      commands.insert(commands.end(), command);
      break;
  }
}

void Scheduler::schedule(String command) {
  // Do nothing currently
}

bool Scheduler::update() {
  if (commands.front().finished()) {
    commands.erase(commands.begin());
    return true;
  }

  return false;
}
