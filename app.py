from flask import Flask, request, jsonify
import subprocess
import os
import json

app = Flask(__name__)

# Path to compiled C program
TOOL_PATH = "./os_lab_toolkit"

# Safe JSON input
def safe_get_json(req):
    try:
        return req.get_json(force=True, silent=False)
    except Exception:
        return None

@app.route("/run", methods=["POST"])
def run_command():
    data = safe_get_json(request)
    
    if not data or "command" not in data:
        return jsonify({"status": "error", "output": "Invalid or missing command"}), 400

    command = data["command"].strip()

    if not command:
        return jsonify({"status": "error", "output": "Empty command"}), 400

    # Ensure toolkit exists
    if not os.path.exists(TOOL_PATH):
        return jsonify({"status": "error", "output": "Toolkit not found"}), 500

    try:
        process = subprocess.Popen(
            [TOOL_PATH, command],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        stdout, stderr = process.communicate()

        if stderr.strip():
            return jsonify({"status": "error", "output": stderr})

        # stdout from C is already JSON
        return app.response_class(stdout, mimetype="application/json")

    except Exception as e:
        return jsonify({"status": "error", "output": f"Execution failed: {str(e)}"})


@app.route("/")
def home():
    return "UniShell backend running."

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5002, debug=False)
