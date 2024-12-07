/* eslint-disable */

export function spCode() {
  //Put your Shader Park Code here
  setStepSize(0.1);
  let buttonHover = input();
  let click = input();
  let n = noise(getSpace() * 8 + buttonHover);
  shine(10);
  color(0.2 * normal * click + n * 0.2 + vec3(0, 0, 2));
  sphere(0.3);
  expand(n * 0.2);
  mixGeo(buttonHover);
  box(0.3, 0.3, 0.3);
}
