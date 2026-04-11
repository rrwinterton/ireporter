# ireporter

A telemetry reporter tool with a high-performance C++ client and a Python-based Flask receiver.

## Project Structure
- `ireporter.cpp`: C++ client that collects and sends telemetry data via WinHTTP.
- `ireporter.py`: Flask-based receiver that saves telemetry data and serves the dashboard.
- `templates/index.html`: Web dashboard UI for monitoring and control.
- `ireportjson/`: Directory where received telemetry JSON files are stored.
- `web.config`: Configuration for hosting the Python receiver on Windows IIS.

## Web Dashboard
The project includes a built-in web dashboard accessible at the root URL (e.g., `http://localhost:5000`).

### Features:
- **Telemetry Monitoring**: Automatically lists all received JSON files with their upload timestamps and file sizes.
- **Receiver Control**: Includes an **Enable/Disable** toggle.
  - **Enabled**: The server accepts and saves incoming POST requests from the C++ client.
  - **Disabled**: The server rejects incoming telemetry with a `503 Service Unavailable` error, allowing you to pause data collection without stopping the server process.
- **Auto-Refresh**: The file list refreshes every 10 seconds to show new uploads in real-time.

## Prerequisites
- **Python 3.x**
- **Flask**: Install via `pip install flask`

---

## Server-Side Setup

### 1. Windows Server (IIS)
To host the receiver on IIS, you must use the **HttpPlatformHandler** module.

1. **Install HttpPlatformHandler**: Download and install it from the official Microsoft site.
2. **Configure IIS**:
   - Create a new Virtual Application named `ireporter` under your Website pointing to the `ireporter` directory.
   - Ensure the `web.config` file is present in the root of the `ireporter` directory.
   - **Permissions**: Grant the IIS AppPool user (e.g., `IIS AppPool\DefaultAppPool`) **Full Control** over the `ireporter` folder.
3. **Verify**: Access the dashboard at `http://localhost/ireporter/`. The receiver will automatically start on a port assigned by IIS.

### 2. Linux Server (Nginx + Gunicorn)
On Linux, it is recommended to use Gunicorn as the application server and Nginx as a reverse proxy.

1. **Install Gunicorn**: `pip install gunicorn`
2. **Create a Systemd Service**:
   Create `/etc/systemd/system/ireporter.service`:
   ```ini
   [Unit]
   Description=Gunicorn instance to serve ireporter
   After=network.target

   [Service]
   User=youruser
   Group=www-data
   WorkingDirectory=/path/to/ireporter
   Environment="PATH=/path/to/ireporter/venv/bin"
   ExecStart=/path/to/ireporter/venv/bin/gunicorn --bind 0.0.0.0:5000 ireporter:app

   [Install]
   WantedBy=multi-user.target
   ```
3. **Configure Nginx**:
   Add a `proxy_pass` to your Nginx site configuration:
   ```nginx
   location /api/data {
       proxy_pass http://localhost:5000;
       proxy_set_header Host $host;
       proxy_set_header X-Real-IP $remote_addr;
   }
   ```
4. **Start the Service**: `systemctl enable --now ireporter`

---

## Data Storage
All telemetry data received by the server is stored in the `ireportjson/` directory. Each report is saved as a unique JSON file named with a timestamp (e.g., `report_20231027_103000.json`).

## Client Build Instructions
See `GEMINI.md` for detailed instructions on building the C++ client using CMake and Clang/Ninja.
