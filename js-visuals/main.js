import { AudioListener, Audio, AudioLoader, AudioAnalyser, Clock, Scene, SphereGeometry, Vector3, PerspectiveCamera, WebGLRenderer, Color, MeshBasicMaterial, MeshStandardMaterial, Mesh } from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js'; // Adjust the local path accordingly
import { createSculptureWithGeometry } from 'shader-park-core';  // Local import from node_modules
import { spCode } from './sp-code.js';  // Local import for custom Shader Park code

let scene = new Scene();

let camera = new PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
camera.position.z = 1.5;

let renderer = new WebGLRenderer({ antialias: true, transparent: true });
renderer.setSize( window.innerWidth, window.innerHeight );
renderer.setPixelRatio( window.devicePixelRatio );
renderer.setClearColor( new Color(1, 1, 1), 0);
document.body.appendChild( renderer.domElement );

let clock = new Clock();

// AUDIO
const listener = new AudioListener();
camera.add(listener);

const sound = new Audio(listener);

let button = document.querySelector('.button');
button.innerHTML = "Loading Audio...";

// Load a sound and set it as the Audio object's buffer
const audioLoader = new AudioLoader();
audioLoader.load('https://cdn.glitch.global/59b80ec2-4e5b-4b54-b910-f3441cac0fd6/OP1Beat.wav?v=1667175863547', function(buffer) {
  sound.setBuffer(buffer);
  sound.setLoop(true);
  sound.setVolume(0.5);
  button.innerHTML = "Play Audio";
  button.addEventListener('pointerdown', () => {
    sound.play();
    button.style.display = 'none';
  }, false);
});

// Create an AudioAnalyser and get frequency data
const analyser = new AudioAnalyser(sound, 32);

let state = {
  mouse: new Vector3(),
  currMouse: new Vector3(),
  pointerDown: 0.0,
  currPointerDown: 0.0,
  audio: 0.0,
  currAudio: 0.0,
  time: 0.0
};

window.addEventListener('pointermove', (event) => {
  state.currMouse.x = (event.clientX / window.innerWidth) * 2 - 1;
  state.currMouse.y = - (event.clientY / window.innerHeight) * 2 + 1;
}, false);

window.addEventListener('pointerdown', (event) => state.currPointerDown = 1.0, false);
window.addEventListener('pointerup', (event) => state.currPointerDown = 0.0, false);

let geometry = new SphereGeometry(2, 45, 45);

// Create Shader Park Sculpture
let mesh = createSculptureWithGeometry(geometry, spCode(), () => ({
  time: state.time,
  pointerDown: state.pointerDown,
  audio: state.audio,
  mouse: state.mouse,
  _scale: .5
}));

scene.add(mesh);

// Add Controls
let controls = new OrbitControls(camera, renderer.domElement, {
  enableDamping: true,
  dampingFactor: 0.25,
  zoomSpeed: 0.5,
  rotateSpeed: 0.5
});

let onWindowResize = () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
};

window.addEventListener('resize', onWindowResize);

let render = () => {
  requestAnimationFrame(render);
  state.time += clock.getDelta();
  controls.update();
  
  if (analyser) {
    state.currAudio += Math.pow((analyser.getFrequencyData()[2] / 255) * .81, 8) + clock.getDelta() * .5;
    state.audio = .2 * state.currAudio + .8 * state.audio;
  }
  
  state.pointerDown = .1 * state.currPointerDown + .9 * state.pointerDown;
  state.mouse.lerp(state.currMouse, .05);
  renderer.render(scene, camera);
};

render();
