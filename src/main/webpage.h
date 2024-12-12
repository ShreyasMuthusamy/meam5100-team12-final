/*
 * HTML for the Final Project
 */

const char body[] PROGMEM = R"===(
  <!DOCTYPE html>
  <html>
    <head>
      <title>Robot Control Dashboard</title>
      <meta name="viewport" content="user-scalable-no">
      <style>
        body {
          background-color: black;
          color: white;
          font-family: "Trebuchet MS", Helvetics, sans-serif;
        }

        .optionsHeader {
          text-align: center;
        }

        .autoSingleOptions {
          margin-left: 100px;
        }

        .miscOptions {
          margin-left: 150px;
        }

        .taskButtons {
          background-color: blue;
          color: white;
          font-family: "Trebuchet MS", Helvetics, sans-serif;
          border: none;
          padding: 20px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 16px;
          margin: 6px 20px;
          cursor: pointer;
          border-radius: 30px;
        }

        .active {
          background-color: grey;
          cursor: default;
        }
      </style>
    </head>
    <body>
      <div class="optionsHeader">
        <h1>Robot Control Options</h1>
      </div>
      <div class="autoSingleOptions">
        <button id="autoAttackLeftButton" class="taskButtons" onclick="handleCommand('autoAttackLeft')">Autonomous Attack on Left Target</button>
        <button id="autoAttackCenterButton" class="taskButtons" onclick="handleCommand('autoAttackCenter')">Autonomous Attack on Center Target</button>
        <button id="autoAttackRightButton" class="taskButtons" onclick="handleCommand('autoAttackRight')">Autonomous Attack on Right Target</button>
      </div>
      <div class="miscOptions">
        <button id="fullAutoAttackButton" class="taskButtons" onclick="handleCommand('fullAutoAttack')">Autonomous Attack on All Targets</button>
        <button id="wallFollowButton" class="taskButtons" onclick="handleCommand('wallFollow')">Wall Following</button>
        <button id="teleopButton" class="taskButtons" onclick="handleCommand('teleop')">Teleoperation</button>
        <button id="stopButton" class="taskButtons" onclick="handleCommand('stop')">Emergency Stop</button>
      </div>
    </body>
    <script>
      let teleop = false;
      let moving = false;
      let stopped = false;

      document.addEventListener("keydown", handleKeyDown);
      document.addEventListener("keyup", handleKeyUp);

      function handleKeyDown(e) {
        if (e.key == "1") {
          handleCommand('autoAttackLeft');
        } else if (e.key == "2") {
          handleCommand('autoAttackCenter');
        } else if (e.key == "3") {
          handleCommand('autoAttackRight');
        } else if (e.key == "4") {
          handleCommand('fullAutoAttack');
        } else if (e.key == "f") {
          handleCommand('wallFollow');
        } else if (e.key == "t") {
          handleCommand('teleop');
        } else if (e.key == " ") {
          handleCommand('stop');
        }

        if (teleop && !moving) {
          if (e.key == "w") {
            sendControl('front');
          } else if (e.key == "a") {
            sendControl('left');
          } else if (e.key == "s") {
            sendControl('back');
          } else if (e.key == "d") {
            sendControl('right');
          } else if (e.key == "q") {
            sendControl('attack');
          }
        }
      }

      function handleKeyUp(e) {
        if (teleop && moving) {
          sendControl('stop');
        }
      }

      function clearButtons() {
        const buttons = document.getElementsByClassName("taskButtons");
        for (var i = 0; i < buttons.length; i++) {
          buttons[i].classList.remove("active");
        }
      }

      function handleCommand(command) {
        if (!document.getElementById(command.concat("Button")).classList.contains("active") && !stopped) {
          if (command === 'teleop') {
            teleop = true;
          } else {
            teleop = false;
            if (command === 'stopped') {
              stopped = true;
            }
          }

          clearButtons();
          document.getElementById(command.concat("Button")).classList.add("active");
          sendCommand(command);
        }
      }

      function sendCommand(command) {
        var xhttp = new XMLHttpRequest();
        var req = "command=".concat(command);
        xhttp.open("GET", req, true);
        xhttp.send();
      }

      function sendControl(control) {
        if (control === 'stop') {
          moving = false;
        } else if (control !== 'attack') {
          moving = true;
        }
        
        var xhttp = new XMLHttpRequest();
        var req = "control=".concat(control);
        xhttp.open("GET", req, true);
        xhttp.send();
      }
    </script>
  </html>
)===";