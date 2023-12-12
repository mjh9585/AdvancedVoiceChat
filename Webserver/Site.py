from flask import Flask, render_template, request, session, redirect, url_for, json
from flask_sock import Sock, ConnectionClosed
import json

app = Flask(__name__)
app.config['SECRET_KEY'] = '61@sdL.Z=]tXnEZ'
app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

# In-memory storage for user sessions
users = {'server': {'username': 'server', 'ws' : None}}
#usernames_to_ws = {}  # Map of usernames to WebSocket connections


@app.route('/')
@app.route('/index')
def index():
    if 'user_id' not in session or session['user_id'] not in users:
        return redirect(url_for('login'))
    return render_template('index.html', username=session['username'], user_id=session['user_id'])

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username'].strip()
        userhash = str(hash(username))
        if(username == "server" or userhash in users):
            return render_template('login.html')
        
        session['username'] = username
        session['user_id'] = userhash  # Use a better method to generate user IDs
        print(f"User {session['user_id']} logged in with name '{username}'")
        users[session['user_id']] = {'username': username, 'ws': None}
        return redirect(url_for('index'))
    return render_template('login.html')

#! TEMPORARY DEBUG!!!
@app.route('/debug')
def debug():
    raise Exception("Forced Debug Page")


@sock.route('/<userid>')
def handle_ws(ws, userid):
    '''
    Handle Websocket connections.
    '''
    if (userid == 'server' and users['server']['ws'] is not None):
        print("WS: WARNING: Server Impersonation!")
        return
    elif (userid not in users or users[userid]['ws'] is not None):
        print(f"WS: invalid User {userid}!")
        print(users)
        return
    
    print(f"WS: User {userid}[{users[userid]['username']}] Connected!")
    
    # Add WebSocket connection to the map
    users[userid]['ws'] = ws
    while True:
        # Attempt te receive a message. when disconnected, remove ws connection from user
        try:
            message = ws.receive()
        except ConnectionClosed:
            print(f"WS: Connection Closed for {userid}[{users[userid]['username']}]!")
            users[userid]['ws'] = None
            return
        
        if message:
            try:
                parsedMessage = json.loads(message)
            except json.JSONDecodeError:
                # Handle invalid JSON
                continue

            receiver_id = parsedMessage.get('id')
            messageType = parsedMessage.get('type')

            if receiver_id in users:
                print(f"{userid}[{users[userid]['username']}] -> {receiver_id}[{users[receiver_id]['username']}]: {parsedMessage}")
                # Forward the message to the specified user
                receiver_ws = users[receiver_id]['ws']
                parsedMessage['id'] = userid
                # sender_message = {'id': userid, 'data': payload}
                if(receiver_ws is not None):
                    receiver_ws.send(json.dumps(parsedMessage))
                else:
                    print(f"WS: Requested user {receiver_id}[{users[receiver_id]['username']}] Not Connected!")
                    parsedMessage['error'] = 'RECEIVER_NOT_CONNECTED'
                    ws.send(json.dumps(parsedMessage))
            


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port="5000", ssl_context=('cert.pem', 'key.pem'))
