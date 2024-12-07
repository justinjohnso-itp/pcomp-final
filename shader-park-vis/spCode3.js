/* eslint-disable */

export function spCode() {
  //Put your Shader Park Code here
  let buttonHover = input();
  let click = input();
  color(vec3(0, 0, 1) * click);
  let r = getRayDirection();

  rotateX(-0.8);
  box(1, 0.1 * noise(r * 10 + time * 0.4) + 0.1, 0.8);
  mixGeo(1 - click);
  sphere(0.2);

  // expand(buttonHover * .08)
  blend(0.2);
  displace(mouse.x, mouse.y, 0);
  sphere(0.1);
}
