export function spCode() {
    return `
      float shape = sdBox(vec3(p.x, p.y, p.z), vec3(1.0, 1.0, 1.0));
      vec3 color = vec3(0.2, 0.8, 1.0);
      return vec4(color, 1.0);
    `;
  }
