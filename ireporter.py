from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/api/data', methods=['POST'])
def handle_client_data():
    # Ensure the incoming request contains JSON
    if request.is_json:
        data = request.get_json()
        print(f"Received data from Windows client: {data}")
        
        # Respond back to the client with new instructions
        response_payload = {
            "status": "success",
            "command": "continue_monitoring"
        }
        return jsonify(response_payload), 200
    else:
        return jsonify({"error": "Request must be JSON"}), 400

if __name__ == '__main__':
    # Run the server on port 5000
    app.run(host='0.0.0.0', port=5000)
