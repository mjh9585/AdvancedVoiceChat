// ################################
// #####     Page Elements     ####
// ################################
const audioSourceSelector = document.querySelector("select#audioSource");
const audioDestinationSelector = document.querySelector("select#audioDestination");
const selectors = [audioSourceSelector, audioDestinationSelector];
const audioElement = document.querySelector("audio");

const startButton = document.querySelector("#startStream");
const stopButton = document.querySelector("#stopStream");
const refreshProperties = document.querySelector("#refreshProperties");

const sdpInstuctionElement = document.querySelector("#sdpInstuctions");
const sdpTextBoxElement = document.querySelector("#sdpTextBox");

//debug device elements:
const deviceList = document.querySelector("#deviceList");
const audioSourceProp = document.querySelector("#audioSourceProp");
const audioDestinationProp = document.querySelector("#audioDestinationProp");

//debug WebRTC elements
const iceConnectionLog = document.querySelector('#ice-connection-state');
const iceGatheringLog = document.querySelector('#ice-gathering-state');
const signalingLog = document.querySelector('#signaling-state');
const dataChannelLog = document.querySelector('#data-channel');

//vars
var audioStream = null;
var isStreaming = false;
let pc = null;

// #####################################
// #####   Device Selection Menus   ####
// #####################################
//Audio Device Selection
function updateDevices(devices){
    const values = selectors.map(select => select.value);
    selectors.forEach(select => {
        while (select.firstChild) {
        select.removeChild(select.firstChild);
        }
    });

    // console.log("List of Devices:");
    deviceList.textContent=""; 

    devices.forEach((device) => {
        console.log(`\t${device.kind}: ${device.label}, id='${device.label}'`)
        deviceList.append(`${device.kind}: ${device.label}, id='${device.label}'\n`);

        const option = document.createElement("option");
        if(device.kind === "audioinput"){
            option.text = device.label || `microphone ${audioSourceSelector.length+1}`
            audioSourceSelector.appendChild(option)
        } else if(device.kind === "audiooutput"){
            option.text = device.label || `speaker ${audioSourceSelector.length+1}`
            audioDestinationSelector.appendChild(option)
        } else if(device.kind === "videoinput"){

        } else {
            console.log('Some other kind of source/device: ', device);
        }
    })

    selectors.forEach((select, selectorIndex) => {
        if (Array.prototype.slice.call(select.childNodes).some(n => n.value === values[selectorIndex])) {
            select.value = values[selectorIndex];
        }
    });

    displayAudioProperties();
}

function displayAudioProperties(){
    //Audio Inputs
    if(audioStream){
        audioSourceProp.textContent = ""
        audioSourceProp.append(`active=${audioStream.active}\n`)
        audioSourceProp.append(`id=${audioStream.id}\n`)

        audioStream.getTracks().forEach((track, num)=>{
            audioSourceProp.append(`MediaStreamTrack #${num}:\n`)
            audioSourceProp.append(`\tid=${track.id}\n`)
            audioSourceProp.append(`\tkind=${track.kind}\n`)
            audioSourceProp.append(`\tlabel=${track.label}\n`)
            audioSourceProp.append(`\tmuted=${track.muted}\n`)
            audioSourceProp.append(`\treadyState=${track.readyState}\n`)
            settings = JSON.stringify(track.getSettings(), null, 2)
            settings = settings.replaceAll("\n", "\n\t\t")
            audioSourceProp.append(`\tsettings=${settings}\n`)
        })
    }else{
        audioSourceProp.textContent = "No active Streams"
    }

    //Audio Outputs
    const audioDest = audioDestinationSelector.value;
    
    navigator.mediaDevices.enumerateDevices().then((devices) => {
        const audioDevice = devices.find((device) => device.label === audioDest);

        audioDestinationProp.textContent = ""
        
        if(audioDevice){
            audioDestinationProp.append(`deviceId=${audioDevice.deviceId}\n`)
            audioDestinationProp.append(`groupId=${audioDevice.groupId}\n`)
            audioDestinationProp.append(`kind=${audioDevice.kind}\n`)
            audioDestinationProp.append(`label=${audioDevice.label}\n`)
        }
    });
}

