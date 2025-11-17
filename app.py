from flask import Flask, request, jsonify, render_template
import subprocess
import socket
import os

app = Flask(__name__)

LOCAL_SHELL = "./os_lab_toolkit"

REMOTE_IP = "192.168.0.125"
REMOTE_PORT = 8080
USERNAME = b"user"
PASSWORD = b"pass123"

def safe_get_json(req):
    try:
        return req.get_json(force=True, silent=True)
    except:
        return None

def recv_until(sock, marker=b"---END---"):
    data = b""
    while marker not in data:
        chunk = sock.recv(2048)
        if not chunk:
            break
        data += chunk
    return data

def execute_remote_command(cmd):
    try:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.settimeout(3)
        client.connect((REMOTE_IP, REMOTE_PORT))

        # Login sequence
        client.recv(1024)
        client.sendall(USERNAME + b"\n")

        client.recv(1024)
        client.sendall(PASSWORD + b"\n")

        login_resp = recv_until(client)
        if b"failed" in login_resp.lower():
            return {"status": "error", "output": "Remote authentication failed"}

        # Send command
        client.sendall(cmd.encode() + b"\n")

        response = recv_until(client).decode(errors='ignore')
        output = response.split("---END---")[0].strip()

        client.close()
        return {"status": "success", "output": output}

    except Exception as e:
        return {"status": "error", "output": f"Remote error: {str(e)}"}

def execute_local_command(cmd):
    try:
        if not os.path.exists(LOCAL_SHELL):
            return {"status": "error", "output": "Local shell not found"}

        # Run C shell
        process = subprocess.Popen(
            [LOCAL_SHELL],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        stdout, stderr = process.communicate(cmd + "\nexit\n", timeout=5)

        # Remove the $ prompt
        if "$ " in stdout:
            stdout = stdout.split("$ ", 1)[1]

        # Remove exit message
        stdout = stdout.replace("Exiting UniShell...", "").strip()

        if stderr.strip():
            return {"status": "error", "output": stderr.strip()}

        return {"status": "success", "output": stdout}

    except subprocess.TimeoutExpired:
        process.kill()
        return {"status": "error", "output": "Local command timed out"}
    except Exception as e:
        return {"status": "error", "output": f"Local error: {str(e)}"}

@app.route("/")
def index():
    return render_template("index.html")



@app.route("/api/execute", methods=["POST"])
def execute():
    data = safe_get_json(request)

    if not data or "command" not in data:
        return jsonify({"status": "error", "output": "Invalid request"}), 400

    command = data["command"].strip()
    mode = data.get("mode", "local")

    if mode == "remote":
        result = execute_remote_command(command)
    else:
        result = execute_local_command(command)

    return jsonify(result)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5002, debug=False)
