// MIDI CC
const midiMappings = {
  6: "note1",
  8: "note2",
  10: "dist1a",
  11: "dist1b",
  12: "dist2a",
  13: "dist2b",
  15: "pot1",
  14: "pot2",
};

let midiInput;

let squareSize = 15; // Initial offset for square size

let ripplesGenerationAllowedLeft = false;
let ripplesGenerationAllowedRight = false;
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

// Noise for Distortion
let dis1aValue = 0;
let dis1bValue = 0;
let dis2aValue = 0;
let dis2bValue = 0;

// Potentiometer values to control number of squares
let pot1Value = 6; // Default for left side
let pot2Value = 6; // Default for right side

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

  if (midiMappings[data1]) {
    const ccName = midiMappings[data1];
    const value = data2; // MIDI CC value

    console.log(ccName, value);

    if (ccName === "note1" || ccName === "note2") {
      // Trigger ripple generation toggle when "note1" or "note2" are received
      if (value > 0) {
        if (ccName === "note1") {
          if (!pressedLeft) {
            pressStartTimeLeft = millis();
          }
          pressedLeft = true;
          ripplesGenerationAllowedLeft = !ripplesGenerationAllowedLeft;
        } else if (ccName === "note2") {
          if (!pressedRight) {
            pressStartTimeRight = millis();
          }
          pressedRight = true;
          ripplesGenerationAllowedRight = !ripplesGenerationAllowedRight;
        }
      } else {
        if (ccName === "note1") {
          pressedLeft = false;
        } else if (ccName === "note2") {
          pressedRight = false;
        }
      }
    }

    // Handle potentiometer controls for number of squares
    if (ccName === "pot1") {
      // Map MIDI value (0-127) to number of squares (1-100)
      pot1Value = map(value, 0, 127, 6, 100);
    } else if (ccName === "pot2") {
      // Map MIDI value (0-127) to number of squares (1-100)
      pot2Value = map(value, 0, 127, 6, 100);
    }

    // Handle other controls like warping and color lerping
    if (ccName === "dist1a") {
      dis1aValue = value / 127; // Normalize to 0-1
      pressedColorLeft = value > 0;
    } else if (ccName === "dist1b") {
      dis1bValue = value / 127; // Normalize to 0-1
      pressedColorLeft = value > 0;
    } else if (ccName === "dist2a") {
      dis2aValue = value / 127; // Normalize to 0-1
      warpingLeft = value > 0;
    } else if (ccName === "dist2b") {
      dis2bValue = value / 127; // Normalize to 0-1
      warpingRight = value > 0;
    }
  }
}

// ************************************ RIPPLES ***********************************************

function draw() {
  background(0);

  // Use pot1 and pot2 values directly for number of squares
  let squaresLeft = pot1Value;
  let squaresRight = pot2Value;

  // Left side ripple logic
  if (ripplesGenerationAllowedLeft || ripplesLeft.length > 0) {
    if (
      ripplesGenerationAllowedLeft &&
      millis() - lastTimeLeft > rippleInterval
    ) {
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
    colorLerpLeft = max(dis1aValue, dis1bValue);

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
          let noiseAmplitude = map(dis1bValue, 0, 1, 30, 0); // Scale with dis1bValue
          warpOffset = map(noiseVal, 0, 1, -noiseAmplitude, noiseAmplitude);
        }

        let x = ripple.x + cos(angle) * (offsetRadius + warpOffset);
        let y = ripple.y + sin(angle) * (offsetRadius + warpOffset);

        let dynamicSize = map(ripple.radius, 0, width, leftShapeSize, 1);
        dynamicSize = constrain(dynamicSize, 1, leftShapeSize);

        let alpha = map(ripple.radius, 0, width, 255, 0);
        let pink = color(random(255, 243), random(0, 200), random(180, 220));
        let orange = color(random(230, 255), random(88, 229), random(0, 180));
        let squareColor = lerpColor(orange, pink, colorLerpLeft);

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
  if (ripplesGenerationAllowedRight || ripplesRight.length > 0) {
    if (
      ripplesGenerationAllowedRight &&
      millis() - lastTimeRight > rippleInterval
    ) {
      lastTimeRight = millis();
      let newRipple = {
        x: width,
        y: height / 2,
        radius: 0,
        warpSeed: random(1000), // Unique seed for noise-based warping
      };
      ripplesRight.push(newRipple);
    }

    // Increment warping time if warping is active
    if (warpingRight) {
      warpTimeRight += 0.05;
    }

    // Smoothly transition color lerp
    colorLerpRight = max(dis2aValue, dis2bValue);

    // Draw and update all ripples on the right
    for (let i = ripplesRight.length - 1; i >= 0; i--) {
      let ripple = ripplesRight[i];
      let offsetRadius = ripple.radius + squareSize;

      for (let j = 0; j < squaresRight; j++) {
        let angle = map(j, 0, squaresRight, 0, TWO_PI);

        // Fluid warping using Perlin noise
        let warpOffset = 0;
        if (warpingRight) {
          let noiseScale = 0.5;
          let noiseVal = noise(
            cos(angle) * noiseScale + warpTimeRight,
            sin(angle) * noiseScale + warpTimeRight,
            ripple.warpSeed
          );
          let noiseAmplitude = map(dis2bValue, 0, 1, 30, 0); // Scale with dis2bValue
          warpOffset = map(noiseVal, 0, 1, -noiseAmplitude, noiseAmplitude);
        }

        let x = ripple.x + cos(angle) * (offsetRadius + warpOffset);
        let y = ripple.y + sin(angle) * (offsetRadius + warpOffset);

        let dynamicSize = map(ripple.radius, 0, width, rightShapeSize, 1);
        dynamicSize = constrain(dynamicSize, 1, rightShapeSize);

        let alpha = map(ripple.radius, 0, width, 255, 0);
        let blue = color(random(0, 50), random(100, 255), random(180, 255));
        let green = color(random(0, 53), random(94, 170), random(0, 138));
        let squareColor = lerpColor(green, blue, colorLerpRight);

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
        ripplesRight.splice(i, 1);
      }
    }
  }
}