// #######################################
// #####   WebRTC Connection/Stream   ####
// #######################################
function createPeerConnection(){
    const config = {
        bundlePolicy: "max-bundle",
    }
    let pc = new RTCPeerConnection(config);

    pc.oniceconnectionstatechange = () => {
        iceConnectionLog.textContent += ' -> ' + pc.iceConnectionState;
    }
    iceConnectionLog.textContent = pc.iceConnectionState;

    //TODO replace with websocket!
    pc.onicegatheringstatechange = () => {
        //temp text box sdp negotiation
        if(pc.iceGatheringState === 'complete'){
            const ans = pc.localDescription;
            sdpInstuctionElement.value = "Paste answer into application."
            sdpTextBoxElement.value = JSON.stringify({type: ans.type, sdp: ans.sdp});
            alert(sdpInstuctionElement.value);
        }

        iceGatheringLog.textContent += ' -> ' + pc.iceGatheringState;
    }
    iceGatheringLog.textContent = pc.iceGatheringState;

    pc.onsignalingstatechange = () => {
        signalingLog.textContent += ' -> ' + pc.signalingState
    }
    signalingLog.textContent = pc.signalingState;

    pc.ontrack = (evt) => {
        console.log("pc track event:", evt);
        audioElement.srcObject = evt.streams[0];
    }

    return pc
}

async function getStream(stream) {
    console.log(stream.getTracks());
    audioStream = stream
    // audioElement.srcObject = stream;

    displayAudioProperties();

    //setup WebRTC
    pc = createPeerConnection()

    //receive offer
    const offer = JSON.parse(sdpTextBoxElement.value);
    document.getElementById('offer-sdp').textContent = offer.sdp;
    await pc.setRemoteDescription(offer);

    //add audio from device
    stream.getTracks().forEach(track => pc.addTrack(track, stream));

    //send answer
    const answer = await pc.createAnswer();
    pc.setLocalDescription(answer);

    document.getElementById('answer-sdp').textContent = answer.sdp;
    return navigator.mediaDevices.enumerateDevices();
}

function handleError(error) {
    console.log("Media error: ", error.name, error.message);
    if(error.name === "OverconstrainedError"){
        console.log(error.constraint)
    }
}


//User Interaction Handlers
function changeOutput(){
    const audioDest = audioDestinationSelector.value;
    
    navigator.mediaDevices.enumerateDevices().then((devices) => {
        const audioDevice = devices.find((device) => device.label === audioDest);
        audioElement.setSinkId(audioDevice.deviceId)
    })
    displayAudioProperties();
}

function changeInput(){
    if(isStreaming){
        start();
    }
}

function stop(){
    if(audioStream){
        audioStream.getTracks().forEach(track => {
            track.stop();
        });
    }
    isStreaming = false;
    displayAudioProperties();
}

function start(){
    if(audioStream){
        audioStream.getTracks().forEach(track => {
            track.stop();
        });
    }

    const audioSource = audioSourceSelector.value;

    navigator.mediaDevices.enumerateDevices().then((devices) => {
        const audioDevice = devices.find((device) => device.label === audioSource);
        // console.log(audioDevice.deviceId); 

        const constraints = {
            audio: {deviceId: { exact: audioDevice.deviceId ? audioDevice.deviceId : undefined},"noiseSuppression":false, "echoCancellation":false, sampleRate: { exact: 48000}},
            video: false
        };

        console.log(constraints);

        navigator.mediaDevices.getUserMedia(constraints).then(getStream).then(updateDevices);//.catch(handleError);
    });
    isStreaming = true;
    changeOutput();
}

//prompt permissions:
navigator.mediaDevices.getUserMedia({audio: true}).then((stream)=>{stream.getTracks().forEach(track => {
    track.stop();
})});
//Page Setup
navigator.mediaDevices.enumerateDevices().then(updateDevices).catch(handleError);

//Element Event handlers
audioSourceSelector.onchange = changeInput;
audioDestinationSelector.onchange = changeOutput;

function refreshDevices(){
    navigator.mediaDevices.enumerateDevices().then(updateDevices)
}


startButton.onclick = start;
stopButton.onclick = stop;
refreshProperties.onclick = refreshDevices;