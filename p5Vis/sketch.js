// MIDI CC
const midiMappings = {
  6: "note1",
  8: "note2",
  10: "dist1a",
  11: "dist1b",
  12: "dist2a",
  13: "dist2b",
  14: "pot1",
  15: "pot2",
};

let midiInput;

let squareSize = 15; // Initial offset for square size

let ripplesActiveLeft = false;
let ripplesActiveRight = false;
let ripplesLeft = []; // Array to store left-side ripples
let ripplesRight = []; // Array to store right-side ripples

let lastTimeLeft = 0; // Variable to control time-based ripple generation for the left side
let lastTimeRight = 0; // Variable to control time-based ripple generation for the right side
let rippleInterval = 1000; // Interval between new ripples (in milliseconds)

let pressStartTimeLeft = 0; // Time when the left MIDI control is activated
let pressStartTimeRight = 0; // Time when the right MIDI control is activated
let maxPressTime = 5000; // Maximum time for MIDI control activation (in milliseconds)
let pressedLeft = false; // State to check if the left MIDI control is activated
let pressedRight = false; // State to check if the right MIDI control is activated

// Warp
let warpingLeft = false; // State to check if left-side warping is active
let warpingRight = false; // State to check if right-side warping is active
let warpTimeLeft = 0; // Time tracker for left-side warping
let warpTimeRight = 0; // Time tracker for right-side warping

// Color
let colorLerpLeft = 0;
let colorLerpRight = 0;
let pressedColorLeft = false;
let pressedColorRight = false;

function setup() {
  createCanvas(windowWidth, windowHeight);
  background(0);

  // Noise for Warp
  noiseDetail(6, 0.7);

  // Initialize MIDI input
  if (navigator.requestMIDIAccess) {
    navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
  } else {
    console.log("Web MIDI not supported.");
  }
}

// ************************************ MIDI ***********************************************

function onMIDISuccess(midiAccess) {
  midiInput = midiAccess.inputs.values().next().value;
  midiInput.onmidimessage = handleMIDIMessage;
}

function onMIDIFailure() {
  console.log("Failed to get MIDI access.");
}

// Handle incoming MIDI message
function handleMIDIMessage(event) {
  const [status, data1, data2] = event.data;

  // console.log(status, data1, data2);

  // Process MIDI CC messages
  // if (status === 144 || status === 128) {
  if (midiMappings[data1]) {
    const ccName = midiMappings[data1];
    const value = data2; // MIDI CC value

    console.log(ccName, value);

    if (ccName === "note1" || ccName === "note2") {
      // Trigger ripple effect when "note1" (CC 20) or "note2" (CC 21) are received
      if (value > 0) {
        if (ccName === "note1") {
          if (!pressedLeft) {
            pressStartTimeLeft = millis();
          }
          pressedLeft = true;
          ripplesActiveLeft = !ripplesActiveLeft;
        } else if (ccName === "note2") {
          if (!pressedRight) {
            pressStartTimeRight = millis();
          }
          pressedRight = true;
          ripplesActiveRight = !ripplesActiveRight;
        }
      } else {
        if (ccName === "note1") {
          pressedLeft = false;
        } else if (ccName === "note2") {
          pressedRight = false;
        }
      }
    }

    // Handle other controls like warping and color lerping
    if (ccName === "dist1a" || ccName === "dist1b") {
      warpingLeft = value > 0;
    } else if (ccName === "dist2a" || ccName === "dist2b") {
      warpingRight = value > 0;
    } else if (ccName === "pot1") {
      pressedColorLeft = value > 0;
    } else if (ccName === "pot2") {
      pressedColorRight = value > 0;
    }
  }
  // }
}

// ************************************ RIPPLES ***********************************************

