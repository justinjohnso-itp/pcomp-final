/* eslint-disable */

export function spCode() {
  //Put your Shader Park Code here
  setStepSize(0.2);
  let buttonHover = input();
  let click = input();
  setStepSize(0.3);

  rotateY(time * 0.2);
  let warpedSpace = warpSpace(getSpace());
  metal(0.9);
  shine(1);
  color(1 - warpedSpace);
  sphere(0.2 + length(warpedSpace) * 0.2);
  expand(buttonHover * 0.08);

  // inspired by https://www.shadertoy.com/view/ttlGDf
  function warpSpace(p) {
    let t = time / 4;
    rotateY(getRayDirection().y * (1 - click) * 4);
    p = getSpace().x * 2.0 * (vec3(0.5, 0.2, 0.1) + p);
    for (let i = 1.0; i < 3.0; i += 1.0) {
      p.x = p.x + buttonHover * sin(3.0 * t + i * 1.5 * p.y) + t * 0.5;
      p.y = p.x + buttonHover * cos(3.0 * t + i * 1.5 * p.x);
    }
    return 0.5 + 0.5 * cos(time + vec3(p.x, p.y, p.x) + vec3(0, 2, 4));
  }
}
