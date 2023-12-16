const audioSourceSelector = document.getElementById('audioInput');
const audioDestinationSelector = document.getElementById('audioOutput');
const selectors = [audioSourceSelector, audioDestinationSelector];
const audioElement = document.querySelector("audio");

const startButtonElement = document.querySelector("#startStream");
const stopButtonElement = document.querySelector("#stopStream");
const refreshButtonElement = document.querySelector("#refreshProperties");

const userName = document.querySelector("#username").textContent;
const userID = document.querySelector("#userid").textContent;

audioSourceSelector.onchange = changeInput
audioDestinationSelector.onchange = changeOutput

let peerConnection = null;
let ws = null
let audioStream = null;
var isStreaming = false;

// Placeholder for connected users
let connectedUsers = [];

// Placeholder for audio settings
let audioSettings = {
    audioInput: null,
    audioOutput: null,
    volume: 50
};

// Placeholder for previous audio settings
let previousAudioSettings = {};

function isFirefox(){
    return navigator.userAgent.search("Firefox") > -1
}

// Function to update audio settings based on form inputs
function updateAudioSettings() {
    audioSettings.audioInput = audioSourceSelector.value;
    audioSettings.audioOutput = audioDestinationSelector.value;
    audioSettings.volume = document.getElementById('volume').value;
}

function getMicStream(stream){
    console.log(stream.getTracks());
    isStreaming = true
    audioStream = stream

    //request peer connection
    requestVoice(ws, userID);

    // audioElement.srcObject = stream
}

function changeInput(){
    if(isStreaming && audioStream){
        console.log("changing input");
    }
}

function changeOutput(){
    updateAudioSettings()
    if(audioSettings.audioOutput){
        navigator.mediaDevices.enumerateDevices().then((devices) => {
            const audioDevice = devices.find((device) => device.deviceId === audioSettings.audioOutput);
            audioElement.setSinkId(audioDevice.deviceId)
        })
        console.log(`Changing output to ${audioSettings.audioOutput}`);
    }
}

// Function to start the connection
function startConnection() {
    updateAudioSettings();
    console.log(`Connecting as ${userName} with id ${userID}`);
    startButtonElement.disabled = true;

    //get mic stream
    navigator.mediaDevices.enumerateDevices().then((devices) => {
        const audioDevice = devices.find((device) => device.deviceId === audioSettings.audioInput);
        console.log(audioDevice); 
        constraints = {
            audio: true,
            video: false,
        };
        if(audioDevice != undefined){
            constraints = {
                audio: {deviceId: { exact: audioDevice.deviceId ? audioDevice.deviceId : undefined},"noiseSuppression":false, "echoCancellation":false, sampleRate: { exact: 48000}},
                video: false
            };
        }

        console.log(constraints);

        navigator.mediaDevices.getUserMedia(constraints).then(getMicStream).catch(err => { console.error(err); startButtonElement.disabled = false;});
    });

    // Start the connection here
    // alert('Connection started\n');
}

// Function to stop the connection
function stopConnection() {
    startButtonElement.disabled = false;
    isStreaming = false;
    audioStream.getTracks().forEach(track => {
        track.stop();
    });
    // Stop the connection here
    // alert('Connection stopped\n');
}

// Function to update properties
function updateProperties() {
    updateAudioSettings();

    const audioInputSelect = document.getElementById('audioInput');
    const audioOutputSelect = document.getElementById('audioOutput');
    const selectors = [audioSourceSelector, audioDestinationSelector]

    // Fetch and populate audio input devices
    navigator.mediaDevices.enumerateDevices()
        .then(devices => {
            audioSourceSelector.innerHTML = ''; // Clear existing options
            audioDestinationSelector.innerHTML = ''; // Clear existing options
            devices.forEach(device => {
                if (device.kind === 'audioinput') {
                    const option = document.createElement('option');
                    option.value = device.deviceId;
                    option.text = device.label || 'Microphone ' + (audioSourceSelector.options.length + 1);
                    audioSourceSelector.add(option);
                }
                if (device.kind === 'audiooutput') {
                    const option = document.createElement('option');
                    option.value = device.deviceId;
                    option.text = device.label || 'Speaker ' + (audioDestinationSelector.options.length + 1);
                    audioDestinationSelector.add(option);
                }
            });

            if(!audioSettings.audioInput){
                audioSettings.audioInput = audioSourceSelector.childNodes[0].value
            }

            if(!audioSettings.audioOutput){
                audioSettings.audioOutput = audioDestinationSelector.childNodes[0].value
            }

            // Set the selected value based on audioSettings
            audioSourceSelector.value = audioSettings.audioInput;
            audioDestinationSelector.value = audioSettings.audioOutput;
        })
        .catch(error => {
            console.error('Error fetching audio devices:', error);
        });

    // Display updated settings
    console.log(`Properties updated!\nAudio Input: ${audioSettings.audioInput}
        \nAudio Output: ${audioSettings.audioOutput}
        \nVolume: ${audioSettings.volume}`);
}

