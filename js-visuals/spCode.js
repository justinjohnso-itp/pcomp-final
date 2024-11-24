// This function will be converted into a string so
// the scope is limited to this function only.

// To pass external data use the 'input' function. See other examples.

export function spCode() {

  let space = getSpace(); // 3D coordinates of the current pixel being rendered

  // lighting and spacing of shapes
  lightDirection(0, 0, 0); // lighting direction
  setMaxIterations(25); // decreasing this number will speed up the render time but may cause artifacts
  setStepSize(.5); // similar purpose to setMaxInterations but better for raymarching

  // variables for animation and coloring
  let array = [0, 0, 0]; // array holding rgb values
  let vectorLength = 2; // length of the vectors
  let animation = time; // time variable for smooth animation

  // looping through the  channels for animation
  for (let i = 0; i < 3; i++) {
    let uv = vec3(space.x, space.y, space.z); // create a vector
    animation += .09; // speed of animation over time
    vectorLength = length(space); // changing length of the vectors over time

    // animation driver applied to the uv vector, modifying the "space function" -> modifying each rendered pixel so it moves through space over time
    // space/vectorLength -> Normalizes the space vector
    // (sin(animation)+1.) -> oscillation between -1 and 1 over time, adding a value shifts the range, ex: adding 1 shifts the range from 0 to 2
    // abs ensures that a value stays positive over time
    // sin(vectorLength*9.-animation* 2) -> scaled vector length and time based oscillation
    uv += space/vectorLength*(sin(animation)+1.)*abs(sin(vectorLength*3.-animation*1.)); 

    array[i] = .02/length(abs(fract(uv-1)-.5)); // calculates a glowing effect based on the uv distortion and the vector length
  }

  // color
  backgroundColor(0, 0, 0);
  let col = vec3(array[0]/vectorLength, array[1]/vectorLength, array[2]/vectorLength); // fade the colors out based on the length of the vectors over time
  color(col*30); // apply color, rach modifier increases or decreases the effect of the colors
  // materials based on the values provided by the color values
  metal(length(col));
  shine(2);
  occlusion(0.5);

  // mirror, (axis, distance factor)
  mirrorN(3, .6);
  
  sphere(length(col)*1.5); // the size of the sphere depends on the values provided by the color vector
}
