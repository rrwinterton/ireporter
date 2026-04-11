# ireporter

A telemetry reporter tool with a high-performance C++ client and a Python-based Flask receiver.

## Project Structure
- `ireporter.cpp`: C++ client that collects and sends telemetry data via WinHTTP.
- `ireporter.py`: Flask-based receiver that saves telemetry data as JSON files.
- `ireportjson/`: Directory where received telemetry JSON files are stored.
- `web.config`: Configuration for hosting the Python receiver on Windows IIS.

## Prerequisites
- **Python 3.x**
- **Flask**: Install via `pip install flask`

---

## Server-Side Setup

### 1. Windows Server (IIS)
To host the receiver on IIS, you must use the **HttpPlatformHandler** module.

1. **Install HttpPlatformHandler**: Download and install it from the official Microsoft site.
2. **Configure IIS**:
   - Create a new Website or Application in IIS pointing to the `ireporter` directory.
   - Ensure the `web.config` file is present in the root.
   - **Permissions**: Grant the IIS AppPool user (e.g., `IIS AppPool\YourSiteName`) **Write** permissions to the `ireporter` folder so it can create the `ireportjson` and `logs` directories.
3. **Verify**: The receiver will automatically start on a port assigned by IIS and begin saving JSON files to `ireportjson/`.

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
