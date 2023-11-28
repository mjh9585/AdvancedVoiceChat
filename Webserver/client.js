// Page Elements
const audioSourceSelector = document.querySelector("select#audioSource");
const audioDestinationSelector = document.querySelector("select#audioDestination");
const selectors = [audioSourceSelector, audioDestinationSelector];
const audioElement = document.querySelector("audio");

const startButton = document.querySelector("#startStream");
const stopButton = document.querySelector("#stopStream");
const refreshProperties = document.querySelector("#refreshProperties");

//vars
var audioStream = null;
var isStreaming = false;

//Audio Device Selection
function updateDevices(devices){
    const deviceList = document.querySelector("#deviceList");
    const values = selectors.map(select => select.value);
    selectors.forEach(select => {
        while (select.firstChild) {
        select.removeChild(select.firstChild);
        }
    });

    // console.log("List of Devices:");
    deviceList.textContent=""; 

    devices.forEach((device) => {
        // console.log(`\t${device.kind}: ${device.label}, id='${device.label}'`)
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
    const audioSourceProp = document.querySelector("#audioSourceProp");
    const audioDestinationProp = document.querySelector("#audioDestinationProp");
    
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
        audioDestinationProp.append(`deviceId=${audioDevice.deviceId}\n`)
        audioDestinationProp.append(`groupId=${audioDevice.groupId}\n`)
        audioDestinationProp.append(`kind=${audioDevice.kind}\n`)
        audioDestinationProp.append(`label=${audioDevice.label}\n`)
    });
}

function getStream(stream) {
    console.log(stream.getTracks());
    audioStream = stream
    audioElement.srcObject = stream;

    displayAudioProperties();
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

        navigator.mediaDevices.getUserMedia(constraints).then(getStream).then(updateDevices).catch(handleError);
    });
    isStreaming = true;
    changeOutput();
}

//Page Setup
navigator.mediaDevices.enumerateDevices().then(updateDevices).catch(handleError);

//Element Event handlers
audioSourceSelector.onchange = changeInput;
audioDestinationSelector.onchange = changeOutput;

startButton.onclick = start;
stopButton.onclick = stop;
refreshProperties.onclick = displayAudioProperties;