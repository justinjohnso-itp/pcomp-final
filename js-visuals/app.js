import {
  Scene,
  PerspectiveCamera,
  WebGLRenderer,
  Color,
  SphereGeometry,
} from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { createSculpture, createSculptureWithGeometry } from "shader-park-core";
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass';
import { spCode } from "./spCode.js";

// Scene setup
let scene = new Scene();
let state = { time: 0 };

// Camera setup
let camera = new PerspectiveCamera(
  15,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
camera.position.z = 1.5;

// Render setup
let renderer = new WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio(window.devicePixelRatio);
renderer.setClearColor(new Color(1, 1, 1), 1);
document.body.appendChild(renderer.domElement);

// Contrast adjustment
const ContrastShader = {
    uniforms: {
        "tDiffuse": { value: null },
        "contrast": { value: 1 }  // Adjust this value to change contrast (1.0 is normal)
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
            
            // Convert to linear space
            color.rgb = color.rgb * color.rgb;
            
            // Adjust contrast
            color.rgb = (color.rgb - 0.5) * contrast + 0.5;
            
            // Ensure values stay in valid range
            color.rgb = clamp(color.rgb, 0.0, 1.0);
            
            // Convert back to gamma space
            color.rgb = sqrt(color.rgb);
            
            gl_FragColor = color;
        }
    `
};

// Post-processing setup
const composer = new EffectComposer(renderer);
const renderPass = new RenderPass(scene, camera);
const contrastPass = new ShaderPass(ContrastShader);
composer.addPass(renderPass);
composer.addPass(contrastPass);

// Shader Park Setup
let geometry = new SphereGeometry(1, 45, 45);
let mesh = createSculptureWithGeometry(geometry, spCode, () => ({
  time: state.time,
}));
scene.add(mesh);

// Controls setup
let controls = new OrbitControls(camera, renderer.domElement, {
  enableDamping: true,
  dampingFactor: 0.25,
  zoomSpeed: 0.5,
  rotateSpeed: 0.5,
});

// Camera rotation variables
let radius = 3;
let angle = 1;

// window resize
window.addEventListener('resize', () => {
    const width = window.innerWidth;
    const height = window.innerHeight;
    
    camera.aspect = width / height;
    camera.updateProjectionMatrix();
    
    renderer.setSize(width, height);
    composer.setSize(width, height);
});

//  contrast control
let adjustContrast = (value) => {
    contrastPass.uniforms.contrast.value = value;
};

// Render loop
let render = () => {
  requestAnimationFrame(render);
  state.time += 0.01;

  // Move the sphere up and down over time
  mesh.position.y = Math.sin(state.time) * 0.2;
  
  // Rotate camera over time
  angle += 0.005;
  camera.position.x = radius * Math.cos(angle);
  camera.position.y = radius * Math.sin(angle);

  camera.lookAt(0, 0, 0);
  controls.update();
  
  composer.render();
};

render();