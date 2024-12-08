export function initializeMIDI(onMIDIMessage) {
    if (navigator.requestMIDIAccess) {
      navigator.requestMIDIAccess().then((midiAccess) => {
        for (let input of midiAccess.inputs.values()) {
          input.onmidimessage = onMIDIMessage;
        }
      }).catch(() => console.error('Could not access MIDI devices.'));
    } else {
      console.error('Web MIDI API not supported in this browser.');
    }
  }