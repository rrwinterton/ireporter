import os
import json
from datetime import datetime
from flask import Flask, request, jsonify

app = Flask(__name__)

# Ensure the output directory 'ireportjson' exists
SAVE_DIR = os.path.join(os.path.dirname(__file__), 'ireportjson')
if not os.path.exists(SAVE_DIR):
    os.makedirs(SAVE_DIR)

@app.route('/api/data', methods=['POST'])
def handle_client_data():
    # Ensure the incoming request contains JSON
    if request.is_json:
        data = request.get_json()
        print(f"Received data from Windows client: {data}")
        
        # Create a unique filename based on the current timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        filename = f"report_{timestamp}.json"
        filepath = os.path.join(SAVE_DIR, filename)

        # Save the JSON data to a file in the 'ireportjson' directory
        try:
            with open(filepath, 'w') as f:
                json.dump(data, f, indent=4)
            print(f"Data saved to {filepath}")
        except Exception as e:
            print(f"Error saving data: {e}")
            return jsonify({"status": "error", "message": str(e)}), 500

        # Respond back to the client
        response_payload = {
            "status": "success",
            "file_saved": filename,
            "command": "continue_monitoring"
        }
        return jsonify(response_payload), 200
    else:
        return jsonify({"error": "Request must be JSON"}), 400

if __name__ == '__main__':
    # Use the port provided by HttpPlatformHandler (via the PORT env variable)
    # or default to 5000 for local development.
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)
