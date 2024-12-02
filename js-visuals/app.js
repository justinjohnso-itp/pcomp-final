import {
  Scene,
  PerspectiveCamera,
  WebGLRenderer,
  Color,
  SphereGeometry,
} from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { createSculptureWithGeometry } from "shader-park-core";
import { EffectComposer } from "three/examples/jsm/postprocessing/EffectComposer";
import { RenderPass } from "three/examples/jsm/postprocessing/RenderPass";
import { ShaderPass } from "three/examples/jsm/postprocessing/ShaderPass";
import { spCode } from "./spCode.js";

// Scene setup
let scene = new Scene();
let state = { time: 0, contrast: 1, sphereSize: 1, speed: 1 }; // Added speed

// Debug Overlay Setup
const debugContainer = document.createElement("div");
debugContainer.style.cssText = `
  position: fixed;
  top: 10px;
  left: 10px;
  background: rgba(0, 0, 0, 0.8);
  color: #fff;
  padding: 15px;
  border-radius: 5px;
  font-family: monospace;
  font-size: 12px;
  z-index: 1000;
  min-width: 200px;
  pointer-events: none;
`;
document.body.appendChild(debugContainer);

// MIDI Logger Setup
const midiLogContainer = document.createElement("div");
midiLogContainer.style.cssText = `
  position: fixed;
  bottom: 10px;
  left: 10px;
  background: rgba(0, 0, 0, 0.8);
  color: #fff;
  padding: 15px;
  border-radius: 5px;
  font-family: monospace;
  font-size: 12px;
  z-index: 1000;
  pointer-events: none;
`;
document.body.appendChild(midiLogContainer);

const midiLogs = [];
const maxLogs = 5;

// Camera setup
let camera = new PerspectiveCamera(
  40,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
camera.position.set(0, 4, 0);
camera.lookAt(0, 0, 0);

// Render setup
let renderer = new WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio(window.devicePixelRatio);
renderer.setClearColor(new Color(1, 1, 1), 1);
document.body.appendChild(renderer.domElement);

// Contrast adjustment shader
const ContrastShader = {
  uniforms: {
    tDiffuse: { value: null },
    contrast: { value: 1 },
  },
  vertexShader: `
    varying vec2 vUv;
    void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
  `,
  fragmentShader: `
    uniform sampler2D tDiffuse;
    uniform float contrast;
    varying vec2 vUv;
    void main() {
      vec4 color = texture2D(tDiffuse, vUv);
      color.rgb = (color.rgb - 0.5) * contrast + 0.5;
      color.rgb = clamp(color.rgb, 0.0, 1.0);
      gl_FragColor = color;
    }
  `,
};

// Post-processing setup
const composer = new EffectComposer(renderer);
const renderPass = new RenderPass(scene, camera);
const contrastPass = new ShaderPass(ContrastShader);
composer.addPass(renderPass);
composer.addPass(contrastPass);

// Sphere Grid Setup
let gridSize = 6;
let spacing = 0.9;

let spheres = [];
for (let x = 0; x < gridSize; x++) {
  for (let y = 0; y < gridSize; y++) {
    for (let z = 0; z < gridSize; z++) {
      let geometry = new SphereGeometry(0.2, 45, 45);
      let mesh = createSculptureWithGeometry(geometry, spCode, () => ({
        time: state.time,
      }));
      mesh.position.set(
        (x - gridSize / 2) * spacing,
        (y - gridSize / 2) * spacing,
        (z - gridSize / 2) * spacing
      );
      scene.add(mesh);
      spheres.push(mesh);
    }
  }
}

// Controls setup
let controls = new OrbitControls(camera, renderer.domElement);
controls.enableZoom = true;

// Update debug overlay
function updateDebugOverlay() {
  debugContainer.innerHTML = `
    <div style="color: #00ff00;">MIDI & Sphere Debug</div>
    <div style="margin-top: 8px;">
      CC12 (Speed): ${state.speed.toFixed(3)}
      <br>CC13 (Contrast): ${state.contrast.toFixed(3)}
      <br>CC14 (Sphere Size): ${state.sphereSize.toFixed(3)}
      <br>Animation Time: ${state.time.toFixed(3)}
      <br>
      <br>Sphere Position Y: ${spheres[0].position.y.toFixed(3)}
      <br>Sphere Scale: ${spheres[0].scale.x.toFixed(3)}
      <br>
      <br>Camera Position:
      <br>X: ${camera.position.x.toFixed(2)}
      <br>Y: ${camera.position.y.toFixed(2)}
      <br>Z: ${camera.position.z.toFixed(2)}
    </div>
  `;
}

// Update MIDI log
function updateMIDILog(message) {
  const [status, data1, data2] = message.data;
  const timestamp = new Date().toLocaleTimeString();
  midiLogs.unshift(`${timestamp} - CC${data1}: ${data2}`);
  if (midiLogs.length > maxLogs) midiLogs.pop();

  midiLogContainer.innerHTML = `
    <div style="color: #00ff00;">Recent MIDI Messages</div>
    <div style="margin-top: 8px;">
      ${midiLogs.join("<br>")}
    </div>
  `;
}

// MIDI Integration
if (navigator.requestMIDIAccess) {
  navigator.requestMIDIAccess()
    .then((midiAccess) => {
      for (let input of midiAccess.inputs.values()) {
        input.onmidimessage = handleMIDIMessage;
      }
    })
    .catch(console.error);
} else {
  console.error("Web MIDI API not supported in this browser.");
}

// MIDI message handler
function handleMIDIMessage(message) {
  const [status, data1, data2] = message.data;

  if (status === 176) {
    if (data1 === 12) {
      state.speed = (data2 / 127) * 5;
    }
    if (data1 === 13) {
      state.sphereSize = 4 - (data2 / 127) * 2;
      spheres.forEach((sphere) =>
        sphere.scale.set(state.sphereSize, state.sphereSize, state.sphereSize)
      );
    }
    if (data1 === 14) {
      state.contrast = (data2 / 127) * 2;
      contrastPass.uniforms.contrast.value = state.contrast;
    }
    updateMIDILog(message);
  }
}

// Render loop
function render() {
  requestAnimationFrame(render);
  state.time += 0.01 * state.speed;

  spheres.forEach((sphere) => {
    sphere.position.y = Math.sin(state.time) * 0.5;
  });

  updateDebugOverlay();
  controls.update();
  composer.render();
}

render();
