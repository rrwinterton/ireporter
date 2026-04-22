#!/usr/bin/env python3
import os
import sys
import subprocess
import traceback
import importlib.util

# 1. Absolute Path Setup
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)

# 2. Local Library Support
LOCAL_LIB = os.path.join(SCRIPT_DIR, 'local_libs')
if not os.path.exists(LOCAL_LIB):
    os.makedirs(LOCAL_LIB)
sys.path.insert(0, LOCAL_LIB)

def ensure_flask():
    """Try to import flask, or install it locally if missing."""
    try:
        import flask
    except ImportError:
        try:
            subprocess.check_call([
                sys.executable, "-m", "pip", "install", 
                "--target", LOCAL_LIB, "flask"
            ])
        except Exception:
            pass

# 3. Main CGI Execution
try:
    ensure_flask()
    
    from wsgiref.handlers import CGIHandler
    
    # Load ireporter.py by its full path
    module_path = os.path.join(SCRIPT_DIR, "ireporter.py")
    spec = importlib.util.spec_from_file_location("ireporter", module_path)
    ireporter = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ireporter)
    
    # Run the Flask app
    CGIHandler().run(ireporter.app)
    
    # CRITICAL: Force immediate exit to prevent server from appending extra headers/text
    sys.exit(0)

except Exception:
    # If it fails, report the error and exit
    print("Content-Type: text/plain; charset=utf-8\n")
    print("Error starting Flask via CGI:\n")
    print(traceback.format_exc())
    sys.exit(1)
