// This function will be converted into a string so
// the scope is limited to this function only.

// To pass external data use the 'input' function. See other examples.

export function spCode() {
  let space = getSpace(); // 3D coordinates of the current pixel being rendered

  lightDirection(0, 0, 0); // lighting direction
  setMaxIterations(50);
  setStepSize(.4);

  let array = [0, 0, 0]; // array holding rgb values
  let vectorLength = length(space) + 0.001; // Add small epsilon to prevent division by zero
  let animation = time;

  for (let i = 0; i < 3; i++) {
    let uv = vec3(space.x, space.y, space.z); // create a vector
    animation += .09;

    // Prevent potential division by zero
    if (vectorLength > 0) {
      let animationTerm = (sin(animation)+1.);
      let oscillationTerm = abs(sin(vectorLength*3.-animation*1.));

      // Safer vector manipulation
      uv = uv + (space/vectorLength) * animationTerm * oscillationTerm;
    }

    array[i] = .02/length(abs(fract(uv-1)-.5));
  }

  backgroundColor(5, 5, 5);
  let col = vec3(
    array[0]/vectorLength, 
    array[1]/vectorLength, 
    array[2]/vectorLength
  );
  
  color(col*30);
  metal(length(col));
  shine(2);
  occlusion(1);

  mirrorN(3, .6);
  
  sphere(length(col)*1.5);
}
