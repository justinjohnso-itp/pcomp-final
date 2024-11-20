// This function will be converted into a string so
// the scope is limited to this function only.

// To pass external data use the 'input' function. See other examples.

export function spCode() {

  let space = getSpace(); // get 3D space coordinates

  // lighting and spacing of shapes
  lightDirection(getRayDirection()); // align light direction with ray direction
  setMaxIterations(50); // decreasing this number will speed up the render time but may cause artifacts
  setStepSize(.8); // similar purpose to setMaxInterations but better for raymarching

  // variables for animation and coloring
  let c = [0, 0, 0]; // array holding rgb values
  let l = 1; // length of the vectors
  let z = time; // time variable for animation

  // looping through the color channels
  for (let i = 0; i < 3; i++) {
    let uv = vec3(space.x, space.y, space.z); // create a vector
    z += .03; // animation over time
    l = length(space); // changing length of the vectors over time
    uv += space/l*(sin(z)+1.)*abs(sin(l*9.-z*2.)); // fractal distortion
    c[i] = .01/length(abs(fract(uv-0.5)-.5)); // glow pattern
  }

  // color
  let col = vec3(c[0]/l, c[1]/l, c[2]/l); // fade the colors out based on the length of the vectors over time
  color(col*5); // apply color
  // materials based on the values provided by the color values
  metal(length(col));
  shine(length(col));

  // mirror
  mirrorN(2, .5);
  
  sphere(length(col)*2); // the size of the sphere depends on the values provided by the color vector
}
