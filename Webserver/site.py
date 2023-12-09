from flask import Flask, render_template, request, session, redirect, url_for, json
from flask_sock import Sock
import json

app = Flask(__name__)
app.config['SECRET_KEY'] = '61@sdL.Z=]tXnEZ'
app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

# In-memory storage for user sessions
users = {}
#usernames_to_ws = {}  # Map of usernames to WebSocket connections


@app.route('/')
@app.route('/index')
def index():
    if 'username' not in session:
        return redirect(url_for('login'))
    return render_template('index.html', username=session['username'], user_id=session['user_id'])


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        session['username'] = username
        session['user_id'] = hash(username)  # Use a better method to generate user IDs
        users[session['user_id']] = session['username']
        return redirect(url_for('index'))
    return render_template('login.html')


@sock.route('/ws')
def handle_ws(ws):
    if 'username' not in session or session['username'] == 'server':
        return

        # Add WebSocket connection to the map
    users[session['username']] = ws
    while not ws.closed:
        message = ws.receive()
        if message:
            try:
                data = json.loads(message)
                receiver_id = data.get('id')
                payload = data.get('data')

                if receiver_id in users:
                    # Forward the message to the specified user
                    receiver_ws = users[receiver_id]
                    sender_username = session['username']
                    sender_message = {'id': sender_username, 'data': payload}
                    receiver_ws.send(json.dumps(sender_message))
            except json.JSONDecodeError:
                # Handle invalid JSON
                pass


if __name__ == '__main__':
    app.run(debug=True)
