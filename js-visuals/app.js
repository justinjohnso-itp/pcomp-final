// Import necessary elements
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { createSculptureWithGeometry } from "shader-park-core";
import { spCode } from "./spCode.js"; // Assuming spCode.js contains your Shader Park code

// Create scene, camera, renderer
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
camera.position.z = 5.5;

const renderer = new THREE.WebGLRenderer({
  antialias: true,
  transparent: true,
});
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// Orbit controls
const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;

// // Create geometry and apply Shader Park
const geometry = new THREE.SphereGeometry(2, 45, 45);
const mesh = createSculptureWithGeometry(geometry, spCode, () => ({
  time: 0.0,
}));
scene.add(mesh);

// Animation loop
const clock = new THREE.Clock();
function animate() {
  requestAnimationFrame(animate);
  const deltaTime = clock.getDelta();
  mesh.material.uniforms.time.value += deltaTime;
  controls.update();
  renderer.render(scene, camera);
}

animate();
