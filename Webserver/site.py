from flask import Flask, render_template, request, session, redirect, url_for
from flask_sock import Sock

app = Flask(__name__)
app.config['SECRET_KEY'] = '61@sdL.Z=]tXnEZ'
app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

# In-memory storage for user sessions
users = {}


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


@sock.route('/ws/<user_id>')
def echo(ws, user_id):
    while True:
        data = ws.receive()
        while True:
            data = ws.receive()
            ws.send(data)
        '''Started work on having the received message actually forwarded to the requested client but could not figure
            out how to correctly do so. get_ws_by_room() does not exist (thanks ChatGPT) but I can't find the substitute
            for the "emit" function in Flask-SocketIO for the simple Flask-Sock library we are using.
        # Iterate through connected clients and send the message individually
        for client_id, client_username in users.items():
            if client_id != user_id:  # Skip sending the message to the sender
                connected_client_ws = sock.get_ws_by_room(str(client_id))
                connected_client_ws.send(data)'''


if __name__ == '__main__':
    app.run(debug=True)