function draw() {
  background(0);

  // Calculate the number of squares based on the MIDI control interaction
  let squaresLeft = 1; // Default number of squares when no MIDI control is active
  let squaresRight = 2; // Default number of squares when no MIDI control is active

  // Adjust the number of squares based on the time the MIDI control is active
  if (pressedLeft) {
    let elapsedTimeLeft = millis() - pressStartTimeLeft;
    squaresLeft = map(elapsedTimeLeft, 0, maxPressTime, 2, 100);
    squaresLeft = constrain(squaresLeft, 2, 100);
  }

  if (pressedRight) {
    let elapsedTimeRight = millis() - pressStartTimeRight;
    squaresRight = map(elapsedTimeRight, 0, maxPressTime, 1, 100);
    squaresRight = constrain(squaresRight, 1, 100);
  }

  // Left side ripple logic
  if (ripplesActiveLeft) {
    if (millis() - lastTimeLeft > rippleInterval) {
      lastTimeLeft = millis();
      let newRipple = {
        x: 0,
        y: height / 2,
        radius: 0,
        warpSeed: random(1000), // Unique seed for noise-based warping
      };
      ripplesLeft.push(newRipple);
    }

    // Increment warping time if warping is active
    if (warpingLeft) {
      warpTimeLeft += 0.05;
    }

    // Smoothly transition color lerp
    if (pressedColorLeft) {
      colorLerpLeft = lerp(colorLerpLeft, 1, 0.2);
    } else {
      colorLerpLeft = lerp(colorLerpLeft, 0, 0.2);
    }

    // Draw and update all ripples on the left
    for (let i = ripplesLeft.length - 1; i >= 0; i--) {
      let ripple = ripplesLeft[i];
      let offsetRadius = ripple.radius + squareSize;

      for (let j = 0; j < squaresLeft; j++) {
        let angle = map(j, 0, squaresLeft, 0, TWO_PI);

        // Fluid warping using Perlin noise
        let warpOffset = 0;
        if (warpingLeft) {
          let noiseScale = 0.5;
          let noiseVal = noise(
            cos(angle) * noiseScale + warpTimeLeft,
            sin(angle) * noiseScale + warpTimeLeft,
            ripple.warpSeed
          );
          warpOffset = map(noiseVal, 0, 1, -30, 30);
        }

        let x = ripple.x + cos(angle) * (offsetRadius + warpOffset);
        let y = ripple.y + sin(angle) * (offsetRadius + warpOffset);

        let dynamicSize = map(ripple.radius, 0, width, squareSize, 1);
        dynamicSize = constrain(dynamicSize, 1, squareSize);

        let alpha = map(ripple.radius, 0, width, 255, 0);
        let blue = color(random(0, 50), random(100, 255), random(200, 255));
        let white = color(255, 255, 255);
        let squareColor = lerpColor(white, blue, colorLerpLeft);

        drawingContext.shadowBlur = 20;
        drawingContext.shadowColor = color(
          squareColor.levels[0],
          squareColor.levels[1],
          squareColor.levels[2],
          alpha
        );

        fill(
          squareColor.levels[0],
          squareColor.levels[1],
          squareColor.levels[2],
          alpha
        );
        noStroke();
        rect(x, y, dynamicSize, dynamicSize);

        drawingContext.shadowBlur = 0;
      }

      ripple.radius += 2;

      if (ripple.radius > width * 1.5) {
        ripplesLeft.splice(i, 1);
      }
    }
  }

  // Right side ripple logic (similar to left side)
  if (ripplesActiveRight) {
    if (millis() - lastTimeRight > rippleInterval) {
      lastTimeRight = millis();
      let newRipple = {
        x: width,
        y: height / 2,
        radius: 0,
        warpSeed: random(1000), // Unique seed for noise-based warping
      };
      ripplesRight.push(newRipple);
    }

    if (warpingRight) {
      warpTimeRight += 0.05;
    }

    if (pressedColorRight) {
      colorLerpRight = lerp(colorLerpRight, 1, 0.2);
    } else {
      colorLerpRight = lerp(colorLerpRight, 0, 0.2);
    }

    for (let i = ripplesRight.length - 1; i >= 0; i--) {
      let ripple = ripplesRight[i];
      let offsetRadius = ripple.radius + squareSize;

      for (let j = 0; j < squaresRight; j++) {
        let angle = map(j, 0, squaresRight, 0, TWO_PI);

        let warpOffset = 0;
        if (warpingRight) {
          let noiseScale = 0.5;
          let noiseVal = noise(
            cos(angle) * noiseScale + warpTimeRight,
            sin(angle) * noiseScale + warpTimeRight,
            ripple.warpSeed
          );
          warpOffset = map(noiseVal, 0, 1, -30, 30);
        }

        let x = ripple.x + cos(angle) * (offsetRadius + warpOffset);
        let y = ripple.y + sin(angle) * (offsetRadius + warpOffset);

        let dynamicSize = map(ripple.radius, 0, width, squareSize, 1);
        dynamicSize = constrain(dynamicSize, 1, squareSize);

        let alpha = map(ripple.radius, 0, width, 255, 0);
        let hotpink = color(
          random(170, 255),
          random(80, 120),
          random(140, 180)
        );
        let white = color(255, 255, 255);
        let squareColor = lerpColor(white, hotpink, colorLerpRight);

        drawingContext.shadowBlur = 20;
        drawingContext.shadowColor = color(
          squareColor.levels[0],
          squareColor.levels[1],
          squareColor.levels[2],
          alpha
        );

        fill(
          squareColor.levels[0],
          squareColor.levels[1],
          squareColor.levels[2],
          alpha
        );
        noStroke();
        rect(x, y, dynamicSize, dynamicSize);

        drawingContext.shadowBlur = 0;
      }

      ripple.radius += 3;

      if (ripple.radius > width * 1.5) {
        ripplesRight.splice(i, 1);
      }
    }
  }
}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}
