// This function will be converted into a string so
// the scope is limited to this function only.

// To pass external data use the 'input' function. See other examples.

export function spCode() {

  let space = getSpace(); // 3D coordinates of the current pixel being rendered

  // lighting and spacing of shapes
  lightDirection(getRayDirection()); // align light direction with ray direction, returns the direction of the ray from the camera
  setMaxIterations(50); // decreasing this number will speed up the render time but may cause artifacts
  setStepSize(.9); // similar purpose to setMaxInterations but better for raymarching

  // variables for animation and coloring
  let arrayColor = [0, 0, 0]; // array holding rgb values
  let vectorLength = 1; // length of the vectors
  let animation = time; // time variable for smooth animation

  // looping through the color channels
  for (let i = 0; i < 3; i++) {
    let uv = vec3(space.x, space.y, space.z); // create a vector
    animation += .09; // speed of animation over time
    vectorLength = length(space); // changing length of the vectors over time
    uv += space/vectorLength*(sin(animation)+1.)*abs(sin(vectorLength*9.-animation*2.)); // animation driver, modifying the "space function" -> modifying each rendered pixel so it moves through space over time
    arrayColor[i] = .02/length(abs(fract(uv-0.5)-.5)); // calculates a glowing effect based on the uv distortion and the vector length
  }

  // color
  let col = vec3(arrayColor[0]/vectorLength, arrayColor[1]/vectorLength, arrayColor[2]/vectorLength); // fade the colors out based on the length of the vectors over time
  color(col*5); // apply color, rach modifier increases or decreases the effect of the colors
  // materials based on the values provided by the color values
  metal(length(col));
  shine(length(col));

  // mirror, (axis, distance factor)
  mirrorN(2, .5);
  
  sphere(length(col)*2); // the size of the sphere depends on the values provided by the color vector
}
