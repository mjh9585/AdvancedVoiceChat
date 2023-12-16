# AdvancedVoiceChat
An advanced group voice chat utilizing WebRTC. The project consist of two main components, a backend web server written in Python and a backend voice server written in C++. The voice server makes use of an audio routing matrix to dynamically connect several users together. 

## Prerequisites
The project is written to run on a linux server so you will need some sort of linux environment to build and test. I recommend setting up WSL if you are working on Windows. 

### Tools
- cmake > 3.18
- python > 3.8

### Web Server Setup
To setup the web server run:
```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
cd Webserver/
openssl req -x509 -newkey rsa:4096 -nodes -out cert.pem -keyout key.pem -days 365
```

The web server for the front end can now be ran from the "Webserver/" directory with `python Site.py`.

### Voice Server Setup

To be able to run the backend voice server, the required libraries are included as submodules which need to be downloaded and built along with the server. Run the following commands to setup and build the voice backend: 

```bash
git submodule update --init --recursive
mkdir build
cd build/
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The server can then ran from the build directory with `./server`. 