function updateConnectedUsers(users) {
    if(users === undefined)
        return;

    connectedUsers = users;
    const connectedUsersDiv = document.getElementById('connectedUsers');
    connectedUsersDiv.innerHTML = '<h2>Connected Users</h2>';
    
    if (connectedUsers.length > 0) {
        const userList = document.createElement('ul');
        connectedUsers.forEach(user => {
            const listItem = document.createElement('li');
            listItem.textContent = user;
            userList.appendChild(listItem);
        });
        connectedUsersDiv.appendChild(userList);
    } else {
        connectedUsersDiv.innerHTML += '<p>No users connected.</p>';
    }
}

// Example: Call updateConnectedUsers with a list of connected users
const exampleConnectedUsers = ['User1', 'User2', 'User3'];
// updateConnectedUsers(exampleConnectedUsers);

// delay permission request a little until the page is fully loaded
window.addEventListener('load', () => {
    navigator.mediaDevices.getUserMedia({audio: true}).then((stream)=>{
        updateProperties();
        stream.getTracks().forEach(track => {
            track.stop();
        });
        console.log("Permission Request");
    });

    //Connect to websocket 
    createConnection(userID).then((ws)=>{
        console.log("Websocket connected, requesting voice!");
        ws.onclose = () =>{
            console.warn('WebSocket disconnected');
            startButtonElement.disabled = false;
            stop();
        }
    }).catch((err) => {
        console.error(err);
        alert("Error while connecting to server, Please reload!")
        startButtonElement.disabled = false;
    });
});

setInterval(() => {
    if(ws && ws.readyState === WebSocket.OPEN){
        ws.send(JSON.stringify({
            id: userID,
            type: "users",
        }));
    }
}, 5000)

function requestVoice(ws, id){
    ws.send(JSON.stringify({
        id: "server",
        type: "request",
    }));
}

function createConnection(userID){
    return new Promise((resolve, reject) => {
        const url = `wss://${location.host}/${userID}`;
        if(!ws){
            ws = new WebSocket(url);
        } else {
            resolve(ws);
        }
        ws.onopen = () => resolve(ws);
        ws.onerror = () => reject(new Error('WebSocket error'));
        ws.onclose = () => console.error('WebSocket disconnected');
        ws.onmessage = (e) => {
            if (typeof (e.data) != 'string')
              return;
            const message = JSON.parse(e.data);
            console.log(message);
            type = message.type;

            if('error' in message){
                console.warn(`Internal error! ${message.error}`)
                if(message.error == "RECEIVER_NOT_CONNECTED"){
                    stopConnection();
                }
            }

            // pc = peerConnection
            // if(!pc){
            //     console.log("No peer connection, creating...")
            //     pc = peerConnection = createPeerConnection();
            // }

            switch(type) {
                case 'offer':
                    id = message.id;
                    if(!peerConnection){
                        console.log(`Received offer from ${id}, creating answer`)
                        peerConnection = createPeerConnection(ws, id);
                        peerConnection.setRemoteDescription(message)
                        audioStream.getTracks().forEach(track => peerConnection.addTrack(track, audioStream));
                        sendLocalDescription(ws, 'server', peerConnection, 'answer')
                    }

                    break;
                case 'answer':
                    pc.setRemoteDescription({
                        sdp: message.sdp,
                        type: message.type
                    });
                case 'users':
                    updateConnectedUsers(message.users);
            }

        }
    })
}

function createPeerConnection(ws) {
    const config = {
        bundlePolicy: "max-bundle",
        iceServers : [ {
            urls : 'stun:stun.l.google.com:19302', // change to your STUN server
          } ],

    }
    let pc = new RTCPeerConnection(config);
    pc.oniceconnectionstatechange = () => {
        console.log(`Connection state: ${pc.iceConnectionState}`);
        if(pc.iceConnectionState === 'disconnected'){
            stop();
        }
    }

    pc.onicegatheringstatechange = () => {
        console.log(`Gathering state: ${pc.iceGatheringState}`);
        // //temp text box sdp negotiation
        // if(pc.iceGatheringState === 'complete'){
        //     const ans = pc.localDescription;
        //     sdpInstuctionElement.value = "Paste answer into application."
        //     sdpTextBoxElement.value = JSON.stringify({type: ans.type, sdp: ans.sdp});
        //     alert(sdpInstuctionElement.value);
        // }
    }

    pc.onsignalingstatechange = () => console.log(`Signalling State: ${pc.signalingState}`);
    
    pc.ontrack = (evt) => {
        console.log("pc track event:", evt);
        audioElement.srcObject = evt.streams[0];
    }

    return pc
}


function sendLocalDescription(ws, id, pc, type) {
    (type == 'offer' ? pc.createOffer() : pc.createAnswer())
        .then((desc) => pc.setLocalDescription(desc))
        .then(() => {
          const {sdp, type} = pc.localDescription;
          ws.send(JSON.stringify({
            id,
            type,
            sdp : sdp,
          }));
        });
  }