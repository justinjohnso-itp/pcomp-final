import { initializeMIDI } from './midi.js';

class MIDIDebugger {
  constructor() {
    this.state = {
      note1: 0,
      note2: 0,
      dist1a: 0,
      dist1b: 0,
      dist2a: 0,
      dist2b: 0,
      pot1: 0,
      pot2: 0
    };

    this.debugLog = [];
    this.maxLogEntries = 50;
    this.setupMIDILogging();
  }

  setupMIDILogging() {
    this.createDebugContainer();
  }

  createDebugContainer() {
    const debugContainer = document.createElement('div');
    debugContainer.id = 'midi-debug-container';
    debugContainer.style.cssText = `
      position: fixed;
      top: 10px;
      right: 10px;
      width: 300px;
      max-height: 400px;
      overflow-y: auto;
      background: rgba(0,0,0,0.7);
      color: #0f0;
      padding: 10px;
      font-family: monospace;
      z-index: 1000;
    `;
    document.body.appendChild(debugContainer);
  }

  handleMIDIMessage(message) {
    const [status, data1, data2] = message.data;

    if (status === 176) {  // Control change message
      this.updateState(data1, data2);
      this.logMIDIMessage(message);
      this.renderDebug();
    }
  }

  updateState(data1, data2) {
    const mappings = {
      20: 'note1',
      21: 'note2',
      10: 'dist1a',
      11: 'dist1b',
      12: 'dist2a',
      13: 'dist2b',
      14: 'pot1',
      15: 'pot2'
    };

    const stateKey = mappings[data1];
    if (stateKey) {
      // Normalize the control data (0 to 1 range)
      this.state[stateKey] = data2 / 127;
    }
  }

  logMIDIMessage(message) {
    const logEntry = {
      timestamp: new Date().toISOString(),
      status: message.data[0],
      data1: message.data[1],
      data2: message.data[2]
    };

    this.debugLog.push(logEntry);

    // Maintain a maximum log size
    if (this.debugLog.length > this.maxLogEntries) {
      this.debugLog.shift();
    }
  }

  renderDebug() {
    const debugContainer = document.getElementById('midi-debug-container');
    if (!debugContainer) return;

    // State display
    const stateHTML = Object.entries(this.state)
      .map(([key, value]) => `${key}: ${value.toFixed(3)}`)
      .join('<br>');

    // Recent log entries
    const logHTML = this.debugLog
      .slice(-10)  // Show last 10 entries
      .map(entry => 
        `[${entry.timestamp.split('T')[1]}] 
         Status: ${entry.status}, 
         Data1: ${entry.data1}, 
         Data2: ${entry.data2}`
      )
      .join('<br>');

    debugContainer.innerHTML = `
      <h3>MIDI State</h3>
      ${stateHTML}
      <hr>
      <h3>Recent MIDI Log</h3>
      ${logHTML}
    `;

    // Expose state to global scope for shader interaction
    window.midiState = this.state;
  }

  startDebugging() {
    try {
      initializeMIDI(this.handleMIDIMessage.bind(this));
      console.log('MIDI Debugging Initialized');
    } catch (error) {
      console.error('MIDI Initialization Error:', error);
      this.renderErrorMessage(error);
    }
  }

  renderErrorMessage(error) {
    const debugContainer = document.getElementById('midi-debug-container');
    if (debugContainer) {
      debugContainer.innerHTML = `
        <h3 style="color: red;">MIDI Error</h3>
        ${error.message}
      `;
    }
  }
}

// ********************* SHADER PARK ********************* 

export function spCode() {
  let space = getSpace(); // 3D coordinates of the current pixel being rendered

  // Access MIDI state globally
  const midiState = window.midiState || {
    note1: 0,
    note2: 0,
    dist1a: 0,
    dist1b: 0,
    dist2a: 0,
    dist2b: 0,
    pot1: 0,
    pot2: 0
  };

  // Dynamic lighting and spacing based on MIDI inputs
  lightDirection(0,0,0);

  // Adjust iterations and step size dynamically
  setMaxIterations(5); 
  setStepSize(0.2); 

  // Variables for animation and coloring
  let array = [0, 0, 0]; // array holding rgb values
  let vectorLength = 10; // dynamic vector length
  let animation = time; // time variable for smooth animation

  // Using MIDI values to influence animation behavior
  animation += midiState.pot1 * 100;  // Control animation speed with pot1
  vectorLength = midiState.dist1a * 20;  // Use dist1a for vector length

  // Looping through the channels for animation
  for (let i = 0; i < 3; i++) {
    let uv = vec3(space.x, space.y, space.z); // create a vector
    animation += 25; // speed of animation over time
    vectorLength = length(space); // changing length of the vectors over time

    // Animation driver
    uv += space/vectorLength * (sin(animation)+2.) * abs(sin(vectorLength*3.-animation*1.)); 

    array[i] = .02/length(abs(fract(uv-1)-.5)); // calculates a glowing effect
  }

  // Color calculation
  backgroundColor(255, 255, 255);
  let col = vec3(
    array[0]/vectorLength, 
    array[1]/vectorLength, 
    array[2]/vectorLength
  );
  color(col * 0.2); // apply color

  // Materials
  metal(10);
  shine(2);
  occlusion(10);

  // Sphere size
  sphere(length(col) * 2); 
}

// ********************* MIDI DEBUG ********************* 

// Initialize MIDI Debugger
const midiDebugger = new MIDIDebugger();
midiDebugger.startDebugging();

export default midiDebugger;
