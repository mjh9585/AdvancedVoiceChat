from flask import Flask, render_template,make_response
import os

app = Flask(__name__)
app.config.from_mapping({"BASE_DIR":os.path.abspath(os.path.dirname(__file__)), "DEBUG": True})

@app.after_request
def addPermissions(response):
    response.headers["Permissions-Policy"]="speaker-selection=(self), microphone=(self)"
    return response

@app.route("/")
def hello_world():
    resp = make_response(render_template("test.html"), 200)
    # resp.headers["Permissions-Policy"] = 
    return resp

app.run(host='0.0.0.0')#, ssl_context=('cert.pem', 'key.pem'))