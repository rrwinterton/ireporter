#!/usr/bin/python
print("Content-type: text/html\n\n")

import os
import json
from datetime import datetime
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# from werkzeug.middleware.proxy_fix import ProxyFix
# app.wsgi_app = ProxyFix(app.wsgi_app, x_for=1, x_proto=1, x_host=1, x_prefix=1)

# State management (In-memory for now)
STATE = {"enabled": True}

# Ensure the output directory 'ireportjson' exists
SAVE_DIR = os.path.join(os.path.dirname(__file__), 'ireportjson')
if not os.path.exists(SAVE_DIR):
    os.makedirs(SAVE_DIR)

@app.route('/')
@app.route('/ireporter/')
def index():
    """Serve the dashboard home page."""
    return render_template('index.html')

@app.route('/test')
@app.route('/ireporter/test')
@app.route('/api/test')
@app.route('/ireporter/api/test')
def test_route():
    return "TEST OK"

@app.route('/api/status', methods=['GET'])
@app.route('/ireporter/api/status', methods=['GET'])
def get_status():
    """Get the current enabled/disabled status."""
    return jsonify({"enabled": STATE["enabled"], "version": "v2"})

@app.route('/api/toggle', methods=['POST'])
@app.route('/ireporter/api/toggle', methods=['POST'])
def toggle_status():
    """Toggle the receiver state."""
    STATE['enabled'] = not STATE['enabled']
    return jsonify(STATE)

@app.route('/api/files', methods=['GET'])
@app.route('/ireporter/api/files', methods=['GET'])
def list_files():
    """List all received JSON files with their metadata."""
    files = []
    for filename in os.listdir(SAVE_DIR):
        if filename.endswith('.json'):
            path = os.path.join(SAVE_DIR, filename)
            stats = os.stat(path)
            files.append({
                "name": filename,
                "size": f"{stats.st_size / 1024:.2f} KB",
                "time": datetime.fromtimestamp(stats.st_mtime).strftime('%Y-%m-%d %H:%M:%S')
            })
    # Sort by time descending
    files.sort(key=lambda x: x['time'], reverse=True)
    return jsonify(files)

@app.route('/api/data', methods=['POST'])
@app.route('/ireporter/api/data', methods=['POST'])
def handle_client_data():
    """Handle incoming telemetry from the C++ client."""
    if not STATE['enabled']:
        return jsonify({"error": "Receiver is currently disabled"}), 503

    if request.is_json:
        data = request.get_json()
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        filename = f"report_{timestamp}.json"
        filepath = os.path.join(SAVE_DIR, filename)

        try:
            with open(filepath, 'w') as f:
                json.dump(data, f, indent=4)
            print(f"[SERVER] Received telemetry: {filename}")
            return jsonify({"status": "success", "file_saved": filename}), 200
        except Exception as e:
            print(f"[SERVER] Error saving data: {e}")
            return jsonify({"status": "error", "message": str(e)}), 500
    else:
        return jsonify({"error": "Request must be JSON"}), 400

@app.route('/upload', methods=['POST'])
@app.route('/ireporter/upload', methods=['POST'])
@app.route('/ireporter/upload/', methods=['POST'])
@app.route('/api/upload', methods=['POST'])
@app.route('/ireporter/api/upload', methods=['POST'])
def handle_upload():
    """Handle raw file uploads."""
    if not STATE['enabled']:
        return jsonify({"error": "Receiver is currently disabled"}), 503

    # Ensure the upload directory exists
    UPLOAD_DIR = os.path.join(os.path.dirname(__file__), 'upload')
    if not os.path.exists(UPLOAD_DIR):
        os.makedirs(UPLOAD_DIR)

    try:
        # For raw binary uploads, we can read the data directly
        data = request.get_data()
        if not data:
            return jsonify({"error": "No data received"}), 400
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        # Try to get filename from headers or use a default
        filename = f"upload_{timestamp}.bin"
        filepath = os.path.join(UPLOAD_DIR, filename)
        
        with open(filepath, 'wb') as f:
            f.write(data)
        
        print(f"[SERVER] Received upload: {filename} ({len(data)} bytes)")
        return jsonify({"status": "success", "file_saved": filename, "size": len(data)}), 200
    except Exception as e:
        cwd = os.getcwd()
        print(f"[SERVER] Error saving upload: {e} (CWD: {cwd})")
        return jsonify({"status": "error", "message": f"{str(e)} (CWD: {cwd})"}), 500

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)


