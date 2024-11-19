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
  // color(vec3(0.2, 0.8, 1.0));
  // box(vec3(0.4, 0.4, 0.4));

  // let s = getSpace();
  // let scale = 0.1;
  let amplitude = 0.2;
  let speed = 5;
  // let n = noise(s + amplitude + speed) * scale;
  let scale = 2.0;
  let s = getSpace();
  let n = 0.1 * noise(scale * s + time);

  // let c = 0.5 * noise(noiseScale * s + time) + 0.5;

  let r = 0.7 * noise(s + amplitude + speed);
  let g = 0.4 * noise(s + amplitude + speed);
  let b = 0.6 * noise(s + amplitude + speed);

  color(r, g, b);
  shine(0.9);
  metal(0.5);
  sphere(0.7 + n);
  expand(n);
}
