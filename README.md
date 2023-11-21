# AdvancedVoiceChat
An advanced group voice chat utilizing WebRTC. This voice chat makes use of a routing matrix to dynamically control who can hear who.

## Prerequisites
The project is written to run on a linux server so you will need some sort of linux environment to build and test. I recommend setting up WSL if you are working on Windows. 

### Tools
- cmake > 3.18
- python > 3.8

To setup the project run :
```bash
python3 -m venv venv
source venv/bin/activate
pip install requirements.txt
```
