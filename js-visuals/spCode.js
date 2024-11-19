// This function will be converted into a string so
// the scope is limited to this function only.

// To pass external data use the 'input' function. See other examples.

export function spCode() {
  // Put your Shader Park Code here

  // Demo shape
  // rotateY((mouse.x * PI) / 2 + time * 0.5);
  // rotateX((mouse.y * PI) / 2);
  // metal(0.5);
  // shine(0.4);
  // color(getRayDirection() + 0.2);
  // rotateY(getRayDirection().y * 4 + time);
  // boxFrame(vec3(0.4), 0.02);
  // expand(0.02);
  // blend(nsin(time) * 0.6);
  // sphere(0.2);

  // Box
  color(vec3(0.2, 0.8, 1.0));
  box(vec3(0.4, 0.4, 0.4));
}
