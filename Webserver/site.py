from flask import Flask, render_template, request, session, redirect, url_for
from flask import SocketIO, emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret_key'
socketio = SocketIO(app)

# In-memory storage for user sessions
users = {}


@app.route('/')
def index():
    if 'username' not in session:
        return redirect(url_for('login'))
    return render_template('index.html', username=session['username'], user_id=session['user_id'])


@app.route('/login', methods=['POST'])
def login():
    username = request.form['username']
    session['username'] = username
    session['user_id'] = hash(username)  # Use a better method to generate user IDs
    users[session['user_id']] = session['username']
    return redirect(url_for('index'))


'''@app.route('/group')
def group():
    if 'username' not in session:
        return redirect(url_for('index'))
    return render_template('group.html', username=session['username'], user_id=session['user_id'])
'''


@socketio.on('message')
def handle_message(msg):
    emit('message', {'username': session['username'], 'message': msg})


if __name__ == '__main__':
    socketio.run(app, debug=True)
