from flask import Flask, render_template, request, session, redirect, url_for
from flask import SocketIO, emit
from flask_sock import Sock

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret_key'
sock = Sock(app)

# In-memory storage for user sessions
users = {}


@app.route('/', '/index')
def index():
    if 'username' not in session:
        return redirect(url_for('/login'))
    return render_template('index.html', username=session['username'], user_id=session['user_id'])


@app.route('/login', methods=['POST'])
def login():
    username = request.form['username']
    session['username'] = username
    session['user_id'] = hash(username)  # Use a better method to generate user IDs
    users[session['user_id']] = session['username']
    return redirect(url_for('/index'))


'''@app.route('/group')
def group():
    if 'username' not in session:
        return redirect(url_for('index'))
    return render_template('group.html', username=session['username'], user_id=session['user_id'])
'''


@sock.route('/ws')
def echo(ws):
    # emit('/ws', {'username': session['username'], '/ws': msg})
    while True:
        data = ws.receive()
        ws.send(data)


if __name__ == '__main__':
    app.run()
    # sock.run(app, debug=True)
